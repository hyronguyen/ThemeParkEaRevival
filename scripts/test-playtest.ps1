param([string]$ZigPath = '', [string]$GameExecutable = 'ThemeParkGame.exe')
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
if (-not $ZigPath) { $ZigPath = Join-Path $projectRoot '.tools/zig-windows-x86_64-0.13.0/zig.exe' }
$env:ZIG_GLOBAL_CACHE_DIR = Join-Path $projectRoot '.tools/zig-cache'
$env:ZIG_LOCAL_CACHE_DIR = Join-Path $projectRoot 'build/zig-cache'
$testExe = Join-Path $projectRoot 'build/park_simulation_test.exe'
& $ZigPath c++ (Join-Path $projectRoot 'tests/park_simulation_test.cpp') -target x86_64-windows-gnu -std=c++17 -O2 -o $testExe
if ($LASTEXITCODE -ne 0) { throw 'Simulation test build failed' }
& $testExe
if ($LASTEXITCODE -ne 0) { throw 'Simulation tests failed' }
$app = Start-Process -FilePath (Join-Path (Join-Path $projectRoot 'build') $GameExecutable) -ArgumentList '--self-test' -WindowStyle Hidden -PassThru -Wait
if ($app.ExitCode -ne 0) { throw 'Native playtest failed; see analysis/runtime_test/model_error.txt' }
Get-Content -LiteralPath (Join-Path $projectRoot 'analysis/runtime_test/playtest_test.json')
