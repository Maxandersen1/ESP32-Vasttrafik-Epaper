#pragma once

#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

#include "config.h"

// Objekt för Waveshare 4.2" E-Paper (SSD1683, 400x300, körs i Portrait: 300x400)
inline GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> display(
    GxEPD2_420_GDEY042T81(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));

// Datastruktur för en enskild avgång
struct DepartureItem {
  String line;
  String destination;
  String track;
  int    minutesLeft;
  bool   isCancelled;
};

// Konvertera svenska UTF-8 tecken till ren ASCII så GFX-teckensnitt kan visa dem korrekt
inline String cleanSwedish(const String& input) {
  String out = "";
  out.reserve(input.length());
  for (size_t i = 0; i < input.length(); i++) {
    uint8_t c = (uint8_t)input[i];
    if (c == 0xC3 && i + 1 < input.length()) {
      uint8_t next = (uint8_t)input[++i];
      switch (next) {
        case 0xA5: out += 'a'; break; // å
        case 0xA4: out += 'a'; break; // ä
        case 0xB6: out += 'o'; break; // ö
        case 0x85: out += 'A'; break; // Å
        case 0x84: out += 'A'; break; // Ä
        case 0x96: out += 'O'; break; // Ö
        case 0xA9: out += 'e'; break; // é
        case 0xA8: out += 'e'; break; // è
        case 0xBC: out += 'u'; break; // ü
        case 0x9C: out += 'U'; break; // Ü
        default:   break;
      }
    } else if (c >= 32 && c <= 126) {
      out += (char)c;
    }
  }
  return out;
}

// Begränsa textlängd med punktation (...) om den överskrider maxBredd i pixlar
inline String fitText(const String& str, uint16_t maxW) {
  int16_t x1, y1; uint16_t w, h;
  display.getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
  if (w <= maxW) return str;

  String s = str;
  while (s.length() > 3) {
    s = s.substring(0, s.length() - 1);
    String candidate = s + "..";
    display.getTextBounds(candidate, 0, 0, &x1, &y1, &w, &h);
    if (w <= maxW) return candidate;
  }
  return str;
}

// Rita WiFi-signalikon
inline void drawWifiIcon(int x, int y, bool connected) {
  if (connected) {
    display.drawPixel(x + 7, y + 11, GxEPD_WHITE);
    display.drawPixel(x + 8, y + 11, GxEPD_WHITE);
    display.drawCircleHelper(x + 7, y + 10, 4, 1 | 2, GxEPD_WHITE);
    display.drawCircleHelper(x + 7, y + 10, 7, 1 | 2, GxEPD_WHITE);
  } else {
    display.drawLine(x + 2, y + 3, x + 12, y + 13, GxEPD_WHITE);
    display.drawLine(x + 2, y + 13, x + 12, y + 3, GxEPD_WHITE);
  }
}

// Rita väderikon (sol, halvklart, moln, regn, snö, åska)
inline void drawWeatherIcon(int x, int y, int code, bool small = false) {
  if (small) {
    if (code <= 1) { // Sol
      display.drawCircle(x, y, 5, GxEPD_BLACK);
      display.drawPixel(x, y - 7, GxEPD_BLACK);
      display.drawPixel(x, y + 7, GxEPD_BLACK);
      display.drawPixel(x - 7, y, GxEPD_BLACK);
      display.drawPixel(x + 7, y, GxEPD_BLACK);
    } else if (code <= 3) { // Moln
      display.drawCircle(x - 3, y + 1, 4, GxEPD_BLACK);
      display.drawCircle(x + 3, y - 1, 5, GxEPD_BLACK);
      display.drawCircle(x + 7, y + 1, 3, GxEPD_BLACK);
      display.drawFastHLine(x - 7, y + 5, 17, GxEPD_BLACK);
    } else if ((code >= 51 && code <= 67) || (code >= 80 && code <= 82)) { // Regn
      display.drawCircle(x - 2, y - 1, 4, GxEPD_BLACK);
      display.drawCircle(x + 3, y - 2, 5, GxEPD_BLACK);
      display.drawFastHLine(x - 6, y + 3, 14, GxEPD_BLACK);
      display.drawLine(x - 3, y + 5, x - 5, y + 9, GxEPD_BLACK);
      display.drawLine(x + 2, y + 5, x, y + 9, GxEPD_BLACK);
    } else { // Snö / annat
      display.drawCircle(x, y, 4, GxEPD_BLACK);
      display.drawFastHLine(x - 5, y + 6, 11, GxEPD_BLACK);
    }
  } else {
    // Stor ikon för väder
    if (code <= 1) { // Sol
      display.drawCircle(x, y, 10, GxEPD_BLACK);
      display.fillCircle(x, y, 8, GxEPD_BLACK);
      display.drawLine(x, y - 16, x, y - 12, GxEPD_BLACK);
      display.drawLine(x, y + 12, x, y + 16, GxEPD_BLACK);
      display.drawLine(x - 16, y, x - 12, y, GxEPD_BLACK);
      display.drawLine(x + 12, y, x + 16, y, GxEPD_BLACK);
      display.drawLine(x - 11, y - 11, x - 8, y - 8, GxEPD_BLACK);
      display.drawLine(x + 8, y + 8, x + 11, y + 11, GxEPD_BLACK);
      display.drawLine(x - 11, y + 11, x - 8, y + 8, GxEPD_BLACK);
      display.drawLine(x + 8, y - 8, x + 11, y - 11, GxEPD_BLACK);
    } else if (code == 2) { // Halvklart
      display.drawCircle(x - 5, y - 6, 7, GxEPD_BLACK);
      display.drawLine(x - 5, y - 16, x - 5, y - 14, GxEPD_BLACK);
      display.drawLine(x - 15, y - 6, x - 13, y - 6, GxEPD_BLACK);
      display.fillCircle(x - 4, y + 4, 7, GxEPD_WHITE);
      display.fillCircle(x + 4, y + 1, 9, GxEPD_WHITE);
      display.fillCircle(x + 11, y + 5, 6, GxEPD_WHITE);
      display.fillRect(x - 4, y + 6, 18, 5, GxEPD_WHITE);

      display.drawCircle(x - 4, y + 4, 7, GxEPD_BLACK);
      display.drawCircle(x + 4, y + 1, 9, GxEPD_BLACK);
      display.drawCircle(x + 11, y + 5, 6, GxEPD_BLACK);
      display.drawFastHLine(x - 6, y + 11, 23, GxEPD_BLACK);
    } else if (code == 3) { // Molnigt
      display.drawCircle(x - 5, y + 2, 8, GxEPD_BLACK);
      display.drawCircle(x + 4, y - 2, 10, GxEPD_BLACK);
      display.drawCircle(x + 12, y + 3, 7, GxEPD_BLACK);
      display.drawFastHLine(x - 9, y + 10, 27, GxEPD_BLACK);
    } else if ((code >= 51 && code <= 67) || (code >= 80 && code <= 82)) { // Regn
      display.drawCircle(x - 5, y - 2, 7, GxEPD_BLACK);
      display.drawCircle(x + 3, y - 5, 9, GxEPD_BLACK);
      display.drawCircle(x + 10, y - 1, 6, GxEPD_BLACK);
      display.drawFastHLine(x - 8, y + 5, 23, GxEPD_BLACK);
      display.drawLine(x - 4, y + 9, x - 6, y + 15, GxEPD_BLACK);
      display.drawLine(x + 3, y + 9, x + 1, y + 15, GxEPD_BLACK);
      display.drawLine(x + 10, y + 9, x + 8, y + 15, GxEPD_BLACK);
    } else if (code >= 71 && code <= 77) { // Snö
      display.drawCircle(x - 4, y - 2, 7, GxEPD_BLACK);
      display.drawCircle(x + 4, y - 4, 8, GxEPD_BLACK);
      display.drawFastHLine(x - 8, y + 5, 20, GxEPD_BLACK);
      display.drawPixel(x - 3, y + 11, GxEPD_BLACK);
      display.drawPixel(x + 4, y + 11, GxEPD_BLACK);
      display.drawPixel(x + 1, y + 15, GxEPD_BLACK);
    } else if (code >= 95) { // Åska
      display.drawCircle(x - 4, y - 3, 7, GxEPD_BLACK);
      display.drawCircle(x + 4, y - 5, 8, GxEPD_BLACK);
      display.drawFastHLine(x - 7, y + 4, 18, GxEPD_BLACK);
      display.drawLine(x, y + 6, x - 3, y + 11, GxEPD_BLACK);
      display.drawLine(x - 3, y + 11, x + 1, y + 11, GxEPD_BLACK);
      display.drawLine(x + 1, y + 11, x - 2, y + 17, GxEPD_BLACK);
    } else { // Dimmigt
      display.drawFastHLine(x - 12, y - 4, 24, GxEPD_BLACK);
      display.drawFastHLine(x - 16, y, 32, GxEPD_BLACK);
      display.drawFastHLine(x - 12, y + 4, 24, GxEPD_BLACK);
    }
  }
}

// Rita hela skärmen (Avgångar + Väderprognos) för 300x400 Portrait
inline void renderDepartureAndWeatherBoard(
    const DepartureItem items[],
    size_t count,
    const struct WeatherInfo& weather,
    const String& currentTimeStr,
    bool wifiOk)
{
  display.setRotation(1); // Portrait: 300 x 400
  display.setFullWindow();
  display.firstPage();

  do {
    display.fillScreen(GxEPD_WHITE);

    // =========================================================================
    // 1. TOPP-HEADER (Svart fält, y = 0..36)
    // =========================================================================
    display.fillRect(0, 0, 300, 36, GxEPD_BLACK);

    // Hållplatsnamn
    display.setTextColor(GxEPD_WHITE);
    display.setFont(&FreeSansBold9pt7b);
    display.setCursor(8, 24);
    display.print(STOP_NAME);

    // Läges-bricka (om angiven)
    if (TARGET_PLATFORM != '\0') {
      int badgeX = 106;
      display.fillRoundRect(badgeX, 6, 26, 24, 4, GxEPD_WHITE);
      display.setTextColor(GxEPD_BLACK);
      display.setFont(&FreeSansBold9pt7b);
      display.setCursor(badgeX + 8, 23);
      display.print(TARGET_PLATFORM);
    }

    // WiFi-ikon
    drawWifiIcon(228, 11, wifiOk);

    // Klockslag i header
    if (currentTimeStr.length() > 0) {
      display.setTextColor(GxEPD_WHITE);
      display.setFont(&FreeSansBold9pt7b);
      int16_t bx, by; uint16_t bw, bh;
      display.getTextBounds(currentTimeStr, 0, 0, &bx, &by, &bw, &bh);
      display.setCursor(292 - bw, 23);
      display.print(currentTimeStr);
    }

    // =========================================================================
    // 2. KOLUMNRUBRIKER FÖR AVGÅNGAR (y = 38..52)
    // =========================================================================
    display.setFont();
    display.setTextSize(1);
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(8, 42);
    display.print("LINJE");
    display.setCursor(48, 42);
    display.print("DESTINATION");
    display.setCursor(242, 42);
    display.print("AVGANG");

    display.drawFastHLine(0, 53, 300, GxEPD_BLACK);

    // =========================================================================
    // 3. AVGÅNGSRADER (Max 4 rader)
    // =========================================================================
    const int maxDeps = 4;
    const int startY = 54;
    const int rowH = 38;

    if (count == 0) {
      display.setFont(&FreeSans9pt7b);
      display.setTextColor(GxEPD_BLACK);
      display.setCursor(25, 110);
      display.print("Inga avgangar just nu.");
    } else {
      for (size_t i = 0; i < count && i < maxDeps; i++) {
        int rowTop = startY + i * rowH;
        int badgeTop = rowTop + 6;
        int baseline = rowTop + 25;

        // Linjenummerbricka (svart rundad rektangel)
        display.fillRoundRect(8, badgeTop, 32, 24, 4, GxEPD_BLACK);
        display.setTextColor(GxEPD_WHITE);
        display.setFont(&FreeSansBold9pt7b);
        int16_t x1, y1; uint16_t w, h;
        display.getTextBounds(items[i].line, 0, 0, &x1, &y1, &w, &h);
        display.setCursor(8 + (32 - w) / 2, baseline);
        display.print(items[i].line);

        // Destination
        display.setTextColor(GxEPD_BLACK);
        display.setFont(&FreeSansBold9pt7b);
        String destClean = cleanSwedish(items[i].destination);
        String destFitted = fitText(destClean, 185);
        display.setCursor(48, baseline);
        display.print(destFitted);

        // Minuter kvar till avgång
        String timeStr;
        if (items[i].minutesLeft <= 0) {
          timeStr = "Nu";
        } else {
          timeStr = String(items[i].minutesLeft) + " min";
        }

        display.setFont(&FreeSansBold9pt7b);
        display.getTextBounds(timeStr, 0, 0, &x1, &y1, &w, &h);
        display.setCursor(292 - w, baseline);
        display.print(timeStr);

        // Skiljelinje mellan rader
        if (i < count - 1 && i < maxDeps - 1) {
          display.drawFastHLine(8, rowTop + rowH, 284, GxEPD_BLACK);
        }
      }
    }

    // =========================================================================
    // 4. VÄDERPROGNOS SEKTION (y = 208..370)
    // =========================================================================
    display.fillRect(0, 208, 300, 24, GxEPD_BLACK);
    display.setTextColor(GxEPD_WHITE);
    display.setFont(&FreeSansBold9pt7b);
    display.setCursor(8, 225);
    display.print("VADERPROGNOS");

    if (weather.valid) {
      // Aktuellt väder
      drawWeatherIcon(32, 264, weather.currentCode, false);

      display.setTextColor(GxEPD_BLACK);
      display.setFont(&FreeSansBold12pt7b);
      display.setCursor(72, 260);
      display.print((int)round(weather.currentTemp));
      display.print(" C");

      display.setFont(&FreeSans9pt7b);
      display.setCursor(72, 280);
      display.print(cleanSwedish(weather.currentDesc));

      display.setFont();
      display.setTextSize(1);
      display.setCursor(72, 292);
      display.print("Vind: ");
      display.print(weather.currentWind, 1);
      display.print(" m/s");

      display.drawFastHLine(10, 308, 280, GxEPD_BLACK);

      // Imorgon
      display.setFont(&FreeSansBold9pt7b);
      display.setCursor(12, 324);
      display.print("Imorgon");

      drawWeatherIcon(26, 346, weather.tomorrowCode, true);

      display.setFont(&FreeSans9pt7b);
      display.setCursor(44, 344);
      display.print((int)round(weather.tomorrowMin));
      display.print("/");
      display.print((int)round(weather.tomorrowMax));
      display.print(" C");

      display.setFont();
      display.setTextSize(1);
      display.setCursor(44, 356);
      display.print(cleanSwedish(weather.tomorrowDesc));

      // Övermorgon
      display.drawFastVLine(150, 310, 56, GxEPD_BLACK);

      display.setFont(&FreeSansBold9pt7b);
      display.setCursor(158, 324);
      display.print("Overmorgon");

      drawWeatherIcon(172, 346, weather.dayAfterCode, true);

      display.setFont(&FreeSans9pt7b);
      display.setCursor(190, 344);
      display.print((int)round(weather.dayAfterMin));
      display.print("/");
      display.print((int)round(weather.dayAfterMax));
      display.print(" C");

      display.setFont();
      display.setTextSize(1);
      display.setCursor(190, 356);
      display.print(cleanSwedish(weather.dayAfterDesc));

    } else {
      display.setFont(&FreeSans9pt7b);
      display.setTextColor(GxEPD_BLACK);
      display.setCursor(40, 270);
      display.print("Hamtning av vaderdata...");
    }

    // =========================================================================
    // 5. FOOTER (y = 372..400)
    // =========================================================================
    display.drawFastHLine(0, 372, 300, GxEPD_BLACK);
    display.drawFastHLine(0, 373, 300, GxEPD_BLACK);

    display.setFont();
    display.setTextSize(1);
    display.setTextColor(GxEPD_BLACK);

    display.setCursor(6, 386);
    display.print("VASTTRAFIK | Var ");
    display.print(REFRESH_INTERVAL_S);
    display.print("s");

    display.setCursor(185, 386);
    if (wifiOk) {
      display.print("IP: ");
      display.print(WiFi.localIP().toString());
    } else {
      display.print("WiFi: Offline");
    }

  } while (display.nextPage());

  display.hibernate();
}

// Rita uppstartsskärm
inline void renderConnectingScreen(const char* ssid) {
  display.setRotation(1); // Portrait: 300x400
  display.setFullWindow();
  display.firstPage();

  do {
    display.fillScreen(GxEPD_WHITE);

    display.fillRect(0, 0, 300, 36, GxEPD_BLACK);
    display.setTextColor(GxEPD_WHITE);
    display.setFont(&FreeSansBold9pt7b);
    display.setCursor(12, 24);
    display.print("Vasttrafik E-Ink Monitor");

    display.drawRoundRect(20, 100, 260, 130, 8, GxEPD_BLACK);
    display.drawRoundRect(19, 99, 262, 132, 9, GxEPD_BLACK);

    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeSansBold9pt7b);
    display.setCursor(40, 135);
    display.print("Ansluter till WiFi...");

    display.setFont(&FreeSans9pt7b);
    display.setCursor(40, 165);
    display.print("Natverk: ");
    display.print(ssid);

    display.setCursor(40, 195);
    display.print(STOP_NAME);

  } while (display.nextPage());

  display.hibernate();
}

// Rita felmeddelandeskärm
inline void renderErrorScreen(const String& title, const String& details) {
  display.setRotation(1); // Portrait: 300x400
  display.setFullWindow();
  display.firstPage();

  do {
    display.fillScreen(GxEPD_WHITE);

    display.fillRect(0, 0, 300, 36, GxEPD_BLACK);
    display.setTextColor(GxEPD_WHITE);
    display.setFont(&FreeSansBold9pt7b);
    display.setCursor(12, 24);
    display.print("Vasttrafik - Status");

    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeSansBold9pt7b);
    display.setCursor(20, 110);
    display.print(title);

    display.setFont(&FreeSans9pt7b);
    display.setCursor(20, 145);
    display.print(details);

    display.setCursor(20, 180);
    display.print("Forsoker igen automatiskt...");

  } while (display.nextPage());

  display.hibernate();
}

// Rita måne och stjärnor för nattvilaskärmen
inline void drawMoonAndStars(int x, int y) {
  display.fillCircle(x, y, 22, GxEPD_BLACK);
  display.fillCircle(x + 9, y - 6, 18, GxEPD_WHITE);

  display.drawPixel(x - 28, y - 12, GxEPD_BLACK);
  display.drawLine(x - 30, y - 12, x - 26, y - 12, GxEPD_BLACK);
  display.drawLine(x - 28, y - 14, x - 28, y - 10, GxEPD_BLACK);

  display.drawPixel(x + 26, y + 14, GxEPD_BLACK);
  display.drawLine(x + 24, y + 14, x + 28, y + 14, GxEPD_BLACK);
  display.drawLine(x + 26, y + 12, x + 26, y + 16, GxEPD_BLACK);

  display.drawPixel(x - 20, y + 20, GxEPD_BLACK);
}

// Rita Nattvila-skärmen
inline void renderNightScreen(const struct WeatherInfo& weather, const String& currentTimeStr, const String& wakeUpTimeStr) {
  display.setRotation(1); // Portrait: 300x400
  display.setFullWindow();
  display.firstPage();

  do {
    display.fillScreen(GxEPD_WHITE);

    // 1. Topp-Header (y = 0..36)
    display.fillRect(0, 0, 300, 36, GxEPD_BLACK);

    display.setTextColor(GxEPD_WHITE);
    display.setFont(&FreeSansBold9pt7b);
    display.setCursor(8, 24);
    display.print(STOP_NAME);

    if (TARGET_PLATFORM != '\0') {
      int badgeX = 106;
      display.fillRoundRect(badgeX, 6, 26, 24, 4, GxEPD_WHITE);
      display.setTextColor(GxEPD_BLACK);
      display.setFont(&FreeSansBold9pt7b);
      display.setCursor(badgeX + 8, 23);
      display.print(TARGET_PLATFORM);
    }

    if (currentTimeStr.length() > 0) {
      display.setTextColor(GxEPD_WHITE);
      display.setFont(&FreeSansBold9pt7b);
      int16_t bx, by; uint16_t bw, bh;
      display.getTextBounds(currentTimeStr, 0, 0, &bx, &by, &bw, &bh);
      display.setCursor(292 - bw, 23);
      display.print(currentTimeStr);
    }

    // 2. Nattvila-box (y = 55..215)
    drawMoonAndStars(150, 95);

    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeSansBold12pt7b);
    int16_t tx1, ty1; uint16_t tw, th;
    display.getTextBounds("NATTVILA", 0, 0, &tx1, &ty1, &tw, &th);
    display.setCursor((300 - tw) / 2, 145);
    display.print("NATTVILA");

    display.setFont(&FreeSansBold9pt7b);
    String line1 = "Skarmen vilar till " + wakeUpTimeStr;
    display.getTextBounds(line1, 0, 0, &tx1, &ty1, &tw, &th);
    display.setCursor((300 - tw) / 2, 175);
    display.print(line1);

    display.setFont(&FreeSans9pt7b);
    String line2 = "Sparar batteri over natten";
    display.getTextBounds(line2, 0, 0, &tx1, &ty1, &tw, &th);
    display.setCursor((300 - tw) / 2, 200);
    display.print(line2);

    // Skiljelinje
    display.drawFastHLine(10, 225, 280, GxEPD_BLACK);

    // 3. Morgondagens väderprognos (y = 235..365)
    display.setFont(&FreeSansBold9pt7b);
    display.setCursor(15, 250);
    display.print("VADER IMORGON");

    if (weather.valid) {
      drawWeatherIcon(45, 290, weather.tomorrowCode, false);

      display.setTextColor(GxEPD_BLACK);
      display.setFont(&FreeSansBold12pt7b);
      display.setCursor(85, 286);
      display.print((int)round(weather.tomorrowMin));
      display.print(" / ");
      display.print((int)round(weather.tomorrowMax));
      display.print(" C");

      display.setFont(&FreeSans9pt7b);
      display.setCursor(85, 310);
      display.print(cleanSwedish(weather.tomorrowDesc));

      display.setFont();
      display.setTextSize(1);
      display.setCursor(85, 326);
      display.print("Aktuell temp: ");
      display.print((int)round(weather.currentTemp));
      display.print(" C");
    } else {
      display.setFont(&FreeSans9pt7b);
      display.setCursor(20, 290);
      display.print("Vaderprognos ej tillganglig");
    }

    // 4. Footer (y = 372..400)
    display.drawFastHLine(0, 372, 300, GxEPD_BLACK);
    display.drawFastHLine(0, 373, 300, GxEPD_BLACK);

    display.setFont();
    display.setTextSize(1);
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(8, 386);
    display.print("Vaknar automatiskt kl ");
    display.print(wakeUpTimeStr);

    display.setCursor(240, 386);
    display.print("Zzz...");

  } while (display.nextPage());

  display.hibernate();
}
