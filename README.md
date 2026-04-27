# EcoTachosTec — IoT Firmware (ESP32)

> Embedded firmware for the ESP32 microcontroller that powers the physical waste sorting mechanism. Connects to the EcoTachosTec backend via WiFi, receives AI classification results, and actuates servomotors to physically direct waste into the correct bin.

---

## What it does

This firmware runs on an ESP32 (38-pin, dual-core 240MHz). Every 5 seconds it polls the Django backend for a pending classification command. When a result arrives (organic, inorganic, or recyclable), it drives the corresponding SG90 servomotor via a PCA9685 PWM driver to open and close the sorting gate — completing the physical side of the AI-to-hardware pipeline.

---

## Hardware

| Component | Details |
|---|---|
| Microcontroller | ESP32 38-pin (Xtensa dual-core, 240MHz, WiFi + BT) |
| PWM driver | PCA9685 via I2C (address 0x40) |
| Servomotors | 2× SG90 (organic gate — channel 0, inorganic gate — channel 1) |
| Status LED | Built-in LED (GPIO 2) |
| Stepper motor | NEMA 17 + Driver A4988 (transport mechanism) |
| Power supply | Modified ATX PSU (+5V for logic, +12V for motors) |
| I2C pins | SDA → GPIO 21, SCL → GPIO 22 |

---

## How it works

```
[ESP32 boots] → connects to WiFi
      │
      ▼ every 5 seconds
[POST /api/iot/esp32/detect/] → { "tacho_id": 2 }
      │
      ▼ backend responds
{ "parpadeos": 1 }  → organic    → open servo channel 0
{ "parpadeos": 2 }  → inorganic  → open servo channel 1
{ "parpadeos": 3 }  → recyclable → blink LED 3x
      │
      ▼
[servo opens 2s] → [servo closes] → waits for next cycle
```

The backend decides the classification (YOLOv8 model) and returns a simple integer action code. The ESP32 only handles actuation — keeping the firmware minimal and the intelligence server-side.

---

## Servo configuration

```cpp
#define SERVO_MIN  120   // closed position (pulse count)
#define SERVO_MAX  500   // open position (pulse count)
#define SERVO_ORGANICO   0   // PCA9685 channel 0
#define SERVO_INORGANICO 1   // PCA9685 channel 1
```

PWM frequency: 50Hz (standard for SG90 servos)

Servo sequence per classification:
1. Set PWM to `SERVO_MAX` → gate opens
2. Wait 2000ms (waste falls through)
3. Set PWM to `SERVO_MIN` → gate closes

---

## Dependencies

Install via Arduino Library Manager or PlatformIO:

```
WiFi.h              (built-in ESP32 core)
HTTPClient.h        (built-in ESP32 core)
ArduinoJson         >= 6.x
Adafruit PWM Servo Driver Library
Wire.h              (built-in)
```

---

## Setup & flash

### Arduino IDE

1. Install ESP32 board support: `https://dl.espressif.com/dl/package_esp32_index.json`
2. Install libraries: ArduinoJson, Adafruit PWM Servo Driver
3. Open `firmware.ino`
4. Update WiFi credentials and server URL:

```cpp
const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* serverUrl = "http://YOUR_BACKEND_IP:8000/api/iot/esp32/detect/";
```

5. Select board: **ESP32 Dev Module**
6. Flash via USB

### PlatformIO (alternative)

```bash
pio run --target upload
pio device monitor --baud 115200
```

---

## Serial monitor output

```
🔌 Conectando WiFi......
✅ WiFi conectado
📡 IP: 192.168.x.x

📩 Respuesta: {"parpadeos": 1}
🟢 ORGÁNICO → servo channel 0 actuated

📩 Respuesta: {"parpadeos": 2}
🔵 INORGÁNICO → servo channel 1 actuated

📩 Respuesta: {"parpadeos": 3}
♻️ RECICLABLE → LED blink x3
```

---

## Wiring diagram

```
ESP32 GPIO 21 (SDA) ──── PCA9685 SDA
ESP32 GPIO 22 (SCL) ──── PCA9685 SCL
ESP32 3.3V          ──── PCA9685 VCC
ESP32 GND           ──── PCA9685 GND

PCA9685 Channel 0   ──── Servo Orgánico (signal wire)
PCA9685 Channel 1   ──── Servo Inorgánico (signal wire)
PCA9685 V+          ──── 5V (from ATX PSU)

ESP32 GPIO 2        ──── Built-in LED (recyclable indicator)
```

---

## Configuration

| Parameter | Default | Description |
|---|---|---|
| `REQUEST_INTERVAL` | 5000ms | How often ESP32 polls the backend |
| `SERVO_MIN` | 120 | PWM pulse for closed gate position |
| `SERVO_MAX` | 500 | PWM pulse for open gate position |
| Servo open duration | 2000ms | Time gate stays open per cycle |
| PWM frequency | 50Hz | Standard for SG90 servos |

---

## Extending

To add a third servo (recyclable physical gate, instead of LED-only):

```cpp
#define SERVO_RECICLABLE 2   // PCA9685 channel 2

// In ejecutarAccion(), replace parpadearLED(3) with:
case 3:
  moverServo(SERVO_RECICLABLE);
  break;
```

To change polling to push (WebSocket) instead of HTTP polling, replace the `loop()` logic with a `WebSocketsClient` listener — the backend already supports WebSocket connections via Django Channels.

---

## Related repositories

- [ecotachostec-backend](https://github.com/Erick5933/ecotachostec-backend) — Django REST API + YOLOv8 inference
- [ecotachostec-frontend](https://github.com/Erick5933/ecotachostec-frontend) — React web dashboard
- [ecotachostec-mobile](https://github.com/Erick5933/ecotachostec-mobile) — React Native mobile app

---

## Authors

Built by Erick Chacón & Edwin Choez — Instituto Tecnológico del Azuay, Ecuador (2025–2026)
