/*
 *  Västtrafik + Väder E-Paper Departure Monitor
 *  -------------------------------------------------------------
 *  En strömsnål avgångstavla för Västtrafik & väderprognos (Open-Meteo)
 *  byggd för ESP32-C3 Supermini och Waveshare 4.2" E-Paper display.
 *
 *  Funktioner:
 *  - E-Ink skärm i portrait-läge (300 x 400 px)
 *  - Snabb WiFi-anslutning med kanal- och BSSID-caching i RTC-minnet (<0.8s)
 *  - Timer Deep Sleep mellan uppdateringar dagtid (sparar >95% batteri)
 *  - Nattvila (00:30 - 06:00) med dedikerad nattvilaskärm & morgonens väderprognos
 *  - 3 sekunders programmeringsbuffert vid kallstart för enkel USB-flashing
 *
 *  Skapad av: Max & Antigravity (Google DeepMind)
 *  Licens: MIT
 */

#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <esp_sleep.h>

#include "config.h"
#include "display_ui.h"
#include "vasttrafik_api.h"
#include "weather_service.h"

// Objekt för Västtrafik & Väder
VasttrafikService vasttrafik;
WeatherService    weather;

DepartureItem departures[MAX_DEPARTURES];
size_t        departureCount = 0;

uint32_t lastRefreshMillis = 0;

// Variabler bevarade i ESP32 RTC-minnet under Deep Sleep
RTC_DATA_ATTR uint32_t rtcBootCount = 0;
RTC_DATA_ATTR bool     rtcNightScreenDrawn = false;
RTC_DATA_ATTR uint8_t  rtcWifiChannel = 0;
RTC_DATA_ATTR uint8_t  rtcWifiBSSID[6] = {0};
RTC_DATA_ATTR bool     rtcWifiValid = false;

// =============================================================================
//  NÄTVERKSANSLUTNING MELLAN DEEP SLEEP (SNABB RECONNECT)
// =============================================================================
bool connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return true;

  WiFi.mode(WIFI_STA);

  // Snabbanslutning med cachad kanal & BSSID sparar 2-3 sekunders söktid
  if (rtcWifiValid && rtcWifiChannel > 0) {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD, rtcWifiChannel, rtcWifiBSSID, true);
  } else {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }

  Serial.printf("[WiFi] Ansluter till '%s'...", WIFI_SSID);

  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_TIMEOUT_MS) {
    delay(100);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    rtcWifiChannel = WiFi.channel();
    memcpy(rtcWifiBSSID, WiFi.BSSID(), 6);
    rtcWifiValid = true;

    Serial.printf("[WiFi] OK! IP: %s (Kanal %d, RSSI %d dBm)\n",
                  WiFi.localIP().toString().c_str(),
                  rtcWifiChannel,
                  WiFi.RSSI());
    return true;
  }

  Serial.println("[WiFi] Misslyckades att ansluta.");
  return false;
}

// =============================================================================
//  NATTVILA LOGIK (00:30 - 06:00)
// =============================================================================
bool checkIsNightTime() {
  struct tm now;
  if (!getLocalTime(&now, 50)) return false;

  int cur   = now.tm_hour * 60 + now.tm_min;
  int start = NIGHT_START_HOUR * 60 + NIGHT_START_MIN;
  int end   = NIGHT_END_HOUR * 60 + NIGHT_END_MIN;

  if (start < end) {
    return (cur >= start && cur < end);
  } else {
    return (cur >= start || cur < end);
  }
}

uint32_t getSecondsUntilNightEnd() {
  struct tm now;
  if (!getLocalTime(&now, 50)) return 60;

  int currentSecOfDay = now.tm_hour * 3600 + now.tm_min * 60 + now.tm_sec;
  int targetSecOfDay  = NIGHT_END_HOUR * 3600 + NIGHT_END_MIN * 60;

  if (targetSecOfDay > currentSecOfDay) {
    return (targetSecOfDay - currentSecOfDay);
  } else {
    return ((24 * 3600 - currentSecOfDay) + targetSecOfDay);
  }
}

// Uppdatera väder och avgångar samt rita om skärmen
void updateAndRender() {
  weather.update();

  if (vasttrafik.getDepartures(departures, MAX_DEPARTURES, departureCount)) {
    String currentTime = vasttrafik.getCurrentTimeString();
    renderDepartureAndWeatherBoard(departures, departureCount, weather.getData(), currentTime, true);
  } else {
    Serial.println("[Main] Kunde inte hämta avgångar.");
    renderErrorScreen("API-fel", "Kunde inte hamta avgangar fran Vasttrafik");
  }
}

// =============================================================================
//  SETUP & LOOP
// =============================================================================
void setup() {
  Serial.begin(115200);
  rtcBootCount++;

  Serial.println("\n==========================================");
  Serial.printf("  Västtrafik + Väder E-Paper Monitor (Boot #%u)\n", rtcBootCount);
  Serial.println("==========================================");

  // Vid kallstart (första gången den kopplas in via USB eller batteri):
  // 3 sekunders paus så att USB och Serial hinner ansluta för ny programmering
  if (rtcBootCount == 1) {
    Serial.println("[System] Första uppstart. Pausar 3s för USB-programmering...");
    delay(3000);
  }

  // Initiera SPI-buss för ESP32-C3 Supermini
  SPI.begin(EPD_SCK, -1, EPD_MOSI, EPD_CS);

  // Initiera Waveshare 4.2" E-Paper display
  display.init(115200, true, 2, false);

  if (rtcBootCount == 1 && !rtcNightScreenDrawn) {
    renderConnectingScreen(WIFI_SSID);
  }

  // Anslut till nätverket
  if (!connectWiFi()) {
    renderErrorScreen("WiFi-anslutning misslyckades", "Kontrollera SSID och losenord i config.h");
    if (ENABLE_DEEP_SLEEP) {
      Serial.println("[Main] Somnar 60s innan nytt försök...");
      esp_sleep_enable_timer_wakeup(60ULL * 1000000ULL);
      esp_deep_sleep_start();
    }
    while (true) {
      delay(5000);
      if (connectWiFi()) break;
    }
  }

  // Initiera tidssynkronisering (NTP)
  vasttrafik.initTimeSync();
  delay(500);

  // Uppdatera väder
  weather.update(true);

  // =========================================================================
  // KONTROLL FÖR NATTVILA (KL 00:30 - 06:00)
  // =========================================================================
  if (ENABLE_NIGHT_SLEEP && checkIsNightTime()) {
    char wakeBuf[10];
    snprintf(wakeBuf, sizeof(wakeBuf), "%02d:%02d", NIGHT_END_HOUR, NIGHT_END_MIN);
    String wakeUpStr = String(wakeBuf);
    String currentTime = vasttrafik.getCurrentTimeString();

    if (!rtcNightScreenDrawn) {
      Serial.printf("[Nattvila] Nattvila aktiv! Ritar nattvilaskärm till kl %s...\n", wakeUpStr.c_str());
      renderNightScreen(weather.getData(), currentTime, wakeUpStr);
      rtcNightScreenDrawn = true;
    } else {
      Serial.println("[Nattvila] Nattvilaskärm redan ritad.");
    }

    uint32_t sleepSeconds = getSecondsUntilNightEnd();
    Serial.printf("[Nattvila] Sover i Deep Sleep i %u sekunder (ca %.1f timmar) till kl %s...\n",
                  sleepSeconds, sleepSeconds / 3600.0f, wakeUpStr.c_str());

    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

    if (ENABLE_DEEP_SLEEP) {
      esp_sleep_enable_timer_wakeup((uint64_t)sleepSeconds * 1000000ULL);
      esp_deep_sleep_start();
    } else {
      delay(sleepSeconds * 1000UL);
    }
    return;
  }

  // Dagtid: Nollställ flaggan för nattvilaskärmen
  rtcNightScreenDrawn = false;

  // Gör uppdatering av avgångar och väder
  updateAndRender();

  // =========================================================================
  // DAGTID TIMER DEEP SLEEP (SPARAR >95% BATTERI)
  // =========================================================================
  if (ENABLE_DEEP_SLEEP) {
    Serial.printf("[Power] Allt klart! Stänger av WiFi och sover i Deep Sleep i %d sekunder...\n", REFRESH_INTERVAL_S);
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

    esp_sleep_enable_timer_wakeup((uint64_t)REFRESH_INTERVAL_S * 1000000ULL);
    esp_deep_sleep_start();
  }

  lastRefreshMillis = millis();
}

void loop() {
  // Körs enbart om ENABLE_DEEP_SLEEP = false
  uint32_t now = millis();

  if (now - lastRefreshMillis >= (REFRESH_INTERVAL_S * 1000UL)) {
    lastRefreshMillis = now;
    Serial.printf("\n[Main] Schemalagd uppdatering (var %d:e sekund)...\n", REFRESH_INTERVAL_S);
    if (WiFi.status() != WL_CONNECTED) {
      connectWiFi();
    }
    updateAndRender();
  }
  delay(100);
}
