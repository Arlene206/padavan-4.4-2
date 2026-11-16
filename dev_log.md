## 2026-11-16 — Complete strict module-state migration for wireless loads

- Parent: `d5d07a7027457ecd56ec217d1f4191ab8227d7ea`; tri-state policy foundation: `bca48425f16b97cdb2b599401bd105fbc5e652ad`.
- Exact-head bca Firewall/WAN checks identified the final legacy Boolean callback in `rc.c:load_wireless_module()`. The shared verifier now preserves negative probe errno; NAT, boot hardware and LAN bridge had already migrated. This correction switches the remaining wireless-driver load to the strict status probe.
- The bca AI handoff contract also rejected an omitted exact "Before context exhaustion" phrase; the mandatory checkpoint wording is restored.
- Fresh exact-head WAN, Firewall, Process, HTTPD, AI handoff and XY-C1 checks are required for this correction SHA. Do not infer target firmware/TRX or physical-router results from earlier commits; physical-router L4 remains NOT VERIFIED.

---

