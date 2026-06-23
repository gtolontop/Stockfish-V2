#!/usr/bin/env bash
set -eu

repo_root=$(cd "$(dirname "$0")/.." && pwd)
parent_dir=$(cd "$repo_root/.." && pwd)

books_dir=${BOOKS_DIR:-"$parent_dir/books"}
book_zip=${BOOK_ZIP:-"$books_dir/UHO_Lichess_4852_v1.epd.zip"}
sample_size=${SAMPLE_SIZE:-256}
out_dir=${OUT_DIR:-"$parent_dir/match-results/openings"}
out_file=${OUT_FILE:-"$out_dir/uho_lichess_4852_sample_${sample_size}.epd"}

if [ ! -f "$book_zip" ]; then
  echo "Missing official book zip: $book_zip" >&2
  echo "Clone https://github.com/official-stockfish/books.git to $books_dir first." >&2
  exit 1
fi

mkdir -p "$out_dir"

python3 - "$book_zip" "$sample_size" "$out_file" <<'PY'
import sys
import zipfile

zip_path = sys.argv[1]
sample_size = int(sys.argv[2])
out_path = sys.argv[3]

if sample_size <= 0:
    raise SystemExit("SAMPLE_SIZE must be positive")

with zipfile.ZipFile(zip_path) as zf:
    names = [name for name in zf.namelist() if name.lower().endswith(".epd")]
    if len(names) != 1:
        raise SystemExit(f"Expected one EPD in {zip_path}, found {names}")
    name = names[0]

    with zf.open(name) as source:
        total = sum(1 for _ in source)

    wanted = min(sample_size, total)
    if wanted == 0:
        raise SystemExit(f"No positions found in {zip_path}")

    targets = {(i * total) // wanted for i in range(wanted)}

    written = 0
    with zf.open(name) as source, open(out_path, "wb") as out:
        for idx, line in enumerate(source):
            if idx in targets:
                out.write(line)
                written += 1
                if written == wanted:
                    break

print(f"Wrote {written} / {total} positions to {out_path}", file=sys.stderr)
PY

echo "$out_file"
