## Current repository state

- Live main before current AliDNS checker argument repair: 824f2b4fcc2365af7502ad833a1ad0cacec1e5a4.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- VSFTPD REST strict 1–15 ASCII-digit parser and regression: `b87b1c10f3808f8bd4f47e374fdc5b0472bdfb94`; checker binding repair: `21699dc5a854afb6aea0121a64421bb836aa447c`. Exact VSFTPD #45356072893, AI #45356072970, Process #45355332631, AI #45355332668, NVRAM #45355332714 and XY-C1 firmware validation #45355332727 all PASS.
- AliDNS POSIX byte encoder and /bin/sh fixture: `ca9267b3c71a9fb2a8de96e82a2878fcf31e5dc6`. Exact e4 SHA #45357277912 passed every functional test but failed the static checker because it passed only one argument to `require(condition,message)`. Checker repair `824f2b4fcc2365af7502ad833a1ad0cacec1e5a4` now separates the boolean predicate from its message with a comma and uses regex command-token detection. Exact 824 AliDNS #45357686604 is running.
- Main stays active; frozen backup is not modified.

## Active task

Verify the AliDNS POSIX encoder contract:
- The production encoder/tests are unchanged from `ca9267`; local checks ran under POSIX `/bin/sh` and BusyBox ash.
- Exact `e4fdbc7`: all functional test steps PASS; static contract failed with `TypeError: require() missing 1 required positional argument: 'message'`.
- Exact `824f2b4`: current static checker repair should fix this, but AliDNS Action #45357686604 is still in progress at last observation.

## Verification boundary

- Exact `e4fdbc7a3a27f6d7a4d77725841242acc8f2e4d3`: AliDNS functional tests PASS, static contract FAIL due missing require message; AI handoff #45357277937 PASS.
- Exact `b87b1c10f3808f8bd4f47e374fdc5b0472bdfb94`: VSFTPD REST C parser, Process, AI, NVRAM and XY-C1 firmware workflow PASS.
- Exact `21699dc5a854afb6aea0121a64421bb836aa447c`: VSFTPD and AI handoff PASS.
- No physical router validation is claimed.

## Exact first action for the next GPT

1. Check exact 824 AliDNS run and, if necessary, inspect the source checker failure.
2. Harden VSFTPD SITE UMASK/SITE CHMOD input: reject malformed, non-octal and out-of-range values before changing umask or opening/chmodding a file.
3. Preserve frozen backup and keep exact SHA/Actions outcomes in the handoff.

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
