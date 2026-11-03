param([switch]$Apply)
$ErrorActionPreference = 'Stop'
$projectRoot = [IO.Path]::GetFullPath((Split-Path -Parent $PSScriptRoot))
$rootPrefix = $projectRoot.TrimEnd('\') + '\'
$batch = [DateTime]::UtcNow.ToString('yyyyMMdd-HHmmss')
$archiveRoot = Join-Path $projectRoot "analysis/archive/$batch"
$candidates = @()
$groups = @(
    @{ Directory='analysis/runtime_test'; Patterns=@('catalog-*.png'); Target='model-captures' },
    @{ Directory='analysis/previews'; Patterns=@('atlas-*.png','ui-sheet-*.jpg','models-*.jpg','ride-sheet.jpg','probe_*.jpg'); Target='contact-sheets' }
)
foreach($group in $groups) {
    foreach($pattern in $group.Patterns) {
        foreach($file in Get-ChildItem -LiteralPath (Join-Path $projectRoot $group.Directory) -Filter $pattern -File) {
            $candidates += @{ Source=$file.FullName; Destination=(Join-Path $archiveRoot ($group.Target + '/' + $file.Name)) }
        }
    }
}
$oldBuild = Join-Path $projectRoot 'build/ThemeParkGame-update.exe'
$mainBuild = Join-Path $projectRoot 'build/ThemeParkGame.exe'
if((Test-Path -LiteralPath $oldBuild) -and (Test-Path -LiteralPath $mainBuild)) {
    if((Get-Item -LiteralPath $oldBuild).LastWriteTimeUtc -lt (Get-Item -LiteralPath $mainBuild).LastWriteTimeUtc) {
        $candidates += @{ Source=$oldBuild; Destination=(Join-Path $archiveRoot 'old-builds/ThemeParkGame-update.exe') }
    }
}
$manifest = @()
foreach($item in $candidates) {
    $source = [IO.Path]::GetFullPath($item.Source)
    $destination = [IO.Path]::GetFullPath($item.Destination)
    if(-not $source.StartsWith($rootPrefix,[StringComparison]::OrdinalIgnoreCase) -or
       -not $destination.StartsWith($rootPrefix,[StringComparison]::OrdinalIgnoreCase)) { throw 'Archive path leaves project root.' }
    if(Test-Path -LiteralPath $destination) { throw "Archive destination already exists: $destination" }
    $file = Get-Item -LiteralPath $source
    $manifest += [ordered]@{ source=$source.Substring($rootPrefix.Length); destination=$destination.Substring($rootPrefix.Length); bytes=$file.Length; sha256=(Get-FileHash -LiteralPath $source -Algorithm SHA256).Hash.ToLowerInvariant() }
}
if(-not $Apply) {
    $totalBytes = ($manifest | ForEach-Object { $_.bytes } | Measure-Object -Sum).Sum
    Write-Output "Preview only: $($manifest.Count) files, $totalBytes bytes. Archive: $archiveRoot"
    Write-Output 'Use -Apply to archive these generated inspection files. No files are deleted.'
    exit 0
}
if($manifest.Count -eq 0) { Write-Output 'No inspection files need archiving.'; exit 0 }
New-Item -ItemType Directory -Path $archiveRoot -Force | Out-Null
# Save the complete move plan before changing any files, so an interrupted run is recoverable.
$manifest | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath (Join-Path $archiveRoot 'manifest.json') -Encoding utf8
foreach($item in $manifest) {
    $source = Join-Path $projectRoot $item.source
    $destination = Join-Path $projectRoot $item.destination
    New-Item -ItemType Directory -Path (Split-Path -Parent $destination) -Force | Out-Null
    Move-Item -LiteralPath $source -Destination $destination
    if((Get-FileHash -LiteralPath $destination -Algorithm SHA256).Hash.ToLowerInvariant() -ne $item.sha256) { throw "Archive verification failed: $destination" }
}
Write-Output "Archived $($manifest.Count) files; byte hashes verified. Manifest: $archiveRoot/manifest.json"
