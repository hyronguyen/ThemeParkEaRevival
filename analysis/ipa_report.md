# iOS IPA investigation

Investigated 2026-10-24–25. Findings refer to the supplied bytes, not a claim that this is an untouched publisher distribution.

## Identity and archive

The requested `input/ThemePark.ipa` was absent. Actual input: `resource/Theme_Park_1.8.3_ios_5.1.1.ipa`, 500,617,742 bytes, 551 ZIP entries. SHA-256 before and after inspection/extraction: `a7d164fbc076c8c35afead1871452af5a9f4b862975423aa6552c11feca4e2a3`.

`Payload/ThemePark.app/Info.plist` identifies `com.ea.themepark.bv`, display name Theme Park, version/build 1.8.3, minimum iOS 5.1.1, iPhone/iPad families, landscape-right orientation, SDK 8.1 and Xcode 6.1. This particular build is later than the game's 2011 launch; its assets/rules must not be assumed identical to the launch release.

## Executable — confirmed

`Payload/ThemePark.app/ThemePark` is 9,525,760 bytes, SHA-256 `bfcd3dcd0d5c783cd4eaf50662b2106b6cb06a415a9d49f45a7489fbfda94ffa`. It is a thin, little-endian 32-bit Mach-O executable: magic `0xfeedface`, CPU type 12, subtype 9 (ARMv7), file type 2, 51 load commands. It is not x64, ARM64 or a universal/fat executable.

`LC_ENCRYPTION_INFO` at executable file offset 2664 reports cryptoff 16384, cryptsize 5079040, **cryptid 0**. The file does not declare an actively encrypted range. Selected Thumb functions also disassemble into coherent symbol-linked calls and resource readers. No decryption was attempted; this does not establish the copy's history.

The symbol table contains 98,009 entries: 73,284 debug/STABS and 24,725 non-debug. These are entries, not 98,009 independent recovered functions. Source filenames and original build paths survive, but their source file contents are not provided. Evidence exports preserve symbol type, section, Thumb descriptor, unslid virtual address, file offset and string-table location.

## Dependencies and engine

Imported libraries/frameworks include OpenGLES, OpenAL, UIKit, Foundation/CoreFoundation, QuartzCore, AudioToolbox, AVFoundation, CFNetwork, Security and numerous social/service frameworks. The complete list is in `ipa_evidence.json` under `macho.dylibs`. Imports are evidence of linkage, not proof every linked feature is exercised.

Compiled namespaces and debug paths identify `rflib_ipad`, `ipad::dk::progressivemedia::skeleton`, `PMGraphics3D`, `FFEngine3D`, `PMMeshManager`, `PMImageManager`, `PMStateManager` and `PMFile::initBFC`. **High confidence: custom Progressive Media C++/Objective-C runtime**, independently corroborated by Android namespaces. No positive Unity or Cocos runtime evidence was found. See `engine_identification.md` and `engine_architecture.md`.

## Assets and compression

There are 108 numbered BFC data chunks plus four device-specific `head_*.bfc` indexes, four `audio_*.pac` files, extensionless `wv` samples, three MP3 music tracks, one ordinary WAVE, six OpenType fonts, PNG/JPEG images, binary plists, nibs and a small SQLite file. Extensionless audio does not automatically mean WAVE, and the SQLite tracking file is not established as the game save.

BFC directory layout is verified by exact size/range checks and PMFile reader evidence. Dimension-prefixed zlib texture payloads decode to RGBA8 and unresolved 16-bit pixel layouts. A sample of PNG derivatives was visually checked against original building/UI art. Four index variants share many physical payload ranges. Original paths/slot data were recovered by CRC32 filename lookup and validated against runtime readers; details are in `asset_formats.md` and `data_structures.md`.

## Network/service evidence

The plist contains `ServerEnvironment=live` and EA Mayhem/IPSP settings. Symbols include ThemeParkSoap, LoadWorldJob, ModifyCashJob, ModifyXPJob, StartBuildJob, Origin/account operations and XML response structures. Literal hosts include EA Synergy/SKUM, Facebook, Fiksu and PlayHaven; no endpoint was contacted. Presence alone does not prove current availability or establish mandatory login behavior.

Direct calls in selected ThemeParkSoap methods construct asynchronous service jobs. This supports server-dependent park/economy operations, but a complete offline dependency graph and remote configuration have not been recovered. No keys, credentials or original purchase validation were requested or bypassed.

## Reproduce and limitations

Run `python tools/inspect_ipa.py`, then optionally `python tools/disassemble_ios.py` with Capstone 5.0.9 installed into `tools/_deps`. The latter bounds listings by the next distinct text symbol. Literal pools and jump tables near tails can be misread as instructions; its output is neither a full control-flow analysis nor compilable recovered source. No original iOS runtime session was executed, so fidelity has no live behavioral baseline yet.

Mach-O field interpretation follows Apple's [loader definitions](https://github.com/apple-oss-distributions/xnu/blob/main/EXTERNAL_HEADERS/mach-o/loader.h). All game-specific conclusions come from the local IPA evidence.
