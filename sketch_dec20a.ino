#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Alexis SET";
const char* password = "09324473";
const char* serverUrl = "http://192.168.0.106:8000/api/iot/esp32/detect/";

#define LED_PIN 2

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  WiFi.begin(ssid, password);
  Serial.print("Conectando WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  }

  Serial.println("\nWiFi conectado");
  Serial.println(WiFi.localIP());

  enviarSolicitud();
}

void loop() {
  // Si quieres repetir cada cierto tiempo:
  // delay(10000);
  // enviarSolicitud();
}

void enviarSolicitud() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    String payload = "{\"tacho_id\":1}";
    int httpResponseCode = http.POST(payload);

    Serial.print("Código HTTP: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode == 200) {
      String response = http.getString();
      Serial.println("Respuesta:");
      Serial.println(response);

      int idx = response.indexOf("\"parpadeos\":");
      if (idx != -1) {
        int fin = response.indexOf(",", idx);
        if (fin == -1) fin = response.indexOf("}", idx);

        String valor = response.substring(idx + 12, fin);
        valor.trim();
        int veces = valor.toInt();

        parpadear(veces);
      }
    } else {
      Serial.println("Error en backend");
    }

    http.end();
  }
}

void parpadear(int veces) {
  Serial.print("Parpadeando ");
  Serial.print(veces);
  Serial.println(" veces");

  for (int i = 0; i < veces; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    delay(500);
  }
}
