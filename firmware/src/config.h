#pragma once

// =============================================================
// Pin Mapping — UICPAL ESP32-S3-N16R8 Expansion Board
// Adjust these if the display doesn't work on first try
// =============================================================
#define TFT_CS    14
#define TFT_DC    47
#define TFT_RST   21
#define TFT_MOSI  45
#define TFT_SCLK  3
#define TFT_MISO  46
// TFT_BL is hardwired to 3.3V on this board (always on)

// =============================================================
// Display
// =============================================================
#define SCREEN_W  320
#define SCREEN_H  240
#define ROTATION  1    // Landscape

// =============================================================
// Network
// =============================================================
#define AP_NAME          "ClaudeDashboard"
#define POLL_INTERVAL_MS 5000
#define HTTP_TIMEOUT_MS  5000
#define CONN_LOST_MS     15000

// Default host (overridden by WiFiManager config)
#define DEFAULT_HOST     "192.168.1.100"
#define DEFAULT_PORT     "8080"

// =============================================================
// Colors (RGB565)
// =============================================================
#define CLR_BG          0x0000  // Black
#define CLR_HEADER_BG   0x1926  // Dark blue-gray
#define CLR_TEXT         0xFFFF  // White
#define CLR_TEXT_DIM     0xB596  // Light gray
#define CLR_ACCENT       0xFD20  // Amber/Orange (Claude brand)
#define CLR_GREEN        0x07E0  // Green
#define CLR_YELLOW       0xFFE0  // Yellow
#define CLR_RED          0xF800  // Red
#define CLR_CHART        0x07FF  // Cyan
#define CLR_DIVIDER      0x4208  // Dark gray
#define CLR_SESSION_BG   0x10A2  // Slightly lighter than black
#define CLR_FOOTER_BG    0x0000

// =============================================================
// Layout Y positions
// =============================================================
#define HEADER_Y      0
#define HEADER_H      46
#define STATS_Y       48
#define STATS_H       58
#define SESSION_Y     108
#define SESSION_H     40
#define CHART_Y       150
#define CHART_H       66
#define FOOTER_Y      218
#define FOOTER_H      22
