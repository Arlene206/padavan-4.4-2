# Cross-session execution mandate

## Workflow
- Develop on main only. One logical slice, one atomic main commit: create all blobs, one tree and one commit, then update main with non-forced expected-SHA compare-and-swap. Never force-push, amend, create branches/PRs, or edit the immutable backup.
- Preserve security, architecture, compatibility and necessary tests. Target GNU89, MIPS/uClibc, BusyBox and Linux 4.4. Never weaken regression contracts.
- Exact-SHA Actions validate only the matching SHA. Do not wait on PENDING; continue independent work. Fix real failures at the first causal point.
- Keep root dev_log.md newest-first with 20–119 dated entries. Preserve docs/dev-log/2026/11/part-021.md and its index. No no-op commits or handoff snapshot archives.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional. Before context exhaustion, inspect live main and compare this resumable record: source baseline, active task, exact-head Actions, validation boundary, blockers and first action. If all remain accurate, make no handoff commit.

## Current repository state
- Parent main before this iNIC NVRAM reliability fix: c1d2f8d9dee93bb374d3674958d6090d5de1e68a. Frozen archival branch backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807; immutable.
- RT3352 MII TxPower normalization fix is live in 917415b; Wireless radio, iNIC signal, watchdog signal, process inventory, AI docs and HTTPD security passed on that exact source SHA.
- SSH lifecycle checker repair was committed as c1d2f8d. Exact Process lifecycle #44906611688 and AI docs #44906611710 PASS on c1d2f8d. NVRAM transaction audit #44906405138 PASS on product source SHA 917415b.
- XY-C1 #44906404894 on exact product source SHA 917415b remains at Build XY-C1; TRX verification and artifact upload pending. No L3 claim until all stages complete on the same source SHA.
- Newly confirmed RT3352 iNIC config-read fail-open defect: start_inic_mii() and its AP caller read inic_disable through nvram_get_int(), whose nvram_get_checked failure collapses to zero. On a transient NVRAM backend error, the iNIC start path could treat an explicitly disabled module as enabled and change GPIO/RGMII/interface/radio state. The AP caller also reread the same flag independently for guest AP and health-monitor startup, allowing inconsistent decisions within one restart. This commit reads the flag once with nvram_get_checked plus strict integer parsing at the top of start_wifi_ap_rt, before mlme or bridge/hardware side effects, and passes one validated enabled decision through start_inic_mii, guest AP and iNIC monitor gates. Missing-key default zero and the legacy exactly-1-disabled rule are preserved; malformed/backend results fail closed.
- iNIC start-gate regression covers backend error, absent/default enabled, disabled, non-1 legacy enabled and NULL output. Wireless source contract pins checked read and error return before the MLME/hardware transition and asserts no raw inic_disable read remains.
- MiniUPnP internal-port-zero parser and SSH/NFS/router-mode/HTTPD/USB protections are already implemented; do not re-audit without new evidence. Physical router L4 remains NOT VERIFIED.

## Active task
1. Check exact-head Wireless radio and XY-C1 Actions created by the iNIC checked-read slice. If a source contract fails, fix only the first causal mismatch while preserving all checks.
2. Inspect XY-C1 Build XY-C1, TRX verification and artifact upload on exact current product-source SHA. Claim L3 only after the complete same-SHA chain passes.
3. Continue broad userland review for another concrete status-loss or unchecked configuration gate, with regression and exact-SHA validation.

## Verification boundary
- iNIC checked configuration preflight, shared boolean policy and GNU89 regression are committed together; no local cross-build or unit binary run is claimed in this environment. Exact-head Wireless validation pending.
- Process lifecycle and AI docs PASS on docs/test-only SHA c1d2f8d. NVRAM audit PASS on product source SHA 917415b.
- XY-C1 Build on 917415b still in progress; TRX/artifact pending. No current L3 or physical L4 claim.

## Exact first action for the next GPT
Fetch live main and inspect exact-head Wireless and XY-C1 runs for this iNIC checked-read slice. Poll the existing 917415b XY-C1 build's TRX/artifact steps. Continue another independent concrete status-propagation review if these are pending.

Before context exhaustion, update this handoff only when active task, exact-head result, validation boundary or first action materially changes; no no-op commit.