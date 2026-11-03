param([string]$OutputName = 'ThemeParkGame-next.exe', [string]$ZigPath = '')
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
if (-not $ZigPath) { $ZigPath = Join-Path $projectRoot '.tools/zig-windows-x86_64-0.13.0/zig.exe' }
if ([IO.Path]::GetFileName($OutputName) -ne $OutputName -or -not $OutputName.EndsWith('.exe')) { throw 'Output must be an executable filename' }
$env:ZIG_GLOBAL_CACHE_DIR = Join-Path $projectRoot '.tools/zig-cache'
$env:ZIG_LOCAL_CACHE_DIR = Join-Path $projectRoot 'build/zig-cache'
New-Item -ItemType Directory -Path (Join-Path $projectRoot 'build') -Force | Out-Null
$resourceFile = Join-Path $projectRoot 'build/game-icon.rc'
$iconPath = (Join-Path $projectRoot 'extracted/runtime/themepark.ico').Replace('\','/')
$iconArgs = @()
if (Test-Path -LiteralPath $iconPath) {
    Set-Content -LiteralPath $resourceFile -Value ('1 ICON "' + $iconPath + '"') -Encoding ASCII
    $res = Join-Path $projectRoot 'build/game-icon.res'
    & $ZigPath rc /fo $res $resourceFile
    if ($LASTEXITCODE -ne 0) { throw 'Icon resource build failed' }
    $iconArgs += $res
}
& $ZigPath c++ (Join-Path $projectRoot 'src/themepark_3d.cpp') @iconArgs -target x86_64-windows-gnu -std=c++17 -O2 -Wall -Wextra -municode '-Wl,--subsystem,windows' -lopengl32 -lgdiplus -lgdi32 -luser32 -lkernel32 -lole32 -lwinmm -o (Join-Path (Join-Path $projectRoot 'build') $OutputName)
if ($LASTEXITCODE -ne 0) { throw 'Game build failed' }
Write-Output (Join-Path (Join-Path $projectRoot 'build') $OutputName)
