"""Transporte BLE real basado en Bleak (no usa puertos COM)."""

from __future__ import annotations

import asyncio
from collections.abc import Callable
from typing import Any

from bleak import BleakClient, BleakScanner

from pipboy.characteristics import select_characteristics
from pipboy.message_processor import LineMessageParser
from pipboy.models import AppEvent, CharacteristicInfo, ConnectionState, DeviceInfo

EventCallback = Callable[[AppEvent], None]


class BleManager:
    def __init__(self, emit: EventCallback) -> None:
        self.emit = emit
        self.client: BleakClient | None = None
        self._devices: dict[str, Any] = {}
        self._write_characteristic: CharacteristicInfo | None = None
        self._notify_characteristic: CharacteristicInfo | None = None
        self._parser = LineMessageParser()
        self._intentional_disconnect = False

    @property
    def connected(self) -> bool:
        return bool(self.client and self.client.is_connected)

    async def scan(self, timeout: float = 5.0) -> list[DeviceInfo]:
        self.emit(AppEvent("state", ConnectionState.SCANNING))
        self.emit(AppEvent("log", ("SYS", "Buscando dispositivos BLE cercanos...")))
        try:
            discovered = await BleakScanner.discover(timeout=timeout, return_adv=True)
            devices: list[DeviceInfo] = []
            self._devices.clear()
            for address, (device, advertisement) in discovered.items():
                name = device.name or advertisement.local_name or "Dispositivo sin nombre"
                self._devices[address] = device
                devices.append(DeviceInfo(name, address, advertisement.rssi))
            devices.sort(key=lambda item: ("hc-08" not in item.name.lower(), item.name.lower()))
            self.emit(AppEvent("devices", devices))
            self.emit(AppEvent("log", ("SYS", f"Escaneo finalizado: {len(devices)} dispositivo(s).")))
            return devices
        except Exception as exc:
            self.emit(AppEvent("error", f"No fue posible escanear BLE: {exc}"))
            return []
        finally:
            if not self.connected:
                self.emit(AppEvent("state", ConnectionState.DISCONNECTED))

    async def connect(self, address: str) -> None:
        if self.connected:
            return
        device = self._devices.get(address)
        if device is None:
            self.emit(AppEvent("error", "El dispositivo ya no esta en el ultimo escaneo."))
            return

        self.emit(AppEvent("state", ConnectionState.CONNECTING))
        self._intentional_disconnect = False
        self._parser.reset()
        try:
            self.client = BleakClient(device, disconnected_callback=self._on_disconnected)
            await self.client.connect()
            characteristics = self._inspect_services(self.client.services)
            self._write_characteristic, self._notify_characteristic = select_characteristics(
                characteristics
            )
            self.emit(AppEvent("gatt", characteristics))
            if self._write_characteristic is None:
                raise RuntimeError("No se encontro una caracteristica con propiedad de escritura")
            if self._notify_characteristic is None:
                raise RuntimeError("No se encontro una caracteristica con notificaciones/indicaciones")

            await self.client.start_notify(
                self._notify_characteristic.uuid, self._notification_handler
            )
            self.emit(AppEvent("state", ConnectionState.CONNECTED))
            self.emit(
                AppEvent(
                    "log",
                    (
                        "SYS",
                        "BLE conectado. TX: "
                        f"{self._write_characteristic.uuid} | RX: {self._notify_characteristic.uuid}",
                    ),
                )
            )
        except Exception as exc:
            self.emit(AppEvent("error", f"Error al conectar: {exc}"))
            await self._cleanup_client()
            self.emit(AppEvent("state", ConnectionState.DISCONNECTED))

    @staticmethod
    def _inspect_services(services: Any) -> list[CharacteristicInfo]:
        result: list[CharacteristicInfo] = []
        for service in services:
            for characteristic in service.characteristics:
                result.append(
                    CharacteristicInfo(
                        service_uuid=str(service.uuid),
                        uuid=str(characteristic.uuid),
                        properties=tuple(str(prop).lower() for prop in characteristic.properties),
                    )
                )
        return result

    async def disconnect(self) -> None:
        self._intentional_disconnect = True
        if not self.client:
            self.emit(AppEvent("state", ConnectionState.DISCONNECTED))
            return
        self.emit(AppEvent("state", ConnectionState.DISCONNECTING))
        try:
            if self.client.is_connected and self._notify_characteristic:
                try:
                    await self.client.stop_notify(self._notify_characteristic.uuid)
                except Exception:
                    pass
            if self.client.is_connected:
                await self.client.disconnect()
        finally:
            await self._cleanup_client()
            self.emit(AppEvent("state", ConnectionState.DISCONNECTED))
            self.emit(AppEvent("log", ("SYS", "BLE desconectado.")))

    async def _cleanup_client(self) -> None:
        client, self.client = self.client, None
        if client and client.is_connected:
            try:
                await client.disconnect()
            except Exception:
                pass
        self._write_characteristic = None
        self._notify_characteristic = None
        self._parser.reset()

    def _on_disconnected(self, _client: BleakClient) -> None:
        self._write_characteristic = None
        self._notify_characteristic = None
        self._parser.reset()
        self.emit(AppEvent("state", ConnectionState.DISCONNECTED))
        if not self._intentional_disconnect:
            self.emit(
                AppEvent(
                    "error",
                    "El dispositivo BLE se desconecto inesperadamente. Puedes reconectar.",
                )
            )

    def _notification_handler(self, _sender: Any, data: bytearray) -> None:
        try:
            for message in self._parser.feed(data):
                self.emit(AppEvent("rx", message))
        except BufferError as exc:
            self.emit(AppEvent("error", str(exc)))

    async def send(self, message: str) -> None:
        if not self.connected or not self.client or not self._write_characteristic:
            self.emit(AppEvent("error", "No hay una conexion BLE activa."))
            return
        payload = message.rstrip("\r\n") + "\n"
        props = set(self._write_characteristic.properties)
        response = "write" in props
        try:
            await self.client.write_gatt_char(
                self._write_characteristic.uuid,
                payload.encode("utf-8"),
                response=response,
            )
            self.emit(AppEvent("tx", payload[:-1]))
        except Exception as exc:
            self.emit(AppEvent("error", f"No fue posible enviar el mensaje: {exc}"))


async def cancel_safely(task: asyncio.Task[Any] | None) -> None:
    if task and not task.done():
        task.cancel()
        await asyncio.gather(task, return_exceptions=True)

