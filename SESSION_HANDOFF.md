## Current repository state

- Active branch: `main`; source parent for this slice: `18af4fb7c5a8027fce5efdd6ffecd794f5c4a230`. Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable.
- No open PRs. HTTPD selector and MTK AP private-ioctl slices retain their previously recorded L3 status; physical-router L4 is unverified.
- WAN source PHY selector range guard remains L3 verified on production SHA `d3733ec` including XY-C1 Build/TRX/artifact.
- WAN first-sample guard on exact SHA `18af4fb7c5a8027fce5efdd6ffecd794f5c4a230`: detect-link #45392147396, AI docs #45392146896, Process inventory #45392147378 / lifecycle #45392146934, USB modem #45392146933, HTTPD auth #45392147738 and NVRAM #45392147111 PASS. XY-C1 #45392147184 is still building; TRX and artifact are pending. No L3 claim yet.

## Active task — VSFTPD hidden-share marker path truncation

- `test_if_hidden_share()` formatted `<mount>/.hidden` into a 256-byte static buffer without checking `snprintf()`. A valid long mount component can truncate the path, make `stat()` miss the marker and expose a hidden share.
- Reuse checked `ftp_acl_path_build_child_path()`, enlarge the destination to `PATH_MAX`, and deny the share if construction fails. Regression covers normal paths, a 255-byte mount component, undersized output, the full bounded path and invalid leaf syntax.
- VSFTPD stability compiles/runs the GNU89 path-policy regression and checks production ordering. XY-C1 already watches `asus_ext.c`, `ftp_acl_path_policy.h` and this regression; exact-head Build/TRX/artifact are required for L3.

## Verification boundary

- At parent SHA `18af4fb7c5a8027fce5efdd6ffecd794f5c4a230`, detect-link, AI docs, Process, USB modem, HTTPD auth and NVRAM passed; XY-C1 Build was running and TRX/artifact pending.
- Check exact-head VSFTPD, AI docs and XY-C1 Actions. A host-only XY-C1 success is not firmware verification. Physical-router L4 remains unverified.

## Exact first action for the next GPT

1. Fetch live `main`, then query Actions for that exact head SHA.
2. Inspect VSFTPD regression/source contract and XY-C1 Build, TRX and artifact steps. Fix the first causal failure atomically; keep firmware L3 open while production build steps are pending or skipped.
3. Continue the next independent stability/security defect without re-auditing completed HTTPD/MTK/WAN-selector slices absent new evidence.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository before ending. Record only material exact-head Action transitions, new source commits, genuine unresolved failures, or a changed first action. Do not manufacture documentation churn or no-op commits.

A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action remain accurate, explicitly conclude the check with no file change and no commit.

# Cross-session execution mandate

Recover once from live repository state and execute development on `main`. Never create or use a development branch or PR; the frozen backup is immutable.

For each multi-file logical slice, create all blobs, one complete tree based on the latest main tree and one commit with the exact main parent, then update `main` using a non-forced compare-and-swap with `expected_sha`. This is one atomic main commit. Never issue sequential partial file commits, force-push, amend, or rewrite published history.

Continue implementation rather than repeating completed audits or stopping at planning. PASS advances; FAIL is investigated and fixed; PENDING does not block independent tasks. Preserve Linux 4.4, GNU89, MIPS/uClibc, router compatibility, security and configuration.

Before context exhaustion, finish the current safe slice and record exact latest main SHA, tests, CI states, unresolved issues and one concrete first action for the next GPT. At session end, inspect live main and exact-head push Actions; update this handoff only when active task or first action materially changes, append substantive milestones to `dev_log.md`, and update roadmap only for material module/status/priority changes. Do not create no-op documentation commits.
