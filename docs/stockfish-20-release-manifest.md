# Stockfish 20 Fork Release Manifest

Date: 2026-06-23
Branch: `fix/stockfish-20-release`
Fork remote: `https://github.com/gtolontop/Stockfish-V2`
Official remote push URL: `DISABLED`

## Release Scope

This is a fork-only Stockfish 20 release candidate. It is not an official upstream Stockfish release and must not be pushed to or opened as a pull request against `official-stockfish/Stockfish`.

The official Stockfish download page still lists Stockfish 18 as the public stable release checked during this release pass:

- `https://stockfishchess.org/download/`

## Selected Artifact

- Name: `stockfish-20-rc2`
- Binary: `C:/Users/teamr/Desktop/stockfish/match-results/bin/stockfish-20-rc2-x86-64-avx512icl`
- Build type: PGO
- Build architecture: `ARCH=native`, selected as `x86-64-avx512icl`
- Network: `nn-71d6d32cb962.nnue`
- UCI id: `Stockfish 20`
- SHA256: `5032BE17BCA6C30115A46D5F8511DFDF07E3F34604B064D6E11FF289D67F7B61`

## Reference Binaries

- `stockfish-20-rc2-nopgo`
  - Path: `C:/Users/teamr/Desktop/stockfish/match-results/bin/stockfish-20-rc2-nopgo`
  - SHA256: `4803AF8F051A0EFEEC27F226C18199BCD086E41B9C9B5AE3BD6BDEDB5BFFA390`
- Saved current-development baseline
  - Path: `C:/Users/teamr/Desktop/stockfish/match-results/bin/stockfish-base`
  - SHA256: `1ACAABC141F266EDFB6A8F2AAC063AC82CC46AE37FECB35454E3229B19006351`
- Official Stockfish 18 local build
  - Path: `C:/Users/teamr/Desktop/stockfish/Stockfish-sf18/src/stockfish`
  - SHA256: `623F4347A71C282877C20929F602AC8871CB83B993952DA2C0F69EE3BF5EDECC`

## Release Verification

Run the release gate before publishing or handing off the fork release candidate:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/run_stockfish20_release_gate.ps1
```

The gate runs release verification, rebuilds the package, verifies the package, and writes a final summary next to the generated zip.

The lower-level release verifier can also be run directly:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/verify_stockfish20_release.ps1
```

The verifier checks:

- Current branch: `fix/stockfish-20-release`.
- Official push remote remains disabled: `DISABLED`.
- Fork push remote remains `https://github.com/gtolontop/Stockfish-V2`.
- Local HEAD matches `origin/fix/stockfish-20-release`.
- Release and reference binary SHA256 values match this manifest.
- The release binary reports `id name Stockfish 20` over UCI.

## Release Packaging

Create a fork-only handoff package after verification passes:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/package_stockfish20_release.ps1
```

The package script writes outside the Git worktree under:

- Directory: `C:/Users/teamr/Desktop/stockfish/match-results/release/stockfish-20-rc2`
- Zip: `C:/Users/teamr/Desktop/stockfish/match-results/release/stockfish-20-rc2-x86-64-avx512icl.zip`

The package contains the release binary, a package README, `SHA256SUMS.txt`, `SOURCE.txt`, `Copying.txt`, `AUTHORS`, and the release documentation set. The zip checksum is printed by the script after each package build.

Verify the generated zip before handoff:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/verify_stockfish20_package.ps1
```

The package verifier extracts the zip to a temporary directory, checks the expected file list, validates every entry in `SHA256SUMS.txt`, checks the source reference, confirms the packaged binary SHA256, and verifies that the extracted binary reports `id name Stockfish 20` over UCI.

## Source State

- Initial release documentation HEAD when this manifest was created: `b80b06cbb771ea03229a577d3ab908353bdb2726`
- `origin/fix/stockfish-20-release` matched local HEAD at verification time.
- `official/master` verification point: `74a0a73715322608332038f7c0151ddf0609a59a`

The selected artifact was documented from source commit `b070c897`, with the accepted engine patch:

- `03b95f10 Add lazy simple evaluation shortcut`

## Validation Highlights

Stockfish 20 rc2 PGO versus saved current-development baseline:

- `TC=5+0.05`, `128` games, seed `2026062320`: baseline perspective `23W / 36L / 69D`, score `44.92%`.
- `TC=5+0.05`, `128` games, seed `2026062361`: baseline perspective `22W / 44L / 62D`, score `41.41%`.

Stockfish 20 rc2 PGO versus official Stockfish 18:

- Tiny smoke book, `32` games: Stockfish 18 perspective `2W / 8L / 22D`, score `40.62%`.
- Official UHO, sample size `256`, `128` games: Stockfish 18 perspective `35W / 42L / 51D`, score `47.27%`.
- Official UHO, sample size `512`, `TC=5+0.05`, `128` games, seed `2026062362`: Stockfish 18 perspective `33W / 39L / 56D`, score `47.66%`.
- Official UHO, sample size `1024`, `20000` nodes, `256` games, seed `2026062368`: Stockfish 18 perspective `73W / 94L / 89D`, score `45.90%`.

## Rejected Candidate Coverage

The experiment log records rejected or not-applied alternatives, including:

- ProbCut depth when improving.
- AVX512 move rank buffer.
- Early TT prefetch.
- Quiet king threat move generation.
- Raw reduction value.
- TT miss data cleanup.
- Multiple Fishtest-active search, history, prefetch, NNUE, and SMP candidates.

See:

- `docs/stockfish-20-release-audit.md`
- `docs/stockfish-20-experiments.md`
- `docs/stockfish-20-test-results.md`

## Release Judgment

`stockfish-20-rc2` remains the selected fork-only release candidate. The artifact identity, SHA256, reference binaries, fork remote safety, and validation evidence are documented. This is still not an official upstream Stockfish release and not a formal Fishtest proof.
