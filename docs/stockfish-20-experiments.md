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

## Accepted: Lazy Simple Evaluation Shortcut

Source:

- Public Fishtest-active branch: `https://github.com/snicolet/Stockfish`, branch `simple_eval10`.
- Fishtest active runs observed on 2026-06-23 had positive LTC/VLTC LLR, but were not finished accepted upstream patches when tested locally.

Patch summary:

- Add `Eval::simple_eval()` for a side-to-move material-only score.
- Store root simple evaluation and root best value in `Search::Worker`.
- In `Search::Worker::evaluate()`, skip the full NNUE evaluation for sufficiently large material/simple-eval cases, adjusted by rule-50 shuffling and root evaluation bounds.

Bench:

- Nodes searched: `3106469`

Initial result from the baseline perspective on `tests/openings/smoke.epd` with `TC=0.2+0.002`:

- Games: `64`
- Wins: `18`
- Losses: `21`
- Draws: `25`
- Points: `30.5 / 64`
- Score: `47.66%`
- Elo: `-16.30 +/- 70.66`

Fixed-node result from the baseline perspective on `uho_lichess_4852_sample_256.epd`:

- Games: `128`
- Wins: `39`
- Losses: `38`
- Draws: `51`
- Points: `64.5 / 128`
- Score: `50.39%`
- Elo: `+2.71 +/- 33.32`

Short time-control results from the baseline perspective on official UHO samples:

- `TC=5+0.05`, `uho_lichess_4852_sample_256.epd`, seed `2026062317`: `13W / 19L / 32D`, score `45.31%`, Elo `-32.67 +/- 35.53`.
- `TC=5+0.05`, `uho_lichess_4852_sample_256.epd`, seed `2026062318`: `13W / 17L / 34D`, score `46.88%`, Elo `-21.74 +/- 42.26`.
- `TC=5+0.05`, `uho_lichess_4852_sample_512.epd`, seed `2026062319`: `26W / 34L / 68D`, score `46.88%`, Elo `-21.74 +/- 26.76`.

Decision:

- Accepted as a Stockfish 20 candidate patch. Fixed-node UHO was neutral, but this patch is intended to save time by bypassing full NNUE in selected positions; three independent short time-control UHO runs all favored the candidate.

## Rejected Incremental: Penalize Negative Singular Extensions

Source:

- Public Fishtest-active branch: `https://github.com/Dubslow/Stockfish`, branch `penalize-stuff-1c`.
- Fishtest active run observed on 2026-06-23 had positive LLR, but was not a finished accepted upstream patch.

Patch:

```diff
             else if (cutNode)
                 extension = -2;
+
+            if (extension < 0)
+                ttWriter.penalize(-1);
```

Incremental baseline:

- `C:/Users/teamr/Desktop/stockfish/match-results/bin/stockfish-20-rc2-nopgo`
- This is the current `rc2` source built without PGO, so the experiment compares non-PGO to non-PGO rather than candidate non-PGO to release PGO.

Bench:

- Nodes searched: `2608431`

Initial result from the `rc2-nopgo` baseline perspective on `tests/openings/smoke.epd` with `TC=0.2+0.002`:

- Games: `64`
- Wins: `19`
- Losses: `28`
- Draws: `17`
- Points: `27.5 / 64`
- Score: `42.97%`
- Elo: `-49.18 +/- 66.40`

Official UHO short time-control results from the `rc2-nopgo` baseline perspective:

- `TC=5+0.05`, `uho_lichess_4852_sample_256.epd`, seed `2026062322`: `14W / 16L / 34D`, score `48.44%`, Elo `-10.86 +/- 42.65`.
- `TC=5+0.05`, `uho_lichess_4852_sample_256.epd`, seed `2026062323`: `18W / 16L / 30D`, score `51.56%`, Elo `+10.86 +/- 42.65`.
- Combined UHO: `32W / 32L / 64D`, score `50.00%`.

Decision:

- Rejected locally as an incremental patch over `rc2`. The smoke result was strong, but independent UHO seeds canceled out exactly.

Retest on 2026-06-23:

- The active public Fishtest run later improved to a positive LLR near `+1.5`, so the same one-line patch was retested locally against `stockfish-20-rc2-nopgo`.
- Bench signature was unchanged from the first local try: `2608431`.
- Smoke, seed `2026062355`, from the `rc2-nopgo` baseline perspective: `22W / 15L / 27D`, score `55.47%`, Elo `+38.15 +/- 61.05`, favoring the baseline.
- Short UHO, seed `2026062356`, from the `rc2-nopgo` baseline perspective: `20W / 13L / 31D`, score `55.47%`, Elo `+38.15 +/- 50.26`, favoring the baseline.

Retest decision:

- Rejected again. The newer public signal did not transfer to the local incremental `rc2` source; both the smoke and UHO retests favored the current release-candidate source.

## Rejected Incremental: Depth-Scaled Shallower Re-Search Threshold

Source:

- Public Fishtest-active branch: `https://github.com/FauziAkram/Stockfish`, branch `nddec2`.
- Fishtest active run observed on 2026-06-23 had slightly positive LLR, but was not a finished accepted upstream patch.

Patch:

```diff
-                const bool doShallowerSearch = value < bestValue + 9;
+                const bool doShallowerSearch = value < bestValue + 9 - depth / 3;
```

Incremental baseline:

- `C:/Users/teamr/Desktop/stockfish/match-results/bin/stockfish-20-rc2-nopgo`

Bench:

- Nodes searched: `2862670`

Initial result from the `rc2-nopgo` baseline perspective on `tests/openings/smoke.epd` with `TC=0.2+0.002`:

- Games: `64`
- Wins: `27`
- Losses: `20`
- Draws: `17`
- Points: `35.5 / 64`
- Score: `55.47%`
- Elo: `+38.15 +/- 72.05`

Decision:

- Rejected. The first incremental smoke filter showed the `rc2` baseline ahead.

## Rejected Incremental: Cached Normal Move Properties

Source:

- Public Fishtest-active branch: `https://github.com/bobbypaper/Stockfish`, branch `normal-move-properties-state`.
- Fishtest active run observed on 2026-06-23 was still in progress, so this was treated as an experimental candidate only.

Patch summary:

- Added a local `NormalMoveState` helper in `src/search.cpp`.
- Cached occupancy, king blockers, and king bitboards for normal move legality and gives-check queries in the main search and qsearch move loops.
- Non-normal moves kept the existing `pos.legal()`, `pos.gives_check()`, and `pos.capture_stage()` paths.

Incremental baseline:

- `C:/Users/teamr/Desktop/stockfish/match-results/bin/stockfish-20-rc2-nopgo`

Bench:

- Nodes searched: `3106469`
- Nodes/second: `927304`

Initial result from the `rc2-nopgo` baseline perspective on `tests/openings/smoke.epd` with `TC=0.2+0.002`:

- Games: `64`
- Wins: `24`
- Losses: `19`
- Draws: `21`
- Points: `34.5 / 64`
- Score: `53.91%`
- Elo: `+27.20 +/- 78.70`

Official UHO short time-control result from the `rc2-nopgo` baseline perspective:

- `TC=5+0.05`, `uho_lichess_4852_sample_256.epd`, seed `2026062326`: `22W / 14L / 28D`, score `56.25%`, Elo `+43.66 +/- 40.64`.

Decision:

- Rejected locally as an incremental patch over `rc2`. The smoke filter was mildly favorable, but the longer UHO test showed the current `rc2` baseline clearly ahead.

## Rejected Incremental: Better Skip NNUE Architecture And Net

Source:

- Public Fishtest-active branch: `https://github.com/anematode/Stockfish`, branch `better-skip2-fishtest`.
- Fishtest active run observed on 2026-06-23 had low positive LLR, but only early evidence.

Patch summary:

- Changed the default network from `nn-71d6d32cb962.nnue` to `nn-e9b0f021454e.nnue`.
- Changed the NNUE architecture around the skip connection and final affine input.

Incremental baseline:

- `C:/Users/teamr/Desktop/stockfish/match-results/bin/stockfish-20-rc2-nopgo`

Bench:

- Nodes searched: `2671360`
- Nodes/second: `863679`

Initial result from the `rc2-nopgo` baseline perspective on `tests/openings/smoke.epd` with `TC=0.2+0.002`:

- Games: `64`
- Wins: `25`
- Losses: `16`
- Draws: `23`
- Points: `36.5 / 64`
- Score: `57.03%`
- Elo: `+49.18 +/- 64.50`

Decision:

- Rejected. The first incremental smoke filter showed the current `rc2` baseline clearly ahead, so the NNUE architecture patch and downloaded candidate network were removed.

## Rejected Incremental: Restrict Singular Extension Depth Increase

Source:

- Public Fishtest-active branch: `https://github.com/locutus2/Stockfish`, branch `singular_ext_depth4`.
- Fishtest active LTC run observed on 2026-06-23 was slightly positive, while the STC run was negative.

Patch:

```diff
-                depth++;
+                depth += !allNode;
```

Incremental baseline:

- `C:/Users/teamr/Desktop/stockfish/match-results/bin/stockfish-20-rc2-nopgo`

Bench:

- Nodes searched: `2520234`
- Nodes/second: `919793`

Initial result from the `rc2-nopgo` baseline perspective on `tests/openings/smoke.epd` with `TC=0.2+0.002`:

- Games: `64`
- Wins: `19`
- Losses: `27`
- Draws: `18`
- Points: `28.0 / 64`
- Score: `43.75%`
- Elo: `-43.66 +/- 71.07`

Official UHO short time-control results from the `rc2-nopgo` baseline perspective:

- `TC=5+0.05`, `uho_lichess_4852_sample_256.epd`, seed `2026062329`: `13W / 14L / 37D`, score `49.22%`, Elo `-5.43 +/- 38.48`.
- `TC=5+0.05`, `uho_lichess_4852_sample_256.epd`, seed `2026062330`: `16W / 12L / 36D`, score `53.12%`, Elo `+21.74 +/- 49.87`.
- Combined UHO: `29W / 26L / 73D`, score `51.17%`.

Decision:

- Rejected locally as an incremental patch over `rc2`. The smoke filter favored the candidate, but the two UHO seeds left the current baseline slightly ahead overall.

## Rejected Incremental: Simplified Thread Selection

Source:

- Public Fishtest-active branch: `https://github.com/robertnurnberg/Stockfish`, branch `sim-threadselection`.
- Fishtest active LTC run observed on 2026-06-23 was an 8-thread simplification test and had drifted near neutral.

Patch summary:

- Removed root depth from the thread voting score in `ThreadPool::get_best_thread()`.
- Used PV length as the tie-breaker when votes are equal.

Incremental baseline:

- `C:/Users/teamr/Desktop/stockfish/match-results/bin/stockfish-20-rc2-nopgo`

Initial SMP result from the `rc2-nopgo` baseline perspective on `tests/openings/smoke.epd`:

- Time control: `1+0.01`
- Threads: `8`
- Concurrency: `1`
- Games: `32`
- Wins: `7`
- Losses: `5`
- Draws: `20`
- Points: `17.0 / 32`
- Score: `53.12%`
- Elo: `+21.74 +/- 51.61`

Decision:

- Rejected locally as an incremental SMP patch over `rc2`. The only local 8-thread smoke test favored the current baseline, and the public active run was not positive enough to justify carrying the simplification.

## Rejected After PGO: testNet02 Default Network

Source:

- Public Fishtest-active branch: `https://github.com/vondele/Stockfish`, branch `testNet02`.
- Fishtest active run observed on 2026-06-23 had very large volume but only near-neutral positive LLR, so this needed local confirmation.

Patch:

```diff
-#define EvalFileDefaultName "nn-71d6d32cb962.nnue"
+#define EvalFileDefaultName "nn-10d7f09dac5e.nnue"
```

Incremental baseline:

- `C:/Users/teamr/Desktop/stockfish/match-results/bin/stockfish-20-rc2-nopgo`

Initial result from the `rc2-nopgo` baseline perspective on `tests/openings/smoke.epd` with `TC=0.2+0.002`:

- Games: `64`
- Wins: `21`
- Losses: `22`
- Draws: `21`
- Points: `31.5 / 64`
- Score: `49.22%`
- Elo: `-5.43 +/- 78.78`

Official UHO short time-control results from the `rc2-nopgo` baseline perspective:

- `TC=5+0.05`, `uho_lichess_4852_sample_256.epd`, seed `2026062333`: `15W / 19L / 30D`, score `46.88%`, Elo `-21.74 +/- 52.17`.
- `TC=5+0.05`, `uho_lichess_4852_sample_256.epd`, seed `2026062334`: `16W / 18L / 30D`, score `48.44%`, Elo `-10.86 +/- 52.44`.
- Combined UHO: `31W / 37L / 60D`, score `47.66%`.

Final PGO artifact validation from the saved current-development baseline perspective:

- `TC=5+0.05`, `uho_lichess_4852_sample_512.epd`, seed `2026062335`: `41W / 34L / 53D`, score `52.73%`, Elo `+19.02 +/- 36.42`.
- `TC=5+0.05`, `uho_lichess_4852_sample_512.epd`, seed `2026062336`: `30W / 34L / 64D`, score `48.44%`, Elo `-10.86 +/- 30.08`.
- Combined PGO vs current dev: `71W / 68L / 117D`, score `50.59%` for the saved current-development baseline.

Decision:

- Rejected after PGO validation. The network looked positive as a non-PGO increment over `rc2`, but the rebuilt `stockfish-20-rc3` artifact did not beat the saved current-development baseline. The source was reverted and `stockfish-20-rc2` remains the release candidate.

## Rejected Incremental: Remove Pawn History Prefetch

Source:

- Public Fishtest-active branch: `https://github.com/ces42/Stockfish`, branch `simp-pf`.
- Fishtest active run observed on 2026-06-23 described the change as no functional change.

Patch:

```diff
-        prefetch(&history->pawn_entry(*this)[pc][to]);
```

Incremental baseline:

- `C:/Users/teamr/Desktop/stockfish/match-results/bin/stockfish-20-rc2-nopgo`

Bench:

- Nodes searched: `3106469`
- Nodes/second: `919345`
- Bench signature matched the current `rc2` source, as expected for a no-functional-change patch.

Initial result from the `rc2-nopgo` baseline perspective on `tests/openings/smoke.epd` with `TC=0.2+0.002`:

- Games: `64`
- Wins: `25`
- Losses: `17`
- Draws: `22`
- Points: `36.0 / 64`
- Score: `56.25%`
- Elo: `+43.66 +/- 71.07`

Decision:

- Rejected. The patch was expected to be a small performance simplification, but the first time-control smoke filter favored the current `rc2` baseline clearly.

## Rejected Incremental: Stable Eval Depth Reduction

Source:

- Public Fishtest-active branch: `https://github.com/AdrianGHUB15/Stockfish`, branch `stableEvalReduction1-SS5`.
- Fishtest active run observed on 2026-06-23 was negative.

Patch summary:

- Added a depth reduction when static evaluation is close to the value from five plies earlier and the current static evaluation improved by at least 25 cp.
- This changed the bench signature, so it was treated as a functional search patch rather than a speed-only patch.

Incremental baseline:

- `C:/Users/teamr/Desktop/stockfish/match-results/bin/stockfish-20-rc2-nopgo`

Bench:

- Nodes searched: `2471057`
- Nodes/second: `854445`

Initial result from the `rc2-nopgo` baseline perspective on `tests/openings/smoke.epd` with `TC=0.2+0.002`:

- Games: `64`
- Wins: `25`
- Losses: `18`
- Draws: `21`
- Points: `35.5 / 64`
- Score: `55.47%`
- Elo: `+38.15 +/- 73.74`

Decision:

- Rejected. The public run was already negative, and the local smoke filter clearly favored the current `rc2` baseline.

## Rejected Incremental: Depth-Scaled Cutoff Mismatch Penalty

Source:

- Public Fishtest-active branch: `https://github.com/Dubslow/Stockfish`, branch `cutoff-mismatch-3d`.
- Isolated top commit: `ec29fc0016074367d066ab07dc8eb26c41cab650`.
- Fishtest active run observed on 2026-06-23 was negative at LTC.

Patch:

```diff
-        ttWriter.penalize(1);
+        ttWriter.penalize(depth / 8);
```

Incremental baseline:

- `C:/Users/teamr/Desktop/stockfish/match-results/bin/stockfish-20-rc2-nopgo`

Bench:

- Nodes searched: `2746588`
- Nodes/second: `969498`
- Bench signature changed, so this was treated as a functional search patch.

Smoke result from the `rc2-nopgo` baseline perspective on `tests/openings/smoke.epd` with `TC=0.2+0.002`:

- Games: `64`
- Wins: `22`
- Losses: `17`
- Draws: `25`
- Points: `34.5 / 64`
- Score: `53.91%`
- Elo: `+27.20 +/- 63.33`

Official UHO short checks from the `rc2-nopgo` baseline perspective:

- Seed `2026062342`: `5W / 7L / 20D`, score `46.88%`, Elo `-21.74 +/- 51.61`.
- Seed `2026062343`: `8W / 7L / 17D`, score `51.56%`, Elo `+10.86 +/- 64.41`.
- Combined: `13W / 14L / 37D`, score `49.22%`, Elo about `-5.43`.

Decision:

- Rejected. The local smoke filter favored the current `rc2` baseline, the two UHO checks were effectively neutral, and the public LTC run was negative.

## Rejected Incremental: Earlier TT Prefetch

Source:

- Public Fishtest-active branch: `https://github.com/ces42/Stockfish`, branch `pfearly`.
- Isolated active-run commit: `8a8e512ce9084834dd5a426cdbd7d011910ff7a7`.
- Fishtest active run observed on 2026-06-23 was negative.

Patch summary:

- Moved the transposition-table prefetch earlier in `Position::do_move()`.
- Added a second TT prefetch after pawn moves reset `rule50`.
- Bench signature was unchanged, so the patch was treated as a performance-only change.

Incremental baseline:

- `C:/Users/teamr/Desktop/stockfish/match-results/bin/stockfish-20-rc2-nopgo`

Bench:

- Nodes searched: `3106469`
- Nodes/second: `915552`
- Bench signature matched the current `rc2` source.

Smoke checks from the `rc2-nopgo` baseline perspective on `tests/openings/smoke.epd` with `TC=0.2+0.002`:

- Seed `2026062344`: `22W / 25L / 17D`, score `47.66%`, Elo `-16.30 +/- 72.34`.
- Seed `2026062345`: `23W / 20L / 21D`, score `52.34%`, Elo `+16.30 +/- 61.64`.
- Combined: `45W / 45L / 38D`, score `50.00%`.

Decision:

- Rejected. The local smoke checks canceled out exactly, local bench speed was not improved, and the public run was already negative.

## Not Applied: LTC SPSA Eval Scaling Constants

Source:

- Public Fishtest-active branch: `https://github.com/TonyCongqianWang/Stockfish`, branch `tune/rm_psqt_calc3`.
- Active-run commit inspected: `7bafc26d1ebac5f371bd59b26ef7a890eb2b2ac5`.
- Fishtest run observed on 2026-06-23 is an LTC tuning/SPSA run, not a direct SPRT pass.

Patch summary:

- The inspected commit retunes `VAL_NNUE_LINEAR`, `VAL_OPT_LINEAR`, `VAL_OPT_QUAD`, and `VAL_OPT_SHAPE`.

Local compatibility check:

- The current `stockfish-20-rc2` source no longer contains these `VAL_*` tuning constants.
- The relevant eval and optimism formulas are already represented differently in `src/evaluate.cpp` and `src/search.cpp`.

Decision:

- Not applied. Forcing the obsolete constants into the current fork would be a new hand-designed patch rather than an isolated upstream/Fishtest experiment.

## Rejected Incremental: Draw Contempt In Search

Source:

- Public Fishtest-active branch: `https://github.com/snicolet/Stockfish`, branch `no_draw2`.
- Isolated commit: `82313e78f2c1907785a5fc9979fbe3c19b9ad5f0`.
- Fishtest active run observed on 2026-06-23 was negative.

Patch summary:

- Added `value_draw_with_contempt(int ply)` returning `+/-200`.
- Used it for immediate draw returns in both main search and qsearch.

Incremental baseline:

- `C:/Users/teamr/Desktop/stockfish/match-results/bin/stockfish-20-rc2-nopgo`

Bench:

- Nodes searched: `2714579`
- Nodes/second: `906370`
- Bench signature changed, so this was treated as a functional search patch.

Smoke result from the `rc2-nopgo` baseline perspective on `tests/openings/smoke.epd` with `TC=0.2+0.002`:

- Seed: `2026062346`
- Games: `64`
- Wins: `24`
- Losses: `19`
- Draws: `21`
- Points: `34.5 / 64`
- Score: `53.91%`
- Elo: `+27.20 +/- 73.89`

Decision:

- Rejected. The local smoke filter favored the current `rc2` baseline, matching the negative direction of the public run.

## Rejected Incremental: Const StateInfo Position Cleanup

Source:

- Public Fishtest-active branch: `https://github.com/ces42/Stockfish`, branch `codex-opt-remarks`.
- Isolated commits tested: `ac875f71` and `7737f75c6eaa85afd2807d8257f350079f4896cd`.
- Fishtest active run observed on 2026-06-23 was negative.

Patch summary:

- Converted `Position::st` and repetition-walk state pointers to `const StateInfo*`.
- Refactored several `Position` helpers to operate on explicit `StateInfo& state`.
- Bench signature was unchanged, so this was treated as a no-functional-change performance patch.

Incremental baseline:

- `C:/Users/teamr/Desktop/stockfish/match-results/bin/stockfish-20-rc2-nopgo`

Bench:

- Nodes searched: `3106469`
- Nodes/second: `875061`
- Bench signature matched the current `rc2` source.

Smoke result from the `rc2-nopgo` baseline perspective on `tests/openings/smoke.epd` with `TC=0.2+0.002`:

- Seed: `2026062347`
- Games: `64`
- Wins: `19`
- Losses: `20`
- Draws: `25`
- Points: `31.5 / 64`
- Score: `49.22%`
- Elo: `-5.43 +/- 67.27`

Decision:

- Rejected. The local smoke result was effectively neutral, local bench speed was not improved, and the public run was negative.

## Rejected After PGO: Optimized Move Prefetch

Source:

- Public Fishtest-active branch: `https://github.com/ces42/Stockfish`, branch `opt-prefetch`.
- Isolated commits tested locally: `f7399c92`, `2832774e`, and `b592d86e8bd8fcb21fc1597ad0c0a2f03efdbd09`.

Patch summary:

- Added an approximate early transposition-table prefetch for each candidate move in the search move loop.
- The patch looked positive as a non-PGO source candidate, but artifact validation did not hold.

Non-PGO local result:

- Combined smoke from the `rc2-nopgo` baseline perspective: `38W / 47L / 43D`, score `46.48%`, favoring the new engine.
- Combined short UHO from the `rc2-nopgo` baseline perspective: `13W / 18L / 33D`, score `46.09%`, favoring the new engine.

PGO artifact:

- Rejected artifact: `C:/Users/teamr/Desktop/stockfish/match-results/bin/stockfish-20-rc4-x86-64-avx512icl`
- SHA256: `BFF4231025DE5011AE788E1E88629F82917176824700BD20AD4E6992D8E21B12`

PGO validation:

- Versus `stockfish-20-rc2-x86-64-avx512icl`, combined UHO from the `rc2` baseline perspective: `28W / 34L / 66D`, score `47.66%`, favoring `rc4`.
- Versus saved current-development baseline `stockfish-base`, UHO from the baseline perspective: `17W / 13L / 34D`, score `53.12%`, favoring the baseline.

Decision:

- Rejected after PGO validation. The source patch was reverted because the final PGO artifact failed the direct saved current-development baseline check, and `stockfish-20-rc2` remains the fork release candidate.

## Rejected Incremental: Improving-Aware ProbCut Depth

Source:

- Official open PR: `https://github.com/official-stockfish/Stockfish/pull/6914`
- Branch tested locally: `https://github.com/Vizvezdenec/Stockfish`, branch `probcutSEDPR`.
- Commit tested: `72443b641fc1e234a3e1a0fb30282fbc312da047`.
- Upstream PR reported passed STC and LTC, so this was worth a local incremental check despite being open.

Patch:

```diff
-        Depth      probCutDepth = depth - 4;
+        Depth      probCutDepth = depth - 4 - improving;
```

Incremental baseline:

- `C:/Users/teamr/Desktop/stockfish/match-results/bin/stockfish-20-rc2-nopgo`

Local result:

- Bench nodes searched: `3140829` over the current local source stack.
- Smoke, seed `2026062357`, from the baseline perspective: `21W / 22L / 21D`, score `49.22%`, slightly favoring the candidate.
- UHO seed `2026062358`, from the baseline perspective: `19W / 17L / 28D`, score `51.56%`, favoring the baseline.
- UHO seed `2026062359`, from the baseline perspective: `11W / 18L / 35D`, score `44.53%`, favoring the candidate.
- UHO seed `2026062360`, from the baseline perspective: `21W / 13L / 30D`, score `56.25%`, favoring the baseline.
- Combined UHO: `51W / 48L / 93D`, score `50.78%` for the baseline.

Decision:

- Rejected locally as an incremental patch over `rc2`. The result was mixed and the combined UHO set slightly favored the current release-candidate source, so this patch was removed.

## Rejected Incremental: AVX512 Move Rank Buffer

Source:

- Official open PR: `https://github.com/official-stockfish/Stockfish/pull/6678`
- Upstream title: `Variant of PR6672 (Use avx512 in move rank)`
- Isolated commit tested locally: `bf3adec0a281bac56782e9de6e1815458da85428`

Patch summary:

- Added `alignas(32)` to `MultiArray`.
- Added an AVX-512-only quiet-history buffer in `MovePicker::score()` to precompute combined history values for each `(piece, square)` pair.

Local result:

- The upstream PR branch was dirty against current master, but the isolated commit touched only `src/misc.h` and `src/movepick.cpp`.
- It applied with one local `misc.h` type-convention conflict because this fork uses `usize` where the PR used `std::size_t`.
- Build passed with `ARCH=native`.
- Bench nodes searched: `3106469`.
- Smoke seed `2026062363`, from the `rc2-nopgo` baseline perspective: `28W / 16L / 20D`, score `59.38%`, Elo `+65.92 +/- 62.63`.

Decision:

- Rejected. The smoke filter clearly favored the current `stockfish-20-rc2` source, so no UHO or PGO follow-up was run.

## Rejected Incremental: Early TT Prefetch

Source:

- Official open PR: `https://github.com/official-stockfish/Stockfish/pull/6911`
- Upstream title: `prefetch the TT entry as soon as the move is known`
- Branch tested locally: `https://github.com/dhanaway/Stockfish`, branch `early-tt-prefetch`
- Head commit inspected: `eafe4208afc37e01eddc7659e56074e029d3e6ad`

Patch summary:

- Added `Position::prefetch_key(Move)` and used it in `Search::Worker::do_move()` to prefetch the child transposition-table entry before `Position::do_move()`.
- The PR is upstream-clean and reported public bench speedups, but it is in the same broad prefetch risk area as the already rejected `opt-prefetch` candidate.

Local result:

- Build passed with `ARCH=native`.
- Bench nodes searched: `3106469`.
- Bench nodes/second: `884026`.
- Smoke seed `2026062364`, from the `rc2-nopgo` baseline perspective: `23W / 18L / 23D`, score `53.91%`, Elo `+27.20 +/- 61.41`.

Decision:

- Rejected as a local incremental patch over `rc2`. The smoke filter favored the current release-candidate source, so no UHO or PGO follow-up was run.

## Not Applied: Quiet King Threat Move Generation

Source:

- Official open PR: `https://github.com/official-stockfish/Stockfish/pull/6288`
- Upstream title: `Use threats in quiet move generation and simplify castling check in pos.legal()`
- Branch inspected locally: `https://github.com/ces42/Stockfish`, branch `qks-rebase`
- Head commit inspected: `7012ded3994336176aa8affb50957d8f4809c5f1`

Public evidence:

- Upstream PR body reports passed STC with LLR `2.97 (-2.94,2.94) <-1.75,0.25>`.
- Upstream PR body reports passed LTC with LLR `2.95 (-2.94,2.94) <-1.75,0.25>`.

Local applicability check:

- GitHub marks the PR as `dirty`, and maintainers noted it is outdated.
- The full branch diff against current master is dominated by base drift, but the isolated patch stack reduces to six source files: `movegen.cpp`, `movegen.h`, `movepick.cpp`, `movepick.h`, `position.cpp`, and `position.h`.
- Attempting the isolated stack over the current `stockfish-20-rc2` source conflicted immediately in `src/movepick.cpp` on the quiet king threat path.

Decision:

- Not applied. Because the PR is stale/dirty and the local stack does not apply cleanly, adapting it manually would create a new unproven patch rather than testing the upstream candidate. Revisit only if upstream rebases it or if a dedicated local rewrite is planned with full validation.

## Rejected Incremental: Raw Reduction Value

Source:

- Official open PR: `https://github.com/official-stockfish/Stockfish/pull/6577`
- Upstream title: `Simplification - Use raw reduction value`
- Branch tested locally: `https://github.com/FauziAkram/Stockfish`, branch `prrisPR`
- Isolated commit tested locally: `cd807ff2811fbd22e7d293124edbe05b8d127047`

Public evidence:

- Upstream PR body reports passed STC with LLR `2.95 (-2.94,2.94) <-1.75,0.25>`.
- Upstream PR body reports passed LTC with LLR `2.94 (-2.94,2.94) <-1.75,0.25>`.

Patch summary:

- Store the raw reduction value in `ss->reduction` instead of the clamped depth delta.
- Compare hindsight and IIR thresholds against raw reduction units (`3200`, `2000`, `3072`) instead of reduced-depth plies.
- The local application kept the fork's existing `!ss->followPV` guard in the IIR condition.

Local result:

- Build passed with `ARCH=native`.
- Bench nodes searched: `2442842`.
- Bench nodes/second: `874004`.
- Smoke seed `2026062365`, from the `rc2-nopgo` baseline perspective: `25W / 20L / 19D`, score `53.91%`, Elo `+27.20 +/- 80.25`.

Decision:

- Rejected as a local incremental patch over `rc2`. Despite strong public STC/LTC evidence on the upstream base, the local smoke filter favored the current release-candidate source, so no UHO or PGO follow-up was run.

## Current Status

- One source-level strength patch has been accepted beyond using the stronger post-Stockfish-18 official development baseline: lazy simple evaluation shortcut.
- The fork release candidate remains `stockfish-20-rc2`.
- Future experiments should target different mechanisms instead of retuning these local history constants.
