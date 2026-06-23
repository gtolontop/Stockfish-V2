param(
    [string] $PackageZip = 'C:/Users/teamr/Desktop/stockfish/match-results/release/stockfish-20-rc2-x86-64-avx512icl.zip'
)

$ErrorActionPreference = 'Stop'

$ExpectedBinaryHash = '5032BE17BCA6C30115A46D5F8511DFDF07E3F34604B064D6E11FF289D67F7B61'
$ExpectedEntries = @(
    'README.md',
    'SHA256SUMS.txt',
    'bin/stockfish-20-x86-64-avx512icl',
    'docs/stockfish-20-experiments.md',
    'docs/stockfish-20-release-audit.md',
    'docs/stockfish-20-release-candidate.md',
    'docs/stockfish-20-release-manifest.md',
    'docs/stockfish-20-release-notes.md',
    'docs/stockfish-20-research.md',
    'docs/stockfish-20-test-results.md'
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

function Convert-ToWslPath {
    param([string] $WindowsPath)

    $ForwardSlashPath = $WindowsPath.Replace('\', '/')
    $Output = & wsl.exe wslpath -a $ForwardSlashPath 2>&1
    if ($LASTEXITCODE -ne 0) {
        throw "Could not convert path to WSL path: $Output"
    }

    $WslPath = ($Output | Select-Object -First 1).Trim()
    if ($WslPath.Contains("'")) {
        throw "WSL path contains a single quote, which this verifier does not support: $WslPath"
    }

    return $WslPath
}

function Get-NormalizedRelativePath {
    param(
        [string] $BasePath,
        [string] $Path
    )

    $FullBase = [System.IO.Path]::GetFullPath($BasePath).TrimEnd('\', '/')
    $FullPath = [System.IO.Path]::GetFullPath($Path)
    $Prefix = $FullBase + [System.IO.Path]::DirectorySeparatorChar
    if (-not $FullPath.StartsWith($Prefix, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Path is not under package extraction root: $FullPath"
    }

    $Relative = $FullPath.Substring($Prefix.Length)
    return $Relative.Replace('\', '/')
}

Assert-File -Label 'package zip' -Path $PackageZip

if (-not (Get-Command wsl.exe -ErrorAction SilentlyContinue)) {
    throw 'wsl.exe is required to run the packaged Linux Stockfish binary for the UCI identity check.'
}

$TempRoot = Join-Path ([System.IO.Path]::GetTempPath()) ('stockfish20-package-' + [Guid]::NewGuid().ToString('N'))

try {
    New-Item -ItemType Directory -Path $TempRoot | Out-Null
    Expand-Archive -LiteralPath $PackageZip -DestinationPath $TempRoot -Force

    $ActualEntries = Get-ChildItem -LiteralPath $TempRoot -Recurse -File |
        ForEach-Object { Get-NormalizedRelativePath -BasePath $TempRoot -Path $_.FullName } |
        Sort-Object

    $ExpectedSorted = $ExpectedEntries | Sort-Object
    Assert-Equal -Label 'package file count' -Actual ([string] $ActualEntries.Count) -Expected ([string] $ExpectedSorted.Count)

    for ($Index = 0; $Index -lt $ExpectedSorted.Count; $Index++) {
        Assert-Equal -Label "package entry $($Index + 1)" -Actual $ActualEntries[$Index] -Expected $ExpectedSorted[$Index]
    }

    $ChecksumPath = Join-Path $TempRoot 'SHA256SUMS.txt'
    $ChecksumLines = Get-Content -LiteralPath $ChecksumPath
    foreach ($Line in $ChecksumLines) {
        if ($Line -notmatch '^([0-9A-Fa-f]{64})\s{2}(.+)$') {
            throw "Invalid SHA256SUMS line: $Line"
        }

        $ExpectedHash = $Matches[1].ToUpperInvariant()
        $RelativePath = $Matches[2]
        $FilePath = Join-Path $TempRoot ($RelativePath.Replace('/', [System.IO.Path]::DirectorySeparatorChar))
        Assert-File -Label "checksummed file $RelativePath" -Path $FilePath
        $ActualHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $FilePath).Hash.ToUpperInvariant()
        Assert-Equal -Label "SHA256 $RelativePath" -Actual $ActualHash -Expected $ExpectedHash
    }

    $PackagedBinary = Join-Path $TempRoot 'bin/stockfish-20-x86-64-avx512icl'
    $PackagedBinaryHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $PackagedBinary).Hash.ToUpperInvariant()
    Assert-Equal -Label 'packaged binary release SHA256' -Actual $PackagedBinaryHash -Expected $ExpectedBinaryHash

    $WslPackagedBinary = Convert-ToWslPath -WindowsPath $PackagedBinary
    $UciCommand = "printf 'uci\nquit\n' | '$WslPackagedBinary' | sed -n '1,20p'"
    $UciOutput = & wsl.exe bash -c $UciCommand 2>&1
    if ($LASTEXITCODE -ne 0) {
        throw "Packaged UCI identity check failed: $($UciOutput -join "`n")"
    }

    $UciText = $UciOutput -join "`n"
    if ($UciText -notmatch 'id name Stockfish 20') {
        throw "Packaged UCI identity mismatch. Expected 'id name Stockfish 20' in:`n$UciText"
    }

    Write-Host '[ok] packaged UCI id name Stockfish 20'
    $ZipHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $PackageZip).Hash.ToUpperInvariant()
    Write-Host "[ok] package zip SHA256: $ZipHash"
    Write-Host 'Stockfish 20 package verification passed.'
}
finally {
    if (Test-Path -LiteralPath $TempRoot) {
        Remove-Item -LiteralPath $TempRoot -Recurse -Force
    }
}
