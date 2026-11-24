## Current repository state

- Active branch: `main`; current parent for this slice: `afba6d0005263387d20539f6719c1befbe8b223b`. Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. No open PRs.
- MTK AP private-ioctl, HTTPS-build HTTPD selector and WAN source-PHY range fixes retain their recorded exact-SHA L3 evidence. Physical-router L4 remains unverified.
- QMI teardown/credential fix on `b05eae9`: USB modem #45396493190, Process inventory #45396493113/#45396492751, AI docs #45396493061, NVRAM audit #45396492703, WAN DHCP callback #45396492710, WAN teardown #45396492829, detect-link #45396493228 and HTTPD #45396492941 PASS. XY-C1 #45396493118 was still building; TRX/artifact pending. Host L2 verified; no L3 claim.
- Initial DHCP pending-release fix on `afba6d0`: udhcpc regression/source contract #45397570960 and AI docs #45397570943 PASS; Process inventory #45397571017 PASS; NVRAM #45397571009 and USB storage #45397570936 in progress; XY-C1 #45397571031 was preparing the build. Follow-up below consolidates SIGUSR2 and `-R` exit handling.

## Active task — Preserve DHCPRELEASE across pending deconfig and exit paths

- In `dhcpc.c`, repeated SIGUSR1 while `RENEW_REQUESTED` invokes `deconfig` before returning to INIT. If that callback fails, the client enters `DECONFIG_PENDING` while the prior lease and server/IP tuple remain active.
- SIGUSR2 previously only changed the resume state; the first repair tracked the active lease and sent DHCPRELEASE, but duplicated packet logic outside `perform_release()`. A later review found `-R` exit could also call `perform_release()` while pending and miss the packet.
- This follow-up makes `perform_release()` recognize the saved active-lease bit during `DECONFIG_PENDING`; the SIGUSR2 branch calls the same routine. It therefore covers manual stop and release-on-quit, without duplicate release on NAK, expiry, ARP conflict, or a normal release already sent. Release-packet send errors are logged. GNU89 policy matrix and Python source-order contracts cover pending eligibility and shared release handling.
- Changed paths in the active task: BusyBox `dhcpc.c`, `udhcpc_script_policy.h`, its GNU89 test and `check_udhcpc_script_status.py`; workflow already watches all four.

## Verification boundary

- Parent `afba6d0005263387d20539f6719c1befbe8b223b`: initial udhcpc GNU89 ASan/UBSan regression and source checker PASS; AI docs PASS. Process inventory PASS; NVRAM/USB storage in progress; XY-C1 was still at build preparation.
- New follow-up commit must pass exact-head udhcpc tests/source checker, AI docs, Process/NVRAM and XY-C1 Build/TRX/artifact. Do not claim L3 until Build, TRX and artifact upload pass. Live DHCP-server transitions and physical-router L4 remain NOT VERIFIED.
- No local full checkout was available during recovery; all remote source changes are single atomic tree/commit updates to `main`. Do not claim local BusyBox/MIPS compilation unless actually run.

## Exact first action for the next GPT

1. Fetch live `main` and query exact-head Actions; inspect `udhcpc-script-status-stability.yml` policy regression and Python contract first.
2. Fix any causal regression failure atomically, then inspect XY-C1 Build/TRX/artifact for that same SHA.
3. Continue the next independent high-priority userland stability/security defect; do not re-audit QMI, SYN-cookie, MTK or HTTPD fixes.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Record only material exact-head Action transitions, new source commits, genuine unresolved failures, or a changed first action. Do not manufacture documentation churn or no-op commits. If all checkpoint facts and the first action remain accurate, explicitly conclude with no file change and no commit.

# Cross-session execution mandate

Recover once from live repository state and execute development on `main`. Never create or use a development branch or PR; the frozen backup is immutable.

For each multi-file logical slice, create all blobs, one complete tree based on the latest main tree and one commit with the exact main parent, then update `main` using a non-forced compare-and-swap with `expected_sha`. This is one atomic main commit. Never issue sequential partial file commits, force-push, amend, or rewrite published history.

Continue implementation rather than repeating completed audits or stopping at planning. PASS advances; FAIL is investigated and fixed; PENDING does not block independent tasks. Preserve Linux 4.4, GNU89, MIPS/uClibc, router compatibility, security and configuration.

Before context exhaustion, finish the current safe slice and record exact latest main SHA, tests, CI states, unresolved issues and one concrete first action for the next GPT. At session end, inspect live main and exact-head push Actions; update this handoff only when active task or first action materially changes, append substantive milestones to `dev_log.md`, and update roadmap only for material module/status/priority changes. Do not create no-op documentation commits.
