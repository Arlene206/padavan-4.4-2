#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(".")
BASE = ROOT / "trunk/user/miniupnpd/miniupnpd-2.x"
events = (BASE / "upnpevents.c").read_text()
http = (BASE / "upnphttp.c").read_text()
policy = (BASE / "miniupnpd_event_subscription_policy.h").read_text()
test = (BASE / "tests/miniupnpd_event_subscription_policy_test.c").read_text()
workflow = (ROOT / ".github/workflows/miniupnpd-event-subscriber-stability.yml").read_text()
linux_makefile = (BASE / "Makefile.linux").read_text()
nft_makefile = (BASE / "Makefile.linux_nft").read_text()


def body(text, signature):
    start = text.index(signature)
    brace = text.index("{", start)
    depth = 0
    for index in range(brace, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[brace + 1:index]
    raise AssertionError("unterminated function: " + signature)


add = body(events, "upnpevents_addSubscriber(")
subscribe_start = http.index("ProcessHTTPSubscribe_upnphttp(struct upnphttp * h, const char * path)")
subscribe_end = http.index("\\nstatic void\\nProcessHTTPUnSubscribe_upnphttp", subscribe_start)
subscribe = http[subscribe_start:subscribe_end]

assert '#include "miniupnpd_event_subscription_policy.h"' in events
assert "MINIUPNPD_EVENT_MAX_SUBSCRIBERS 32U" in policy
assert "return current_count < MINIUPNPD_EVENT_MAX_SUBSCRIBERS;" in policy
assert "size_t subscriber_count = 0;" in add
assert "for (current = subscriberlist.lh_first" in add
assert "miniupnpd_event_subscriber_has_capacity(subscriber_count)" in add
assert add.index("miniupnpd_event_subscriber_has_capacity") < add.index("newSubscriber(")
assert "maximum event subscribers" in add
assert "if(sid)" in subscribe
assert "BuildResp_upnphttp(h, 0, 0);" in subscribe
assert "BuildResp2_upnphttp(h, 503, \"Service Unavailable\", NULL, 0);" in subscribe
assert subscribe.index("if(sid)") < subscribe.index("BuildResp2_upnphttp(h, 503")
assert "FLAG_TIMEOUT | FLAG_SID" in subscribe
assert "h->respflags = 0;" in subscribe
for makefile in (linux_makefile, nft_makefile):
    assert "upnpevents.o: upnppermissions.h miniupnpdtypes.h upnpdescgen.h upnputils.h miniupnpd_event_subscription_policy.h" in makefile

for token in (
    "test_subscriber_capacity_boundaries",
    "MINIUPNPD_EVENT_MAX_SUBSCRIBERS - 1U",
    "MINIUPNPD_EVENT_MAX_SUBSCRIBERS + 1U",
):
    assert token in test

paths = (
    ".github/workflows/miniupnpd-event-subscriber-stability.yml",
    "trunk/user/miniupnpd/miniupnpd-2.x/upnpevents.c",
    "trunk/user/miniupnpd/miniupnpd-2.x/upnphttp.c",
    "trunk/user/miniupnpd/miniupnpd-2.x/miniupnpd_event_subscription_policy.h",
    "trunk/user/miniupnpd/miniupnpd-2.x/tests/miniupnpd_event_subscription_policy_test.c",
    "trunk/user/miniupnpd/miniupnpd-2.x/Makefile.linux",
    "trunk/user/miniupnpd/miniupnpd-2.x/Makefile.linux_nft",
    "scripts/check_miniupnpd_event_subscription.py",
)
push, pr = workflow.split("  pull_request:", 1)[0], workflow.split("  pull_request:", 1)[1].split("permissions:", 1)[0]
for path in paths:
    quoted_path = '"' + path + '"'
    assert push.count(quoted_path) == 1, ("push path missing/duplicated", path)
    assert pr.count(quoted_path) == 1, ("pull_request path missing/duplicated", path)

print("MiniUPnP event subscription resource-cap source contract: PASS")
