# 2026-11-18 continuation checkpoint — SCUT/MentoHUST detached startup readiness

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; work on `main` only.
- Parent main for this atomic source/test checkpoint: `aab4a9a3eccc2ceb25bd584a8b2fb16952656534`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains archival at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not check out, modify, merge, delete, rename or push it.
- PR #1 is closed and unmerged; do not reopen or use its feature branch.
- Recent source slices include APCLI/WISP timeout handling (`d486aba51fc1614a4e63836f1d192e2c7899ca24`), XUPnPD detached-child readiness (`4388a1cca473a89255ba0ab938f19db450ee1ba5`), WAN sysctl propagation (`49f1ab1d4c921306ed67a34c45cadd84a21f17ae`) and its follow-ups (`180ac810ea6d9c4551512ce7aa2d5bc599d60935`, `aab4a9a3eccc2ceb25bd584a8b2fb16952656534`).

## Active source slice
- Shared `process_lifecycle.sh` gains `pl_wait_started(NAME, INITIAL_WAIT, PROBES, RETRY_WAIT)`: 0 only for confirmed RUNNING; 1 for bounded confirmed ABSENT; 2 for UNKNOWN, invalid arguments or wait failure.
- SCUT and MentoHUST scripts require a post-launch RUNNING observation after `start-stop-daemon -S -b` before returning success. Original launcher failures remain propagated; no signal is sent on UNKNOWN.
- Shared shell regression and `scripts/check_optional_app_start_status.py` cover absent, delayed and UNKNOWN detached children, sleep failure and invalid arguments.
- Because the shared helper is installed into ROMFS, XY-C1 path filters and firmware-input tests include it as a production input. The Process regression workflow already covers `trunk/user/**` and the optional-app checker.

## Verification boundary
- Parent `66d64ca44f939851f24c68ba17d864c1b7b38db9`: Process run `44253310572` PASS; AI handoff run `44253310676` PASS.
- Parent `180ac810ea6d9c4551512ce7aa2d5bc599d60935`: WAN route run `44254662647` PASS; AI handoff run `44254662816` PASS.
- XY-C1 run `44254415889` was building production SHA `49f1ab1d4c921306ed67a34c45cadd84a21f17ae`; Build was IN_PROGRESS and TRX/artifact were PENDING at last poll. The shared-helper source change should trigger a fresh XY-C1 validation.
- This checkpoint's Process, AI handoff and XY-C1 Actions must be queried by exact `head_sha`. Require Build, TRX validation and artifact upload to pass on the same SHA before claiming L3. No local shell execution or cross-build is claimed; physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch live `main`; inspect exact-head push Actions for Process lifecycle regression, AI handoff documentation and XY-C1 validation. Require Build, TRX and artifact upload on that same SHA.
2. If an exact-head check fails, fetch its first causal job log and repair on latest main with a complete Git tree/commit and expected-SHA non-force ref update. If pending, continue independent work.
3. After verification, continue the P1 whole-userland detached-start audit. Find enabled `start-stop-daemon -b` or equivalent fork paths that report success without checking child presence. Do not repeat XUPnPD, APCLI/WISP, SCUT, MentoHUST, Adbyby or ttyd unless a new exact-head failure requires it.
4. Preserve the frozen backup and keep physical-router L4 marked NOT VERIFIED.

## Mandatory handoff checkpoint
Inspect live main and exact-SHA Actions before ending. Update this file only when active task, verification gate or next first action materially changes; do not create SHA-only documentation commits.

## Cross-session execution mandate
One logical source/test/workflow milestone per atomic main commit; exact-SHA Actions; then next ranked defect. No force-push, development branch, PR development, backup mutation or unsupported test/build/hardware claims.
