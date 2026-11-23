# SESSION HANDOFF — Padavan 4.4

## Current repository state

- Development branch: `main` only. Do not create feature branches or PRs; never force-push.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Preserve it permanently; do not modify, rename, delete or check it out.
- Current source checkpoint: THIS COMMIT (trigger XY-C1 on MTK ESW kernel changes); parent main SHA: `a3a7d64ba1fbaab7cc6f6d74d9c9650c017242ee`. Fetch live `main` on takeover; do not chase a docs-only SHA.
- Continue implementation rather than repeating completed audits or asking what to do next.

## Active task

The MTK ESW link-mode validation is in parent `a905ff6c262f066cbba6c6cb2c704dee01cd3d9f`. Its Switch CLI host regression passed, but the XY-C1 workflow did not trigger on that driver change because `.github/workflows/xy-c1-firmware-validation.yml` omitted `ioctl_mt762x.c`, `link_mode_policy.h`, and the new policy test from its push-path filter. A descendant commit `a3a7d64ba1fbaab7cc6f6d74d9c9650c017242ee` happened to trigger XY-C1 through an unrelated Samba change and includes the MTK code, but that accidental trigger is not reliable coverage. This commit adds the three paths and makes the Switch CLI contract assert the firmware trigger allowlist.
## Verification boundary

Parent source SHA `a3a7d64ba1fbaab7cc6f6d74d9c9650c017242ee`:
- Previous MTK ESW source SHA `a905ff6c262f066cbba6c6cb2c704dee01cd3d9f`: Switch CLI `45091472374` PASS, AI docs `45091472379` PASS; NVRAM `45091472420` was cancelled after another main update.
- Descendant `a3a7d64ba1fbaab7cc6f6d74d9c9650c017242ee` triggered XY-C1 run `45091634154` through an unrelated Samba source change; at last poll the workflow job was IN_PROGRESS with Build XY-C1 still PENDING. The build includes the MTK code but is not proof that MTK changes themselves trigger the workflow.
- Descendant NVRAM run `45091634173` was IN_PROGRESS; other host checks were queued/running.

THIS COMMIT changes the XY-C1 workflow itself, which is in its push allowlist; inspect its new exact-head build and ensure Build, TRX and artifact steps execute.

## Exact first action for the next GPT

1. Fetch live `main` once and verify frozen backup `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not touch it.
2. Inspect current exact-head Process inventory run `45091972548` and remaining host regressions on `4ecc28778b3605743262514dbce53b7ffbebda2d`; this parent adds Samba checker whitespace normalization and miniupnpd LAN identity snapshots. Repair first causal failures only.
3. Confirm AI handoff docs pass after the mandatory checkpoint section in this commit. Previous exact-head run `45091738990` failed solely because that required section/sentence was absent.
4. Switch CLI `45091738973` passed on `7dd05bff`. XY-C1 `45091739058` completed successfully but Build/TRX/artifact were SKIPPED because `7dd05bff` changed only workflow/docs; the MTK kernel path filter is corrected but its next relevant-source exact-head Build/TRX/artifact must be observed.
5. Continue independent concrete stability work while checks are pending; keep complete-tree atomic commits on fresh main with `force:false` and exact `expected_sha`.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

At the end of every session, inspect the live repository state and compare it with this handoff. Verify the current main SHA, frozen backup SHA, engineering baseline, active task, relevant exact-head Actions results (including transitions from PENDING to PASS/FAIL), validation boundaries, unresolved blockers, and the exact first action for the next GPT. Update this handoff and dev log when those facts materially change. If the facts and first action remain accurate, leave the files untouched and do not create a no-op commit.

## End-of-session accuracy checkpoint

- Current implementation checkpoint: THIS COMMIT; parent main SHA `4ecc28778b3605743262514dbce53b7ffbebda2d`.
- Exact-head Switch CLI `45091738973` on `7dd05bff`: PASS, including MTK ESW GNU89/ASan/UBSan policy test and source-order checks.
- Exact-head XY-C1 `45091739058` on `7dd05bff`: workflow SUCCESS; Build XY-C1, TRX verification and artifact upload were SKIPPED because the commit changed only workflow/docs. It is not L3 validation.
- Exact-head AI docs `45091738990` on `7dd05bff`: FAIL solely due to missing mandatory handoff checkpoint section/sentence; this commit restores them.
- Exact-head Process lifecycle regression `45091738946` on `7dd05bff`: FAILED at a Samba multiline-call source assertion. Concurrent `b803d7a5` normalized that whitespace; current parent `4ecc2877` additionally adds checked miniupnpd LAN identity snapshots and regression coverage. Its Process inventory/other exact-head checks are in flight.
- The MTK ESW XY-C1 push filter now includes driver/header/test. Same-SHA kernel Build/TRX/artifact and hardware L4 remain NOT VERIFIED.
