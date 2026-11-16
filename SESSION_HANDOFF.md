# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect live repository state and complete an accuracy checkpoint before ending. The checkpoint is mandatory; a document edit is conditional.

Continue the recorded engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using a non-forced compare-and-swap. Never force-push, rewrite history, create partial commits, or touch the frozen backup. Before context exhaustion, record precise live refs, exact Action results and the next concrete task.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; active branch: main.
- Parent for THIS XY-C1 modem checker correction: 5c74753c5af2951f477f733b1f3d6e1d78dc1824. Refetch live main before any later write and use expected-SHA non-forced CAS.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 = 72f4f157a084f8184b9597ca5bf334b8080e5807; immutable. Startup protocol/archive/PR recovery are complete; do not repeat.
- Root dev_log.md has 95 newest-first records before THIS entry; this entry makes 96 (keep 20–119).

## Active task

Repair the XY-C1 modem configuration checker after the modem loader was refactored to alternative arrays:
- reload_modem_modules() stores 7 NDIS and 4 RAS alternatives in ndis_modules[] / ras_modules[], forwards prefer_mbim=0 through the aligned NDIS parameter array, and calls load_modem_module_group().
- load_modem_module_group() uses rc_modem_module_load_group_verified() with module_smart_load plus is_module_loaded; it is intentionally incorrect for a source checker to require literal module_smart_load("cdc_ether", ...) calls inside reload_modem_modules().
- The previous exact-head XY-C1 run 43848100619 on aeba90439f0b68cd35a67be7d3e821164597bc4f failed before firmware build at scripts/check_usb_modem_driver_config.py:66 with AssertionError: RC modem reload no longer requests expected module cdc_ether. The actual USB identity/readiness regression 43848100315 passed.
- THIS patch changes only checker semantics: it verifies exact NDIS/RAS array contents/order, all 11 mapped Kconfig symbols as modules, the cdc_ncm parameter alignment, NDIS/RAS branch dispatch, generic verified loader callbacks and /sys/module post-load probe order. It retains USB serial option Kconfig/Makefile checks.
- Concurrent commit 5c74753c5af2951f477f733b1f3d6e1d78dc1824 propagated LAN startup errors from bridge IFUP, AP DHCP/manual setup and router static routes through PID 1 boot status and full_restart_lan(). Its Process, XY-C1 and several Actions were queued/in progress at last inspection; do not claim their result before rechecking.
- USB interface-to-driver binding remains distinct from module-family presence under /sys/module; not proven by this change.

## Verification boundary

Exact-head observations on parent 5c74753c5af2951f477f733b1f3d6e1d78dc1824:
- XY-C1 Firmware Validation 43848474467: IN_PROGRESS at last inspection, still before the modem configuration step; no Build/TRX/artifact L3 result is claimed.
- Process inventory and lifecycle 43848475098: IN_PROGRESS (inventory step running); the LAN host regression/source contract is part of this workflow.
- WAN ARP 43848474894, VPN server 43848474624, Inadyn 43848474126, detect_internet 43848474938, PID 1 notification queue 43848474904: SUCCESS.
- Other Actions on parent 5c74753c5af2951f477f733b1f3d6e1d78dc1824 were queued/in progress at last snapshot; re-fetch exact run list instead of reusing ancestor results.
- Ancestor aeba90439f0b68cd35a67be7d3e821164597bc4f: USB identity 43848100315 SUCCESS, HTTPD 43848100398 SUCCESS, AI docs 43848100602 SUCCESS, WAN route 43848100725 SUCCESS, WAN teardown 43848100704 SUCCESS, WAN DHCP 43848100491 SUCCESS, NVRAM 43848100605 was IN_PROGRESS. Ancestor XY-C1 43848100619 FAILED solely at the stale source checker described above.
- No current-parent Build XY-C1, TRX verification, artifact upload or physical-router/L4 success is established.

## Exact first action for the next GPT

1. Fetch live main; inspect this checker-fix commit's exact-head XY-C1, USB modem identity, Process/LAN, AI handoff, NVRAM and other relevant Actions. For L3, require Build XY-C1, TRX verification and artifact upload all PASS on one exact SHA.
2. Continue checking real NDIS interface-to-driver binding only if USB sysfs/hotplug exposes a reliable same-device association; module presence alone does not establish binding.
3. Audit the next concrete RC service failure propagation or stability defect outside the modem/LAN slices, and add the source regression before claiming closure.

## Governance constraints

Preserve security, architecture, compatibility, target toolchain and mandatory tests. Main only; frozen backup read-only. One complete Git tree and one atomic commit per logical task, non-forced expected-SHA update. Update dev_log.md for every source/documentation commit; keep 20–119 records and preserve archive shards/indexes. Distinguish host tests, exact-head CI, firmware/TRX and physical-router evidence. Never claim an unrun result.
