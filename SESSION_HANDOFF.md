## Current repository state

- Live main before the AliDNS static-check token-boundary repair: 32263cbbfe129697874d0f70a2760f989bbdfa8c.
- Frozen backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807.
- VSFTPD REST strict input validation and checker repair: 21699dc5a854afb6aea0121a64421bb836aa447c; exact VSFTPD #45356072893 and AI handoff #45356072970 PASS.
- AliDNS POSIX byte encoder/direct /bin/sh tests: ca9267b3c71a9fb2a8de96e82a2878fcf31e5dc6. Exact ca9267 shell tests all passed; the static checker later failed at the over-quoted printf assertion, fixed by 32263cbbfe129697874d0f70a2760f989bbdfa8c. That SHA's functional tests all passed but the static contract then falsely matched "od" inside the word "encode". The current checker now tests for a standalone od command by splitting potential shell pipeline segments and checking their first word.

## Active task

Re-run exact-head AliDNS source contract:
- Production encoder and test fixture are unchanged; only the static checker is modified.
- Exact 32263 AliDNS #45356801815: shell syntax, signed transport, address lookup, lifecycle/NVRAM and HMAC privacy tests PASS; final static contract failed on false-positive "od" substring. AI handoff #45356801657 PASS.
- Replaced the substring check with command-token detection; exact run on the next SHA pending.

## Verification boundary

- Exact ca9267: all functional AliDNS steps PASS; Python checker failed with SyntaxError due bad quoting.
- Exact 32263: all functional steps PASS; static contract failed because "od" substring matched "encode".
- Exact 21699: VSFTPD #45356072893 and AI #45356072970 PASS.
- No firmware hardware validation.

## Exact first action for the next GPT

1. Verify exact-head AliDNS source contract and POSIX shell regression after the command-token fix.
2. Check final XY-C1 Build/TRX/artifact run #45355332727 status.
3. Continue another independent userland stability/security defect; maintain the frozen backup and develop on main only.

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
