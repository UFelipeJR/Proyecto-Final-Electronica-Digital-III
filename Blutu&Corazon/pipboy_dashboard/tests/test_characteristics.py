from pipboy.characteristics import select_characteristics
from pipboy.models import CharacteristicInfo


def characteristic(uuid: str, *properties: str) -> CharacteristicInfo:
    return CharacteristicInfo("service", uuid, properties)


def test_prefers_ffe1_when_it_has_required_properties() -> None:
    items = [
        characteristic("other-write", "write"),
        characteristic("other-notify", "notify"),
        characteristic(
            "0000ffe1-0000-1000-8000-00805f9b34fb",
            "write-without-response",
            "notify",
        ),
    ]
    write, notify = select_characteristics(items)
    assert write is not None and "ffe1" in write.uuid
    assert notify is not None and "ffe1" in notify.uuid


def test_selects_by_properties_when_ffe1_is_absent() -> None:
    write, notify = select_characteristics(
        [
            characteristic("read-only", "read"),
            characteristic("tx", "write-without-response"),
            characteristic("rx", "indicate"),
        ]
    )
    assert write is not None and write.uuid == "tx"
    assert notify is not None and notify.uuid == "rx"


def test_reports_missing_capabilities() -> None:
    write, notify = select_characteristics([characteristic("read", "read")])
    assert write is None
    assert notify is None

