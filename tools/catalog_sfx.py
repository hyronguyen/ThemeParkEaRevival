"""Read-only source audit and audition export; does not change game assignments."""
import csv, hashlib, html, json, struct, wave
from pathlib import Path
from prepare_game_sfx import EVENTS
ROOT=Path(__file__).resolve().parents[1]
def main():
    source=ROOT/'extracted/unknown/Theme_Park_1.8.3_ios_5.1.1/members/Payload/ThemePark.app'
    data=(source/'audio_ipad3.pac').read_bytes()
    magic,version,reserved,count,playables=struct.unpack_from('<5I',data)
    assert (magic,version,reserved,count,playables)==(0x13370226,1,0,141,134)
    offset=20+count*16; rates={}; playable_ids={}
    for _ in range(playables):
        tag,index,kind,flags=struct.unpack_from('<4I',data,offset);offset+=16
        assert tag==0x20000001
        if kind==1:
            sample,rate=struct.unpack_from('<2I',data,offset);offset+=8
            assert sample not in rates
            rates[sample]=rate;playable_ids[sample]=index
        else: assert kind==0 and index==141
    assert offset==len(data)-4
    out=ROOT/'analysis/sfx-catalog';(out/'audio').mkdir(parents=True,exist_ok=True)
    rows=[]
    for index in range(count):
        tag,sample,encoding,size=struct.unpack_from('<4I',data,20+index*16)
        assert tag==0x10000001 and sample==index
        filename=f'wv{index:04x}';raw=(source/filename).read_bytes();assert len(raw)==size
        rate=rates.get(index);output='';duration=''
        status='Thiếu tần số trong playable table' if rate is None else 'Encoding 0: chưa xác minh cách giải mã' if encoding!=1 else 'Nghe được — PCM16 mono'
        if rate and encoding==1:
            assert size%2==0
            output=f'audio/SFX-{index:03d}.wav';duration=round(size/2/rate,3)
            with wave.open(str(out/output),'wb') as f:
                f.setparams((1,2,rate,0,'NONE','not compressed'));f.writeframes(raw)
            with wave.open(str(out/output),'rb') as f: assert f.readframes(f.getnframes())==raw
        rows.append(dict(id=f'SFX-{index:03d}',sample=index,source=filename,playable=playable_ids.get(index,''),encoding=encoding,rate=rate or '',seconds=duration,bytes=size,current_event=next((e for e,i in EVENTS.items() if i==index),''),status=status,output=output,sha256=hashlib.sha256(raw).hexdigest()))
    (out/'catalog.json').write_text(json.dumps(rows,ensure_ascii=False,indent=2),encoding='utf-8')
    with (out/'catalog.csv').open('w',encoding='utf-8-sig',newline='') as f:
        writer=csv.DictWriter(f,fieldnames=list(rows[0]));writer.writeheader();writer.writerows(rows)
    table=[]
    for r in rows:
        player=f'<audio controls preload="none" src="{r["output"]}"></audio>' if r['output'] else html.escape(r['status'])
        table.append(f'<tr data-used="{bool(r["current_event"])}"><td><strong>{r["id"]}</strong><small>{r["source"]}</small></td><td>{r["current_event"] or "—"}</td><td>{r["seconds"]} s<br><small>{r["rate"] or "?"} Hz · encoding {r["encoding"]}</small></td><td>{player}</td></tr>')
    pn=ROOT/'extracted/audio/Theme_Park_1.8.3_ios_5.1.1/members/Payload/ThemePark.app/pnsound.wav'
    if pn.exists(): (out/'audio/pnsound.wav').write_bytes(pn.read_bytes())
    page="""<!doctype html><html lang="vi"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Theme Park — SFX catalog</title>
<style>body{font:16px system-ui;background:#f3f5f8;color:#17283c;max-width:1100px;margin:32px auto;padding:0 20px}h1{margin-bottom:8px}p{line-height:1.6}header{background:white;padding:24px;border-radius:16px}input,button,select{font:inherit;padding:10px;border:1px solid #ccd4df;border-radius:8px}nav{position:sticky;top:0;background:#f3f5f8;padding:16px 0;display:flex;gap:10px;flex-wrap:wrap}table{width:100%;border-collapse:collapse;background:white}th,td{padding:14px;text-align:left;border-bottom:1px solid #e4e9ee}small{display:block;color:#617086;margin-top:5px}audio{width:320px;max-width:100%}th{background:#dfe8f3}.hint{color:#526274}</style>
<header><h1>Kho âm thanh Theme Park</h1><p>141 sample trong PAC iPad3 • 129 bản PCM16 nghe được • 12 bản chờ metadata/giải mã.<br>Tên sự kiện là mapping bản PC; sáu mẫu đang dùng đã đối chiếu vị trí gọi trong mã gốc. Chưa phân loại tiếng bằng tai. Chọn theo ID, ví dụ: <b>click → SFX-138</b>.</p><p class="hint">Mỗi lần chỉ phát một âm. Âm lượng nghe thử mặc định 35%. Không tự phát, không sửa cấu hình game.</p></header>
<nav><input id="search" placeholder="Tìm ID, wv hoặc sự kiện"><select id="filter"><option value="all">Tất cả 141 mẫu</option value="used">6 mẫu đang dùng</option></select><button id="stop">Dừng âm đang phát</button><span id="count"></span></nav>
<table><thead><tr><th>ID / file gốc</th><th>Sự kiện hiện tại</th><th>Độ dài / định dạng</th><th>Nghe thử</th></tr></thead><tbody>ROWS</tbody></table>
<h2>WAV riêng ngoài PAC</h2><p>pnsound.wav — giữ tên file gốc, chưa xác minh sự kiện.</p><audio controls preload="none" src="audio/pnsound.wav"></audio>
<h2>Nhạc nền (không tính vào SFX)</h2><p>audio_music_track_1.mp3 · audio_music_track_2.mp3 · audio_music_track_3.mp3</p>
<script>const players=[...document.querySelectorAll('audio')];players.forEach(a=>{a.volume=.35;a.addEventListener('play',()=>players.forEach(b=>{if(b!==a){b.pause();b.currentTime=0}}))});document.querySelector('#stop').onclick=()=>players.forEach(a=>{a.pause();a.currentTime=0});function filter(){let n=0;document.querySelectorAll('tbody tr').forEach(r=>{let show=r.textContent.toLowerCase().includes(document.querySelector('#search').value.toLowerCase())&&(document.querySelector('#filter').value!=='used'||r.dataset.used==='True');r.hidden=!show;if(show)n++});document.querySelector('#count').textContent=n+' mẫu'}document.querySelector('#search').oninput=filter;document.querySelector('#filter').onchange=filter;filter();</script></html>"""
    (out/'index.html').write_text(page.replace('ROWS',''.join(table)),encoding='utf-8')
    lines=['# Danh mục SFX hiện có', '', 'Nguồn: audio_ipad3.pac và wvXXXX trong asset iOS. Không đổi mapping game.', '', '141 samples; 133 sample có playable metadata; 129 PCM16 đã bọc WAV, giữ nguyên payload; 4 encoding 0 có rate chưa giải mã; 8 sample không có rate từ playable table. Ngoài PAC có pnsound.wav. Ba MP3 là nhạc nền.', '', '| ID | File gốc | Giây | Hz | Sự kiện tạm | Trạng thái |','|---|---|---:|---:|---|---|']
    for r in rows: lines.append(f'| {r["id"]} | {r["source"]} | {r["seconds"]} | {r["rate"]} | {r["current_event"] or "—"} | {r["status"]} |')
    (ROOT/'docs/sfx-catalog.md').write_text('\n'.join(lines)+'\n',encoding='utf-8')
    print(json.dumps(dict(samples=len(rows),audition_wavs=sum(bool(r['output']) for r in rows),pending=[r['id'] for r in rows if not r['output']],page=str(out/'index.html')),ensure_ascii=False))
if __name__=='__main__':main()
