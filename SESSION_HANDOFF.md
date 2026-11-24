## Current repository state

- Active branch: `main`. Main SHA before this checkpoint commit: `72e1e72dd494e6d930bdfef46f349f20da7065be`; re-fetch live main before every write.
- Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`; never modify, use for development, or delete it.
- No open PRs were found. Legacy PR #1 is closed; its valid CA trust-store bootstrap and AliDDNS HMAC work were ported to main.
- Latest product-source commit: `72e1e72dd494e6d930bdfef46f349f20da7065be` (`fix(mtk): validate 802.11r boolean ioctl copies`). It is the current firmware-input baseline and includes the LLDT PID descriptor policy.
- LLDT production fix: `529cf0739b3089bca88936c4c06c75cbd3eb79da`. Dedicated checker follow-ups: `8796b356`, `2fdf8e7a`, `9b97f743`. Process integration checker alignment: `be37029341b79273e520fda75f3b087b41757372`.

## Active task — Verify LLDT integration and current MTK kernel fix

- MTK AP 802.11r: MT76x2, MT76x3, MT7615 and MT7915 handlers for SUPPORT/RIC/OTD now copy into `BOOLEAN`, enforce exact input length, check `copy_from_user()`, return `-EFAULT` on failure, and update flags only after a successful copy. Focused regression exact run #45375797791 on `72e1e72` is **PASS**.
- Same-SHA XY-C1 #45375799312 and NVRAM #45375798550 were in progress at last check. Process lifecycle #45375797796 on the same SHA was cancelled before LLTD PID parser and integration-checker steps; it is not a PASS.
- LLDT sidecar lock and PID descriptors are validated as regular, effective-UID-owned, single-link and not group/world writable before flock/read/truncate. The shared helper is `lldt_pidfile_validate_regular_file()`; opens retain `O_NOFOLLOW` and `O_NONBLOCK`. Tests cover directories, wrong owner, hardlinks, unsafe modes and NULL metadata. Dedicated LLDT exact run #45375259525 on `9b97f74377509e2740442475b210bf9921b04915` passed.
- Process checker correction `be370293` replaces obsolete inline-`S_ISREG` counting with checks for both PID-descriptor validations before read/truncate, lock validation before flock, and the helper's metadata policy. Exact Process rerun is not verified: #45375797796 was cancelled before relevant steps; rerun attempt 2 of #45375705202 on `be370293` was still in progress at last inspection.
- AI handoff run #45375798381 on `72e1e72` failed because this file omitted the required mandatory checkpoint section. This commit restores that exact section; its exact-SHA rerun is pending.
- Earlier failures were isolated and corrected: #45374510284 lacked `<string.h>` in the test fixture (fixed in `8796b356`); #45374673177 had a stale dedicated owner-check assertion (fixed in `2fdf8e7a`); #45374955437 had an LLDT/LLTD output-marker typo (fixed in `9b97f743`).

## Verification boundary

- L1: source contracts cover descriptor validation ordering and metadata policy. No local repository checkout was available in this runtime; no separate local execution is claimed.
- L2: dedicated LLDT GNU89 parser/metadata regression and source checker PASS #45375259525 on exact SHA `9b97f74377509e2740442475b210bf9921b04915`; focused MTK ioctl regression PASS #45375797791 on exact SHA `72e1e72dd494e6d930bdfef46f349f20da7065be`. Process integration still needs a completed exact run of the corrected checker.
- L3: XY-C1 #45375799312 on production SHA `72e1e72dd494e6d930bdfef46f349f20da7065be` was **IN_PROGRESS** at last check; require Build, TRX and artifact success on that same SHA. NVRAM #45375798550 was also **IN_PROGRESS**.
- L4: NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch exact-head Actions for this checkpoint commit and confirm AI handoff documentation passes. Resolve any new validator assertion without removing mandatory checkpoint language.
2. Refresh Process #45375797796 and rerun attempt 2 of #45375705202. The cancelled run skipped LLTD PID parser/integration steps; require a completed Process PASS on a SHA containing `be370293`.
3. Refresh XY-C1 #45375799312 and NVRAM #45375798550 for exact source SHA `72e1e72dd494e6d930bdfef46f349f20da7065be`. Do not claim L3 unless Build, TRX and artifact all pass on that same SHA. Pending jobs do not block independent work.
4. After these checks are closed or running independently, select the next P1 stability/security defect outside LLDT.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state before ending. Record only material exact-head Action transitions, new source commits, genuine unresolved failures or a changed first action. Do not manufacture documentation churn or no-op commits.

A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

# Cross-session execution mandate

Recover once from the live repository, then execute on `main`. Never create or use a development branch or PR; the frozen backup above is immutable.

For every multi-file logical slice, use Git data APIs to create all blobs, one complete tree based on the latest main tree, and one commit with the exact main parent. Update `main` using a non-forced compare-and-swap with `expected_sha`. This is one atomic main commit; never issue sequential partial file commits, force-push, amend, or rewrite published history.

Continue source fixes and regression work rather than repeating completed audits or stopping at planning. PASS advances, FAIL is investigated and fixed, PENDING does not block independent work. Preserve Linux 4.4, GNU89, MIPS/uClibc, router configuration, security and compatibility.

Before context exhaustion, finish the current safe slice and record exact latest main SHA, tests, CI states, unresolved issues and one concrete first action for the next GPT. At session end, inspect live main and exact-head push Actions; update this handoff only when active task or first action materially changes, append substantive milestones to `dev_log.md`, and update the roadmap only for material module/status/priority changes. Do not create no-op documentation commits.
