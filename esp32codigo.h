#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "FIUNA";
const char* password = "fiuna#2024";

WebServer server(80);

// Cambiá si usás otro pin para el LED
const int LED_PIN = 2;

void handleOn() {
  digitalWrite(LED_PIN, HIGH);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", "{\"ok\":true,\"state\":\"on\"}");
}

void handleOff() {
  digitalWrite(LED_PIN, LOW);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", "{\"ok\":true,\"state\":\"off\"}");
}

void handleRoot() {
  // Usamos raw string con delimitador HTML → no hay que escapar comillas
  String html = R"HTML(
<!doctype html>
<html><head><meta charset="utf-8"><title>ESP32 LED</title></head>
<body style="font-family:sans-serif">
  <h1>Control LED</h1>
  <button onclick="fetch('/led/on')">Encender</button>
  <button onclick="fetch('/led/off')">Apagar</button>
</body></html>
)HTML";
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/html", html);
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.begin(115200);
  Serial.printf("Conectando a %s...\n", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(250); Serial.print("."); }
  Serial.println("\nWiFi conectado");
  Serial.print("IP ESP32: "); Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/led/on",  HTTP_GET, handleOn);
  server.on("/led/off", HTTP_GET, handleOff);

  // Soporte CORS/preflight por si hace falta
  server.onNotFound([](){
    if (server.method() == HTTP_OPTIONS) {
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.sendHeader("Access-Control-Allow-Methods", "GET,OPTIONS");
      server.sendHeader("Access-Control-Allow-Headers", "*");
      server.send(204);
    } else {
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(404, "text/plain", "Not found");
    }
  });

  server.begin();
}

void loop() {
  server.handleClient();
}
