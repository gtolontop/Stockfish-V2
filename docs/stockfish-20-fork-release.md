# Stockfish 20 RC2 Fork Release

Tag suggestion: `stockfish-20-rc2`
Branch: `fix/stockfish-20-release`
Status: fork-only release candidate

## Release Asset

Attach this verified package to the fork release:

- `C:/Users/teamr/Desktop/stockfish/match-results/release/stockfish-20-rc2-x86-64-avx512icl.zip`

The package contains:

- `bin/stockfish-20-x86-64-avx512icl`
- `SOURCE.txt`
- `Copying.txt`
- `AUTHORS`
- `SHA256SUMS.txt`
- Release audit, manifest, notes, test results, experiment log, and research notes.

## Verification

Before attaching the asset, run:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/run_stockfish20_release_gate.ps1
```

The gate rebuilds the package and verifies:

- Fork-only remote safety.
- Release binary SHA256.
- Reference binary SHA256 values.
- Release UCI identity: `id name Stockfish 20`.
- Package file list.
- Internal `SHA256SUMS.txt`.
- `SOURCE.txt` fork/source metadata.
- Packaged binary SHA256.
- Packaged binary UCI identity.

The gate writes the final local handoff summary to:

```text
C:/Users/teamr/Desktop/stockfish/match-results/release/LAST_VERIFIED_STOCKFISH20_PACKAGE.txt
```

## Current Evidence

Selected artifact:

- Name: `stockfish-20-rc2`
- Architecture: `x86-64-avx512icl`
- Build: PGO, `ARCH=native`
- UCI id: `Stockfish 20`
- Binary SHA256: `5032BE17BCA6C30115A46D5F8511DFDF07E3F34604B064D6E11FF289D67F7B61`

Validation summary:

- Repeated local `TC=5+0.05` UHO checks favor `stockfish-20-rc2` over the saved current-development baseline.
- Local fixed-node and time-control checks favor `stockfish-20-rc2` over official Stockfish 18.
- Rejected candidate experiments are documented to avoid reusing weak or incompatible ideas.

See:

- `docs/stockfish-20-release-audit.md`
- `docs/stockfish-20-release-manifest.md`
- `docs/stockfish-20-release-notes.md`
- `docs/stockfish-20-test-results.md`
- `docs/stockfish-20-experiments.md`

## Release Text

This is a fork-only Stockfish 20 RC2 release candidate from `gtolontop/Stockfish-V2`.

It is not an official upstream Stockfish release and must not be represented as one. The official upstream stable release observed during this pass remains Stockfish 18, and the local release branch is protected from pushing to `official-stockfish/Stockfish` by using `official` as a fetch-only remote with push URL `DISABLED`.

The package includes the verified AVX-512ICL PGO binary, source reference metadata, GPL license text, authorship file, internal checksums, and the supporting release documentation.

Known limits:

- This is not a formal Fishtest proof.
- It does not prove deterministic wins against every current Stockfish build.
- It targets `x86-64-avx512icl`; other CPU targets need separate builds and validation.
