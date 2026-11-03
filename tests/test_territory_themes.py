"""Integration checks for the original assets used by territory customization."""
import json
from pathlib import Path
import unittest
ROOT=Path(__file__).resolve().parents[1]
class TerritoryThemeAssets(unittest.TestCase):
    def test_theme_models_and_materials_are_available(self):
        models={int(line.split()[0]) for line in (ROOT/'extracted/runtime/models.tsv').read_text().splitlines() if line and not line.startswith('#')}
        for line in (ROOT/'extracted/runtime/territory_themes.tsv').read_text().splitlines():
            i,statue,ground,texture,f4,f6,f16,gate,name=line.split()
            for model in (statue,ground,f4,f6,f16,gate):
                if int(model): self.assertIn(int(model),models,name)
            self.assertTrue((ROOT/'extracted'/texture).is_file(),name)
    def test_reserved_theme_plots_do_not_overlap_attraction_plots(self):
        plots=[list(map(int,line.split()[1:])) for line in (ROOT/'extracted/runtime/park_layout.tsv').read_text().splitlines() if line.startswith('S')]
        themes=[list(map(int,line.split())) for line in (ROOT/'extracted/runtime/theme_plots.tsv').read_text().splitlines()]
        self.assertEqual(set(t[0] for t in themes),set(range(20)))
        for territory,x,y,w,h in themes:
            for _,a,b,c,d in plots:
                self.assertFalse(x<a+c and x+w>a and y<b+d and y+h>b, (territory,(a,b,c,d)))
if __name__=='__main__': unittest.main()
