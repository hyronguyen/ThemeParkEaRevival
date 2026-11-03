# Platform comparison

| Property | iOS 1.8.3 | Android 4.2.1 | Android 4.5.88 |
|---|---|---|---|
| Bundle/package | com.ea.themepark.bv | com.eamobile.themepark_row | same |
| Supplied package size | 500,617,742 bytes | 5,573,886 bytes | 6,907,329 bytes |
| Logic | ARMv7 C++/ObjC Mach-O | DEX + ARMv7 JNI | DEX + ARMv7 JNI |
| Native introspection | Rich symbols, debug filenames, cryptid=0 | ELF exports; many DEX classes obfuscated | Same, useful diagnostic strings |
| Engine evidence | Progressive Media rflib/skeleton | Progressive Media Java/native | Progressive Media Java/native |
| Main art | 108 BFC chunks, four indexes | Expansion data absent | Expansion data absent |
| Loose assets | UI, music, fonts, plists, audio samples | Downloader/EASP UI | Downloader, ad UI and wallpaper art |
| Renderer | OpenGL ES linkage, PMGraphics3D/FFEngine3D | GLES1 native imports | Explicit GLES1.1 manifest requirement |
| Offline concerns | ThemeParkSoap asynchronous park/economy jobs | EA/account/store/downloader integrations | Park/configuration endpoints, EA Nimble/Origin, downloader |

The IPA is currently the best supplied source for original assets, readable symbol identities and bounded native-reader inspection. Android 4.5.88 is useful for instruction-aligned DEX string/call references and future decompilation. Neither mobile executable format is directly reusable as a Windows x64 module.

Both Android `librflib.so` copies are byte-identical: SHA-256 `54859ebab0d5d7e4daa0701acd52916b6290d14a25f5e8ee8a6694d3a2f3146a`. Other libraries differ, and SDK additions must not be mistaken for engine changes. 4.2.1 duplicates game class sets for normal/large/xlarge layouts; 4.5.88 reorganizes namespaces.

The extractor compared SHA-256 values of extracted loose archive members across iOS and Android: no byte-identical cross-platform loose assets were found. This is a byte-level result, not a visual-equivalence test. APK artwork is largely service/wallpaper content, and absence of OBB means corresponding game BFC payloads cannot be compared. iOS device variants retain all directory references while deduplicating 7,965 physical BFC ranges.

Different platform/version numbers do not establish equal content, mechanics or chronology. The project has not observed either original game running. Asset/behavior matching remains unverified until suitable original runtime recordings or further recovered evidence are available.
