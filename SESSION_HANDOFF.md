## Current repository state

- Active branch: `main`; parent for this slice: `aa3d55bf141f88590c6b675d09ff28c51eb0e6ce`. Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. No open PRs.
- MTK AP private-ioctl and HTTPD selector fixes remain L3 verified on `d33fcf0` and `659b91b` respectively, including exact-SHA firmware Build/TRX/artifact. WAN source-PHY range guard is L3 verified on `d3733ec` with XY-C1 Build/TRX/artifact PASS.
- WAN first-sample UNKNOWN guard: detect-link #45392147396, AI #45392146896, Process #45392147378/#45392146934, HTTPD #45392147738, NVRAM #45392147111 and USB modem #45392146933 PASS on `18af4fb`; its firmware Build/TRX/artifact verification is not recorded as complete.
- VSFTPD hidden-share fix on `5a7500e`: VSFTPD #45392889702, AI #45392889910 and Process #45392889923 PASS. EPSV ALL cleanup on `aa3d55b`: VSFTPD #45393626354, AI #45393626335, Process #45393626312 and NVRAM #45393626295 PASS. Current XY-C1 #45393626376 is still building; TRX and artifact steps are pending. Both VSFTPD production fixes remain below L3 pending same-SHA firmware verification.

## Active task — Reject whitespace/sign prefixes in the SYN-cookie selector

- `rc_network_sysctl_parse_syncookies_status()` used `strtol()` and checked its final boolean domain, but `strtol()` normalizes leading whitespace and optional signs. Corrupt NVRAM values such as `" 0"`, `"\t1"`, `"+0"` or `"-0"` could therefore be treated as a deliberate SYN-cookie disable.
- Reject a non-decimal first byte before calling `strtol()`, preserving full-token, 0/1 and overflow checks. Regression covers signs, spaces, tabs/newlines and unchanged output on rejection. `wan-route-status.yml` already compiles/runs the GNU89 ASan/UBSan regression and executes the Python source contract; XY-C1 watches the production policy header.
- Concurrent XY-C1 #45393626376 is still running Build with TRX/artifact pending; the changed policy header will trigger a fresh exact-head firmware Build.

## Verification boundary

- Parent `aa3d55bf141f88590c6b675d09ff28c51eb0e6ce`: VSFTPD, AI docs, Process and NVRAM PASS; XY-C1 Build in progress, TRX/artifact pending.
- New exact-head WAN route regression/source checker, AI docs, NVRAM and XY-C1 Build/TRX/artifact must be checked. No L3 claim while any firmware step remains pending or skipped. Physical-router L4 remains unverified.

## Exact first action for the next GPT

1. Fetch live `main` and query Actions by that exact SHA.
2. Inspect SYN-cookie GNU89/ASan/UBSan policy regression and source checker; fix any causal failure atomically.
3. Inspect XY-C1 Build/TRX/artifact for the same production SHA, then continue the next independent stability/security defect without re-auditing completed MTK/HTTPD/WAN-selector slices.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository before ending. Record only material exact-head Action transitions, new source commits, genuine unresolved failures, or a changed first action. Do not manufacture documentation churn or no-op commits.

A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action remain accurate, explicitly conclude the check with no file change and no commit.

# Cross-session execution mandate

Recover once from live repository state and execute development on `main`. Never create or use a development branch or PR; the frozen backup is immutable.

For each multi-file logical slice, create all blobs, one complete tree based on the latest main tree and one commit with the exact main parent, then update `main` using a non-forced compare-and-swap with `expected_sha`. This is one atomic main commit. Never issue sequential partial file commits, force-push, amend, or rewrite published history.

Continue implementation rather than repeating completed audits or stopping at planning. PASS advances; FAIL is investigated and fixed; PENDING does not block independent tasks. Preserve Linux 4.4, GNU89, MIPS/uClibc, router compatibility, security and configuration.

Before context exhaustion, finish the current safe slice and record exact latest main SHA, tests, CI states, unresolved issues and one concrete first action for the next GPT. At session end, inspect live main and exact-head push Actions; update this handoff only when active task or first action materially changes, append substantive milestones to `dev_log.md`, and update roadmap only for material module/status/priority changes. Do not create no-op documentation commits.
