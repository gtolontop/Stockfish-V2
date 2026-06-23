# Stockfish 20 Fork Release Notes

Date: 2026-06-23
Release candidate: `stockfish-20-rc2`
Status: fork-only release candidate

## Summary

`stockfish-20-rc2` is the selected Stockfish 20 fork release candidate for this repository. It is built from the fork branch `fix/stockfish-20-release`, identifies as `Stockfish 20` over UCI, and is intended to ship as a local/fork artifact only.

This is not an official upstream Stockfish release and must not be pushed to or opened as a pull request against `official-stockfish/Stockfish`.

## Artifact

- Binary: `C:/Users/teamr/Desktop/stockfish/match-results/bin/stockfish-20-rc2-x86-64-avx512icl`
- Build: PGO, `ARCH=native`
- Selected architecture: `x86-64-avx512icl`
- Network: `nn-71d6d32cb962.nnue`
- UCI id: `Stockfish 20`
- SHA256: `5032BE17BCA6C30115A46D5F8511DFDF07E3F34604B064D6E11FF289D67F7B61`

## Main Engine Change

The accepted source-level change is the lazy simple evaluation shortcut:

- Adds a material-only `Eval::simple_eval()` path.
- Stores root simple evaluation and root best value in the worker.
- Skips full NNUE evaluation in selected high-margin positions where the shortcut is judged safe.

The goal is practical time saving in real-time play while preserving fixed-node behavior.

## Validation

Final PGO `stockfish-20-rc2` versus saved current-development baseline:

- `TC=5+0.05`, `128` games, seed `2026062320`: baseline scored `44.92%`.
- `TC=5+0.05`, `128` games, seed `2026062361`: baseline scored `41.41%`.
- Combined: `256` games, baseline scored `43.16%`, favoring `stockfish-20-rc2`.

Final PGO `stockfish-20-rc2` versus official Stockfish 18:

- Tiny smoke book, `32` games: Stockfish 18 scored `40.62%`.
- Official UHO sample `256`, `128` games: Stockfish 18 scored `47.27%`.
- Official UHO sample `512`, `TC=5+0.05`, `128` games: Stockfish 18 scored `47.66%`.
- Official UHO sample `1024`, `20000` nodes, `256` games: Stockfish 18 scored `45.90%`.

The strongest local evidence is the repeated time-control validation against the saved current-development baseline. The Stockfish 18 checks confirm that the release candidate remains ahead of the public stable baseline used during this release pass.

## Verification

Before handing off the artifact, run:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/verify_stockfish20_release.ps1
```

The verifier checks the branch, fork-only remote safety, origin synchronization, release/reference binary hashes, and UCI identity.

## Packaging

Create the handoff zip with:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/package_stockfish20_release.ps1
```

The script writes the package outside the repository under `C:/Users/teamr/Desktop/stockfish/match-results/release/`, includes `SHA256SUMS.txt`, and prints the zip SHA256 after packaging.

## Known Limits

- This is not a formal Fishtest proof.
- It does not prove a deterministic 10/10 match score against every current Stockfish build.
- Several upstream and Fishtest-inspired candidates were tested and rejected locally because they failed to improve over `stockfish-20-rc2`.

Supporting records:

- `docs/stockfish-20-release-manifest.md`
- `docs/stockfish-20-release-candidate.md`
- `docs/stockfish-20-test-results.md`
- `docs/stockfish-20-experiments.md`
