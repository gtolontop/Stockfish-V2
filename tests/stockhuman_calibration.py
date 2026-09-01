#!/usr/bin/env python3
"""Calibration guardrails for StockHuman.

This is not an Elo proof. It checks that the human layer behaves sanely on a
small set of tactical, calm, opening and timing positions.

Usage:
    python tests/stockhuman_calibration.py path/to/stockfish
"""

from __future__ import annotations

import dataclasses
import queue
import re
import subprocess
import sys
import threading
import time
from pathlib import Path


@dataclasses.dataclass(frozen=True)
class Case:
    name: str
    position: str
    go: str
    accepted: set[str] | None = None
    rejected: set[str] | None = None
    max_loss: int | None = None
    max_target: int | None = None
    min_target: int | None = None


QUALITY_CASES = [
    Case(
        name="natural_start",
        position="startpos",
        go="go depth 8",
        accepted={"e2e4", "d2d4", "g1f3", "c2c4"},
        max_loss=180,
    ),
    Case(
        name="do_not_play_absurd_opening",
        position="startpos",
        go="go depth 8",
        rejected={"a2a4", "h2h4", "g2g4", "b1a3", "g1h3"},
        max_loss=220,
    ),
    Case(
        name="simple_mate",
        position="fen r1bqkbnr/pppp1ppp/2n5/4p2Q/2B1P3/8/PPPP1PPP/RNB1K1NR w KQkq - 4 4",
        go="go depth 8",
        accepted={"h5f7"},
        max_loss=0,
    ),
    Case(
        name="obvious_recapture_quality",
        position="fen r1bqkbnr/1ppp1ppp/p1B5/4p3/4P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 0 4",
        go="go depth 8",
        accepted={"d7c6", "b7c6"},
        max_loss=120,
    ),
]


TIME_CASES = [
    Case(
        name="obvious_recapture_fast",
        position="fen r1bqkbnr/1ppp1ppp/p1B5/4p3/4P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 0 4",
        go="go wtime 300000 btime 300000 winc 2000 binc 2000",
        accepted={"d7c6", "b7c6"},
        max_target=1300,
    ),
    Case(
        name="technical_endgame_hesitates",
        position="fen 8/5pk1/6p1/3K4/8/6P1/5P2/8 w - - 0 45",
        go="go wtime 300000 btime 300000 winc 2000 binc 2000",
        min_target=1500,
    ),
    Case(
        name="low_clock_caps_thinking",
        position="fen r2q1rk1/pp2bppp/2n1bn2/2pp4/3P4/2PBPN2/PPQN1PPP/R1B2RK1 w - - 2 10",
        go="go wtime 7000 btime 7000 winc 1000 binc 1000",
        max_target=950,
    ),
]


def reader_thread(stream, out_queue: "queue.Queue[str]") -> None:
    for line in iter(stream.readline, ""):
        out_queue.put(line.rstrip("\n"))


def send(proc: subprocess.Popen[str], command: str) -> None:
    assert proc.stdin is not None
    proc.stdin.write(command + "\n")
    proc.stdin.flush()


def read_until(out_queue: "queue.Queue[str]", pattern: str, timeout: float = 30.0) -> list[str]:
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

    raise TimeoutError(f"Timed out waiting for {pattern!r}. Last lines: {lines[-16:]}")


def parse_field(lines: list[str], field: str) -> int | None:
    pattern = re.compile(rf"\b{re.escape(field)}=(-?\d+)")
    for line in lines:
        match = pattern.search(line)
        if match:
            return int(match.group(1))
    return None


def bestmove(lines: list[str]) -> str:
    for line in reversed(lines):
        if line.startswith("bestmove "):
            return line.split()[1]
    raise AssertionError(f"No bestmove in lines: {lines[-12:]}")


def run_case(proc: subprocess.Popen[str], out_queue: "queue.Queue[str]", case: Case) -> None:
    send(proc, "ucinewgame")
    send(proc, f"position {case.position}")
    send(proc, case.go)
    lines = read_until(out_queue, r"^bestmove\s+", timeout=45.0)

    move = bestmove(lines)
    loss = parse_field(lines, "loss")
    target = parse_field(lines, "target")

    if not any("info string StockHuman decision" in line for line in lines):
        raise AssertionError(f"{case.name}: missing StockHuman decision debug")

    if case.accepted is not None and move not in case.accepted:
        raise AssertionError(f"{case.name}: move {move} not in accepted set {sorted(case.accepted)}")

    if case.rejected is not None and move in case.rejected:
        raise AssertionError(f"{case.name}: move {move} is explicitly rejected")

    if case.max_loss is not None:
        if loss is None:
            raise AssertionError(f"{case.name}: missing loss field")
        if loss > case.max_loss:
            raise AssertionError(f"{case.name}: loss {loss} > {case.max_loss}")

    if case.max_target is not None:
        if target is None:
            raise AssertionError(f"{case.name}: missing target field")
        if target > case.max_target:
            raise AssertionError(f"{case.name}: target {target} > {case.max_target}")

    if case.min_target is not None:
        if target is None:
            raise AssertionError(f"{case.name}: missing target field")
        if target < case.min_target:
            raise AssertionError(f"{case.name}: target {target} < {case.min_target}")

    print(f"{case.name}: move={move} loss={loss} target={target}")


def configure(proc: subprocess.Popen[str], out_queue: "queue.Queue[str]", think_time: int) -> None:
    commands = [
        "setoption name Threads value 1",
        "setoption name Hash value 16",
        "setoption name StockHuman value true",
        "setoption name HumanElo value 1900",
        "setoption name HumanStyle value Balanced",
        "setoption name HumanConsistency value 82",
        "setoption name HumanTilt value 10",
        f"setoption name HumanThinkTime value {think_time}",
        "setoption name HumanOpeningKnowledge value 72",
        "setoption name HumanAdaptation value 20",
        "setoption name HumanPolicyMix value 0",
        "setoption name HumanRandomSeed value 1900",
        "setoption name HumanDebug value true",
        "isready",
    ]
    for command in commands:
        send(proc, command)
    read_until(out_queue, r"^readyok$", timeout=10.0)


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
        for option in ("HumanPolicyFile", "HumanPolicyMix"):
            if not any(f"option name {option}" in line for line in uci_lines):
                raise AssertionError(f"Missing UCI option {option}")

        configure(proc, out_queue, think_time=0)
        for case in QUALITY_CASES:
            run_case(proc, out_queue, case)

        configure(proc, out_queue, think_time=100)
        for case in TIME_CASES:
            run_case(proc, out_queue, case)

    finally:
        if proc.poll() is None:
            send(proc, "quit")
            try:
                proc.wait(timeout=5.0)
            except subprocess.TimeoutExpired:
                proc.kill()

    print("StockHuman calibration checks passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
