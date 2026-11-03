# Binary function map

Addresses below are unslid iOS ARMv7 virtual addresses and file offsets inside `Payload/ThemePark.app/ThemePark`, not offsets in the compressed IPA. Raw mangled names are preserved exactly. Functions were selected by non-debug text symbols with the Thumb flag; listing bounds use the next distinct text symbol and are not guaranteed semantic function lengths.

The collector labels direct branch targets only when they match known symbols. It does not resolve all virtual/indirect calls, reconstruct full control flow or separate every literal pool. No listing is presented as portable recovered source.

| Original symbol | Unslid VA | Executable file offset |
|---|---:|---:|
| `__ZN10rflib_ipad6PMFile7calcCRCE8PMString` | `0x000449ec` | `0x000409ec` |
| `__ZN10rflib_ipad6PMFile7initBFCEv` | `0x00044298` | `0x00040298` |
| `__ZN10rflib_ipad6PMFile8openFileEi` | `0x00044bb0` | `0x00040bb0` |
| `__ZN10rflib_ipad6PMFile17readUnsignedShortEv` | `0x00044e14` | `0x00040e14` |
| `__ZN10rflib_ipad6PMFile7readIntEv` | `0x00044fd0` | `0x00040fd0` |
| `__ZN10rflib_ipad6PMFile16fileToFloatArrayEi` | `0x00045898` | `0x00041898` |
| `__ZN10rflib_ipad14PMImageManager13loadImageDataEihRsS1_` | `0x00032688` | `0x0002e688` |
| `__ZN10rflib_ipad14PMImageManager4loadEi` | `0x000319dc` | `0x0002d9dc` |
| `__ZN10rflib_ipad14PMImageManager4loadEii` | `0x00032ebc` | `0x0002eebc` |
| `__ZN10rflib_ipad6PMMesh13loadFromFilesEiss` | `0x00038d20` | `0x00034d20` |
| `__ZN10rflib_ipad6PMMesh8setVertsEi7PMArrayIfES1_IsE` | `0x00038cac` | `0x00034cac` |
| `__ZN10rflib_ipad13PMMeshManager4initEi` | `0x00039a24` | `0x00035a24` |
| `__ZN4ipad2dk16progressivemedia8skeleton4Main11stateUpdateEv` | `0x00173128` | `0x0016f128` |
| `__ZN4ipad2dk16progressivemedia8skeleton4Main12tryGameStartEv` | `0x00171844` | `0x0016d844` |
| `__ZN4ipad2dk16progressivemedia8skeleton5state11StateInGame6updateEv` | `0x001da074` | `0x001d6074` |
| `__ZN4ipad2dk16progressivemedia8skeleton5state11StateInGame10updateGameEb` | `0x001dab00` | `0x001d6b00` |
| `__ZN4ipad2dk16progressivemedia8skeleton5state11StateInGame6renderEv` | `0x001d9598` | `0x001d5598` |
| `__ZN4ipad2dk16progressivemedia8skeleton5state11StateInGame17updateGameObjectsEv` | `0x001d875c` | `0x001d475c` |
| `__ZN4ipad2dk16progressivemedia8skeleton5state11StateInGame9loadBeginEi` | `0x001d6b4c` | `0x001d2b4c` |
| `__ZN4ipad2dk16progressivemedia8skeleton14coreStructures7profile7MapData9loadLevelE8PMHandleINS2_7service8LoadDataEEb` | `0x00073098` | `0x0006f098` |
| `__ZN4ipad2dk16progressivemedia8skeleton4game7Control8loadDataE8PMHandleINS2_7service8LoadDataEE` | `0x00085ef8` | `0x00081ef8` |
| `__ZN4ipad2dk16progressivemedia8skeleton4game7Control7processEv` | `0x00086f60` | `0x00082f60` |
| `__ZN4ipad2dk16progressivemedia8skeleton14coreStructures13PlaceableItem12loadInstanceE8PMHandleINS2_7service13WorldInstanceEE` | `0x00070208` | `0x0006c208` |
| `__ZN4ipad2dk16progressivemedia8skeleton14coreStructures12Serializable12loadInstanceE8PMHandleINS2_7service13WorldInstanceEE` | `0x000754fc` | `0x000714fc` |
| `__ZN4ipad2dk16progressivemedia8skeleton14coreStructures13objectStructs16AttractionStruct14getUpgradeCostEv` | `0x00068fd0` | `0x00064fd0` |
| `__ZN4ipad2dk16progressivemedia8skeleton14coreStructures13objectStructs10RideStruct7getTypeEv` | `0x0006a450` | `0x00066450` |
| `__ZN4ipad2dk16progressivemedia8skeleton14coreStructures13objectStructs10RideStruct12getFlagShiftEv` | `0x0006a468` | `0x00066468` |
| `__ZN4ipad2dk16progressivemedia8skeleton4game11gameobjects5Guest6updateEv` | `0x000a51e4` | `0x000a11e4` |
| `__ZN4ipad2dk16progressivemedia8skeleton4game11gameobjects5Guest18calculateNewTargetEii` | `0x000a5de0` | `0x000a1de0` |
| `__ZN4ipad2dk16progressivemedia8skeleton5astar5AStar8huristicE8PMHandleINS3_9AStarNodeEEf` | `0x00063b88` | `0x0005fb88` |
| `__ZN4ipad2dk16progressivemedia8skeleton5astar5AStar8FindPathEv` | `0x00063bfc` | `0x0005fbfc` |
| `__ZN4ipad2dk16progressivemedia8skeleton5astar5AStar12ValidateNodeE8PMHandleINS3_9AStarNodeEE` | `0x00064400` | `0x00060400` |
| `__ZN4ipad2dk16progressivemedia8skeleton7service12LoadWorldJob3runE8PMHandleIN10rflib_ipad11PMAsyncTaskEE` | `0x001a0ba8` | `0x0019cba8` |
| `__ZN4ipad2dk16progressivemedia8skeleton7service8LoadData7fromXmlE8PMStringS5_` | `0x0019c1d8` | `0x001981d8` |
| `__ZN4ipad2dk16progressivemedia8skeleton7service8LoadData17parseGameSettingsE8PMHandleIN10rflib_ipad9XmlParserEE` | `0x0019e198` | `0x0019a198` |
| `__ZN4ipad2dk16progressivemedia8skeleton7service13WorldInstance7fromXmlE8PMHandleIN10rflib_ipad9XmlParserEE` | `0x001cca50` | `0x001c8a50` |
| `__ZN4ipad2dk16progressivemedia8skeleton7service5Point7fromXmlE8PMString` | `0x001a6434` | `0x001a2434` |
| `__ZN10rflib_ipad16PMProfileStorage4saveE7PMArrayI8PMHandleINS_9PMProfileEEE` | `0x00055978` | `0x00051978` |
| `__ZN10rflib_ipad16PMProfileStorage4loadEv` | `0x00055dbc` | `0x00051dbc` |
| `__ZN4ipad2dk16progressivemedia8skeleton7service13ThemeParkSoap9LoadWorldE8PMStringbb` | `0x001c00b8` | `0x001bc0b8` |
| `__ZN4ipad2dk16progressivemedia8skeleton7service13ThemeParkSoap10ModifyCashE8PMStringi` | `0x001beb60` | `0x001bab60` |
| `__ZN4ipad2dk16progressivemedia8skeleton7service13ThemeParkSoap8ModifyXPEi` | `0x001bf6b4` | `0x001bb6b4` |
| `__ZN4ipad2dk16progressivemedia8skeleton7service13ThemeParkSoap10StartBuildE8PMStringiiiS5_b8PMHandleINS2_4game11gameobjects10AttractionEE` | `0x001b9de0` | `0x001b5de0` |
| `__ZN10rflib_ipad5RFLib4tickEv` | `0x000482b8` | `0x000442b8` |
| `__ZN4ipad2dk16progressivemedia8skeleton4game7GameMap7InitMapEv` | `0x00092170` | `0x0008e170` |
| `__ZN4ipad2dk16progressivemedia8skeleton4game7GameMap4initEv` | `0x00090ea0` | `0x0008cea0` |
| `__ZN4ipad2dk16progressivemedia8skeleton4game7GameMap13loadRideSlotsEb` | `0x00095074` | `0x00091074` |
| `__ZN10rflib_ipad10FFEngine3D6renderEv` | `0x00016b54` | `0x00012b54` |
| `__ZN10rflib_ipad10FFEngine3D6updateEv` | `0x00016a54` | `0x00012a54` |
| `__ZN4ipad2dk16progressivemedia8skeleton4game11gameobjects8RideSlot4readE7PMArrayIiEi` | `0x000b7c68` | `0x000b3c68` |
| `__ZN4ipad2dk16progressivemedia8skeleton4game11gameobjects8RideSlot15getSlotCenter3DEv` | `0x000b8bb4` | `0x000b4bb4` |
| `__ZN10rflib_ipad9PMTextureC2Ejsshj` | `0x000343a4` | `0x000303a4` |

## Conclusions supported by inspected instructions

- PMFile::calcCRC uses low bytes of code units, a CRC lookup table, initial all-ones state and final complement. Its table matches standard CRC32. Named resource matches are independently checked against index ranges and parsed records.
- PMImageManager::loadImageData reads and byte-swaps dimensions, allocates pixel storage and invokes zlib uncompress. Dimension/length/stream validation and visual samples corroborate RGBA8 extraction.
- PMMesh::loadFromFiles reads float and optional short arrays then invokes setVerts. This does not yet establish the complete geometry/material/animation format.
- GameMap::loadRideSlots loads named layout arrays. At VA 0x95842 onward, coordinate pairs are passed to Path::AddPath (call site 0x9585e) and MiniMap::addStaticPath (0x95876).
- RideSlot::read maps ten integer values into its instance; getSlotCenter3D corroborates position/width/height fields and the original 32-unit tile scale. See data_structures.md.
- StateInGame::update directly calls Main::stateUpdate, CalculateTimeslice, NetworkTaskManager::update and other components; updateGame calls camera, quest and game input components. Exact branching/timing remains incomplete.
- ThemeParkSoap::LoadWorld/ModifyCash/ModifyXP/StartBuild construct corresponding asynchronous jobs. Their presence does not prove every action always requires a live server or establish complete fallback rules.

## Android references

The APK report maps actual DEX code_item offsets for renderer/touch callbacks, park initialization, building, upgrades, quests, entity/config parsing and backend operations. These offsets are not interchangeable with native VAs. Full DEX method/instruction-reference indexes are in analysis/android/.

Reproduce with `python tools/inspect_ipa.py`, `python tools/disassemble_ios.py` (Capstone 5.0.9), then `python tools/write_function_map.py`.
