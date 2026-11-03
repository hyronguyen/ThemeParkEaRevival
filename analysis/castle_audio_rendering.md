> Updated 2026-10-26: see [expansion_ui_gameplay.md](expansion_ui_gameplay.md) for the
> current no-queue admission rule, 163-item original-art catalog, bus lifecycle,
> ten guest variants, office/decorations, built fences and TPPLAY4 saves.
> The earlier milestone descriptions below retain their historical parameters.

# Castle passengers, original music and rendering — 2026-10-26

The playable build now offers five rides. Bouncy Castle uses original model 404,
walking models 0–7 and jumping poses 405–408. Four recovered attachment positions
place passengers inside the castle. Entry occupies the first 16% of a ride,
bouncing the middle 68%, and exit the final 16%. Capacity four, 16-second duration,
paths, pose scheduling and bounce timing are explicitly provisional. Other rides
still hide boarded guests; original operating animation is not yet recovered.

## Invisible geometry correction

Original `FFModel::_Construct` at 0x1cd24 constructs CRCs for names
`vis_loc01` through `vis_loc34` (UTF-16 strings at file offset 0x4cfb00).
`FFModel::load` compares node hashes against this list at 0x1eaf8–0x1eb1e;
a match branches to 0x1ebe6, bypassing mesh initialization.
`Ride::playRidingAnim` at 0xb2538 locates these nodes and parents passengers
to them. Evidence: `ios/model_locators.txt`, `ios/model_hide_locators.txt`,
`ios/ride_passengers.txt`.

The exporter identifies 727 marker nodes across the model catalog. Rendering now
retains their transforms but suppresses their meshes. Castle nodes 1–4 are
locators 4–1; Water Slides nodes 1–5 are locators 5–1. This removes unintended
visible blocks; raw model data and provenance remain unchanged.

Original default fence models 650–653 surround occupied 4/6/8/16-tile square
plots. The mapping follows RideSlot's theme-0 table at 0x4cbc10. These are
default fences, not a reconstruction of every themed border in the reference.
Optional 4x MSAA, trilinear mipmaps and up to 4x anisotropy are desktop quality
upgrades. Water materials, effects, themed decorations and transparent ordering
still differ from the original game.

## Audio and saves

`tools/prepare_game_audio.py` stages the three original `audio_music_track_*.mp3`
files with SHA-256 verification and provenance. Windows WinMM/MCI plays these
unchanged files. Music, Next and Volume controls appear in the build panel;
backgrounding the application pauses music. Ride sound effects are not restored.

Progress uses `playtest-v3.txt` / `TPPLAY3`. The reader accepts TPPLAY2 and imports
the old file when no v3 save exists, leaving the old file intact. Castle appends
type index 4, preserving earlier type indices. Audio preferences are session-only.

## Verification

- 44 Python tests pass, including real locator and audio provenance checks.
- Native gameplay test: 40 guests, five rides, 200 completed visits in 310.4
  simulated seconds; placement, pause/speed, closure and save/restore pass.
- Castle test: three visible riders, changing bounce positions, all three MP3s
  successfully played at volume zero; 4x MSAA enabled on this host. This verifies
  playback commands, not a listening assessment of the speaker output.
- Full 730-model native regression and independent C++ simulation tests pass.
- Game and workbench PE checks pass: AMD64, no CLR, Windows system imports,
  including WinMM. Binary size 766,464 bytes; SHA-256
  `8ecba3b500b5028c28ff0cf5afb64ca86432ff732c352df64112c05403004bde`.
- Visually checked `runtime_test/castle-playing-0.png`, `castle-playing-1.png`
  and `water-slides-corrected.png`.

Reproduce assets with `tools/export_model_locators.py` and
`tools/prepare_game_audio.py`, then `scripts/build.ps1` and
`scripts/test-playtest.ps1`. Run the full model regression using
`build/ThemePark3D.exe --self-test`.
