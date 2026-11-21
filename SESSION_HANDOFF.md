# Cross-session execution mandate

## Workflow
- Develop on `main` only. One logical slice, one atomic main commit: create all blobs, one tree and one commit, then update main with non-forced expected-SHA compare-and-swap. Never force-push, amend, create branches/PRs, or edit the immutable backup.
- Preserve security, architecture, compatibility and necessary tests. Target GNU89, MIPS/uClibc, BusyBox and Linux 4.4. Never weaken regression contracts.
- Exact-head Actions validate only the matching SHA. Do not wait on PENDING; continue independent work. Fix a real FAIL at its cause.
- Keep root `dev_log.md` newest-first with 20–119 entries. Preserve `docs/dev-log/2026/11/part-021.md` and its index. No no-op commits or handoff snapshot archives.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live repository state and compare it with this handoff: source baseline, active task, exact-head Actions, validation boundary, blockers and first action. If all remain accurate, conclude without a file change or commit. Record relevant exact-head Action state changes when they alter validation gates.

## Current repository state
- Parent main before router-mode checker correction: `d9bffce5e1dd5e3683102a5ced9350e26c15d02e`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, immutable.
- Router-mode fix commit `d9bffce5e1dd5e3683102a5ced9350e26c15d02e` changed `update_router_mode()` to a checked NVRAM transaction, preserved AP mode/no-op and absent-key defaults, validates complete decimal values, and gates full WAN restart before stop and NETFILTER NAT/firewall reload on success.
- Its exact workflow #44904764742 passed GNU89 `-Wall -Wextra -Werror` ASan/UBSan policy compile and execution, then failed only at a formatting-sensitive Python source assertion: the production checked-write call wraps after the comma. The correction in this commit whitespace-normalizes the extracted function before checking the same full-call token. Exact rerun pending.
- HTTPD #44904764718 and NFS #44904764717 PASS on `d9bffce5`. XY-C1 #44904764739 is building, NVRAM audit #44904764756 and AI docs #44904764588 were queued/in progress at the last poll.
- NFS fix `ce227363` plus placement correction `b284c285`: exact NFS lifecycle #44903863601 PASS on `b284c285`. HTTPD stale NFS wrapper-contract corrected in `624dd246`; HTTPD #44904054203 and AI docs #44904054208 PASS on that SHA.
- MiniUPnP checked service-policy fix `0b0df9fd`: lifecycle/docs and multiple process, Samba, HTTPD, WAN and DNSMasq exact gates PASS on that SHA.
- Historical XY-C1 #44902070311 passed Build/TRX/artifact on product SHA `232e31d7` only; does not validate later source. Physical router L4 NOT VERIFIED.
- Legacy PR #1 is closed/unmerged; backup unchanged.

## Active task
1. Check exact-head router-mode workflow rerun after whitespace normalization; policy binary already passes under ASan/UBSan.
2. Inspect current XY-C1 Build/TRX/artifact, NVRAM audit and docs checks for the latest product-source SHA.
3. Fix exact-head failures at their root cause. Continue an independent P1 userland defect while actions are PENDING.

## Verification boundary
- Router-mode policy binary passed on `d9bffce5`; first source-contract run failed only because checker did not normalize a multiline call. Corrected rerun pending.
- HTTPD #44904764718 and NFS #44904764717 PASS on `d9bffce5`.
- XY-C1 #44904764739, NVRAM audit #44904764756 and AI docs #44904764588 were pending at last poll. No current-source firmware/TRX success claimed.
- No local full RC compile or hardware L4 claimed.

## Exact first action for the next GPT
1. Fetch live main and inspect exact-head router-mode workflow result after whitespace normalization.
2. Inspect the corresponding XY-C1 Build, TRX verification, artifact upload, NVRAM audit and AI docs results.
3. Repair any new source-contract or build failure without weakening the tests, then continue a distinct P1 issue outside these recent fixes.

Before context exhaustion, update this handoff only when active task, exact-head status, validation boundary or first action materially changes; no no-op commit.
