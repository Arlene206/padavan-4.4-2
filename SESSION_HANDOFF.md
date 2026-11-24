## Current repository state

- Live parent main before the current AliDNS portability repair: 825e3ac7faab97eec889dc1e0109b34ba4785547.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains untouched at 72f4f157a084f8184b9597ca5bf334b8080e5807.
- FRP process argument/identity hardening and checker repair: d2b954034d1a41de7dd9c170d597fbb6c2fa39b0, f933a44bcfa494d4aa4622b69d50c1713b6d2fc4.
- VSFTPD PORT/EPRT numeric parsing, IPv4/IPv6 structure, hextet width, RFC2428 delimiter support and DEL fixture repair: d7490d832b53d7e74b92ed92642413e1c19ed412, 990fe8a814c515df8c161231d088af0efb9de41f, 008c440e32d624fc98f7e2d0cf6ee56279b46abc, b77c7a3aca7a4d7a1a7026488e5ac453c3363dc7, b37af6b5caf6fa0346ef27cd94dd7c9e6b5f2fd5, ffeba1d20c4f03ed3f46b173cc92c7f253fa67bf.
- VSFTPD REST strict 1–15 digit parser and regression: b87b1c10f3808f8bd4f47e374fdc5b0472bdfb94. Exact checker repair 21699dc5a854afb6aea0121a64421bb836aa447c: VSFTPD #45356072893 and AI handoff #45356072970 PASS. NVRAM #45355332714 later PASS; XY-C1 #45355332727 was still IN_PROGRESS at last observation.
- AliDNS RFC3986 encoder was changed in d651b0f9d17861459b10f5026ef146172d568c54, then moved to optional ash read -d/-n in 7b9b8aeb8c56f539e1a8532654056c6cb90748a8. That extension is not enabled in the board configuration and previous tests masked the mismatch by running Bash. Commit 825e3ac7faab97eec889dc1e0109b34ba4785547 restored the full fixture after an e942 test-script break; exact AliDNS #45356096386 and Process #45356096367 PASS.

## Active task

Remove the AliDNS BusyBox ash read-extension dependency:
- Replaced read -d '' -n1 with a POSIX read -r line loop under LC_ALL=C; byte-wise shell parameter expansion and printf percent-encode octets, while each consumed LF is explicitly encoded as %0A. The encoder uses no read -n, read -d, od or Bash.
- Removed the test's Bash re-exec and helper; tests call the sourced production functions directly under /bin/sh. Local /bin/sh checks passed for reserved ASCII, quote/backslash, spaces, UTF-8, controls, one/multiple/empty LF and unterminated final bytes. Exact-head CI pending.
- Parent 825e3ac7faab97eec889dc1e0109b34ba4785547: AliDNS #45356096386 and Process #45356096367 PASS. Parent 21699dc5a854afb6aea0121a64421bb836aa447c: VSFTPD #45356072893 and AI handoff #45356072970 PASS.

## Verification boundary

- Exact 825e3ac7faab97eec889dc1e0109b34ba4785547: AliDNS signed API transport regression and Process lifecycle inventory PASS.
- Exact 21699dc5a854afb6aea0121a64421bb836aa447c: VSFTPD stability regression and AI handoff contract PASS.
- Exact ffeba1d20c4f03ed3f46b173cc92c7f253fa67bf: VSFTPD #45354611125, Process #45354611121, AI handoff #45354611170 and NVRAM #45354611026 PASS.
- No physical router validation; in-progress XY-C1 runs are not marked PASS.

## Exact first action for the next GPT

1. Verify exact-head POSIX AliDNS encoder tests and static contract.
2. Check exact b87 XY-C1 Build/TRX/artifact run #45355332727 final status; if still pending, inspect latest firmware SHA.
3. Continue independent userland stability/security work; keep backup frozen and main-only.

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
