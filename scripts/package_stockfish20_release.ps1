$ErrorActionPreference = 'Stop'

$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$ReleaseName = 'stockfish-20-rc2'
$ReleaseBinary = 'C:/Users/teamr/Desktop/stockfish/match-results/bin/stockfish-20-rc2-x86-64-avx512icl'
$ReleaseRoot = 'C:/Users/teamr/Desktop/stockfish/match-results/release'
$StagingDir = Join-Path $ReleaseRoot $ReleaseName
$ZipPath = Join-Path $ReleaseRoot "$ReleaseName-x86-64-avx512icl.zip"

$Docs = @(
    'docs/stockfish-20-fork-release.md',
    'docs/stockfish-20-release-audit.md',
    'docs/stockfish-20-release-notes.md',
    'docs/stockfish-20-release-manifest.md',
    'docs/stockfish-20-release-candidate.md',
    'docs/stockfish-20-test-results.md',
    'docs/stockfish-20-experiments.md',
    'docs/stockfish-20-research.md'
)

$RootFiles = @(
    'AUTHORS',
    'Copying.txt'
)

function Assert-File {
    param(
        [string] $Label,
        [string] $Path
    )

    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        throw "$Label is missing: $Path"
    }
}

function Get-Sha256Line {
    param(
        [string] $Path,
        [string] $DisplayName
    )

    $Hash = (Get-FileHash -Algorithm SHA256 -LiteralPath $Path).Hash.ToUpperInvariant()
    return "$Hash  $DisplayName"
}

function Assert-ChildPath {
    param(
        [string] $Label,
        [string] $Path,
        [string] $Parent
    )

    $FullPath = [System.IO.Path]::GetFullPath($Path).TrimEnd('\', '/')
    $FullParent = [System.IO.Path]::GetFullPath($Parent).TrimEnd('\', '/')
    $Prefix = $FullParent + [System.IO.Path]::DirectorySeparatorChar

    if (-not $FullPath.StartsWith($Prefix, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "$Label must stay under $FullParent, got $FullPath"
    }
}

Write-Host 'Running release verification before packaging...'
& powershell -ExecutionPolicy Bypass -File (Join-Path $PSScriptRoot 'verify_stockfish20_release.ps1')
if ($LASTEXITCODE -ne 0) {
    throw 'Release verification failed; package was not created.'
}

Assert-File -Label 'release binary' -Path $ReleaseBinary
foreach ($Doc in $Docs) {
    Assert-File -Label 'release document' -Path (Join-Path $RepoRoot $Doc)
}
foreach ($RootFile in $RootFiles) {
    Assert-File -Label 'release root file' -Path (Join-Path $RepoRoot $RootFile)
}

New-Item -ItemType Directory -Path $ReleaseRoot -Force | Out-Null
Assert-ChildPath -Label 'staging directory' -Path $StagingDir -Parent $ReleaseRoot
Assert-ChildPath -Label 'package zip' -Path $ZipPath -Parent $ReleaseRoot

if (Test-Path -LiteralPath $StagingDir) {
    Remove-Item -LiteralPath $StagingDir -Recurse -Force
}

New-Item -ItemType Directory -Path $StagingDir | Out-Null
New-Item -ItemType Directory -Path (Join-Path $StagingDir 'bin') | Out-Null
New-Item -ItemType Directory -Path (Join-Path $StagingDir 'docs') | Out-Null

$PackagedBinaryName = 'stockfish-20-x86-64-avx512icl'
$PackagedBinary = Join-Path (Join-Path $StagingDir 'bin') $PackagedBinaryName
Copy-Item -LiteralPath $ReleaseBinary -Destination $PackagedBinary

foreach ($Doc in $Docs) {
    Copy-Item -LiteralPath (Join-Path $RepoRoot $Doc) -Destination (Join-Path (Join-Path $StagingDir 'docs') (Split-Path $Doc -Leaf))
}
foreach ($RootFile in $RootFiles) {
    Copy-Item -LiteralPath (Join-Path $RepoRoot $RootFile) -Destination (Join-Path $StagingDir $RootFile)
}

$HeadCommit = (& git -C $RepoRoot rev-parse HEAD 2>&1)
if ($LASTEXITCODE -ne 0) {
    throw "Could not read HEAD commit: $HeadCommit"
}
$HeadCommit = ($HeadCommit | Select-Object -First 1).Trim()

$BranchName = (& git -C $RepoRoot rev-parse --abbrev-ref HEAD 2>&1)
if ($LASTEXITCODE -ne 0) {
    throw "Could not read branch name: $BranchName"
}
$BranchName = ($BranchName | Select-Object -First 1).Trim()

$OriginUrl = (& git -C $RepoRoot remote get-url origin 2>&1)
if ($LASTEXITCODE -ne 0) {
    throw "Could not read origin URL: $OriginUrl"
}
$OriginUrl = ($OriginUrl | Select-Object -First 1).Trim()

$SourcePath = Join-Path $StagingDir 'SOURCE.txt'
@(
    'Stockfish 20 RC2 fork release source reference',
    '',
    "Fork remote: $OriginUrl",
    "Branch: $BranchName",
    "Release documentation commit: $HeadCommit",
    'Selected engine artifact source commit: b070c897',
    'Accepted engine patch: 03b95f10 Add lazy simple evaluation shortcut',
    '',
    'This package is fork-only and not an official upstream Stockfish release.',
    'The repository source, GPL license text, and authorship information must accompany binary redistribution.'
) | Set-Content -LiteralPath $SourcePath -Encoding ASCII

$ReadmePath = Join-Path $StagingDir 'README.md'
@(
    '# Stockfish 20 RC2 Fork Release Package',
    '',
    'This package is a fork-only Stockfish 20 release candidate from `gtolontop/Stockfish-V2`.',
    '',
    'It is not an official upstream Stockfish release and must not be published as an official Stockfish build.',
    '',
    '## Binary',
    '',
    "- ``bin/$PackagedBinaryName``",
    '- UCI id: `Stockfish 20`',
    '- Architecture: `x86-64-avx512icl`',
    '- Build type: PGO, `ARCH=native`',
    '',
    '## Documentation',
    '',
    'The `docs/` directory contains the release audit, release notes, manifest, candidate description, test results, experiment log, and research notes.',
    '',
    '## Source, License, And Authors',
    '',
    'See `SOURCE.txt`, `Copying.txt`, and `AUTHORS` in this package.',
    '',
    '## Verification',
    '',
    'Use `SHA256SUMS.txt` to verify the packaged files. The source repository also contains `scripts/verify_stockfish20_release.ps1` for pre-package release verification.'
) | Set-Content -LiteralPath $ReadmePath -Encoding ASCII

$ChecksumPath = Join-Path $StagingDir 'SHA256SUMS.txt'
$ChecksumLines = @()
$ChecksumLines += Get-Sha256Line -Path $PackagedBinary -DisplayName "bin/$PackagedBinaryName"
$ChecksumLines += Get-Sha256Line -Path (Join-Path $StagingDir 'AUTHORS') -DisplayName 'AUTHORS'
$ChecksumLines += Get-Sha256Line -Path (Join-Path $StagingDir 'Copying.txt') -DisplayName 'Copying.txt'
$ChecksumLines += Get-Sha256Line -Path $ReadmePath -DisplayName 'README.md'
$ChecksumLines += Get-Sha256Line -Path $SourcePath -DisplayName 'SOURCE.txt'
foreach ($Doc in Get-ChildItem -LiteralPath (Join-Path $StagingDir 'docs') -File | Sort-Object Name) {
    $ChecksumLines += Get-Sha256Line -Path $Doc.FullName -DisplayName "docs/$($Doc.Name)"
}
$ChecksumLines | Set-Content -LiteralPath $ChecksumPath -Encoding ASCII

if (Test-Path -LiteralPath $ZipPath) {
    Remove-Item -LiteralPath $ZipPath -Force
}

Compress-Archive -Path (Join-Path $StagingDir '*') -DestinationPath $ZipPath -Force

$ZipHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $ZipPath).Hash.ToUpperInvariant()
Write-Host "[ok] Package directory: $StagingDir"
Write-Host "[ok] Package zip: $ZipPath"
Write-Host "[ok] Package zip SHA256: $ZipHash"
