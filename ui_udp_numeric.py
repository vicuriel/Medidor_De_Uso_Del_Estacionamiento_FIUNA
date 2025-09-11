# ui_udp_numeric.py
# UI Tkinter que escucha UDP en 0.0.0.0:8000.
# Protocolo: <ID><ACCION> (mínimo 2 dígitos). Último dígito = acción (1=IN, 2=OUT).
# Ejemplos válidos: "11", "12", "21", "101" (ID=10, acción=1), "102" (ID=10, acción=2)

import socket, threading, queue, tkinter as tk
from tkinter import ttk
from Estacionamiento import Estacionamiento

# ------------------ Config UDP ------------------
UDP_HOST = "0.0.0.0"   # escucha en todas las interfaces (tu IP incluida)
UDP_PORT = 8000

# Ajustá nombres/capacidades e IDs numéricos
ZONAS = {
    1: Estacionamiento(1, "Principal", 220),
    2: Estacionamiento(2, "Bozzano", 140),
    3: Estacionamiento(3, "Yuyal", 90),
    4: Estacionamiento(4, "LAR", 60),
}

# ------------------ Listener UDP (hilo) ------------------
class UdpListener(threading.Thread):
    def _init_(self, host, port, out_queue):
        super()._init_(daemon=True)
        self.host = host
        self.port = port
        self.q = out_queue
        self._stop = False

    def run(self):
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind((self.host, self.port))
        print(f"Escuchando UDP en {self.host}:{self.port} ...")
        while not self._stop:
            try:
                data, addr = sock.recvfrom(1024)
            except OSError:
                break
            msg = data.decode("utf-8", errors="ignore").strip()
            # soporta múltiples códigos en un mismo datagrama separados por espacios, comas o saltos de línea
            for token in msg.replace(",", " ").split():
                self.q.put(token)  # ejemplo: "11"
        sock.close()

    def stop(self):
        self._stop = True
        # enviar datagrama dummy para desbloquear recvfrom
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
                s.sendto(b"00", ("127.0.0.1", self.port))
        except:
            pass

# ------------------ UI ------------------
class App(tk.Tk):
    def _init_(self):
        super()._init_()
        self.title("Ocupación por zona (UDP)")
        self.geometry("820x340")

        self.event_q: queue.Queue[str] = queue.Queue()
        self.listener = UdpListener(UDP_HOST, UDP_PORT, self.event_q)
        self.listener.start()

        cols = ("id", "nombre", "cap_max", "cap_act", "disp", "util")
        self.tree = ttk.Treeview(self, columns=cols, show="headings", height=len(ZONAS))
        headers = {
            "id": "ID", "nombre": "Nombre",
            "cap_max": "Cap. Máxima", "cap_act": "Cap. Actual",
            "disp": "Disponibles", "util": "Util. (%)"
        }
        for k in cols:
            self.tree.heading(k, text=headers[k])
            anchor = "w" if k in ("id", "nombre") else "center"
            width = 260 if k == "nombre" else 110
            self.tree.column(k, anchor=anchor, width=width, stretch=True)
        self.tree.pack(fill="both", expand=True, padx=12, pady=12)

        self.rows = {}
        for z in sorted(ZONAS.values(), key=lambda z: z.id):
            iid = self.tree.insert("", "end", values=self._vals(z))
            self.rows[z.id] = iid

        self.status = ttk.Label(self, text=f"Escuchando UDP en {UDP_HOST}:{UDP_PORT}")
        self.status.pack(anchor="w", padx=12, pady=(0, 10))

        self.after(80, self._poll_events)
        self.after(300, self._refresh_table)
        self.protocol("WM_DELETE_WINDOW", self.on_close)

    def _vals(self, z: Estacionamiento):
        return (z.id, z.nombre, z.capacidad_maxima, z.capacidad_actual, z.disponibles, f"{z.utilizacion*100:.1f}")

    def _apply_code(self, code: str) -> str:
        # Regla robusta: último dígito = acción; el resto = ID (permite ID de varios dígitos).
        if not code.isdigit() or len(code) < 2:
            return f"Código inválido: '{code}'"
        accion = int(code[-1])
        zona_id = int(code[:-1])
        z = ZONAS.get(zona_id)
        if z is None:
            return f"Zona desconocida: {zona_id}"
        if accion == 1:
            z.ingresar(1)
            return f"Z{zona_id} IN +1 -> {z.capacidad_actual}/{z.capacidad_maxima}"
        elif accion == 2:
            z.salir(1)
            return f"Z{zona_id} OUT -1 -> {z.capacidad_actual}/{z.capacidad_maxima}"
        else:
            return f"Acción inválida: {accion} (usar 1=IN, 2=OUT)"

    def _poll_events(self):
        # procesa todos los datagramas pendientes
        try:
            while True:
                token = self.event_q.get_nowait()
                msg = self._apply_code(token)
                print("[UDP]", token, "->", msg)
                self.status.config(text=f"{msg}   |   Escuchando UDP en {UDP_HOST}:{UDP_PORT}")
        except queue.Empty:
            pass
        self.after(80, self._poll_events)

    def _refresh_table(self):
        for z in sorted(ZONAS.values(), key=lambda z: z.id):
            self.tree.item(self.rows[z.id], values=self._vals(z))
        self.after(300, self._refresh_table)

    def on_close(self):
        try:
            if self.listener and self.listener.is_alive():
                self.listener.stop()
        except:
            pass
        self.destroy()

if _name_ == "_main_":
    App().mainloop()
