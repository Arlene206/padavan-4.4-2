# Cross-session execution mandate

## Workflow
- Develop on `main` only. One logical slice, one atomic main commit: create all blobs, one tree and one commit, then update main with non-forced expected-SHA compare-and-swap. Never force-push, amend, create branches/PRs, or edit the immutable backup.
- Preserve security, architecture, compatibility and necessary tests. Target GNU89, MIPS/uClibc, BusyBox and Linux 4.4. Never weaken regression contracts.
- Exact-head Actions validate only the matching SHA. Do not wait on PENDING; continue independent work. Fix a real FAIL at its cause.
- Keep root `dev_log.md` newest-first with 20–119 entries. Preserve `docs/dev-log/2026/11/part-021.md` and its index. No no-op commits or handoff snapshot archives.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live repository state and compare it with this handoff: source baseline, active task, exact-head Actions, validation boundary, blockers and first action. If all remain accurate, conclude without a file change or commit. Record relevant exact-head Action state changes when they alter validation gates.

## Current repository state
- Parent main before router-mode update fix: `624dd2468ec8bc6422a4278c55779b69222c8f48`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, immutable.
- New P1 defect: `common_ex.c:update_router_mode()` used error-collapsing `nvram_get_int()` reads for `sw_mode` and `wan_nat_x`, allowing transient NVRAM backend faults to rewrite persistent router mode to 4 or 1. This slice adds checked transactional reads and validates the complete values before any write; AP mode retains its legacy no-op behavior.
- Full WAN restart preflights router-mode refresh before `stop_wan()`; NETFILTER notification does not reload NAT/firewall after a failed refresh, while conntrack cleanup remains independent.
- NFS checked-enable fix `ce227363` + restart preflight correction `b284c285`: NFS lifecycle #44903863601 PASS on `b284c285`. Restart reads `nfsd_enable` after storage lock acquisition and before inventory/stop/unload.
- HTTPD duplicate NFS contract checker was corrected in `624dd2468ec8bc6422a4278c55779b69222c8f48`; exact HTTPD #44904054203 and docs #44904054208 both PASS on that SHA.
- On parent `b284c285`, NVRAM audit #44903863567 ultimately PASS; XY-C1 #44903863658 was in progress with Build underway and TRX/artifact pending at last observed state. Do not claim L3 until all three stages pass on one exact product-source SHA.
- MiniUPnP checked service-policy fix `0b0df9fd`: MiniUPnP lifecycle/docs and multiple process, Samba, HTTPD, WAN and DNSMasq exact gates passed on that SHA.
- Historical XY-C1 #44902070311 passed Build/TRX/artifact on product SHA `232e31d7` only; not later source. Physical router L4 NOT VERIFIED.
- Legacy PR #1 closed/unmerged; immutable backup unchanged.

## Active task
1. Inspect exact-head router-mode regression (new `router-mode-update` workflow compiles policy tests with GNU89/Werror/ASan/UBSan and checks callsite wiring).
2. Inspect XY-C1 Build/TRX/artifact for the latest production-source SHA and any current NVRAM audit.
3. If an Action fails, read the first causal log line and repair the real issue. Continue independent P1 work while checks are PENDING.

## Verification boundary
- Router-mode source/tests/workflow are included in the pending atomic commit. No local compilation or target build was performed.
- HTTPD #44904054203 and AI docs #44904054208 PASS on parent `624dd246`.
- NFS exact lifecycle #44903863601 PASS on `b284c285`; NVRAM audit #44903863567 PASS on same SHA.
- XY-C1 #44903863658 was in progress on older parent `b284c285`; Build/TRX/artifact result must be inspected independently. No current-source firmware/TRX pass is claimed.
- Hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch live main and inspect exact-head `router-mode-update` regression and docs.
2. Inspect XY-C1 Build/TRX/artifact and NVRAM audit for the latest production-source SHA.
3. Fix any exact-head failure at its cause; continue one distinct P1 reliability/security defect when Actions are pending.

Before context exhaustion, update this handoff only when active task, exact-head status, validation boundary or first action materially changes; no no-op commit.
