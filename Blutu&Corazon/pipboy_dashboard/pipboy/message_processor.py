"""Reconstruccion de lineas BLE y decodificacion de telemetria."""

from __future__ import annotations

import json
import re
from dataclasses import dataclass


class LineMessageParser:
    """Acumula bytes fragmentados y entrega mensajes delimitados por LF."""

    def __init__(self, max_buffer_size: int = 65_536) -> None:
        self._buffer = bytearray()
        self.max_buffer_size = max_buffer_size

    @property
    def pending_bytes(self) -> bytes:
        return bytes(self._buffer)

    def reset(self) -> None:
        self._buffer.clear()

    def feed(self, chunk: bytes | bytearray) -> list[str]:
        if not isinstance(chunk, (bytes, bytearray)):
            raise TypeError("chunk debe contener bytes")
        self._buffer.extend(chunk)
        if len(self._buffer) > self.max_buffer_size:
            self._buffer.clear()
            raise BufferError("El buffer de recepcion excedio el limite permitido")

        messages: list[str] = []
        while True:
            delimiter = self._buffer.find(b"\n")
            if delimiter < 0:
                break
            raw = bytes(self._buffer[:delimiter])
            del self._buffer[: delimiter + 1]
            if raw.endswith(b"\r"):
                raw = raw[:-1]
            messages.append(raw.decode("utf-8", errors="replace"))
        return messages


@dataclass(frozen=True, slots=True)
class SensorReading:
    key: str
    value: float


SENSOR_ALIASES = {
    "bpm": "bpm",
    "heart_rate": "bpm",
    "hr": "bpm",
    "spo2": "spo2",
    "oxygen": "spo2",
    "temp": "temperature",
    "temperature": "temperature",
    "battery": "battery",
    "bat": "battery",
}
VALID_RANGES = {
    "bpm": (0.0, 300.0),
    "spo2": (0.0, 100.0),
    "temperature": (-50.0, 150.0),
    "battery": (0.0, 100.0),
}


def parse_sensor_message(message: str) -> list[SensorReading]:
    """Acepta JSON o pares ``CLAVE=valor`` separados por coma/espacio.

    Formatos previstos para el futuro firmware::

        {"bpm": 72, "spo2": 98, "temperature": 36.5, "battery": 84}
        BPM=72,SPO2=98,TEMP=36.5,BAT=84
    """

    values: dict[str, object]
    try:
        decoded = json.loads(message)
        values = decoded if isinstance(decoded, dict) else {}
    except json.JSONDecodeError:
        values = {}
        for key, value in re.findall(
            r"([A-Za-z_][A-Za-z0-9_]*)\s*[:=]\s*(-?\d+(?:\.\d+)?)", message
        ):
            values[key] = value

    readings: list[SensorReading] = []
    for raw_key, raw_value in values.items():
        key = SENSOR_ALIASES.get(str(raw_key).lower())
        if not key:
            continue
        try:
            value = float(raw_value)  # type: ignore[arg-type]
        except (TypeError, ValueError):
            continue
        low, high = VALID_RANGES[key]
        if low <= value <= high:
            readings.append(SensorReading(key, value))
    return readings

