#!/usr/bin/env bash
set -eu

repo_root=$(cd "$(dirname "$0")/.." && pwd)
parent_dir=$(cd "$repo_root/.." && pwd)

fastchess=${FASTCHESS:-"$parent_dir/fastchess/fastchess"}
base_engine=${BASE_ENGINE:-"$repo_root/src/stockfish"}
new_engine=${NEW_ENGINE:-"$repo_root/src/stockfish"}
openings=${OPENINGS:-"$repo_root/tests/openings/smoke.epd"}
out_dir=${OUT_DIR:-"$parent_dir/match-results"}
rounds=${ROUNDS:-4}
concurrency=${CONCURRENCY:-2}
tc=${TC:-"0.2+0.002"}
nodes=${NODES:-}
hash=${HASH:-16}
threads=${THREADS:-1}
seed=${SEED:-20260623}

mkdir -p "$out_dir"

limit_args=(tc="$tc")
if [[ -n "$nodes" ]]; then
  limit_args=(nodes="$nodes")
fi

"$fastchess" \
  -engine name=Base cmd="$base_engine" dir="$repo_root" \
  -engine name=New cmd="$new_engine" dir="$repo_root" \
  -each proto=uci "${limit_args[@]}" option.Hash="$hash" option.Threads="$threads" \
  -openings file="$openings" format=epd order=random \
  -srand "$seed" \
  -rounds "$rounds" \
  -repeat \
  -concurrency "$concurrency" \
  -ratinginterval 1 \
  -scoreinterval 1 \
  -report penta=true \
  -resign movecount=3 score=600 \
  -draw movenumber=34 movecount=8 score=20 \
  -pgnout file="$out_dir/fastchess-smoke.pgn"
