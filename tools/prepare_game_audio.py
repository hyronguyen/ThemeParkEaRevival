"""Stage the three original MP3 tracks, preserving bytes and provenance."""
import hashlib,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def main():
    inventory=json.loads((ROOT/'analysis/asset_inventory.json').read_text())
    out=ROOT/'extracted/runtime/audio';out.mkdir(parents=True,exist_ok=True);records=[]
    for index in range(1,4):
        name=f'audio_music_track_{index}.mp3'
        matches=[r for r in inventory['archive_members'] if r['path']==f'Payload/ThemePark.app/{name}' and r.get('format')=='MP3']
        if len(matches)!=1:raise ValueError('Missing or ambiguous original music')
        record=matches[0];data=(ROOT/record['output']).read_bytes()
        if hashlib.sha256(data).hexdigest()!=record['sha256']:raise ValueError('Original audio hash mismatch')
        target=out/f'track{index}.mp3';target.write_bytes(data)
        records.append(dict(source=record['path'],sha256=record['sha256'],output=target.relative_to(ROOT).as_posix(),size=len(data)))
    (out/'provenance.json').write_text(json.dumps(records,indent=2),encoding='utf-8');print(f'Staged {len(records)} original MP3 tracks unchanged.')
if __name__=='__main__':main()
