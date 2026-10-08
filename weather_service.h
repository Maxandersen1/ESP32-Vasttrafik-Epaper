#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "config.h"

// Struktur för väderdata (Open-Meteo API)
struct WeatherInfo {
  bool   valid = false;
  float  currentTemp = 0.0;
  int    currentCode = 0;
  float  currentWind = 0.0; // m/s
  String currentDesc = "Klart";

  // Idag
  float  todayMax = 0.0;
  float  todayMin = 0.0;

  // Imorgon
  float  tomorrowMax = 0.0;
  float  tomorrowMin = 0.0;
  int    tomorrowCode = 0;
  String tomorrowDesc = "";

  // I övermorgon
  float  dayAfterMax = 0.0;
  float  dayAfterMin = 0.0;
  int    dayAfterCode = 0;
  String dayAfterDesc = "";
};

class WeatherService {
private:
  WeatherInfo _data;
  uint32_t    _lastFetchMs = 0;
  const uint32_t UPDATE_INTERVAL_MS = 15 * 60 * 1000UL; // Uppdatera var 15:e minut

  String decodeWmoCode(int code) {
    switch (code) {
      case 0:  return "Klart";
      case 1:  return "Mestadels klart";
      case 2:  return "Halvklart";
      case 3:  return "Molnigt";
      case 45:
      case 48: return "Dimmigt";
      case 51:
      case 53:
      case 55: return "Duggregn";
      case 61:
      case 63:
      case 65: return "Regn";
      case 66:
      case 67: return "Underkylt regn";
      case 71:
      case 73:
      case 75: return "Snofall";
      case 77: return "Kornsnö";
      case 80:
      case 81:
      case 82: return "Regnskurar";
      case 85:
      case 86: return "Snobyar";
      case 95:
      case 96:
      case 99: return "Aska";
      default: return (code > 60 && code < 70) ? "Regn" : "Molnigt";
    }
  }

public:
  WeatherService() {}

  WeatherInfo getData() {
    return _data;
  }

  bool update(bool force = false) {
    uint32_t now = millis();
    if (!force && _data.valid && (now - _lastFetchMs < UPDATE_INTERVAL_MS)) {
      return true; // Använd cachad data
    }

    if (WiFi.status() != WL_CONNECTED) {
      return false;
    }

    WiFiClient client;
    HTTPClient http;

    // Open-Meteo REST API (Gratis, ingen API-nyckel krävs)
    String url = "http://api.open-meteo.com/v1/forecast?latitude=" LATITUDE "&longitude=" LONGITUDE
                 "&current=temperature_2m,weather_code,wind_speed_10m"
                 "&daily=weather_code,temperature_2m_max,temperature_2m_min"
                 "&forecast_days=3&timezone=auto";

    if (!http.begin(client, url)) {
      Serial.println("[Väder] Kunde inte ansluta till Open-Meteo");
      return false;
    }

    http.setTimeout(8000);
    int httpCode = http.GET();

    if (httpCode != 200) {
      Serial.printf("[Väder] HTTP %d från Open-Meteo\n", httpCode);
      http.end();
      return false;
    }

    String payload = http.getString();
    http.end();

#if ARDUINOJSON_VERSION_MAJOR >= 7
    JsonDocument doc;
#else
    DynamicJsonDocument doc(4096);
#endif
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      Serial.printf("[Väder] JSON-parsning misslyckades: %s\n", error.c_str());
      return false;
    }

    _data.currentTemp = doc["current"]["temperature_2m"] | 0.0f;
    _data.currentCode = doc["current"]["weather_code"] | 0;
    float windKmH     = doc["current"]["wind_speed_10m"] | 0.0f;
    _data.currentWind = windKmH / 3.6f;
    _data.currentDesc = decodeWmoCode(_data.currentCode);

    _data.todayMax = doc["daily"]["temperature_2m_max"][0] | _data.currentTemp;
    _data.todayMin = doc["daily"]["temperature_2m_min"][0] | _data.currentTemp;

    _data.tomorrowMax  = doc["daily"]["temperature_2m_max"][1] | 0.0f;
    _data.tomorrowMin  = doc["daily"]["temperature_2m_min"][1] | 0.0f;
    _data.tomorrowCode = doc["daily"]["weather_code"][1] | 0;
    _data.tomorrowDesc = decodeWmoCode(_data.tomorrowCode);

    _data.dayAfterMax  = doc["daily"]["temperature_2m_max"][2] | 0.0f;
    _data.dayAfterMin  = doc["daily"]["temperature_2m_min"][2] | 0.0f;
    _data.dayAfterCode = doc["daily"]["weather_code"][2] | 0;
    _data.dayAfterDesc = decodeWmoCode(_data.dayAfterCode);

    _data.valid = true;
    _lastFetchMs = now;
    Serial.printf("[Väder] OK! Temp: %.1f C, %s (Vind: %.1f m/s)\n",
                  _data.currentTemp, _data.currentDesc.c_str(), _data.currentWind);
    return true;
  }
};
