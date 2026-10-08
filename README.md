# 🚌 ESP32 Västtrafik + Väder E-Paper Departure Display

En strömsnål och stilren avgångstavla för **Västtrafik** och väderprognos (**Open-Meteo**) byggd med en **ESP32-C3 Supermini** och en **Waveshare 4.2" E-Paper display (400x300 / SSD1683)**.

Perfekt för att hänga på väggen i hallen eller placera i en tavelram!

---

## ✨ Funktioner

- 🚌 **Västtrafik API v4 Realtime Data**: Visar linjenummer, destination och minuter kvar till avgång.
- 🌤️ **3-Dagars Väderprognos**: Temperatur, väderikoner och vindhastighet via avgiftsfria Open-Meteo API.
- 🔋 **Extrem Batterieffektivitet**:
  - **Timer Deep Sleep**: ESP32 somnar mellan uppdateringarna (strömförbrukning ~0.03 mA i vila).
  - **Fast WiFi Reconnect**: Cachar WiFi-kanal & BSSID i RTC-minnet för blixtsnabb anslutning (<0.8s).
  - **Nattvila (00:30 – 06:00)**: Skärmen visar en dedikerad nattvilaskärm (*"NATTVILA"* med måne, stjärnor och morgondagens väder) och sover kontinuerligt över natten.
- 🖥️ **Snygg Typografi & Ikoner**: Anpassade väder- och WiFi-ikoner samt svensk teckenrensning.

---

## 🛠️ Hårdvara som krävs

1. **Mikrokontroller**: ESP32-C3 Supermini (eller standard ESP32 / ESP32-S3)
2. **Skärm**: Waveshare 4.2" E-Paper Display Module (Svart/Vit, SSD1683 / GDEY042T81, 400x300 px)
3. **Strömförsörjning**: 1S LiPo / Li-ion batteri (t.ex. 3.7V LiPo Pouch-cell eller 18650) + TP4056 laddmodul.

---

## 🔌 Kopplingsschema (Pinout)

| Waveshare 4.2" E-Paper | ESP32-C3 Supermini Pin | Beskrivning |
| :--- | :--- | :--- |
| **VCC** | `3V3` | 3.3V Matning |
| **GND** | `GND` | Jord |
| **CLK / SCK** | `GPIO 4` | SPI Klocka |
| **DIN / MOSI** | `GPIO 6` | SPI Data |
| **CS** | `GPIO 7` | Chip Select |
| **DC** | `GPIO 5` | Data / Command Control |
| **RST** | `GPIO 3` | Reset |
| **BUSY** | `GPIO 1` | Busy Signal |

---

## 💻 Mjukvarukrav & Bibliotek (Arduino IDE)

Följande bibliotek behövs i Arduino IDE:

1. **GxEPD2** (av Jean-Marc Zingg) – *Sök och installera i Library Manager*
2. **Adafruit GFX Library** – *Grafikbibliotek*
3. **ArduinoJson** (v6 eller v7) – *För parsning av API-svar*

---

## 🚀 Snabbstart & Konfiguration

1. **Klona / Ladda ner detta repository**.
2. Öppna mappen `ESP32_Vasttrafik_E-Paper` i Arduino IDE.
3. Öppna filen `config.h` och fyll i dina uppgifter:

```cpp
// 1. WiFi-uppgifter
#define WIFI_SSID           "DITT_WIFI_SSID"
#define WIFI_PASSWORD       "DITT_WIFI_LOSENORD"

// 2. Västtrafik API (Skaffa gratis på https://developer.vasttrafik.se/)
#define CLIENT_ID           "DIN_CLIENT_ID"
#define CLIENT_SECRET       "DIN_CLIENT_SECRET"

// 3. Hållplats & Koordinater
#define STOP_GID            "9021014002150000" // Exempel: Ejdergatan
#define STOP_NAME           "Ejdergatan"
#define TARGET_PLATFORM     'A'                // Läge/Plattform ('A', 'B' osv.)

#define LATITUDE            "57.72"            // För väder i Göteborg
#define LONGITUDE           "12.01"
```

4. Välj kort i Arduino IDE: **ESP32C3 Dev Module** (eller motsvarande för din ESP32).
5. Ladda upp koden!

---

## 🔑 Hur du skaffar Västtrafik API-nycklar

1. Gå till [Västtrafik Developer Portal](https://developer.vasttrafik.se/).
2. Skapa ett konto och logga in.
3. Skapa en **Applikation** och aktivera **Reseplaneraren v4 / Departure Board v4 API**.
4. Kopiera din **Client ID** och **Client Secret** till `config.h`.
5. För att hitta din hållplats **STOP_GID**, sök i Västtrafiks API eller använd deras hållplatssök-endpoint.

---

## 📄 Licens

Detta projekt är öppen källkod under [MIT-licensen](LICENSE). Använd och modifiera fritt!
