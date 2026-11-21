# Cross-session execution mandate

## Workflow
- Develop on main only. One logical slice, one atomic main commit: create all blobs, one tree and one commit, then update main with non-forced expected-SHA compare-and-swap. Never force-push, amend, create branches/PRs, or edit the immutable backup.
- Preserve security, architecture, compatibility and necessary tests. Target GNU89, MIPS/uClibc, BusyBox and Linux 4.4. Never weaken regression contracts.
- Exact-SHA Actions validate only the matching SHA. Do not wait on PENDING; continue independent work. Fix real failures at the first causal point.
- Keep root dev_log.md newest-first with 20–119 dated entries. Preserve docs/dev-log/2026/11/part-021.md and its index. No no-op commits or handoff snapshot archives.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional. Before context exhaustion, inspect live main and compare this resumable record: source baseline, active task, exact-head Actions, validation boundary, blockers and first action. If all remain accurate, make no handoff commit.

## Current repository state
- Product source fix parent: 75d7dee28c30bca036a0287c5124dfb475e3b684. Frozen archival branch backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807; immutable.
- RT3352 iNIC NVRAM fail-closed fix is committed in 75d7dee: start_wifi_ap_rt() reads inic_disable once via nvram_get_checked and strict integer parse before MLME/bridge/radio side effects, passes one validated enabled decision to start_inic_mii, guest AP and health monitor. Backend/parse errors fail closed; absent default and legacy exactly-1 disabled semantics preserved. The GNU89 policy test covers backend error/default enabled/disabled/legacy non-1/NULL output.
- Wireless radio #44907026259 on exact 75d7dee passed the compiled wireless, bridge membership/teardown and guest-LAN status tests, then failed at its late Python source contract line 318. The last stale assumption was its health-monitor assertion still expected an independent raw nvram_get_int("inic_disable") gate. The production caller now uses the same validated inic_enabled snapshot as its start and guest AP. This commit updates that assertion and explicitly prohibits raw unchecked reads; it does not remove test stages.
- Process lifecycle #44906611688 and AI docs #44906611710 PASS on documentation/checker SHA c1d2f8d. The earlier exact Process run on 917415b had a checker-only failure, corrected and rerun PASS on c1d2f8d.
- XY-C1 #44907026295 is currently running on exact production source SHA 75d7dee; firmware input/build detection has not completed yet. The predecessor XY-C1 #44906404894 on 917415b remained at Build XY-C1 with TRX/artifact pending. No L3 claim.
- NVRAM transaction audit #44906405138 PASS on production source 917415b. The latest audit #44907026244 is in progress on 75d7dee at raw ioctl/kernel caller audit.
- MiniUPnP internal-port validation, SSH lifecycle, NFS checked start/restart, router-mode transactional updates, HTTPD and USB storage source contracts are already implemented and verified on their earlier exact source SHAs. Avoid repeating unless fresh evidence. Physical router L4 remains NOT VERIFIED.

## Active task
1. Inspect exact-head Wireless rerun from the corrected monitor-gate source contract; if it fails, diagnose the first remaining assertion/log without weakening source coverage.
2. Inspect XY-C1 #44907026295 Build, TRX verification and artifact upload on production source SHA 75d7dee; claim L3 only after all stages pass on that same SHA.
3. Check the NVRAM audit #44907026244 and continue broad userland review for a separate concrete status-loss or unchecked-config gate.

## Verification boundary
- iNIC checked config read/one-snapshot policy and GNU89 tests committed; compiled wireless/bridge/guest-LAN regression binaries PASS on 75d7dee, but one stale Python source assertion failed late and is corrected in this commit; exact-head rerun pending.
- Process lifecycle and AI docs PASS on c1d2f8d. NVRAM audit PASS on 917415b; newer audit pending.
- XY-C1 Build/TRX/artifact on 75d7dee pending. No L3 or physical L4 claim. No local target cross-build or router test is claimed.

## Exact first action for the next GPT
Fetch live main and inspect the newly triggered Wireless and XY-C1 runs for the monitor-gate checker repair. Check NVRAM audit #44907026244 independently. If those remain pending, continue another confirmed source-level reliability fix.

Before context exhaustion, update this handoff only when active task, exact-head result, validation boundary or first action materially changes; no no-op commit.