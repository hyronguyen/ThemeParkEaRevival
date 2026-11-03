# Static park assembly evidence

Implemented in `tools/export_park_scene.py` and the native OpenGL workbench.
This is the original static layout with default grass/path materials, not a
recovered player's park. All territories are visible; ownership, attractions,
decorations and progression are not inferred.

## Coordinates and resources

All addresses below refer to the supplied iOS ARMv7 executable. Named tables
are count-prefixed big-endian int32 arrays. The scene exporter checks the
archive SHA-256, extracted table hashes and decoded rows before assembly.
`analysis/park_scene.json` retains table provenance and instance source rows.

| Layer | Resource and interpretation | Placement |
|---|---|---|
| Grass: 20 | `guidelines`: x,y,width,height,territory ID; model 392 | `(x+w/2)*32, -(y+h/2)*32, -5` |
| Dividers: 329 | `dividerWalls`: record ID,model,x,y,w,h,angle degrees | Same footprint center, z=0, Z rotation from row |
| Paths: 5,585 unique | `paths`: x,y; 5,604 rows, 19 duplicates | `(x+0.5)*32, -(y+0.5)*32, -1` |

`GameMap::loadRideSlots` at 0x95074 reads guideline bounds around 0x9531e
and divider rows around 0x958fa. Wall model load is at 0x95994, position at
0x95a08 and rotation at 0x95a3c. Half-width/height use floating-point division.
`PMMatrix44::toRotationXYZ` (0x2cbb8) produces the usual positive column-vector
Z rotation after its angle/sign conventions are composed.

`ThemeSlot::createDefaultGrass` (0xc0a18) loads model 392 at 0xc0ada. Its
unscaled plane spans 1,152 world units (36 tiles). All 20 guideline footprints
are 36x36; other dimensions are rejected. `ThemeSlot::_Construct` (0xbf870)
initializes `GRASS_Z_HEIGHT` at 0x541ad4 to float -5.0.

## Path shape and material

`Path::AddPath` sets adjacency bits left=1, up=2, right=4, down=8. Only the
static path table participates here; dynamic attraction/queue connections
remain unresolved. Duplicate coordinates become one rendered instance with
all source row numbers retained. This deduplication is a preview choice,
not a claim about original duplicate-object behavior.

`Path::setPathTypeFromFlag` (0xacef4) maps masks 0..15 to types:
`0,5,3,10,6,2,9,12,4,8,1,13,7,11,14,15`.

`Path::pathModels` at VA 0x4cba70 contains 16 little-endian model IDs:
`306,307,308,309,310,311,312,313,314,315,316,319,317,320,318,321`.
The exporter verifies these bytes directly in the archive executable.
`Path::createMeshForMutable` (0xad8b0) sets the tile-center transform;
`ThemeSlot::generatePath` (0xc039c) places its merged path batch at z=-1.

For a slot without an active theme, `ThemeSlot::getPathTexture(-1)` loads
model 306 and returns its first material (material 354, alpha 1, texture
resource 18). `Path::setTexture` (0xadb18) installs that material on the
path mesh. All path instances therefore override texture with
`models/textures/00000012.png`; using each template's own material is wrong.

## Native rendering and limits

`PMImageManager::loadFor3D` at 0x32f84 overrides the 2D texture settings:
S/T wrap GL_REPEAT at 0x32fb4/0x32fc4, minification
GL_LINEAR_MIPMAP_NEAREST at 0x32fd4, magnification GL_LINEAR at 0x32fe4,
and automatic mipmaps at 0x32ff4. The native viewer uses these wrap/filter
settings and generates a CPU box-filter mip chain for portable GL 1.1.
This can differ slightly from the original driver's mipmap generation.

22 model/material combinations are cached in OpenGL display lists for 5,934
instances. Lists are invalidated before textures when changing models, and
rebuilt after changing wireframe state. P switches model/park; 1/2/3 switch
grass/divider/path layers. The orbit camera and HUD remain diagnostic.

No decorations are placed yet: their definitions determine footprint and
rotation beyond the four-field static table. Ride-slot rectangles alone do
not prove which owned attraction or operating state belongs in each slot.
Transparent sorting, effects, original camera behavior and gameplay remain
unfinished. See `gameplay_reference.md` for the user-supplied video baseline.

Focused disassembly is retained locally under `analysis/ios/scene_*.txt`,
`path_*.txt` and `ground_path.txt`; proprietary derivatives are ignored.
