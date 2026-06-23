# Stockfish 20 Test Plan

Date: 2026-06-23

## Baseline Definition

The local fork currently points at official development commit `74a0a737`, tagged by upstream as `stockfish-dev-20260614-74a0a737`.

Important consequence:

- Official release `sf_18` is not the real local baseline.
- The local baseline already includes roughly 195 official commits after `sf_18`.
- Stockfish 20 candidate changes must therefore be measured against `74a0a737`, not only against `sf_18`.

## Remote Safety

Configured remotes:

- `origin`: fork, `https://github.com/gtolontop/Stockfish-V2`
- `official`: official upstream, fetch-only

The `official` push URL is explicitly set to `DISABLED`.

## Repository Inventory From Official GitHub Org

Official organization repositories checked on 2026-06-23:

- `.github`: organization profile and metadata.
- `Stockfish`: C++ engine.
- `fishtest`: distributed testing framework.
- `nnue-pytorch`: NNUE trainer.
- `docs`: generated documentation.
- `stockfish-web`: official website.
- `books`: opening/test books.
- `networks`: network storage/distribution.
- `WDL_model`: win/draw/loss modeling.
- `docker-fishtest`: Docker support for Fishtest.
- `stockfish-wiki-bot`: wiki/doc automation.

## Local Tooling Status

Available:

- WSL Ubuntu.
- `make`.
- `g++`.
- Python.

Not found in current PATH:

- `fastchess`.
- `cutechess-cli`.
- `c-chess-cli`.
- `ordo`.

## Testing Ladder

Use progressively stronger tests. Do not jump straight to long matches for every tiny idea.

1. Compile check

```bash
cd /mnt/c/Users/teamr/Desktop/stockfish/Stockfish-V2/src
make -j"$(nproc)" build ARCH=native
```

2. Bench smoke check

```bash
./stockfish bench
```

Capture the `Nodes searched` line as the signature.

3. Fixed-depth sanity check

```bash
./stockfish bench 16 1 13 default depth
```

Current reference result for this command:

- Nodes searched: `3493826`
- Nodes/second: `740374`

4. Short engine-vs-engine test

Preferred once installed:

- `fastchess` with paired openings, alternating colors, fixed hash and threads.
- Alternative: `cutechess-cli`.
- Fallback: local Python UCI harness for smoke match testing only.

Local scripts:

```bash
# Tiny built-in smoke book
bash tests/run_fastchess_smoke.sh

# Deterministic sample from official-stockfish/books UHO_Lichess_4852_v1
bash tests/run_fastchess_official.sh
```

The official-book runner expects `official-stockfish/books` cloned next to this repository:

```bash
git clone https://github.com/official-stockfish/books.git ../books
```

5. Promotion test

Before branding a release candidate, run a larger paired match with:

- Same binary architecture.
- Same network.
- Same hash.
- Same thread count.
- Alternating colors.
- Repeated opening positions.
- Saved PGN and summary.

## Candidate Evaluation Rules

- Never trust a 10-game result as proof of strength.
- A 10-0 mini-match is useful only as a smoke signal, not as Elo proof.
- Keep functional changes tiny and test one idea at a time.
- Revert or discard ideas that fail compile, bench, or short-match sanity.
- Commit only a coherent tested change, with an English message.

## Immediate Next Steps

1. Add or install a repeatable match harness.
2. Save baseline binary outside tracked source files.
3. Make one tiny search or parameter experiment.
4. Build candidate binary.
5. Run baseline vs candidate.
6. Commit only if the result is not obviously worse.
