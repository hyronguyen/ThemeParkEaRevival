"""Create a concise function index from actually collected symbols/disassembly."""
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
rows=json.loads((ROOT/'analysis/ios/disassembly.json').read_text())
out=['# Binary function map','',
     'Addresses below are unslid iOS ARMv7 virtual addresses and file offsets inside `Payload/ThemePark.app/ThemePark`, not offsets in the compressed IPA. Raw mangled names are preserved exactly. Functions were selected by non-debug text symbols with the Thumb flag; listing bounds use the next distinct text symbol and are not guaranteed semantic function lengths.',
     '', 'The collector labels direct branch targets only when they match known symbols. It does not resolve all virtual/indirect calls, reconstruct full control flow or separate every literal pool. No listing is presented as portable recovered source.', '',
     '| Original symbol | Unslid VA | Executable file offset |', '|---|---:|---:|']
for r in rows:out.append(f'| `{r["name"]}` | `0x{r["address"]:08x}` | `0x{r["file_offset"]:08x}` |')
out += ['', '## Conclusions supported by inspected instructions', '',
    '- PMFile::calcCRC uses low bytes of code units, a CRC lookup table, initial all-ones state and final complement. Its table matches standard CRC32. Named resource matches are independently checked against index ranges and parsed records.',
    '- PMImageManager::loadImageData reads and byte-swaps dimensions, allocates pixel storage and invokes zlib uncompress. Dimension/length/stream validation and visual samples corroborate RGBA8 extraction.',
    '- PMMesh::loadFromFiles reads float and optional short arrays then invokes setVerts. This does not yet establish the complete geometry/material/animation format.',
    '- GameMap::loadRideSlots loads named layout arrays. At VA 0x95842 onward, coordinate pairs are passed to Path::AddPath (call site 0x9585e) and MiniMap::addStaticPath (0x95876).',
    '- RideSlot::read maps ten integer values into its instance; getSlotCenter3D corroborates position/width/height fields and the original 32-unit tile scale. See data_structures.md.',
    '- StateInGame::update directly calls Main::stateUpdate, CalculateTimeslice, NetworkTaskManager::update and other components; updateGame calls camera, quest and game input components. Exact branching/timing remains incomplete.',
    '- ThemeParkSoap::LoadWorld/ModifyCash/ModifyXP/StartBuild construct corresponding asynchronous jobs. Their presence does not prove every action always requires a live server or establish complete fallback rules.',
    '', '## Android references', '',
    'The APK report maps actual DEX code_item offsets for renderer/touch callbacks, park initialization, building, upgrades, quests, entity/config parsing and backend operations. These offsets are not interchangeable with native VAs. Full DEX method/instruction-reference indexes are in analysis/android/.', '',
    'Reproduce with `python tools/inspect_ipa.py`, `python tools/disassemble_ios.py` (Capstone 5.0.9), then `python tools/write_function_map.py`.']
(ROOT/'analysis/binary_function_map.md').write_text('\n'.join(out)+'\n',encoding='utf-8')
print(f'Wrote {len(rows)} evidenced native function entries')
