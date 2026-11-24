#!/usr/bin/env python3
"""Reject HTML-only NVRAM output in JavaScript execution contexts."""

from pathlib import Path
import re
import sys

ROOT = Path("trunk/user/www/n56u_ribbon_fixed")
WEB_SOURCE = Path("trunk/user/httpd/src/web_ex.c")
HTML_POLICY = Path("trunk/user/httpd/src/httpd_html_output_policy.h")
DIRECT_POLICY = Path("trunk/user/httpd/src/httpd_direct_ej_policy.h")
RAW_JS_ENDPOINTS = {"lan_clients.asp", "ajax_ddnscode.asp"}
CALL = re.compile(r"<%\s*(nvram_get_x|nvram_get_js|nvram_get_ddns)\s*\(")


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
                    if callback == "nvram_get_x":
                        violations.append(
                            f"{path}:{line_number}: HTML-only nvram_get_x used in JavaScript"
                        )
                    elif state is not None:
                        violations.append(
                            f"{path}:{line_number}: {callback} is wrapped by a JavaScript string"
                        )
                    elif callback == "nvram_get_js":
                        js_calls += 1
                    elif callback == "nvram_get_ddns":
                        ddns_calls += 1
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
        f"{ddns_calls} nvram_get_ddns; numeric Device Map types retained)"
    )


def check_handler_wiring():
    source = WEB_SOURCE.read_text(encoding="utf-8")
    html_policy = HTML_POLICY.read_text(encoding="utf-8")
    direct_policy = DIRECT_POLICY.read_text(encoding="utf-8")

    generic_start = source.index("static int\nej_nvram_get_x(")
    js_start = source.index("static int\nej_nvram_get_js(", generic_start)
    ddns_start = source.index("static int\nej_nvram_get_ddns(", js_start)
    match_start = source.index("static int\nej_nvram_match_x(", ddns_start)
    generic = source[generic_start:js_start]
    ddns = source[ddns_start:match_start]

    assert "httpd_html_escape_byte(*c, escaped)" in generic
    assert "fwrite(escaped, 1, escaped_len, wp)" in generic
    assert "ej_nvram_get_js(eid, wp, argc, argv)" in ddns
    assert '{ "nvram_get_js", ej_nvram_get_js}' in source
    assert '{ "nvram_get_js", HTTPD_DIRECT_EJ_ESCAPED_OUTPUT }' in direct_policy
    assert 'entity = "&#39;"' in html_policy
    print("HTML apostrophe escaping and JavaScript-safe DDNS handler wiring: PASS")


def main():
    check_handler_wiring()
    check_templates()


if __name__ == "__main__":
    try:
        main()
    except (AssertionError, OSError, ValueError) as error:
        print(f"HTTPD NVRAM output-context check: FAIL: {error}", file=sys.stderr)
        sys.exit(1)
