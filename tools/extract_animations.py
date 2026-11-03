"""Recover original transform tracks and code-proven idle bindings.

Evidence: PMAnimationManager::init / PMAnimationTask::run / PMAnimationData::getFrame.
"""
import hashlib,json,math,struct
from pathlib import Path
from extract_models import Reader
ROOT=Path(__file__).resolve().parents[1]

def track_table(data):
    r=Reader(data);rows=[r.read('hh') for _ in range(r.read('H'))];r.done()
    if any(rid<0 or n<1 for rid,n in rows):raise ValueError('Invalid animation metadata')
    return rows

def groups_table(data):
    r=Reader(data);rows=[]
    for _ in range(r.read('H')):rows.append([r.read('h') for _ in range(r.read('H'))])
    r.done();return rows

def track_data(data,count):
    r=Reader(data);first=list(r.read('10f'));flags=r.read('H')
    if flags&~1023:raise ValueError('Unknown animation channel flag')
    channels=[]
    for i in range(10):
        values=[first[i]] if flags&(1<<i) else [r.read('f') for _ in range(count)]
        if not all(math.isfinite(x) for x in values):raise ValueError('Invalid animation value')
        channels.append(values)
    r.done();return dict(first=first,flags=flags,count=count,channels=channels)

# Ride::playHackyIdleAnim, VA 0xb3174; model field also consumed at 0x98c0a.
IDLE_BINDINGS={425:47,427:49,515:151,522:159}

def main():
    inv=json.loads((ROOT/'analysis/asset_inventory.json').read_text())
    resources={int(ref['resource_id'],16):record for record in inv['bfc_resources'] for ref in record['references'] if 'head_ipad3.' in ref['index_path']}
    sources={}
    def raw(key):
        r=resources[key];b=(ROOT/r['output']).read_bytes()
        if hashlib.sha256(b).hexdigest()!=r['sha256']:raise ValueError('Animation source hash mismatch')
        sources[str(key)]={k:r[k] for k in ('container_path','offset','size','sha256','output')};return b
    table=track_table(raw(0x96f19831));groups=groups_table(raw(0xdb5513ca))
    tracks=[dict(index=i,resource_id=rid,**track_data(raw(rid),n)) for i,(rid,n) in enumerate(table)]
    models=json.loads((ROOT/'analysis/models.json').read_text())['models'];bindings=[]
    folder=ROOT/'extracted/animations/ios';folder.mkdir(parents=True,exist_ok=True)
    for gi,links in enumerate(groups):
        if any(i<0 or i>=len(tracks) for i in links):raise ValueError('Invalid animation group link')
        path=folder/f'group_{gi:04d}.tpa'
        with path.open('w',encoding='utf-8',newline='\n') as f:
            f.write(f'TPA1 {gi} {len(links)} 33\n')
            for i in links:
                t=tracks[i];f.write(f'T {i} {t["count"]}\n')
                for channel in t['channels']:f.write(str(len(channel))+' '+' '.join(format(v,'.9g') for v in channel)+'\n')
        bindings.append(dict(group=gi,nodes=len(links),tracks=links,output=path.relative_to(ROOT).as_posix()))
    rows=[]
    for mi,gi in IDLE_BINDINGS.items():
        if len(models[mi]['nodes'])!=len(groups[gi]):raise ValueError('Code-proven model/group node count mismatch')
        rows.append(f'{mi}\t{gi}\tanimations/ios/group_{gi:04d}.tpa')
    (ROOT/'extracted/runtime/animations.tsv').write_text('# Code-proven idle bindings: Ride::playHackyIdleAnim\n'+'\n'.join(rows)+'\n',encoding='utf-8')
    report=dict(track_count=len(tracks),group_count=len(groups),frame_ms=33,tracks=tracks,groups=bindings,idle_bindings=IDLE_BINDINGS,source_resources=sources)
    (ROOT/'analysis/animations.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
    print('Tracks:',len(tracks),'Groups:',len(groups),'Verified idle bindings:',IDLE_BINDINGS)
if __name__=='__main__':main()
