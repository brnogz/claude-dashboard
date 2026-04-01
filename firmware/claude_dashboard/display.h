#pragma once
#include <TFT_eSPI.h>
#include "config.h"
#include "data.h"

class DashboardDisplay {
public:
    void begin(TFT_eSPI& tft);
    void drawStatic(TFT_eSPI& tft);
    void update(TFT_eSPI& tft, const DashboardData& data);
    void showMessage(TFT_eSPI& tft, const char* line1, const char* line2 = nullptr);
    void showConnectionLost(TFT_eSPI& tft);

private:
    void drawHeader(TFT_eSPI& tft, const DashboardData& data);
    void drawStats(TFT_eSPI& tft, const DashboardData& data);
    void drawSession(TFT_eSPI& tft, const DashboardData& data);
    void drawChart(TFT_eSPI& tft, const DashboardData& data);
    void drawFooter(TFT_eSPI& tft, const DashboardData& data);
    void drawProgressBar(TFT_eSPI& tft, int x, int y, int w, int h, int pct, uint16_t color);

    // Previous state for partial redraw
    DashboardData _prev;
    bool _firstDraw;
};

// --- Implementation ---

void DashboardDisplay::begin(TFT_eSPI& tft) {
    _firstDraw = true;
    memset(&_prev, 0, sizeof(_prev));
    tft.setRotation(ROTATION);
    tft.fillScreen(CLR_BG);
}

void DashboardDisplay::showMessage(TFT_eSPI& tft, const char* line1, const char* line2) {
    tft.fillScreen(CLR_BG);
    tft.setTextColor(CLR_ACCENT, CLR_BG);
    tft.setTextDatum(MC_DATUM);
    tft.setTextFont(4);
    tft.drawString(line1, SCREEN_W / 2, SCREEN_H / 2 - (line2 ? 16 : 0));
    if (line2) {
        tft.setTextFont(2);
        tft.setTextColor(CLR_TEXT_DIM, CLR_BG);
        tft.drawString(line2, SCREEN_W / 2, SCREEN_H / 2 + 20);
    }
    tft.setTextDatum(TL_DATUM);
}

void DashboardDisplay::showConnectionLost(TFT_eSPI& tft) {
    // Draw a red indicator in the header area
    tft.fillRect(SCREEN_W - 80, HEADER_Y + 2, 78, HEADER_H - 4, CLR_HEADER_BG);
    tft.setTextColor(CLR_RED, CLR_HEADER_BG);
    tft.setTextFont(2);
    tft.setTextDatum(TR_DATUM);
    tft.drawString("NO LINK", SCREEN_W - 4, HEADER_Y + 6);
    tft.setTextDatum(TL_DATUM);
}

void DashboardDisplay::drawStatic(TFT_eSPI& tft) {
    // Header background
    tft.fillRect(0, HEADER_Y, SCREEN_W, HEADER_H, CLR_HEADER_BG);
    tft.setTextColor(CLR_ACCENT, CLR_HEADER_BG);
    tft.setTextFont(4);
    tft.drawString("CLAUDE", 6, HEADER_Y + 2);
    tft.setTextColor(CLR_TEXT, CLR_HEADER_BG);
    tft.drawString("DASHBOARD", 92, HEADER_Y + 2);

    // Divider lines
    tft.drawFastHLine(0, STATS_Y - 1, SCREEN_W, CLR_DIVIDER);
    tft.drawFastHLine(0, SESSION_Y - 1, SCREEN_W, CLR_DIVIDER);
    tft.drawFastHLine(0, CHART_Y - 1, SCREEN_W, CLR_DIVIDER);
    tft.drawFastHLine(0, FOOTER_Y - 1, SCREEN_W, CLR_DIVIDER);

    // Vertical divider between Today and Week
    tft.drawFastVLine(SCREEN_W / 2, STATS_Y, STATS_H, CLR_DIVIDER);

    // Section labels
    tft.setTextFont(2);
    tft.setTextColor(CLR_TEXT_DIM, CLR_BG);
    tft.drawString("TODAY", 6, STATS_Y + 2);
    tft.drawString("THIS WEEK", SCREEN_W / 2 + 6, STATS_Y + 2);

    // Chart day labels
    tft.setTextFont(1);
    tft.setTextColor(CLR_TEXT_DIM, CLR_BG);
    const char* days[] = {"Mo", "Tu", "We", "Th", "Fr", "Sa", "Su"};
    int chartLeft = 10;
    int barSpacing = (SCREEN_W - 20) / 7;
    for (int i = 0; i < 7; i++) {
        int x = chartLeft + i * barSpacing + barSpacing / 2 - 5;
        tft.drawString(days[i], x, CHART_Y + CHART_H - 10);
    }
}

void DashboardDisplay::update(TFT_eSPI& tft, const DashboardData& data) {
    if (_firstDraw) {
        drawStatic(tft);
        _firstDraw = false;
    }
    drawHeader(tft, data);
    drawStats(tft, data);
    drawSession(tft, data);
    drawChart(tft, data);
    drawFooter(tft, data);
    _prev = data;
}

void DashboardDisplay::drawHeader(TFT_eSPI& tft, const DashboardData& data) {
    if (!_firstDraw &&
        data.activeSessions == _prev.activeSessions &&
        strcmp(data.plan, _prev.plan) == 0) return;

    // Clear right side of header
    tft.fillRect(200, HEADER_Y + 2, SCREEN_W - 202, HEADER_H - 4, CLR_HEADER_BG);

    char buf[32];
    tft.setTextFont(2);
    tft.setTextDatum(TR_DATUM);

    // Plan badge
    tft.setTextColor(CLR_GREEN, CLR_HEADER_BG);
    snprintf(buf, sizeof(buf), "[%s]", data.plan);
    for (char* p = buf; *p; p++) *p = toupper(*p);
    tft.drawString(buf, SCREEN_W - 60, HEADER_Y + 6);

    // Active sessions count
    tft.setTextColor(CLR_TEXT_DIM, CLR_HEADER_BG);
    snprintf(buf, sizeof(buf), "%d active", data.activeSessions);
    tft.drawString(buf, SCREEN_W - 4, HEADER_Y + 6);

    tft.setTextDatum(TL_DATUM);
}

void DashboardDisplay::drawStats(TFT_eSPI& tft, const DashboardData& data) {
    char buf[16];
    int halfW = SCREEN_W / 2;

    // --- TODAY column ---
    // Total tokens
    uint32_t todayTotal = data.today.inputTokens + data.today.outputTokens +
                          data.today.cacheRead + data.today.cacheCreate;
    uint32_t prevTodayTotal = _prev.today.inputTokens + _prev.today.outputTokens +
                              _prev.today.cacheRead + _prev.today.cacheCreate;

    if (_firstDraw || todayTotal != prevTodayTotal || data.today.messages != _prev.today.messages) {
        // Token count
        tft.fillRect(6, STATS_Y + 16, halfW - 12, 18, CLR_BG);
        formatTokens(todayTotal, buf, sizeof(buf));
        tft.setTextFont(4);
        tft.setTextColor(CLR_TEXT, CLR_BG);
        tft.drawString(buf, 6, STATS_Y + 16);

        // "tokens" label next to the number
        int tw = tft.textWidth(buf);
        tft.setTextFont(2);
        tft.setTextColor(CLR_TEXT_DIM, CLR_BG);
        tft.drawString("tok", 6 + tw + 4, STATS_Y + 22);

        // Messages
        tft.fillRect(6, STATS_Y + 38, halfW - 12, 16, CLR_BG);
        tft.setTextFont(2);
        tft.setTextColor(CLR_TEXT_DIM, CLR_BG);
        snprintf(buf, sizeof(buf), "%d msgs", data.today.messages);
        tft.drawString(buf, 6, STATS_Y + 38);
    }

    // --- WEEK column ---
    uint32_t weekTotal = data.week.inputTokens + data.week.outputTokens +
                         data.week.cacheRead + data.week.cacheCreate;
    uint32_t prevWeekTotal = _prev.week.inputTokens + _prev.week.outputTokens +
                             _prev.week.cacheRead + _prev.week.cacheCreate;

    if (_firstDraw || weekTotal != prevWeekTotal || data.week.messages != _prev.week.messages) {
        tft.fillRect(halfW + 6, STATS_Y + 16, halfW - 12, 18, CLR_BG);
        formatTokens(weekTotal, buf, sizeof(buf));
        tft.setTextFont(4);
        tft.setTextColor(CLR_TEXT, CLR_BG);
        tft.drawString(buf, halfW + 6, STATS_Y + 16);

        int tw = tft.textWidth(buf);
        tft.setTextFont(2);
        tft.setTextColor(CLR_TEXT_DIM, CLR_BG);
        tft.drawString("tok", halfW + 6 + tw + 4, STATS_Y + 22);

        tft.fillRect(halfW + 6, STATS_Y + 38, halfW - 12, 16, CLR_BG);
        tft.setTextFont(2);
        tft.setTextColor(CLR_TEXT_DIM, CLR_BG);
        snprintf(buf, sizeof(buf), "%d msgs", data.week.messages);
        tft.drawString(buf, halfW + 6, STATS_Y + 38);
    }
}

void DashboardDisplay::drawSession(TFT_eSPI& tft, const DashboardData& data) {
    if (!_firstDraw &&
        data.current.messages == _prev.current.messages &&
        strcmp(data.current.id, _prev.current.id) == 0) return;

    tft.fillRect(0, SESSION_Y, SCREEN_W, SESSION_H, CLR_BG);

    char buf[32];
    int halfW = SCREEN_W / 2;

    // Session ID and duration
    tft.setTextFont(2);
    tft.setTextColor(CLR_ACCENT, CLR_BG);
    snprintf(buf, sizeof(buf), "SESSION: %s", data.current.id);
    tft.drawString(buf, 6, SESSION_Y + 2);

    char durBuf[16];
    formatDuration(data.current.duration, durBuf, sizeof(durBuf));
    tft.setTextColor(CLR_TEXT_DIM, CLR_BG);
    tft.setTextDatum(TR_DATUM);
    tft.drawString(durBuf, SCREEN_W - 6, SESSION_Y + 2);
    tft.setTextDatum(TL_DATUM);

    // Session token total and messages
    uint32_t sessTotal = data.current.inputTokens + data.current.outputTokens +
                         data.current.cacheRead + data.current.cacheCreate;
    formatTokens(sessTotal, buf, sizeof(buf));
    tft.setTextFont(2);
    tft.setTextColor(CLR_TEXT, CLR_BG);
    char line2[48];
    snprintf(line2, sizeof(line2), "Tokens: %s", buf);
    tft.drawString(line2, 6, SESSION_Y + 20);

    snprintf(buf, sizeof(buf), "Msgs: %d", data.current.messages);
    tft.setTextColor(CLR_TEXT_DIM, CLR_BG);
    tft.setTextDatum(TR_DATUM);
    tft.drawString(buf, SCREEN_W - 6, SESSION_Y + 20);
    tft.setTextDatum(TL_DATUM);
}

void DashboardDisplay::drawChart(TFT_eSPI& tft, const DashboardData& data) {
    // Check if chart data changed
    bool changed = _firstDraw;
    if (!changed) {
        for (int i = 0; i < 7; i++) {
            if (data.weekDaily[i] != _prev.weekDaily[i]) { changed = true; break; }
        }
    }
    if (!changed) return;

    int chartLeft = 10;
    int chartTop = CHART_Y + 4;
    int chartBottom = CHART_Y + CHART_H - 14;
    int barAreaH = chartBottom - chartTop;
    int barSpacing = (SCREEN_W - 20) / 7;
    int barW = barSpacing - 6;

    // Clear chart area (keep day labels)
    tft.fillRect(0, CHART_Y, SCREEN_W, CHART_H - 12, CLR_BG);

    // Find max value for scaling
    uint32_t maxVal = 1;
    for (int i = 0; i < 7; i++) {
        if (data.weekDaily[i] > maxVal) maxVal = data.weekDaily[i];
    }

    // Draw bars
    for (int i = 0; i < 7; i++) {
        if (data.weekDaily[i] == 0) continue;
        int barH = (int)((float)data.weekDaily[i] / maxVal * barAreaH);
        if (barH < 2) barH = 2;
        int x = chartLeft + i * barSpacing + 3;
        int y = chartBottom - barH;

        // Color: today's bar in accent, others in cyan
        uint16_t color = (i == 6) ? CLR_ACCENT : CLR_CHART;
        tft.fillRect(x, y, barW, barH, color);
    }
}

void DashboardDisplay::drawFooter(TFT_eSPI& tft, const DashboardData& data) {
    if (!_firstDraw && data.usagePct == _prev.usagePct) return;

    tft.fillRect(0, FOOTER_Y, SCREEN_W, FOOTER_H, CLR_BG);

    char buf[32];
    tft.setTextFont(2);

    // "Usage: XX%"
    tft.setTextColor(CLR_TEXT, CLR_BG);
    snprintf(buf, sizeof(buf), "Usage: %d%%", data.usagePct);
    tft.drawString(buf, 6, FOOTER_Y + 2);

    // Progress bar
    int barX = 100;
    int barW = SCREEN_W - 160;
    uint16_t barColor = CLR_GREEN;
    if (data.usagePct > 80) barColor = CLR_RED;
    else if (data.usagePct > 50) barColor = CLR_YELLOW;
    drawProgressBar(tft, barX, FOOTER_Y + 4, barW, 14, data.usagePct, barColor);

    // "XX% left"
    snprintf(buf, sizeof(buf), "%d%% left", 100 - data.usagePct);
    tft.setTextColor(CLR_TEXT_DIM, CLR_BG);
    tft.setTextDatum(TR_DATUM);
    tft.drawString(buf, SCREEN_W - 4, FOOTER_Y + 2);
    tft.setTextDatum(TL_DATUM);
}

void DashboardDisplay::drawProgressBar(TFT_eSPI& tft, int x, int y, int w, int h, int pct, uint16_t color) {
    // Background
    tft.fillRect(x, y, w, h, CLR_DIVIDER);
    // Filled portion
    int fillW = (w * pct) / 100;
    if (fillW > 0) {
        tft.fillRect(x, y, fillW, h, color);
    }
    // Border
    tft.drawRect(x, y, w, h, CLR_TEXT_DIM);
}
