#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "Alexis SET";
const char* password = "09324473";
unsigned long lastRequestTime = 0;
const unsigned long REQUEST_INTERVAL = 5000; // 5 segundos

// ⚠️ IP DEL BACKEND (correcta)
const char* serverUrl = "http://192.168.0.106:8000/api/iot/esp32/detect/";

#define LED_PIN 2

WiFiClient client;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  Serial.print("🔌 Conectando WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  }

  digitalWrite(LED_PIN, LOW);

  Serial.println("\n✅ WiFi conectado");
  Serial.print("📡 IP ESP32: ");
  Serial.println(WiFi.localIP());

  enviarSolicitud();
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastRequestTime >= REQUEST_INTERVAL) {
    lastRequestTime = currentMillis;
    enviarSolicitud();
  }
}

void enviarSolicitud() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ WiFi desconectado");
    return;
  }

  HTTPClient http;

  Serial.println("📤 Enviando POST al backend...");
  http.begin(client, serverUrl);  // 🔥 ESTA LÍNEA ES LA CLAVE
  http.addHeader("Content-Type", "application/json");

  String payload = "{\"tacho_id\":2}";

  int httpResponseCode = http.POST(payload);

  Serial.print("📥 Código HTTP: ");
  Serial.println(httpResponseCode);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("📩 Respuesta backend:");
    Serial.println(response);

    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, response);

    if (error) {
      Serial.println("❌ Error parseando JSON");
      http.end();
      return;
    }

    int parpadeos = doc["parpadeos"] | 0;

    Serial.print("💡 Parpadeos recibidos: ");
    Serial.println(parpadeos);

    parpadear(parpadeos);
  } else {
    Serial.println("❌ No se pudo conectar al backend");
  }

  http.end();
}

void parpadear(int veces) {
  if (veces <= 0) return;

  Serial.print("🔁 Parpadeando ");
  Serial.print(veces);
  Serial.println(" veces");

  for (int i = 0; i < veces; i++) {
    digitalWrite(LED_PIN, LOW);   // ✅ ENCIENDE LED
    delay(500);
    digitalWrite(LED_PIN, HIGH);  // ✅ APAGA LED
    delay(500);
  }
}
