# Asset inventory and formats

Extraction runs read-only against all three original archives. `asset_inventory.json` records package hash, original ZIP path, member size/CRC, extracted SHA-256/path, and BFC directory aliases, identifiers, flags, container offsets and sizes. `park_layout.json` adds recovered named-layout provenance. Raw files remain intact beside derivatives.

## Confirmed extraction results

- 835 file members inventoried across all packages; code/signing metadata are inventoried but not copied as game assets.
- Four iOS BFC indexes resolve to 7,965 unique physical payload ranges totaling 501,092,688 bytes. Every referenced range fits its chunk.
- 2,672 dimension-prefixed zlib texture payloads validated: 2,280 RGBA8 PNG previews and 392 unresolved packed16 textures retained raw.
- Five additional BFC payloads have PNG signatures; 5,288 remaining physical payloads are not given a speculative format name.
- The native catalog contains 774 decoded RGBA8 resources referenced by the iPad3 directory. Other device variants remain in the full inventory.
- Loose members include 224 standard PNGs, 146 Apple CgBI PNGs, three JPEGs, three MP3s, one WAVE, six OpenType CFF fonts, binary plists/nibs, XML/text, SQLite, four PAC descriptors and two DBPF packages. Counts describe signatures, not necessarily gameplay assets.

## BFC directory — verified

There is no BFC magic signature in these directory files. Identification depends on exact structure and executable reader evidence, not suffix alone. A two-byte **big-endian** count is followed by that many 13-byte entries:

| Byte offset in entry | Width | Interpretation |
|---|---:|---|
| 0 | 4 | Big-endian numeric resource identifier / CRC for named resources |
| 4 | 1 | Original flag, typically FE or FF; not treated as a universal asset-type tag |
| 5 | 3 | Big-endian byte offset inside numbered BFC chunk |
| 8 | 1 | Numbered BFC chunk identifier |
| 9 | 4 | Big-endian byte length |

Directory lengths equal `2 + count * 13`; all ranges are checked. `head_ipad3.bfc` contains 8,637 entries. Different identifiers/device indexes can alias the same physical bytes, hence fewer unique ranges than total entries. Numeric/generated names preserve actual identity; unknown original filenames are never invented.

PMFile::calcCRC uses the standard CRC32 table and low bytes of UTF16 code units, starting with all bits set and complementing the result. For recovered ASCII names, Python `zlib.crc32(name.encode('ascii'))` matches. UTF16 strings in the executable recover names such as `paths`, `slotData`, `themeSlots` and `dividerWalls`. Hash matching is corroborated by reader calls and coherent record lengths; arbitrary guessed names are not accepted as evidence.

## Textures

Recognized texture records begin with little-endian uint16 width/height followed by a complete zlib stream. Reader evidence: PMImageManager::loadImageData at unslid VA `0x32688`; it reads dimensions, swaps bytes, computes allocation and calls `_uncompress`. The extractor bounds decompression and requires exact end-of-stream and pixel length.

RGBA8 payloads contain `width * height * 4` bytes and decode with ordinary RGBA channel order. Original transparency and representative art were visually checked. The initial inventory leaves packed16 unresolved because size alone cannot identify packing. The later model exporter uses recovered image-table types to decode RGB565/RGBA5551 without guessing; see `model_formats.md`. Non-power-of-two texture handling is not generalized beyond validated supplied records; the original loader rounds allocation dimensions.

PNG previews are non-destructive derivatives, named with numeric resource id, chunk and offset. They are not proof of original material assignment or animation-frame ordering. CgBI PNG members remain byte-exact; this tool does not yet undo Apple's premultiplication/channel/Deflate differences.

The zlib stream interpretation follows the [zlib manual](https://www.zlib.net/manual.html) and [RFC 1950](https://datatracker.ietf.org/doc/html/rfc1950). BFC details were recovered from these local files, not a presumed public format.

## Layout, animation, audio and remaining containers

Named layout payloads are big-endian int32 arrays: count, then fixed-stride records. Exact counts/strides and verified fields are in `data_structures.md`. Six small named animation arrays export frame indices; they do not establish how to animate a model.

Extensionless `wv` files resemble raw audio samples, but sample encoding/rate/channel semantics require further PAC and reader work. They stay under unknown. PAC magic is `26 02 37 13`; the full format is unresolved. Android EASP packages begin `DBPF` version 2 and contain PNG bytes, but a complete DBPF entry extractor has not been implemented. Both original packages remain available raw.

The later `extract_models.py` and `extract_animations.py` readers now populate `models/` and `animations/`: 730 complete model assemblies, 299 model texture derivatives, 3,363 tracks in 203 groups. Four code-proven idle associations play in the native viewer. `model_formats.md`, `models.json` and `animations.json` document these readers and source provenance. The initial broad inventory/raw file paths are preserved, so an `unknown/` raw path is not a current claim that the payload remains unidentified. Named frame-index arrays remain under `level_data/.../named/`.
