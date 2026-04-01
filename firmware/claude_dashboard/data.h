#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

struct SessionData {
    char id[12];
    uint32_t inputTokens;
    uint32_t outputTokens;
    uint32_t cacheRead;
    uint32_t cacheCreate;
    uint16_t messages;
    uint32_t duration;  // seconds
};

struct PeriodData {
    uint32_t inputTokens;
    uint32_t outputTokens;
    uint32_t cacheRead;
    uint32_t cacheCreate;
    uint16_t messages;
};

struct DashboardData {
    uint8_t activeSessions;
    SessionData current;
    PeriodData today;
    PeriodData week;
    uint32_t weekDaily[7];  // daily token totals, oldest first
    uint8_t usagePct;
    char plan[8];
    bool valid;
    uint32_t lastUpdateMs;
};

// Format a token count into a human-readable string like "123.4M", "45.6K"
inline void formatTokens(uint32_t tokens, char* buf, size_t bufLen) {
    if (tokens >= 1000000000) {
        snprintf(buf, bufLen, "%.1fB", tokens / 1000000000.0);
    } else if (tokens >= 1000000) {
        snprintf(buf, bufLen, "%.1fM", tokens / 1000000.0);
    } else if (tokens >= 1000) {
        snprintf(buf, bufLen, "%.1fK", tokens / 1000.0);
    } else {
        snprintf(buf, bufLen, "%lu", (unsigned long)tokens);
    }
}

// Format duration in seconds to "1h 23m" or "5m"
inline void formatDuration(uint32_t seconds, char* buf, size_t bufLen) {
    if (seconds >= 3600) {
        snprintf(buf, bufLen, "%luh %02lum", (unsigned long)(seconds / 3600), (unsigned long)((seconds % 3600) / 60));
    } else {
        snprintf(buf, bufLen, "%lum", (unsigned long)(seconds / 60));
    }
}
