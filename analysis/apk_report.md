# Android APK investigation

Date: 2026-10-24. Original archives were opened read-only, not installed or run. No game service was contacted. Findings below are static evidence, not claims about successful original gameplay.

## Inputs and identities — confirmed

The requested `input/ThemePark.apk` does not exist. Both discovered APKs were inspected because their manifest identities match the target, despite different filenames.

| Field | `resource/05447-Theme-Park-4.2.1.apk` | `resource/theme-park.apk` |
|---|---|---|
| Archive bytes | 5,573,886 | 6,907,329 |
| Package | `com.eamobile.themepark_row` | `com.eamobile.themepark_row` |
| Version name / code | `4.2.1` / `4002001` | `4.5.88` / `4005088` |
| Minimum / target Android API | 7 / 14 | 13 / 19 |
| ZIP members | 55 | 229 |
| DEX | one `classes.dex`, DEX 035 | one `classes.dex`, DEX 035 |
| DEX class definitions / method IDs | 3,200 / 30,003 | 4,996 / 37,842 |
| Native ABI present | `armeabi-v7a` only | `armeabi-v7a` only |
| Explicit graphics requirement | no `uses-feature` GLES declaration found | `glEsVersion=0x00010001` (OpenGL ES 1.1) |

SHA-256:

- 4.2.1: `fcb40dd67395c8b85dafe96783235915c33fd8c478483f34114ad0d6a1f0af41`
- 4.5.88: `25d327903ebb9801504e33a79da4b454fb2191a53c392f1a744098bac348c8b5`

Both are the same Android package, not unrelated games. The newer package also includes a Theme Park live wallpaper service and image sets. Those wallpaper assets should not be mistaken for complete playable park assets.

## Engine and architecture

**High confidence: a custom Progressive Media framework with substantial Java/Dalvik game logic and native graphics/audio helpers.** This is not evidence of a generic EA engine name. Attribution follows the compiled namespaces and exported symbols, not visual appearance.

- Both define `dk.progressivemedia.android.PMActivity`, `PMGLSurface`, `PMGraphics`, `PMAudioAPI`, `PMOS` and `PMScreenSelector`.
- 4.2.1 repeats 453 classes under each of `normal/dk/progressivemedia`, `large/dk/progressivemedia`, `xlarge/dk/progressivemedia`. 4.5.88 has a single set including 361 `dk/progressivemedia/skeleton` and 133 `dk/progressivemedia/a` classes. Many class/method names are obfuscated, but diagnostic and XML-field strings survive.
- `librflib.so` exports JNI entry points including `Java_dk_progressivemedia_android_PMGraphics_drawImage`, `..._fillRect`, `..._flushBuffer`, `..._setTexture`, and C++ `PMOpenGL` symbols. Its imports include `glDrawElements`, `glDrawArrays`, `glVertexPointer`, `glColorPointer`, `glTexCoordPointer`, `glBindTexture`, `glTranslatex` and `glScalex`. This supports a batched, fixed-function OpenGL ES renderer. Exact ordering and batching policies remain to be recovered.
- `libAudioLib.so` exports JNI `PMAudioAPI_Load`, `Play`, `Stop`, `Update`, `SetVolume`, `Initialize`, `Destroy` and related operations. Audio codecs/mixer internals are not yet established.
- 4.5.88 DEX contains `com.playhaven.android.compat.UnityCompat`, an ad SDK compatibility adapter. This isolated name is **not** evidence the game uses Unity. No `libunity.so`, Unity data structure, Cocos2d native library, or Marmalade/Airplay runtime signature was identified in these packages.

## Native ELF files — confirmed

All supplied `.so` files parse as little-endian ELF32 shared objects, `e_machine=40` (ARM), not Windows x64 modules.

| Package | Library | Uncompressed bytes | DT_NEEDED |
|---|---|---:|---|
| 4.2.1 | `libAudioLib.so` | 131,137 | libc, libstdc++, libm, liblog, libGLESv1_CM |
| 4.2.1 | `libEASPStoreUIExample.so` | 5,604,984 | liblog, libGLESv1_CM, libstdc++, libm, libc, libdl |
| 4.2.1 | `librflib.so` | 70,772 | liblog, libGLESv1_CM, libstdc++, libm, libc, libdl |
| 4.5.88 | `libAudioLib.so` | 103,536 | liblog, libstdc++, libm, libc, libdl |
| 4.5.88 | `librflib.so` | 70,772 | liblog, libGLESv1_CM, libstdc++, libm, libc, libdl |

All dependency names above carry the `.so` suffix in the actual ELF. The two copies of `librflib.so` are byte-identical (SHA-256 `54859ebab0d5d7e4daa0701acd52916b6290d14a25f5e8ee8a6694d3a2f3146a`). The EASP library exposes EA I/O, threading, browser, Facebook, store and `PMiPSP` service integration functions. Its size alone must not be mistaken for evidence it contains the core gameplay engine.

## Resources and missing expansion data

ZIP members use stored or Deflate compression. Actual PNG signatures are present in image files. 4.2.1 bundles `assets/EASP/WinSet/Package/Android/EASP.package` (1,042,984 bytes) and `EASP1.package` (246,351 bytes), each beginning `44 42 50 46 02 00 00 00` (`DBPF`, version 2 header). Both contain a PNG signature at offset 96. These are service UI resource packages; internal indexing/compression has not been fully parsed by this APK tool. `assets/EASP/WinSet/resource_Android.cfg` supplies package configuration.

Neither APK contains `0.bfc` or an `.obb` expansion. Their absence is significant because actual DEX instructions establish expansion loading:

- In 4.5.88, `PMActivity.TryOpenZipResources` at `classes.dex` **code_item** offset `0x13b488` checks the APK asset list for `0.bfc`, checks external storage, then calls `APKExpansionSupport.getAPKExpansionZipFile(Context,int,int)` with version-code lookups. It has error branches/logging for unavailable storage and null expansion files.
- `PMAADActivity.getMainFile` at `0x139464` constructs `/Android/obb/` and `main.` / package / `.obb` strings. The resulting naming scheme is `main.<version>.<package>.obb`; exact eligible versions require further call-site tracing.
- `PMAADActivity.onCreate` at `0x139928` calls `hasVerifiedDownload`; the method includes either launching an activity or creating/starting the download client. A downloader service and activity appear in both manifests.

Thus the APKs supply useful logic but not all primary game resources. The IPA is an alternative asset source, subject to differences in resource version and layout.

## Game logic evidence and recovery potential

The custom collector walks real DEX instruction boundaries and extracts `const-string` and invocation references; the following are verified **code_item offsets in the uncompressed 4.5.88 classes.dex**, not native addresses. Obfuscated names are preserved verbatim and no recovered pseudocode is claimed to be compilable source.

| Class / method | Offset | Evidence and conservative interpretation |
|---|---:|---|
| `dk/progressivemedia/a/i/a.onDrawFrame` | `0x2c8830` | Actual renderer callback invokes custom framework update/render routines; their exact semantic partition still unknown. |
| `dk/progressivemedia/a/i/a.onTouchEvent` | `0x2c8c74` | Invokes `dk/progressivemedia/a/f/d.a(MotionEvent)`; touch bridge located. |
| `dk/progressivemedia/skeleton/d/g.a` | `0x307794` | Diagnostics for `loadRideSlots`, `InitMap`, theme slots, territory zones/entrances, paths, decorations, park office and clouds. Park/slot initialization candidate. |
| `dk/progressivemedia/skeleton/d/g.g` | `0x308b08` | `Build slot clicked` and `Ride/Shop Menu`; slot interaction candidate. |
| `dk/progressivemedia/skeleton/d/b/a.d` | `0x2da648` | `StartBuild`; building lifecycle candidate. |
| `dk/progressivemedia/skeleton/d/b/a.i` | `0x2daae0` | `CompleteBuild`; building lifecycle candidate. |
| `dk/progressivemedia/skeleton/d/b/r.h` | `0x2e1f2c` | `Upgraded ride`; ride-upgrade candidate. |
| `dk/progressivemedia/skeleton/g/a.c` | `0x31b498` | `Unlock Ride`, `Add Park Rank`, `Unlock Build Slot`, wait-for-build diagnostics; quest/progression candidate. |
| `dk/progressivemedia/skeleton/h/f.a` | `0x3246e0` | Entity schema parser candidate: build time/cost, capacity, sickness, earnings, entrance, upgrade effects, payout time, guests served, unlock levels. |
| `dk/progressivemedia/skeleton/h/ao.a` | `0x31f2ac` | Park response/configuration parser candidate: `Resources`, `Entity`, `Quest`, `WorldInstance`, `SavedScript`, `ThemeUser`, `cash`, `superTickets`, `xpLevel`, `xpProgress`, rewards and daily bonuses. |
| `dk/progressivemedia/skeleton/i/c.f` | `0x32aab0` | `Create guests`, `Create park`, `started park`; scene start candidate. |
| `dk/progressivemedia/skeleton/i/c.v` | `0x32d0e0` | `max guests`, `No People In Park`; guest-count diagnostic candidate. |

These findings locate useful methods but do not recover pathfinding rules, economy constants, timing semantics, collision behavior or save schema. The Java gameplay is a better immediate target for deeper decompilation than treating the small ARM rendering library as the whole game.

## Networking, accounts and offline implications

Confirmed integrations include INTERNET/network-state permissions, Android billing and license-check permissions, download services, and EA `PMiPSP` bridge code. 4.2.1 includes EA EASP and Facebook/store native integrations; 4.5.88 includes EA Nimble identity, `AuthenticatorOrigin`, Synergy networking, purchase handling and Google Play service code. SDK presence alone does not prove every included SDK feature is used by gameplay.

Game-specific DEX methods reference `fullpark/`, `fullpark/x/themeuser`, `cash/`, `build/start/worldinstance/`, `build/complete/worldinstance/`, `build/finish/worldinstance/`, `savedscript/?verifier=` and park/Origin account-migration states. Examples in 4.5.88 are `h/ap.a` (`0x31fe6c`, full park), `h/bt.a` (`0x322050`, start build), `h/t.a` (`0x3266e0`, complete build) and `h/aa.a` (`0x31df6c`, finish build). This is strong static evidence of server-shaped progression/park operations; precise success, failure and offline fallback behavior still requires full control-flow analysis.

Literal endpoints include the EA SKUM Theme Park MTX timestamp path and `/mh/games/themepark/`. They were recorded only, never contacted. Their current availability and historical payloads are unknown. There is no recovered server database or authoritative remote game configuration in these APKs. An offline implementation must define independent local state and distinguish reconstructed rules from evidence-backed original behavior. Account credentials, keys, license bypasses and store transactions are outside this work.

## Reproduction, verification and limitations

Run `python tools/inspect_apk.py` from the project root. Outputs include `analysis/apk_evidence.json`, each decoded manifest, DEX method indexes/references, gameplay string xrefs, ELF section/symbol/import metadata and printable string offsets under `analysis/android/`.

Both ZIP CRC checks, DEX Adler-32 checksum, DEX SHA-1 signature and header-size checks pass. These validate package/index consistency; they do not authenticate the publisher certificate or prove unmodified publisher provenance. All defined nonnative DEX code items were walked to their declared instruction boundary without parser overrun. Input hashes permit later unchanged-file checks.

The collector is intentionally an evidence/index tool, not a complete verifier or decompiler. DEX modified-UTF8 is decoded permissively for display; non-ASCII edge cases may be replaced. It does not resolve every Android resource ID, exception handler, dataflow/branch relationship, virtual dispatch or native relocation. File offsets in DEX/ELF reports refer to the decompressed member, not the outer APK. Printable native strings do not establish function use without cross-references.

`jadx`, `apktool`, `readelf` and `objdump` were not found on PATH. A standard-library parser provided the required initial inspection without installing tooling. A dedicated decompiler is still needed before claiming semantic recovery or compiling translated gameplay.

Format references used: [AOSP DEX format](https://source.android.com/docs/core/runtime/dex-format) and [AOSP Dalvik instruction format](https://source.android.com/docs/core/runtime/dalvik-bytecode). Engine and package conclusions come from the local binaries, not these general format references.
