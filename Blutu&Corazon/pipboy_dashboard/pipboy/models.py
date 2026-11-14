"""Modelos compartidos que no dependen de la interfaz grafica."""

from __future__ import annotations

from dataclasses import dataclass
from enum import Enum
from typing import Any


class ConnectionState(str, Enum):
    DISCONNECTED = "Desconectado"
    SCANNING = "Escaneando"
    CONNECTING = "Conectando"
    CONNECTED = "Conectado"
    DISCONNECTING = "Desconectando"


@dataclass(frozen=True, slots=True)
class DeviceInfo:
    name: str
    address: str
    rssi: int | None = None

    @property
    def display_name(self) -> str:
        signal = f" | {self.rssi} dBm" if self.rssi is not None else ""
        return f"{self.name or 'Dispositivo sin nombre'} | {self.address}{signal}"


@dataclass(frozen=True, slots=True)
class CharacteristicInfo:
    service_uuid: str
    uuid: str
    properties: tuple[str, ...]


@dataclass(frozen=True, slots=True)
class AppEvent:
    kind: str
    data: Any = None

