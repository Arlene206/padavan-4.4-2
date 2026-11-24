## Current repository state

- Live parent main before this atomic checker fix: e942deb224896716d3c968a80255430eb4006ae7.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains untouched at 72f4f157a084f8184b9597ca5bf334b8080e5807.
- FRP process argument/identity fix and checker correction: d2b954034d1a41de7dd9c170d597fbb6c2fa39b0, f933a44bcfa494d4aa4622b69d50c1713b6d2fc4.
- VSFTPD PORT/EPRT decimal parsing, IPv4/IPv6 structural validation, hextet width, RFC2428 delimiter and DEL-fixture repair: d7490d832b53d7e74b92ed92642413e1c19ed412, 990fe8a814c515df8c161231d088af0efb9de41f, 008c440e32d624fc98f7e2d0cf6ee56279b46abc, b77c7a3aca7a4d7a1a7026488e5ac453c3363dc7, b37af6b5caf6fa0346ef27cd94dd7c9e6b5f2fd5, ffeba1d20c4f03ed3f46b173cc92c7f253fa67bf.
- FTP REST strict marker validation: b87b1c10f3808f8bd4f47e374fdc5b0472bdfb94; unit-test compile/execution PASS, first source checker missed the decimal parser local binding (NameError). This commit restores the binding and only changes the checker.
- AliDNS encoder changed in d651b0f9d17861459b10f5026ef146172d568c54, then moved to BusyBox ash read -d/-n in 7b9b8aeb8c56f539e1a8532654056c6cb90748a8. The current test fixture re-execs Bash and an overlapping concurrent change removed several closing braces in the test script, so exact e942 AliDNS run #45355770998 failed at its test step. The read -n dependency is not enabled in the board's BusyBox ash configuration; replace it with target-safe logic, not a Bash-only test workaround.

## Active task

Restore VSFTPD static-source contract and validate the existing REST guard:
- Exact b87b1c10f3808f8bd4f47e374fdc5b0472bdfb94 run #45355332629 had GNU89 compilation and C policy tests PASS (including REST offset test), then failed the static contract with NameError: name 'decimal_parser' is not defined. The REST insertion replaced the pre-existing decimal parser binding.
- Restored decimal_parser = function_body(vsftpd_ipaddrparse, "vsf_sysutil_parse_uint(") before octet_parser. No production code or test behavior changed; exact-head validation pending.
- Exact b87 Process #45355332631 and AI handoff #45355332668 PASS; XY-C1 #45355332727 and NVRAM #45355332714 were still running at last observed status. The e942 AliDNS test run #45355770998 FAILED; Process #45355770855 PASS.

## Verification boundary

- Exact ffeba1d20c4f03ed3f46b173cc92c7f253fa67bf: VSFTPD #45354611125, Process #45354611121, AI handoff #45354611170 and NVRAM #45354611026 PASS.
- Exact b87b1c10f3808f8bd4f47e374fdc5b0472bdfb94: VSFTPD C compile/tests PASS; Python source contract FAIL (missing decimal_parser binding); Process/AI PASS; XY-C1 and NVRAM pending at last check.
- Exact e942deb224896716d3c968a80255430eb4006ae7: AliDNS #45355770998 FAIL in shell regression test; Process #45355770855 PASS.
- No claimed physical router validation. Do not equate a queued firmware build with PASS.

## Exact first action for the next GPT

1. Verify the VSFTPD static source checker passes on this checker-only commit.
2. Repair AliDNS test script structure and replace optional ash read -n usage with a target-safe byte-preserving encoder plus tests executed under POSIX ash-compatible shell.
3. Recheck exact b87 XY-C1/NVRAM run final statuses, then continue an independent userland stability/security defect.
4. Keep backup frozen and all development on main only.

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
