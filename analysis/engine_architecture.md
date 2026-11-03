# Engine architecture and confidence

This is a reconstruction of supported relationships, not original source documentation.

| System | Evidence-supported conclusion | Confidence and remaining uncertainty |
|---|---|---|
| Game loop | `RFLib::tick` calls `PMStateManager::tick`; StateInGame::update calls state/network/game components | High for listed calls; timing, scheduling and all branches not recovered |
| Rendering | Custom FFEngine3D/PMGraphics3D/PMMesh stack using OpenGL ES; Android JNI imports include fixed-function vertex/index/texture operations | High for components; medium for complete batch/order/material semantics |
| Scene/state management | PMStateManager, StateInGame and FF scene/group/camera types | High for symbols; medium for reconstructed ownership graph |
| Geometry | PMMesh tables, raw XYZ/UV triangle streams, FFModel nodes and image/material indirection recovered | 730 complete assemblies render; see model_formats.md |
| Animation | PMAnimationTask/PMAnimationData tracks, 33 ms frame interval and quaternion interpolation recovered | 203 groups decoded; four idle model associations verified and played; operating-state bindings remain incomplete |
| Audio | OpenAL/AudioToolbox imports; PMAudioSample/mixer/ADPCM symbols; Android native PMAudioAPI exports | High for components; exact PAC/sample rates/channel encodings unresolved |
| Input | iOS Touch/Control methods; Android onTouchEvent invokes framework touch handling | High for bridges; original gestures and picking transforms not fully reproduced |
| Resource loading | BFC index lookup; CRC32 of low bytes of string code units; dimension-prefixed zlib textures | High: extraction, exact boundaries and selected readers validated |
| Static park layout | GameMap::loadRideSlots reads named integer arrays; paths pairs are passed to Path::AddPath and MiniMap::addStaticPath | High: original topology/slot footprint inspection implemented |
| Runtime park state | MapData/PlaceableItem load WorldInstance/LoadData service structures | High for relationships; initial owned park and saved-state schemas incomplete |
| Local storage | PMProfileStorage save/load uses PMBinaryWriter/Reader | High for presence; scope and full schema not established; not evidence of complete offline saves |
| Service state | ThemeParkSoap creates jobs for LoadWorld, StartBuild, ModifyCash, ModifyXP | High for direct calls; exact server authority/fallback semantics unknown |

The layout preview uses independent Win32/GDI+ drawing. The separate native OpenGL viewer renders original model geometry, hierarchy, UVs/materials and four verified idle animations. Neither re-executes the mobile engine. Full park composition, remaining animation/state bindings, original rendering effects, audio, simulation and UI behavior remain development milestones.

Reproducible supporting records: `ipa_evidence.json`, `ios/symbols.json`, `ios/disassembly.json`, `ios/named_resources.json` and the Android DEX/ELF evidence directories. The curated function map identifies exact addresses and confidence boundaries.
