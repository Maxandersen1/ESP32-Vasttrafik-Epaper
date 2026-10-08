# ESP32 Västtrafik and Weather E-Paper Departure Board

An ultra-low-power departure monitor for Swedish public transit (Västtrafik) and local weather forecasts (Open-Meteo), displayed on a Waveshare 4.2-inch black-and-white E-Paper screen and powered by an ESP32-C3 Supermini.

Designed to fit inside a picture frame with a 3D-printed enclosure, running for months on rechargeable lithium batteries.

---

## Photos

![Departure Board](pictures/Finished_product.jpg)
*Live display showing upcoming departures, current weather, and multi-day forecast.*

![Enclosure Back](photos/enclosure_back.jpg)
*Custom 3D-printed slim mounting frame and electronics bay.*

---

## Features

- Real-Time Departures: Connects to Västtrafik API v4 to fetch live tram and bus departures, line designations, destinations, and remaining minutes.
- Weather Forecast: Integrated Open-Meteo API displaying current temperature, weather icons, wind speeds, and next two days forecast.
- Power Management:
  - Timer Deep Sleep: The ESP32 enters deep sleep between updates, drawing only around 30 to 50 uA during standby.
  - Fast WiFi Reconnection: Caches WiFi channel and router BSSID in RTC memory to reconnect in under 0.8 seconds.
  - Night Rest Mode: Automatically activates between 00:30 and 06:00. Displays a dedicated night screen showing tomorrow's forecast and remains in deep sleep until morning.
- Clean Portrait Typography: 300x400 layout optimized for high contrast, Swedish ASCII sanitization, and clean line badges.
- Boot Safeguard: Includes a 3-second initialization delay on cold boot to ensure reliable USB flashing without sleep interruptions.

---

## Hardware Requirements

- Microcontroller: ESP32-C3 Supermini (or standard ESP32 / ESP32-S3)
- Display: Waveshare 4.2-inch E-Paper Module (Black/White, SSD1683 / GDEY042T81, 400x300 pixels)
- Power: 3.7V LiPo pouch cell or 18650 Li-ion battery with a TP4056 USB-C charging board
- Frame / Enclosure: Picture frame with 3D-printed mounting brackets

---

## Wiring and Pinout

| Waveshare 4.2" E-Paper | ESP32-C3 Supermini Pin | Description |
| :--- | :--- | :--- |
| VCC | 3V3 | 3.3V Power |
| GND | GND | Ground |
| CLK / SCK | GPIO 4 | SPI Clock |
| DIN / MOSI | GPIO 6 | SPI MOSI (Data) |
| CS | GPIO 7 | Chip Select |
| DC | GPIO 5 | Data / Command |
| RST | GPIO 3 | Hardware Reset |
| BUSY | GPIO 1 | Busy Status Signal |

---

## 3D CAD Files

CAD models for the custom frame and component brackets are provided in the `cad/` directory:

- STEP (`.step` / `.stp`): Recommended for parametric CAD tools (Fusion 360, SolidWorks, FreeCAD).
- STL (`.stl`): Ready for 3D printer slicers (Bambu Studio, PrusaSlicer, Cura).

---

## Software Dependencies

Install the following libraries using the Arduino Library Manager:

1. GxEPD2 (by Jean-Marc Zingg)
2. Adafruit GFX Library
3. ArduinoJson (version 6 or 7)

Board definition in Arduino IDE:
- ESP32 by Espressif Systems (Select `ESP32C3 Dev Module`)

---

## Setup and Configuration

1. Clone or download this repository.
2. Open `config.h` and update the placeholders:

```cpp
// WiFi Configuration
#define WIFI_SSID           "YOUR_WIFI_SSID"
#define WIFI_PASSWORD       "YOUR_WIFI_PASSWORD"

// Västtrafik OAuth2 API Credentials
#define CLIENT_ID           "YOUR_CLIENT_ID"
#define CLIENT_SECRET       "YOUR_CLIENT_SECRET"

// Stop Location and Platform
#define STOP_GID            "9021014002150000"  // Stop area GID
#define STOP_NAME           "Ejdergatan"        // Header display name
#define TARGET_PLATFORM     'A'                 // Platform filter ('A', 'B', etc.)

// Weather Coordinates
#define LATITUDE            "57.72"             // Local latitude
#define LONGITUDE           "12.01"             // Local longitude
---

## 📄 Licens

Detta projekt är öppen källkod under [MIT-licensen](LICENSE). Använd och modifiera fritt!
