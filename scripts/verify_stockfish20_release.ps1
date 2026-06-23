$ErrorActionPreference = 'Stop'

$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$ExpectedBranch = 'fix/stockfish-20-release'
$ExpectedOfficialPushUrl = 'DISABLED'
$ExpectedOriginPushUrl = 'https://github.com/gtolontop/Stockfish-V2'

$ReleaseBinary = 'C:/Users/teamr/Desktop/stockfish/match-results/bin/stockfish-20-rc2-x86-64-avx512icl'
$NoPgoBinary = 'C:/Users/teamr/Desktop/stockfish/match-results/bin/stockfish-20-rc2-nopgo'
$CurrentDevBinary = 'C:/Users/teamr/Desktop/stockfish/match-results/bin/stockfish-base'
$Stockfish18Binary = 'C:/Users/teamr/Desktop/stockfish/Stockfish-sf18/src/stockfish'

$ExpectedReleaseHash = '5032BE17BCA6C30115A46D5F8511DFDF07E3F34604B064D6E11FF289D67F7B61'
$ExpectedNoPgoHash = '4803AF8F051A0EFEEC27F226C18199BCD086E41B9C9B5AE3BD6BDEDB5BFFA390'
$ExpectedCurrentDevHash = '1ACAABC141F266EDFB6A8F2AAC063AC82CC46AE37FECB35454E3229B19006351'
$ExpectedStockfish18Hash = '623F4347A71C282877C20929F602AC8871CB83B993952DA2C0F69EE3BF5EDECC'

function Assert-Equal {
    param(
        [string] $Label,
        [string] $Actual,
        [string] $Expected
    )

    if ($Actual -ne $Expected) {
        throw "$Label mismatch. Expected '$Expected', got '$Actual'."
    }

    Write-Host "[ok] $Label"
}

function Assert-FileHash {
    param(
        [string] $Label,
        [string] $Path,
        [string] $Expected
    )

    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        throw "$Label is missing: $Path"
    }

    $Actual = (Get-FileHash -Algorithm SHA256 -LiteralPath $Path).Hash.ToUpperInvariant()
    Assert-Equal -Label "$Label SHA256" -Actual $Actual -Expected $Expected
}

function Invoke-GitText {
    param([string[]] $Arguments)

    $Output = & git -C $RepoRoot @Arguments 2>&1
    if ($LASTEXITCODE -ne 0) {
        throw "git $($Arguments -join ' ') failed: $Output"
    }

    return ($Output -join "`n").Trim()
}

Write-Host "Verifying Stockfish 20 fork release candidate..."

$Branch = Invoke-GitText -Arguments @('rev-parse', '--abbrev-ref', 'HEAD')
Assert-Equal -Label 'branch' -Actual $Branch -Expected $ExpectedBranch

$OfficialPushUrl = Invoke-GitText -Arguments @('remote', 'get-url', '--push', 'official')
Assert-Equal -Label 'official push URL' -Actual $OfficialPushUrl -Expected $ExpectedOfficialPushUrl

$OriginPushUrl = Invoke-GitText -Arguments @('remote', 'get-url', '--push', 'origin')
Assert-Equal -Label 'origin push URL' -Actual $OriginPushUrl -Expected $ExpectedOriginPushUrl

$Head = Invoke-GitText -Arguments @('rev-parse', 'HEAD')
$OriginHead = Invoke-GitText -Arguments @('rev-parse', 'origin/fix/stockfish-20-release')
Assert-Equal -Label 'origin branch head' -Actual $OriginHead -Expected $Head

Assert-FileHash -Label 'Stockfish 20 rc2 PGO binary' -Path $ReleaseBinary -Expected $ExpectedReleaseHash
Assert-FileHash -Label 'Stockfish 20 rc2 non-PGO binary' -Path $NoPgoBinary -Expected $ExpectedNoPgoHash
Assert-FileHash -Label 'saved current-development baseline binary' -Path $CurrentDevBinary -Expected $ExpectedCurrentDevHash
Assert-FileHash -Label 'official Stockfish 18 binary' -Path $Stockfish18Binary -Expected $ExpectedStockfish18Hash

if (-not (Get-Command wsl.exe -ErrorAction SilentlyContinue)) {
    throw 'wsl.exe is required to run the Linux Stockfish binary for the UCI identity check.'
}

$WslReleaseBinary = (& wsl.exe wslpath -a $ReleaseBinary 2>&1)
if ($LASTEXITCODE -ne 0) {
    throw "Could not convert release binary path to WSL path: $WslReleaseBinary"
}

$WslReleaseBinary = ($WslReleaseBinary | Select-Object -First 1).Trim()
if ($WslReleaseBinary.Contains("'")) {
    throw "Release binary WSL path contains a single quote, which this verifier does not support: $WslReleaseBinary"
}

$UciCommand = "printf 'uci\nquit\n' | '$WslReleaseBinary' | sed -n '1,20p'"
$UciOutput = & wsl.exe bash -c $UciCommand 2>&1
if ($LASTEXITCODE -ne 0) {
    throw "UCI identity check failed: $($UciOutput -join "`n")"
}

$UciText = $UciOutput -join "`n"
if ($UciText -notmatch 'id name Stockfish 20') {
    throw "UCI identity mismatch. Expected 'id name Stockfish 20' in:`n$UciText"
}

Write-Host '[ok] UCI id name Stockfish 20'
Write-Host 'Stockfish 20 release verification passed.'
