## Current repository state

- Live parent main before this atomic REST-input fix: `ffeba1d20c4f03ed3f46b173cc92c7f253fa67bf`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- FRP exact argv/process-identity hardening: `d2b954034d1a41de7dd9c170d597fbb6c2fa39b0`; checker correction: `f933a44bcfa494d4aa4622b69d50c1713b6d2fc4`.
- VSFTPD strict numeric parsing, IPv4/IPv6 shape validation, IPv6 hextet width and RFC2428 EPRT delimiter handling: `d7490d832b53d7e74b92ed92642413e1c19ed412`, `990fe8a814c515df8c161231d088af0efb9de41f`, `008c440e32d624fc98f7e2d0cf6ee56279b46abc`, `b77c7a3aca7a4d7a1a7026488e5ac453c3363dc7`, `b37af6b5caf6fa0346ef27cd94dd7c9e6b5f2fd5`, fixture-only correction `ffeba1d20c4f03ed3f46b173cc92c7f253fa67bf`.
- Concurrent AliDNS RFC3986 byte-encoding fix: `d651b0f9d17861459b10f5026ef146172d568c54`; test calibration `6a82821ef8f39fd6964f6fb314d493c6fd5c7379`. Exact `6a82821` AliDNS run #45354431136 failed at HMAC expected-signature calculation; its job log reported `HMAC helper could not sign the expected canonical request`. Investigate without weakening the encoder.

## Active task

VSFTPD REST marker input validation:
- `handle_rest()` used `str_a_to_filesize_t()` then tested `val < 0`. The underlying converter returns zero for invalid characters and inputs longer than 15 bytes, so malformed values (empty, `abc`, `+1`, `-1`, whitespace, 16 digits) were acknowledged with FTP 350 as offset zero.
- Added a GNU89 policy parser for 1–15 ASCII decimal digits, rejecting null/empty/non-decimal/overlength input and preserving output on failure. `handle_rest()` now rejects invalid input with FTP_BADCMD, clears stale one-shot restart state, and only sends FTP_RESTOK after a valid parse. Regression covers zero, leading zeroes, maximum 15-digit input, signs, suffix, whitespace, embedded NUL, null/empty and overlength.
- Exact parent `ffeba1d20c4f03ed3f46b173cc92c7f253fa67bf`: VSFTPD #45354611125 PASS; Process inventory #45354611121 PASS; AI handoff #45354611170 PASS; NVRAM audit #45354611026 was IN_PROGRESS at last observation. This REST change needs exact-head verification. No XY-C1 firmware workflow was triggered by the prior test-only change.

## Verification boundary

- Exact `ffeba1d20c4f03ed3f46b173cc92c7f253fa67bf`: VSFTPD #45354611125, Process #45354611121 and AI handoff #45354611170 PASS; NVRAM #45354611026 pending at last observation.
- Exact `6a82821ef8f39fd6964f6fb314d493c6fd5c7379`: Process #45354431099 PASS; AliDNS #45354431136 FAIL at HMAC expected-signature calculation.
- No exact-SHA XY-C1 Build/TRX/artifact L3 or physical router L4 claim for this work.

## Exact first action for the next GPT

1. Verify exact-head GNU89 REST-offset regression and VSFTPD static source contract.
2. Fix AliDNS expected-signature calculation on latest inherited SHA without weakening RFC3986 byte encoding.
3. Recheck NVRAM audit #45354611026 and continue the next independent userland stability/security defect.
4. Preserve the frozen backup and develop on main only.

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
