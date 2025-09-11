# Medidor de Uso del Estacionamiento - PROYECTO LPV

**Autores:** Víctor Curiel, Ximena Quenhan, Hernán Espínola, Santiago Elgue

---

## Resumen
Sistema para medir la ocupación de estacionamientos: un **ESP32** con 2 sensores ultrasónicos detecta eventos (vehículo) y envía por **UDP** un token (`"11"` o `"12"`) a una **PC**. En la PC corre una interfaz en **Tkinter** (`ui_udp_numeric.py`) que escucha esos eventos y muestra la ocupación por zonas usando la clase `Estacionamiento` (`Estacionamiento.py`).

---

## Estructura del repositorio
- `codigoesp32.ino` — Sketch para ESP32 (envía `"11"` o `"12"` por UDP al host configurado).
- `Estacionamiento.py` — Modelo de zona/estacionamiento (métodos `ingresar`, `salir`, `snapshot`).
- `ui_udp_numeric.py` — Interfaz en Tkinter que:
  - Escucha UDP (por defecto `0.0.0.0:5000`).
  - Interpreta códigos `<ID><ACCION>` (último dígito = acción; `1`=IN, `2`=OUT).
  - Actualiza una tabla con ocupación por zona.
- `README.md` — Este archivo.

---

## Protocolo UDP
- Mensaje simple: token numérico (sin terminador especial).
  - `"11"` → Zona ID=1, acción IN (entró)
  - `"12"` → Zona ID=1, acción OUT (salió)
  - `"101"` → Zona ID=10, acción IN (permite IDs de varios dígitos)
- Puerto por defecto: **5000** (debe coincidir en `codigoesp32.ino` y `ui_udp_numeric.py`).

---

## Requisitos
- ESP32 (o similar) y 2 × HC-SR04 (o sensor equivalente).  
- PC con **Python 3.8+**. `tkinter` suele venir instalado por defecto en la mayoría de distribuciones.  
- Ambos (ESP32 y PC) en la **misma red/subred** (mismo SSID).

