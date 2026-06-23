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

## Rejected: Extend LMR Re-Search Depth On Cut Or PV Nodes

Source:

- Public Fishtest-active branch: `https://github.com/Vizvezdenec/Stockfish`, branch `cnLmrSpawn4`.
- Fishtest active run observed on 2026-06-23 had positive LLR, but was not a finished accepted upstream patch.

Patch:

```diff
-            Depth d = std::max(1, std::min(newDepth - r / 1024, newDepth + 2)) + PvNode;
+            Depth d = std::max(1, std::min(newDepth - r / 1024, newDepth + 2 + (cutNode || PvNode))) + PvNode;
```

Bench:

- Nodes searched: `2448547`

Initial result from the baseline perspective on `tests/openings/smoke.epd` with `TC=0.2+0.002`:

- Games: `64`
- Wins: `15`
- Losses: `26`
- Draws: `23`
- Points: `26.5 / 64`
- Score: `41.41%`
- Elo: `-60.31 +/- 69.66`

Fixed-node result from the baseline perspective on `uho_lichess_4852_sample_256.epd`:

- Games: `128`
- Wins: `35`
- Losses: `35`
- Draws: `58`
- Points: `64.0 / 128`
- Score: `50.00%`
- Elo: `0.00 +/- 37.77`

Short time-control results from the baseline perspective on official UHO samples:

- `TC=5+0.05`, `uho_lichess_4852_sample_256.epd`, seed `2026062310`: `15W / 21L / 28D`, score `45.31%`, Elo `-32.67 +/- 49.38`.
- `TC=5+0.05`, `uho_lichess_4852_sample_256.epd`, seed `2026062311`: `15W / 16L / 33D`, score `49.22%`, Elo `-5.43 +/- 31.96`.
- `TC=5+0.05`, `uho_lichess_4852_sample_512.epd`, seed `2026062312`: `32W / 31L / 65D`, score `50.39%`, Elo `+2.71 +/- 27.70`.

Decision:

- Rejected locally. The smoke result was strong, but fixed-node UHO and the larger time-control confirmation were neutral; this is not sufficient release evidence.

## Rejected: Gate Correction History Updates By Depth

Source:

- Public Fishtest-active branch: `https://github.com/anematode/Stockfish`, branch `emotibonk`.
- Fishtest active run observed on 2026-06-23 had positive LLR, but was not a finished accepted upstream patch.

Patch:

```diff
-    if (!ss->inCheck && !(bestMove && pos.capture(bestMove))
+    if (!ss->inCheck && !(bestMove && pos.capture(bestMove)) && depth > 2
         && (bestValue > ss->staticEval) == bool(bestMove))
```

Bench:

- Nodes searched: `2849871`

Initial result from the baseline perspective on `tests/openings/smoke.epd` with `TC=0.2+0.002`:

- Games: `64`
- Wins: `22`
- Losses: `21`
- Draws: `21`
- Points: `32.5 / 64`
- Score: `50.78%`
- Elo: `+5.43 +/- 75.65`

Decision:

- Rejected. The first local smoke filter did not show an advantage for the candidate.

## Rejected: Average Null-Move Fail-High Value With Beta

Source:

- Public Fishtest-active branch: `https://github.com/Vizvezdenec/Stockfish`, branch `nmpFrA1`.
- Fishtest active run observed on 2026-06-23 had positive LLR, but was not a finished accepted upstream patch.

Patch:

```diff
-                return nullValue;
+                return (nullValue + beta) / 2;
```

Bench:

- Nodes searched: `3083255`

Initial result from the baseline perspective on `tests/openings/smoke.epd` with `TC=0.2+0.002`:

- Games: `64`
- Wins: `24`
- Losses: `16`
- Draws: `24`
- Points: `36.0 / 64`
- Score: `56.25%`
- Elo: `+43.66 +/- 59.81`

Decision:

- Rejected. The first local smoke filter showed the baseline clearly ahead.

## Current Status

- No source-level strength patch has been accepted beyond using the stronger post-Stockfish-18 official development baseline.
- The fork release candidate remains `stockfish-20-rc1`.
- Future experiments should target different mechanisms instead of retuning these local history constants.
