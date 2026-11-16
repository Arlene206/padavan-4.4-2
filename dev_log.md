## 2026-11-16 — Propagate USB modem reconnect failures and preserve hotplug retry events

- Parent main: ea794e3ef680da5b03ca0ffdd621601c97dc7d86. Review found try_start_usb_modem_to_wan() discarded try_wan_reconnect() status, while on_deferred_hotplug_dev() cleared usb_unplug_md/usb_hotplug_md before reconnect and never re-armed them after failure.
- Converted the helper/prototype to int. Existing no-op eligibility paths still return 0; attempted reconnect returns the actual try_wan_reconnect() status. Deferred modem unplug and plug flows now log failure, restore usb_unplug_md or usb_hotplug_md respectively, and schedule the existing five-second alarm retry.
- Extended usb-modem-identity-stability.yml with helper-to-caller status/retry assertions and added services_stor.c/services_usb.c to both push and pull_request filters; rc.h is included in the checker path contract.
- Exact-parent storage 43882865182 passed all 24 steps, including final safe-remove/persistence checker. AI docs 43882864787 failed only because SESSION_HANDOFF.md lacked the required "Mandatory handoff checkpoint" and conditional-edit statement; this handoff restores it. AdGuardHome, infosvr, AliyunDrive, process inventory, NVRAM and AliDDNS passed.
- Firmware L3 remains verified on product-source SHA cd5ccbefc26a9f71b0c971800b2f5b0d9cad450d (Build/TRX/artifact same-SHA PASS). This modem change must earn a new same-SHA Build/TRX/artifact result; physical-router L4 remains NOT VERIFIED.
- Next: inspect USB modem identity, XY-C1 and AI docs exact-SHA Actions, then continue P1 cross-userland startup-result propagation.

---

