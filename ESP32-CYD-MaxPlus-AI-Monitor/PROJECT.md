# CYD_max — ESP32 AI Monitor

## Hardware
- **Board:** ESP32-2432S028R (CYD / HW-458)
- **Display:** ILI9341, 320×240px, landscape (rotation=1)
- **Framework:** PlatformIO + Arduino

## Layout หน้าจอ (320×240)
```
┌─────────────────────────────────────────┐  y=0
│  displayName    MaxPlus AI       IP/NOWIFI │  y=12  font1
├────────────────────┬────────────────────┤  y=25
│ CREDIT REMAINING   │   ^emoji^          │
│ $XXX.XX (font4)    │   ほっぺ pink      │  h=60
└────────────────────┴────────────────────┘  y=85
──── [ PAGER ] ──────────────────────────   y=88/95
  feed[0] ข้อความล่าสุด  (สีขาว)           y=104+ASCENT
  feed[1..7] เก่า         (สีฟ้า)           y=104+i*17+ASCENT
                                             bottom≈y=235
```

## UI Constants (แก้ตรงนี้ถ้าต้องการปรับ layout)
| ค่า | ไฟล์ | ความหมาย |
|-----|------|----------|
| `FEED_LINES 8` | main.cpp:32 | จำนวนบรรทัด feed |
| `THAI_ASCENT 17` | main.cpp:35 | ascent ของ NotoSansThai 16px |
| `feed x=31` | main.cpp:124 | indent ซ้ายของ feed |
| `feed y=104` | main.cpp:124 | y เริ่มต้น feed[0] (baseline) |
| `feed spacing=17` | main.cpp:126 | ระยะห่างแต่ละบรรทัด |
| PAGER line y=95 | main.cpp:121,123 | เส้น HLine ซ้าย/ขวา |
| PAGER text y=88 | main.cpp:122 | ข้อความ "[ PAGER ]" |

## Icon Keywords (ใส่ใน MQTT message)
| keyword | icon |
|---------|------|
| `[WARN]` | สามเหลี่ยมแดง + ! |
| `[LOVE]` หรือ `<3` | หัวใจแดง |
| `[ONLINE]` | วงกลมเขียว |

## ฟอนต์ภาษาไทย
- **Library:** `tcmenu/tcUnicodeHelper @ ^1.2.3`
- **Font file:** `src/NotoSansThai16_tc.h` (16px, 186 glyphs, ASCII+Thai)
- **สร้างฟอนต์ใหม่:**
  ```
  python make_tcfont.py
  ```
  แก้ `FONT_SIZE` และ `OUT_H` / `FONT_NAME` ใน script ก่อน
- **TTF ต้นทาง:** `NotoSansThai.ttf` (root project)

## Config (src/CYD_Config.h)
- WiFi SSID/Password
- API URL + Bearer key (maxplus-ai.cc)
- MQTT host/port/user/pass/topic (`cyd/pager`)
- RGB LED pins: RED=4, GREEN=16, BLUE=17
- `REFRESH_INTERVAL_MS` = 60000 (1 นาที)

## Build & Upload
```bash
# build + upload firmware
pio run --target upload

# upload font ไป SPIFFS (ไม่จำเป็นแล้ว — font อยู่ใน flash)
# pio run --target uploadfs
```

## Files สำคัญ
```
src/
  main.cpp          — โค้ดหลักทั้งหมด
  CYD_Config.h      — WiFi/MQTT/API credentials
  NotoSansThai16_tc.h — font header (auto-generated)
make_tcfont.py      — script สร้างฟอนต์ .h จาก .ttf
NotoSansThai.ttf    — ฟอนต์ต้นทาง
platformio.ini      — build config + pin defines

data/               — ไม่ใช้แล้ว (เคยเป็น SPIFFS)
src/main.cpp.bak    — backup ก่อน Thai font
```

## แก้ปัญหาพื้นฐาน
| ปัญหา | วิธีแก้ |
|-------|---------|
| ภาษาไทยไม่แสดง | ตรวจ `fontHandler.setFont(&NotoSansThai16)` ก่อน print |
| icon ไม่ตรงข้อความ | ปรับ `THAI_ASCENT` ให้ตรงกับ ascent ของฟอนต์ที่ใช้ |
| ตัวเลข credit ล้นกรอบ | credit box กว้าง 175px สูง 60px เริ่ม x=12,y=25 |
| MQTT ไม่รับ | ตรวจ MQTT_HOST/PORT ใน CYD_Config.h, reconnect ทุก loop |
| WiFi reconnect | auto reconnect ทุก 5 นาที ถ้า disconnect |
