#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(".")
BASE = ROOT / "trunk/user/miniupnpd/miniupnpd-2.x"
HTTP = (BASE / "upnphttp.c").read_text()
EVENTS = (BASE / "upnpevents.c").read_text()
POLICY = (BASE / "miniupnpd_event_callback_policy.h").read_text()
TEST = (BASE / "tests/miniupnpd_event_callback_policy_test.c").read_text()
WORKFLOW = (ROOT / ".github/workflows/miniupnpd-event-callback-stability.yml").read_text()
MAKEFILES = [(BASE / "Makefile.linux").read_text(),
             (BASE / "Makefile.linux_nft").read_text()]


def region(source, start, end):
    first = source.index(start)
    last = source.index(end, first + len(start))
    return source[first:last]


check_callback = region(
    HTTP, "static int\ncheckCallbackURL(struct upnphttp * h)", "\nstatic ")
notify_connect = region(
    EVENTS, "static void\nupnp_event_notify_connect(struct upnp_event_notify * obj)",
    "\nstatic ")
prepare = EVENTS[EVENTS.index("upnp_event_prepare("):]
prepare = prepare[:prepare.index("\nstatic ")] if "\nstatic " in prepare else prepare

assert '#include "miniupnpd_event_callback_policy.h"' in HTTP
assert '#include "miniupnpd_event_callback_policy.h"' in EVENTS
assert "miniupnpd_event_parse_callback_url(open + 1" in check_callback
assert "close = memchr(open + 1, '>'" in check_callback
assert "h->req_CallbackOff = (int)(open + 1 - h->req_buf)" in check_callback
assert "miniupnpd_event_parse_callback_url(callback, callback_len" in notify_connect
assert "obj->path = callback + parsed.path_offset;" in notify_connect
assert "while(*p != '/')" not in notify_connect
assert "atoi(p)" not in notify_connect
assert "parsed.port" in notify_connect
assert "inet_pton(AF_INET6, parsed.host" in notify_connect
assert "inet_pton(AF_INET, parsed.host" in notify_connect
assert "url[cursor] != '/')" in POLICY
assert "port > (65535U - digit) / 10U" in POLICY
assert "miniupnpd_event_buffer_length_fits(written" in prepare
assert "obj->tosend = (unsigned int)written;" in prepare
assert prepare.index("miniupnpd_event_buffer_length_fits") < prepare.index("obj->tosend = (unsigned int)written;")
assert "return written >= 0 && (size_t)written < capacity;" in POLICY

for token in (
    "http://192.0.2.10",
    "http://192.0.2.10:65536/event",
    "test_ipv6_bracket_and_port_boundaries",
    "test_notification_length_boundaries",
):
    assert token in TEST

for makefile in MAKEFILES:
    assert "miniupnpd_event_callback_policy.h" in makefile

paths = (
    ".github/workflows/miniupnpd-event-callback-stability.yml",
    "trunk/user/miniupnpd/miniupnpd-2.x/upnphttp.c",
    "trunk/user/miniupnpd/miniupnpd-2.x/upnpevents.c",
    "trunk/user/miniupnpd/miniupnpd-2.x/miniupnpd_event_callback_policy.h",
    "trunk/user/miniupnpd/miniupnpd-2.x/tests/miniupnpd_event_callback_policy_test.c",
    "trunk/user/miniupnpd/miniupnpd-2.x/Makefile.linux",
    "trunk/user/miniupnpd/miniupnpd-2.x/Makefile.linux_nft",
    "scripts/check_miniupnpd_event_callback.py",
)
push = WORKFLOW.split("  pull_request:", 1)[0]
pr = WORKFLOW.split("  pull_request:", 1)[1].split("permissions:", 1)[0]
for path in paths:
    quoted = '"' + path + '"'
    assert push.count(quoted) == 1, ("push path missing/duplicated", path)
    assert pr.count(quoted) == 1, ("pull_request path missing/duplicated", path)

print("MiniUPnP event callback URL and notification buffer bounds: PASS")
