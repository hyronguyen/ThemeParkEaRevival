# Windows toolchain

No native C/C++ compiler/SDK was initially found on PATH or inspected conventional install locations. Java and Python were available; managed C# compilation would not satisfy the native x64 target.

A portable **Zig 0.13.0 Windows x86_64** toolchain was downloaded from the official Zig site into `.tools/`, with no global installation. The pinned ZIP is 79,163,968 bytes; SHA-256 `d859994725ef9402381e557c60bb57497215682e355204d754ee3df75ee3c158` matched the [official release index](https://ziglang.org/download/index.json). Version selection is a reproducibility pin, not a claim that this is the newest release.

`python tools/bootstrap_toolchain.py` downloads and validates the archive. `powershell -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1` invokes `zig c++` for `x86_64-windows-gnu`, C++17, optimization O2, Windows GUI subsystem and system GDI+/Win32 libraries. `-ZigPath` can select a compatible local Zig executable. Cache directories stay inside the project.

Native PE verification checks AMD64/PE32+, GUI subsystem, zero CLR directory and import names. Imports are Windows GDI+/User32/Kernel32 and Universal CRT API sets; no Java, .NET, Python, Android runtime or bundled proprietary mobile library is required to run the EXE. The [Universal CRT is a Windows 10+ component](https://learn.microsoft.com/en-us/cpp/windows/universal-crt-deployment?view=msvc-170). Extracted asset files are still required beside the project; standalone means native execution without a mobile emulator, not artwork embedded in one distributable EXE.

The initial cold toolchain build emitted upstream libunwind export-attribute warnings. Subsequent project builds succeeded without source diagnostics. Sandbox temporary-file permissions blocked compilation and temporary-directory tests, so those were rerun with approved escalation. This is an execution-environment issue, not evidence of code correctness on every Windows installation.

Python 3.14 runs the standard-library collectors/tests. PNG generation used bundled Python 3.12 with Pillow 12.3.0. Capstone 5.0.9 was installed locally in `tools/_deps` for optional ARM disassembly. Neither Capstone nor Pillow is needed to launch the native preview after assets have been prepared.

The build script also produces `ThemePark3D.exe` from `src/themepark_3d.cpp`, linking system `opengl32` for WGL/OpenGL. Its verified imports remain Windows system libraries/API sets. No external graphics SDK or new global dependency was installed. `tools/verify_native.py ThemePark3D.exe` records its PE/import/hash evidence separately.

Optional local video inspection uses `imageio-ffmpeg==0.6.0`, installed in the ignored `tools/_deps` directory (bundled FFmpeg 7.1). It reads the user-supplied MP4 and samples reference frames. Neither FFmpeg nor this Python package is needed by the native application.
