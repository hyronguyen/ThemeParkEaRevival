"""Recover PM mesh/material/model tables from verified iOS reader layouts.

Generated TPM files are independent preview interchange, not a claimed original
format. Source bytes and provenance remain in the BFC inventory.
"""
import json,struct,math,hashlib
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]

class Reader:
    def __init__(self,data):self.data=data;self.pos=0
    def read(self,fmt):
        n=struct.calcsize('>'+fmt)
        if self.pos+n>len(self.data):raise ValueError('Truncated model resource')
        value=struct.unpack_from('>'+fmt,self.data,self.pos);self.pos+=n
        return value[0] if len(value)==1 else value
    def done(self):
        if self.pos!=len(self.data):raise ValueError('Unexpected resource trailing bytes')

def mesh_table(data):
    r=Reader(data);count=r.read('H');result=[]
    for _ in range(count):
        resource,flags,*bounds=r.read('hh6f')
        if flags&~7 or not flags&1 or not all(math.isfinite(x) for x in bounds):raise ValueError('Unsupported mesh metadata')
        result.append({'resource':resource,'flags':flags,'bounds':bounds})
    r.done();return result

def mesh_links(data):
    r=Reader(data);result=[]
    for _ in range(r.read('H')):
        nodes=[]
        for _ in range(r.read('H')):nodes.append([r.read('h') for _ in range(r.read('H'))])
        result.append(nodes)
    r.done();return result

def materials(data):
    r=Reader(data);rows=[r.read('fh') for _ in range(r.read('H'))];r.done();return rows

def image_table(data):
    r=Reader(data);ni,nt=r.read('HH')
    images=[r.read('8h') for _ in range(ni)]
    textures=[r.read('Bii') for _ in range(nt)];r.done()
    if any(i[0]<0 or i[0]>=nt for i in images):raise ValueError('Invalid image texture index')
    return images,textures

def packed_pixels(raw,kind):
    if len(raw)%2 or kind not in (11,13):raise ValueError('Invalid packed texture')
    out=bytearray()
    for (v,) in struct.iter_unpack('<H',raw):
        if kind==11: r,g,b,a=(v>>11)&31,(v>>5)&63,v&31,255;g=(g<<2)|(g>>4)
        else: r,g,b,a=(v>>11)&31,(v>>6)&31,(v>>1)&31,(v&1)*255;g=(g<<3)|(g>>2)
        out.extend(((r<<3)|(r>>2),g,(b<<3)|(b>>2),a))
    return bytes(out)

def model_data(data):
    r=Reader(data);model_id,count=r.read('HH');nodes=[]
    for i in range(count):
        parent,kind,name_hash,material=r.read('bbIh');transform=list(r.read('10f'))
        if parent>=i or parent < -1 or not all(math.isfinite(x) for x in transform):raise ValueError('Invalid model hierarchy/transform')
        nodes.append(dict(parent=parent,kind=kind,name_hash=f'{name_hash:08x}',material=material,transform=transform))
    r.done();return model_id,nodes

def vertices(data,flags):
    stride=(3 if flags&1 else 0)+(3 if flags&2 else 0)+(2 if flags&4 else 0)
    if not stride or len(data)%(stride*4*3):raise ValueError('Not a complete triangle stream')
    values=struct.unpack('>'+str(len(data)//4)+'f',data)
    if not all(math.isfinite(x) and abs(x)<1e8 for x in values):raise ValueError('Invalid mesh float')
    return [values[i:i+stride] for i in range(0,len(values),stride)]

def main():
    inv=json.loads((ROOT/'analysis/asset_inventory.json').read_text())
    resources={int(ref['resource_id'],16):record for record in inv['bfc_resources'] for ref in record['references'] if 'head_ipad3.' in ref['index_path']}
    used={}
    def raw(key):
        record=resources[key];b=(ROOT/record['output']).read_bytes()
        if hashlib.sha256(b).hexdigest()!=record['sha256']:raise ValueError('Extracted source hash mismatch')
        used[f'{key:08x}']={k:record[k] for k in ('container_path','offset','size','sha256','output')}
        return b
    meshes=mesh_table(raw(0x8ac7bdd4));links=mesh_links(raw(0x8111949b));mats=materials(raw(0xb0529f7a))
    id_bytes=raw(0xe34742d0);ids=struct.unpack('>'+str(len(id_bytes)//4)+'i',id_bytes)
    images,textures=image_table(raw(0xc9261c1f))
    texture_cache={}
    def texture_path(image_id):
        if image_id==-1:return -1,'-'
        if not 0<=image_id<len(images):raise ValueError('Invalid material image index')
        image=images[image_id]
        if image[4] or image[5]:raise ValueError('Cropped model image needs UV transform')
        kind,rid,_=textures[image[0]];kind&=15
        if rid in texture_cache:return rid,texture_cache[rid]
        if kind not in (11,12,13):return rid,None
        from extract_assets import decode_texture
        from PIL import Image
        w,h,fmt,pixels=decode_texture(raw(rid))
        if kind==12 and fmt!='RGBA8':raise ValueError('RGBA image type/size mismatch')
        if kind in (11,13):
            if len(pixels)!=w*h*2:raise ValueError('Packed image type/size mismatch')
            pixels=packed_pixels(pixels,kind)
        path=ROOT/f'extracted/models/textures/{rid:08x}.png';path.parent.mkdir(parents=True,exist_ok=True)
        Image.frombytes('RGBA',(w,h),pixels).save(path)
        texture_cache[rid]=path.relative_to(ROOT/'extracted').as_posix()
        return rid,texture_cache[rid]
    target=ROOT/'extracted/models/ios';target.mkdir(parents=True,exist_ok=True)
    summary=[];catalog=[];mesh_cache={}
    for index,rid in enumerate(ids):
        model_id,nodes=model_data(raw(rid))
        if model_id>=len(links) or len(links[model_id])!=len(nodes):raise ValueError('Model/node link table mismatch')
        parts=[];missing=[]
        for ni,node in enumerate(nodes):
            if node['kind']!=1:continue
            linked=links[model_id][ni]
            if not linked or linked[0]<0:continue
            mi=linked[0] # PMMeshRenderer::render selects first link.
            meta=meshes[mi];vrid=meta['resource']
            if vrid not in mesh_cache:mesh_cache[vrid]=vertices(raw(vrid),meta['flags'])
            verts=mesh_cache[vrid];alpha,img=mats[node['material']]
            tex,path=texture_path(img)
            if path is None:missing.append(img);continue
            parts.append(dict(node=ni,mesh=mi,vertex_resource=vrid,flags=meta['flags'],image_id=img,texture_id=tex,alpha=alpha,
                              texture=path,vertices=verts))
        item=dict(index=index,model_id=model_id,resource_id=rid,nodes=nodes,parts=[{k:v for k,v in p.items() if k!='vertices'} for p in parts],missing_textures=missing,
                  triangles=sum(len(p['vertices'])//3 for p in parts))
        # Export only complete textured assemblies; never silently omit a part.
        if parts and not missing:
            path=target/f'model_{index:04d}.tpm'
            with path.open('w',encoding='utf-8',newline='\n') as f:
                f.write(f'TPM1 {index} {len(nodes)} {len(parts)}\n')
                for n in nodes:f.write('N '+str(n['parent'])+' '+' '.join(format(x,'.9g') for x in n['transform'])+'\n')
                for p in parts:
                    f.write(f'P {p["node"]} {p["mesh"]} {p["alpha"]:.9g} {len(p["vertices"])} {p["texture"]}\n')
                    for v in p['vertices']:
                        uv=v[-2:] if p['flags']&4 else (0,0)
                        f.write(' '.join(format(x,'.9g') for x in (*v[:3],*uv))+'\n')
            item['output']=path.relative_to(ROOT).as_posix();catalog.append(f'{index}\tmodels/ios/{path.name}\t{item["triangles"]}')
        summary.append(item)
    report=dict(mesh_count=len(meshes),model_count=len(summary),material_count=len(mats),exported=len(catalog),models=summary,source_resources=used)
    (ROOT/'analysis/models.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
    (ROOT/'extracted/runtime/models.tsv').write_text('# Original complete model assemblies\n'+'\n'.join(catalog)+'\n',encoding='utf-8')
    print(json.dumps({k:report[k] for k in ('mesh_count','model_count','material_count','exported')}))
    print('Small assemblies using castle atlas texture 104:',[(m['index'],m['triangles'],len(m['nodes'])) for m in summary if any(p['texture_id']==104 for p in m['parts'])][:30])

if __name__=='__main__':main()
