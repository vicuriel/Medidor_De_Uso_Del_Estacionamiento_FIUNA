#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// ======================= CONFIG WiFi =======================
const char* WIFI_SSID = "AndroidAPbe06";
const char* WIFI_PASS = "cocacola";

// ======================= HTTP =======================
WebServer server(80);

// ======================= SENSORES (solo lugar 1) ==========
// Sensor 1
const int TRIG1 = 13;
const int ECHO1 = 23;

// Sensor 2
const int TRIG2 = 14;
const int ECHO2 = 25;

// Menos que esto (cm) se considera "hay auto"
const int DIST_THRESHOLD_CM = 40;

// Ocupación del estacionamiento 1
int ocupacion1 = 0;

// ======================= ESTADOS PARA EL PASO DE AUTOS =====
enum EstadoPaso {
  PASO_IDLE,          // Esperando que alguien active un primer sensor
  PASO_S1_PRIMERO,    // Ya se activó S1 primero, espero S2 (entrada)
  PASO_S2_PRIMERO,    // Ya se activó S2 primero, espero S1 (salida)
  PASO_ESPERAR_LIBRE  // Ya conté, espero que ambos sensores se liberen
};

EstadoPaso estadoPaso = PASO_IDLE;
unsigned long tInicioPaso = 0;

// Tiempo máximo entre el primer y segundo sensor (ms)
const unsigned long MAX_INTERVALO_MS = 1500;  // ajustable

// Guardamos el estado anterior de cada sensor (para detectar flancos)
bool prev_s1_activo = false;
bool prev_s2_activo = false;

// ======================= FUNCIONES SENSORES =================
// Timeout reducido a 8000us (~8 ms) para que no bloquee tanto
long medirDistanciaCm(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duracion = pulseIn(echoPin, HIGH, 8000); // antes 25000
  if (duracion == 0) return 9999;

  long distancia = duracion / 58; // en cm
  return distancia;
}

void actualizarLugar1() {
  // 1) Leer sensores
  long d1 = medirDistanciaCm(TRIG1, ECHO1);
  long d2 = medirDistanciaCm(TRIG2, ECHO2);

  bool s1_activo = d1 < DIST_THRESHOLD_CM;
  bool s2_activo = d2 < DIST_THRESHOLD_CM;

  // 2) Detectar flancos de subida (0 -> 1)
  bool flanco_s1 = (s1_activo && !prev_s1_activo);
  bool flanco_s2 = (s2_activo && !prev_s2_activo);

  // Actualizamos estados previos para el próximo ciclo
  prev_s1_activo = s1_activo;
  prev_s2_activo = s2_activo;

  unsigned long ahora = millis();

  switch (estadoPaso) {

    case PASO_IDLE:
      // Esperamos el PRIMER flanco
      if (flanco_s1 && !s2_activo) {
        // Empezó por S1 → posible ENTRADA
        estadoPaso = PASO_S1_PRIMERO;
        tInicioPaso = ahora;
        Serial.println("S1 primero → posible ENTRADA");
      }
      else if (flanco_s2 && !s1_activo) {
        // Empezó por S2 → posible SALIDA
        estadoPaso = PASO_S2_PRIMERO;
        tInicioPaso = ahora;
        Serial.println("S2 primero → posible SALIDA");
      }
      break;

    case PASO_S1_PRIMERO:
      // ENTRADA: espero un FLANCO en S2
      if (flanco_s2) {
        if (ahora - tInicioPaso <= MAX_INTERVALO_MS) {
          ocupacion1++;
          if (ocupacion1 < 0) ocupacion1 = 0;
          Serial.printf("ENTRA auto. Ocupación = %d\n", ocupacion1);
          estadoPaso = PASO_ESPERAR_LIBRE;
        } else {
          Serial.println("Timeout ENTRADA, reseteando.");
          estadoPaso = PASO_IDLE;
        }
      }
      else if (ahora - tInicioPaso > MAX_INTERVALO_MS) {
        // No llegó S2 a tiempo
        Serial.println("Timeout ENTRADA sin S2 → cancelado.");
        estadoPaso = PASO_IDLE;
      }
      break;

    case PASO_S2_PRIMERO:
      // SALIDA: espero un FLANCO en S1
      if (flanco_s1) {
        if (ahora - tInicioPaso <= MAX_INTERVALO_MS) {
          ocupacion1--;
          if (ocupacion1 < 0) ocupacion1 = 0;
          Serial.printf("SALE auto. Ocupación = %d\n", ocupacion1);
          estadoPaso = PASO_ESPERAR_LIBRE;
        } else {
          Serial.println("Timeout SALIDA, reseteando.");
          estadoPaso = PASO_IDLE;
        }
      }
      else if (ahora - tInicioPaso > MAX_INTERVALO_MS) {
        Serial.println("Timeout SALIDA sin S1 → cancelado.");
        estadoPaso = PASO_IDLE;
      }
      break;

    case PASO_ESPERAR_LIBRE:
      // Ya se contó. Esperamos que ambos sensores se liberen
      if (!s1_activo && !s2_activo) {
        estadoPaso = PASO_IDLE;
        Serial.println("Sensores libres → listo para el próximo auto.");
      }
      break;
  }

  // Debug opcional:
  /*
  Serial.print("d1="); Serial.print(d1);
  Serial.print(" d2="); Serial.print(d2);
  Serial.print(" | s1="); Serial.print(s1_activo);
  Serial.print(" s2="); Serial.print(s2_activo);
  Serial.print(" | estado="); Serial.print(estadoPaso);
  Serial.print(" | ocup="); Serial.println(ocupacion1);
  */
}

// ======================= CORS ===============================
void addCORS() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

void handleOptions() {
  addCORS();
  server.send(204);
}

// ======================= ENDPOINTS HTTP =====================
// GET /status
void handleStatus() {
  addCORS();
  server.send(200, "application/json", "{\"ok\":true}");
}

// GET /state → por ahora solo lugar 1
// Respuesta: {"1":3}
void handleState() {
  addCORS();
  String json = "{";
  json += "\"1\":" + String(ocupacion1);
  json += "}";
  server.send(200, "application/json", json);
}

// 404
void handleNotFound() {
  addCORS();
  server.send(404, "text/plain", "404");
}

// ======================= SETUP ==============================
void setup() {
  Serial.begin(115200);
  delay(400);
  Serial.println("ESP32 estacionamiento - SOLO LUGAR 1 (flancos)");

  // Configurar pines de sensores
  pinMode(TRIG1, OUTPUT);
  pinMode(ECHO1, INPUT);
  pinMode(TRIG2, OUTPUT);
  pinMode(ECHO2, INPUT);

  digitalWrite(TRIG1, LOW);
  digitalWrite(TRIG2, LOW);

  ocupacion1 = 0;
  estadoPaso  = PASO_IDLE;
  prev_s1_activo = false;
  prev_s2_activo = false;

  // Conexión WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Conectado. IP: ");
  Serial.println(WiFi.localIP());

  // Rutas HTTP
  server.on("/status", HTTP_GET,      handleStatus);
  server.on("/status", HTTP_OPTIONS,  handleOptions);

  server.on("/state",  HTTP_GET,      handleState);
  server.on("/state",  HTTP_OPTIONS,  handleOptions);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server listo en /state y /status");
}

// ======================= LOOP ===============================
void loop() {
  server.handleClient();
  actualizarLugar1();
}
