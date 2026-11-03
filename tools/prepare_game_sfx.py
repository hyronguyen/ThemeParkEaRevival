"""Wrap original PCM samples using PAC metadata. Mappings follow inspected original callsites; see docs/sfx-original-mapping.md."""
import hashlib,json,struct,wave
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
EVENTS={'click':137,'select':135,'build':93,'collect':131,'sell':84,'unlock':87,'cheer':10}
def extract():
 p=ROOT/'extracted/unknown/Theme_Park_1.8.3_ios_5.1.1/members/Payload/ThemePark.app';data=(p/'audio_ipad3.pac').read_bytes()
 magic,version,reserved,count,playables=struct.unpack_from('<5I',data)
 if (magic,version,reserved,count,playables)!=(0x13370226,1,0,141,134):raise ValueError('Unexpected PAC header')
 samples={}
 for i in range(count):
  tag,index,encoding,size=struct.unpack_from('<4I',data,20+i*16)
  if tag!=0x10000001 or index!=i:raise ValueError('Invalid sample entry')
  samples[index]=(encoding,size)
 offset=20+count*16;rates={}
 for _ in range(playables):
  tag,index,kind,flags=struct.unpack_from('<4I',data,offset);offset+=16
  if tag!=0x20000001:raise ValueError('Invalid playable')
  if kind==1:
   sample,rate=struct.unpack_from('<2I',data,offset);offset+=8
   if rate not in (11025,16000,22050,44100,96000):raise ValueError('Unexpected rate')
   rates[sample]=rate
  elif kind!=0 or index!=141:raise ValueError('Unsupported playable kind')
 if offset!=len(data)-4:raise ValueError('Trailing PAC data')
 out=ROOT/'extracted/runtime/audio/sfx';out.mkdir(exist_ok=True);manifest=[]
 for event,index in EVENTS.items():
  encoding,size=samples[index];raw=(p/f'wv{index:04x}').read_bytes()
  if encoding!=1 or len(raw)!=size or size%2:raise ValueError('Invalid PCM16 sample')
  with wave.open(str(out/f'{event}.wav'),'wb') as f:f.setparams((1,2,rates[index],0,'NONE','not compressed'));f.writeframes(raw)
  manifest.append(dict(event=event,sample=index,rate=rates[index],bytes=size,sha256=hashlib.sha256(raw).hexdigest(),mapping='guest_voice_local_cheer_event' if event=='cheer' else 'original_callsite' ))
 (out/'provenance.json').write_text(json.dumps(manifest,indent=2));return manifest
if __name__=='__main__':print(json.dumps(extract(),indent=2))

