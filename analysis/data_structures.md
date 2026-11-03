# Recovered data structures

Only the fields below are supported by exact binary layouts and/or inspected reader instructions. These are not reconstructed C++ headers or a complete game schema. Addresses refer to the supplied iOS executable, unslid virtual addresses.

## Static layout arrays

Each resource starts with a big-endian int32 record count, followed by `count * stride` big-endian int32 values. All byte lengths exactly match that formula. `tools/export_park_layout.py` retains raw bytes and exports rows without inventing unnamed fields.

| Original name | Records | Int32 values per record | Interpretation |
|---|---:|---:|---|
| paths | 5,604 | 2 | Verified tile x,y |
| slotData | 247 | 10 | First five: id,x,y,width,height; remaining five retained |
| themeSlots | 20 | 9 | Named table, detailed fields unresolved |
| guidelines | 20 | 5 | x, y, width, height, territory ID |
| entrances_b | 62 | 4 | Named table, detailed fields unresolved |
| dividerWalls | 329 | 7 | record ID, model ID, x, y, width, height, Z angle in degrees |
| decorations | 328 | 4 | Named table, detailed fields unresolved |
| themedDecData | 529 | 7 | Named table, detailed fields unresolved |

`paths` CRC id is `8bba8611`, BFC81 byte offset 1,266,054, length 44,836. GameMap::loadRideSlots (entry `0x95074`) reads pairs at `0x95842` onward and passes them to Path::AddPath (`0x9585e` call site) and MiniMap::addStaticPath (`0x95876`). Coordinates range x=1–179, y=1–144. These are original layout rows, not 5,604 unique walkable tiles or a proven full navigation graph; duplicates and ordering are preserved.

`slotData` CRC id is `d29e174a`, BFC81 offset 1,312,428, length 9,884. RideSlot::read (`0xb7c68`) copies ten values. The first seven go to object offsets `0x1c` through `0x34`; field 7 goes to `0x10`, field 8 to `0x14`, and field 9 controls a branch. The stored `0x10` value is passed to registerRideSlotWithTerritory. Do not label other unknown values from plausible ranges alone.

RideSlot::getSlotCenter3D (`0xb8bb4`) reads fields at `0x20,0x24,0x28,0x2c`; it computes `(x + width/2) * 32` and `-(y + height/2) * 32` with integer half-size operations and zero on the third axis. Combined with the record reader, this establishes record fields 1–4 as position and dimensions. The preview displays their original footprint in a top-down diagnostic projection, not the original camera/materials. Field 0 is used as the slot identifier; matching original ownership/unlock state has not been recovered.

Exact names, hashes, chunk locations, row arrays and raw hashes for every exported table are in `park_layout.json`. Physical resource aliases across device directories remain in `asset_inventory.json`. Static 3D scene transforms, adjacency and default material evidence are documented in `scene_formats.md`. Grass and dividers use floating half-sizes, unlike the integer half-size operations described above for RideSlot centers.

## Small animation index arrays

`scaffHammerAnim` has 7 indices, `scaffBrushAnim` 5, `scaffWalkAnim` 5, `sharkIdle` 12, `bearIdle` 12 and `walkAnim2` 8. Their count-prefixed big-endian integer structure is exact. Names and indices alone do not establish timing, interpolation, frame resources or skeleton/transform semantics.

## Other original structures

Symbols establish PMArray, PMHandle, PMMesh, PMTexture, MapData, WorldInstance, AttractionStruct, RideStruct, SOAPEntity and LoadData types. Mesh vertex layouts, model hierarchy and material associations are now recovered in `model_formats.md`. XML field names in Android include build time/cost, earnings, guest capacity, unlock levels and progression values, but field names do not supply constants or server data. No complete gameplay object layout or serializer is claimed.

## Independent preview save schema

The preview writes `%LOCALAPPDATA%/ThemeParkPreservation/preview.json`; smoke tests use `analysis/runtime_test/smoke-preview.json`. This is **not an original game save** and contains no game economy, entitlement, account or server state.

Schema version 1 has numeric keys in this emitted order: `schema_version`, `asset`, `mode`, `selected_record`, `map_zoom`, `map_x`, `map_y`, `art_zoom`, `art_x`, `art_y`. `asset` indexes the supplied catalog; `mode` is 0 for layout or 1 for artwork. Selection is -1 for none, 0–5603 for original path rows, or 5604 plus the slot row index. Camera offsets are screen pixels. Saves are tied to this catalog/layout export and do not promise compatibility with a changed asset order.

The bounded reader accepts the application's emitted numeric schema and whitespace, not arbitrary reordered JSON. It rejects unknown versions, missing/malformed fields, trailing content, invalid selections and non-finite/out-of-range camera values. Writes use a temporary file and Windows replacement. S saves explicitly; L loads explicitly. Original archive files are never save targets.
