#!/usr/bin/env bash
set -eu

repo_root=$(cd "$(dirname "$0")/.." && pwd)
parent_dir=$(cd "$repo_root/.." && pwd)

fastchess=${FASTCHESS:-"$parent_dir/fastchess/fastchess"}
base_engine=${BASE_ENGINE:-"$parent_dir/match-results/bin/stockfish-base"}
new_engine=${NEW_ENGINE:-"$repo_root/src/stockfish"}
out_dir=${OUT_DIR:-"$parent_dir/match-results"}
rounds=${ROUNDS:-64}
concurrency=${CONCURRENCY:-4}
nodes=${NODES:-20000}
hash=${HASH:-16}
threads=${THREADS:-1}
seed=${SEED:-20260623}
sample_size=${SAMPLE_SIZE:-256}

openings=$("$repo_root/tests/prepare_official_openings.sh")

mkdir -p "$out_dir"

"$fastchess" \
  -engine name=Base cmd="$base_engine" dir="$repo_root" \
  -engine name=New cmd="$new_engine" dir="$repo_root" \
  -each proto=uci nodes="$nodes" option.Hash="$hash" option.Threads="$threads" \
  -openings file="$openings" format=epd order=random \
  -srand "$seed" \
  -rounds "$rounds" \
  -repeat \
  -concurrency "$concurrency" \
  -ratinginterval 16 \
  -scoreinterval 16 \
  -report penta=true \
  -resign movecount=3 score=600 \
  -draw movenumber=34 movecount=8 score=20 \
  -pgnout file="$out_dir/fastchess-official-uho-${sample_size}.pgn"
