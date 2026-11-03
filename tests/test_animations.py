import os,json,pathlib,struct,sys,unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from extract_animations import track_table,groups_table,track_data,IDLE_BINDINGS

class AnimationFormats(unittest.TestCase):
    def test_metadata_and_groups(self):
        self.assertEqual(track_table(struct.pack('>H4h',2,20000,1,20001,36)),[(20000,1),(20001,36)])
        self.assertEqual(groups_table(struct.pack('>HH3h',1,3,0,1,2)),[[0,1,2]])
    def test_constants_and_variable_channel(self):
        initial=[1,1,1,0,0,0,1,0,0,0];flags=1023^(1<<7)
        t=track_data(struct.pack('>10fH3f',*initial,flags,-1,0,1),3)
        self.assertEqual(t['channels'][0],[1]);self.assertEqual(t['channels'][7],[-1,0,1]);self.assertEqual(t['count'],3)
    def test_truncated_stream_and_unknown_flags(self):
        for data in (b'',struct.pack('>10fH',*([1]*10),0),struct.pack('>10fH',*([1]*10),2047)):
            with self.assertRaises(ValueError):track_data(data,3)
    def test_exact_channel_consumption(self):
        data=struct.pack('>10fH',*([1]*10),1023)
        self.assertEqual(len(track_data(data,1)['channels']),10)
        with self.assertRaises(ValueError):track_data(data+b'\0',1)
    @unittest.skipUnless(os.environ.get('THEMEPARK_VERIFY_REAL_ASSETS')=='1','Requires local original-derived assets')
    def test_original_tracks_and_code_bindings(self):
        report=json.loads((ROOT/'analysis/animations.json').read_text())
        models=json.loads((ROOT/'analysis/models.json').read_text())['models']
        self.assertEqual(report['track_count'],3363);self.assertEqual(report['group_count'],203)
        self.assertEqual(report['frame_ms'],33)
        for model,group in IDLE_BINDINGS.items():
            self.assertEqual(report['groups'][group]['nodes'],len(models[model]['nodes']))
            self.assertTrue(any(len(report['tracks'][t]['channels'][c])>1 for t in report['groups'][group]['tracks'] for c in range(10)))
if __name__=='__main__':unittest.main()
