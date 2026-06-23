# Stockfish 20 Engine Experiments

Date: 2026-06-23

This log records engine experiments tested against the current development baseline. A candidate is kept only if it beats the baseline with useful evidence. Rejected experiments are recorded to avoid retesting the same weak ideas.

## Baseline

- Source baseline: `stockfish-dev-20260614-74a0a737`
- Saved binary: `C:/Users/teamr/Desktop/stockfish/match-results/bin/stockfish-base`
- Candidate comparison method: Fastchess, fixed nodes, paired openings.
- Default local candidate-vs-baseline test:
  - Nodes: `20000`
  - Threads: `1`
  - Hash: `16 MB`
  - Rounds: `32`
  - Games: `64`
  - Opening book: `tests/openings/smoke.epd`

## Rejected: Loosen Good Quiet Threshold

Patch:

```diff
-    constexpr int goodQuietThreshold = -14000;
+    constexpr int goodQuietThreshold = -13500;
```

Result from the baseline perspective:

- Wins: `20`
- Losses: `16`
- Draws: `28`
- Points: `34.0 / 64`
- Score: `53.12%`
- Elo: `+21.74 +/- 36.36`

Decision:

- Rejected. The baseline scored ahead, so the candidate was worse.

## Rejected: Tighten Good Quiet Threshold

Patch:

```diff
-    constexpr int goodQuietThreshold = -14000;
+    constexpr int goodQuietThreshold = -14500;
```

Result from the baseline perspective:

- Wins: `16`
- Losses: `12`
- Draws: `36`
- Points: `34.0 / 64`
- Score: `53.12%`
- Elo: `+21.74 +/- 47.47`

Decision:

- Rejected. The baseline scored ahead, so the candidate was worse.

## Current Status

- No source-level strength patch has been accepted beyond using the stronger post-Stockfish-18 official development baseline.
- The fork release candidate remains `stockfish-20-rc1`.
- Future experiments should target different mechanisms instead of retuning this threshold locally.
