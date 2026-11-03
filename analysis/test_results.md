> Updated 2026-10-26: see [expansion_ui_gameplay.md](expansion_ui_gameplay.md) for the
> current no-queue admission rule, 163-item original-art catalog, bus lifecycle,
> ten guest variants, office/decorations, built fences and TPPLAY4 saves.
> The earlier milestone descriptions below retain their historical parameters.

# Latest verification — 2026-10-26

44 Python tests, full native model regression, C++ simulation and native playtest
pass. The playtest now places five rides and completes 200 visits in 310.4 seconds
with 40 guests. Castle tests confirm three visible bouncing passengers, three
original tracks played muted and 4x MSAA. Game/workbench PE verification passes
with Windows WinMM audio imports. See `castle_audio_rendering.md` for the current
binary hash and evidence. The following sections record older milestones; their
hashes and hidden-passenger/audio limitations are historical.

# Test results — 2026-10-25

## Offline playtest addition

`ThemeParkGame.exe --self-test` passes: 40 guests, four placed rides, 193 completed visits over 310 simulated seconds; mouse placement, occupied-slot rejection, pause/speed, close/reopen, visitor inspector and exact save/restore. Real hidden HWND/WGL captures `playtest-{start,running,build,guest}.png` were inspected.

The independent C++ simulation test passes: 550 simulated seconds, 24 guests conserved, 1,043 completed visits, capacity/state invariants, disconnected/oversized/occupied placement rejection, closure and deterministic save/resume. Malformed/truncated/unknown/trailing save input is rejected without replacing live state. Reproduce with `scripts/test-playtest.ps1` after building.

The 41 Python checks and complete 730-model/5,934-instance native regression also pass after integration. The current `ThemeParkGame.exe` and `ThemePark3D.exe` share a 738,816-byte binary, SHA-256 `3697ca10522c29d856039976f250d90e0bed3ece5e38eb6f7b4a0656dd90b9be`; executable name or `--play` chooses the initial mode. PE checks confirm AMD64/PE32+, no CLR and only Windows system imports. Earlier binary hashes below describe the preceding static-scene milestone.

Playtest rules are authorized approximations, not recovered original economy or AI. Guests are hidden while boarded; seat attachment, operating animations, original UI, audio and broader gameplay remain unimplemented. See `playtest_mechanics.md`.

## Passed on this Windows host

| Verification | Result |
|---|---|
| Original archive SHA-256 before/after and current hashes | All three unchanged |
| APK ZIP CRC and DEX Adler32/SHA1/header checks | Both packages pass |
| BFC directory counts and all referenced ranges | Pass; 7,965 unique physical records |
| Raw extraction provenance and sampled output hashes | Pass |
| Texture exact byte count, truncated/trailing stream rejection, decompression limit | Pass |
| Archive traversal rejection, missing chunk and range-overrun rejection | Pass |
| Layout count/stride, signed big-endian parsing and malformed record rejection | Pass |
| Python regression suite, including real input/manifest checks | **41 tests pass** |
| Native C++ builds | Pass; `build/ThemeParkPreview.exe` and `build/ThemePark3D.exe` |
| PE verification | AMD64 PE32+, Windows GUI subsystem 2, no CLR directory |
| Native PNG loading | All 774 catalog images load; missing PNG rejected |
| Native save roundtrip | Pass for selected resource and camera values |
| Corrupt/unknown-version save and non-finite state | Rejected |
| Window/event loop smoke test | Exit 0 |
| Keyboard pan, mouse wheel zoom, right-button drag | Pass through window messages |
| Original path picking and ride-slot footprint picking | Pass |
| Layout/art view switch and previous/next image | Pass |
| S/L save/load through native input handler | Pass |
| Native render outputs | Layout and artwork PNGs produced and visually inspected |
| Model parser and image indirection | All 730 assemblies, 3,211 mesh parts, 299 textures; no missing texture parts |
| Original animation decoding | 3,363 tracks, 203 groups; exact payload consumption |
| Native OpenGL draw verification | All 730 models / 322,730 triangles; no GL errors on Intel UHD Graphics |
| Original idle animation bindings | Models 425/427/515/522; groups 47/49/151/159, verified from ARM code |
| Animated native rendering | Four models move; 60 sample times each render successfully |
| Playback controls | Pause, 33 ms frame step, bind-pose toggle, next animated model pass |
| 3D visual evidence | Model 409 front/back/wireframe and model 425 at 0/330 ms inspected |
| Static park assembly | 20 grass areas, 329 dividers, 5,585 unique paths; 19 duplicate rows retain provenance |
| Static park native rendering | 5,934 instances, 22 cached model/material combinations; no GL errors |
| Scene controls and resource lifetime | Layer toggles, wireframe, model/park switch and cache rebuild pass |
| Scene visual evidence | Overview and top-down detail inspected; grass tiles, path corners and canal bridges connect |
| User video reference | MP4 16:59.69, 17 sampled frames, unchanged SHA-256; sparse visual comparison only |

The layout EXE baseline is 610,304 bytes, SHA-256 `6fe5be991ec75c3690475baa539efebbd68f07033e4dc03c3c47aeef01cf622a`. The current 3D/park EXE is 630,272 bytes, SHA-256 `7ab675a37ef3e39ca1489033fd12b24885cd296c3eb37035d370030f9037515c`. Rebuilding may change bytes; `tools/verify_native.py [executable-name]` records the current result.

## Evidence and reproduction

Run from the project root:

```powershell
$env:THEMEPARK_VERIFY_REAL_ASSETS = '1'
python -m unittest discover -s tests -v
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1
python tools/verify_native.py
$test = Start-Process .\build\ThemeParkPreview.exe --self-test -WindowStyle Hidden -PassThru -Wait
$test.ExitCode
$smoke = Start-Process .\build\ThemeParkPreview.exe --smoke-test -WindowStyle Hidden -PassThru -Wait
$smoke.ExitCode
python tools/verify_native.py ThemePark3D.exe
$models = Start-Process .\build\ThemePark3D.exe --self-test -WindowStyle Hidden -PassThru -Wait
$models.ExitCode
```

Machine-readable evidence: `analysis/runtime_test/self_test.json`, `smoke_test.json`, `pe_verification.json`. The smoke test creates a real HWND/event loop and sends keyboard/mouse messages through the application handler. PNG captures use the application's native render routine into GDI+ bitmaps. They are render outputs, not full desktop screenshots or proof of physical user-input delivery.

The 3D counterpart writes `model_test.json`, `ThemePark3D_pe.json` and `native-3d-*.png` / `native-animation-*.png` / `native-park-*.png`. It uses a real hidden HWND and WGL context, reads the GL back buffer, checks transformed matrices at different times and exercises application input handlers. The final mipmap/HUD build passed the complete native suite again. Original renderer fidelity is not established by successful GL calls alone.

The supplied Windows sandbox initially denied temporary-directory operations and compiler temporary-file renames. Approved runs outside that sandbox passed. Failed sandbox attempts are environment failures and are not counted as passing tests.

## Not tested or not implemented

No original game runtime was executed. The video provides sparse visual corroboration, not a frame-synchronized gameplay comparison. Original account/login behavior, server responses, operating ride animations, audio, economy/progression and original save compatibility remain unimplemented. The new playtest implements provisional construction and guest states as described above; this is separate from the older workbench's footprint inspection. Original animation validation remains limited to four code-proven idle bindings.

Only this host was tested; Windows 10 and 11 are the intended target, not a completed multi-version/DPI/accessibility compatibility matrix. No long-duration simulation soak is meaningful until simulation exists. No game services were contacted during verification.
