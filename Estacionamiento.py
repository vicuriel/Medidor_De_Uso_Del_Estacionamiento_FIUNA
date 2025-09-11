class Estacionamiento:
    def _init_(self, id:int, nombre, capacidad_maxima):
        self.id = id
        self.nombre = str(nombre)
        self.capacidad_maxima = int(capacidad_maxima)
        self.capacidad_actual = 0
        # Ajustes simples para mantener valores en rango
        if self.capacidad_maxima < 1:
            self.capacidad_maxima = 1
        if self.capacidad_actual < 0:
            self.capacidad_actual = 0
        if self.capacidad_actual > self.capacidad_maxima:
            self.capacidad_actual = self.capacidad_maxima

    def ingresar(self, n=1):
        """Aumenta ocupación (por defecto en 1), sin pasar la capacidad."""
        self.capacidad_actual = min(self.capacidad_actual + int(n), self.capacidad_maxima)
        return self.capacidad_actual

    def salir(self, n=1):
        """Disminuye ocupación (por defecto en 1), sin bajar de 0."""
        self.capacidad_actual = max(self.capacidad_actual - int(n), 0)
        return self.capacidad_actual

    @property
    def disponibles(self):
        """Lugares libres."""
        return self.capacidad_maxima - self.capacidad_actual

    @property
    def utilizacion(self):
        """Ocupación relativa 0..1."""
        return self.capacidad_actual / self.capacidad_maxima if self.capacidad_maxima else 0.0

    def snapshot(self):
        """Estado listo para mostrar o enviar a una API."""
        return {
            "ID": self.id,
            "Nombre/n": self.nombre,
            "Capacidad máxima/n": self.capacidad_maxima,
            "Capacidad actual/n": self.capacidad_actual,
            "Lugares disponibles": self.disponibles,
            "Nivel de utilización": round(self.utilizacion, 3),
        }
    
    def snapshot_text(self, porcentaje: bool = True) -> str:
        util = f"{self.utilizacion*100:.1f}%" if porcentaje else f"{self.utilizacion:.3f}"
        return (
            f"ID: {self.id}\n"
            f"Nombre: {self.nombre}\n"
            f"Capacidad máxima: {self.capacidad_maxima}\n"
            f"Capacidad actual: {self.capacidad_actual}\n"
            f"Lugares disponibles: {self.disponibles}\n"
            f"Nivel de utilización: {util}"
        )
