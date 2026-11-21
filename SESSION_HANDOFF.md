# Cross-session execution mandate

## Workflow
- Develop on main only. One logical slice, one atomic main commit: create all blobs, one tree and one commit, then update main with non-forced expected-SHA compare-and-swap. Never force-push, amend, create branches/PRs, or edit the immutable backup.
- Preserve security, architecture, compatibility and necessary tests. Target GNU89, MIPS/uClibc, BusyBox and Linux 4.4. Never weaken regression contracts.
- Exact-SHA Actions validate only the matching SHA. Do not wait on PENDING; continue independent work. Fix real failures at the first causal point.
- Keep root dev_log.md newest-first with 20–119 dated entries. Preserve docs/dev-log/2026/11/part-021.md and its index. No no-op commits or handoff snapshot archives.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional. Before context exhaustion, inspect live main and compare this resumable record: source baseline, active task, exact-head Actions, validation boundary, blockers and first action. If all remain accurate, make no handoff commit.

## Current repository state
- Parent main before SSH fix: e84b9a30b63f1ed62ed96e361d5b085f70f8be3c. Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807 and is immutable.
- Product-source baseline before this SSH slice: d9bffce5e1dd5e3683102a5ced9350e26c15d02e. Subsequent commits through e84b9a30b63f1ed62ed96e361d5b085f70f8be3c are checker/workflow/handoff changes; firmware has not been rebuilt for that source baseline.
- SSH defect: start_sshd() used error-collapsing nvram_get_int("sshd_enable"); restart_sshd() stopped the running listener before this read. This slice adds checked mode parsing, a checked direct-start wrapper and a restart preflight before inventory/stop; restart reuses the same validated value after teardown.
- Router-mode NVRAM transaction fix d9bffce5; dedicated policy/source wiring run #44904910387 PASS on 0398903. WAN restart is gated before stop and NETFILTER NAT/firewall reload is gated on successful update.
- NFS checked-enable fix ce227363 + restart preflight correction b284c285: exact NFS lifecycle #44903863601 PASS on b284c285. HTTPD wrapper contract correction 624dd246: HTTPD #44904054203 and AI docs #44904054208 PASS.
- MiniUPnP checked service policy 0b0df9fd: lifecycle/docs and process, Samba, HTTPD, WAN, DNSMasq and Inadyn exact gates passed on that SHA. SOAP internal-port-zero validation was already implemented in 1eb75818300dbefc79fbd3f706670df0e3011a6a with tests; AddAnyPortMapping external-port-zero behavior remains permitted.
- Checker/docs SHA 2222f27: Process inventory #44905479794, USB modem #44905479781 and Firewall Startup #44905479780 PASS. XY-C1 #44905479795 concluded success, but Build XY-C1, TRX and artifact upload were all SKIPPED since no product-source files changed. It is not L3 for source baseline d9bffce5.
- AI documentation #44905479770 initially failed because handoff headings were noncanonical; the headings were corrected in e84b9a30. Exact rerun status must be checked.
- Legacy PR #1 is closed/unmerged; backup unchanged. Physical router L4 remains NOT VERIFIED.

## Active task
1. Verify exact-head process-lifecycle after this SSH checked-enable fix, especially GNU89/ASan/UBSan sshd policy tests and updated start/restart source contract.
2. Verify XY-C1 Build, TRX validation and artifact upload independently for the production-source SHA. Do not claim L3 until all three pass on that same SHA.
3. Verify AI handoff documentation after e84b9a30's section-heading correction. Continue independent P1 work while Actions are PENDING.

## Verification boundary
- SSH production source, policy tests and source contract are included in this atomic slice. Exact-head Actions pending.
- Router-mode dedicated policy/wiring test passed on earlier source-equivalent SHA 0398903; current production source baseline d9bffce5 still needs a successful full target build.
- XY-C1 #44905479795 performed no build: Build, TRX and artifact steps were skipped due docs/checker-only changes.
- No local RC compile, current-source firmware/TRX result or physical router L4 is claimed.

## Exact first action for the next GPT
1. Fetch live main and inspect exact-SHA process-lifecycle SSH regression and AI handoff documentation.
2. Inspect XY-C1 Build XY-C1, Verify TRX output and artifact upload steps separately for the same production-source SHA.
3. Fix exact-head failures at their first cause, then continue the next distinct P1 userland stability/security defect.

Before context exhaustion, update this handoff only when active task, exact-head result, validation boundary or first action materially changes; no no-op commit.
