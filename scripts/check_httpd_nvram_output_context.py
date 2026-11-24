#!/usr/bin/env python3
"""Reject HTML-only NVRAM output in JavaScript execution contexts."""

from pathlib import Path
import re
import sys

ROOT = Path("trunk/user/www/n56u_ribbon_fixed")
WEB_SOURCE = Path("trunk/user/httpd/src/web_ex.c")
HTML_POLICY = Path("trunk/user/httpd/src/httpd_html_output_policy.h")
JSON_POLICY = Path("trunk/user/httpd/src/httpd_json_output_policy.h")
AIDISK_SOURCE = Path("trunk/user/httpd/src/aidisk.c")
DIRECT_POLICY = Path("trunk/user/httpd/src/httpd_direct_ej_policy.h")
RAW_JS_ENDPOINTS = {"lan_clients.asp", "ajax_ddnscode.asp"}
CALL = re.compile(r"<%\s*(nvram_get_list_x|nvram_get_buf_x|nvram_get_table_x|get_nvram_list|nvram_get_js|nvram_get_ddns|nvram_get_x|nvram_get)\s*\(")


def js_state_before(line: str, stop: int):
    """Return None for code, 'comment' for a comment, or the open JS quote."""
    quote = None
    escaped = False
    index = 0
    while index < stop:
        if line.startswith("<%", index):
            close = line.find("%>", index + 2)
            if close >= 0 and close < stop:
                index = close + 2
                continue
        char = line[index]
        if quote:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == quote:
                quote = None
        else:
            if line.startswith("//", index):
                return "comment"
            if line.startswith("/*", index):
                close = line.find("*/", index + 2)
                if close < 0 or close >= stop:
                    return "comment"
                index = close + 2
                continue
            if char in ("'", '"', chr(96)):
                quote = char
        index += 1
    return quote


def check_templates():
    js_calls = 0
    ddns_calls = 0
    json_calls = 0
    violations = []
    candidates = sorted(
        path for path in ROOT.rglob("*")
        if path.is_file() and path.suffix.lower() in {".asp", ".js", ".htm", ".html"}
    )
    for path in candidates:
        in_script = path.suffix.lower() == ".js" or path.name in RAW_JS_ENDPOINTS
        for line_number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
            lowered = line.lower()
            if "<script" in lowered:
                in_script = True
            if in_script:
                for match in CALL.finditer(line):
                    state = js_state_before(line, match.start())
                    if state == "comment":
                        continue
                    callback = match.group(1)
                    if callback in {
                        "nvram_get_x",
                        "nvram_get",
                        "nvram_get_list_x",
                        "nvram_get_buf_x",
                        "nvram_get_table_x",
                    }:
                        violations.append(
                            f"{path}:{line_number}: unsafe {callback} emitter used in JavaScript"
                        )
                    elif state is not None:
                        violations.append(
                            f"{path}:{line_number}: {callback} is wrapped by a JavaScript string"
                        )
                    elif callback == "nvram_get_js":
                        js_calls += 1
                    elif callback == "nvram_get_ddns":
                        ddns_calls += 1
                    elif callback == "get_nvram_list":
                        json_calls += 1
            if "</script" in lowered:
                in_script = False

    if js_calls < 115:
        violations.append(
            f"only {js_calls} safe nvram_get_js calls found; expected at least 115 migrated sinks"
        )
    if ddns_calls != 3:
        violations.append(
            f"expected 3 unquoted nvram_get_ddns JavaScript calls, found {ddns_calls}"
        )
    if json_calls < 10:
        violations.append(
            f"only {json_calls} JSON-safe get_nvram_list calls found; expected at least 10"
        )
    clients = (ROOT / "device-map" / "clients.asp").read_text(encoding="utf-8")
    for variable, key in (
        ("arpon", "dhcp_static_arp"),
        ("mdhcp", "dhcp_static_x"),
    ):
        if f"var {variable} = Number(<% nvram_get_js" not in clients or key not in clients:
            violations.append(
                f"Device Map numeric variable {variable} no longer preserves number semantics"
            )
    if violations:
        raise AssertionError("\n".join(violations))
    print(
        f"JavaScript NVRAM contexts: PASS ({js_calls} nvram_get_js, "
        f"{ddns_calls} nvram_get_ddns, {json_calls} get_nvram_list; "
        f"numeric Device Map types retained)"
    )


def check_handler_wiring():
    source = WEB_SOURCE.read_text(encoding="utf-8")
    html_policy = HTML_POLICY.read_text(encoding="utf-8")
    json_policy = JSON_POLICY.read_text(encoding="utf-8")
    direct_policy = DIRECT_POLICY.read_text(encoding="utf-8")

    generic_start = source.index("static int\nej_nvram_get_x(")
    js_start = source.index("static int\nej_nvram_get_js(", generic_start)
    ddns_start = source.index("static int\nej_nvram_get_ddns(", js_start)
    match_start = source.index("static int\nej_nvram_match_x(", ddns_start)
    generic = source[generic_start:js_start]
    ddns = source[ddns_start:match_start]
    list_start = source.index("static int ej_get_nvram_list(")
    list_end = source.index("\nstatic void\nwrite_json_string(", list_start)
    json_list = source[list_start:list_end]
    writer_start = source.index("write_json_string(webs_t wp, const char *value)")
    writer_end = source.index("\nstatic int ej_get_static_client", writer_start)
    json_writer = source[writer_start:writer_end]

    assert "httpd_html_escape_byte(*c, escaped)" in generic
    assert "fwrite(escaped, 1, escaped_len, wp)" in generic
    assert "ej_nvram_get_js(eid, wp, argc, argv)" in ddns
    assert '{ "nvram_get_js", ej_nvram_get_js}' in source
    assert '{ "nvram_get_js", HTTPD_DIRECT_EJ_ESCAPED_OUTPUT }' in direct_policy
    assert 'entity = "&#39;"' in html_policy
    assert '#include "httpd_json_output_policy.h"' in source
    assert "write_json_string(wp, nvram_get_list_x(gv->name, i));" in json_list
    assert "httpd_json_escape_next" in json_writer and "fwrite(" in json_writer
    assert "httpd_json_escape_next" in json_policy
    print("HTML, JSON and JavaScript NVRAM output-context wiring: PASS")


def check_dynamic_js_hooks():
    source = WEB_SOURCE.read_text(encoding="utf-8")

    wan_start = source.index("static int\nwanlink_hook(")
    lan_start = source.index("\nstatic int\nlanlink_hook(", wan_start)
    wan = source[wan_start:lan_start]
    for value in (
        "etherlink", "apclilink", "wan_desc", "wan0_ip", "wan0_gw",
        "wanx_ip", "wanx_gw", "wan_ip6", "lan_ip6", "wan_dns", "wan_mac",
    ):
        assert f"write_js_string(wp, {value});" in wan, (
            f"wanlink_hook emits {value} without JavaScript string escaping"
        )
    assert "return '%s'" not in wan

    lan_end = source.index("\nstatic int\nwan_action_hook(", lan_start)
    lan = source[lan_start:lan_end]
    assert "write_js_string(wp, port_status);" in lan
    assert "return '%s'" not in source
    assert source.count("write_js_string(wp, action_mode);") == 2

    wol_start = source.index("static int\nwol_action_hook(")
    nf_start = source.index("\nstatic int\nnf_values_hook(", wol_start)
    wol = source[wol_start:nf_start]
    assert 'websWrite(wp, "{\\\"wol_mac\\\": \\\"%s\\\"}", wol_mac);' not in wol
    assert 'websWrite(wp, "{\\\"wol_mac\\\": ");' in wol
    assert "write_json_string(wp, wol_mac);" in wol
    assert 'websWrite(wp, "}");' in wol
    assert "isxdigit(*p)" in wol and "dst_mac[i] != ':'" in wol

    for value in ("ver", "nf_count"):
        assert f"write_js_string(wp, {value});" in source, (
            f"dynamic status string {value} is not JavaScript escaped"
        )
    assert source.count("write_js_string(wp, count);") >= 2, (
        "route-list status strings must use the JavaScript string writer"
    )
    print("Dynamic HTTPD JavaScript hook strings: PASS")


def check_wol_dom_sinks():
    """Keep client and third-party labels in text-only DOM sinks."""
    source = (ROOT / "Advanced_WOL_Content.asp").read_text(encoding="utf-8")

    assert ".find('td.vendor').text(vendorName);" in source
    assert ".find('td.vendor').text(String(company));" in source
    assert "$j('<td>').addClass('mac').text(mac).appendTo($row);" in source
    assert "$j('<td>').text(name).appendTo($row);" in source
    assert "$j('<td>').addClass('vendor').appendTo($row);" in source
    assert "t_body += " not in source, "client data must not be concatenated into HTML"
    assert ".html(vendorObj.company)" not in source
    assert ".html(company)" not in source
    print("WOL client/vendor DOM text sinks: PASS")


def check_aidisk_script_messages():
    source = AIDISK_SOURCE.read_text(encoding="utf-8")
    assert '#include "httpd_json_output_policy.h"' in source
    assert "httpd_json_escape_next" in source
    helper_start = source.index("static void\nwrite_script_message(")
    helper_end = source.index("\nstatic void\nwrite_tree_entry(", helper_start)
    helper = source[helper_start:helper_end]
    assert r'websWrite(wp, "<script>\n%s(", callback);' in helper
    assert "write_json_string(wp, message);" in helper
    assert r'websWrite(wp, ");\n</script>\n");' in helper
    message_lines = [line for line in source.splitlines() if "get_alert_msg_from_dict(" in line]
    assert len(message_lines) == 48, (
        f"expected 48 AiDisk error messages, found {len(message_lines)}"
    )
    assert all("write_script_message(wp," in line for line in message_lines)
    assert all("websWrite(wp," not in line for line in message_lines)
    assert source.count('write_script_message(wp, "safely_remove_disk_error"') == 2
    print("AiDisk JavaScript error-message encoding: PASS")

def check_aidisk_usb_string_outputs():
    source = AIDISK_SOURCE.read_text(encoding="utf-8")
    start = source.index("int\\nej_get_usb_ports_info(")
    end = source.index("\\n#endif", start)
    usb = source[start:end]
    for label in ("hub", "printer", "modem_eth", "modem_tty", "storage", "unknown"):
        assert f'= "{label}";' in usb
    assert "write_json_string(wp, usb_dev_string[0]);" in usb
    assert "write_json_string(wp, usb_dev_string[1]);" in usb
    assert 'write_json_string(wp, "unknown");' in usb
    assert 'write_json_string(wp, (follow_usb->dev_type == DEVICE_TYPE_MODEM_TTY) ? "RAS" : "NDIS/RNDIS");' in usb
    print("AiDisk USB enum JavaScript string encoding: PASS")

def main():
    check_handler_wiring()
    check_dynamic_js_hooks()
    check_templates()
    check_wol_dom_sinks()
    check_aidisk_script_messages()
    check_aidisk_usb_string_outputs()


if __name__ == "__main__":
    try:
        main()
    except (AssertionError, OSError, ValueError) as error:
        print(f"HTTPD NVRAM output-context check: FAIL: {error}", file=sys.stderr)
        sys.exit(1)
