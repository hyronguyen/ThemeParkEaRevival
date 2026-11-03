# Porting feasibility — 2026-10-24–25

## Decision

Build a small native Windows x64 **preservation preview and asset inspection runtime** now. Do not present it as a playable port or a recreation of recovered park simulation. A complete faithful game remains a separate, evidence-dependent development effort.

The working long-term candidate is Strategy C: a narrowly scoped native runtime reimplementation, reusing original data and documenting each recovered rule. Before implementing gameplay, compare the readable Android DEX methods with the symbol-rich iOS ARMv7 functions. Reuse translated, understood logic where possible; do not start a speculative full rewrite. This selection is provisional pending recovery of original scene/model layouts and server protocols.

## Actual evidence

- The supplied IPA is version 1.8.3, ARMv7 Mach-O. Its encryption load command reports cryptid=0, and 98,009 nlist entries include game/runtime names. This permits static inspection of the supplied bytes; it is not evidence of how this copy was obtained, nor an invitation to decrypt anything.
- Runtime namespaces identify Progressive Media (`dk::progressivemedia::skeleton`, `rflib_ipad`), with OpenGL ES and OpenAL linkage. Android independently contains `dk/progressivemedia` DEX classes and JNI rendering/audio libraries. No positive Unity/Cocos evidence has been found.
- The IPA has 108 numbered BFC data files and four device-specific BFC directories. The iPad3 directory has 8,637 fixed-size resource records; all proposed offset/length ranges fit their corresponding data files. Texture records yield dimension-checked zlib RGBA data, with visually verified original art.
- Both supplied APKs contain executable DEX gameplay logic and native renderer/audio components, but no expansion OBB archive. APK size alone must not be interpreted as absence of game logic.
- Android instruction-aligned string references include original scene initialization concepts and backend build/fullpark operations. These establish code references, not current server availability or a complete offline dependency graph.

## Alternatives

| Approach | Feasibility and limitation |
|---|---|
| A — native code reuse | No portable source tree is provided. ARMv7 ELF/Mach-O objects cannot link into Windows x64. Symbols and disassembly are useful evidence, not C++ source. Independently understood routines may later be translated and tested. |
| B — compatibility runtime | Direct ARM execution is impossible on x64 without translation/emulation. Android DEX reuse requires a VM/AOT strategy and substantial Android/EA/JNI substitutes. iOS additionally requires ABI/framework emulation. Not selected for this milestone; no evidence that a thin wrapper suffices. |
| C — runtime reimplementation | Feasible for file formats, original-asset display, camera/input, and independent local state. Faithful simulation requires much more recovered evidence. Selected only for the bounded native inspection milestone. |

## Prototype contract and preservation limits

Implement a native Win32 C++ application under `src/`, displaying extracted original PNG derivatives and the recovered original path/ride-slot layout. Provide resource selection, mouse/keyboard camera controls, original slot inspection and local versioned preview-state save/load. Do not add invented ride economics, guests, unlocks, or a generic park simulator. This is an asset/layout scene inspection tool, **not** complete original park-state loading, original UI behavior, or a complete Phase 6 gameplay vertical slice. A rendered thumbnail/card is not a recovered 3D building. The path coordinates and first five slot fields are now instruction-verified; their diagnostic view is implemented.

Original archives remain read-only. Raw extracted records and derived previews stay local and ignored by Git. The executable must not embed or redistribute mobile proprietary binaries or artwork. No original EA endpoints are contacted. No authentication, entitlement, purchase validation, or DRM bypass is implemented.

## Blockers and next milestones

1. Finish indexed extraction, preserve exact identifiers and physical locations, validate textures, and classify remaining binary resources.
2. Geometry/material/transform animation milestone achieved: 730 models render and four code-proven idle groups play. Next recover attraction definition/model/group/placement links and original scene draw behavior before building a park renderer. See `model_formats.md`.
3. Cross-check actual function bodies and DEX behavior for placement, pathfinding, economy and progression. Symbols alone cannot establish rules or structure field offsets.
4. Recover complete client/server state schemas and distinguish locally computed state from server responses. Build independent local equivalents only for well-understood legitimate offline gameplay; never assume currency/server authoritativeness from a string alone.
5. Port verified systems incrementally with recorded original-behavior fixtures. Original save compatibility is not promised; preview state is a separate documented schema.
6. Obtain user-provided matching Android expansion data if needed for Android asset comparison. Do not fetch missing content from live EA services.

The native compiler is now configured reproducibly with workspace-local Zig 0.13.0, and a Windows x64 GUI EXE has been built and tested. No global toolchain installation was required. See `toolchain.md` and `test_results.md`.
