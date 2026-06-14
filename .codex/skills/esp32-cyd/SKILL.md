---
name: esp32-cyd
description: Work on ESP32 Cheap Yellow Display / ESP32-2432S028R / CYD projects in this folder, especially Arduino sketches using the witnessmenow ESP32-Cheap-Yellow-Display reference, ILI9341 320x240 TFT display, XPT2046 touch, WiFi, HTTP/API dashboards, and local arduino-cli builds.
---

# ESP32 CYD

Use this skill when working in `D:\docker\ESP32_28` on ESP32-2432S028R / Cheap Yellow Display code.

Primary reference: [witnessmenow/ESP32-Cheap-Yellow-Display](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display). Open the upstream docs when exact pin tables, setup notes, or troubleshooting details are needed.

## Hardware Notes

- Target board: ESP32-2432S028R / CYD / Cheap Yellow Display, often marked HW-458.
- Display: 2.8 inch ILI9341 TFT, 320x240.
- Touch: XPT2046 resistive touch on SPI.
- Typical display SPI pins used in this project: `MISO=12`, `MOSI=13`, `SCLK=14`, `TFT_CS=15`, `TFT_DC=2` for TFT_eSPI configs, `TFT_RST=-1`, `TFT_BL=21`, `TOUCH_CS=25`.
- Existing manual SPI sketch in this folder uses `PIN_DC=16`, `PIN_CS=15`, `PIN_BL=21`, `SPI.begin(14, 12, 13, -1)`. Preserve working local wiring unless hardware behavior proves otherwise.
- Use landscape `320x240` layouts by default. Keep text inside the visible safe area and leave margin for the bezel.

## Project Conventions

- Prefer Arduino sketches unless the user asks for PlatformIO or ESP-IDF.
- Use `WiFi.h`, `WiFiClientSecure.h`, `HTTPClient.h`, and `ArduinoJson.h` for HTTPS API dashboard sketches.
- For HTTPS tests where certificate management is not the focus, `WiFiClientSecure::setInsecure()` is acceptable on this local dashboard device.
- Avoid rendering Thai text with the tiny built-in 5x7 ASCII font unless a Thai font is explicitly added. Use English labels or bitmap/font assets for Thai UI.
- Treat WiFi passwords and API keys as secrets. Do not commit new secrets when a config/header alternative is available; if a user provides a key in chat, keep it scoped to the local sketch and warn if it may be exposed.

## Build And Upload

Use the bundled CLI in this folder when available:

```powershell
.\arduino-cli.exe compile --fqbn esp32:esp32:esp32 .\CreditDisplay\CreditDisplay_Sketch
```

For uploads, first identify the COM port, then upload:

```powershell
.\arduino-cli.exe board list
.\arduino-cli.exe upload -p COMx --fqbn esp32:esp32:esp32 .\CreditDisplay\CreditDisplay_Sketch
```

If builds fail because libraries are missing, install the ESP32 Arduino core and libraries matching the sketch (`ArduinoJson`, optionally `TFT_eSPI`).

## UI Guidance

- For dashboard screens like the local credit monitor, draw the important number first, then status/network information.
- Use high-contrast colors: black background, cyan/green outlines, yellow for primary percent/value, red for errors.
- On ILI9341 manual rendering, minimize full-screen redraws after boot; update changed rectangles when possible.
- Keep API status visible: last HTTP status, parsed credit value, and WiFi state.
