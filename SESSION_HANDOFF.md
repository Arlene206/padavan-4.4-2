# Cross-session execution mandate

## Workflow
- Develop on main only. One logical slice, one atomic main commit: create all blobs, one tree and one commit, then update main with non-forced expected-SHA compare-and-swap. Never force-push, amend, create branches/PRs, or edit the immutable backup.
- Preserve security, architecture, compatibility and necessary tests. Target GNU89, MIPS/uClibc, BusyBox and Linux 4.4. Never weaken regression contracts.
- Exact-SHA Actions validate only the matching SHA. Do not wait on PENDING; continue independent work. Fix real failures at the first causal point.
- Keep root dev_log.md newest-first with 20–119 entries. Preserve docs/dev-log/2026/11/part-021.md and its index. No no-op commits or handoff snapshot archives.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional. Before context exhaustion, inspect live main and compare this resumable record: source baseline, active task, exact-head Actions, validation boundary, blockers and first action. If all remain accurate, make no handoff commit.

## Current repository state
- Current main before this RT3352 compile-safety correction: `7f5b2a1676aca910352ac9106c3c8289384aa963`. Product source includes PPP checked-option work from `bf43b52d3039ff89a882b8260eca4823c92b0ae2`, workflow path-filter correction `2e960bf`, and route status checker alignment `7f5b2a1`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. Only main and backup branches exist.
- PPP WAN operation status Action #44908223067 PASS on exact main SHA `7f5b2a1`; AI handoff docs #44908223050 PASS on the same SHA. NVRAM audit #44907884214 and Wireless radio #44907884270 PASS on product SHA `bf43b52`; Process lifecycle #44907884319 and process inventory #44907884347 PASS on `bf43b52`; NFS/HTTPD/USB storage checks passed on the same source SHA.
- XY-C1 #44907884176 is still in progress against production-source SHA `bf43b52`; its Build XY-C1 step was in progress, with TRX verification and artifact upload pending at last inspection. No L3 claim.
- RT3352 MII iNIC checked-disable source fix in `75d7dee` assigned the result of `start_inic_mii(inic_enabled)` to `operation_status` under `USE_RT3352_MII` but did not declare that identifier in `start_wifi_ap_rt()` scope. The identifier is not declared in `rc.h` or the wireless policy header. This commit declares a function-local variable within the same conditional block and adds a source contract requiring it; behavior is otherwise unchanged. This closes a target-configuration compile blocker that the stand-alone policy tests do not compile.
- Existing completed source protections: transactional router-mode NVRAM update; checked NFS start/reload/restart; HTTPD hardening; USB storage gates; bounded MiniUPnP internal-port parser; RT3352 TxPower normalization; iNIC checked config preflight; checked SSH start/restart; PPP typed-option preflight and checked default-route primary. Do not repeat those audits absent fresh failure evidence. Physical router L4 remains NOT VERIFIED.

## Active task
1. Inspect exact-head Wireless radio and XY-C1 Actions triggered by this RT3352 compile correction. Resolve the first actual compiler or source-contract failure, if any.
2. Inspect XY-C1 Build, TRX verification and artifact upload on the new product-source SHA. Claim L3 only if all three pass on that same SHA.
3. Continue a separate confirmed P1 reliability/security issue after checking current main and preserving the existing regression set.

## Verification boundary
- PPP WAN route operation status and AI handoff documentation PASS on `7f5b2a1`.
- Wireless radio, Process lifecycle/inventory and NVRAM audit PASS on prior production-source SHA `bf43b52`; these do not validate the new RT3352 local declaration.
- XY-C1 Build remains in progress on `bf43b52`; TRX/artifact pending. This source correction should trigger a new same-SHA build. No current L3 or physical L4 claim.
- No local target cross-compile or physical router test is claimed.

## Exact first action for the next GPT
Fetch live main and inspect exact-head Wireless and XY-C1 runs for this RT3352 source correction. Review the actual compile log for any next failure; do not wait on pending jobs before continuing independent source work.

Before context exhaustion, update this handoff only when active task, exact-head result, verification boundary or first action materially changes; no no-op commit.