# Engine identification

**Conclusion: custom Progressive Media runtime — high confidence.** The iOS build contains native C++/Objective-C game code; Android contains substantial Java/Dalvik game code with native graphics/audio helpers. The evidence does not support calling either package Unity, Unreal, Cocos2d or a known generic EA engine.

| Evidence | Location | Confidence / implication |
|---|---|---|
| `rflib_ipad`, `ipad::dk::progressivemedia::skeleton` | iOS non-debug C++ symbols | High: runtime and game namespaces |
| Original source path containing `dk/progressivemedia/rflib/.../PMFile.mm` | IPA executable printable strings; exact offsets in evidence | High: custom library provenance; no source contents recovered |
| PMFile BFC reader and PMImageManager zlib calls | Thumb function bodies and resource range validation | High: custom asset loading actually used |
| FFEngine3D, PMGraphics3D, PMMeshManager | iOS symbols and direct call annotations | High for presence; medium for full rendering architecture |
| `dk.progressivemedia.android.PMActivity`, PMGraphics, PMGLSurface, PMAudioAPI | Both APK DEX class definitions | High: independent Android corroboration |
| JNI PMGraphics functions and OpenGL ES calls | `librflib.so` ELF symbols/imports | High: native rendering bridge |
| `com.playhaven.android.compat.UnityCompat` | Newer APK ad SDK | Not Unity engine evidence: no accompanying Unity runtime/data |

Absence of common engine filenames alone is weak evidence; the positive linked symbols, method definitions, reader behavior and debug paths establish the conclusion. "Proprietary/custom" describes this runtime, not an inferred commercial engine product name. Full architecture confidence is broken down in `engine_architecture.md`.
