# Pipboy Dashboard

Aplicacion de escritorio para Windows que comunica un computador directamente con un
modulo SH-HC-08 por **Bluetooth Low Energy (BLE/GATT)**. No trata el modulo como un
puerto COM.

La aplicacion inicia en modo simulacion para poder verificar la interfaz sin hardware.
El simulador esta identificado en la pantalla y genera valores de BPM, SpO2,
temperatura y bateria marcados como `SIMULADO`.

## Requisitos

- Windows 10 u 11 con Bluetooth LE habilitado.
- Python 3.11 o posterior.
- Adaptador Bluetooth compatible con BLE.

## Instalacion

Desde PowerShell, dentro de esta carpeta:

```powershell
python -m venv .venv
.\.venv\Scripts\Activate.ps1
python -m pip install --upgrade pip
python -m pip install -r requirements.txt
```

Si PowerShell bloquea la activacion, no es obligatorio activarla; se puede usar el
interprete directamente:

```powershell
.\.venv\Scripts\python.exe main.py
```

## Inicio y modo simulado

```powershell
.\.venv\Scripts\Activate.ps1
python main.py
```

El programa inicia con `MODO SIMULACION` activo y escanea automaticamente. Luego:

1. Selecciona `SH-HC-08 (SIMULADO)`.
2. Pulsa **CONECTAR**.
3. Pulsa **PING**.
4. El historial debe mostrar una linea `TX PING` y una linea `RX PONG`.

El PONG simulado se entrega internamente en varios fragmentos para validar la
reconstruccion por salto de linea.

## Prueba con el SH-HC-08 real

1. Enciende la Pico y el SH-HC-08 y habilita Bluetooth en Windows.
2. Desactiva `MODO SIMULACION`. Tambien puedes iniciar directamente con
   `python main.py --real`.
3. Pulsa **ESCANEAR** y selecciona el SH-HC-08 por nombre/direccion.
4. Pulsa **CONECTAR**. El historial mostrara todos los servicios y caracteristicas
   descubiertos con sus propiedades.
5. Comprueba que aparece una caracteristica que permita `write` o
   `write-without-response` y otra que permita `notify` o `indicate`. La aplicacion
   prioriza FFE1 cuando es valida, pero no la presupone.
6. Pulsa **PING**. La aplicacion transmite exactamente los bytes de `PING\n`
   (`50 49 4E 47 0A`). Si firmware y enlace responden, el historial mostrara
   `RX PONG`.

La conexion BLE real queda implementada, pero solo la prueba fisica anterior puede
confirmar la interoperabilidad concreta con tu modulo, su firmware y el adaptador
Bluetooth del computador.

Si hay una desconexion inesperada, el indicador cambia a rojo, se registra el error
y se habilita de nuevo **CONECTAR**. Si Windows ya no conserva el dispositivo, vuelve
a escanear antes de reconectar.

## Telemetria futura

El procesador acepta mensajes terminados en `\n` con cualquiera de estos formatos:

```text
BPM=72,SPO2=98,TEMP=36.5,BAT=84
{"bpm":72,"spo2":98,"temperature":36.5,"battery":84}
```

Los alias `HR`, `HEART_RATE`, `OXYGEN` y `BATTERY` tambien son validos. Al recibir
datos reales en modo BLE, las tarjetas cambian su fuente a `SENSOR`.

## Pruebas automatizadas

```powershell
python -m pytest
python main.py --smoke-test
```

Las pruebas cubren reconstruccion de mensajes fragmentados, multiples lineas por
notificacion, limites del buffer, telemetria JSON/texto, seleccion GATT por propiedades,
los bytes exactos de `PING\n` y el flujo PING/PONG simulado integrado con la GUI.

## Estructura

```text
main.py                         Entrada de la aplicacion
pipboy/app.py                   Interfaz y cola de eventos del hilo Tkinter
pipboy/async_runner.py          Bucle asyncio en un hilo dedicado
pipboy/ble_manager.py           Escaneo, GATT, notificaciones y escritura Bleak
pipboy/simulator.py             Backend sin hardware
pipboy/message_processor.py     Buffer de lineas y telemetria
pipboy/characteristics.py       Seleccion de caracteristicas por propiedades
pipboy/models.py                Modelos compartidos
tests/                          Pruebas sin hardware
```

Tkinter se ejecuta solo en el hilo principal. Bleak y `asyncio` trabajan en un hilo
dedicado y publican eventos mediante una cola segura; la GUI consume esa cola con
`after`, por lo que ningun callback BLE modifica widgets directamente.

