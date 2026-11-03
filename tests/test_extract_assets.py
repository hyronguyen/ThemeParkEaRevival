"""Regression tests for archive boundaries, exact decoding and provenance.

Default tests use small synthetic packages only. Set
THEMEPARK_VERIFY_REAL_ASSETS=1 after extraction to verify the real manifest and
unchanged reference archives without re-running extraction.
"""
from __future__ import annotations
import contextlib
import importlib.util
import io
import json
import os
from pathlib import Path
import struct
import sys
import tempfile
import types
import unittest
from unittest import mock
import zipfile
import zlib

PROJECT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('themepark_extract_assets', PROJECT/'tools'/'extract_assets.py')
extract = importlib.util.module_from_spec(spec)
spec.loader.exec_module(extract)

def index_record(resource=0x1234abcd, flag=255, offset=0x123456, pack=42, size=0x10203040):
    return struct.pack('>IB',resource,flag)+offset.to_bytes(3,'big')+bytes([pack])+struct.pack('>I',size)

def texture(w,h,pixels):
    return struct.pack('<HH',w,h)+zlib.compress(pixels)

class BfcDirectoryTests(unittest.TestCase):
    def test_big_endian_field_widths_and_record_order(self):
        data=struct.pack('>H',2)+index_record()+index_record(resource=9,flag=254,offset=0,pack=255,size=1)
        self.assertEqual(extract.parse_bfc_index(data),[
            dict(resource_id='1234abcd',flag=255,offset=0x123456,pack=42,size=0x10203040,index_record=0),
            dict(resource_id='00000009',flag=254,offset=0,pack=255,size=1,index_record=1)])

    def test_empty_directory(self):
        self.assertEqual(extract.parse_bfc_index(b'\0\0'),[])

    def test_rejects_short_header_and_record(self):
        for data in (b'',b'\0',b'\0\1',b'\0\1'+index_record()[:-1]):
            with self.subTest(length=len(data)),self.assertRaises(ValueError):
                extract.parse_bfc_index(data)

    def test_rejects_count_mismatch_and_trailing_data(self):
        for data in (b'\0\0'+index_record(),b'\0\2'+index_record(),b'\0\1'+index_record()+b'!'):
            with self.subTest(length=len(data)),self.assertRaises(ValueError):
                extract.parse_bfc_index(data)

class ArchivePathTests(unittest.TestCase):
    def test_preserves_valid_internal_path(self):
        self.assertEqual(extract.safe_member('Payload/ThemePark.app/0.bfc'),Path('Payload','ThemePark.app','0.bfc'))

    def test_rejects_archive_traversal_absolute_and_windows_paths(self):
        for name in ('../outside','a/../../outside','/absolute','C:/outside','C:outside',r'..\outside',r'\\server\share\file','a/file:stream'):
            with self.subTest(name=name),self.assertRaises(ValueError):
                extract.safe_member(name)

class TextureTests(unittest.TestCase):
    def test_rgba_decoding_preserves_every_channel_byte(self):
        pixels=bytes([255,0,10,128,30,40,50,255,7,8,9,0,100,110,120,130])
        self.assertEqual(extract.decode_texture(texture(2,2,pixels)),(2,2,'RGBA8',pixels))

    def test_packed16_is_retained_as_unresolved(self):
        pixels=b'\x11\x22\x33\x44'
        self.assertEqual(extract.decode_texture(texture(2,1,pixels)),(2,1,'packed16_unresolved',pixels))

    def test_rejects_truncated_header(self):
        for n in range(6):
            with self.subTest(size=n),self.assertRaises(ValueError):
                extract.decode_texture(b'\1'*n)

    def test_rejects_invalid_dimensions_before_decompression(self):
        for w,h in ((0,1),(1,0),(8193,1),(8192,8192)):
            with self.subTest(w=w,h=h),self.assertRaisesRegex(ValueError,'dimensions'):
                extract.decode_texture(texture(w,h,b'\0'*4))

    def test_rejects_truncated_zlib_payload(self):
        data=texture(2,2,bytes(range(16)))
        for missing in (1,2,5):
            with self.subTest(missing=missing),self.assertRaises((ValueError,zlib.error)):
                extract.decode_texture(data[:-missing])

    def test_rejects_trailing_bytes_and_second_zlib_stream(self):
        data=texture(1,1,b'RGBA')
        for suffix in (b'!',b'\0',zlib.compress(b'RGBA')):
            with self.subTest(suffix=suffix),self.assertRaises(ValueError):
                extract.decode_texture(data+suffix)

    def test_rejects_incorrect_pixel_count(self):
        for size in (0,1,3,5,8):
            with self.subTest(size=size),self.assertRaises(ValueError):
                extract.decode_texture(texture(1,1,b'x'*size))

    def test_limits_decompression_of_high_expansion_payload(self):
        # The declared image permits 4 bytes; a valid compressed stream producing
        # 1 MiB must be rejected after bounded decompression.
        with self.assertRaises(ValueError):
            extract.decode_texture(texture(1,1,b'\0'*(1024*1024)))

class SyntheticExtractionTests(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory(prefix='themepark-extraction-test-')
        self.root=Path(self.temp.name)
        (self.root/'resource').mkdir()
        self.addCleanup(self.temp.cleanup)

    def package(self, head, pack=None, extra=None):
        path=self.root/'resource'/'fixture.ipa'
        with zipfile.ZipFile(path,'w',zipfile.ZIP_DEFLATED) as z:
            z.writestr('Payload/ThemePark.app/head_ipad3.bfc',head)
            if pack is not None:z.writestr('Payload/ThemePark.app/0.bfc',pack)
            for name,data in (extra or {}).items():z.writestr(name,data)
        return path

    def run_extractor(self):
        # Exercise raw extraction and manifest generation without a Pillow dependency.
        with mock.patch.object(extract,'ROOT',self.root),mock.patch.dict(sys.modules,{'PIL':None}),contextlib.redirect_stdout(io.StringIO()):
            return extract.run(types.SimpleNamespace())

    def test_deduplication_exact_raw_payload_and_read_only_source(self):
        pixels=bytes(range(16));encoded=texture(2,2,pixels);prefix=b'opaque-prefix'
        record=index_record(resource=108,offset=len(prefix),pack=0,size=len(encoded))
        head=struct.pack('>H',2)+record+record
        path=self.package(head,prefix+encoded,{'Payload/ThemePark.app/ThemePark':b'not executed','notes.txt':b'fixture'})
        before=path.read_bytes();inventory=self.run_extractor()
        self.assertEqual(path.read_bytes(),before)
        source=inventory['inputs'][0]
        self.assertEqual(source['sha256'],source['sha256_after'])
        self.assertTrue(source['unchanged'])
        self.assertEqual(inventory['counts']['unique_bfc_resources'],1)
        item=inventory['bfc_resources'][0]
        self.assertEqual(len(item['references']),2)
        self.assertEqual(item['pixel_format'],'RGBA8')
        self.assertEqual((self.root/item['output']).read_bytes(),encoded)
        self.assertEqual(item['sha256'],extract.sha(encoded))
        code=next(x for x in inventory['archive_members'] if x['path'].endswith('/ThemePark'))
        self.assertEqual(code['status'],'code_or_signing_metadata_not_asset')
        self.assertNotIn('output',code)
        saved=json.loads((self.root/'analysis'/'asset_inventory.json').read_text())
        self.assertEqual(saved,inventory)

    def test_rejects_missing_bfc_container(self):
        self.package(b'\0\1'+index_record(offset=0,pack=0,size=1))
        with self.assertRaisesRegex(ValueError,'Out-of-bounds BFC entry'):
            self.run_extractor()

    def test_rejects_bfc_slice_past_container_end(self):
        self.package(b'\0\1'+index_record(offset=3,pack=0,size=2),b'abcd')
        with self.assertRaisesRegex(ValueError,'Out-of-bounds BFC entry'):
            self.run_extractor()

    def test_accepts_bfc_slice_ending_exactly_at_container_end(self):
        self.package(b'\0\1'+index_record(flag=254,offset=1,pack=0,size=3),b'abcd')
        inventory=self.run_extractor();item=inventory['bfc_resources'][0]
        self.assertEqual((self.root/item['output']).read_bytes(),b'bcd')

    def test_rejects_member_traversal_before_write(self):
        self.package(b'\0\0',extra={'../outside.txt':b'escape'})
        with self.assertRaisesRegex(ValueError,'Unsafe archive path'):
            self.run_extractor()
        self.assertFalse((self.root/'outside.txt').exists())

@unittest.skipUnless(os.environ.get('THEMEPARK_VERIFY_REAL_ASSETS')=='1','Opt-in real asset manifest check')
class RealAssetManifestTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.inventory=json.loads((PROJECT/'analysis'/'asset_inventory.json').read_text(encoding='utf-8'))

    def test_input_hashes_still_match_original_before_and_after_hashes(self):
        for item in self.inventory['inputs']:
            with self.subTest(package=item['path']):
                path=PROJECT/item['path']
                self.assertEqual(path.stat().st_size,item['size'])
                self.assertEqual(extract.file_sha(path),item['sha256'])
                self.assertEqual(item['sha256'],item['sha256_after'])
                self.assertTrue(item['unchanged'])

    def test_manifest_counts_ranges_outputs_and_sample_hashes(self):
        inventory=self.inventory;records=inventory['bfc_resources'];members=inventory['archive_members']
        self.assertEqual(inventory['counts']['unique_bfc_resources'],len(records))
        self.assertEqual(inventory['counts']['archive_members'],len(members))
        sizes={(x['package'],x['path']):x['size'] for x in members}
        seen=set()
        for i,item in enumerate(records):
            location=(item['package'],item['container_path'],item['offset'],item['size'])
            self.assertNotIn(location,seen);seen.add(location)
            self.assertGreaterEqual(item['offset'],0)
            self.assertLessEqual(item['offset']+item['size'],sizes[(item['package'],item['container_path'])])
            output=(PROJECT/item['output']).resolve()
            self.assertTrue(output.is_relative_to((PROJECT/'extracted').resolve()))
            self.assertEqual(output.stat().st_size,item['size'])
            if i%max(1,len(records)//25)==0:self.assertEqual(extract.file_sha(output),item['sha256'])
            if 'preview' in item:self.assertTrue((PROJECT/item['preview']).is_file())

if __name__=='__main__':unittest.main()
