import json
import unittest
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]

class ExpansionAssets(unittest.TestCase):
    def test_catalog_preserves_existing_save_indices(self):
        rows=json.loads((ROOT/'analysis/attraction_catalog.json').read_text())
        self.assertEqual([r['model'] for r in rows[:5]],[425,427,515,522,404])
        self.assertEqual(len(rows),len({r['model'] for r in rows}))
        self.assertEqual({r['category'] for r in rows},{'RIDES','SHOPS'})
        models={r['model_id'] for r in json.loads((ROOT/'analysis/models.json').read_text())['models']}
        ui={int(line.split()[0]) for line in (ROOT/'extracted/runtime/ui_assets.tsv').read_text().splitlines()}
        for row in rows:
            self.assertIn(row['model'],models)
            if row['thumbnail']:
                self.assertIn(row['thumbnail'],ui)

    def test_decoration_rows_are_original(self):
        source=json.loads((ROOT/'extracted/level_data/Theme_Park_1.8.3_ios_5.1.1/named/decorations.json').read_text())['rows']
        rows=[list(map(int,line.split())) for line in (ROOT/'extracted/runtime/decorations.tsv').read_text().splitlines()]
        self.assertEqual(rows,source)
        self.assertEqual(len(rows),328)

    def test_ui_regions_are_inside_original_textures(self):
        assets={int(line.split()[0]):line.split()[1] for line in (ROOT/'extracted/runtime/ui_assets.tsv').read_text().splitlines()}
        bounds={}
        for line in (ROOT/'extracted/runtime/ui_bounds.tsv').read_text().splitlines():
            key,*values=line.split();u0,v0,u1,v1=map(float,values)
            self.assertTrue(0<=u0<u1<=1 and 0<=v0<v1<=1)
            bounds[int(key)]=values
        self.assertEqual(set(bounds),set(assets))
        for path in assets.values():
            self.assertTrue((ROOT/'extracted'/path).is_file())

if __name__=='__main__':unittest.main()
