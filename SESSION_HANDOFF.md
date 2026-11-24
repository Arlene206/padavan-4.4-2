## Current repository state

- Live main before the current AliDNS checker repair: e4fdbc7a3a27f6d7a4d77725841242acc8f2e4d3.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- VSFTPD strict REST 1–15 ASCII-digit parser and regression: `b87b1c10f3808f8bd4f47e374fdc5b0472bdfb94`; checker binding repair: `21699dc5a854afb6aea0121a64421bb836aa447c`. Exact VSFTPD #45356072893 and AI contract #45356072970 PASS; exact b87 Process #45355332631, AI #45355332668, NVRAM #45355332714 and XY-C1 firmware validation #45355332727 all PASS.
- AliDNS POSIX byte encoder and /bin/sh regression: `ca9267b3c71a9fb2a8de96e82a2878fcf31e5dc6`. Functional steps pass; checker-only fixes are `3151906ee28c42f38a8a283e4f94768cc31126bb`, `32263cbbfe129697874d0f70a2760f989bbdfa8c`, and `e4fdbc7a3a27f6d7a4d77725841242acc8f2e4d3`. Exact e4 run #45357277912 failed only because the new `require()` call omitted the message argument. Current change fixes the call and the standalone-`od` token check; exact SHA pending.
- Exact e4 AI handoff contract #45357277937 PASS. Backup remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## Active task

Finish AliDNS POSIX encoder static contract:
- Exact `e4fdbc7a3a27f6d7a4d77725841242acc8f2e4d3`: all executable shell/API/lifecycle/HMAC privacy test steps PASS. Static contract failed with `TypeError: require() missing 1 required positional argument: 'message'`, because the standalone-`od` predicate was accidentally joined into the boolean expression instead of being comma-separated from its diagnostic.
- Added the missing comma and a Python regex token-boundary matcher, which detects a standalone `od` command without matching the substring in `encode`. Production encoder and shell test remain unchanged.
- The POSIX encoder was exercised locally under `/bin/sh` and BusyBox ash for reserved punctuation, quote/backslash, spaces, UTF-8, controls, empty/multiple newlines and unterminated input.

## Verification boundary

- Exact `e4fdbc7a3a27f6d7a4d77725841242acc8f2e4d3`: AliDNS functional steps PASS; static contract FAIL only on missing `require()` diagnostic argument. AI handoff #45357277937 PASS.
- Exact `b87b1c10f3808f8bd4f47e374fdc5b0472bdfb94`: C REST policy compile/tests, Process, AI handoff, NVRAM and XY-C1 firmware validation PASS.
- Exact `21699dc5a854afb6aea0121a64421bb836aa447c`: VSFTPD #45356072893 and AI handoff #45356072970 PASS.
- No physical router L4 validation is claimed.

## Exact first action for the next GPT

1. Verify exact-head AliDNS shell regression and static source checker after the require()/regex repair.
2. Investigate SITE UMASK and SITE CHMOD octal parsing: both handlers use permissive `str_octal_to_uint()` without validating syntax or command-specific range. Prevent malformed UMASK input from silently setting an overly permissive zero mask.
3. Keep backup immutable; log exact SHA/CI after each atomic commit.

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
