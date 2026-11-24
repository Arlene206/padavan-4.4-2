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



def extract_case_to_next_case(source: str, marker: str) -> str:
    start = source.index(marker)
    tail = source[start + len(marker):]
    following = re.search(r"(?m)^[ \t]*case\s+", tail)
    end = len(source) if following is None else start + len(marker) + following.start()
    return source[start:end]


def verify_checked_usercopy_case(path: Path, source: str, marker: str,
                                 variable: str, expected_type: str) -> None:
    case = extract_case_to_next_case(source, marker)
    compact = " ".join(case.split())
    length_guard = f"wrq->u.data.length != sizeof({expected_type})"
    copy_guard = (
        f"if (copy_from_user(&{variable}, wrq->u.data.pointer, "
        "wrq->u.data.length))"
    )
    assert length_guard in compact, (
        f"{path}: {marker} must enforce its exact input size"
    )
    assert copy_guard in compact, (
        f"{path}: {marker} must check copy_from_user before consuming {variable}"
    )
    length_pos = compact.index(length_guard)
    copy_pos = compact.index(copy_guard)
    error_pos = compact.index("Status = -EFAULT;", copy_pos)
    break_pos = compact.index("break;", error_pos)
    fail_block_end = compact.index("}", break_pos)
    success_pos = compact.index("Status = 0;", fail_block_end)
    assert length_pos < copy_pos < error_pos < break_pos < fail_block_end < success_pos
    assert re.search(rf"\b{re.escape(variable)}\b",
                     compact[success_pos + len("Status = 0;"):]), (
        f"{path}: {marker} no longer consumes the validated {variable}"
    )



def verify_deauth_copy_fail_closed(path: Path, source: str) -> None:
    case = extract_case(
        source, "case OID_802_11_DEAUTHENTICATION:", "case RT_SET_IAPP_PID:"
    )
    compact = " ".join(case.split())
    expected_size = (
        "sizeof(MLME_DISCONNECT_STRUCT)"
        if path == SOURCES[3] else "sizeof(MLME_DEAUTH_REQ_STRUCT)"
    )
    assert f"if (wrq->u.data.length != {expected_size})" in compact, (
        f"{path}: deauth input must have the exact structure size"
    )
    copy = "if (copy_from_user(pInfo, wrq->u.data.pointer, wrq->u.data.length))"
    expected_copies = 2 if path in (SOURCES[0], SOURCES[3]) else 1
    assert compact.count(copy) == expected_copies, (
        f"{path}: every deauth usercopy must fail closed"
    )
    assert "Status = copy_from_user(pInfo" not in compact
    for offset in (i for i in range(len(compact)) if compact.startswith(copy, i)):
        error = compact.index("Status = -EFAULT;", offset)
        stop = compact.index("break;", error)
        failure = compact[offset:stop]
        assert "os_free_mem" in failure, (
            f"{path}: failed deauth copy must release its allocated buffer"
        )
        assert not any(token in failure for token in (
            "DeAuthFrame.Reason =", "MacTableLookup(", "MlmeEnqueue("
        )), f"{path}: failed input must not trigger deauth state changes"
        success = compact.index("Status = 0;", stop)
        assert success > stop, (
            f"{path}: successful status must follow the checked copy"
        )
    if path in (SOURCES[0], SOURCES[3]):
        first_copy = compact.index(copy)
        allocation_guard = compact.index("if (pInfo == NULL)")
        allocation_error = compact.index("Status = -ENOMEM;", allocation_guard)
        assert allocation_guard < allocation_error < first_copy, (
            f"{path}: APCLI deauth allocation must be checked before usercopy"
        )



def verify_security_input_copy_fail_closed(path: Path, source: str) -> None:
    for marker, variable, expected_type in (
        ("case OID_802_11_AUTHENTICATION_MODE:", "AuthMode",
         "NDIS_802_11_AUTHENTICATION_MODE"),
        ("case OID_802_11_WEP_STATUS:", "WepStatus",
         "NDIS_802_11_WEP_STATUS"),
        ("case OID_802_11_SSID:", "Ssid", "NDIS_802_11_SSID"),
    ):
        verify_checked_usercopy_case(
            path, source, marker, variable, expected_type
        )


def verify_apcli_boolean_copy_fail_closed(path: Path, source: str) -> None:
    for marker, variable in (
        ("case OID_802_11_SET_IEEE8021X:", "IEEE8021xState"),
        ("case OID_802_11_SET_IEEE8021X_REQUIRE_KEY:",
         "IEEE8021x_required_keys"),
    ):
        verify_checked_usercopy_case(
            path, source, marker, variable, "BOOLEAN"
        )



def verify_psk_length_contract(path: Path, source: str) -> None:
    case = extract_case(source, "case OID_SET_PSK:", "#ifdef SNMP_SUPPORT")
    compact = " ".join(case.split())
    guard = "if (!mtk_wpa_psk_length_valid(wrq->u.data.length, sizeof(PSK) - 1))"
    copy = "if (copy_from_user(PSK, wrq->u.data.pointer, wrq->u.data.length))"
    terminator = r"PSK[wrq->u.data.length] = '\0';"
    assert '#include "../../mtk_wpa_psk_policy.h"' in source
    assert guard in compact, f"{path}: PSK input must be capped at 64 bytes"
    assert copy in compact, f"{path}: copy PSK once into bounded staging"
    copy_pos = compact.index(copy)
    error_pos = compact.index("Status = -EFAULT;", copy_pos)
    error_break = compact.index("break;", error_pos)
    terminator_pos = compact.index(terminator)
    assert copy_pos < error_pos < error_break < terminator_pos
    assert compact.count("copy_from_user(") == 1
    assert "NdisMoveMemory(pAd->ApCfg.ApCliTab[pObj->ioctl_if].PSK," in compact
    assert "NdisMoveMemory(pAd->ApCfg.MBSSID[pObj->ioctl_if].WPAKeyString," in compact
    assert "NdisMoveMemory(pWscControl->WpaPsk, PSK," in compact
    assert "PSK length = %d" in compact
    assert "PSK = %s" not in compact, f"{path}: do not log plaintext PSKs"
    assert "WSC PSK length = %d" in compact


def verify_extended_psk_copy(path: Path, source: str) -> None:
    assert '#include "../../../mtk_wpa_psk_policy.h"' in source
    case = extract_case(source, "case OID_SET_PSK:", "#ifdef SNMP_SUPPORT")
    compact = " ".join(case.split())
    guard = "if (!mtk_wpa_psk_length_valid(wrq->u.data.length, LEN_PSK))"
    temporary = "UCHAR psk[LEN_PSK + 1] = {0};"
    copy = "if (copy_from_user(psk, wrq->u.data.pointer, wrq->u.data.length))"
    terminator = r"psk[wrq->u.data.length] = '\0';"
    assert guard in compact, f"{path}: cap PSK to LEN_PSK"
    assert temporary in compact, f"{path}: stage PSK in LEN_PSK+1 bytes"
    assert copy in compact, f"{path}: copy only into staging buffer"
    copy_pos = compact.index(copy)
    error_pos = compact.index("Status = -EFAULT;", copy_pos)
    error_break = compact.index("break;", error_pos)
    terminator_pos = compact.index(terminator)
    psec_clear = compact.index("NdisZeroMemory(pSecConfig->PSK, sizeof(pSecConfig->PSK));")
    psec_publish = compact.index("NdisMoveMemory(pSecConfig->PSK, psk, wrq->u.data.length + 1);")
    wsc_clear = compact.index("NdisZeroMemory(pWscControl->WpaPsk, sizeof(pWscControl->WpaPsk));")
    wsc_publish = compact.index("NdisMoveMemory(pWscControl->WpaPsk, psk, wrq->u.data.length);")
    wsc_len = compact.index("pWscControl->WpaPskLen = wrq->u.data.length;")
    assert compact.index(guard) < copy_pos < error_pos < error_break < terminator_pos
    assert terminator_pos < psec_clear < psec_publish < wsc_clear < wsc_publish < wsc_len
    assert compact.count("copy_from_user(") == 1, (
        f"{path}: do not usercopy independently into WSC storage"
    )
    assert "PSK length = %d" in compact
    assert "WSC PSK length = %d" in compact
    assert "PSK = %s" not in compact, f"{path}: do not log plaintext PSKs"
    assert "copy_from_user(pWscControl->WpaPsk" not in compact


def verify_wpa_psk_policy() -> None:
    header = ROOT / "trunk/linux-4.4.x/drivers/net/wireless/mediatek/mtk_wpa_psk_policy.h"
    test = ROOT / "trunk/linux-4.4.x/drivers/net/wireless/mediatek/tests/wpa_psk_policy.c"
    policy = header.read_text(encoding="utf-8")
    regression = test.read_text(encoding="utf-8")
    assert "return maximum != 0U && length <= maximum;" in policy
    for path in SOURCES[2:]:
        include_dir = path.parents[1] / "include"
        wpa = b" ".join((include_dir / "security/dot11i_wpa.h").read_bytes().split())
        sec = b" ".join((include_dir / "security/sec_cmm.h").read_bytes().split())
        wsc = b" ".join((include_dir / "wsc.h").read_bytes().split())
        assert b"#define LEN_PSK 64" in wpa
        assert b"PSK[LEN_PSK + 1]" in sec
        assert b"WpaPsk[LEN_PSK]" in wsc
    for marker in ("length_valid(0U, 64U)", "length_valid(64U, 64U)",
                   "length_valid(65U, 64U)", "length_valid(UINT_MAX, 64U)"):
        assert marker in regression, f"missing PSK boundary: {marker}"

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
    guard = "if (!mtk_ft_r0khid_input_length_valid(wrq->u.data.length, FT_ROKH_ID_LEN))"
    temporary = "UCHAR r0kh_id[FT_ROKH_ID_LEN + 1] = {0};"
    copy = "if (copy_from_user(r0kh_id, wrq->u.data.pointer, wrq->u.data.length))"
    normalize = (
        "else if (mtk_ft_r0khid_normalize_length( r0kh_id, wrq->u.data.length, "
        "FT_ROKH_ID_LEN, &r0kh_id_len))"
    )
    assert guard in compact, f"{path}: reject lengths beyond payload plus optional NUL"
    assert temporary in compact, f"{path}: stage into a zeroed 49-byte buffer"
    assert copy in compact, f"{path}: copy only into the bounded temporary"
    assert normalize in compact, f"{path}: normalize optional trailing NUL after copy"
    guard_pos = compact.index(guard)
    temporary_pos = compact.index(temporary)
    copy_pos = compact.index(copy)
    copy_error_pos = compact.index("Status = -EFAULT;", copy_pos)
    normalize_pos = compact.index(normalize, copy_error_pos)
    normalize_error_pos = compact.index("Status = -EINVAL;", normalize_pos)
    success_pos = compact.index("else {", normalize_error_pos)
    prefix = "pAd->ApCfg.MBSSID[apidx]"
    if ".wdev.FtCfg." in compact:
        prefix += ".wdev"
    destination = prefix + ".FtCfg.FtR0khId"
    destination_len = prefix + ".FtCfg.FtR0khIdLen"
    terminate = "r0kh_id[r0kh_id_len] = '\\0';"
    publish = f"memcpy({destination}, r0kh_id, sizeof(r0kh_id));"
    set_length = f"{destination_len} = r0kh_id_len;"
    terminate_pos = compact.index(terminate, success_pos)
    publish_pos = compact.index(publish, terminate_pos)
    length_pos = compact.index(set_length, publish_pos)
    assert guard_pos < temporary_pos < copy_pos < copy_error_pos < normalize_pos
    assert normalize_pos < normalize_error_pos < success_pos < terminate_pos < publish_pos < length_pos
    assert "Status = 0;" in compact[success_pos:terminate_pos]
    assert f"copy_from_user({destination}," not in compact
    assert "wrq->u.data.length <= FT_ROKH_ID_LEN" not in compact


def verify_ft_r0khid_policy() -> None:
    header = ROOT / "trunk/linux-4.4.x/drivers/net/wireless/mediatek/mtk_ft_r0khid_policy.h"
    test = ROOT / "trunk/linux-4.4.x/drivers/net/wireless/mediatek/tests/ft_r0khid_policy.c"
    policy = header.read_text(encoding="utf-8")
    regression = test.read_text(encoding="utf-8")
    assert "length - 1U <= maximum" in policy
    assert "value[length - 1U] == '\\0'" in policy
    for path in SOURCES:
        include_dir = path.parents[1] / "include"
        macro = (include_dir / "dot11r_ft.h").read_bytes()
        structure = (include_dir / "ft_cmm.h").read_bytes()
        macro = b" ".join(macro.split())
        structure = b" ".join(structure.split())
        assert b"#define FT_ROKH_ID_LEN 48" in macro
        assert b"FtR0khId[FT_ROKH_ID_LEN + 1]" in structure
    for marker in ("input_length_valid(0U, 48U)", "input_length_valid(49U, 48U)",
                   "input_length_valid(50U, 48U)", "normalize_length(value, 49U, 48U, &length)",
                   "value[48] = '\\0';", "value[48] = 'x';"):
        assert marker in regression, f"missing R0KH-ID boundary/termination case: {marker}"


def main() -> None:
    verify_ft_r0khid_policy()
    verify_wpa_psk_policy()
    for path in SOURCES:
        source = path.read_text(encoding="utf-8")
        if path in SOURCES[:2]:
            for case in CASES:
                verify_fixed_copy(path, source, case)
        verify_ft_boolean_copy(path, source)
        verify_security_input_copy_fail_closed(path, source)
        verify_deauth_copy_fail_closed(path, source)
        if path in (SOURCES[0], SOURCES[3]):
            verify_apcli_boolean_copy_fail_closed(path, source)
        verify_ft_r0khid_copy(path, source)
        if path == SOURCES[1]:
            verify_psk_length_contract(path, source)
        elif path in SOURCES[2:]:
            verify_extended_psk_copy(path, source)
        print(f"MTK AP ioctl copy contracts: PASS ({path.relative_to(ROOT)})")


if __name__ == "__main__":
    main()
