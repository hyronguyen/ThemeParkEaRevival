"""Read-only IPA/Mach-O inventory; no decryption or execution is performed.

Python standard library only. Reports addresses as unslid Mach-O virtual addresses;
file offsets are separate. Supported executable here: little-endian 32-bit Mach-O.
"""
from __future__ import annotations
import argparse
import collections
import hashlib
import json
from pathlib import Path
import plistlib
import re
import struct
import zipfile
import zlib


def sha256(path):
    h = hashlib.sha256()
    with path.open('rb') as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b''):
            h.update(chunk)
    return h.hexdigest()


def inspect_macho(data):
    if len(data) < 28 or data[:4] != b'\xce\xfa\xed\xfe':
        raise ValueError('Expected little-endian 32-bit Mach-O, not a fat/64-bit image')
    magic, cpu, subtype, filetype, ncmds, sizeofcmds, flags = struct.unpack_from('<7I', data)
    out = dict(magic=hex(magic), cpu_type=cpu, cpu_subtype=subtype, file_type=filetype,
               ncmds=ncmds, sizeofcmds=sizeofcmds, flags=hex(flags), dylibs=[], segments=[],
               sections=[], encryption=[], load_commands=[])
    cursor = 28
    symtab = None
    for _ in range(ncmds):
        cmd, size = struct.unpack_from('<II', data, cursor)
        if size < 8 or cursor + size > 28 + sizeofcmds:
            raise ValueError('Invalid load command bounds')
        out['load_commands'].append(dict(command=hex(cmd), offset=cursor, size=size))
        if cmd in (0xc, 0x80000018, 0x8000001f, 0x80000023):
            name_off, stamp, cur, compat = struct.unpack_from('<4I', data, cursor + 8)
            name = data[cursor + name_off:cursor + size].split(b'\0', 1)[0].decode()
            out['dylibs'].append(dict(name=name, command_offset=cursor, current_version=cur,
                                     compatibility_version=compat))
        elif cmd == 0x21:
            off, length, cryptid = struct.unpack_from('<3I', data, cursor + 8)
            out['encryption'].append(dict(command_offset=cursor, cryptoff=off,
                                          cryptsize=length, cryptid=cryptid))
        elif cmd == 2:
            off, nsyms, stroff, strsize = struct.unpack_from('<4I', data, cursor + 8)
            symtab = dict(offset=off, nsyms=nsyms, string_offset=stroff, string_size=strsize)
            out['symbol_table'] = symtab
        elif cmd == 1:
            vals = struct.unpack_from('<16s8I', data, cursor + 8)
            segname = vals[0].split(b'\0')[0].decode()
            segment = dict(name=segname, vmaddr=vals[1], vmsize=vals[2],
                           fileoff=vals[3], filesize=vals[4], nsects=vals[7])
            out['segments'].append(segment)
            for n in range(vals[7]):
                fields = struct.unpack_from('<16s16s9I', data, cursor + 56 + n * 68)
                out['sections'].append(dict(index=len(out['sections']) + 1,
                    section=fields[0].split(b'\0')[0].decode(),
                    segment=fields[1].split(b'\0')[0].decode(), address=fields[2], size=fields[3],
                    offset=fields[4], flags=hex(fields[8])))
        elif cmd == 0x25:
            version, sdk = struct.unpack_from('<II', data, cursor + 8)
            def ver(x): return '.'.join(str(i) for i in (x >> 16, (x >> 8) & 255, x & 255))
            out['minimum_ios'] = ver(version)
            out['sdk'] = ver(sdk)
        cursor += size
    symbols = []
    if symtab:
        assert symtab['offset'] + symtab['nsyms'] * 12 <= len(data)
        table = data[symtab['string_offset']:symtab['string_offset'] + symtab['string_size']]
        for index in range(symtab['nsyms']):
            strx, typ, sect, desc, value = struct.unpack_from('<IBBHI', data, symtab['offset'] + index * 12)
            name = table[strx:table.find(b'\0', strx)].decode('utf-8', 'replace') if strx < len(table) else ''
            stab = bool(typ & 0xe0)
            fileoff = None
            for seg in out['segments']:
                if seg['vmaddr'] <= value < seg['vmaddr'] + seg['filesize']:
                    fileoff = value - seg['vmaddr'] + seg['fileoff']
                    break
            symbols.append(dict(index=index, name=name, type=typ, section=sect, desc=desc,
                                value=value, file_offset=fileoff, is_debug=stab,
                                name_offset=symtab['string_offset'] + strx))
    return out, symbols


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('ipa', nargs='?', default='resource/Theme_Park_1.8.3_ios_5.1.1.ipa')
    parser.add_argument('--output', default='analysis')
    args = parser.parse_args()
    source = Path(args.ipa)
    output = Path(args.output)
    detail = output / 'ios'
    detail.mkdir(parents=True, exist_ok=True)
    initial_hash = sha256(source)
    with zipfile.ZipFile(source) as archive:
        plists = [n for n in archive.namelist() if n.endswith('.app/Info.plist')]
        if len(plists) != 1:
            raise ValueError('Expected exactly one application Info.plist')
        plist_path = plists[0]
        plist = plistlib.loads(archive.read(plist_path))
        prefix = plist_path.rsplit('/', 1)[0] + '/'
        executable = prefix + plist['CFBundleExecutable']
        data = archive.read(executable)
        macho, symbols = inspect_macho(data)
        members = [dict(path=x.filename, size=x.file_size, compressed_size=x.compress_size,
                        zip_compression=x.compress_type, crc32=f'{x.CRC:08x}')
                   for x in archive.infolist()]
        strings = [dict(offset=m.start(), value=m.group().decode('ascii'))
                   for m in re.finditer(rb'[ -~]{5,}', data)]
        ustrings = []
        for section in macho['sections']:
            if section['section'] == '__ustring':
                text = data[section['offset']:section['offset'] + section['size']].decode('utf-16-le')
                ustrings.extend(dict(offset=section['offset'] + m.start() * 2, value=m.group())
                                for m in re.finditer('[^\x00]+', text))
        evidence = dict(source=str(source), sha256=initial_hash, size=source.stat().st_size,
                        entry_count=len(members), info_plist_path=plist_path, info_plist=plist,
                        executable=executable, executable_sha256=hashlib.sha256(data).hexdigest(),
                        executable_size=len(data), macho=macho,
                        extension_counts=dict(collections.Counter(Path(x['path']).suffix.lower() or '(none)' for x in members)),
                        symbol_counts=dict(total=len(symbols), debug=sum(s['is_debug'] for s in symbols),
                                           non_debug=sum(not s['is_debug'] for s in symbols)),
                        engine_evidence=[s for s in strings if re.search(r'(rflib_ipad|progressivemedia|FFEngine3D|PMFile7initBFC)', s['value'])][:60],
                        source_files=sorted(set(s['value'] for s in strings if re.fullmatch(r'[A-Za-z0-9_.-]+\.(cpp|mm|m)', s['value']))),
                        network_hosts=sorted(set(re.findall(r'https?://([A-Za-z0-9.:-]+)', data.decode('latin1')))))
        (detail / 'members.json').write_text(json.dumps(members, indent=2), encoding='utf-8')
        (detail / 'symbols.json').write_text(json.dumps(symbols, indent=2), encoding='utf-8')
        with (detail / 'symbols.tsv').open('w', encoding='utf-8') as f:
            f.write('index\ttype\tsection\tdesc\tunslid_va\tfile_offset\tis_debug\tname\n')
            for s in symbols:
                f.write(f"{s['index']}\t{s['type']:02x}\t{s['section']}\t{s['desc']:04x}\t{s['value']:08x}\t{'' if s['file_offset'] is None else format(s['file_offset'],'08x')}\t{s['is_debug']}\t{s['name']}\n")
        (detail / 'strings.json').write_text(json.dumps(strings, indent=2), encoding='utf-8')
        (detail / 'ustrings.json').write_text(json.dumps(ustrings, indent=2), encoding='utf-8')
        header_name = prefix + 'head_ipad3.bfc'
        if header_name in archive.namelist():
            header = archive.read(header_name)
            count = int.from_bytes(header[:2], 'big')
            if len(header) != 2 + count * 13:
                raise ValueError('Unexpected BFC header size')
            records = {}
            for pos in range(2, len(header), 13):
                resource_id = int.from_bytes(header[pos:pos + 4], 'big')
                records[resource_id] = dict(bfc_offset=int.from_bytes(header[pos + 5:pos + 8], 'big'),
                                           bfc_number=header[pos + 8],
                                           size=int.from_bytes(header[pos + 9:pos + 13], 'big'))
            named = []
            for item in ustrings:
                resource_id = zlib.crc32(bytes(ord(c) & 255 for c in item['value']))
                if resource_id in records:
                    named.append(dict(name=item['value'], name_string_file_offset=item['offset'],
                                      resource_id=resource_id, **records[resource_id]))
            (detail / 'named_resources.json').write_text(json.dumps(named, indent=2), encoding='utf-8')
    evidence['hash_after'] = sha256(source)
    assert initial_hash == evidence['hash_after'], 'Original input unexpectedly changed'
    (output / 'ipa_evidence.json').write_text(json.dumps(evidence, indent=2), encoding='utf-8')
    print(json.dumps(dict(source=str(source), sha256=initial_hash, entries=evidence['entry_count'],
                          symbols=evidence['symbol_counts'], encryption=macho['encryption']), indent=2))


if __name__ == '__main__':
    main()
