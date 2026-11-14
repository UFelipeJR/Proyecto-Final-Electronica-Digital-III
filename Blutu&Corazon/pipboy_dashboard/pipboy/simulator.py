"""Backend determinista para probar la aplicacion sin hardware BLE."""

from __future__ import annotations

import asyncio
import math
from collections.abc import Callable

from pipboy.message_processor import LineMessageParser
from pipboy.models import AppEvent, CharacteristicInfo, ConnectionState, DeviceInfo


class SimulatedBleManager:
    ADDRESS = "SIM:HC-08:01"

    def __init__(self, emit: Callable[[AppEvent], None]) -> None:
        self.emit = emit
        self.connected = False
        self._parser = LineMessageParser()
        self._telemetry_task: asyncio.Task[None] | None = None

    async def scan(self, timeout: float = 0.35) -> list[DeviceInfo]:
        self.emit(AppEvent("state", ConnectionState.SCANNING))
        await asyncio.sleep(timeout)
        devices = [DeviceInfo("SH-HC-08 (SIMULADO)", self.ADDRESS, -48)]
        self.emit(AppEvent("devices", devices))
        self.emit(AppEvent("state", ConnectionState.DISCONNECTED))
        self.emit(AppEvent("log", ("SIM", "Dispositivo simulado encontrado.")))
        return devices

    async def connect(self, address: str) -> None:
        if address != self.ADDRESS:
            self.emit(AppEvent("error", "Dispositivo simulado desconocido."))
            return
        self.emit(AppEvent("state", ConnectionState.CONNECTING))
        await asyncio.sleep(0.25)
        self.connected = True
        characteristics = [
            CharacteristicInfo(
                "0000ffe0-0000-1000-8000-00805f9b34fb",
                "0000ffe1-0000-1000-8000-00805f9b34fb",
                ("read", "write-without-response", "notify"),
            )
        ]
        self.emit(AppEvent("gatt", characteristics))
        self.emit(AppEvent("state", ConnectionState.CONNECTED))
        self.emit(AppEvent("log", ("SIM", "Conexion simulada activa; no usa hardware.")))
        self._telemetry_task = asyncio.create_task(self._telemetry_loop())

    async def disconnect(self) -> None:
        self.emit(AppEvent("state", ConnectionState.DISCONNECTING))
        self.connected = False
        if self._telemetry_task:
            self._telemetry_task.cancel()
            await asyncio.gather(self._telemetry_task, return_exceptions=True)
            self._telemetry_task = None
        self._parser.reset()
        await asyncio.sleep(0.05)
        self.emit(AppEvent("state", ConnectionState.DISCONNECTED))
        self.emit(AppEvent("log", ("SIM", "Simulador desconectado.")))

    async def send(self, message: str) -> None:
        if not self.connected:
            self.emit(AppEvent("error", "El simulador no esta conectado."))
            return
        normalized = message.rstrip("\r\n")
        self.emit(AppEvent("tx", normalized))
        await asyncio.sleep(0.08)
        response = "PONG\n" if normalized.upper() == "PING" else f"ECHO: {normalized}\n"
        split = max(1, len(response) // 2)
        for fragment in (response[:split], response[split:]):
            await asyncio.sleep(0.06)
            for complete in self._parser.feed(fragment.encode("utf-8")):
                self.emit(AppEvent("rx", complete))

    async def _telemetry_loop(self) -> None:
        tick = 0
        while self.connected:
            await asyncio.sleep(2.0)
            tick += 1
            bpm = 72 + round(4 * math.sin(tick / 2))
            temperature = 36.5 + round(0.2 * math.sin(tick / 3), 1)
            battery = max(5, 87 - tick // 15)
            message = (
                f"BPM={bpm},SPO2=98,TEMP={temperature:.1f},BAT={battery}\n"
            )
            # Se fragmenta a proposito para ejercitar el mismo parser que BLE real.
            for fragment in (message[:7], message[7:19], message[19:]):
                for complete in self._parser.feed(fragment.encode("utf-8")):
                    self.emit(AppEvent("rx", complete))

