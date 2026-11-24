## Current repository state

- Active development branch: `main`. Main SHA before this handoff commit: `9b97f74377509e2740442475b210bf9921b04915`; re-fetch live main before any write.
- Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Do not modify, use for development, or delete it.
- No open PRs were found. Legacy PR #1 is closed; its valid unmerged CA bootstrap and AliDDNS HMAC work were ported to main and documented in its closing comment.
- Active product-source baseline for the current LLDT work: `529cf0739b3089bca88936c4c06c75cbd3eb79da` (`fix(lldt): validate pidfile ownership and link count`). Follow-up commits `8796b356`, `2fdf8e7a`, and `9b97f743` fixed the GNU89 fixture include and the dedicated LLDT source-contract assertions; no later firmware-source changes were made in those follow-ups.

## Active task — Complete LLDT PID-file descriptor validation integration

- `trunk/user/lldt/src/osl-linux.c` validates the sidecar lock descriptor before flock and validates the existing/read and final/write PID descriptors before reading or truncating.
- `trunk/user/lldt/src/lldt_pidfile_policy.h` centralizes the policy: opened object must be regular, owned by effective UID, have exactly one hard link, and not be group/world writable. `O_NOFOLLOW` and `O_NONBLOCK` remain in the opens.
- `trunk/user/lldt/src/tests/lldt_pidfile_policy.c` covers regular-file success, directory, wrong owner, hardlink, group/world-write and NULL metadata. The fixture originally missed `<string.h>`; commit `8796b356` added it.
- Current bounded correction changes only `scripts/check_process_lifecycle_integration.py`: replace the obsolete inline-`S_ISREG` count with checks that the shared descriptor validator is called at the lock and PID-file boundaries, that calls precede flock/read/truncate, and that the helper contains the required metadata policy.
- Dedicated LLDT workflow exact run #45375259525 on `9b97f74377509e2740442475b210bf9921b04915` is **PASS**: GNU89 compile, parser/metadata regression and dedicated production source checker all succeeded.
- Process lifecycle integration exact run #45374673151 on `8796b356087fe77920a90e43f2889e86a6096013` failed only at `check_process_lifecycle_integration.py` because it still counted two inline `S_ISREG(st.st_mode)` checks. The LLTD parser regression step passed; the new checker change addresses that stale contract. Exact follow-up verification is pending.
- Earlier exact LLDT failures: #45374510284 on `529cf073` failed because the stat fixture lacked `<string.h>` (fixed in `8796b356`); #45374673177 on `8796b356` failed because the dedicated checker expected inline owner checks (fixed in `2fdf8e7a`); #45374955437 on `2fdf8e7a` failed due LLDT/LLTD spelling mismatch (fixed in `9b97f743`).
- XY-C1 firmware validation #45374510309 is for production-source SHA `529cf0739b3089bca88936c4c06c75cbd3eb79da`. At last inspection, host checks had passed and Build XY-C1 was still running; TRX verification and artifact upload were pending. Do not claim L3 unless Build, TRX and artifact all pass for this same source SHA. No physical L4 validation is claimed.

## Verification boundary

- L1: descriptor validation call ordering and shared helper semantics are enforced by source contracts; no local repository checkout was available in this runtime.
- L2: dedicated LLDT GNU89 regression and checker passed on exact main SHA `9b97f74377509e2740442475b210bf9921b04915` (#45375259525). Process lifecycle integration remains **PENDING** exact rerun of the checker correction introduced in this commit.
- L3: XY-C1 run #45374510309 on source SHA `529cf0739b3089bca88936c4c06c75cbd3eb79da` was **IN_PROGRESS** at last observation; TRX and artifact not yet confirmed.
- L4: NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch Actions for the exact new main SHA from this commit. Inspect Process inventory/lifecycle and AI handoff jobs by matching `head_sha` and `event=push`; run #45374673151 is the known stale-checker failure being fixed here. Repair any newly exposed assertion without weakening production checks.
2. Refresh XY-C1 run #45374510309 and inspect Build, TRX and artifact status for its exact product-source SHA `529cf0739b3089bca88936c4c06c75cbd3eb79da`. If still pending, do not wait idly.
3. After the LLDT slice is validated, select a fresh P1 stability/security defect from the repository-wide roadmap outside LLDT, and continue the source → regression → atomic main commit → exact-SHA Actions loop.

# Cross-session execution mandate

Recover once from the live repository. All development, commits and validation occur on `main`; never create/use another development branch or PR. The backup branch above is the sole immutable archival exception.

For every multi-file logical slice, create all blobs, one complete tree based on the latest main tree, one commit with the exact main parent, then update `main` using a non-forced compare-and-swap with `expected_sha`. This is one atomic main commit; never issue sequential partial file commits, force-push, amend, or rewrite published history.

Continue implementation rather than repeating completed audits or stopping at planning. Pending Actions do not block independent tasks. Keep Linux 4.4, GNU89, MIPS/uClibc and router compatibility. Do not weaken regression coverage or claim host tests as target/hardware validation.

At every session end, inspect current main and exact-head push Actions. Update this handoff only when the active task or its first action materially changes; append substantive milestones to `dev_log.md`; update `REFACTOR_ROADMAP.md` only when module evidence/status/priority materially changes. If checkpoint facts remain accurate, make no no-op documentation commit. Before ending, finish a safe slice, commit it, record pending checks and the exact next action.
