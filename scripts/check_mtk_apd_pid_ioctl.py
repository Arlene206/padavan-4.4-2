#!/usr/bin/env python3
"""Guard the legacy MTK AP daemon PID private ioctl against stack overreads/writes."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCES = (
    ROOT / "trunk/linux-4.4.x/drivers/net/wireless/mediatek/mt76x2/ap/ap_cfg.c",
    ROOT / "trunk/linux-4.4.x/drivers/net/wireless/mediatek/mt76x3/ap/ap_cfg.c",
)


def extract_case(source: str, marker: str, next_marker: str) -> str:
    start = source.index(marker)
    end = source.index(next_marker, start)
    return source[start:end]


def verify_source(path: Path) -> None:
    source = path.read_text(encoding="utf-8")
    case = extract_case(
        source,
        "case RT_SET_APD_PID:",
        "case RT_SET_DEL_MAC_ENTRY:",
    )
    length_guard = re.search(
        r"if\s*\(\s*wrq->u\.data\.length\s*!=\s*"
        r"sizeof\s*\(\s*apd_pid\s*\)\s*\)\s*\{"
        r"\s*Status\s*=\s*-EFAULT\s*;\s*break\s*;\s*\}",
        case,
    )
    copy_call = re.search(
        r"copy_from_user\s*\(\s*&apd_pid\s*,\s*"
        r"wrq->u\.data\.pointer\s*,\s*sizeof\s*\(\s*apd_pid\s*\)\s*\)",
        case,
    )
    assert length_guard, f"{path}: APD PID input length must match sizeof(apd_pid)"
    assert copy_call, f"{path}: copy_from_user must use the fixed destination size"
    assert length_guard.start() < copy_call.start(), (
        f"{path}: validate length before copying into the stack variable"
    )
    assert "copy_from_user(&apd_pid, wrq->u.data.pointer, wrq->u.data.length)" not in case, (
        f"{path}: untrusted ioctl length must never size a stack copy"
    )
    assert case.index("pObj->apd_pid_nr = apd_pid") > copy_call.end(), (
        f"{path}: do not publish APD PID before a successful bounded copy"
    )


def main() -> None:
    for path in SOURCES:
        verify_source(path)
        print(f"APD PID ioctl length contract: PASS ({path.relative_to(ROOT)})")


if __name__ == "__main__":
    main()
