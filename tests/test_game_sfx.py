import hashlib,json,unittest,wave
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
class GameSFX(unittest.TestCase):
 def test_pcm_matches_original(self):
  folder=ROOT/'extracted/runtime/audio/sfx';rows=json.loads((folder/'provenance.json').read_text());self.assertEqual(len(rows),7)
  original=ROOT/'extracted/unknown/Theme_Park_1.8.3_ios_5.1.1/members/Payload/ThemePark.app'
  for row in rows:
   raw=(original/f"wv{row['sample']:04x}").read_bytes()
   with wave.open(str(folder/(row['event']+'.wav')),'rb') as w:
    self.assertEqual((w.getnchannels(),w.getsampwidth(),w.getframerate()),(1,2,row['rate']))
    self.assertEqual(w.readframes(w.getnframes()),raw)
   self.assertEqual(hashlib.sha256(raw).hexdigest(),row['sha256'])
