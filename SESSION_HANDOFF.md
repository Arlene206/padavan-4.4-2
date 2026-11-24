## Current repository state

- Live main before the AliDNS checker syntax repair: 3151906ee28c42f38a8a283e4f94768cc31126bb.
- Frozen backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807.
- VSFTPD REST validation and checker repair are verified on 21699dc5a854afb6aea0121a64421bb836aa447c: VSFTPD #45356072893 and AI handoff #45356072970 PASS.
- AliDNS POSIX byte encoder and POSIX /bin/sh regression are in ca9267b3c71a9fb2a8de96e82a2878fcf31e5dc6. On exact ca9267, the shell syntax and all executed regression suites passed; the Python source-contract step failed with SyntaxError at line 127 caused by the over-quoted checker assertion. The checker-only fix simplifies the byte format assertion to a valid Python double-quoted literal.

## Active task

Verify the corrected AliDNS static source contract:
- POSIX encoder implementation and test shell suite are unchanged; exact ca9267 test run #45356506023 passed signed API transport, address lookup, lifecycle/NVRAM and HMAC argument-privacy steps. Only the final Python checker failed due SyntaxError.
- Corrected the checker assertion to look for the actual printf '%%%02X' format string without fragile nested quotes. Exact-head CI #45356634683 and AI handoff #45356634556 were in progress at the last observation.

## Verification boundary

- Exact 825e3ac7faab97eec889dc1e0109b34ba4785547: AliDNS #45356096386 and Process #45356096367 PASS.
- Exact 21699dc5a854afb6aea0121a64421bb836aa447c: VSFTPD #45356072893 and AI handoff #45356072970 PASS.
- Exact ca9267: shell syntax and signed API / IPv4 / IPv6 / lifecycle / HMAC privacy tests PASS; Python static contract FAIL (line 127 SyntaxError).
- No physical router validation; exact b87 XY-C1 #45355332727 was still in progress at last observation.

## Exact first action for the next GPT

1. Verify exact-head AliDNS regression and source checker after the quoting correction.
2. Check XY-C1 Build/TRX/artifact run #45355332727 final state.
3. Continue another independent userland stability/security defect; keep backup frozen and develop on main only.

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
