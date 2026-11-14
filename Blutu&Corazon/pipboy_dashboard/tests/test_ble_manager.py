import asyncio

from pipboy.ble_manager import BleManager
from pipboy.models import AppEvent, CharacteristicInfo


class FakeClient:
    is_connected = True

    def __init__(self) -> None:
        self.writes: list[tuple[str, bytes, bool]] = []

    async def write_gatt_char(self, uuid: str, data: bytes, *, response: bool) -> None:
        self.writes.append((uuid, data, response))


def test_ping_is_written_with_one_real_line_feed() -> None:
    events: list[AppEvent] = []
    manager = BleManager(events.append)
    client = FakeClient()
    manager.client = client  # type: ignore[assignment]
    manager._write_characteristic = CharacteristicInfo(  # noqa: SLF001
        "ffe0", "ffe1", ("write-without-response",)
    )

    asyncio.run(manager.send("PING\n"))

    assert client.writes == [("ffe1", b"PING\n", False)]
    assert events == [AppEvent("tx", "PING")]
