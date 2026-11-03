import importlib.util
import unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
class PassengerPoses(unittest.TestCase):
    def test_original_car_and_kart_resources(self):
        spec=importlib.util.spec_from_file_location('passenger_export',ROOT/'tools/export_passenger_poses.py')
        module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        rows=module.recover()
        self.assertEqual([(r['ride'],r['model']) for r in rows],[(521,504),(521,505),(521,506),(521,507),(503,524),(503,525),(503,526),(503,527)])
        saved=[tuple(map(int,line.split())) for line in (ROOT/'extracted/runtime/passenger_poses.tsv').read_text().splitlines()]
        self.assertEqual(saved,[(r['ride'],r['variant'],r['model']) for r in rows])
