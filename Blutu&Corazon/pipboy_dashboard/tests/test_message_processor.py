import pytest

from pipboy.message_processor import LineMessageParser, parse_sensor_message


def test_reassembles_fragmented_ble_message() -> None:
    parser = LineMessageParser()
    assert parser.feed(b"PO") == []
    assert parser.feed(b"NG") == []
    assert parser.feed(b"\n") == ["PONG"]


def test_extracts_multiple_messages_and_preserves_remainder() -> None:
    parser = LineMessageParser()
    assert parser.feed(b"ONE\nTWO\r\nTH") == ["ONE", "TWO"]
    assert parser.pending_bytes == b"TH"
    assert parser.feed(b"REE\n") == ["THREE"]


def test_replaces_invalid_utf8_without_losing_line() -> None:
    parser = LineMessageParser()
    assert parser.feed(b"A\xffB\n") == ["A\ufffdB"]


def test_buffer_limit_resets_parser() -> None:
    parser = LineMessageParser(max_buffer_size=4)
    with pytest.raises(BufferError):
        parser.feed(b"12345")
    assert parser.pending_bytes == b""


def test_parses_json_sensor_message() -> None:
    readings = parse_sensor_message(
        '{"bpm": 74, "spo2": 97, "temperature": 36.7, "battery": 80}'
    )
    assert {item.key: item.value for item in readings} == {
        "bpm": 74.0,
        "spo2": 97.0,
        "temperature": 36.7,
        "battery": 80.0,
    }


def test_parses_key_value_telemetry_and_ignores_invalid_ranges() -> None:
    readings = parse_sensor_message("BPM=72,SPO2=101,TEMP:36.5 BAT=84 unknown=9")
    assert {item.key: item.value for item in readings} == {
        "bpm": 72.0,
        "temperature": 36.5,
        "battery": 84.0,
    }

