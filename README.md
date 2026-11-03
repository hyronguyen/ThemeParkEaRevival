# Theme Park EA preservation research / native Windows preview

**Tài liệu phát triển:** [Trạng thái hiện tại](docs/STATUS.md) · [Roadmap](docs/ROADMAP.md) · [Cấu trúc/build/test](docs/DEVELOPMENT.md) · [Mục lục bằng chứng](analysis/README.md).

This repository contains the Windows game source, preparation tools, tests and research notes. To play, supply the locally prepared `extracted/` runtime from your own game files alongside `build/`. Original game packages, textures, models, audio and compiled releases are excluded from Git.

This workspace investigates the supplied Theme Park mobile packages and contains a **native Windows x64 offline playtest** plus model/animation and layout workbenches. The playtest uses original assets with user-authorized provisional NPC/ride rules. A faithful complete game port is still unfinished.

The engine is identified with high confidence as a custom Progressive Media runtime: C++/Objective-C on iOS and substantial Java/Dalvik code with native rendering/audio helpers on Android. The supplied IPA has readable ARMv7 code with rich symbols. Both APKs contain useful logic but omit their main expansion assets.

## Play the offline sandbox

Launch `build/ThemeParkGame.exe` after preparing the runtime. A new park starts at the entrance with four incoming visitors. Build Shop shows the selected artwork above a strip of 3D models. Its catalog includes 96 rides, 38 shops and five staff houses.

Press **B** to browse, choose a matching plot and build. Select a ride for its 20-second minigame, or select the park office to change its style and expand it. The overview camera zooms out to 4.00. Selecting a building moves the camera closer, and closing its panel restores the overview.

The castle's original water channel and bridge are now visible, with built-attraction fences and gates. The first territory contains the original park office, and a front road, park gate and bus connect to the paths. Original decoration rows add trees/rocks; ten texture variants diversify visitors.

| Control | Action |
|---|---|
| B / bottom-left build icon | Open original-art RIDES/SHOPS catalog |
| F1 / gear | Show or hide debug controls |
| O | View office, park entrance and bus stop |
| G | Cycle selected attraction's four original gate levels |
| Left click | Select / build / use menu |
| Right drag / wheel | Pan / zoom |
| Q / E / Home | Orbit / focus selected attraction |
| R / Esc | Rotate proposed placement / cancel or close catalog |
| Space / upper-right arrow | Pause / change simulation speed |
| S / L | Save / load |
| Debug music controls | Toggle, change original track, adjust volume |

Progress saves to `%LOCALAPPDATA%/ThemeParkPreservation/playtest-v6.txt`. Previous v5/v4/v3/v2 files import when no v6 file exists and remain unchanged. Keep the EXE in `build/` and the generated `extracted/` tree in the project root.

**27 October update:** click an empty plot for exact-size items; locked plots have rocks and UNLOCK is currently free. Castles animate only when occupied, and guests appear instantly inside. Wheel zoom adjusts tilt smoothly without changing yaw. 160 catalog entries fit recovered plots; three 1×1 utilities await matching plots. See [current refinement notes](docs/refinements-2026-10-27.md); these supersede the earlier entrance/exit animation description above.

**Provisional:** attraction names/categories, capacities, independent visit durations, castle elasticity, bus scheduling/road placement and some placement footprints. UI artwork and font are original; native desktop menu logic is reconstructed. Shops accept timed visits but do not yet implement inventory/income. Original operating animations, effects, economy and full UI behavior remain unfinished. See `analysis/expansion_ui_gameplay.md` for evidence and limits.

Build with `scripts/build.ps1`; test with `scripts/test-playtest.ps1`.

## Run the preservation workbenches

Double-click `build/ThemePark3D.exe` to open the original static park: **20 grass areas, 329 dividers and 5,585 unique path tiles** (19 duplicate source rows retained in provenance). Geometry, positions, path adjacency and default materials come from the supplied resources and verified original code. This view shows all territories with their default theme; owned rides and progression are not reconstructed.

Press P for the model viewer, or launch with `--models`. It loads all **730 original models**, 3,211 mesh parts and 299 original texture derivatives. Four code-verified idle animations are playable (models 425, 427, 515 and 522); model mode starts with 425. Source tracks use 33 ms/frame, linear scale/translation interpolation and shortest-path quaternion interpolation.

| 3D control | Action |
|---|---|
| Left drag / right drag / wheel | Orbit / pan / zoom |
| P | Switch static park / model viewer |
| 1 / 2 / 3 (park) | Toggle grass / dividers / paths |
| Left / Right arrows | Previous / next model |
| A | Next model with verified animation |
| Space | Pause / resume |
| [ / ] | Step backward / forward 33 ms |
| B | Toggle original bind pose |
| W / G / Home | Wireframe / grid / reset camera |
| Esc | Close |

Keep the EXE in `build/` and the generated `extracted/` tree alongside it in the project root. Other models remain in bind pose until their animation associations are verified. This viewer does not reconstruct ride operation, passengers, sound, original camera, UI or gameplay. See `analysis/model_formats.md` for evidence and fidelity limits.

Double-click `build/ThemeParkPreview.exe`. Keep it inside `build/` with the generated `extracted/` tree in the project root. No mobile emulator, Python, Java or compiler is needed to run the prepared preview; it uses Windows system libraries and local asset files.

The older layout app displays 774 original iPad3 RGBA textures and the original static layout of 5,604 path rows plus 247 ride-slot footprints. Its top-down drawing and inspection UI are diagnostic; original UI behavior and simulation are not implemented.

| Control | Action |
|---|---|
| Tab | Switch original layout / artwork |
| Mouse wheel | Zoom |
| Right mouse drag or arrow keys | Pan |
| Left click on layout | Inspect a path or original ride-slot footprint |
| Page Up / Page Down or Previous / Next | Browse textures |
| Home | Fit current view |
| S / L | Save / load local preview state |
| Esc | Close |

Normal saves go to `%LOCALAPPDATA%/ThemeParkPreservation/preview.json`. They contain camera/resource selection only, not original game progression. The save format is documented in `analysis/data_structures.md`.

## Supplied references

The requested `input/ThemePark.ipa` and `input/ThemePark.apk` were not present. The discovered originals are:

- `resource/Theme_Park_1.8.3_ios_5.1.1.ipa`
- `resource/05447-Theme-Park-4.2.1.apk`
- `resource/theme-park.apk` (manifest version 4.5.88)

All were treated read-only. SHA-256 checks confirm they remain unchanged. Generated assets, original archives, proprietary evidence derivatives, toolchain and builds are ignored by `.gitignore`. This workspace has not been published or committed to a public repository. No proprietary mobile binaries or artwork are embedded in the preview source/EXE.

## Reproduce investigation and extraction

Use Python 3.12+; Pillow is needed for PNG generation. The standard-library inspectors do not require apktool/JADX/Ghidra. Run from the project root:

```powershell
python tools/inspect_ipa.py
python tools/inspect_apk.py
python tools/extract_assets.py
python tools/export_park_layout.py
python tools/extract_models.py
python tools/extract_animations.py
python tools/export_park_scene.py
python tools/export_model_locators.py
python tools/export_passenger_poses.py
python tools/prepare_game_audio.py
python tools/prepare_game_sfx.py
python tools/prepare_expansion.py
```

In this workspace, PNG extraction was run using the bundled Python with Pillow 12.3.0:

```powershell
& 'C:\Users\kutao\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe' tools/extract_assets.py
```

Without Pillow, the extractor still writes raw assets and provenance but cannot populate the PNG runtime catalog. Use an environment with Pillow when rebuilding runnable assets. Extraction preserves 7,965 unique BFC payloads, with 2,280 RGBA8 PNG derivatives across device variants. Unidentified records remain raw under `extracted/unknown/`; filenames and offsets are retained in `analysis/asset_inventory.json`.

Optional bounded native disassembly:

```powershell
python -m pip install --target tools/_deps capstone==5.0.9
python tools/disassemble_ios.py
python tools/write_function_map.py
```

These listings are analysis evidence, not recovered compilable C++.

## Build Windows x64

The pinned workspace-local compiler is Zig 0.13.0. Bootstrap needs internet access to the official compiler source, verifies SHA-256, and does not install globally:

```powershell
python tools/bootstrap_toolchain.py
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1
python tools/verify_native.py
```

The resulting PE32+ AMD64 GUI executables use Windows system libraries and Universal CRT. The 3D viewer also uses system OpenGL through WGL. Runtime dependencies and toolchain provenance are documented in `analysis/toolchain.md`. The compiler and extraction tools are development dependencies only.

## Verification and reports

47 Python tests pass, including real archive integrity/range checks, model/animation parsing and static scene assembly. `ThemePark3D.exe --self-test` draws all 730 models and the 5,934-instance park through real OpenGL, verifies four moving assemblies, camera/playback controls, scene layers and cache reload. The older preview tests cover PNG loading, persistence and layout picking. See `analysis/test_results.md` for exact commands and limitations.

The user-supplied MP4 in `resource/` is now the visual reference. Seventeen sampled frames and source integrity metadata are under `analysis/previews/gameplay/`. See `analysis/gameplay_reference.md` and `analysis/scene_formats.md` for observations, evidence and limits.

Reports:

- `analysis/ipa_report.md`, `apk_report.md`, `platform_comparison.md`
- `analysis/engine_identification.md`, `engine_architecture.md`
- `analysis/asset_formats.md`, `asset_inventory.json`, `park_layout.json`
- `analysis/model_formats.md`, `models.json`, `animations.json`
- `analysis/gameplay_systems.md`, `binary_function_map.md`, `data_structures.md`
- `analysis/porting_strategy.md`, `offline_compatibility.md`
- `analysis/test_results.md`, `remaining_issues.md`

The next milestone is passenger attachment and operating animation for the other rides, followed by themed materials, effects and guest appearance variants. Guests, ride operation, economy, progression, original save compatibility and full offline gameplay remain unfinished. No live EA service was contacted, no DRM was defeated and no account/key material was obtained.
