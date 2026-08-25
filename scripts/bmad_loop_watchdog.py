#!/usr/bin/env python3
"""Best-effort supervisor for one-story bmad-loop runs.

The bmad-loop engine is intentionally conservative: a blocked story can pause a
serial sprint, and a coding CLI that never emits a Stop event can remain live
without giving the engine a verdict. This supervisor trades strict completion
for queue progress. It runs one explicitly selected story at a time, restarts a
stalled child a bounded number of times, and records stories it skips after the
retry budget is exhausted.

It never edits sprint-status.yaml. A skipped story is recorded in the watchdog
state file and remains visible on the sprint board for later repair.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import subprocess
import sys
import time
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


RUN_RE = re.compile(r"\brun\s+(20\d{6,}-[0-9a-f]+)\s+starting\b")
DRY_RUN_STORY_RE = re.compile(r"^\s{2}(\d+-\d+-[a-z0-9][a-z0-9-]*)\s+\(epic\s+\d+,\s+status\s+[^)]+\)")
STATUS_STORY_RE = re.compile(r"^\s{2}(\d+-\d+-[a-z0-9][a-z0-9-]*):\s*([^\s#]+)")
TERMINAL_STORY_STATUSES = {"done", "blocked", "awaiting-operator"}


def now() -> str:
    return datetime.now(timezone.utc).isoformat(timespec="seconds")


class Watchdog:
    def __init__(self, project: Path, args: argparse.Namespace) -> None:
        self.project = project.resolve()
        self.args = args
        self.runtime_dir = self.project / ".bmad-loop" / "watchdog"
        self.runtime_dir.mkdir(parents=True, exist_ok=True)
        self.state_path = self.runtime_dir / "state.json"
        self.log_path = self.runtime_dir / "watchdog.log"
        self.state = self.load_state()
        self.active_run_id: str | None = None
        self.active_process: subprocess.Popen[str] | None = None

    def log(self, message: str) -> None:
        line = f"[{now()}] {message}"
        print(line, flush=True)
        with self.log_path.open("a", encoding="utf-8") as handle:
            handle.write(line + "\n")

    def load_state(self) -> dict[str, Any]:
        if self.args.reset and self.state_path.exists():
            self.state_path.unlink()
        if not self.state_path.exists():
            return {"version": 1, "attempts": {}, "skipped": {}, "completed": []}
        try:
            state = json.loads(self.state_path.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError) as error:
            raise RuntimeError(f"cannot read watchdog state: {error}") from error
        if not isinstance(state, dict):
            raise RuntimeError("watchdog state must be a JSON object")
        state.setdefault("version", 1)
        state.setdefault("attempts", {})
        state.setdefault("skipped", {})
        state.setdefault("completed", [])
        return state

    def save_state(self) -> None:
        temporary = self.state_path.with_suffix(".tmp")
        temporary.write_text(
            json.dumps(self.state, indent=2, sort_keys=True) + "\n", encoding="utf-8"
        )
        temporary.replace(self.state_path)

    def command(
        self, args: list[str], *, timeout: float = 30, capture: bool = True
    ) -> subprocess.CompletedProcess[str]:
        return subprocess.run(
            args,
            cwd=self.project,
            text=True,
            capture_output=capture,
            timeout=timeout,
            check=False,
        )

    def list_runs(self) -> list[dict[str, Any]]:
        result = self.command(["bmad-loop", "list", "--project", str(self.project), "--json"])
        if result.returncode != 0:
            return []
        try:
            payload = json.loads(result.stdout)
        except json.JSONDecodeError:
            return []
        runs = payload.get("runs", [])
        return runs if isinstance(runs, list) else []

    def live_runs(self) -> list[dict[str, Any]]:
        return [run for run in self.list_runs() if run.get("status") == "running"]

    def status(self, run_id: str) -> dict[str, Any]:
        result = self.command(
            ["bmad-loop", "status", run_id, "--project", str(self.project), "--json"],
            timeout=15,
        )
        if result.returncode != 0:
            return {}
        try:
            payload = json.loads(result.stdout)
        except json.JSONDecodeError:
            return {}
        return payload if isinstance(payload, dict) else {}

    def story_statuses(self) -> dict[str, str]:
        status_path = self.project / "_bmad-output" / "implementation-artifacts" / "sprint-status.yaml"
        try:
            lines = status_path.read_text(encoding="utf-8").splitlines()
        except OSError as error:
            raise RuntimeError(f"cannot read sprint status: {error}") from error
        statuses: dict[str, str] = {}
        for line in lines:
            match = STATUS_STORY_RE.match(line)
            if match:
                statuses[match.group(1)] = match.group(2)
        return statuses

    def story_queue(self) -> list[str]:
        result = self.command(
            ["bmad-loop", "run", "--project", str(self.project), "--dry-run"],
            timeout=30,
        )
        stories = []
        if result.returncode == 0:
            stories = [
                match.group(1)
                for line in result.stdout.splitlines()
                if (match := DRY_RUN_STORY_RE.match(line))
            ]
        if stories:
            return stories
        return [story for story, status in self.story_statuses().items() if status not in {"done"}]

    def pane_fingerprint(self, run_id: str) -> str:
        session = f"bmad-loop-{run_id}"
        for target in (f"{session}:1.0", f"{session}:0.0", session):
            result = subprocess.run(
                ["tmux", "capture-pane", "-p", "-t", target, "-S", "-80"],
                cwd=self.project,
                text=True,
                capture_output=True,
                timeout=10,
                check=False,
            )
            if result.returncode == 0:
                return hashlib.sha256(result.stdout[-12000:].encode()).hexdigest()
        return ""

    def task_fingerprint(self, payload: dict[str, Any]) -> tuple[Any, ...]:
        tasks = payload.get("tasks", [])
        if not isinstance(tasks, list):
            tasks = []
        compact = []
        for task in tasks:
            if isinstance(task, dict):
                compact.append(
                    (
                        task.get("story_key"),
                        task.get("phase"),
                        task.get("attempt"),
                        task.get("review_cycle"),
                        task.get("commit_sha"),
                        task.get("defer_reason"),
                    )
                )
        return (payload.get("status"), tuple(compact))

    def discover_run(self, before: set[str], log_path: Path, process: subprocess.Popen[str]) -> str | None:
        deadline = time.monotonic() + 45
        while time.monotonic() < deadline:
            candidates = [
                run for run in self.list_runs() if run.get("run_id") not in before
            ]
            if candidates:
                candidates.sort(key=lambda run: str(run.get("started_at", "")), reverse=True)
                return str(candidates[0]["run_id"])
            try:
                text = log_path.read_text(encoding="utf-8")
            except OSError:
                text = ""
            for run_id in reversed(RUN_RE.findall(text)):
                if run_id not in before:
                    return run_id
            if process.poll() is not None:
                return None
            time.sleep(1)
        return None

    def stop_run(self, run_id: str) -> None:
        self.log(f"stopping run {run_id}")
        result = self.command(
            ["bmad-loop", "stop", run_id, "--project", str(self.project)], timeout=45
        )
        if result.returncode != 0:
            self.log(f"stop returned {result.returncode}: {result.stderr.strip()}")

    def final_outcome(self, run_id: str, story: str) -> str:
        payload = self.status(run_id)
        story_status = self.story_statuses().get(story, "")
        if story_status == "done":
            return "done"
        if payload.get("status") == "finished":
            tasks = payload.get("tasks", [])
            task = tasks[0] if tasks and isinstance(tasks[0], dict) else {}
            phase = str(task.get("phase", ""))
            if task.get("defer_reason") or any(
                marker in phase for marker in ("defer", "blocked", "awaiting")
            ):
                return "deferred"
            return "finished"
        return "failed"

    def run_story(self, story: str) -> str:
        attempts = int(self.state["attempts"].get(story, 0)) + 1
        self.state["attempts"][story] = attempts
        self.save_state()
        log_path = self.runtime_dir / f"{story}-attempt-{attempts}.log"
        before = {str(run.get("run_id")) for run in self.list_runs()}
        self.log(f"starting story {story}, attempt {attempts}/{self.args.max_retries}")
        with log_path.open("w", encoding="utf-8") as output:
            process = subprocess.Popen(
                [
                    "bmad-loop",
                    "run",
                    "--project",
                    str(self.project),
                    "--story",
                    story,
                    "--max-stories",
                    "1",
                ],
                cwd=self.project,
                stdout=output,
                stderr=subprocess.STDOUT,
                text=True,
                start_new_session=True,
            )
            self.active_process = process
            run_id = self.discover_run(before, log_path, process)
            if run_id is None:
                process.wait(timeout=60)
                self.log(f"story {story} did not produce a discoverable run")
                self.active_process = None
                return "failed"
            self.active_run_id = run_id
            self.log(f"story {story} is supervised by run {run_id}")
            last_fingerprint: tuple[Any, ...] | None = None
            last_progress = time.monotonic()
            try:
                while process.poll() is None:
                    payload = self.status(run_id)
                    fingerprint = self.task_fingerprint(payload) + (
                        self.pane_fingerprint(run_id),
                    )
                    if fingerprint != last_fingerprint:
                        last_fingerprint = fingerprint
                        last_progress = time.monotonic()
                    if time.monotonic() - last_progress >= self.args.stall_timeout:
                        self.log(
                            f"story {story} has made no observable progress for "
                            f"{self.args.stall_timeout}s"
                        )
                        self.stop_run(run_id)
                        try:
                            process.wait(timeout=45)
                        except subprocess.TimeoutExpired:
                            process.terminate()
                        self.active_run_id = None
                        self.active_process = None
                        return "stalled"
                    time.sleep(self.args.poll_seconds)
            finally:
                self.active_run_id = None
                self.active_process = None
            try:
                process.wait(timeout=45)
            except subprocess.TimeoutExpired:
                process.terminate()
                process.wait(timeout=15)
            outcome = self.final_outcome(run_id, story)
            self.log(f"story {story} run {run_id} ended with outcome {outcome}")
            return outcome

    def run(self) -> int:
        live = self.live_runs()
        if live:
            ids = ", ".join(str(run.get("run_id")) for run in live)
            raise RuntimeError(f"live bmad-loop run already exists: {ids}")
        self.log(
            f"watchdog started: stall_timeout={self.args.stall_timeout}s, "
            f"max_retries={self.args.max_retries}, poll={self.args.poll_seconds}s"
        )
        queue = self.story_queue()
        self.log(f"story queue contains {len(queue)} entries")
        for story in queue:
            status = self.story_statuses().get(story, "")
            if status in TERMINAL_STORY_STATUSES:
                self.log(f"skipping {story}: board status is {status}")
                continue
            if story in self.state["skipped"]:
                self.log(f"skipping {story}: watchdog previously exhausted retries")
                continue
            while True:
                outcome = self.run_story(story)
                if outcome in {"done", "finished", "deferred"}:
                    if outcome in {"done", "finished"}:
                        self.state["completed"].append(story)
                        self.save_state()
                    break
                attempts = int(self.state["attempts"].get(story, 0))
                if attempts >= self.args.max_retries:
                    self.state["skipped"][story] = {
                        "attempts": attempts,
                        "reason": outcome,
                        "at": now(),
                    }
                    self.save_state()
                    self.log(
                        f"skipping {story} after {attempts} attempts; "
                        "the story remains on the sprint board for repair"
                    )
                    break
                self.log(f"retrying {story} after outcome {outcome}")
        self.log("watchdog queue complete")
        return 0

    def close(self) -> None:
        if self.active_run_id:
            self.stop_run(self.active_run_id)
        if self.active_process and self.active_process.poll() is None:
            self.active_process.terminate()


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--project", type=Path, default=Path.cwd())
    parser.add_argument("--stall-timeout", type=int, default=900)
    parser.add_argument("--max-retries", type=int, default=3)
    parser.add_argument("--poll-seconds", type=int, default=10)
    parser.add_argument("--reset", action="store_true", help="discard watchdog state before starting")
    args = parser.parse_args()
    if args.stall_timeout < 30:
        parser.error("--stall-timeout must be at least 30 seconds")
    if args.max_retries < 1:
        parser.error("--max-retries must be at least 1")
    if args.poll_seconds < 1:
        parser.error("--poll-seconds must be at least 1 second")
    return args


def main() -> int:
    args = parse_args()
    watchdog = Watchdog(args.project, args)
    try:
        return watchdog.run()
    except KeyboardInterrupt:
        watchdog.log("watchdog interrupted")
        return 130
    except Exception as error:  # noqa: BLE001 - supervisor must report and exit cleanly
        watchdog.log(f"watchdog failed: {error}")
        return 1
    finally:
        watchdog.close()


if __name__ == "__main__":
    sys.exit(main())
