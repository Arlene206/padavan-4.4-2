## Current repository state

- Live parent main before the VSFTPD workflow path-checker correction: `ea7d7e28be9988175fbcc423f9378e5d765de764`.
- Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- FRP procfs transform failure remains UNKNOWN instead of false ABSENT: `0104186915ee6132d99871bede241ecf1835c279`; exact FRP lifecycle #45358242268 and Process inventory #45358242232 PASS.
- VSFTPD SITE CHMOD/UMASK strict parser and GNU89 regression: `ea7d7e28be9988175fbcc423f9378e5d765de764`. Runtime compile/test completed before the inline workflow contract failed at a global YAML path count; correction now validates each new path in the proper push/PR filter blocks.
- AliDNS POSIX byte encoder source `ca9267b3c71a9fb2a8de96e82a2878fcf31e5dc6`; exact AliDNS #45357686604 PASS on checker SHA `824f2b4fcc2365af7502ad833a1ad0cacec1e5a4`. Exact XY-C1 #45356505924 on ca9267 PASS for Build, TRX and artifact upload; Process #45356505921 and HTTPD auth #45356505920 PASS.
- VSFTPD REST strict decimal parser `b87b1c10f3808f8bd4f47e374fdc5b0472bdfb94`; checker binding correction `21699dc5a854afb6aea0121a64421bb836aa447c`; their exact validation is recorded in the preceding handoff.
- Main remains the active branch; the frozen backup is untouched.

## Active task — Re-run SITE CHMOD/UMASK strict parser checks

- `ftp_octal_arg_parse()` accepts only complete ASCII octal tokens in 0000..0777 with overflow checks and unchanged output on failure. Both SITE handlers validate before fchmod/open-side effects or process umask mutation.
- The runtime policy regression compiled under GNU89 and passed locally. Exact `ea7d7e2` VSFTPD run #45358712640 failed later in the inline Python source contract. The failure was the generic global occurrence count for the new file paths, not the parser/test assertions: those paths appear in push+PR filters plus build/checker references. This checker-only commit moves both paths into the existing filter-specific validation tuple.
- Exact `ea7d7e2` AI handoff #45358712746 PASS. Process/NVRAM/XY-C1 runs were still pending at the last checkpoint; inspect their exact-head state independently.

## Verification boundary

- Exact `0104186915ee6132d99871bede241ecf1835c279`: FRP lifecycle #45358242268 PASS; Process inventory #45358242232 PASS.
- Exact `ea7d7e28be9988175fbcc423f9378e5d765de764`: AI handoff #45358712746 PASS; VSFTPD #45358712640 FAIL in static workflow path-count assertion after runtime policy compile/test. Process inventory #45358712716, NVRAM #45358712663 and XY-C1 #45358712652 were IN_PROGRESS at last poll.
- Exact AliDNS #45357686604 and earlier XY-C1 build/TRX/artifact validations are as above.
- No physical router L4 validation is claimed.

## Exact first action for the next GPT

1. Check the exact-head VSFTPD and AI handoff Actions for this path-filter checker repair. Fix any further source-contract failure based on its exact failing assertion.
2. Confirm Process, NVRAM and XY-C1 outcomes for product SHA `ea7d7e28be9988175fbcc423f9378e5d765de764`; only claim L3 if Build, TRX verification and artifact upload all pass on the same SHA.
3. Once this slice passes, continue the next open userland stability/security item; do not repeat completed FRP/AliDNS audits.
4. Keep the frozen backup immutable and only advance main through non-forced expected-SHA commits.

# Cross-session execution mandate

Treat this as continuous work across GPT conversations. Recover once, then execute on `main`; each logical source or CI change must land as one atomic main commit. Create all changed blobs, one complete Git tree and one commit before updating the branch with a non-forced expected-SHA compare-and-swap. Never force-push or modify the frozen backup.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST perform a handoff checkpoint before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

Every session MUST inspect live repository state before ending. Record only material exact-head Action transitions, new source commits, genuine unresolved failures or a changed first action. Do not manufacture documentation churn or no-op commits.

## Development loop

- Source modification → targeted regression → one complete Git tree/one atomic commit → exact-SHA Actions.
- PASS advances; FAIL is diagnosed and fixed; PENDING does not block independent work.
- Preserve Linux 4.4, GNU89, MIPS/uClibc compatibility, router configuration, security, architecture and backward compatibility.
- Before context exhaustion, record exact latest SHA, tests, CI state, unresolved issues and one concrete first task.
