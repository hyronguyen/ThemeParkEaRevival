"""Bounded, static Thumb disassembly of named iOS functions.

Requires Capstone (python -m pip install --target tools/_deps capstone==5.0.9).
Run inspect_ipa.py first. Never executes the game or modifies the IPA.
Function extents are bounded by the next distinct non-debug text symbol; literal
pools and jump-table data may be decoded as instructions near function tails.
Direct-call annotations are symbol matches, not a reconstructed complete call graph.
"""
import bisect
import json
from pathlib import Path
import sys
import zipfile

sys.path.insert(0, str(Path(__file__).parent / '_deps'))
from capstone import Cs, CS_ARCH_ARM, CS_MODE_THUMB

PATTERNS = [
    '6PMFile7calcCRC', '6PMFile7initBFC', '6PMFile8openFileEi',
    '6PMFile17readUnsignedShort', '6PMFile7readInt', '6PMFile16fileToFloatArray',
    '14PMImageManager13loadImageData', '14PMImageManager4loadEi',
    '6PMMesh13loadFromFiles', '6PMMesh8setVerts', '13PMMeshManager4init',
    '4Main11stateUpdate', '4Main12tryGameStart',
    '11StateInGame6update', '11StateInGame10updateGame', '11StateInGame6render',
    '11StateInGame17updateGameObjects', '11StateInGame9loadBegin',
    '7MapData9loadLevel', '7Control8loadData', '7Control7process',
    '13PlaceableItem12loadInstance', '12Serializable12loadInstance',
    '16AttractionStruct14getUpgradeCost', '10RideStruct7getType',
    '10RideStruct12getFlagShift', '5Guest6update', '5Guest18calculateNewTarget',
    '5AStar8huristic', '5AStar8FindPath', '5AStar12ValidateNode',
    '12LoadWorldJob3run', '8LoadData7fromXml', '8LoadData17parseGameSettings',
    '13WorldInstance7fromXml', '5Point7fromXml',
    '16PMProfileStorage4save', '16PMProfileStorage4load',
    '13ThemeParkSoap9LoadWorld', '13ThemeParkSoap10ModifyCash',
    '13ThemeParkSoap8ModifyXP', '13ThemeParkSoap10StartBuild',
    '13ThemeParkSoap13MoveAttraction',
    '5RFLib4tick', '7GameMap7InitMap', '7GameMap4init',
    '7GameMap13loadRideSlots', '10FFEngine3D6render',
    '10FFEngine3D6update', '12FFSceneNode',
    '8RideSlot4read', '8RideSlot15getSlotCenter3D', '9PMTextureC2',
]


def main():
    symbols = json.loads(Path('analysis/ios/symbols.json').read_text(encoding='utf-8'))
    symbols = [s for s in symbols if not s['is_debug'] and s['section'] == 1]
    by_address = {}
    for s in symbols:
        by_address.setdefault(s['value'], []).append(s['name'])
    addresses = sorted(by_address)
    with zipfile.ZipFile('resource/Theme_Park_1.8.3_ios_5.1.1.ipa') as archive:
        binary = archive.read('Payload/ThemePark.app/ThemePark')
    engine = Cs(CS_ARCH_ARM, CS_MODE_THUMB)
    result = []
    for pattern in PATTERNS:
        for s in (x for x in symbols if pattern in x['name']):
            if not s['desc'] & 8:
                raise ValueError('Target is not marked Thumb: ' + s['name'])
            end = addresses[bisect.bisect_right(addresses, s['value'])]
            instructions = []
            calls = []
            for ins in engine.disasm(binary[s['file_offset']:s['file_offset'] + end - s['value']], s['value']):
                item = dict(address=ins.address, bytes=bytes(ins.bytes).hex(), mnemonic=ins.mnemonic, operands=ins.op_str)
                if ins.mnemonic in ('bl', 'blx', 'b.w') and ins.op_str.startswith('#'):
                    target = int(ins.op_str[1:], 0)
                    item['target'] = target
                    item['symbols'] = by_address.get(target, [])
                    if item['symbols']:
                        calls.append(dict(address=ins.address, target=target, symbols=item['symbols']))
                instructions.append(item)
            result.append(dict(name=s['name'], address=s['value'], file_offset=s['file_offset'],
                               extent_upper_bound=end - s['value'], instructions=instructions, calls=calls))
    target = Path('analysis/ios/disassembly.json')
    target.write_text(json.dumps(result, indent=2), encoding='utf-8')
    with target.with_suffix('.txt').open('w', encoding='utf-8') as f:
        for function in result:
            f.write(f"\n{function['name']}\nVA 0x{function['address']:08x}; file offset 0x{function['file_offset']:08x}; next-symbol bound {function['extent_upper_bound']}\n")
            for ins in function['instructions']:
                f.write(f"0x{ins['address']:08x}  {ins['bytes']:12} {ins['mnemonic']:10} {ins['operands']}" +
                        (' ; ' + ' | '.join(ins['symbols']) if ins.get('symbols') else '') + '\n')
    print(f'Wrote {len(result)} function listings to {target}')


if __name__ == '__main__':
    main()
