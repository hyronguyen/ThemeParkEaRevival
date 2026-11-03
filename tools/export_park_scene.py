"""Assemble only code-verified static park geometry from original resources.

Evidence and deliberately unresolved state: analysis/scene_formats.md.
"""
import hashlib
import json
from pathlib import Path
import struct
import zipfile
from export_park_layout import decode_table, file_sha

ROOT = Path(__file__).resolve().parents[1]
PATH_MODELS = (306,307,308,309,310,311,312,313,314,315,316,319,317,320,318,321)
MASK_TO_TYPE = (0,5,3,10,6,2,9,12,4,8,1,13,7,11,14,15)
PATH_TEXTURE = 'models/textures/00000012.png'

def center(x, y, width, height):
    if width <= 0 or height <= 0:
        raise ValueError('Nonpositive footprint')
    return (x + width / 2) * 32, -(y + height / 2) * 32

def path_mask(x, y, coordinates):
    return sum(bit for dx,dy,bit in ((-1,0,1),(0,-1,2),(1,0,4),(0,1,8))
               if (x+dx,y+dy) in coordinates)

def assemble(tables, model_ids):
    instances = []
    def add(kind, row, model, xy, z, angle=0, texture='-', **extra):
        if model not in model_ids:
            raise ValueError(f'Missing original model {model}')
        instances.append(dict(kind=kind, source_row=row, model=model, x=xy[0], y=xy[1],
                              z=z, angle=angle, texture=texture, **extra))
    for i,(x,y,w,h,territory) in enumerate(tables['guidelines']):
        # Model 392 is a fixed 36x36-cell plane. Do not stretch unknown terrain.
        if (w,h) != (36,36):
            raise ValueError('Unverified grass dimensions')
        add('G',i,392,center(x,y,w,h),-5,territory=territory)
    for i,(record,model,x,y,w,h,angle) in enumerate(tables['dividerWalls']):
        add('W',i,model,center(x,y,w,h),0,angle,record_id=record)
    coordinates = {}
    for i,(x,y) in enumerate(tables['paths']):
        coordinates.setdefault((x,y),[]).append(i)
    for (x,y),rows in coordinates.items():
        mask = path_mask(x,y,coordinates)
        add('P',rows[0],PATH_MODELS[MASK_TO_TYPE[mask]],center(x,y,1,1),-1,
            texture=PATH_TEXTURE, neighbor_mask=mask, source_rows=rows)
    return instances

def main():
    layout = json.loads((ROOT/'analysis/park_layout.json').read_text())
    package = ROOT/layout['source']
    if file_sha(package) != layout['source_sha256']:
        raise ValueError('Original archive hash mismatch')
    # This table is original code data, not a visual guess about path shapes.
    with zipfile.ZipFile(package) as archive:
        binary = archive.read('Payload/ThemePark.app/ThemePark')
    if struct.unpack_from('<16i',binary,0x4cba70-0x4000) != PATH_MODELS:
        raise ValueError('Original path model table differs')
    tables = {}
    for name in ('guidelines','dividerWalls','paths'):
        metadata = layout['tables'][name]
        raw = (ROOT/'extracted/level_data'/package.stem/'named'/(name+'.bin')).read_bytes()
        rows = decode_table(raw,metadata['record_stride_int32'])
        if hashlib.sha256(raw).hexdigest() != metadata['sha256'] or rows != metadata['rows']:
            raise ValueError(f'Layout provenance mismatch: {name}')
        tables[name] = rows
    models = json.loads((ROOT/'analysis/models.json').read_text())['models']
    instances = assemble(tables,{m['model_id'] for m in models})
    for item in instances:
        if item['texture'] != '-' and not (ROOT/'extracted'/item['texture']).is_file():
            raise ValueError('Missing default path texture')
    counts = {k:sum(i['kind']==k for i in instances) for k in 'GWP'}
    report = dict(schema_version=1, source=layout['source'], source_sha256=layout['source_sha256'],
                  state='static layout with default grass/path theme; no ownership or attractions',
                  tables={n:{k:v for k,v in layout['tables'][n].items() if k!='rows'} for n in tables},
                  counts=counts, path_input_rows=len(tables['paths']),
                  duplicate_path_rows=len(tables['paths'])-counts['P'], instances=instances)
    (ROOT/'analysis/park_scene.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
    output = ROOT/'extracted/runtime/park_scene.tsv'
    output.write_text(f'TPS1 {len(instances)}\n'+''.join(
        ' '.join(str(i[k]) for k in ('kind','source_row','model','x','y','z','angle','texture'))+'\n'
        for i in instances),encoding='utf-8')
    print(json.dumps(dict(instances=len(instances),**counts,duplicate_path_rows=report['duplicate_path_rows'])))

if __name__ == '__main__':
    main()
