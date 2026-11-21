# Cross-session execution mandate

## Workflow
- Develop on `main` only. One logical slice, one atomic main commit: create all blobs, one tree and one commit, then update main with non-forced expected-SHA compare-and-swap. Never force-push, amend, create branches/PRs, or edit the immutable backup.
- Preserve security, architecture, compatibility and necessary tests. Target GNU89, MIPS/uClibc, BusyBox and Linux 4.4. Never weaken regression contracts.
- Exact-head Actions validate only the matching SHA. Do not wait on PENDING; continue independent work. Fix real failures at their cause.
- Keep root `dev_log.md` newest-first with 20–119 entries. Preserve `docs/dev-log/2026/11/part-021.md` and its index. No no-op commits or handoff snapshot archives.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live repository state and compare it with this handoff: source baseline, active task, exact-head Actions, validation boundary, blockers and first action. If all remain accurate, conclude without a file change or commit. Record relevant exact-head Action changes when they alter validation gates.

## Current repository state
- Parent main before XY-C1 source-contract repair: `0398903bb6475668035313743f51a827cfe7f919`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, immutable.
- Router-mode production fix `d9bffce5e1dd5e3683102a5ced9350e26c15d02e`: `update_router_mode()` now uses one checked NVRAM transaction, preserves AP mode and absent defaults, validates complete values, and publishes writes only after successful reads. Full WAN restart preflights before stop; NETFILTER NAT/firewall reload is gated on update success.
- Router mode dedicated regression #44904910387 PASS on `0398903bb6475668035313743f51a827cfe7f919`: GNU89 `-Wall -Wextra -Werror` ASan/UBSan policy test and source wiring contract both succeeded.
- XY-C1 #44904764739 on `d9bffce5` FAILED before firmware compile/build, in `scripts/check_modem_wan_start_status.py` assertion #1114: the source contract counted only a one-line call, while the legitimate NETFILTER failure guard reflowed one call. This commit makes the assertion whitespace-tolerant and retains exact count=2.
- HTTPD #44904764718 and NFS #44904764717 PASS on `d9bffce5`. NVRAM audit #44904764756 and AI docs #44904764588 were still in progress/queued at last poll.
- NFS checked-enable fix `ce227363` plus placement correction `b284c285`: NFS lifecycle #44903863601 PASS. HTTPD NFS wrapper checker correction `624dd246`: HTTPD #44904054203 and AI docs #44904054208 PASS on that SHA.
- MiniUPnP checked service-policy fix `0b0df9fd`: lifecycle/docs and multiple process, Samba, HTTPD, WAN and DNSMasq source checks PASS on that SHA.
- Historical XY-C1 #44902070311 passed Build/TRX/artifact on source `232e31d7` only. No newer product-source L3 claim yet. Physical router L4 NOT VERIFIED.
- Legacy PR #1 is closed/unmerged; backup unchanged.

## Active task
1. Verify the whitespace-tolerant XY-C1 source contract on the current SHA, then inspect actual Build XY-C1, Verify TRX and artifact upload stages individually.
2. Inspect NVRAM transaction audit and AI handoff docs for the same source SHA.
3. Continue an independent P1 userland issue while actions are pending; fix exact-head failures at first causal log.

## Verification boundary
- Router-mode dedicated policy/wiring workflow PASS on `0398903b`.
- XY-C1 #44904764739 was a source-checker failure before cross-build, not a compiler failure. Its checker correction is committed, exact rerun pending.
- HTTPD #44904764718 and NFS #44904764717 PASS on `d9bffce5`.
- NVRAM audit #44904764756 and docs #44904764588 were pending at last poll.
- No current-source full MIPS firmware, TRX, artifact or hardware L4 success claimed.

## Exact first action for the next GPT
1. Fetch live main and exact-head XY-C1 result; verify the corrected checker and inspect Build, TRX and artifact job steps.
2. Inspect exact-head NVRAM audit and AI docs.
3. Fix any new failure at its root cause, then continue distinct P1 userland stability/security work.

Before context exhaustion, update this handoff only when active task, exact-head status, validation boundary or first action materially changes; no no-op commit.
