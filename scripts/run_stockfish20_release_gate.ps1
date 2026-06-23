$ErrorActionPreference = 'Stop'

$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$ReleaseRoot = 'C:/Users/teamr/Desktop/stockfish/match-results/release'
$PackageZip = Join-Path $ReleaseRoot 'stockfish-20-rc2-x86-64-avx512icl.zip'
$SummaryPath = Join-Path $ReleaseRoot 'LAST_VERIFIED_STOCKFISH20_PACKAGE.txt'

function Invoke-Step {
    param(
        [string] $Name,
        [scriptblock] $Action
    )

    Write-Host ""
    Write-Host "== $Name =="
    & $Action
    if ($LASTEXITCODE -ne 0) {
        throw "$Name failed."
    }
}

function Invoke-GitText {
    param([string[]] $Arguments)

    $Output = & git -C $RepoRoot @Arguments 2>&1
    if ($LASTEXITCODE -ne 0) {
        throw "git $($Arguments -join ' ') failed: $Output"
    }

    return ($Output -join "`n").Trim()
}

Invoke-Step -Name 'Release verifier' -Action {
    & powershell -ExecutionPolicy Bypass -File (Join-Path $PSScriptRoot 'verify_stockfish20_release.ps1')
}

Invoke-Step -Name 'Package builder' -Action {
    & powershell -ExecutionPolicy Bypass -File (Join-Path $PSScriptRoot 'package_stockfish20_release.ps1')
}

Invoke-Step -Name 'Package verifier' -Action {
    & powershell -ExecutionPolicy Bypass -File (Join-Path $PSScriptRoot 'verify_stockfish20_package.ps1') -PackageZip $PackageZip
}

$ZipHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $PackageZip).Hash.ToUpperInvariant()
$Head = Invoke-GitText -Arguments @('rev-parse', 'HEAD')
$Branch = Invoke-GitText -Arguments @('rev-parse', '--abbrev-ref', 'HEAD')
$OriginHead = Invoke-GitText -Arguments @('rev-parse', 'origin/fix/stockfish-20-release')
$OfficialPushUrl = Invoke-GitText -Arguments @('remote', 'get-url', '--push', 'official')
$GeneratedAt = (Get-Date).ToUniversalTime().ToString('yyyy-MM-ddTHH:mm:ssZ')

@(
    'Stockfish 20 RC2 fork release gate passed',
    '',
    "Generated at UTC: $GeneratedAt",
    "Branch: $Branch",
    "HEAD: $Head",
    "origin/fix/stockfish-20-release: $OriginHead",
    "official push URL: $OfficialPushUrl",
    "Package zip: $PackageZip",
    "Package zip SHA256: $ZipHash",
    '',
    'Checks passed:',
    '- scripts/verify_stockfish20_release.ps1',
    '- scripts/package_stockfish20_release.ps1',
    '- scripts/verify_stockfish20_package.ps1',
    '',
    'This is a fork-only release candidate, not an official upstream Stockfish release.'
) | Set-Content -LiteralPath $SummaryPath -Encoding ASCII

Write-Host ""
Write-Host "[ok] Release gate summary: $SummaryPath"
Write-Host "[ok] Package zip SHA256: $ZipHash"
Write-Host 'Stockfish 20 release gate passed.'
