# Stockfish 20 Test Results

Date: 2026-06-23

## Baseline: Stockfish 18 vs Current Dev

Purpose:

- Verify whether the current fork baseline already improves on official `sf_18`.
- Use this before making risky engine changes.

Builds:

- `SF18`: upstream tag `sf_18`, commit `cb3d4ee9`.
- `SF20Dev`: fork branch `fix/stockfish-20-release`, based on `stockfish-dev-20260614-74a0a737`.
- Both built with `make -j$(nproc) build ARCH=native` under WSL.
- Both selected `x86-64-avx512icl`.

Match command summary:

- Runner: Fastchess.
- Opening book: `tests/openings/smoke.epd`.
- Time mode: fixed nodes.
- Nodes: `20000`.
- Threads: `1`.
- Hash: `16 MB`.
- Rounds: `16`.
- Repeat colors: yes.
- Games: `32`.
- PGN: `C:/Users/teamr/Desktop/stockfish/match-results/sf18-vs-dev-nodes20k.pgn`.

Result from the `SF18` perspective:

- Wins: `2`
- Losses: `8`
- Draws: `22`
- Points: `13.0 / 32`
- Score: `40.62%`
- Elo: `-65.92 +/- 62.12`
- LOS for `SF18`: `1.56%`

Interpretation:

- In this local smoke test, the current dev baseline is clearly ahead of official Stockfish 18.
- This is not a formal Fishtest-strength proof, but it is enough to treat the current dev branch as a stronger Stockfish 20 base than `sf_18`.
- Next tests should compare future source changes against `stockfish-dev-20260614-74a0a737`, not against `sf_18`.
