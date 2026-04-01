# Claude Code Usage Dashboard

A physical dashboard that shows your Claude Code usage stats on an ESP32-S3 with a 2.8" TFT display.

## Display Layout

```
┌──────────────────────────────────────────────────┐
│  CLAUDE CODE                    [MAX] 8 active   │  HEADER
├────────────────────────┬─────────────────────────┤
│  TODAY                 │  THIS WEEK              │
│  405.7M tok            │  724.2M tok             │  STATS
│  4756 msgs             │  9655 msgs              │
├────────────────────────┴─────────────────────────┤
│  SESSION: 7c146053                        1h 47m │
│  Tokens: 13.7M                        Msgs: 274 │  SESSION
├──────────────────────────────────────────────────┤
│                                                  │
│  ▐█  ░░  ▐█  ░░  ▐█  ██████  ▐█                 │  WEEKLY
│  Th  Fr  Sa  Su  Mo  Tu      We                  │  CHART
├──────────────────────────────────────────────────┤
│  Usage: 81%  [████████████░░░░░░]      19% left  │  FOOTER
└──────────────────────────────────────────────────┘
```

### Header
- **CLAUDE CODE** — just the title
- **[MAX]** — your Claude subscription plan (green text)
- **8 active** — number of Claude Code sessions currently running on your Mac (processes with active PIDs)

### Stats (Today / This Week)
- **405.7M tok** — total tokens consumed. This includes all token types added together: input tokens you sent, output tokens Claude generated, cache read tokens (context reuse), and cache creation tokens. This is the raw total that counts toward your usage limit
- **4756 msgs** — number of assistant responses received (each back-and-forth counts as one message)
- **TODAY** column counts files modified today — so if a session started yesterday but you're still using it, its tokens count as today
- **THIS WEEK** column covers the last 7 calendar days

### Session
- **SESSION: 7c146053** — the first 8 characters of your most recent session's UUID. This identifies which conversation is shown
- **1h 47m** — how long this session file has existed (time since creation)
- **Tokens: 13.7M** — total tokens for this specific session, including any sub-agents it spawned
- **Msgs: 274** — message count for this session

### Weekly Chart
- 7 vertical bars showing token usage per day for the last 7 days
- Day labels (Th, Fr, Sa, etc.) are the actual calendar days, **not** fixed Mon-Sun
- The rightmost bar is always **today**, shown in amber/orange
- Other bars are cyan
- Bar heights are relative — the tallest bar fills the chart area, others scale proportionally
- A missing bar means zero tokens that day

### Footer
- **Usage: 81%** — estimated percentage of your daily token budget used. Based on a configurable limit of 500M tokens/day (you can change `DAILY_TOKEN_BUDGET` in the host script)
- **Progress bar** — green (<50%), yellow (50-80%), red (>80%)
- **19% left** — remaining budget estimate

## Token Types Explained

Claude Code tracks several token types per API call:

| Type | What it is |
|------|-----------|
| **Input tokens** | Tokens in your prompt that were sent fresh (not cached) |
| **Output tokens** | Tokens Claude generated in its response |
| **Cache read tokens** | Tokens reused from a previous turn's context (cheaper, avoids resending) |
| **Cache creation tokens** | Tokens written into cache for future reuse |

The dashboard sums all four types together for the totals shown. Cache tokens (especially cache reads) make up the majority of the count — a session with "13.7M tokens" might only have ~50K of actual input/output, with the rest being cache operations.

## Architecture

```
~/.claude/ (usage data files)
        │
        ▼
Python host script (port 8080)  ◄── auto-starts via macOS LaunchAgent
        │
        │  WiFi / HTTP GET /stats
        ▼
ESP32-S3  ──►  2.8" ILI9341 TFT
```

The ESP32 cannot read your Mac's files directly, so a small Python script reads `~/.claude/` and serves the data as JSON. The ESP32 fetches it over WiFi every 5 seconds.

## Files

```
claude_dashboard/
├── host/
│   ├── claude_dashboard_host.py    # Python HTTP server (reads ~/.claude/)
│   ├── com.claude.dashboard.plist  # macOS LaunchAgent (auto-start)
│   └── dashboard.log               # Server log output
├── firmware/
│   ├── platformio.ini              # PlatformIO build config
│   └── src/
│       ├── main.cpp                # Main sketch (WiFi, HTTP, JSON parsing)
│       ├── config.h                # Pin mapping, colors, layout constants
│       ├── data.h                  # Data structs and formatters
│       └── display.h               # TFT drawing functions
```

## Configuration

### Change daily token budget
Edit `DAILY_TOKEN_BUDGET` in `host/claude_dashboard_host.py` (default: 500,000,000). This affects the usage percentage shown in the footer.

### Change WiFi or host IP
Hold BOOT + press RST on the ESP32 to force the WiFi config portal. Connect to the "ClaudeDashboard" AP and enter new credentials.

### Host server management
```bash
# Check if running
launchctl list | grep claude

# Stop
launchctl unload ~/Library/LaunchAgents/com.claude.dashboard.plist

# Start
launchctl load ~/Library/LaunchAgents/com.claude.dashboard.plist

# View logs
tail -f ~/Desktop/claude_dashboard/host/dashboard.log
```
