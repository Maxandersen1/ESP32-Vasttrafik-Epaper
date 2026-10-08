#pragma once

#include <Arduino.h>

// =============================================================================
//  KONFIGURATION: VÄSTTRAFIK + VÄDER E-INK MONITOREN (GITHUB TEMPLATE)
// =============================================================================

// --- 1. WiFi-inställningar ---
#define WIFI_SSID           "DITT_WIFI_SSID"        // Byt ut mot ditt WiFi-namn
#define WIFI_PASSWORD       "DITT_WIFI_LOSENORD"    // Byt ut mot ditt WiFi-lösenord
#define WIFI_TIMEOUT_MS     20000

// --- 2. Västtrafik API (OAuth2) ---
// Skaffa kostnadsfria API-nycklar på: https://developer.vasttrafik.se/
#define CLIENT_ID           "DIN_CLIENT_ID"         // Din Västtrafik Client ID
#define CLIENT_SECRET       "DIN_CLIENT_SECRET"     // Din Västtrafik Client Secret

#define API_HOST            "https://ext-api.vasttrafik.se"
#define API_VER             "v4"

// --- 3. Hållplats, Läge & Position ---
// Hitta ditt STOP_GID (Stop Area GID) via Västtrafik API (t.ex. 9021014002150000 för Ejdergatan)
#define STOP_GID            "9021014002150000"      // Ändra till din hållplats GID
#define STOP_NAME           "Ejdergatan"            // Namn som visas i rubriken på skärmen
#define TARGET_PLATFORM     'A'                     // Läge/Plattform (t.ex. 'A', 'B' eller '\0' för alla)

// Koordinater för väder (Open-Meteo API, kräver ingen nyckel)
#define LATITUDE            "57.72"                 // Din breddgrad (ex. 57.72 för Göteborg)
#define LONGITUDE           "12.01"                 // Din längdgrad (ex. 12.01 för Göteborg)

// --- 4. Filter & Visning ---
#define ALLOW_TRAMS         true                    // true = visa spårvagnar
#define ALLOW_BUSES         true                    // true = visa bussar
#define MAX_DEPARTURES      4                       // Antal avgångar som visas (4 st lämnar plats åt vädret)
#define REFRESH_INTERVAL_S  60                      // Uppdateringsintervall i sekunder dagtid (rekommenderat 60s)
#define HTTP_TIMEOUT_MS     12000                   // HTTP Timeout i millisekunder

// --- 5. Tidszon & NTP (Sverige: automatisk sommar/vintertid) ---
#define TIMEZONE_POSIX      "CET-1CEST,M3.5.0,M10.5.0/3"
#define NTP_SERVER_1        "se.pool.ntp.org"
#define NTP_SERVER_2        "pool.ntp.org"

// --- 6. Waveshare 4.2" E-Paper Pinout (ESP32-C3 Supermini) ---
// Kopplingsschema:
// VCC  -> 3V3 (3.3V)
// GND  -> GND
// CLK  -> GPIO 4 (SCK / Klocka)
// DIN  -> GPIO 6 (MOSI / Data)
// CS   -> GPIO 7 (Chip Select)
// DC   -> GPIO 5 (Data / Command)
// RST  -> GPIO 3 (Reset)
// BUSY -> GPIO 1 (Busy signal)
#define EPD_SCK             4
#define EPD_MOSI            6
#define EPD_CS              7
#define EPD_DC              5
#define EPD_RST             3
#define EPD_BUSY            1

// --- 7. Strömspar: Timer Deep Sleep & Nattvila ---
#define ENABLE_DEEP_SLEEP   true    // true = somna mellan uppdateringar (sparar >95% batteri!)
#define ENABLE_NIGHT_SLEEP  true    // true = nattvila mellan start- och sluttid
#define NIGHT_START_HOUR    0       // Nattvila startar kl 00:30
#define NIGHT_START_MIN     30
#define NIGHT_END_HOUR      6       // Nattvila slutar kl 06:00
#define NIGHT_END_MIN       0
