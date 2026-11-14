import asyncio

from pipboy.models import AppEvent, ConnectionState
from pipboy.simulator import SimulatedBleManager


def test_simulator_ping_returns_fragmented_pong_as_one_message() -> None:
    events: list[AppEvent] = []

    async def scenario() -> None:
        simulator = SimulatedBleManager(events.append)
        devices = await simulator.scan(timeout=0)
        await simulator.connect(devices[0].address)
        await simulator.send("PING")
        await simulator.disconnect()

    asyncio.run(scenario())
    assert AppEvent("tx", "PING") in events
    assert AppEvent("rx", "PONG") in events
    assert AppEvent("state", ConnectionState.CONNECTED) in events
    assert events[-2:] == [
        AppEvent("state", ConnectionState.DISCONNECTED),
        AppEvent("log", ("SIM", "Simulador desconectado.")),
    ]

