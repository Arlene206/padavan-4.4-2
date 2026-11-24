## Current repository state

- Live parent main before this atomic change: `b77c7a3aca7a4d7a1a7026488e5ac453c3363dc7`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- FRP exact argv/process-identity hardening and checker correction are recorded in `d2b954034d1a41de7dd9c170d597fbb6c2fa39b0` and `f933a44bcfa494d4aa4622b69d50c1713b6d2fc4`.
- VSFTPD strict numeric PORT/EPRT parsing and IPv4/IPv6 shape validation are in `d7490d832b53d7e74b92ed92642413e1c19ed412`, `990fe8a814c515df8c161231d088af0efb9de41f`, `008c440e32d624fc98f7e2d0cf6ee56279b46abc`, and `b77c7a3aca7a4d7a1a7026488e5ac453c3363dc7`.

## Active task

RFC 2428 EPRT delimiter compatibility:
- `handle_eprt()` split all four EPRT fields on the literal `|`, although RFC 2428 section 2 permits any printable ASCII delimiter (33–126) and recommends `|` only when it does not conflict with a character needed by the address.
- Added `vsf_sysutil_parse_eprt_delimiter()` to validate the opening delimiter and exactly four occurrences, then use the captured delimiter for all four EPRT field splits. Host regression covers standard/alternate delimiters, ASCII range rejection, missing/extra delimiters and unchanged output on rejection.
- Exact `b77c7a3aca7a4d7a1a7026488e5ac453c3363dc7`: VSFTPD run `#45353454360` PASS; Process inventory `#45353454331` PASS; AI handoff `#45353454513` PASS; NVRAM audit `#45353454240` was IN_PROGRESS at last observation. Exact-head verification for this change is pending. No hardware validation.

## Verification boundary

- Exact `e9ab40bb6118977f757d617e1deb5e49f1861f0c`: VSFTPD `#45353255628` PASS and AI handoff `#45353255604` PASS.
- Exact `008c440e32d624fc98f7e2d0cf6ee56279b46abc`: Process inventory `#45352668198` PASS and NVRAM boundary audit `#45352668211` PASS. VSFTPD `#45352668295` had a static checker locator false-negative, corrected and verified by `e9ab40b`.
- Exact `b77c7a3aca7a4d7a1a7026488e5ac453c3363dc7`: VSFTPD/Process/AI handoff PASS; NVRAM audit was still running at last observation.
- No XY-C1 firmware Build/TRX/artifact L3 or physical router L4 validation is claimed for this VSFTPD slice.

## Exact first action for the next GPT

1. Verify exact-head VSFTPD GNU89 parser regression and static source contract for RFC 2428 delimiter handling.
2. Check the final status of NVRAM audit `#45353454240`, then continue the next independent userland stability/security defect.
3. Record exact latest SHA and Actions transitions, and keep the frozen backup untouched.

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
