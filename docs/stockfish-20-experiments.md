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

## Rejected: Raise Capture History Initialization

Patch:

```diff
-    captureHistory.fill(-699);
+    captureHistory.fill(-640);
```

Initial result from the baseline perspective on `tests/openings/smoke.epd`:

- Games: `64`
- Wins: `8`
- Losses: `16`
- Draws: `40`
- Points: `28.0 / 64`
- Score: `43.75%`
- Elo: `-43.66 +/- 40.64`
- LOS for baseline: `1.63%`

Follow-up result from the baseline perspective on `uho_lichess_4852_sample_256.epd`:

- Games: `128`
- Wins: `34`
- Losses: `37`
- Draws: `57`
- Points: `62.5 / 128`
- Score: `48.83%`
- Elo: `-8.14 +/- 38.11`
- LOS for baseline: `33.70%`

Confirmation result from the baseline perspective on `uho_lichess_4852_sample_512.epd`:

- Games: `256`
- Wins: `72`
- Losses: `73`
- Draws: `111`
- Points: `127.5 / 256`
- Score: `49.80%`
- Elo: `-1.36 +/- 25.97`
- LOS for baseline: `45.91%`

Decision:

- Rejected. The large UHO confirmation was effectively neutral, so this is not strong enough evidence for a release strength patch.

## Current Status

- No source-level strength patch has been accepted beyond using the stronger post-Stockfish-18 official development baseline.
- The fork release candidate remains `stockfish-20-rc1`.
- Future experiments should target different mechanisms instead of retuning these local history constants.
