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
    assert 'WSC PSK length = %d' in case, (
        f"{path}: diagnostics must print the length, not the WSC PSK array"
    )


def verify_ft_r0khid_copy(path: Path, source: str) -> None:
    include_by_source = {
        SOURCES[0]: "../../mtk_ft_r0khid_policy.h",
        SOURCES[1]: "../../mtk_ft_r0khid_policy.h",
        SOURCES[2]: "../../../mtk_ft_r0khid_policy.h",
        SOURCES[3]: "../../../mtk_ft_r0khid_policy.h",
    }
    assert f'#include "{include_by_source[path]}"' in source
    case = extract_case(
        source, "case OID_802_11R_R0KHID:", "case OID_802_11R_RIC:"
    )
    compact = " ".join(case.split())
    guard = "if (!mtk_ft_r0khid_length_valid(wrq->u.data.length, FT_ROKH_ID_LEN))"
    temporary = "UCHAR r0kh_id[FT_ROKH_ID_LEN + 1] = {0};"
    copy = "if (copy_from_user(r0kh_id, wrq->u.data.pointer, wrq->u.data.length))"
    assert guard in compact, f"{path}: enforce R0KH-ID length 1..48"
    assert temporary in compact, f"{path}: stage into a zeroed 49-byte buffer"
    assert copy in compact, f"{path}: copy only into the bounded temporary"
    guard_pos = compact.index(guard)
    temporary_pos = compact.index(temporary)
    copy_pos = compact.index(copy)
    error_pos = compact.index("Status = -EFAULT;", copy_pos)
    success_pos = compact.index("else {", error_pos)
    prefix = "pAd->ApCfg.MBSSID[apidx]"
    if ".wdev.FtCfg." in compact:
        prefix += ".wdev"
    destination = prefix + ".FtCfg.FtR0khId"
    destination_len = prefix + ".FtCfg.FtR0khIdLen"
    publish = f"memcpy({destination}, r0kh_id, sizeof(r0kh_id));"
    set_length = f"{destination_len} = wrq->u.data.length;"
    publish_pos = compact.index(publish)
    length_pos = compact.index(set_length)
    assert guard_pos < temporary_pos < copy_pos < error_pos < success_pos < publish_pos < length_pos
    assert "Status = 0;" in compact[success_pos:publish_pos]
    assert f"copy_from_user({destination}," not in compact
    assert "wrq->u.data.length <= FT_ROKH_ID_LEN" not in compact


def verify_ft_r0khid_policy() -> None:
    header = ROOT / "trunk/linux-4.4.x/drivers/net/wireless/mediatek/mtk_ft_r0khid_policy.h"
    test = ROOT / "trunk/linux-4.4.x/drivers/net/wireless/mediatek/tests/ft_r0khid_policy.c"
    policy = header.read_text(encoding="utf-8")
    regression = test.read_text(encoding="utf-8")
    assert "return length != 0U && maximum != 0U && length <= maximum;" in policy
    for path in SOURCES:
        include_dir = path.parents[1] / "include"
        macro = " ".join((include_dir / "dot11r_ft.h").read_text(encoding="utf-8").split())
        structure = " ".join((include_dir / "ft_cmm.h").read_text(encoding="utf-8").split())
        assert "#define FT_ROKH_ID_LEN 48" in macro
        assert "FtR0khId[FT_ROKH_ID_LEN + 1]" in structure
    for marker in ("length_valid(0U, 48U)", "length_valid(1U, 48U)",
                   "length_valid(48U, 48U)", "length_valid(49U, 48U)",
                   "length_valid(UINT_MAX, 48U)"):
        assert marker in regression, f"missing R0KH-ID boundary: {marker}"


def main() -> None:
    verify_ft_r0khid_policy()
    for path in SOURCES:
        source = path.read_text(encoding="utf-8")
        if path in SOURCES[:2]:
            for case in CASES:
                verify_fixed_copy(path, source, case)
        verify_ft_boolean_copy(path, source)
        verify_ft_r0khid_copy(path, source)
        if path == SOURCES[1]:
            verify_psk_length_contract(path, source)
        print(f"MTK AP ioctl copy contracts: PASS ({path.relative_to(ROOT)})")


if __name__ == "__main__":
    main()
