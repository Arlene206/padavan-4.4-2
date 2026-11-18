## 2026-11-18 — Restore mandatory handoff checkpoint clause after exact-head docs failure

- Exact main SHA \`ca5f182341dde1bc2323774c1b284c20171e719a\` had AI handoff documentation run \`44328925370\` fail because \`SESSION_HANDOFF.md\` no longer contained the mandatory "Mandatory handoff checkpoint" heading and exact checkpoint-accuracy sentence. The checker correctly rejected this omission.
- Restore the canonical checkpoint section in the live handoff; do not weaken \`scripts/check_ai_handoff_docs.py\`. Preserve the detect-internet active task, exact source SHA, pending Actions and hardware-validation limits.
- Exact \`ca5f182\` detect-internet \`44328925527\`, WAN route \`44328925623\`, Process regression \`44328925872\`, NVRAM \`44328925729\` and XY-C1 \`44328925613\` were pending at last observation; WAN DHCP, iNIC, watchdog, Wireless, FRP and PID 1 notification runs passed.
- This is a documentation-contract correction only. No local checkout test was run; exact-head AI docs rerun is required. Product-source L3 and router hardware L4 remain unverified.

---

## 2026-11-18 — Make manual radio/guest watchdog settling signal-safe

- Parent main: `0b8acba5190a4765f643090a852cbc53d216b97d`. In four manual WLAN enable paths, `notify_watchdog_wifi()` was followed by raw `usleep(300000)` before changing NVRAM and invoking radio/guest control. Signal interruption could shorten this serialization window and race the watchdog's Wi-Fi notification handling.
- Add `wait_wifi_watchdog_update()` using `service_wait_microseconds(300000U)`; preserve errno/EIO and log. If the settle wait fails, each manual path returns before its NVRAM mutation/control call, instead of proceeding in a potentially racy state. No behavior change for disable paths or when no state change is required.
- Extend Wireless source regression to check helper hard-error behavior and all four notify → wait → fail-return → NVRAM → control orderings; raw 300ms usleep is rejected. Existing shared-wait unit tests cover EINTR retry and non-EINTR failure.
- Predecessor `0b8acba` exact Wireless `44328293451`, iNIC `44328293059`, watchdog `44328291899` and Process inventory `44328294449` PASS; Process fail-closed, XY-C1, NVRAM and HTTPD were pending at last poll. Exact-head reruns required. No local or hardware test claimed.

---
