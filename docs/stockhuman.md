# StockHuman

StockHuman is a human-like opponent layer for this Stockfish fork. It is still
heuristic, not a trained neural human policy, but it is no longer just "pick a
weaker MultiPV line plus a delay". The current design tries to approximate a
strong club player: natural moves, plausible misses, bounded blunders, clock
pressure, fatigue and position-dependent thinking time.

Stockfish remains the hidden judge. StockHuman only activates when the UCI
option `StockHuman` is true. Classical Stockfish search and move output remain
unchanged when that option is false.

## What changed from the first prototype

The first prototype mostly used candidate rank, evaluation loss, style and a
post-search sleep. That made two bad behaviors possible:

1. It could feel engine-like because the best line was still selected too often.
2. It could spend real Stockfish time before applying a human delay, so obvious
   recaptures or forced moves could still be slow.

The current model adds:

- A root-position profile: opening/middlegame/endgame, forcing move density,
  close alternatives, obvious tactics, hidden quiet engine resources and clock
  pressure.
- A move profile: safe captures, checks, castling, promotions, development,
  center play, early queen/rook moves, hanging-piece risk and endgame king
  activity.
- A bounded stochastic selector: it samples among plausible searched moves, but
  filters out absurd losses for 1800+ play unless a rare mood/pressure window is
  allowed.
- A StockHuman search cap: under normal clock controls the search itself stops
  around the human response target instead of searching for Stockfish's full
  time allocation.
- Debug lines with machine-readable fields for calibration:
  `StockHuman decision ...` and `StockHuman time ...`.

## Recommended 1800-2000 settings

For a strong but fallible human opponent:

```text
setoption name StockHuman value true
setoption name HumanElo value 1900
setoption name HumanStyle value Balanced
setoption name HumanConsistency value 80
setoption name HumanTilt value 10
setoption name HumanThinkTime value 100
setoption name HumanOpeningKnowledge value 72
setoption name HumanAdaptation value 20
setoption name HumanDebug value true
```

For a cleaner 2000-ish player, raise `HumanConsistency` to `84..88`, lower
`HumanTilt` to `6..9`, and use `Solid` or `Positional`. For a sharper 1800-ish
player, use `Tactical`, `HumanConsistency 74..78`, `HumanTilt 12..18`.

## UCI options

`StockHuman`
: Enables the StockHuman layer. Default: `false`.

`HumanElo`
: Target human level. Default: `1500`, range `800..3000`.

`HumanStyle`
: Personality prior. Values: `Balanced`, `Solid`, `Aggressive`, `Tactical`,
  `Positional`.

`HumanConsistency`
: Higher values reduce drift from top candidates and reduce random-looking
  misses. Default: `72`, range `0..100`.

`HumanTilt`
: Higher values increase risk, mood windows and time-pressure mistakes.
  Default: `12`, range `0..100`.

`HumanThinkTime`
: Scales the whole human time model. `0` disables StockHuman timing, `45` is
  quick, `100` normal, `165` slow. Default: `100`.

`HumanOpeningKnowledge`
: Makes early known-looking moves quicker and more natural. Default: `65`.

`HumanAdaptation`
: Loosens the player when already winning and tightens it when worse.
  Default: `25`.

`HumanRandomSeed`
: `0` means nondeterministic. Any non-zero value makes the same position and
  settings reproducible.

`HumanPolicyFile`
: Optional external policy prior file. Empty by default.

`HumanPolicyMix`
: Blends the external policy prior into the heuristic selector. `0` disables it.

`HumanDebug`
: Prints `info string` lines with decision and time fields.

## External policy format

No Maia or Lichess code is integrated. The optional file is only a simple
interface for a future data-driven policy.

Each non-comment line has:

```text
<fen board> <side> <castling> <ep> | <uciMove> <weight> <uciMove> <weight> ...
```

Example:

```text
rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - | e2e4 0.31 d2d4 0.29 g1f3 0.17 c2c4 0.11
```

The key intentionally ignores halfmove and fullmove counters. A future training
pipeline can produce this file from Lichess/OTB-like games grouped by Elo,
clock, phase and style, then set `HumanPolicyMix` to blend it with Stockfish's
quality filter.

## Time model

StockHuman time is position-aware:

- Single legal moves, safe recaptures, obvious promotions and early book-like
  development are fast.
- Critical positions with many close candidates, checks/captures or hidden
  engine resources are slower.
- Endgames without forcing moves become more hesitant.
- Low clock increases pressure, shortens thinking time and increases plausible
  mistake rate.
- The search cap is tied to the same target, so Stockfish cannot silently spend
  a long engine allocation before the human delay is applied.

Use normal UCI time controls (`go wtime ... btime ... winc ... binc ...`) to
exercise this model. `go depth`, `go nodes`, `go mate`, `go movetime`,
`go infinite` and ponder mode bypass StockHuman timing.

## Arena

The local arena is in `playground/stockhuman-arena`.

```text
cd playground/stockhuman-arena
npm start
```

Open `http://localhost:5177`. The UI has presets for `1200`, `1600`, `1800`,
`2000`, `2200`, and speed buttons `Quick`, `Normal`, `Slow`. The debug panel
shows elapsed time plus the StockHuman decision/time lines.

For the Chess.com bridge in maximum-strength continuous mode:

```text
cd playground/stockhuman-arena
npm run continuous
```

The bridge defaults to unhandicapped Stockfish with 8 threads, 1024 MB hash,
and 4000 ms per move. It starts automatically after injection, retries a move
whose click was not accepted, detects game-over dialogs, and activates a
visible rematch/new-game control. The PowerShell supervisor restarts the Node
server and UCI engine after an unexpected exit. These defaults can be changed
with `-Threads`, `-HashMb`, `-MoveMs`, and `-Port` on
`scripts/start-continuous.ps1`.

## Calibration

Run the smoke test:

```text
python tests/stockhuman_uci_smoke.py src/stockfish-stockhuman.exe
```

Run the calibration suite:

```text
python tests/stockhuman_calibration.py src/stockfish-stockhuman.exe
```

The calibration suite checks:

- Natural opening choices.
- No absurd 1800-2000 opening moves.
- Simple tactical or material responses are not missed.
- Calm positions can choose natural non-engine moves without huge eval loss.
- Obvious responses get low target time.
- Complex/endgame positions get more thinking time than obvious recaptures.

These tests are guardrails, not proof of Elo. Real validation still needs game
matches against humans and large PGN comparison.

## Honest limits

StockHuman is not yet Maia. It does not learn the opponent, does not have a real
opening repertoire, does not model premove habits, and still depends on
Stockfish candidates. The heuristic can be tuned and tested, but the final
human feel should eventually come from a trained move policy plus a trained
clock model, with Stockfish kept as a safety judge.
