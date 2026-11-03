"""Recover FFModel's vis_loc CRCs; keep transforms but suppress marker meshes."""
import json,zipfile,zlib
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def main():
    with zipfile.ZipFile(ROOT/'resource/Theme_Park_1.8.3_ios_5.1.1.ipa') as z:
        binary=z.read('Payload/ThemePark.app/ThemePark')
    offset=0x4cfb00;names=[]
    while True:
        end=offset
        while binary[end:end+2]!=b'\0\0':end+=2
        name=binary[offset:end].decode('utf-16le')
        if not name.startswith('vis_loc'):break
        names.append(name);offset=end+2
    if names!=[f'vis_loc{i:02}' for i in range(1,len(names)+1)] or len(names)<5:
        raise ValueError('Unexpected original locator names')
    hashes={f'{zlib.crc32(n.encode("ascii")):08x}':i+1 for i,n in enumerate(names)}
    models=json.loads((ROOT/'analysis/models.json').read_text())['models'];rows=[]
    for m in models:
        for index,n in enumerate(m['nodes']):
            if n['name_hash'] in hashes:rows.append((m['model_id'],index,hashes[n['name_hash']]))
    (ROOT/'extracted/runtime/model_locators.tsv').write_text(''.join(f'{m} {n} {i}\n' for m,n,i in rows),encoding='utf-8')
    print(json.dumps(dict(original_names=len(names),locator_nodes=len(rows),selected=[r for r in rows if r[0] in (404,425)])))
if __name__=='__main__':main()
