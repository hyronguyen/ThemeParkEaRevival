"""Focused read-only Thumb listings for model format recovery."""
import sys,json,bisect,zipfile
from pathlib import Path
sys.path.insert(0,str(Path(__file__).parent/'_deps'))
from capstone import Cs,CS_ARCH_ARM,CS_MODE_THUMB
root=Path(__file__).resolve().parents[1]
symbols=[s for s in json.loads((root/'analysis/ios/symbols.json').read_text()) if not s['is_debug'] and s['section']==1]
by={}
for s in symbols:by.setdefault(s['value'],[]).append(s['name'])
addresses=sorted(by)
patterns=sys.argv[1:] or ['13PMMeshManager4load','6PMMesh6render','11FFModelDataC2','14FFModelManager4init','10FFMeshNode4initEv']
with zipfile.ZipFile(root/'resource/Theme_Park_1.8.3_ios_5.1.1.ipa') as z:binary=z.read('Payload/ThemePark.app/ThemePark')
engine=Cs(CS_ARCH_ARM,CS_MODE_THUMB)
lines=[]
for s in symbols:
    if not any(p in s['name'] for p in patterns):continue
    end=addresses[bisect.bisect_right(addresses,s['value'])]
    lines.append('\n'+s['name']+' VA '+hex(s['value'])+' file '+hex(s['file_offset']))
    for i in engine.disasm(binary[s['file_offset']:s['file_offset']+end-s['value']],s['value']):
        names=[]
        if i.mnemonic in ('bl','blx','b.w') and i.op_str.startswith('#'):names=by.get(int(i.op_str[1:],0),[])
        lines.append(f'{i.address:08x} {i.mnemonic:10} {i.op_str} '+','.join(names))
out=root/'analysis/ios/model_code.txt';out.write_text('\n'.join(lines),encoding='utf-8')
print('\n'.join(lines))
