## Current repository state

- Live parent main before this atomic VSFTPD test correction: `6a82821ef8f39fd6964f6fb314d493c6fd5c7379`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- FRP exact argv/process-identity hardening and checker correction: `d2b954034d1a41de7dd9c170d597fbb6c2fa39b0`, `f933a44bcfa494d4aa4622b69d50c1713b6d2fc4`.
- VSFTPD strict numeric parsing, IPv4/IPv6 shape checks, hextet width, and RFC 2428 EPRT delimiter implementation: `d7490d832b53d7e74b92ed92642413e1c19ed412`, `990fe8a814c515df8c161231d088af0efb9de41f`, `008c440e32d624fc98f7e2d0cf6ee56279b46abc`, `b77c7a3aca7a4d7a1a7026488e5ac453c3363dc7`, `b37af6b5caf6fa0346ef27cd94dd7c9e6b5f2fd5`.
- Concurrent AliDNS RFC3986 byte-encoding fix: `d651b0f9d17861459b10f5026ef146172d568c54`; exact `d651b0f` run #45354160627 failed on the pre-change signature-encoding expectation. Follow-up test calibration is in `6a82821ef8f39fd6964f6fb314d493c6fd5c7379`; exact-head Actions are still running.

## Active task

Correct and re-run the EPRT DEL-byte regression:
- Exact `b37af6b5caf6fa0346ef27cd94dd7c9e6b5f2fd5` VSFTPD run `#45353862964` compiled all policy binaries and passed earlier FTP suites, but parser policy failed because the C fixture had two backslashes before `x7f`, passing literal text instead of byte 127.
- Corrected the DEL input to adjacent C string literals with the bounded octal escape `\177`, and corrected the diagnostic newline. Adjacent literals also avoid C hexadecimal-escape digit coalescing. Production parser/handler remain unchanged; exact-head regression and static checker still need to pass.
- Exact `b37af6b`: Process inventory `#45353862966` and AI handoff `#45353863108` PASS.
- Exact `d651b0f`: HTTPD #45354160704 PASS, Process inventory #45354160643 PASS, AliDNS #45354160627 FAIL on the old signature expectation, and XY-C1 #45354160727 was pending. The follow-up AliDNS test commit on `6a82821` requires exact-head confirmation.

## Verification boundary

- Exact `b77c7a3aca7a4d7a1a7026488e5ac453c3363dc7`: VSFTPD #45353454360, Process inventory #45353454331, AI handoff #45353454513 PASS; NVRAM #45353454240 cancelled.
- Exact `b37af6b`: VSFTPD failed only on the malformed DEL test fixture; Process and AI docs PASS.
- Exact `d651b0f`: Process/HTTPD PASS; AliDNS test failed; XY-C1 was pending.
- Exact `6a82821`: AliDNS regression and Process inventory were in progress at last observation.
- No XY-C1 Build/TRX/artifact L3 or physical router L4 claim until exact-SHA evidence is complete.

## Exact first action for the next GPT

1. Verify the corrected exact-head VSFTPD GNU89 parser regression and static source contract.
2. Confirm exact-head AliDNS regression and Process results for `6a82821`; fix any remaining test or production issue without weakening RFC3986 byte encoding.
3. Check exact-SHA XY-C1 Build, TRX and artifact state; then continue another independent userland stability/security defect.
4. Keep the frozen backup untouched and development on main only.

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
