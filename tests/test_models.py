import math,os,pathlib,struct,sys,unittest,json
ROOT=pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from extract_models import mesh_table,mesh_links,model_data,image_table,packed_pixels,vertices

class ModelFormats(unittest.TestCase):
    def test_original_byte_order_and_signed_links(self):
        self.assertEqual(mesh_links(struct.pack('>HHH2h',1,1,2,7,-1)),[[[7,-1]]])
        m=mesh_table(struct.pack('>Hhh6f',1,10001,5,-1,-2,-3,1,2,3))[0]
        self.assertEqual(m['resource'],10001);self.assertEqual(m['bounds'],[-1,-2,-3,1,2,3])
    def test_hierarchy_parent_rejected(self):
        for parent in (-2,0,2):
            with self.subTest(parent=parent),self.assertRaises(ValueError):model_data(struct.pack('>HHbbIh10f',0,1,parent,1,0,0,*([1]*10)))
    def test_finite_transforms(self):
        with self.assertRaises(ValueError):model_data(struct.pack('>HHbbIh10f',0,1,-1,1,0,0,*([math.nan]*10)))
    def test_triangle_size_and_floats(self):
        self.assertEqual(len(vertices(struct.pack('>15f',*range(15)),5)),3)
        for data in (b'\0'*40,struct.pack('>15f',*([math.inf]*15))):
            with self.assertRaises(ValueError):vertices(data,5)
    def test_image_indirection(self):
        images,textures=image_table(struct.pack('>HH8hBii',1,1,0,128,128,-256,0,0,0,0,11,104,105))
        self.assertEqual(textures[images[0][0]],(11,104,105))
    def test_rgb565_and_rgba5551_little_endian(self):
        self.assertEqual(packed_pixels(struct.pack('<4H',0xf800,0x07e0,0x001f,0xffff),11),bytes([255,0,0,255,0,255,0,255,0,0,255,255,255,255,255,255]))
        self.assertEqual(packed_pixels(struct.pack('<3H',0xf801,0x07c1,0x003e),13),bytes([255,0,0,255,0,255,0,255,0,0,255,0]))
    def test_exact_resource_consumption(self):
        for fn in (mesh_table,mesh_links,image_table,model_data):
            for bad in (b'',b'\0',b'\0\0\0\0trailing'):
                with self.subTest(fn=fn.__name__,bad=bad),self.assertRaises(ValueError):fn(bad)
    @unittest.skipUnless(os.environ.get('THEMEPARK_VERIFY_REAL_ASSETS')=='1','Requires local original-derived assets')
    def test_all_original_models_complete(self):
        report=json.loads((ROOT/'analysis/models.json').read_text())
        self.assertEqual(report['model_count'],730);self.assertEqual(report['exported'],730)
        for m in report['models']:
            self.assertFalse(m['missing_textures']);self.assertTrue((ROOT/m['output']).is_file())
        # Parent, material and image relations are exported, not guessed from numeric resource IDs.
        self.assertEqual(report['models'][259]['parts'][0]['texture_id'],104)
if __name__=='__main__':unittest.main()
