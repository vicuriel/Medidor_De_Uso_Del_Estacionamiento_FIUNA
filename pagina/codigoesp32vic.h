#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "FIUNA";
const char* password = "fiuna#2024";
const int LED_PIN = 2;

WebServer server(80);

static void addCORS() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
}

void handleOn() {
  digitalWrite(LED_PIN, HIGH);
  addCORS();
  server.send(200, "application/json", "{\"ok\":true,\"state\":\"on\"}");
}

void handleOff() {
  digitalWrite(LED_PIN, LOW);
  addCORS();
  server.send(200, "application/json", "{\"ok\":true,\"state\":\"off\"}");
}

// (Opcional) raíz mínima para diagnósticos
void handleRoot() {
  addCORS();
  server.send(200, "text/plain", "ESP32 API OK. Usa /led/on y /led/off");
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(250); Serial.print("."); }
  Serial.printf("\nIP: %s\n", WiFi.localIP().toString().c_str());

  // Podés comentar esta línea si querés cero HTML interno:
  server.on("/", handleRoot);

  server.on("/led/on",  HTTP_GET, handleOn);
  server.on("/led/off", HTTP_GET, handleOff);

  // CORS / preflight y 404
  server.onNotFound([](){
    if (server.method() == HTTP_OPTIONS) {
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.sendHeader("Access-Control-Allow-Methods", "GET,OPTIONS");
      server.sendHeader("Access-Control-Allow-Headers", "*");
      server.send(204);
    } else {
      addCORS();
      server.send(404, "text/plain", "Not found");
    }
  });

  server.begin();
}

void loop() {
  server.handleClient();
}
