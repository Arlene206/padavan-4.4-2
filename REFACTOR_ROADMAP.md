# 2026-11-17 latest source checkpoint

- Parent main SHA: `0a90c01469edfa5b63129cfeac042ddc94d616fe`; THIS COMMIT adds `msd_lite` to WAN teardown and extends its source contract. Frozen backup `72f4f157a084f8184b9597ca5bf334b8080e5807` remains unchanged.
- Parent HTTPD security `44156398138` and AI handoff `44156398311` passed. Parent XY-C1 `44156397861`, Process `44156397929`, and NVRAM `44156397925` were still in progress at last poll.
- SD/MMC hotplug tri-state handling is covered by storage and the repaired HTTPD workflow. The XY-C1 path filter did not include `hotplug_stor.c`; THIS COMMIT adds it so future hotplug changes trigger the firmware workflow.
- WAN teardown omitted `msd_lite` despite its optional launch path and presence in the shared proxy-stop list; THIS COMMIT includes it and tests the complete proxy inventory. Exact-head checks pending.

Next: verify exact-SHA WAN, XY-C1 Build/TRX/artifact, Process, AI and NVRAM; continue independent lifecycle work while pending.