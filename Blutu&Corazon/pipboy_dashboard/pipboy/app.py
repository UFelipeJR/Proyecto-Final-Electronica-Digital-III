"""Interfaz CustomTkinter de Pipboy Dashboard."""

from __future__ import annotations

import queue
from datetime import datetime
from typing import Any

import customtkinter as ctk

from pipboy.async_runner import AsyncRunner
from pipboy.ble_manager import BleManager
from pipboy.message_processor import parse_sensor_message
from pipboy.models import AppEvent, CharacteristicInfo, ConnectionState, DeviceInfo
from pipboy.simulator import SimulatedBleManager

GREEN = "#39ff88"
GREEN_DARK = "#1fbf68"
PANEL = "#101a16"
PANEL_ALT = "#15231d"
BACKGROUND = "#08110d"
MUTED = "#7ba88f"
RED = "#ff5d68"
AMBER = "#ffcc66"


class MetricCard(ctk.CTkFrame):
    def __init__(self, master: Any, title: str, unit: str) -> None:
        super().__init__(master, fg_color=PANEL, corner_radius=12, border_width=1, border_color="#214f36")
        self.grid_columnconfigure(0, weight=1)
        ctk.CTkLabel(
            self, text=title, text_color=MUTED, font=ctk.CTkFont(size=13, weight="bold")
        ).grid(row=0, column=0, padx=14, pady=(12, 1), sticky="w")
        self.value_label = ctk.CTkLabel(
            self, text="--", text_color=GREEN, font=ctk.CTkFont(size=30, weight="bold")
        )
        self.value_label.grid(row=1, column=0, padx=14, pady=0)
        ctk.CTkLabel(self, text=unit, text_color=MUTED, font=ctk.CTkFont(size=12)).grid(
            row=2, column=0, padx=14, pady=(0, 5)
        )
        self.source_label = ctk.CTkLabel(
            self, text="SIN DATOS", text_color=AMBER, font=ctk.CTkFont(size=10, weight="bold")
        )
        self.source_label.grid(row=3, column=0, padx=14, pady=(0, 10))

    def set_value(self, value: str, simulated: bool) -> None:
        self.value_label.configure(text=value)
        self.source_label.configure(
            text="SIMULADO" if simulated else "SENSOR",
            text_color=AMBER if simulated else GREEN,
        )

    def clear(self) -> None:
        self.value_label.configure(text="--")
        self.source_label.configure(text="SIN DATOS", text_color=AMBER)


class PipboyApp(ctk.CTk):
    def __init__(self, start_in_simulation: bool = True) -> None:
        ctk.set_appearance_mode("dark")
        ctk.set_default_color_theme("green")
        super().__init__()
        self.title("Pipboy Dashboard")
        self.geometry("1120x720")
        self.minsize(950, 650)
        self.configure(fg_color=BACKGROUND)

        self.events: queue.Queue[AppEvent] = queue.Queue()
        self.runner = AsyncRunner()
        self.devices: dict[str, DeviceInfo] = {}
        self.display_to_address: dict[str, str] = {}
        self.connection_state = ConnectionState.DISCONNECTED
        self.simulation_mode = start_in_simulation
        self.manager: BleManager | SimulatedBleManager = self._new_manager()
        self._closing = False

        self._build_ui()
        if start_in_simulation:
            self.mode_switch.select()
            self._set_initial_simulated_values()
        else:
            self.mode_switch.deselect()
        self._apply_state(ConnectionState.DISCONNECTED)
        self.protocol("WM_DELETE_WINDOW", self.close)
        self.after(50, self._drain_events)
        self.after(150, self.scan_devices)

    def _new_manager(self) -> BleManager | SimulatedBleManager:
        return SimulatedBleManager(self.events.put) if self.simulation_mode else BleManager(self.events.put)

    def _build_ui(self) -> None:
        self.grid_columnconfigure(0, weight=1)
        self.grid_rowconfigure(2, weight=1)

        header = ctk.CTkFrame(self, fg_color="transparent")
        header.grid(row=0, column=0, padx=22, pady=(18, 8), sticky="ew")
        header.grid_columnconfigure(1, weight=1)
        ctk.CTkLabel(
            header,
            text="PIPBOY // DASHBOARD",
            text_color=GREEN,
            font=ctk.CTkFont(family="Consolas", size=26, weight="bold"),
        ).grid(row=0, column=0, sticky="w")
        self.status_dot = ctk.CTkLabel(
            header, text="●", width=22, text_color=RED, font=ctk.CTkFont(size=22)
        )
        self.status_dot.grid(row=0, column=2, padx=(10, 2))
        self.status_label = ctk.CTkLabel(
            header, text="DESCONECTADO", text_color=MUTED, font=ctk.CTkFont(size=13, weight="bold")
        )
        self.status_label.grid(row=0, column=3, padx=(0, 18))
        self.mode_switch = ctk.CTkSwitch(
            header,
            text="MODO SIMULACION",
            command=self._toggle_mode,
            progress_color=GREEN_DARK,
            button_color=GREEN,
            text_color=MUTED,
        )
        self.mode_switch.grid(row=0, column=4, sticky="e")

        metrics = ctk.CTkFrame(self, fg_color="transparent")
        metrics.grid(row=1, column=0, padx=22, pady=8, sticky="ew")
        for column in range(4):
            metrics.grid_columnconfigure(column, weight=1, uniform="metric")
        definitions = [
            ("bpm", "RITMO CARDIACO", "BPM"),
            ("spo2", "OXIGENO", "% SpO2"),
            ("temperature", "TEMPERATURA", "°C"),
            ("battery", "BATERIA", "%"),
        ]
        self.metric_cards: dict[str, MetricCard] = {}
        for column, (key, title, unit) in enumerate(definitions):
            card = MetricCard(metrics, title, unit)
            card.grid(row=0, column=column, padx=(0 if column == 0 else 6, 0 if column == 3 else 6), sticky="ew")
            self.metric_cards[key] = card

        content = ctk.CTkFrame(self, fg_color="transparent")
        content.grid(row=2, column=0, padx=22, pady=8, sticky="nsew")
        content.grid_columnconfigure(0, weight=0, minsize=300)
        content.grid_columnconfigure(1, weight=1)
        content.grid_rowconfigure(0, weight=1)

        controls = ctk.CTkFrame(content, fg_color=PANEL, corner_radius=12)
        controls.grid(row=0, column=0, padx=(0, 12), sticky="nsew")
        controls.grid_columnconfigure(0, weight=1)
        ctk.CTkLabel(
            controls, text="ENLACE BLE", text_color=GREEN, font=ctk.CTkFont(size=16, weight="bold")
        ).grid(row=0, column=0, padx=16, pady=(18, 8), sticky="w")
        self.device_combo = ctk.CTkComboBox(
            controls,
            values=["Escanea para buscar dispositivos"],
            state="readonly",
            fg_color=PANEL_ALT,
            border_color="#285b3e",
            button_color=GREEN_DARK,
            dropdown_fg_color=PANEL_ALT,
        )
        self.device_combo.grid(row=1, column=0, padx=16, pady=8, sticky="ew")
        self.scan_button = self._button(controls, "ESCANEAR", self.scan_devices)
        self.scan_button.grid(row=2, column=0, padx=16, pady=(8, 4), sticky="ew")
        self.connect_button = self._button(controls, "CONECTAR", self.connect_device)
        self.connect_button.grid(row=3, column=0, padx=16, pady=4, sticky="ew")
        self.disconnect_button = self._button(controls, "DESCONECTAR", self.disconnect_device, danger=True)
        self.disconnect_button.grid(row=4, column=0, padx=16, pady=4, sticky="ew")
        ctk.CTkLabel(
            controls,
            text="El modo real usa BLE/GATT.\nNo utiliza ni crea un puerto COM.",
            justify="left",
            text_color=MUTED,
            font=ctk.CTkFont(size=11),
        ).grid(row=5, column=0, padx=16, pady=(14, 8), sticky="w")
        self.gatt_label = ctk.CTkLabel(
            controls,
            text="GATT: pendiente",
            justify="left",
            wraplength=265,
            text_color=MUTED,
            font=ctk.CTkFont(family="Consolas", size=10),
        )
        self.gatt_label.grid(row=6, column=0, padx=16, pady=8, sticky="nw")

        terminal = ctk.CTkFrame(content, fg_color=PANEL, corner_radius=12)
        terminal.grid(row=0, column=1, sticky="nsew")
        terminal.grid_columnconfigure(0, weight=1)
        terminal.grid_rowconfigure(1, weight=1)
        ctk.CTkLabel(
            terminal, text="TRAFICO / TELEMETRIA", text_color=GREEN, font=ctk.CTkFont(size=16, weight="bold")
        ).grid(row=0, column=0, padx=16, pady=(16, 6), sticky="w")
        self.history = ctk.CTkTextbox(
            terminal,
            state="disabled",
            fg_color="#07100c",
            text_color="#a6ffc8",
            border_width=1,
            border_color="#214f36",
            font=ctk.CTkFont(family="Consolas", size=12),
            wrap="word",
        )
        self.history.grid(row=1, column=0, columnspan=2, padx=16, pady=8, sticky="nsew")
        self.message_entry = ctk.CTkEntry(
            terminal,
            placeholder_text="Mensaje personalizado (se agregara \\n automaticamente)",
            fg_color=PANEL_ALT,
            border_color="#285b3e",
        )
        self.message_entry.grid(row=2, column=0, padx=(16, 7), pady=(8, 16), sticky="ew")
        self.message_entry.bind("<Return>", lambda _event: self.send_custom())
        button_box = ctk.CTkFrame(terminal, fg_color="transparent")
        button_box.grid(row=2, column=1, padx=(0, 16), pady=(8, 16))
        self.ping_button = self._button(button_box, "PING", self.send_ping)
        self.ping_button.grid(row=0, column=0, padx=(0, 5))
        self.send_button = self._button(button_box, "ENVIAR", self.send_custom)
        self.send_button.grid(row=0, column=1)

        footer = ctk.CTkLabel(
            self,
            text="RP2040  •  SH-HC-08  •  UART1 9600 baud  •  Mensajes delimitados por LF",
            text_color="#4f7960",
            font=ctk.CTkFont(family="Consolas", size=10),
        )
        footer.grid(row=3, column=0, padx=22, pady=(3, 12), sticky="w")

    @staticmethod
    def _button(master: Any, text: str, command: Any, danger: bool = False) -> ctk.CTkButton:
        color = "#7c3038" if danger else "#176b42"
        hover = "#a8404b" if danger else GREEN_DARK
        return ctk.CTkButton(
            master,
            text=text,
            command=command,
            fg_color=color,
            hover_color=hover,
            text_color="#e5fff0",
            font=ctk.CTkFont(size=12, weight="bold"),
        )

    def _set_initial_simulated_values(self) -> None:
        for key, value in {"bpm": "72", "spo2": "98", "temperature": "36.5", "battery": "87"}.items():
            self.metric_cards[key].set_value(value, simulated=True)

    def _toggle_mode(self) -> None:
        wanted_simulation = bool(self.mode_switch.get())
        if wanted_simulation == self.simulation_mode:
            return
        if self.connection_state != ConnectionState.DISCONNECTED:
            self._append_log("SYS", "Desconecta antes de cambiar de modo.")
            self.mode_switch.select() if self.simulation_mode else self.mode_switch.deselect()
            return
        self.simulation_mode = wanted_simulation
        self.manager = self._new_manager()
        self.devices.clear()
        self.display_to_address.clear()
        self.device_combo.configure(values=["Escanea para buscar dispositivos"])
        self.device_combo.set("Escanea para buscar dispositivos")
        self.gatt_label.configure(text="GATT: pendiente")
        if self.simulation_mode:
            self._set_initial_simulated_values()
            self._append_log("SIM", "Modo simulacion seleccionado.")
        else:
            for card in self.metric_cards.values():
                card.clear()
            self._append_log("SYS", "Modo BLE real seleccionado.")
        self.scan_devices()

    def _submit(self, coroutine: Any) -> None:
        self.runner.submit(coroutine)

    def scan_devices(self) -> None:
        if self.connection_state in {ConnectionState.CONNECTED, ConnectionState.CONNECTING, ConnectionState.DISCONNECTING}:
            return
        self._submit(self.manager.scan())

    def connect_device(self) -> None:
        display = self.device_combo.get()
        address = self.display_to_address.get(display)
        if not address:
            self._append_log("ERR", "Selecciona un dispositivo del ultimo escaneo.")
            return
        self._submit(self.manager.connect(address))

    def disconnect_device(self) -> None:
        self._submit(self.manager.disconnect())

    def send_ping(self) -> None:
        self._submit(self.manager.send("PING"))

    def send_custom(self) -> None:
        message = self.message_entry.get()
        if not message.strip():
            return
        self.message_entry.delete(0, "end")
        self._submit(self.manager.send(message))

    def _drain_events(self) -> None:
        if self._closing:
            return
        while True:
            try:
                event = self.events.get_nowait()
            except queue.Empty:
                break
            self._handle_event(event)
        self.after(50, self._drain_events)

    def _handle_event(self, event: AppEvent) -> None:
        if event.kind == "state":
            self._apply_state(event.data)
        elif event.kind == "devices":
            self._show_devices(event.data)
        elif event.kind == "gatt":
            self._show_gatt(event.data)
        elif event.kind in {"tx", "rx"}:
            self._append_log(event.kind.upper(), str(event.data))
            if event.kind == "rx":
                self._update_sensor_values(str(event.data))
        elif event.kind == "log":
            source, message = event.data
            self._append_log(source, message)
        elif event.kind == "error":
            self._append_log("ERR", str(event.data))

    def _apply_state(self, state: ConnectionState) -> None:
        self.connection_state = state
        connected = state == ConnectionState.CONNECTED
        busy = state in {ConnectionState.SCANNING, ConnectionState.CONNECTING, ConnectionState.DISCONNECTING}
        self.status_label.configure(text=state.value.upper())
        self.status_dot.configure(text_color=GREEN if connected else (AMBER if busy else RED))
        self.scan_button.configure(state="disabled" if busy or connected else "normal")
        self.connect_button.configure(state="disabled" if busy or connected or not self.devices else "normal")
        self.disconnect_button.configure(state="normal" if connected else "disabled")
        message_state = "normal" if connected else "disabled"
        self.ping_button.configure(state=message_state)
        self.send_button.configure(state=message_state)
        self.message_entry.configure(state=message_state)
        self.mode_switch.configure(state="disabled" if busy or connected else "normal")

    def _show_devices(self, devices: list[DeviceInfo]) -> None:
        self.devices = {item.address: item for item in devices}
        self.display_to_address = {item.display_name: item.address for item in devices}
        values = list(self.display_to_address) or ["No se encontraron dispositivos"]
        self.device_combo.configure(values=values)
        self.device_combo.set(values[0])
        self._apply_state(self.connection_state)

    def _show_gatt(self, characteristics: list[CharacteristicInfo]) -> None:
        lines = ["GATT descubierto:"]
        for item in characteristics:
            props = ", ".join(item.properties)
            lines.append(f"• {item.uuid}\n  [{props}]")
            self._append_log("GATT", f"{item.service_uuid} / {item.uuid} [{props}]")
        self.gatt_label.configure(text="\n".join(lines))

    def _update_sensor_values(self, message: str) -> None:
        for reading in parse_sensor_message(message):
            if reading.key in {"bpm", "spo2", "battery"}:
                value = str(int(reading.value))
            else:
                value = f"{reading.value:.1f}"
            self.metric_cards[reading.key].set_value(value, simulated=self.simulation_mode)

    def _append_log(self, direction: str, message: str) -> None:
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]
        safe_message = message.replace("\r", "\\r").replace("\n", "\\n")
        self.history.configure(state="normal")
        self.history.insert("end", f"[{timestamp}] {direction:<4}  {safe_message}\n")
        self.history.see("end")
        self.history.configure(state="disabled")

    def close(self) -> None:
        if self._closing:
            return
        self._closing = True
        try:
            future = self.runner.submit(self.manager.disconnect())
            future.result(timeout=2)
        except Exception:
            pass
        self.runner.stop()
        self.destroy()

