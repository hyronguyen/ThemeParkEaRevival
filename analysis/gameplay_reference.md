# User-supplied gameplay reference

Local source: `resource/Theme Park - iPad 2 - HD Gameplay Trailer - TouchGameplay (360p, h264).mp4`.
The file was supplied by the user, not downloaded by the investigation.
SHA-256 before and after extraction:
`153ebe01b213c9c5f5dded07f12d002e53b67ec512f55209697cbe2eb7abafce`.

Container metadata: title “Theme Park - iPad 2 - HD Gameplay Trailer”, artist
“TouchGameplay”; duration 1,019.69 seconds (16:59.69), H.264 640x360 at
29.97 fps, stereo AAC 44.1 kHz. These labels do not establish the original
game build, recording date or equality with the supplied IPA version.

## Initial visual observations

Seventeen frames sampled at 00:30, 01:30, …, 16:30 provide a sparse overview.
They are not a complete viewing or validation of simulation timing.

| Reference time | Visible evidence | Use for the port |
|---|---|---|
| 00:30 | Construction scaffold, confirmation mark, grass and paved paths | Construction presentation and slot surroundings |
| 02:30 | Open food shop, tables and tutorial prompt | Shop/UI reference; no inferred economy rules |
| 04:30–05:30 | Inflatable castle and contextual ride controls | Future model/animation identification |
| 08:30 | Pirate Boat build card | Candidate identity for matching original attraction definitions |
| 12:30–13:30 | Wheel attraction, guests, surrounding paths | Future animation, scale and camera comparison |
| 14:30 | Castle Base card | Office/theme definition reference |
| 15:30 | Canal, bridge/barrier, paved corners, rocks and trees | Direct visual corroboration of recovered dividers and path tiles |
| 16:30 | Jupiter Jet Jumper unlock card | Catalog/UI reference, not entitlement behavior |

The static native scene reproduces the recognizable grass, paved path corners
and canal/bridge geometry seen at 15:30. It still lacks rocks/trees, owned rides,
guests, original UI and the original camera. All recovered territories are
shown at once, so an overview is not expected to match this player's view.
No numeric camera calibration or pixel-exact equivalence is claimed.

## Reproduce locally

Install the optional pinned helper in the ignored local tools directory:
`python -m pip install --target tools/_deps imageio-ffmpeg==0.6.0`.
Run `python tools/inspect_gameplay_video.py`; use Python with Pillow to also
produce the contact sheet. The helper samples local video only, checks that
its SHA-256 is unchanged, and writes requested timestamps, metadata and
frame paths to `analysis/previews/gameplay/reference.json`.

`frame-0930s.png` is the 15:30 canal reference. `contact-sheet.jpg` contains
all 17 samples. Source video and image derivatives stay in ignored folders.
FFmpeg and Python are analysis tools, not dependencies of the native EXE.
