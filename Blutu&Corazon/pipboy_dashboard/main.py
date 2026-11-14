"""Punto de entrada de Pipboy Dashboard."""

from __future__ import annotations

import argparse

from pipboy.app import PipboyApp


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Pipboy Dashboard BLE")
    parser.add_argument(
        "--real",
        action="store_true",
        help="iniciar con el modo BLE real seleccionado (por defecto: simulacion)",
    )
    parser.add_argument(
        "--smoke-test",
        action="store_true",
        help="crear la interfaz, procesar un ciclo y cerrarla",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    app = PipboyApp(start_in_simulation=not args.real)
    if args.smoke_test:
        app.withdraw()
        result: list[str] = []

        def connect_simulator() -> None:
            app.connect_device()

        def send_ping() -> None:
            app.send_ping()

        def verify_and_close() -> None:
            history = app.history.get("1.0", "end")
            if "TX    PING" in history and "RX    PONG" in history:
                result.append("GUI + simulacion PING/PONG: OK")
            else:
                result.append(f"FALLO: historial inesperado\n{history}")
            app.close()

        # El constructor agenda un escaneo simulado; estas acciones recorren el
        # flujo real de cola, widgets, hilo asyncio y parser fragmentado.
        app.after(700, connect_simulator)
        app.after(1200, send_ping)
        app.after(1900, verify_and_close)
        app.mainloop()
        if not result or result[0].startswith("FALLO"):
            raise RuntimeError(result[0] if result else "El smoke test no finalizo")
        print(result[0])
        return
    app.mainloop()


if __name__ == "__main__":
    main()

