#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "config.h"
#include "display_ui.h"

class VasttrafikService {
private:
  String   _accessToken;
  uint32_t _tokenObtainedMs = 0;
  uint32_t _tokenExpiresSec  = 0;

  WiFiClientSecure createSecureClient() {
    WiFiClientSecure client;
    client.setTimeout(HTTP_TIMEOUT_MS / 1000);
    client.setInsecure(); // Ignorera certifikatvalidering för embedded enheter
    return client;
  }

  // Räkna ut minuter kvar till avgång baserat på ESP32:s NTP-klocka
  int calculateMinutesRemaining(const String& isoTime) {
    if (isoTime.length() < 16) return -999;

    struct tm now;
    if (!getLocalTime(&now, 50)) {
      return -999; // NTP ej synkad
    }

    int depYear  = isoTime.substring(0, 4).toInt();
    int depMonth = isoTime.substring(5, 7).toInt();
    int depDay   = isoTime.substring(8, 10).toInt();
    int depHour  = isoTime.substring(11, 13).toInt();
    int depMin   = isoTime.substring(14, 16).toInt();

    int nowMinutes = now.tm_hour * 60 + now.tm_min;
    int depMinutes = depHour * 60 + depMin;

    // Om avgången sker efter midnatt och klockan nu är sent på kvällen
    if (depDay != now.tm_mday) {
      if (depDay > now.tm_mday || (now.tm_mday >= 28 && depDay == 1)) {
        depMinutes += 24 * 60;
      }
    }

    return depMinutes - nowMinutes;
  }

public:
  VasttrafikService() {}

  // Initiera tidsynk via NTP
  void initTimeSync() {
    configTzTime(TIMEZONE_POSIX, NTP_SERVER_1, NTP_SERVER_2);
    Serial.println("[NTP] Synkroniserar tid med svensk tidszon (CET/CEST)...");
  }

  // Hämta klockslag som sträng (HH:MM)
  String getCurrentTimeString() {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 50)) {
      char buf[10];
      strftime(buf, sizeof(buf), "%H:%M", &timeinfo);
      return String(buf);
    }
    return "";
  }

  // Hämta OAuth2 Access Token från Västtrafik
  bool fetchAccessToken() {
    uint32_t nowMs = millis();
    if (_accessToken.length() > 0 && (nowMs - _tokenObtainedMs < (_tokenExpiresSec - 60) * 1000UL)) {
      return true; // Giltig token finns lagrad
    }

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("[Auth] WiFi frånkopplat, kan inte hämta token.");
      return false;
    }

    WiFiClientSecure client = createSecureClient();
    HTTPClient http;
    String url = String(API_HOST) + "/token";

    if (!http.begin(client, url)) {
      Serial.println("[Auth] Kunde inte initiera HTTP-anslutning.");
      return false;
    }

    http.setTimeout(HTTP_TIMEOUT_MS);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.setAuthorization(CLIENT_ID, CLIENT_SECRET);

    int httpCode = http.POST("grant_type=client_credentials");

    if (httpCode != 200) {
      Serial.printf("[Auth] Fel vid token-förfrågan! HTTP-kod: %d\n", httpCode);
      http.end();
      return false;
    }

    String payload = http.getString();
    http.end();

#if ARDUINOJSON_VERSION_MAJOR >= 7
    JsonDocument doc;
#else
    DynamicJsonDocument doc(2048);
#endif
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      Serial.printf("[Auth] JSON-parsning misslyckades: %s\n", error.c_str());
      return false;
    }

    _accessToken = doc["access_token"].as<String>();
    _tokenExpiresSec = doc["expires_in"] | 3600;
    _tokenObtainedMs = nowMs;

    Serial.println("[Auth] OAuth2 Access Token hämtad OK!");
    return true;
  }

  // Hämta nästa avgångar från Västtrafik API v4
  bool getDepartures(DepartureItem outDepartures[], size_t maxCount, size_t& actualCount) {
    actualCount = 0;

    if (!fetchAccessToken()) {
      return false;
    }

    WiFiClientSecure client = createSecureClient();
    HTTPClient http;

    // Bygg API v4 URL för avgångstavla
    String url = String(API_HOST) + "/bin/rest.exe/v2/departureBoard?id=" + String(STOP_GID) +
                 "&format=json&needBikes=0";

    // Om vi använder API v4 OpenAPI endpoint
    String v4url = String(API_HOST) + "/departure-boards/v4/stop-areas/" + String(STOP_GID) +
                   "/departures?limit=30";

    if (!http.begin(client, v4url)) {
      Serial.println("[API] Kunde inte ansluta till Västtrafik API endpoint.");
      return false;
    }

    http.setTimeout(HTTP_TIMEOUT_MS);
    http.addHeader("Authorization", "Bearer " + _accessToken);
    http.addHeader("Accept", "application/json");

    int httpCode = http.GET();

    if (httpCode != 200) {
      Serial.printf("[API] HTTP-fel vid hämtning av avgångar: %d\n", httpCode);
      http.end();
      return false;
    }

    String payload = http.getString();
    http.end();

#if ARDUINOJSON_VERSION_MAJOR >= 7
    JsonDocument doc;
#else
    DynamicJsonDocument doc(16384);
#endif
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      Serial.printf("[API] JSON-parsning misslyckades: %s\n", error.c_str());
      return false;
    }

    JsonArray results = doc["results"].as<JsonArray>();
    if (results.isNull() || results.size() == 0) {
      Serial.println("[API] Inga avgångar hittades i svaret.");
      return true; // Giltigt svar men inga avgångar
    }

    for (JsonObject dep : results) {
      if (actualCount >= maxCount) break;

      // Filtrera på läge/plattform om TARGET_PLATFORM är angivet
      String platform = dep["serviceJourney"]["line"]["platform"] | "";
      if (TARGET_PLATFORM != '\0' && platform.length() > 0 && platform[0] != TARGET_PLATFORM) {
        continue; // Hoppa över om det inte matchar angivet läge
      }

      // Filtrera på transportslag
      String transportMode = dep["serviceJourney"]["line"]["transportMode"] | "";
      if (!ALLOW_TRAMS && transportMode.equalsIgnoreCase("tram")) continue;
      if (!ALLOW_BUSES && transportMode.equalsIgnoreCase("bus")) continue;

      String lineDesignation = dep["serviceJourney"]["line"]["shortName"] | "";
      String destination = dep["serviceJourney"]["direction"] | "";
      String isoTime = dep["estimatedTime"] | dep["plannedTime"] | "";

      int minsLeft = calculateMinutesRemaining(isoTime);

      // Spara i avgångsarrayen
      outDepartures[actualCount].line = lineDesignation;
      outDepartures[actualCount].destination = destination;
      outDepartures[actualCount].track = platform;
      outDepartures[actualCount].minutesLeft = minsLeft;
      outDepartures[actualCount].isCancelled = dep["isCancelled"] | false;

      actualCount++;
    }

    Serial.printf("[API] %d avgångar hämtade framgångsrikt!\n", (int)actualCount);
    return true;
  }
};
