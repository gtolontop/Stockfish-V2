#!/usr/bin/env python3
"""Small UCI smoke test for the StockHuman prototype.

Usage:
    python tests/stockhuman_uci_smoke.py path/to/stockfish
"""

from __future__ import annotations

import queue
import re
import subprocess
import sys
import threading
import time
from pathlib import Path


def reader_thread(stream, out_queue: "queue.Queue[str]") -> None:
    for line in iter(stream.readline, ""):
        out_queue.put(line.rstrip("\n"))


def send(proc: subprocess.Popen[str], command: str) -> None:
    assert proc.stdin is not None
    proc.stdin.write(command + "\n")
    proc.stdin.flush()


def read_until(out_queue: "queue.Queue[str]", pattern: str, timeout: float = 10.0) -> list[str]:
    deadline = time.monotonic() + timeout
    lines: list[str] = []
    compiled = re.compile(pattern)

    while time.monotonic() < deadline:
        try:
            line = out_queue.get(timeout=0.1)
        except queue.Empty:
            continue

        lines.append(line)
        if compiled.search(line):
            return lines

    raise TimeoutError(f"Timed out waiting for {pattern!r}. Last lines: {lines[-12:]}")


def main() -> int:
    if len(sys.argv) != 2:
        print(__doc__.strip(), file=sys.stderr)
        return 2

    engine = Path(sys.argv[1])
    if not engine.exists():
        print(f"Engine not found: {engine}", file=sys.stderr)
        return 2

    proc = subprocess.Popen(
        [str(engine)],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        bufsize=1,
    )

    assert proc.stdout is not None
    out_queue: "queue.Queue[str]" = queue.Queue()
    thread = threading.Thread(target=reader_thread, args=(proc.stdout, out_queue), daemon=True)
    thread.start()

    try:
        send(proc, "uci")
        uci_lines = read_until(out_queue, r"^uciok$", timeout=10.0)

        required_options = [
            "option name StockHuman",
            "option name HumanElo",
            "option name HumanStyle",
            "option name HumanThinkTime",
            "option name HumanAdaptation",
            "option name HumanPolicyFile",
            "option name HumanPolicyMix",
        ]
        missing = [opt for opt in required_options if not any(opt in line for line in uci_lines)]
        if missing:
            raise AssertionError(f"Missing UCI options: {missing}")

        send(proc, "setoption name StockHuman value true")
        send(proc, "setoption name HumanElo value 1400")
        send(proc, "setoption name HumanStyle value Tactical")
        send(proc, "setoption name HumanThinkTime value 0")
        send(proc, "setoption name HumanAdaptation value 35")
        send(proc, "setoption name HumanRandomSeed value 123")
        send(proc, "setoption name HumanDebug value true")
        send(proc, "isready")
        read_until(out_queue, r"^readyok$", timeout=10.0)

        send(proc, "position startpos moves e2e4 e7e5 g1f3 b8c6")
        send(proc, "go depth 4")
        move_lines = read_until(out_queue, r"^bestmove\s+[a-h][1-8][a-h][1-8][qrbn]?", timeout=20.0)

        if not any("info string StockHuman decision" in line for line in move_lines):
            raise AssertionError("HumanDebug did not emit StockHuman decision info")

    finally:
        if proc.poll() is None:
            send(proc, "quit")
            try:
                proc.wait(timeout=5.0)
            except subprocess.TimeoutExpired:
                proc.kill()

    print("StockHuman UCI smoke test passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
