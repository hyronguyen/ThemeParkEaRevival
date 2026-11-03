import hashlib,json,os,pathlib,unittest,zlib
ROOT=pathlib.Path(__file__).resolve().parents[1]
class LocatorAudio(unittest.TestCase):
    def test_locator_crc(self):
        self.assertEqual(zlib.crc32(b'vis_loc04'),0x01a8afd3)
        self.assertEqual(zlib.crc32(b'vis_loc01'),0x71c25b5c)
    @unittest.skipUnless(os.environ.get('THEMEPARK_VERIFY_REAL_ASSETS')=='1','Local assets required')
    def test_original_locator_mapping(self):
        rows=[tuple(map(int,row.split())) for row in (ROOT/'extracted/runtime/model_locators.tsv').read_text().splitlines()]
        self.assertEqual(len(rows),727);self.assertEqual(len(set(rows)),727)
        models=json.loads((ROOT/'analysis/models.json').read_text())['models']
        for model,node,locator in rows:
            self.assertEqual(models[model]['nodes'][node]['name_hash'],f'{zlib.crc32(f"vis_loc{locator:02}".encode()):08x}')
        self.assertEqual([n for m,n,i in rows if m==404],[1,2,3,4])
        self.assertEqual([n for m,n,i in rows if m==425],[1,2,3,4,5])
    @unittest.skipUnless(os.environ.get('THEMEPARK_VERIFY_REAL_ASSETS')=='1','Local assets required')
    def test_music_bytes_unchanged(self):
        records=json.loads((ROOT/'extracted/runtime/audio/provenance.json').read_text())
        self.assertEqual(len(records),3)
        for r in records:
            data=(ROOT/r['output']).read_bytes()
            self.assertEqual(len(data),r['size']);self.assertEqual(hashlib.sha256(data).hexdigest(),r['sha256'])
if __name__=='__main__':unittest.main()
