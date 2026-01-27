#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// ------------------ WIFI ------------------
const char* ssid = "TEC_EP_203";
const char* password = "Tec_aula_203*";
const char* serverUrl = "http://192.168.54.52:8000/api/iot/esp32/detect/";

unsigned long lastRequestTime = 0;
const unsigned long REQUEST_INTERVAL = 5000;

// ------------------ HARDWARE ------------------
#define LED_PIN 2

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

// Ajustes para SG90
#define SERVO_MIN  120   // Pulso mínimo
#define SERVO_MAX  500   // Pulso máximo

#define SERVO_ORGANICO   0   // Canal 0
#define SERVO_INORGANICO 1   // Canal 1

WiFiClient client;

// ------------------ SETUP ------------------
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  Wire.begin(21, 22);
  pwm.begin();
  pwm.setPWMFreq(50); // 50Hz para servos

  cerrarServos();

  Serial.print("🔌 Conectando WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  }

  digitalWrite(LED_PIN, LOW);

  Serial.println("\n✅ WiFi conectado");
  Serial.print("📡 IP: ");
  Serial.println(WiFi.localIP());
}

// ------------------ LOOP ------------------
void loop() {
  if (millis() - lastRequestTime >= REQUEST_INTERVAL) {
    lastRequestTime = millis();
    enviarSolicitud();
  }
}

// ------------------ BACKEND ------------------
void enviarSolicitud() {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin(client, serverUrl);
  http.addHeader("Content-Type", "application/json");

  String payload = "{\"tacho_id\":2}";
  int httpCode = http.POST(payload);

  if (httpCode > 0) {
    String response = http.getString();
    Serial.println("📩 Respuesta:");
    Serial.println(response);

    StaticJsonDocument<256> doc;
    if (deserializeJson(doc, response)) return;

    int accion = doc["parpadeos"] | 0;
    ejecutarAccion(accion);
  }

  http.end();
}

// ------------------ ACCIONES ------------------
void ejecutarAccion(int accion) {
  switch (accion) {
    case 1:
      Serial.println("🟢 ORGÁNICO");
      moverServo(SERVO_ORGANICO);
      break;

    case 2:
      Serial.println("🔵 INORGÁNICO");
      moverServo(SERVO_INORGANICO);
      break;

    case 3:
      Serial.println("♻️ RECICLABLE");
      parpadearLED(3);
      break;

    default:
      break;
  }
}

// ------------------ SERVOS ------------------
void moverServo(uint8_t canal) {
  pwm.setPWM(canal, 0, SERVO_MAX); // abrir
  delay(2000);
  pwm.setPWM(canal, 0, SERVO_MIN); // cerrar
}

void cerrarServos() {
  pwm.setPWM(SERVO_ORGANICO, 0, SERVO_MIN);
  pwm.setPWM(SERVO_INORGANICO, 0, SERVO_MIN);
}

// ------------------ LED ------------------
void parpadearLED(int veces) {
  for (int i = 0; i < veces; i++) {
    digitalWrite(LED_PIN, LOW);
    delay(400);
    digitalWrite(LED_PIN, HIGH);
    delay(400);
  }
}
