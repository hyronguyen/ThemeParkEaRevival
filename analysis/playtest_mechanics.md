> Updated 2026-10-26: see [expansion_ui_gameplay.md](expansion_ui_gameplay.md) for the
> current no-queue admission rule, 163-item original-art catalog, bus lifecycle,
> ten guest variants, office/decorations, built fences and TPPLAY4 saves.
> The earlier milestone descriptions below retain their historical parameters.

# Offline NPC / ride playtest

On 2026-10-25 the user authorized provisional parameters to get a playable
build, with provisional behavior documented. `build/ThemeParkGame.exe` now
implements that scope. It is a native Windows sandbox using the recovered
park, models, textures and four verified idle animations. It is not a claim
that the original economy, AI, operating ride animation or UI has been ported.

## What can be played

- A starting demonstration park has four rides and 32 visitors.
- Build five original ride assemblies on compatible original ride-slot footprints.
- Green plots are empty, large enough and connected to the local entrance.
- Rotate placement, preview the model, find a suitable plot, or cancel building.
- Visitors choose reachable open rides, follow orthogonal path edges, queue,
  board a capacity-limited batch, finish, and choose another ride.
- After three rides, visitors walk back to the entrance before starting a new visit.
- Close/reopen a ride. Closing releases its waiting queue; boarded guests finish.
- Pause, run at 1x/2x/4x, add visitors up to 96, and inspect/follow a visitor.
- Save/load park progress, queues, active batches, guest routes and random state.

The window uses a cream/gold/teal build panel, contextual ride panel, counters,
progress bar, placement preview and status feedback. This is a new desktop
interaction layer inspired by the reference recording's palette and flows;
it does not reproduce the original UI artwork/layout or claim pixel parity.
The five display names are descriptive playtest labels, not recovered catalog names.

## Preserved assets and evidence

Original terrain/dividers/paths and coordinate transforms follow
`scene_formats.md`. Build slots are the original `slotData` footprints.
Models 425, 427, 515 and 522 retain their mesh scale and verified idle groups
47, 49, 151 and 159. Model bounds determine minimum slot dimensions; models
are centered in the selected footprint. Original attraction entrance/orientation
metadata is still unresolved, so this centering and gate selection are provisional.

NPC geometry uses original models 0–7 with original texture resource 0.
`walkAnim2` is the recovered count-prefixed array `[0,1,2,3,4,5,6,7]`.
`AnimSet::init` (0x67518) reads the count and integer entries;
`AnimSetManager::createModelSet` (0x67e54) passes entries to `PeopleModel::load`,
which calls `FFModel::load` at 0xc848e/0xc84e6. Native captures confirm a
textured guest in walking poses. The playtest cycles these models while moving;
original interpolation, timing and UV appearance variants have not been ported.
The focused evidence is local in `analysis/ios/anim_set_init.txt`,
`anim_sets.txt`, `person_bindings.txt`, `people_models.txt` and `people_render.txt`.

## Provisional rules

| Rule | Current value / behavior |
|---|---|
| Simulation | Fixed 50 ms steps, deterministic random generator |
| Navigation | BFS shortest route on the 5,585 unique original path coordinates |
| Walking | 1.4 tiles/second; eight-model cycle at 100 ms/model |
| Population | 32 initial; add 8; maximum 96 |
| Destination | Random reachable open ride with fewer than 24 waiting guests |
| Capacity | 6 guests for the first four rides; 4 for Bouncy Castle |
| Dispatch | At capacity, or after 2 seconds with at least one waiting guest |
| Ride durations | 12 / 15 / 18 / 21 / 16 seconds in card order |
| Queue | FIFO, maximum 24, compact visual offsets inside the gate tile |
| Exit | Gate tile; after 3 rides walk back to the local entrance |
| Placement | Original empty slot, bounding footprint fit, reachable adjacent gate |
| Construction | Immediate, no build price or premium currency |
| Starting area | Locally selected near tile 90,90; not original ownership or spawning |

The static graph does not reproduce original A-star weights, dynamic queues,
territory unlocks or ride entrance rules. It never routes diagonally or across
grass. Disconnected plots cannot be built. Ride turns do not resolve original
entrance metadata; a nearest adjacent path tile is selected instead.

## Important visual/gameplay limits

Guests are visible walking and queuing. Bouncy Castle now shows guests entering,
bouncing and leaving, using original attachment locations and poses with
provisional timing. Other rides still hide boarded guests. Four ride models play
verified idle animations, not recovered operating animations. One guest appearance
is used. Three original music tracks play with toggle/track/volume controls;
ride sound effects, staff, needs, happiness, shops, maintenance, money and quests
remain unfinished. See `castle_audio_rendering.md` for evidence and checks.

## Save and validation

`%LOCALAPPDATA%/ThemeParkPreservation/playtest-v3.txt` uses independent schema
`TPPLAY3`. The reader also accepts TPPLAY2; an old v2 file is imported when no v3 file exists and remains unchanged. S / Save writes; L / Load restores; closing the window saves too.
Writes go to a temporary file, flush, then replace with `MoveFileExW`.
Self-tests write only `analysis/runtime_test/playtest-save.txt`.

The save contains simulation time/random state, completed visits, spawn index,
ride type/plot/rotation/open state, batch timer, FIFO queue and boarded IDs;
each guest stores state, target, current tile, route/progress, visit count,
waiting time and walking time. No original account/server state is used.
Pause/speed and camera are session controls, not persistent progression.
Indices are tied to the current layout and four-type catalog; changing either
requires a schema migration/version bump. This is not an original game save.

Parsing is bounded. Validation rejects invalid indices, nonfinite values,
oversized/unreachable placements, duplicate plots, duplicate queue membership,
capacity violations, invalid route edges, malformed/truncated/trailing input.
Loads validate a temporary state and replace the live state only on success.
Save/load preserves deterministic continuation; failures leave the live park intact.

## Verification

`tests/park_simulation_test.cpp` exercises placement, disconnected paths,
capacity/state invariants, close/reopen, conservation of 24 visitors, 550
simulated seconds, deterministic save/resume, pause and malformed-save rollback.
The successful run completed 1,043 visits.

`ThemeParkGame.exe --self-test` uses a hidden real HWND/WGL context and the
actual input handlers. The final run simulated 310 seconds, completed 193
visits, added a fourth ride by mouse, tested pause/speed, closure, save/load,
visitor inspection, and added eight visitors (40 total). Captures are
`playtest-start.png`, `playtest-running.png`, `playtest-build.png` and
`playtest-guest.png` under `analysis/runtime_test/`.

These are bounded local tests, not evidence of original-game equivalence or
a long-running production simulation.
