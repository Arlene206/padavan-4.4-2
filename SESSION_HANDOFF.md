# Cross-session execution mandate

## Workflow
- Develop on `main` only. One logical slice, one atomic main commit: create all blobs, one tree and one commit, then update main with non-forced expected-SHA compare-and-swap. Never force-push, amend, create branches/PRs, or edit the immutable backup.
- Preserve security, architecture, compatibility and necessary tests. Target GNU89, MIPS/uClibc, BusyBox and Linux 4.4. Never weaken regression contracts.
- Exact-head Actions validate only the matching SHA. Do not wait on PENDING; continue independent work. Fix a real FAIL at its cause.
- Keep root `dev_log.md` newest-first with 20–119 entries. Preserve `docs/dev-log/2026/11/part-021.md` and its index. No no-op commits or handoff snapshot archives.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect the live repository state before ending and compare it with this handoff: engineering/source baseline, active task, exact-head Actions, validation boundary, blockers and the exact first action. If all checkpoint facts and the first action remain accurate, explicitly conclude the check with no file change and no commit. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information when it changes the validation gate or next action.

## Current repository state
- Parent main before this NFS service-enable fix: `0b0df9fd43ab9d674ed4a55d1d6c52857cca7c61`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, immutable.
- MiniUPnP checked service-policy fix `0b0df9fd43ab9d674ed4a55d1d6c52857cca7c61`: MiniUPnP lifecycle #44903303300 PASS; AI handoff #44903303195 PASS; Samba #44903303343, Process fail-closed #44903303308, Process inventory #44903303227, HTTPD #44903303211, WAN route #44903303205, DNSMasq #44903303199 and Inadyn #44903303186 PASS. NVRAM audit #44903303263 and XY-C1 #44903303317 remain IN_PROGRESS. UPnP start/update use checked transactional reads of `upnp_enable_x`, `wan_nat_x`, and `wan_route_x`; errors abort without stopping a live daemon.
- Older exact source gates on `f99d7a43`: Firewall #44902745711, WAN route #44902745725, WAN DHCP #44902745734, HTTPD #44902745884, Process inventory #44902745720, FULLCONENAT #44902745686, AI docs #44902745743 and NVRAM audit #44902745715 PASS. XY-C1 #44902745804 is stale to newer product source.
- WAN route #44902460823 PASS and AI docs #44902460812 PASS on `dffca184e64dc44d8cfae11a43bbcebe21d9e0b6`; DNSMasq #44902327424 PASS and AI docs #44902327438 PASS on `5a4366981557ddef27f4bb0717543f84b353b36c`.
- XY-C1 #44902070311 on product source `232e31d7eeaa5a6a5e7510ce74a59d9b2f9a7950` passed Build, TRX and artifact upload, but that does not validate newer production commits. Physical router L4 remains NOT VERIFIED.
- NFS issue now addressed in this slice: `run_nfsd_locked()` and `reload_nfsd_locked()` previously collapsed `nfsd_enable` read errors through `nvram_invmatch()`; `restart_nfsd()` checked `nvram_match()` only after stopping NFS. New checked exact-match status preserves absent-as-disabled behavior and ensures restart preflights configuration before any stop.
- Legacy PR #1 remains closed/unmerged; backup unchanged.

## Active task
- Verify exact-head NFS host/source-contract regression, NVRAM audit and XY-C1 Build/TRX/artifact for the checked enablement gate.
- Confirm run/reload use checked `nfsd_enable` reads; restart reads before the first stop side effect and reuses the validated decision. No `nvram_match/invmatch("nfsd_enable", "1")` may remain in this code.
- Repair any exact-head failure at its first causal step. Keep independent PENDING work moving.
- Continue a distinct P1 defect in an unexamined userland subsystem only after the NFS gates are understood.

## Verification boundary
- MiniUPnP lifecycle and AI handoff Actions PASS on `0b0df9fd43ab9d674ed4a55d1d6c52857cca7c61`; that SHA's XY-C1 and NVRAM audit were pending at last poll.
- NFS policy regression/source assertions are committed but exact-head run pending; no local compile was run here.
- Earlier XY-C1 #44902070311 passed only on `232e31d7`; no current-source firmware/TRX success claimed until exact latest source build completes.
- Hardware L4 NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch live main; inspect exact-SHA NFS source/host contract, NVRAM audit and XY-C1 Build/TRX/artifact results.
2. Fix any FAIL from its exact job log; do not weaken tests or wait on PENDING.
3. Continue a distinct P1 userland reliability/security fix outside NFS, MiniUPnP, WAN, DNSMasq and firewall.

Before context exhaustion, update this handoff only when active task, exact-head status, validation boundary or first next action materially changes; no no-op commit.
