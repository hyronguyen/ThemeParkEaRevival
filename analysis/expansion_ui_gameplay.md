# Original-art UI and park expansion — 2026-10-26

## Implemented

- The castle screenshot supplied as reference matches original thumbnail texture
  `00000250`. It is pre-rendered artwork, not a capture of the previous executable.
  The actual castle model 404 already includes its water channel, bridge and shrubs.
  Translucent plot highlights previously wrote depth above that floor and hid it.
  Highlights now blend without writing depth.
  Highlight drawing also restores the previous alpha-blending state, preventing
  black texture margins on shops and water effects.
- Built plots use original Funland wall models 669/670/671 for 4/6/16-tile plots.
  Models 650–653 are vacant-slot fences and no longer surround built attractions.
  The mapping is in GameMap's original `funlandWalls_values` table. Default gate
  models 216–219 are its four gate levels. Press G on a selected attraction to
  cycle these levels; each attraction's level persists in the local save.
- Gates use the front edge (lower-left in the default isometric camera).
  Arrivals have a 50% admission chance if open and below capacity. There is no
  ride queue. Admitted guests start immediately and have independent visit timers;
  full/closed rides turn guests away. This follows the user's explicit rule.
- Castle guests remain visible entering, bouncing and leaving. Inflatable nodes
  5–12 also flex while occupied. This elastic motion and passenger timing are
  provisional, not a recovered original castle animation binding.
- Office model 273 plus original base 274 is placed at the first territory's
  `themeSlots` row 0 footprint `(87,129,8,6)`, centered at `(91,132)`. New demo
  layouts put Log Flume behind it. Existing saved placements are preserved.
- All 328 `decorations` rows are staged unchanged, rendering their original
  model IDs and map positions. FixedDecoration::init's coordinate conversion is
  preserved; the current center assumes a one-tile object footprint because the
  server-side decoration definitions are unavailable. The separate themedDecData
  table was located but is not fully reconstructed.
- Original park gate 633, road 634 and bus 295 form the front entrance. A short
  walkway connects existing paths to a bus stop. New path nodes are appended,
  preserving old saved path indices. Visitors leave the bus, visit attractions,
  return to the stop, board, and arrive again on a later bus cycle. Placement of
  the road/stop, 40-second cycle and passenger scheduling are provisional.
- Ten guest appearances use the original `guestUVs_values` offsets: eight at
  U=0..0.875/V=0 and two at U=0,0.125/V=0.25. They reuse original walking/jump
  geometry. This restores texture variants, not every original body/behavior set.

## Original UI assets and build catalog

`park_original_ui.h` composes original iPad3 frame, gradient, button, icon and
thumbnail artwork, including atlases 0xb6 and 0xf2/0xf4. Transparent canvas margins
are excluded and panel borders use nine-slice rendering. The supplied
ITCKabelStd-Bold.otf is loaded privately and rasterized with antialiasing. No new
illustrated UI assets were generated. Desktop layout, hit testing and menu logic
are new native code; this is not a claim that the original UI implementation,
economy, rank, prices, missions or timers have been ported.

The catalog contains 163 original assemblies: 96 under RIDES and 67 under SHOPS.
All have a fitting connected plot in the recovered map. The first five indices
remain unchanged for save compatibility. Separate passenger poses, vehicles and
construction markers are excluded. Category and display-name assignments are a
visual working catalog; server catalog completeness and exact names are not
verified. Unknown names retain model IDs. Original thumbnails are used where
identified; otherwise the menu renders the original 3D model directly. Footprints
allow up to half a tile of model overhang; these placement dimensions are provisional.
Shops currently use the same timed visit system, without inventory or income.

Controls: B or the bottom-left build icon opens the catalog; select RIDES/SHOPS,
an item, BUILD, then click a highlighted plot. Right arrow pages through items.
F1 or the gear toggles debug controls. O shows the office/entrance area. G changes
the selected gate level. S/L save/load, Space pauses, the upper-right arrow changes
simulation speed. Music controls remain in the debug panel.

## Saves and limits

`playtest-v4.txt` / TPPLAY4 adds per-attraction gate levels and bus visitor states.
The reader accepts TPPLAY2/3, releases their old queues and migrates boarded
visitors to independent timers. When v4 is absent the previous v3/v2 file is
imported without overwriting it. Existing attraction indices and path indices
remain stable. The entrance becomes the new bus stop.

Remaining fidelity work: original castle/other ride operating animations, visible
passengers on rides other than the castle, full themed materials/effects (notably
water transparency), exact server catalog/placement metadata and economy, original
bus animation timing, full UI behavior and themed decoration assembly.

## Reproduction and verification

Run `tools/prepare_expansion.py` using Python with Pillow after normal asset
extraction, then `scripts/build.ps1` and `scripts/test-playtest.ps1`.
The asset staging tool writes catalogs and alpha bounds, never modifies originals.

Automated checks cover source decoration equality, catalog identities/thumbnail
references, UI bounds, 50% admission over 1,000 arrivals, absence of ride queues,
capacity/conservation, bus arrival, legacy save migration and deterministic
save/resume. The native test checks all 163 catalog entries fit, operates SHOPS
and BUILD through mouse messages, places a shop, upgrades its gate and reloads
the saved upgrade. Captures under runtime_test include original-ui-castle.png,
original-ui-rides.png, original-ui-shops.png, original-shop-built.png and
original-entrance-office.png.

Final results: 47 Python tests pass; independent C++ simulation/admission/bus/
migration checks pass; the native playtest records 40 guests, six placements and
149 completed visits over 328.65 simulated seconds. All 163 entries fit a plot,
and shop placement plus saved gate upgrades pass. The full 730-model workbench
regression also passes. Screenshots were visually inspected after the alpha fix.
The final AMD64 native game is 812,544 bytes, SHA-256
`c4d7c8c98bc7f72c4fcf5ab8a37577783a92e3d1149660abb6ee12770f4a345f`.
