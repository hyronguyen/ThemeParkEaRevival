param([string]$ZigPath = '')
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
if (-not $ZigPath) { $ZigPath = Join-Path $projectRoot '.tools/zig-windows-x86_64-0.13.0/zig.exe' }
if (-not (Test-Path -LiteralPath $ZigPath)) { throw 'Missing Zig 0.13.0. Run python tools/bootstrap_toolchain.py or supply -ZigPath.' }
$buildRoot = Join-Path $projectRoot 'build'
New-Item -ItemType Directory -Path $buildRoot -Force | Out-Null
$env:ZIG_GLOBAL_CACHE_DIR = Join-Path $projectRoot '.tools/zig-cache'
$env:ZIG_LOCAL_CACHE_DIR = Join-Path $buildRoot 'zig-cache'
& $ZigPath c++ (Join-Path $projectRoot 'src/themepark_preview.cpp') -target x86_64-windows-gnu -std=c++17 -O2 -Wall -Wextra -municode '-Wl,--subsystem,windows' -lgdiplus -lgdi32 -luser32 -lkernel32 -lole32 -o (Join-Path $buildRoot 'ThemeParkPreview.exe')
if ($LASTEXITCODE -ne 0) { throw "Native build failed with exit code $LASTEXITCODE" }
Write-Output (Join-Path $buildRoot 'ThemeParkPreview.exe')
& (Join-Path $PSScriptRoot 'build-game.ps1') -OutputName 'ThemePark3D.exe' -ZigPath $ZigPath
if ($LASTEXITCODE -ne 0) { throw "3D native build failed with exit code $LASTEXITCODE" }
Write-Output (Join-Path $buildRoot 'ThemePark3D.exe')
$gameOutput = Join-Path $buildRoot 'ThemeParkGame.exe'
try { Copy-Item -LiteralPath (Join-Path $buildRoot 'ThemePark3D.exe') -Destination $gameOutput -Force }
catch [System.IO.IOException] {
    $gameOutput = Join-Path $buildRoot 'ThemeParkGame-update.exe'
    Copy-Item -LiteralPath (Join-Path $buildRoot 'ThemePark3D.exe') -Destination $gameOutput -Force
    Write-Output 'Existing game is in use; wrote a separate update without interrupting it.'
}
Write-Output $gameOutput
