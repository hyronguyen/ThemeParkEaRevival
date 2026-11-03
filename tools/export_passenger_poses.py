"""Code-proven special passenger poses; never infer operating ride groups."""
import json, struct, hashlib, zipfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def recover():
    inv=json.loads((ROOT/'analysis/asset_inventory.json').read_text())
    resources={int(ref['resource_id'],16):r for r in inv['bfc_resources'] for ref in r['references'] if 'head_ipad3.' in ref['index_path']}
    with zipfile.ZipFile(ROOT/'resource/Theme_Park_1.8.3_ios_5.1.1.ipa') as z:binary=z.read('Payload/ThemePark.app/ThemePark')
    result=[]
    # Ride::playRidingAnim at 0xb287e (521) and 0xb28cc (503).
    for ride,offset in [(521,0x4c7b58),(503,0x4c7b68)]:
        for variant,key in enumerate(struct.unpack_from('<4I',binary,offset)):
            r=resources[key];raw=(ROOT/r['output']).read_bytes()
            if hashlib.sha256(raw).hexdigest()!=r['sha256']:raise ValueError('Passenger source changed')
            count=struct.unpack_from('>I',raw)[0]
            if len(raw)!=4*(count+1) or count!=1:raise ValueError('Unexpected passenger pose set')
            pose=struct.unpack_from('>I',raw,4)[0]
            if pose>=730:raise ValueError('Invalid passenger model')
            result.append(dict(ride=ride,variant=variant,model=pose,resource=f'{key:08x}',sha256=r['sha256']))
    return result
if __name__=='__main__':
    rows=recover();(ROOT/'extracted/runtime/passenger_poses.tsv').write_text(''.join(f"{r['ride']} {r['variant']} {r['model']}\n" for r in rows))
    (ROOT/'analysis/passenger_poses.json').write_text(json.dumps(rows,indent=2));print(rows)
