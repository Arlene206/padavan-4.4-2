## Current repository state

- Live main before the AliDNS checker syntax correction: ca9267b3c71a9fb2a8de96e82a2878fcf31e5dc6.
- Frozen backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807.
- VSFTPD REST validation is committed in b87b1c10f3808f8bd4f47e374fdc5b0472bdfb94. Its exact checker repair 21699dc5a854afb6aea0121a64421bb836aa447c passed VSFTPD #45356072893 and AI handoff #45356072970.
- AliDNS POSIX encoder and /bin/sh test conversion are in ca9267b3c71a9fb2a8de96e82a2878fcf31e5dc6. This commit's checker assertion around printf single-quote formatting was itself incorrectly quoted in Python and has been simplified to independent format/value token assertions; exact-head CI will retest.
- The parent 825e3ac7faab97eec889dc1e0109b34ba4785547 AliDNS regression and Process runs passed. Parent 21699 VSFTPD/AI documentation checks passed. Exact 825 b87 XY-C1 #45355332727 was still pending when last checked; b87 NVRAM #45355332714 passed.

## Active task

Finish the POSIX AliDNS byte encoder:
- Production uses POSIX read -r line input, LC_ALL=C byte slicing and %XX output; newline separators become %0A. It avoids BusyBox ash optional read -n/read -d and unavailable od.
- Test no longer invokes Bash; sourced functions are invoked in /bin/sh.
- The first ca9267 commit included a malformed Python quoting assertion in scripts/check_aliddns_api_transport.py. Replaced that with separate checks for "%02X" and the leading quote-value expression. The original shell implementation and fixture are unchanged by this checker correction.
- Exact ca9267 Actions were queued/in flight at the last observation.

## Verification boundary

- Exact 825e3ac7faab97eec889dc1e0109b34ba4785547: AliDNS #45356096386 and Process #45356096367 PASS.
- Exact 21699dc5a854afb6aea0121a64421bb836aa447c: VSFTPD #45356072893 and AI handoff #45356072970 PASS.
- Exact ffeba1d20c4f03ed3f46b173cc92c7f253fa67bf: VSFTPD #45354611125, Process #45354611121, AI #45354611170 and NVRAM #45354611026 PASS.
- Exact ca9267 POSIX AliDNS test/source contract must be verified on a corrected checker SHA. No hardware validation.

## Exact first action for the next GPT

1. Check the corrected exact-head AliDNS regression and static checker; diagnose runtime issues rather than weakening byte preservation.
2. Check final state of XY-C1 run #45355332727; if it is still pending, inspect the latest firmware build run.
3. Continue a separate userland stability/security defect, keeping the backup immutable.

# Cross-session execution mandate

Treat this as continuous work across GPT conversations. Recover once, then execute on `main`; each logical source or CI change is one atomic main commit. Create all changed blobs, one complete Git tree and one commit before updating the branch with a non-forced expected-SHA compare-and-swap. Never force-push or modify the frozen backup.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST perform a handoff checkpoint before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

Every session MUST inspect live repository state before ending. Record only material exact-head Action transitions, new source commits, genuine unresolved failures or a changed first action. Do not manufacture documentation churn or no-op commits.

## Development loop

- Source modification → targeted regression → one complete Git tree/one atomic commit → exact-SHA Actions.
- PASS advances; FAIL is diagnosed and fixed; PENDING does not block independent work.
- Preserve Linux 4.4, GNU89, MIPS/uClibc compatibility, router configuration, security, architecture and backward compatibility.
- Before context exhaustion, record exact latest SHA, tests, CI state, unresolved issues and one concrete first task. Never claim unverified work.
