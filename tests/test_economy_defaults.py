import importlib.util
import re
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

class EconomyDefaults(unittest.TestCase):
    def test_compiled_thresholds_match_original_binary(self):
        spec = importlib.util.spec_from_file_location('recover', ROOT/'tools/recover_economy_defaults.py')
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        values, _ = module.recover()
        header = (ROOT/'src/original_economy.h').read_text().split('{{')[1].split('}}')[0]
        self.assertEqual(list(map(int, re.findall(r'\d+', header))), values)
        self.assertEqual(values[:8], [400,400,600,500,200,520,850,1320])

    def test_scaffold_parts_meet_at_storey_boundaries(self):
        for base in (638,641,644):
            vertices = [list(map(float,line.split())) for line in (ROOT/f'extracted/models/ios/model_{base+2:04d}.tpm').read_text().splitlines() if len(line.split()) == 5]
            self.assertEqual(min(v[2] for v in vertices), 0)
            self.assertEqual(max(v[2] for v in vertices), 32)
