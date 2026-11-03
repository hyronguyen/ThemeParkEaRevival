"""Read-only ZIP/BFC extraction with provenance and bounded texture decoding.

Run with Python + Pillow for PNG previews; without Pillow raw extraction still works.
No network calls. Does not decrypt executable code or infer unknown asset formats.
"""
from __future__ import annotations
import argparse
import collections
import hashlib
import io
import json
from pathlib import Path, PurePosixPath
import re
import struct
import zipfile
import zlib

ROOT = Path(__file__).resolve().parents[1]
CATEGORIES = ('textures','models','animations','audio','UI','fonts','configuration','level_data','unknown')
MAX_TEXTURE_BYTES = 64 * 1024 * 1024

def sha(data): return hashlib.sha256(data).hexdigest()

def file_sha(path):
    h = hashlib.sha256()
    with path.open('rb') as f:
        for b in iter(lambda: f.read(1024 * 1024), b''): h.update(b)
    return h.hexdigest()

def safe_member(name):
    p = PurePosixPath(name)
    if p.is_absolute() or '..' in p.parts or '\\' in name or ':' in name:
        raise ValueError(f'Unsafe archive path: {name}')
    return Path(*p.parts)

def parse_bfc_index(data):
    if len(data) < 2: raise ValueError('Truncated BFC directory')
    count = struct.unpack_from('>H', data)[0]
    if len(data) != 2 + count * 13: raise ValueError('BFC directory size/count mismatch')
    result=[]
    for i in range(count):
        r=data[2+i*13:15+i*13]
        result.append(dict(resource_id=f'{int.from_bytes(r[:4], "big"):08x}',
            flag=r[4], offset=int.from_bytes(r[5:8], 'big'), pack=r[8], size=int.from_bytes(r[9:], 'big'), index_record=i))
    return result

def decode_texture(data):
    if len(data)<6: raise ValueError('Texture header truncated')
    w,h=struct.unpack_from('<HH',data)
    if not (0<w<=8192 and 0<h<=8192 and w*h*4<=MAX_TEXTURE_BYTES): raise ValueError('Texture dimensions invalid')
    dec=zlib.decompressobj()
    raw=dec.decompress(data[4:], w*h*4+1)
    if not dec.eof or dec.unused_data or dec.unconsumed_tail: raise ValueError('Texture zlib stream not exact/bounded')
    if len(raw)==w*h*4: return w,h,'RGBA8',raw
    if len(raw)==w*h*2: return w,h,'packed16_unresolved',raw
    raise ValueError(f'Unexpected pixel byte count {len(raw)} for {w}x{h}')

def detect(data, name):
    if data.startswith(b'\x89PNG\r\n\x1a\n'): return 'textures','PNG_CgBI' if b'CgBI' in data[:48] else 'PNG'
    if data.startswith(b'\xff\xd8\xff'): return 'textures','JPEG'
    if data.startswith(b'OggS'): return 'audio','Ogg'
    if data.startswith(b'RIFF') and data[8:12]==b'WAVE': return 'audio','WAVE'
    if data.startswith(b'caff'): return 'audio','CAF'
    if data.startswith(b'ID3') or (len(data)>2 and data[0]==255 and data[1]&0xe0==0xe0 and str(name).endswith('.mp3')): return 'audio','MP3'
    if data.startswith(b'OTTO'): return 'fonts','OpenType_CFF'
    if data.startswith(b'\x00\x01\x00\x00') and str(name).endswith('.ttf'): return 'fonts','TrueType'
    if data.startswith(b'SQLite format 3\x00'): return 'configuration','SQLite'
    if data.startswith(b'bplist00'): return 'configuration','binary_plist'
    if data.lstrip().startswith((b'<?xml',b'<plist',b'<resources',b'<config')): return 'configuration','XML'
    if data.startswith(b'DBPF'): return 'unknown','DBPF'
    if data.startswith(b'\x26\x02\x37\x13'): return 'unknown','PM_audio_PAC'
    if str(name).endswith(('.cfg','.properties','.strings','.css','.html','.txt')):
        try: data.decode('utf-8'); return 'configuration','UTF8_text'
        except UnicodeDecodeError: pass
    return 'unknown','unidentified'

def write_bytes(path,data):
    path.parent.mkdir(parents=True,exist_ok=True)
    path.write_bytes(data)

def run(args):
    try:
        from PIL import Image, ImageDraw
    except ImportError:
        Image=ImageDraw=None
    out=ROOT/'extracted'
    for cat in CATEGORIES: (out/cat).mkdir(parents=True,exist_ok=True)
    packages=sorted({*list((ROOT/'resource').glob('*.ipa')),*list((ROOT/'resource').glob('*.apk')),*list((ROOT/'input').glob('*.ipa')),*list((ROOT/'input').glob('*.apk'))})
    if not packages: raise SystemExit('No packages in resource/ or input/')
    inventory=dict(schema_version=1, inputs=[], archive_members=[], bfc_resources=[], counts={}, limitations=[])
    catalog=[]
    for package in packages:
        label=package.stem
        before=file_sha(package)
        source=dict(path=package.relative_to(ROOT).as_posix(),sha256=before,size=package.stat().st_size)
        inventory['inputs'].append(source)
        print(f'Inspect/extract {package.name}',flush=True)
        with zipfile.ZipFile(package) as z:
            members={m.filename:m for m in z.infolist() if not m.is_dir()}
            for name,m in members.items():
                rel=safe_member(name)
                entry=dict(package=source['path'],path=name,size=m.file_size,zip_crc32=f'{m.CRC:08x}')
                # Code/signing artifacts are inventoried, not copied into the asset tree.
                if name.startswith('META-INF/') or '/_CodeSignature/' in name or '/SC_Info/' in name or name.endswith(('.dex','.so','.sinf','.supp','.supf')) or name.endswith('/ThemePark'):
                    entry['status']='code_or_signing_metadata_not_asset'
                else:
                    data=z.read(name)
                    cat,fmt=detect(data,name)
                    target=out/cat/label/'members'/rel
                    write_bytes(target,data)
                    entry.update(category=cat,format=fmt,sha256=sha(data),output=target.relative_to(ROOT).as_posix(),status='extracted_raw')
                inventory['archive_members'].append(entry)
            heads=[n for n in members if re.search(r'/head_[^/]+\.bfc$',n)]
            references=collections.defaultdict(list)
            for head in heads:
                for r in parse_bfc_index(z.read(head)):
                    pack_name=str(PurePosixPath(head).parent/f'{r["pack"]}.bfc')
                    if pack_name not in members or r['offset']+r['size']>members[pack_name].file_size:
                        raise ValueError(f'Out-of-bounds BFC entry: {head} {r}')
                    references[(pack_name,r['offset'],r['size'])].append(dict(index_path=head,**r))
            groups=collections.defaultdict(list)
            for loc,refs in references.items(): groups[loc[0]].append((loc,refs))
            for pack_name,records in groups.items():
                pack=z.read(pack_name)
                for (_,offset,size),refs in records:
                    data=pack[offset:offset+size]
                    cat,fmt=detect(data,'')
                    record=dict(package=source['path'],container_path=pack_name,offset=offset,size=size,sha256=sha(data),references=refs)
                    pixels=None
                    if all(r['flag']==255 for r in refs):
                        try:
                            w,h,pix,raw=decode_texture(data)
                            cat,fmt='textures','PM_dimension_zlib_texture'
                            record.update(width=w,height=h,pixel_format=pix,decoded_bytes=len(raw))
                            if pix=='RGBA8': pixels=raw
                        except (ValueError,zlib.error) as e: record['decode_note']=str(e)
                    stem=f'{refs[0]["resource_id"]}_b{refs[0]["pack"]:03d}_{offset:06x}'
                    raw_path=out/cat/label/'bfc'/f'{stem}.bin'
                    write_bytes(raw_path,data)
                    record.update(category=cat,format=fmt,output=raw_path.relative_to(ROOT).as_posix())
                    if pixels is not None and Image is not None:
                        png=raw_path.with_suffix('.png')
                        Image.frombytes('RGBA',(w,h),pixels).save(png,compress_level=1)
                        record['preview']=png.relative_to(ROOT).as_posix()
                        preferred=next((r for r in refs if 'head_ipad3.' in r['index_path']),None)
                        if preferred:
                            catalog.append((preferred['resource_id'],png.relative_to(out).as_posix()))
                    inventory['bfc_resources'].append(record)
            source['bfc_index_count']=len(heads)
            source['bfc_unique_ranges']=len(references)
        after=file_sha(package)
        source['sha256_after']=after
        source['unchanged']=before==after
        if before!=after: raise RuntimeError('Input changed during read-only extraction')
    counts=collections.Counter(r['format'] for r in inventory['bfc_resources'])
    inventory['counts']=dict(archive_members=len(inventory['archive_members']),unique_bfc_resources=len(inventory['bfc_resources']),bfc_formats=dict(counts),runtime_catalog=len(catalog))
    inventory['limitations']=[
        'BFC directory contains numeric identifiers, not original filenames; no names are invented.',
        'BFC flags 0xFE and 0xFF are preserved; only validated texture layouts receive texture labels.',
        'Packed16 texture channel order is unresolved; only RGBA8 textures receive PNG previews.',
        'Models/animations/level data may be among unidentified records; not decoded merely by guessed extensions.',
        'CgBI originals remain byte-exact and are not converted by this tool.',
        'Extensionless wv audio is retained as unknown until PAC rate/channel/encoding semantics are verified.',
        'Runtime catalog uses decoded iPad3 directory resources; not an original park scene.']
    # Put a verified original castle sprite first; retain actual resource identity.
    catalog.sort(key=lambda x:(x[0]!='0000006c',x[0],x[1]))
    runtime=out/'runtime';runtime.mkdir(exist_ok=True)
    (runtime/'catalog.tsv').write_text('# ThemePark original iPad3 RGBA8 resources; paths relative to extracted/\n'+''.join(f'{key}\t{path}\n' for key,path in catalog),encoding='utf-8')
    analysis=ROOT/'analysis';analysis.mkdir(exist_ok=True)
    (analysis/'asset_inventory.json').write_text(json.dumps(inventory,indent=2),encoding='utf-8')
    if Image is not None:
        preview=analysis/'previews';preview.mkdir(exist_ok=True)
        sheet=Image.new('RGB',(1200,900),(40,47,54));draw=ImageDraw.Draw(sheet)
        # Sample across the catalog, not just the first texture atlas.
        chosen=catalog[:40]+catalog[40::max(1,(len(catalog)-40)//20)][:20]
        for i,(key,path) in enumerate(chosen):
            im=Image.open(out/path).convert('RGBA');im.thumbnail((112,122))
            x=i%10*120;y=i//10*150;sheet.paste(im,(x,y),im);draw.text((x+3,y+128),key,fill='white')
        sheet.save(preview/'original_texture_contact_sheet.jpg')
    print(json.dumps(inventory['counts'],indent=2),flush=True)
    return inventory

if __name__=='__main__':
    run(argparse.ArgumentParser(description=__doc__).parse_args())
