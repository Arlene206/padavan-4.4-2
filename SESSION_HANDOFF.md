## Current repository state

- Active branch: `main`; source parent for EPSV ALL fix: `5a7500ed1f2902f57b7faedec82fa87c587944a2`. Re-fetch live main/tree before every write.
- Frozen archive `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. No open PRs.
- MTK AP private-ioctl safety L3 verified on `d33fcf0`; MTK/NVRAM/AI/XY-C1 exact-SHA checks passed Build/TRX/artifact. HTTPD selector L3 verified on `659b91b`; HTTPD/AI/Process/XY-C1 passed Build/TRX/artifact.
- WAN source PHY range guard L3 verified on production SHA `d3733ec`; XY-C1 #45390205170 passed Build/TRX/artifact. WAN first-sample UNKNOWN guard on `18af4fb`: detect-link #45392147396, AI #45392146896, Process #45392147378, HTTPD security #45392147738, NVRAM #45392147111 and USB WDM #45392146933 PASS; XY-C1 #45392147184 was building at last poll.
- VSFTPD hidden-share path fix is on `5a7500e`; VSFTPD #45392889702, AI #45392889910 and Process #45392889923 PASS; XY-C1 #45392889756 was building and NVRAM #45392889664 pending at last poll. Physical L4 remains unverified.

## Active task — Clear stale FTP data endpoints on EPSV ALL

- `handle_pasv()` accepted EPSV ALL by setting `epsv_all` and returning before active/passive cleanup. A client could arm a PORT/EPRT destination or PASV listener, send EPSV ALL, then issue a transfer without new data setup; dispatch blocks later non-EPSV setup, but the earlier endpoint remained usable by `get_remote_transfer_fd()`.
- THIS COMMIT closes the passive listener and clears the active destination before setting EPSV ALL or sending success. The VSFTPD source contract requires both cleanup calls exactly once and before the state flag, success reply and return.
- VSFTPD and XY-C1 already watch `trunk/user/vsftpd/src/postlogin.c`; unknown changed production paths trigger full XY-C1 firmware validation.

## Verification boundary

- THIS COMMIT contains production cleanup, exact-source regression, handoff/roadmap/log updates in one atomic tree.
- Parent `5a7500e`: VSFTPD #45392889702, AI #45392889910 and Process #45392889923 PASS; NVRAM #45392889664 pending and XY-C1 #45392889756 building at last poll.
- Verify this SHA's VSFTPD regression and XY-C1 Build/TRX/artifact. Physical L4 remains unverified.

## Exact first action for the next GPT

1. Fetch main and inspect exact-head VSFTPD, AI, NVRAM and XY-C1 Build/TRX/artifact.
2. Fix the first causal CI failure atomically; keep firmware L3 open until Build, TRX and artifact pass on the same production SHA.
3. Continue the next unresolved stability/security issue without repeating verified MTK, HTTPD or WAN-selector work absent new evidence.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository before ending. Record only material exact-head Action transitions, new source commits, genuine unresolved failures, or a changed first action. Do not manufacture documentation churn or no-op commits.

A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action remain accurate, explicitly conclude the check with no file change and no commit.

# Cross-session execution mandate

Recover once from live repository state and execute development on `main`. Never create or use a development branch or PR; the frozen backup is immutable.

For each multi-file logical slice, create all blobs, one complete tree based on the latest main tree and one commit with the exact main parent, then update `main` using a non-forced compare-and-swap with `expected_sha`. This is one atomic main commit. Never issue sequential partial file commits, force-push, amend, or rewrite published history.

Continue implementation rather than repeating completed audits or stopping at planning. PASS advances; FAIL is investigated and fixed; PENDING does not block independent tasks. Preserve Linux 4.4, GNU89, MIPS/uClibc, router compatibility, security and configuration.

Before context exhaustion, finish the current safe slice and record exact latest main SHA, tests, CI states, unresolved issues and one concrete first action for the next GPT. At session end, inspect live main and exact-head push Actions; update this handoff only when active task or first action materially changes, append substantive milestones to `dev_log.md`, and update roadmap only for material module/status/priority changes. Do not create no-op documentation commits.
