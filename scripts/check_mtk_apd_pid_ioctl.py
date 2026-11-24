#!/usr/bin/env python3
"""Guard fixed-size MTK AP private-ioctl copies against unsafe user buffers."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCES = (
    ROOT / "trunk/linux-4.4.x/drivers/net/wireless/mediatek/mt76x2/ap/ap_cfg.c",
    ROOT / "trunk/linux-4.4.x/drivers/net/wireless/mediatek/mt76x3/ap/ap_cfg.c",
    ROOT / "trunk/linux-4.4.x/drivers/net/wireless/mediatek/mt7615/embedded/ap/ap_cfg.c",
    ROOT / "trunk/linux-4.4.x/drivers/net/wireless/mediatek/mt7915/embedded/ap/ap_cfg.c",
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



FT_BOOL_CASES = (
    ("OID_802_11R_SUPPORT", "OID_802_11R_RIC", "Dot11rFtEnable"),
    ("OID_802_11R_RIC", "OID_802_11R_OTD", "RsrReqCap"),
    ("OID_802_11R_OTD", "#endif /* DOT11R_FT_SUPPORT */", "FtOverDs"),
)


def extract_ft_case(source: str, start_marker: str, end_marker: str) -> str:
    start = source.index(f"case {start_marker}:")
    end_marker = end_marker if end_marker.startswith("#") else f"case {end_marker}:"
    return source[start:source.index(end_marker, start + 1)]


def verify_ft_boolean_copy(path: Path, source: str) -> None:
    for name, next_name, field in FT_BOOL_CASES:
        case = extract_ft_case(source, name, next_name)
        assert "BOOLEAN value;" in case, f"{path}: {name} must use BOOLEAN"
        assert "ULONG value;" not in case, f"{path}: {name} must not use ULONG"
        assert "if (wrq->u.data.length != sizeof(BOOLEAN))" in case, (
            f"{path}: {name} must validate BOOLEAN input length"
        )
        guard = "if (copy_from_user(&value, wrq->u.data.pointer, sizeof(value)))"
        assert guard in case, f"{path}: {name} must copy exactly sizeof(value)"
        copy_pos = case.index(guard)
        success_pos = case.index("else", copy_pos)
        assert "Status = -EFAULT;" in case[copy_pos:success_pos], (
            f"{path}: {name} must fail closed on copy_from_user error"
        )
        prefix = "pAd->ApCfg.MBSSID[apidx]"
        if ".wdev.FtCfg" in case:
            prefix += ".wdev"
        target = f"{prefix}.FtCfg.FtCapFlag.{field} = (value == 0 ? FALSE : TRUE);"
        assert case.find(target) > success_pos, (
            f"{path}: {name} may update only after a successful copy"
        )
        assert "Status = copy_from_user(&value" not in case, (
            f"{path}: {name} must not consume partially initialized stack data"
        )



def verify_psk_length_contract(path: Path, source: str) -> None:
    case = extract_case(source, "case OID_SET_PSK:", "#ifdef SNMP_SUPPORT")
    length_guard = "if (wrq->u.data.length > sizeof(PSK) - 1)"
    copy_guard = "if (copy_from_user(PSK,"
    assert length_guard in case, f"{path}: PSK length must be capped at 64 bytes"
    assert copy_guard in case, f"{path}: stage PSK in the bounded local buffer"
    assert case.index(length_guard) < case.index(copy_guard), (
        f"{path}: reject oversized PSK input before copy_from_user"
    )
    copy_pos = case.index(copy_guard)
    error_pos = case.index("Status = -EFAULT;", copy_pos)
    error_break = case.index("break;", error_pos)
    terminator_pos = case.index("PSK[wrq->u.data.length] = '\\0';")
    assert copy_pos < error_pos < error_break < terminator_pos, (
        f"{path}: failed user copies must stop before using the staged PSK"
    )
    assert case.count("copy_from_user(") == 1, (
        f"{path}: all PSK destinations must use the validated staged copy"
    )
    assert "NdisMoveMemory(pAd->ApCfg.ApCliTab[pObj->ioctl_if].PSK," in case
    assert "NdisMoveMemory(pAd->ApCfg.MBSSID[pObj->ioctl_if].WPAKeyString," in case
    assert "NdisMoveMemory(pWscControl->WpaPsk, PSK," in case
    assert case.count("wrq->u.data.length + 1") == 2, (
        f"{path}: the two string destinations must copy only the validated terminator"
    )
    assert "sizeof(pWscControl->WpaPsk)" in case, (
        f"{path}: zero the complete fixed-size WSC PSK storage"
    )
    assert "__func__, PSK" in case, (
        f"{path}: log only the NUL-terminated staging buffer"
    )
    assert 'pWscControl->WpaPsk));' not in case, (
        f"{path}: the WSC PSK array is not NUL-terminated"
    )

def main() -> None:
    for path in SOURCES:
        source = path.read_text(encoding="utf-8")
        if path in SOURCES[:2]:
            for case in CASES:
                verify_fixed_copy(path, source, case)
        verify_ft_boolean_copy(path, source)
        if path == SOURCES[1]:
            verify_psk_length_contract(path, source)
        print(f"MTK AP ioctl copy contracts: PASS ({path.relative_to(ROOT)})")


if __name__ == "__main__":
    main()
