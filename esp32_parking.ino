// ESP32 - 2 x HC-SR04 -> enviar "11" o "12" por UDP solo cuando detectan (flanco 0->1)
// Sensor 1 -> ID "11": TRIG=25, ECHO=26
// Sensor 2 -> ID "12": TRIG=12, ECHO=14

#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid     = "FIUNA";
const char* password = "fiuna#2024";
const char* HOST_PC  = "172.16.237.239"; // tu IP privada
const uint16_t HOST_PORT = 8000;

const int TRIG1 = 25;
const int ECHO1 = 26;
const char* ID1 = "11";

const int TRIG2 = 12;
const int ECHO2 = 14;
const char* ID2 = "12";

const unsigned long ECHO_TIMEOUT = 30000UL; // µs
const float DIST_THRESHOLD_CM = 10.0;       // umbral detección

WiFiUDP udp;

void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(TRIG1, OUTPUT);
  pinMode(ECHO1, INPUT);
  digitalWrite(TRIG1, LOW);

  pinMode(TRIG2, OUTPUT);
  pinMode(ECHO2, INPUT);
  digitalWrite(TRIG2, LOW);

  Serial.print("Conectando a WiFi ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
    if (millis() - t0 > 20000) {
      Serial.println("\nTimeout WiFi - revisa SSID/Password.");
    }
  }
  Serial.println("\nWiFi conectado.");
  Serial.print("ESP IP: ");
  Serial.println(WiFi.localIP());

  udp.begin(0); // iniciar para enviar
}

// Promedia N lecturas en el par trig/echo y devuelve distancia en cm
float measureDistanceCM(int trigPin, int echoPin, int N = 3) {
  long sum_us = 0;
  int valid = 0;
  for (int i = 0; i < N; ++i) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    unsigned long dur = pulseIn(echoPin, HIGH, ECHO_TIMEOUT);
    if (dur > 0 && dur < ECHO_TIMEOUT) {
      sum_us += dur;
      valid++;
    }
    delay(25);
  }
  if (valid == 0) return 1e6;
  float avg_us = (float)sum_us / (float)valid;
  return avg_us / 58.0;
}

void sendUDP_str(const char* s) {
  udp.beginPacket(HOST_PC, HOST_PORT);
  udp.print(s); // sencillo y compatible
  udp.endPacket();
  Serial.print("ENVIADO UDP -> ");
  Serial.println(s);
}

void loop() {
  static int lastState1 = 0; // 0=no detecta, 1=detecta
  static int lastState2 = 0;

  // medir sensor 1
  float d1 = measureDistanceCM(TRIG1, ECHO1);
  int state1 = (d1 <= DIST_THRESHOLD_CM) ? 1 : 0;

  // breve pausa para minimizar interferencias
  delay(10);

  // medir sensor 2
  float d2 = measureDistanceCM(TRIG2, ECHO2);
  int state2 = (d2 <= DIST_THRESHOLD_CM) ? 1 : 0;

  // debug serial (opcional)
  if (d1 < 1e5) Serial.printf("S1 Dist: %.2f cm -> st=%d\n", d1, state1);
  else Serial.printf("S1 No eco -> st=%d\n", state1);
  if (d2 < 1e5) Serial.printf("S2 Dist: %.2f cm -> st=%d\n", d2, state2);
  else Serial.printf("S2 No eco -> st=%d\n", state2);

  // ENVIAR solo al pasar de 0->1 (flanco ascendente)
  if (state1 == 1 && lastState1 == 0) {
    sendUDP_str(ID1); // envía "11"
  }
  if (state2 == 1 && lastState2 == 0) {
    sendUDP_str(ID2); // envía "12"
  }

  lastState1 = state1;
  lastState2 = state2;

  delay(150); // ajuste de frecuencia
}
