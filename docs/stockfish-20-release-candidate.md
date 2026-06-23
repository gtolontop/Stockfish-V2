# Stockfish 20 Release Candidate

Date: 2026-06-23
Branch: `fix/stockfish-20-release`
Source commit: `b070c897`
Fork remote: `https://github.com/gtolontop/Stockfish-V2`

## Candidate Artifact

- Release candidate: `stockfish-20-rc2`
- Source commit: `b070c897`
- Binary: `C:/Users/teamr/Desktop/stockfish/match-results/bin/stockfish-20-rc2-x86-64-avx512icl`
- Build type: PGO, `ARCH=native`
- Selected architecture: `x86-64-avx512icl`
- Network: `nn-71d6d32cb962.nnue`
- SHA256: `5032BE17BCA6C30115A46D5F8511DFDF07E3F34604B064D6E11FF289D67F7B61`

## Post-rc2 Source Candidate

- The saved release artifact remains `stockfish-20-rc2`.
- The current fork source has one additional accepted local candidate after `rc2`: optimized move prefetch.
- This post-rc2 source candidate must pass PGO artifact validation before replacing the saved `stockfish-20-rc2` binary.

UCI identity:

```text
Stockfish 20 by the Stockfish developers (see AUTHORS file)
id name Stockfish 20
id author the Stockfish developers (see AUTHORS file)
```

## Build Command

```bash
cd /mnt/c/Users/teamr/Desktop/stockfish/Stockfish-V2/src
make -j$(nproc) profile-build ARCH=native
make strip ARCH=native
cp stockfish /mnt/c/Users/teamr/Desktop/stockfish/match-results/bin/stockfish-20-rc2-x86-64-avx512icl
```

## Validation Summary

Accepted engine patch:

- `Add lazy simple evaluation shortcut`
- Commit: `03b95f10`
- Rationale: selected high-margin material/simple-eval positions can bypass full NNUE evaluation, improving real-time play while fixed-node strength remains neutral.

Pre-PGO bench for the accepted patch:

- Command: `./stockfish bench 16 1 13 default depth`
- Nodes searched: `3106469`

Current dev baseline comparison using official UHO samples:

- Fixed nodes, `20000` nodes, `128` games: from baseline perspective `39W / 38L / 51D`, score `50.39%`, Elo `+2.71 +/- 33.32`.
- `TC=5+0.05`, `64` games, seed `2026062317`: from baseline perspective `13W / 19L / 32D`, score `45.31%`, Elo `-32.67 +/- 35.53`.
- `TC=5+0.05`, `64` games, seed `2026062318`: from baseline perspective `13W / 17L / 34D`, score `46.88%`, Elo `-21.74 +/- 42.26`.
- `TC=5+0.05`, `128` games, seed `2026062319`: from baseline perspective `26W / 34L / 68D`, score `46.88%`, Elo `-21.74 +/- 26.76`.
- Final PGO artifact, `TC=5+0.05`, `128` games, seed `2026062320`: from baseline perspective `23W / 36L / 69D`, score `44.92%`, Elo `-35.41 +/- 29.69`.

Stockfish 18 comparison using tiny in-repo smoke book:

- Games: `32`
- Result from Stockfish 18 perspective: `2W / 8L / 22D`
- Stockfish 18 score: `40.62%`
- Elo from Stockfish 18 perspective: `-65.92 +/- 62.12`

Stockfish 18 comparison using official UHO Lichess sample:

- Book source: `official-stockfish/books`, `UHO_Lichess_4852_v1.epd.zip`
- Sample size: `256`
- Games: `128`
- Result from Stockfish 18 perspective: `35W / 42L / 51D`
- Stockfish 18 score: `47.27%`
- Elo from Stockfish 18 perspective: `-19.02 +/- 37.97`

## Release Judgment

This is a valid fork-only Stockfish 20 release candidate:

- It is built from the fork branch, not pushed to or based on changes in the official upstream repository.
- The official remote is configured fetch-only with push URL `DISABLED`.
- It identifies as `Stockfish 20`.
- It beats official Stockfish 18 in the local fixed-node official-book validation.
- It has one locally accepted source-level candidate patch that beats the saved current-dev baseline in short time-control UHO validation.

Known limitation:

- This is not a formal Fishtest proof and does not prove a 10/10 crush over the current official development head. The accepted patch has promising local time-control evidence and should still be treated as a fork release candidate, not an upstream-quality proof.
