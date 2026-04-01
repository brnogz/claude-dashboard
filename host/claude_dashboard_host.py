#!/usr/bin/env python3
"""Claude Code Usage Dashboard — Host Server
Reads Claude Code usage data from ~/.claude/ and serves it as JSON on HTTP.
ESP32 and Android app fetch this data over WiFi.
"""

import json
import os
import signal
from datetime import datetime, timedelta, timezone
from http.server import HTTPServer, BaseHTTPRequestHandler
from pathlib import Path

# --- Configuration ---
HOST = "0.0.0.0"
PORT = 8080
CLAUDE_DIR = Path.home() / ".claude"
PROJECTS_DIR = CLAUDE_DIR / "projects"
SESSIONS_DIR = CLAUDE_DIR / "sessions"

# Claude Max 5-hour rolling window API call limit.
# Each assistant response (including tool-use continuations) = 1 API call.
# Max 5x: ~625, Max 20x: ~2500. Adjust to match your plan.
FIVE_HOUR_MSG_LIMIT = 2500

# --- Caches ---
# Token cache: {filepath: (mtime, start_date_str, totals_dict)}
_token_cache: dict[str, tuple[float, str, dict]] = {}
# 5h window cache: {filepath: (mtime, list_of_timestamps)}
_ts_cache: dict[str, tuple[float, list[float]]] = {}

DAY_ABBR = ["Mo", "Tu", "We", "Th", "Fr", "Sa", "Su"]


def _get_session_start_date(filepath: Path) -> str:
    """Read the first timestamp from a JSONL file."""
    try:
        with open(filepath, "r") as f:
            for line in f:
                line = line.strip()
                if not line:
                    continue
                try:
                    obj = json.loads(line)
                    ts = obj.get("timestamp")
                    if ts:
                        return ts[:10]
                except (json.JSONDecodeError, KeyError, TypeError):
                    continue
    except OSError:
        pass
    try:
        return datetime.fromtimestamp(filepath.stat().st_birthtime).strftime("%Y-%m-%d")
    except (OSError, AttributeError):
        try:
            return datetime.fromtimestamp(filepath.stat().st_ctime).strftime("%Y-%m-%d")
        except OSError:
            return datetime.now().strftime("%Y-%m-%d")


def _parse_iso_ts(ts_str: str) -> float:
    """Parse ISO timestamp string to Unix epoch. Fast path for common format."""
    try:
        # "2026-03-31T19:13:52.601Z"
        if ts_str.endswith("Z"):
            ts_str = ts_str[:-1] + "+00:00"
        return datetime.fromisoformat(ts_str).timestamp()
    except (ValueError, TypeError):
        return 0.0


def parse_jsonl_tokens(filepath: Path) -> tuple[str, dict]:
    """Parse a JSONL file and sum all token usage from assistant messages."""
    fpath = str(filepath)
    try:
        mtime = filepath.stat().st_mtime
    except OSError:
        return datetime.now().strftime("%Y-%m-%d"), {
            "input": 0, "output": 0, "cache_read": 0, "cache_create": 0, "messages": 0
        }

    if fpath in _token_cache and _token_cache[fpath][0] == mtime:
        return _token_cache[fpath][1], _token_cache[fpath][2]

    start_date = _get_session_start_date(filepath)
    totals = {"input": 0, "output": 0, "cache_read": 0, "cache_create": 0, "messages": 0}
    try:
        with open(filepath, "r") as f:
            for line in f:
                line = line.strip()
                if not line:
                    continue
                try:
                    obj = json.loads(line)
                except json.JSONDecodeError:
                    continue
                if obj.get("type") == "assistant" and "message" in obj:
                    msg = obj["message"]
                    usage = msg.get("usage", {})
                    totals["input"] += usage.get("input_tokens", 0)
                    totals["output"] += usage.get("output_tokens", 0)
                    totals["cache_read"] += usage.get("cache_read_input_tokens", 0)
                    totals["cache_create"] += usage.get("cache_creation_input_tokens", 0)
                    totals["messages"] += 1
    except OSError:
        pass

    _token_cache[fpath] = (mtime, start_date, totals)
    return start_date, totals


def _parse_api_call_timestamps(filepath: Path) -> list[float]:
    """Parse unique assistant response timestamps from a JSONL file. Cached by mtime.
    Each assistant response = 1 API call, which is what Anthropic counts for rate limiting.
    Deduplicates by message ID (same response logged multiple times for streaming)."""
    fpath = str(filepath)
    try:
        mtime = filepath.stat().st_mtime
    except OSError:
        return []

    if fpath in _ts_cache and _ts_cache[fpath][0] == mtime:
        return _ts_cache[fpath][1]

    seen_ids: set[str] = set()
    timestamps = []
    try:
        with open(filepath, "r") as f:
            for line in f:
                line = line.strip()
                if not line:
                    continue
                try:
                    obj = json.loads(line)
                except json.JSONDecodeError:
                    continue
                msg = obj.get("message", {})
                if msg.get("role") != "assistant":
                    continue
                msg_id = msg.get("id", "")
                if not msg_id or msg_id in seen_ids:
                    continue
                seen_ids.add(msg_id)
                ts_str = obj.get("timestamp", "")
                if ts_str:
                    ts = _parse_iso_ts(ts_str)
                    if ts > 0:
                        timestamps.append(ts)
    except OSError:
        pass

    _ts_cache[fpath] = (mtime, timestamps)
    return timestamps


def get_all_jsonl_files() -> list[Path]:
    """Find all JSONL files across all projects (including subagents)."""
    files = []
    if not PROJECTS_DIR.exists():
        return files
    for project_dir in PROJECTS_DIR.iterdir():
        if not project_dir.is_dir():
            continue
        for f in project_dir.glob("*.jsonl"):
            files.append(f)
        for session_dir in project_dir.iterdir():
            if session_dir.is_dir():
                subagents_dir = session_dir / "subagents"
                if subagents_dir.exists():
                    for f in subagents_dir.glob("*.jsonl"):
                        files.append(f)
    return files


def get_active_sessions() -> list[dict]:
    active = []
    if not SESSIONS_DIR.exists():
        return active
    for f in SESSIONS_DIR.glob("*.json"):
        try:
            data = json.loads(f.read_text())
            pid = data.get("pid")
            if pid:
                try:
                    os.kill(pid, 0)
                    active.append(data)
                except (OSError, ProcessLookupError):
                    pass
        except (json.JSONDecodeError, OSError):
            pass
    return active


def find_current_session_jsonl(active_sessions: list[dict]) -> Path | None:
    if not active_sessions:
        return None
    sorted_sessions = sorted(active_sessions, key=lambda s: s.get("startedAt", 0), reverse=True)
    for session in sorted_sessions:
        session_id = session.get("sessionId", "")
        cwd = session.get("cwd", "")
        if not session_id or not cwd:
            continue
        project_name = cwd.replace("/", "-")
        if not project_name.startswith("-"):
            project_name = "-" + project_name
        jsonl_file = PROJECTS_DIR / project_name / f"{session_id}.jsonl"
        if jsonl_file.exists():
            return jsonl_file
    all_files = get_all_jsonl_files()
    if all_files:
        return max(all_files, key=lambda f: f.stat().st_mtime)
    return None


def get_period_data(days_back: int) -> tuple[dict, list[int], list[str]]:
    """Aggregate tokens for the last N days."""
    now = datetime.now()
    cutoff_ts = (now - timedelta(days=days_back)).timestamp()

    date_strs = []
    for i in range(days_back - 1, -1, -1):
        date_strs.append((now - timedelta(days=i)).strftime("%Y-%m-%d"))

    all_files = get_all_jsonl_files()
    totals = {"input": 0, "output": 0, "cache_read": 0, "cache_create": 0, "messages": 0}
    daily: dict[str, int] = {}

    for f in all_files:
        try:
            mtime = f.stat().st_mtime
        except OSError:
            continue
        if mtime < cutoff_ts:
            continue
        _, t = parse_jsonl_tokens(f)
        for key in totals:
            totals[key] += t[key]
        mdate = datetime.fromtimestamp(mtime).strftime("%Y-%m-%d")
        total_tokens = t["input"] + t["output"] + t["cache_read"] + t["cache_create"]
        daily[mdate] = daily.get(mdate, 0) + total_tokens

    breakdown = []
    labels = []
    for ds in date_strs:
        breakdown.append(daily.get(ds, 0))
        dt = datetime.strptime(ds, "%Y-%m-%d")
        labels.append(DAY_ABBR[dt.weekday()])

    return totals, breakdown, labels


def get_today_tokens() -> dict:
    today_str = datetime.now().strftime("%Y-%m-%d")
    all_files = get_all_jsonl_files()
    totals = {"input": 0, "output": 0, "cache_read": 0, "cache_create": 0, "messages": 0}
    for f in all_files:
        try:
            mdate = datetime.fromtimestamp(f.stat().st_mtime).strftime("%Y-%m-%d")
        except OSError:
            continue
        if mdate != today_str:
            continue
        _, t = parse_jsonl_tokens(f)
        for key in totals:
            totals[key] += t[key]
    return totals


def _get_all_session_files() -> list[Path]:
    """Get all JSONL session files (including subagents)."""
    files = []
    if not PROJECTS_DIR.exists():
        return files
    for project_dir in PROJECTS_DIR.iterdir():
        if not project_dir.is_dir():
            continue
        for f in project_dir.rglob("*.jsonl"):
            files.append(f)
    return files


def get_5h_window_usage() -> dict:
    """Count API calls (unique assistant responses) within the last 5 hours.
    Each assistant response = 1 API call toward Anthropic's rate limit."""
    now_ts = datetime.now().timestamp()
    cutoff_ts = now_ts - (5 * 3600)
    mtime_cutoff = cutoff_ts - 3600  # 1h buffer

    all_files = _get_all_session_files()
    msg_count = 0

    for f in all_files:
        try:
            mtime = f.stat().st_mtime
        except OSError:
            continue
        if mtime < mtime_cutoff:
            continue

        timestamps = _parse_api_call_timestamps(f)
        for ts in timestamps:
            if ts >= cutoff_ts:
                msg_count += 1

    pct = min(100, int((msg_count / FIVE_HOUR_MSG_LIMIT) * 100)) if FIVE_HOUR_MSG_LIMIT > 0 else 0
    return {
        "pct": pct,
        "plan": "max",
        "window": "5h",
        "used": msg_count,
        "limit": FIVE_HOUR_MSG_LIMIT,
    }


def build_stats() -> dict:
    active = get_active_sessions()
    current_jsonl = find_current_session_jsonl(active)

    current_session = {"id": "", "inputTokens": 0, "outputTokens": 0,
                       "cacheRead": 0, "cacheCreate": 0, "messages": 0, "duration": 0}
    if current_jsonl:
        _, t = parse_jsonl_tokens(current_jsonl)
        sid = current_jsonl.stem[:8] if current_jsonl.stem else ""
        try:
            stat = current_jsonl.stat()
            dur = int(stat.st_mtime - stat.st_birthtime)
        except (OSError, AttributeError):
            try:
                dur = int(stat.st_mtime - stat.st_ctime)
            except OSError:
                dur = 0

        subagent_dir = current_jsonl.parent / current_jsonl.stem / "subagents"
        sub_totals = {"input": 0, "output": 0, "cache_read": 0, "cache_create": 0, "messages": 0}
        if subagent_dir.exists():
            for sf in subagent_dir.glob("*.jsonl"):
                _, st = parse_jsonl_tokens(sf)
                for k in sub_totals:
                    sub_totals[k] += st[k]

        current_session = {
            "id": sid,
            "inputTokens": t["input"] + sub_totals["input"],
            "outputTokens": t["output"] + sub_totals["output"],
            "cacheRead": t["cache_read"] + sub_totals["cache_read"],
            "cacheCreate": t["cache_create"] + sub_totals["cache_create"],
            "messages": t["messages"] + sub_totals["messages"],
            "duration": dur,
        }

    today = get_today_tokens()
    week, daily_breakdown, day_labels = get_period_data(7)
    usage = get_5h_window_usage()

    return {
        "sessions": {"active": len(active)},
        "current": current_session,
        "today": {
            "inputTokens": today["input"],
            "outputTokens": today["output"],
            "cacheRead": today["cache_read"],
            "cacheCreate": today["cache_create"],
            "messages": today["messages"],
        },
        "week": {
            "inputTokens": week["input"],
            "outputTokens": week["output"],
            "cacheRead": week["cache_read"],
            "cacheCreate": week["cache_create"],
            "messages": week["messages"],
            "daily": daily_breakdown,
            "labels": day_labels,
        },
        "usage": usage,
    }


class DashboardHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == "/stats":
            try:
                stats = build_stats()
                body = json.dumps(stats).encode()
                self.send_response(200)
                self.send_header("Content-Type", "application/json")
                self.send_header("Content-Length", str(len(body)))
                self.send_header("Access-Control-Allow-Origin", "*")
                self.end_headers()
                self.wfile.write(body)
            except Exception as e:
                body = json.dumps({"error": str(e)}).encode()
                self.send_response(500)
                self.send_header("Content-Type", "application/json")
                self.end_headers()
                self.wfile.write(body)
        else:
            self.send_response(404)
            self.end_headers()

    def log_message(self, format, *args):
        if args and "200" not in str(args[0]):
            super().log_message(format, *args)


def main():
    server = HTTPServer((HOST, PORT), DashboardHandler)
    print(f"Claude Dashboard server running on http://{HOST}:{PORT}/stats")
    print(f"5-hour message limit: {FIVE_HOUR_MSG_LIMIT}")
    print("Press Ctrl+C to stop.")

    signal.signal(signal.SIGINT, lambda s, f: server.shutdown())

    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        server.server_close()


if __name__ == "__main__":
    main()
