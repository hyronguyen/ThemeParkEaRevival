# Original 3D models and transform animation — 2026-10-25

The Windows workbench now renders 730 original model assemblies, comprising 3,251 hierarchy nodes (3,211 mesh nodes and 40 non-mesh nodes), 322,730 triangles and 299 texture resources. All 730 assemblies exported without missing material textures. Four original idle animations have code-proven model associations and native playback. This is a data/runtime inspection milestone, not a playable port or a claim of pixel-identical output.

## Recovered readers

All integers/floats below are big-endian unless specified. Keys refer to the supplied iOS iPad3 BFC namespace. Exact source chunk/offset/size/SHA-256 records are in `models.json` and `animations.json`. Scripts verify source payload hashes before reading them; originals remain unchanged.

| Resource key | Layout and evidence |
|---|---|
| `8ac7bdd4` | u16 count 3,211, then signed16 vertex resource, signed16 vertex flags, 6 float bounds. PMMeshManager::init at `0x39a24`, load at `0x39844`. |
| `8111949b` | Nested u16 model/node/link counts with signed16 mesh indices. PMMeshRenderer::render at `0x3c108` selects the first link. Other link semantics are not assumed. |
| `e34742d0` | 730 raw int32 model resource IDs. FFModelManager::init at `0x21034`; load at `0x21250`. |
| Model resources | u16 model ID, u16 node count, then 48-byte nodes: signed8 parent, signed8 kind, u32 name/hash, signed16 material, 10 floats. FFModelData constructor at `0x20688`; FFModel::load at `0x1e5f0`. |
| `b0529f7a` | u16 material count 3,211; float alpha + signed16 image index. `-1` means no texture, not a missing resource. PMMaterial::load `0x38804`, getAlphaType `0x38818`. |
| `c9261c1f` | u16 image count 1,741, u16 texture count 1,188; image rows contain 8 signed16 fields; texture rows contain byte type + two int32 resource IDs. PMImageManager::init `0x311d4`. |
| `96f19831` | u16 track count 3,363; signed16 resource ID + signed16 frame count. PMAnimationManager::init `0x35220`. |
| `db5513ca` | u16 group count 203; each group has u16 node count followed by signed16 track indices. Same initializer. |

Mesh vertex files contain raw float32 values without a header. Flag 1 adds XYZ, flag 2 normals, flag 4 UV. Supplied meshes use flags 5 (2,736 meshes) or 1 (475 meshes). `PMMesh::loadFromFiles` at `0x38d20` receives no index resource for this table. `PMMesh::render` at `0x38ec0` uses GL_TRIANGLES / glDrawArrays; every three vertices form a triangle. Unindexed geometry and UVs are retained without welding or remeshing.

Node transforms are scale XYZ, quaternion XYZW, translation XYZ. Parent indices precede children; -1 is a root. `PMTransformFrame` constructor `0x3ec8c`, getMatrix `0x3f04c`, quaternion matrix `0x3f42c` and matrix multiply `0x47308` establish the column-matrix equivalent `T * R * S`. The helper named multiply writes its third matrix argument times its second; treating it as ordinary argument order would reverse hierarchy/animation transforms.

## Image indirection and packed pixels

A material's signed16 value is an **image index**, not a BFC resource ID. Image field 0 selects a texture row; its primary resource ID selects the payload. All images used by these models have zero crop offsets. Material `-1` renders without texturing. This indirection is essential: direct resource lookup produced plausible but incorrect assignments during investigation and is not used by the final exporter.

PMImageManager::load (`0x319dc`) dispatches on type low nibble:

- 11: RGB / GL_UNSIGNED_SHORT_5_6_5 (`0x8363`).
- 12: RGBA / GL_UNSIGNED_BYTE (`0x1401`).
- 13: RGBA / GL_UNSIGNED_SHORT_5_5_5_1 (`0x8034`).

`loadImageData` (`0x32688`) reads little-endian width/height followed by zlib, and uploads decompressed bytes without swapping pixel words. Packed words are therefore little-endian RGB565 or RGBA5551. Conversion expands 5/6-bit channels by bit replication; alpha is 0/255 for 5551. Source scanline order is retained in OpenGL uploads, matching the original loader. Type 3 is a runtime render target and is not decoded as an ordinary file texture.

The initial broad inventory intentionally retains its unresolved packed16 classification; the later model exporter resolves required textures using the reader's type metadata. It does not relabel every unknown payload or assert that all device variants have been revalidated.

## Animation format and playback

`PMAnimationTask::run` (`0x36c28`) reads ten initial floats, a u16 channel mask, then float arrays for each of ten channels whose mask bit is **clear**. A set bit selects its constant initial float. Each variable array has the frame count from `96f19831`. All 3,363 records were consumed exactly. Sample order matches scale/quaternion/translation above.

The initializer calls SetAnimationMSPerFrame with 33 (`0x35524`). The task's resampling divisor is also 33.0. With this native preview's 33 ms frame interval no resampling is required. PMAnimationManager::get uses floor/ceil frames modulo each track's length. Scale/translation interpolate linearly; PMQuaternion::lerpTo (`0x3f588`) uses shortest-path spherical interpolation, falling back to linear weights when `1-dot <= 1e-7` (literal at `0x3f75c`). The native implementation follows those branches.

These transforms are animation **deltas**, not replacements for the stored bind pose. FFTransformNode::update (`0x2beb4`) yields `parentWorld * bindLocal * animationDelta`. Matching animation initial floats against bind transforms is not a reliable way to associate groups and is not used.

`Ride::playHackyIdleAnim` (`0xb3174`) explicitly binds:

| Model | Idle group | Nodes | Longest track |
|---|---|---|---|
| 425 | 47 | 41 | 32 frames |
| 427 | 49 | 29 | 40 frames |
| 515 | 151 | 9 | 60 frames |
| 522 | 159 | 57 | 39 frames |

The dispatch reads AttractionStruct field `+0x50`. Attraction::loadInstance (`0x98c0a`) passes the same field to FFModel's virtual `load(int)` slot (`vtable +0x10`, target `0x1fd31`). Model identity is therefore supported by code as well as matching node counts. Calls at `0xb3392`, `0xb33ac`, `0xb33c6`, `0xb33e0` start the associated groups with time 0 and looping enabled. The model 409 pirate ship binding remains unverified and is not animated in the delivered viewer.

## Generated interface and Windows renderer

`extracted/models/ios/*.tpm` and `extracted/animations/ios/*.tpa` are project-defined text interchange formats, **not** original mobile formats. TPM1 stores node transforms and mesh XYZUV streams/material paths. TPA1 stores group ID, node count, frame interval and constant/variable channel arrays. Runtime catalogs use relative paths under `extracted/`. Native readers reject invalid hierarchy, traversal, missing textures, invalid lengths and non-finite values.

`ThemePark3D.exe` uses Win32/WGL, system OpenGL, GDI+ PNG decoding and Windows CRT. WGL setup follows [Microsoft's context creation contract](https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-wglcreatecontext). No mobile executable code is run. The orbit camera, text overlay and reference grid are diagnostic UI. Geometry, UVs, hierarchy, textures and the four animation tracks come from original data.

## Validation and limits

37 Python tests pass. Native hidden-window verification parses and renders every model, uploads all referenced textures, exercises camera/navigation, advances each of four animations through 60 sample times, checks changed node matrices, and tests pause/frame-step/bind-pose/navigation. The actual renderer was Intel UHD Graphics. PNG evidence shows model 409 from two directions and model 425 at 0 and 330 ms. These are framebuffer captures, not a mobile-runtime fidelity comparison.

No original gameplay build was run. Original draw ordering, alpha queues, dynamic material modifiers, render-target effects, lighting, camera projection, animated bounds and full scene composition have not been reproduced. Some water/particle details can differ. All 730 assemblies pass structural/render checks, but only representative screenshots were inspected visually. Only the four listed idle groups have code-verified bindings; exporting 203 groups is not proof of 203 correctly bound rides. Audio, guests, operating ride states, economics and save compatibility remain future work.
