# Stockfish 20 Release Audit

Date: 2026-06-23
Branch: `fix/stockfish-20-release`
Release candidate: `stockfish-20-rc2`

## Audit Result

`stockfish-20-rc2` is ready as a fork-only Stockfish 20 release candidate.

The release is verified as:

- Fork-only, with `official` push disabled.
- Built and identified as `Stockfish 20`.
- Packaged with release notes, manifest, research notes, test results, and experiment logs.
- Tested locally against the saved current-development baseline and official Stockfish 18.
- Stronger than the saved current-development baseline in repeated local short time-control UHO validation.
- Stronger than official Stockfish 18 in the documented local validation set.

It is not an official upstream Stockfish release, not a formal Fishtest proof, and not a deterministic proof that the engine wins every game against every current Stockfish build.

## Requirement Evidence

Fork-only release:

- Evidence: `git remote -v` shows `origin` as `https://github.com/gtolontop/Stockfish-V2`.
- Evidence: `official` push URL is `DISABLED`.
- Verification: `scripts/verify_stockfish20_release.ps1` checks the protected official push URL and fork push URL.

Stockfish 20 identity:

- Evidence: selected artifact `C:/Users/teamr/Desktop/stockfish/match-results/bin/stockfish-20-rc2-x86-64-avx512icl`.
- Evidence: SHA256 `5032BE17BCA6C30115A46D5F8511DFDF07E3F34604B064D6E11FF289D67F7B61`.
- Verification: `scripts/verify_stockfish20_release.ps1` checks the hash and UCI output `id name Stockfish 20`.

Current official state researched:

- Evidence: `docs/stockfish-20-research.md`.
- Evidence: official stable release observed as `Stockfish 18`.
- Evidence: official development reference observed as `stockfish-dev-20260614-74a0a737`.
- Verification: local `official/master` refreshed to `74a0a73715322608332038f7c0151ddf0609a59a`.

Baseline and candidate testing:

- Evidence: `docs/stockfish-20-test-results.md`.
- Evidence: repeated `TC=5+0.05` UHO validation versus saved current-development baseline.
- Evidence: fixed-node and time-control validations versus official Stockfish 18.

Engine iteration:

- Evidence: `docs/stockfish-20-experiments.md`.
- Accepted: lazy simple evaluation shortcut.
- Rejected or not applied: ProbCut depth, AVX512 move rank buffer, early TT prefetch, quiet king threat move generation, raw reduction value, TT miss cleanup, and additional Fishtest-inspired search, NNUE, prefetch, history, and SMP ideas.

Package handoff:

- Evidence: package path `C:/Users/teamr/Desktop/stockfish/match-results/release/stockfish-20-rc2-x86-64-avx512icl.zip`.
- Verification: `scripts/package_stockfish20_release.ps1` builds the package outside the Git worktree.
- Verification: `scripts/verify_stockfish20_package.ps1` extracts the zip, validates file list and checksums, checks `SOURCE.txt`, confirms packaged binary SHA256, and checks packaged UCI identity.
- Package contents include `SOURCE.txt`, `Copying.txt`, and `AUTHORS` alongside the binary and release documentation.

Regular commits:

- Evidence: branch history contains small English commits for research, tests, rejected experiments, release manifest, release notes, release packaging, and verifiers.
- Latest release-support commits include:
  - `Add Stockfish 20 release manifest`
  - `Add Stockfish 20 release verifier`
  - `Add Stockfish 20 release notes`
  - `Refresh official Stockfish release snapshot`
  - `Add Stockfish 20 release packaging script`
  - `Add Stockfish 20 package verifier`
  - `Add Stockfish 20 release audit`

## Final Local Commands

Run the one-command release gate before handing off the zip:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/run_stockfish20_release_gate.ps1
```

The release gate runs the full verification sequence, rebuilds the package, verifies the generated zip, and writes `C:/Users/teamr/Desktop/stockfish/match-results/release/LAST_VERIFIED_STOCKFISH20_PACKAGE.txt`.

Equivalent detailed sequence:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/verify_stockfish20_release.ps1
powershell -ExecutionPolicy Bypass -File scripts/package_stockfish20_release.ps1
powershell -ExecutionPolicy Bypass -File scripts/verify_stockfish20_package.ps1
```

Expected outcome:

- Release verifier passes.
- Package script prints the output directory, zip path, and zip SHA256.
- Package verifier passes and confirms packaged UCI id `Stockfish 20`.
- Release gate writes a final summary containing HEAD, branch, official push URL, package path, and package SHA256.

## Residual Risk

- Local validation is not a substitute for upstream Fishtest.
- Short time-control evidence is useful for this fork release candidate, but it is not a universal Elo proof.
- The selected artifact targets `x86-64-avx512icl`; other CPU targets would need separate builds and validation.
- The zip SHA256 can change when the package is regenerated because archive metadata can change, so treat the package verifier and internal `SHA256SUMS.txt` as the handoff integrity gate for the current generated zip.
- The package includes license and authorship files from this repository, but external redistribution still needs the distributor to respect the GPL terms.
