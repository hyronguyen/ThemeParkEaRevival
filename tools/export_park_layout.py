"""Export original BFC layout arrays; never synthesize missing park state.

Names were recovered from Mach-O UTF16 strings and PMFile::calcCRC.
Paths' coordinate semantics and slotData's first five fields are verified by
loadRideSlots, RideSlot::read and RideSlot::getSlotCenter3D instructions.
"""
import hashlib
import json
from pathlib import Path
import struct
import zipfile
import zlib
from extract_assets import parse_bfc_index, file_sha

ROOT=Path(__file__).resolve().parents[1]
TABLES={'paths':2,'slotData':10,'themeSlots':9,'guidelines':5,
        'entrances_b':4,'dividerWalls':7,'decorations':4,'themedDecData':7}
ANIMATIONS=('scaffHammerAnim','scaffBrushAnim','scaffWalkAnim','sharkIdle','bearIdle','walkAnim2')

def decode_table(data,stride):
    if len(data)<4 or len(data)%4: raise ValueError('Invalid int32 array length')
    words=struct.unpack('>'+str(len(data)//4)+'i',data)
    count=words[0]
    if count<0 or len(words)!=1+count*stride: raise ValueError('Count/stride mismatch')
    return [list(words[1+i*stride:1+(i+1)*stride]) for i in range(count)]

def main():
    package=ROOT/'resource/Theme_Park_1.8.3_ios_5.1.1.ipa'
    if not package.exists():package=ROOT/'input/ThemePark.ipa'
    before=file_sha(package)
    result={'schema_version':1,'source':package.relative_to(ROOT).as_posix(),'source_sha256':before,'tables':{},'animation_indices':{}}
    out=ROOT/'extracted/level_data'/package.stem/'named';out.mkdir(parents=True,exist_ok=True)
    with zipfile.ZipFile(package) as z:
        index='Payload/ThemePark.app/head_ipad3.bfc'
        entries={int(r['resource_id'],16):r for r in parse_bfc_index(z.read(index))}
        packs={}
        for name,stride in {**TABLES,**dict.fromkeys(ANIMATIONS,1)}.items():
            key=zlib.crc32(name.encode('ascii'))
            r=entries[key]
            if r['pack'] not in packs:packs[r['pack']]=z.read(f'Payload/ThemePark.app/{r["pack"]}.bfc')
            pack=packs[r['pack']]
            if r['offset']+r['size']>len(pack):raise ValueError('Resource outside BFC range')
            raw=pack[r['offset']:r['offset']+r['size']]
            rows=decode_table(raw,stride)
            (out/(name+'.bin')).write_bytes(raw)
            table={'original_name':name,'resource_id':f'{key:08x}','index':index,'bfc_number':r['pack'],
                   'offset':r['offset'],'byte_length':r['size'],'sha256':hashlib.sha256(raw).hexdigest(),
                   'record_count':len(rows),'record_stride_int32':stride,'byte_order':'big','rows':rows,
                   'semantics':('verified path tile x,y' if name=='paths' else
                       'first five fields: slot id, x, y, width, height; remaining fields retained without interpretation' if name=='slotData' else
                       'field semantics unresolved; preserve original order')}
            result['tables' if name in TABLES else 'animation_indices'][name]=table
            (out/(name+'.json')).write_text(json.dumps(table,indent=2),encoding='utf-8')
    result['source_sha256_after']=file_sha(package)
    if result['source_sha256_after']!=before:raise ValueError('Source archive changed')
    runtime=ROOT/'extracted/runtime';runtime.mkdir(parents=True,exist_ok=True)
    paths=result['tables']['paths']['rows']
    (runtime/'park_layout.tsv').write_text(
        '# Original paths and slotData resources. P x y / S id x y width height.\n'+
        ''.join(f'P\t{x}\t{y}\n' for x,y in paths)+
        ''.join('S\t'+'\t'.join(map(str,row[:5]))+'\n' for row in result['tables']['slotData']['rows']),encoding='utf-8')
    (ROOT/'analysis/park_layout.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
    print(json.dumps({name:t['record_count'] for name,t in result['tables'].items()}))
    print('Original path bounds:', min(p[0] for p in paths),max(p[0] for p in paths),min(p[1] for p in paths),max(p[1] for p in paths))

if __name__=='__main__':main()
