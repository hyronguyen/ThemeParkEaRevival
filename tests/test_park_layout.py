import importlib.util,pathlib,struct,sys,unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from export_park_layout import decode_table
class LayoutTests(unittest.TestCase):
    def test_big_endian_signed_rows(self):
        self.assertEqual(decode_table(struct.pack('>5i',2,7,-3,20,10),2),[[7,-3],[20,10]])
    def test_truncated_and_wrong_stride_rejected(self):
        for raw in (b'',b'\0\0\0',struct.pack('>2i',1,3),struct.pack('>3i',-1,0,0)):
            with self.subTest(raw=raw),self.assertRaises(ValueError):decode_table(raw,2)
    def test_zero_count(self):self.assertEqual(decode_table(struct.pack('>i',0),10),[])
if __name__=='__main__':unittest.main()
