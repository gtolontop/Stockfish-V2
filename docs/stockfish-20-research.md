# Stockfish 20 Fork Research Notes

Date: 2026-06-23
Branch: `fix/stockfish-20-release`
Fork remote: `https://github.com/gtolontop/Stockfish-V2`

## Source Boundaries

- Work only on the fork remote, never on `official-stockfish/Stockfish`.
- Use official Stockfish, Fishtest, and docs pages as primary sources.
- Treat `Stockfish 20` as this fork's release name. The latest public official release found during research is Stockfish 18, announced on 2026-01-31.

## Official Project Map

- Main engine: `official-stockfish/Stockfish`, C++ UCI engine.
- Test framework: `official-stockfish/fishtest`, distributed engine testing.
- Neural net trainer: `official-stockfish/nnue-pytorch`, PyTorch NNUE training.
- Docs: `official-stockfish/docs`, generated documentation site.
- Website: `official-stockfish/stockfish-web`.
- Related tooling: `WDL_model` for win/draw/loss modeling.

## Current Strength Direction

Stockfish 18 highlights point at the main high-value areas:

- NNUE architecture changes: SFNNv10 introduced threat-input features.
- Search refinements: correction history, fortress/stalemate handling, repetition edge-case fixes.
- Hardware efficiency: shared memory for NNUE weights, modern CPU instruction paths, thread interaction improvements.
- Training workflow: automated and reproducible NNUE training with very large Lc0-derived position datasets.

For this fork, the practical short-term improvement route is search and parameter experiments with strict tests. Full NNUE training is possible but much heavier because it needs data, trainer setup, net conversion, net naming, and many games.

## GUI And Test Harness Options

The official docs list several GUIs:

- Free desktop: En Croissant, Nibbler, Arena, Lichess Local Engine, BanksiaGUI, Cutechess, ChessX, LiGround, Lucas Chess, Scid vs. PC, XBoard, jose, JFXChess.
- Mobile: DroidFish, SmallFish, Chessis.
- Paid: ChessBase, Hiarcs, Shredder.
- Online: Lichess, Chess.com, ChessMonitor, Chessify, DecodeChess.

For automated engine-vs-engine testing, GUI convenience matters less than repeatable CLI results. Preferred local testing stack:

- Build engine with WSL or MSYS2.
- Use Stockfish `bench` for signature/smoke checks.
- Use fastchess or Cute Chess CLI for match testing.
- Use Fishtest methodology as the statistical reference.

## Fishtest Methodology Notes

- Functional search changes must be verified statistically; Stockfish's own docs say functional patches change the search tree and must be verified by Fishtest.
- Contributor guidance favors small, atomic changes and one test per idea.
- Current public regression criteria include 1-thread and 8-thread tests at `60+0.6` for 60,000 games, using UHO opening books.
- SPSA is used for parameter tuning. Candidate parameters need exposed tune ranges, enough games, and care with `ck`/`rk` so the values actually move.
- A branch signature comes from `bench`, normally the `Nodes searched` line.

## Local Build Baseline

Environment:

- Windows workspace with WSL Ubuntu available.
- WSL has `/usr/bin/make` and `/usr/bin/g++`.
- Native build selected `x86-64-avx512icl`.
- The build downloaded and validated `nn-71d6d32cb962.nnue`.

Baseline smoke command:

```bash
cd /mnt/c/Users/teamr/Desktop/stockfish/Stockfish-V2/src
make -j build ARCH=native
./stockfish bench 16 1 13 default depth
```

Baseline result:

- Engine id: `Stockfish dev-20260614-74a0a737`
- Nodes searched: `3493826`
- Nodes/second: `740374`
- Total time: `4719 ms`

## Release Plan For This Fork

1. Keep a clean branch with small English commits.
2. Add local repeatable testing scripts before risky engine changes.
3. Build a baseline binary and a candidate binary from each idea.
4. Run fast smoke tests after every code change.
5. Run short matches before committing functional changes.
6. Promote only changes that compile, keep bench sane, and improve or at least do not regress local match results.
7. Use version branding only after the engine changes and test logs are stable.

## Initial Experiment Backlog

- Add a local match harness around fastchess or Cute Chess CLI.
- Pull official upstream as a fetch-only remote for comparison, without pushing there.
- Compare fork branch against official latest master and official Stockfish 18 release.
- Investigate recent official commits after Stockfish 18 for search ideas already validated upstream.
- Try tiny, isolated search parameter experiments first; avoid giant bundled changes.
- Only consider NNUE training after the local match harness is reliable.

## Sources

- Official site: https://stockfishchess.org/
- Stockfish 18 release post: https://stockfishchess.org/blog/2026/stockfish-18/
- Download and GUI docs: https://official-stockfish.github.io/docs/stockfish-wiki/Download-and-usage.html
- Compiling docs: https://official-stockfish.github.io/docs/stockfish-wiki/Compiling-from-source.html
- Developer docs: https://official-stockfish.github.io/docs/stockfish-wiki/Developers.html
- Regression tests docs: https://official-stockfish.github.io/docs/stockfish-wiki/Regression-Tests.html
- Creating Fishtest tests: https://official-stockfish.github.io/docs/fishtest-wiki/Creating-my-first-test.html
- Fishtest mathematics: https://official-stockfish.github.io/docs/fishtest-wiki/Fishtest-Mathematics.html
- Official GitHub org: https://github.com/official-stockfish
- Fishtest repo: https://github.com/official-stockfish/fishtest
- NNUE trainer repo: https://github.com/official-stockfish/nnue-pytorch
