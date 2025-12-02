# FIUNA PARKING - PROYECTO LPV

**Autores:** Víctor Curiel, Ximena Quenhan, Hernán Espínola, Santiago Elgue  

Sistema sencillo de conteo de autos y visualización web para medir la ocupación de distintos estacionamientos de la FIUNA usando un ESP32 y sensores ultrasónicos.

---

## 1. Descripción general

El proyecto está pensado para que cualquier persona pueda:

1. **Medir la ocupación** de uno o varios estacionamientos usando sensores ultrasónicos.
2. **Publicar el estado en una API HTTP** dentro del ESP32.
3. **Visualizar la ocupación desde una página web**, con:
   - Login básico.
   - Pantalla de selección de lugar.
   - Pantalla de estadísticas detalladas para cada lugar.

Actualmente el código del ESP32 implementa **un lugar de estacionamiento (Lugar 1)** con dos sensores ultrasónicos.  
La página web está preparada para **4 lugares** (Principal, Bozzano, Yuyal y LAR); los otros tres pueden implementarse agregando más sensores y modificando el código del ESP32.

---

## 2. Estructura del repositorio

- `login.html`  
  Página de acceso al sistema.  
  - Usuarios válidos: `victor`, `santi`, `hernan`, `xime`.  
  - Contraseña para todos: `1234`.  
  - Guarda el usuario actual en `localStorage` y registra el ingreso en Firebase Realtime Database (nodo `/auth/logins`).

- `seleccion.html`  
  Página principal después del login. Muestra una **grilla con los 4 lugares** de estacionamiento, cada uno con:
  - Imagen representativa.
  - Barra de porcentaje de ocupación.
  - Texto con el porcentaje actual.

  Cada tarjeta lee continuamente el estado desde el ESP32 (endpoint `/state`) y actualiza la barra.

- `principal.html`  
  Página de **estadísticas detalladas de un lugar** (se abre con el parámetro `?slot=N`, donde `N` va de 1 a 4).  
  Muestra:
  - Nombre del lugar.
  - Cantidad de autos detectados.
  - Estado (“Vacío” u “Ocupado (N)”).
  - Barra con el uso del estacionamiento (`contador / capacidad máxima`).

- `imagenes/`  
  Carpeta con las imágenes usadas en `seleccion.html`:
  - `principal.png`
  - `bozzano.png`
  - `yuyal.png`
  - `lar.png`

- `esp32_parking.ino/` 
  Contiene:
  - Conexión WiFi.
  - Lectura de los sensores ultrasónicos.
  - Lógica de entrada/salida de vehículos.
  - Servidor HTTP embebido con los endpoints `/status` y `/state`.

---

## 3. Requisitos

### Hardware

- 1 × **ESP32** (módulo tipo DevKit).
- 2 × **sensores ultrasónicos HC-SR04** por lugar de estacionamiento.  
  En el ejemplo se usa **solo un lugar** → 2 sensores.
- Fuente de **5 V** capaz de alimentar todos los sensores.
- Cables dupont y protoboard / PCB.

> Para 4 lugares se necesitarían 8 sensores (2 por cada lugar).

### Software

- Arduino IDE o PlatformIO para cargar el código al ESP32.
- Navegador web moderno (Chrome, Edge, Firefox).
- (Opcional) Proyecto de Firebase Realtime Database para registrar los logins.

---

## 4. Puesta en marcha rápida

1. **Configurar WiFi en el ESP32**

   En el archivo `.ino`:

   ```cpp
   const char* WIFI_SSID = "FIUNA";
   const char* WIFI_PASS = "fiuna#2024";
