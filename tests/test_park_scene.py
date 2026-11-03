import json, os, pathlib, sys, unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from export_park_scene import assemble,center,path_mask,PATH_MODELS,MASK_TO_TYPE

class ParkScene(unittest.TestCase):
    def test_half_cell_and_negative_y(self):
        self.assertEqual(center(72,115,2,6),(2336,-3776))
        self.assertEqual(center(1,2,1,3),(48,-112))
        with self.assertRaises(ValueError):center(1,2,0,3)

    def test_all_neighbor_masks(self):
        neighbors=[(-1,0,1),(0,-1,2),(1,0,4),(0,1,8)]
        expected=[306,311,309,316,312,308,315,317,310,314,307,320,313,319,318,321]
        for mask in range(16):
            points={(10+dx,20+dy) for dx,dy,bit in neighbors if mask&bit}
            self.assertEqual(path_mask(10,20,points),mask)
            self.assertEqual(PATH_MODELS[MASK_TO_TYPE[mask]],expected[mask])

    def test_layers_duplicates_and_material_override(self):
        tables={'guidelines':[[1,1,36,36,222]],'dividerWalls':[[7,292,72,115,2,6,90]],
                'paths':[[1,1],[2,1],[1,1]]}
        scene=assemble(tables,set(range(730)))
        self.assertEqual(len(scene),4)
        self.assertEqual([i['z'] for i in scene],[-5,0,-1,-1])
        self.assertEqual(scene[1]['angle'],90)
        self.assertEqual(scene[2]['source_rows'],[0,2])
        self.assertEqual(scene[2]['texture'],'models/textures/00000012.png')
        with self.assertRaises(ValueError):assemble(tables,{392})
        tables['guidelines'][0][2]=35
        with self.assertRaises(ValueError):assemble(tables,set(range(730)))

    @unittest.skipUnless(os.environ.get('THEMEPARK_VERIFY_REAL_ASSETS')=='1','Local extracted assets required')
    def test_original_scene(self):
        r=json.loads((ROOT/'analysis/park_scene.json').read_text())
        self.assertEqual(r['counts'],{'G':20,'W':329,'P':5585})
        self.assertEqual(r['duplicate_path_rows'],19)
        self.assertEqual(len(r['instances']),5934)
        rows=[n for i in r['instances'] if i['kind']=='P' for n in i['source_rows']]
        self.assertEqual(sorted(rows),list(range(5604)))

if __name__=='__main__':unittest.main()
