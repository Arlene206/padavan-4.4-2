#!/usr/bin/env python3
"""Guard fixed-size MTK AP private-ioctl copies against caller-sized stack writes."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCES = (
    ROOT / "trunk/linux-4.4.x/drivers/net/wireless/mediatek/mt76x2/ap/ap_cfg.c",
    ROOT / "trunk/linux-4.4.x/drivers/net/wireless/mediatek/mt76x3/ap/ap_cfg.c",
)

CASES = (
    ("APD PID", "case RT_SET_APD_PID:", "case RT_SET_DEL_MAC_ENTRY:",
     "apd_pid", "sizeof(apd_pid)", "pObj->apd_pid_nr = apd_pid;"),
    ("STA MAC delete", "case RT_SET_DEL_MAC_ENTRY:", "#ifdef WSC_AP_SUPPORT",
     "Addr", "MAC_ADDR_LEN", "pEntry = MacTableLookup(pAd, Addr);"),
    ("IAPP PID", "case RT_SET_IAPP_PID:", "#ifdef DOT11R_FT_SUPPORT",
     "IappPid", "sizeof(IappPid)", "pObj->IappPid_nr = IappPid;"),
    ("WAPI PID", "case OID_802_11_WAPI_PID:",
     "case OID_802_11_PORT_SECURE_STATE:", "wapi_pid",
     "sizeof(wapi_pid)", "pObj->wapi_pid_nr = wapi_pid;"),
)


def extract_case(source: str, marker: str, next_marker: str) -> str:
    start = source.index(marker)
    end = source.index(next_marker, start)
    return source[start:end]


def verify_fixed_copy(path: Path, source: str, case_info: tuple[str, ...]) -> None:
    name, start, end, variable, expected_length, publish = case_info
    case = extract_case(source, start, end)
    length_guard = re.search(
        rf"if\s*\(\s*wrq->u\.data\.length\s*!=\s*{re.escape(expected_length)}"
        rf"\s*\)\s*\{{\s*Status\s*=\s*-EFAULT\s*;\s*break\s*;\s*\}}",
        case,
    )
    if variable == "Addr":
        copy_pattern = (
            r"copy_from_user\s*\(\s*Addr\s*,\s*wrq->u\.data\.pointer\s*,"
            r"\s*MAC_ADDR_LEN\s*\)"
        )
        unsafe_pattern = r"copy_from_user\s*\(\s*Addr\s*,\s*wrq->u\.data\.pointer\s*,\s*wrq->u\.data\.length"
    else:
        copy_pattern = (
            rf"copy_from_user\s*\(\s*&{re.escape(variable)}\s*,\s*"
            rf"wrq->u\.data\.pointer\s*,\s*sizeof\s*\(\s*{re.escape(variable)}\s*\)\s*\)"
        )
        unsafe_pattern = (
            rf"copy_from_user\s*\(\s*&{re.escape(variable)}\s*,\s*"
            r"wrq->u\.data\.pointer\s*,\s*wrq->u\.data\.length"
        )
    copy_call = re.search(copy_pattern, case)
    assert length_guard, f"{path}: {name} must reject non-exact input lengths"
    assert copy_call, f"{path}: {name} copy must use a fixed destination size"
    assert length_guard.start() < copy_call.start(), (
        f"{path}: {name} length check must precede copy_from_user"
    )
    assert not re.search(unsafe_pattern, case), (
        f"{path}: {name} must not copy caller-controlled length into a fixed stack object"
    )
    assert case.index(publish) > copy_call.end(), (
        f"{path}: {name} must not publish/use copied data before the bounded copy"
    )


def main() -> None:
    for path in SOURCES:
        source = path.read_text(encoding="utf-8")
        for case in CASES:
            verify_fixed_copy(path, source, case)
        print(f"MTK fixed-size AP ioctl copy contracts: PASS ({path.relative_to(ROOT)})")


if __name__ == "__main__":
    main()
