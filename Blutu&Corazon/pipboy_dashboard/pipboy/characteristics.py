"""Seleccion de de caracteristicas GATT a partir de sus propiedades."""

from __future__ import annotations

from collections.abc import Iterable

from pipboy.models import CharacteristicInfo


def _is_ffe1(uuid: str) -> bool:
    normalized = uuid.lower().replace("-", "")
    return normalized == "ffe1" or normalized.startswith("0000ffe1")


def select_characteristics(
    characteristics: Iterable[CharacteristicInfo],
) -> tuple[CharacteristicInfo | None, CharacteristicInfo | None]:
    """Retorna (escritura, notificacion), priorizando FFE1 si es utilizable."""

    items = list(characteristics)
    writable = [
        item
        for item in items
        if {"write", "write-without-response"}.intersection(item.properties)
    ]
    notifiable = [
        item
        for item in items
        if {"notify", "indicate"}.intersection(item.properties)
    ]
    writable.sort(key=lambda item: not _is_ffe1(item.uuid))
    notifiable.sort(key=lambda item: not _is_ffe1(item.uuid))
    return (writable[0] if writable else None, notifiable[0] if notifiable else None)

