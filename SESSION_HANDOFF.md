# Cross-session execution mandate

## Workflow
- Develop on main only. One logical slice, one atomic main commit: create all blobs, one tree and one commit, then update main with non-forced expected-SHA compare-and-swap. Never force-push, amend, create branches/PRs, or edit the immutable backup.
- Preserve security, architecture, compatibility and necessary tests. Target GNU89, MIPS/uClibc, BusyBox and Linux 4.4. Never weaken regression contracts.
- Exact-SHA Actions validate only the matching SHA. Do not wait on PENDING; continue independent work. Fix real failures at the first causal point.
- Keep root dev_log.md newest-first with 20–119 dated entries. Preserve docs/dev-log/2026/11/part-021.md and its index. No no-op commits or handoff snapshot archives.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional. Before context exhaustion, inspect live main and compare this resumable record: source baseline, active task, exact-head Actions, validation boundary, blockers and first action. If all remain accurate, make no handoff commit.

## Current repository state
- Product source base: 75d7dee28c30bca036a0287c5124dfb475e3b684; workflow/handoff-only child main before this indentation repair: 7344df1f59b04155c8cbffd8a96ee3a34600ac9d. Frozen archival backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807; immutable.
- RT3352 iNIC NVRAM fail-closed fix remains in 75d7dee: single checked inic_disable read before MLME/bridge/GPIO/RGMII/interface transitions; pass snapshot through start helper, guest gate and health monitor; GNU89 regression covers backend error/default/disabled/legacy non-1/NULL.
- Wireless radio #44907150820 on SHA 7344df1 produced a workflow run with zero jobs, caused by the prior edit inserting the new assertion that nvram_get_int("inic_disable") must be absent without the Python block's ten-space YAML indentation. The old actual Wireless job #44907026259 on SHA 75d7dee had passed all compiled wireless/bridge/guest-LAN tests then failed only at the stale monitor-gate source check. This commit fixes only accidental indentation; it restores the Python assert within the existing run block and does not weaken its contract.
- AI handoff docs #44907152023 PASS on SHA 7344df1.
- Process lifecycle #44907026283 and process inventory #44907026337 PASS on SHA 75d7dee; HTTPD #44907026445 PASS on SHA 75d7dee; NVRAM transaction audit #44907026244 PASS on SHA 75d7dee. iNIC signal #44907026286 and watchdog signal #44907026275 PASS on SHA 75d7dee.
- XY-C1 #44907026295 remains running on exact product-source SHA 75d7dee at Build XY-C1; TRX verification and artifact upload are pending. Earlier #44906404894 on 917415b likewise stayed in Build. No L3 claim until a same-SHA Build/TRX/artifact chain completes.
- Prior verified source work remains: RT3352 MII TxPower normalization in 917415b; checked SSH start/restart; NFS start/reload/restart checked config; router-mode NVRAM transaction; HTTPD hardening; USB storage gates; MiniUPnP SOAP bounded nonzero internal-port validation. Do not restart those tasks without fresh evidence. Physical router L4 remains NOT VERIFIED.

## Active task
1. Verify the corrected Wireless workflow is parsed and receives its single expected job; if source contract now runs, fix any next causal assertion without weakening tests.
2. Inspect XY-C1 #44907026295 Build/TRX/artifact on product SHA 75d7dee; current Build is pending/in progress, do not wait idly.
3. Continue another confirmed P1 source defect while CI jobs remain pending; check current main before every write.

## Verification boundary
- Wireless binaries passed on source SHA 75d7dee before a late source-contract mismatch; that assertion was corrected in 7344df1 but the new run had zero jobs due indentation error. This commit restores valid indentation; exact-head proper workflow job rerun pending.
- AI docs PASS on 7344df1; Process lifecycle, process inventory, HTTPD, NVRAM audit, iNIC signal and watchdog signal PASS on 75d7dee.
- XY-C1 Build remains in progress on 75d7dee; TRX/artifact pending. No same-SHA L3 result or physical L4 claim. No local cross-build/hardware test claimed.

## Exact first action for the next GPT
Fetch live main and inspect the corrected Wireless run's job count and first source-contract result. Check XY-C1 Build/TRX/artifact and continue independent source work while pending. Do not repeat completed module audits.

Before context exhaustion, update this handoff only when active task, exact-head result, validation boundary or first action materially changes; no no-op commit.