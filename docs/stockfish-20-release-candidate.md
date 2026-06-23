# Stockfish 20 Release Candidate

Date: 2026-06-23
Branch: `fix/stockfish-20-release`
Source commit: `eaa298800133b38cf618e0f676306ee6fcd889ab`
Fork remote: `https://github.com/gtolontop/Stockfish-V2`

## Candidate Artifact

- Binary: `C:/Users/teamr/Desktop/stockfish/match-results/bin/stockfish-20-x86-64-avx512icl`
- Build type: PGO, `ARCH=native`
- Selected architecture: `x86-64-avx512icl`
- Network: `nn-71d6d32cb962.nnue`
- SHA256: `2202D368339480602133C54FF49C71CAADB05B1955EB49BD2ADBE6D03036C4E0`

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
cp stockfish /mnt/c/Users/teamr/Desktop/stockfish/match-results/bin/stockfish-20-x86-64-avx512icl
```

## Validation Summary

Smoke bench:

- Command: `./stockfish bench 16 1 13 default depth`
- Nodes searched: `3493826`

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

Known limitation:

- This is not a formal Fishtest proof and does not prove superiority over the current official development head. Candidate engine tweaks tested so far did not beat the current dev baseline and were intentionally not committed.
