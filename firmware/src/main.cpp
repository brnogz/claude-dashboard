/*
 * Claude Code Usage Dashboard — ESP32-S3 + ILI9341 2.8" TFT
 *
 * Connects to WiFi via captive portal (WiFiManager),
 * fetches usage stats from a Python HTTP server on the LAN,
 * and displays them on the TFT.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <Preferences.h>

#include "config.h"
#include "data.h"
#include "display.h"

// --- Globals ---
TFT_eSPI tft = TFT_eSPI();
DashboardDisplay dashboard;
DashboardData dashData;
Preferences prefs;

char hostAddress[64] = DEFAULT_HOST;
char hostPort[8] = DEFAULT_PORT;

unsigned long lastPollMs = 0;
unsigned long lastSuccessMs = 0;
bool connectionLostShown = false;

// --- WiFi Manager custom parameters ---
WiFiManagerParameter customHost("host", "Dashboard Host IP", DEFAULT_HOST, 40);
WiFiManagerParameter customPort("port", "Dashboard Host Port", DEFAULT_PORT, 6);

void saveConfigCallback() {
    strncpy(hostAddress, customHost.getValue(), sizeof(hostAddress) - 1);
    strncpy(hostPort, customPort.getValue(), sizeof(hostPort) - 1);
    prefs.begin("dashboard", false);
    prefs.putString("host", hostAddress);
    prefs.putString("port", hostPort);
    prefs.end();
    Serial.printf("Saved config: host=%s port=%s\n", hostAddress, hostPort);
}

void setupWiFi() {
    prefs.begin("dashboard", true);
    String savedHost = prefs.getString("host", DEFAULT_HOST);
    String savedPort = prefs.getString("port", DEFAULT_PORT);
    prefs.end();
    strncpy(hostAddress, savedHost.c_str(), sizeof(hostAddress) - 1);
    strncpy(hostPort, savedPort.c_str(), sizeof(hostPort) - 1);

    customHost.setValue(hostAddress, 40);
    customPort.setValue(hostPort, 6);

    dashboard.showMessage(tft, "WiFi Setup", "Connect to ClaudeDashboard AP");

    WiFiManager wm;
    wm.setConfigPortalTimeout(180);
    wm.setSaveParamsCallback(saveConfigCallback);
    wm.addParameter(&customHost);
    wm.addParameter(&customPort);

    bool connected = wm.autoConnect(AP_NAME);

    if (connected) {
        if (strlen(customHost.getValue()) > 0)
            strncpy(hostAddress, customHost.getValue(), sizeof(hostAddress) - 1);
        if (strlen(customPort.getValue()) > 0)
            strncpy(hostPort, customPort.getValue(), sizeof(hostPort) - 1);

        dashboard.showMessage(tft, "Connected!", WiFi.localIP().toString().c_str());
        delay(1500);
    } else {
        dashboard.showMessage(tft, "WiFi Failed", "Restarting...");
        delay(2000);
        ESP.restart();
    }
}

bool fetchStats() {
    char url[128];
    snprintf(url, sizeof(url), "http://%s:%s/stats", hostAddress, hostPort);

    HTTPClient http;
    http.begin(url);
    http.setTimeout(HTTP_TIMEOUT_MS);
    int httpCode = http.GET();

    if (httpCode != 200) {
        Serial.printf("HTTP error: %d\n", httpCode);
        http.end();
        return false;
    }

    String payload = http.getString();
    http.end();

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload);
    if (err) {
        Serial.printf("JSON parse error: %s\n", err.c_str());
        return false;
    }

    dashData.activeSessions = doc["sessions"]["active"] | 0;

    JsonObject cur = doc["current"];
    strncpy(dashData.current.id, cur["id"] | "", sizeof(dashData.current.id) - 1);
    dashData.current.inputTokens = cur["inputTokens"] | 0;
    dashData.current.outputTokens = cur["outputTokens"] | 0;
    dashData.current.cacheRead = cur["cacheRead"] | 0;
    dashData.current.cacheCreate = cur["cacheCreate"] | 0;
    dashData.current.messages = cur["messages"] | 0;
    dashData.current.duration = cur["duration"] | 0;

    JsonObject today = doc["today"];
    dashData.today.inputTokens = today["inputTokens"] | 0;
    dashData.today.outputTokens = today["outputTokens"] | 0;
    dashData.today.cacheRead = today["cacheRead"] | 0;
    dashData.today.cacheCreate = today["cacheCreate"] | 0;
    dashData.today.messages = today["messages"] | 0;

    JsonObject week = doc["week"];
    dashData.week.inputTokens = week["inputTokens"] | 0;
    dashData.week.outputTokens = week["outputTokens"] | 0;
    dashData.week.cacheRead = week["cacheRead"] | 0;
    dashData.week.cacheCreate = week["cacheCreate"] | 0;
    dashData.week.messages = week["messages"] | 0;

    JsonArray daily = week["daily"];
    for (int i = 0; i < 7 && i < (int)daily.size(); i++) {
        dashData.weekDaily[i] = daily[i] | 0;
    }

    JsonArray labels = week["labels"];
    for (int i = 0; i < 7 && i < (int)labels.size(); i++) {
        strncpy(dashData.weekLabels[i], labels[i] | "?", 2);
        dashData.weekLabels[i][2] = '\0';
    }

    dashData.usagePct = doc["usage"]["pct"] | 0;
    dashData.windowUsed = doc["usage"]["used"] | 0;
    dashData.windowLimit = doc["usage"]["limit"] | 0;
    strncpy(dashData.plan, doc["usage"]["plan"] | "?", sizeof(dashData.plan) - 1);
    dashData.valid = true;
    dashData.lastUpdateMs = millis();

    return true;
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== Claude Dashboard ===");

    // TFT (backlight is hardwired on this board)
    tft.init();
    dashboard.begin(tft);
    dashboard.showMessage(tft, "Claude Dashboard", "Starting...");
    Serial.println("Display ready.");
    delay(500);

    // WiFi
    Serial.println("Starting WiFi...");
    setupWiFi();

    // Reset for dashboard mode
    memset(&dashData, 0, sizeof(dashData));
    dashboard.begin(tft);

    Serial.printf("Host: %s:%s\n", hostAddress, hostPort);
    Serial.println("Polling for stats...");
}

void loop() {
    unsigned long now = millis();

    if (now - lastPollMs >= POLL_INTERVAL_MS) {
        lastPollMs = now;

        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi disconnected, reconnecting...");
            WiFi.reconnect();
            delay(1000);
            return;
        }

        bool ok = fetchStats();
        if (ok) {
            lastSuccessMs = now;
            connectionLostShown = false;
            dashboard.update(tft, dashData);
            Serial.printf("Updated: %d%% usage, %d msgs today\n",
                          dashData.usagePct, dashData.today.messages);
        } else {
            Serial.println("Fetch failed");
        }
    }

    if (!connectionLostShown && lastSuccessMs > 0 && (now - lastSuccessMs > CONN_LOST_MS)) {
        dashboard.showConnectionLost(tft);
        connectionLostShown = true;
    }
}
