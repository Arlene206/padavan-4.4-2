# 2026-11-18 continuation checkpoint — SCUT/MentoHUST detached startup readiness and exact-head CI repair

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; develop on `main` only.
- Source milestone commit: `777d291bfad084cb7d6c7a3a28e8ed5d609c5acb` (parent `aab4a9a3eccc2ceb25bd584a8b2fb16952656534`).
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains archival at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not check out, modify, merge, delete, rename or push it.
- PR #1 is closed and unmerged; do not reopen or use its feature branch.
- Recent relevant source commits: APCLI/WISP timeout correction `d486aba51fc1614a4e63836f1d192e2c7899ca24`, XUPnPD detached-child readiness `4388a1cca473a89255ba0ab938f19db450ee1ba5`, WAN sysctl propagation `49f1ab1d4c921306ed67a34c45cadd84a21f17ae`, and follow-ups `180ac810ea6d9c4551512ce7aa2d5bc599d60935` / `aab4a9a3eccc2ceb25bd584a8b2fb16952656534`.

## Active task
- Shared `trunk/user/shared/scripts/process_lifecycle.sh` now defines `pl_wait_started(NAME, INITIAL_WAIT, PROBES, RETRY_WAIT)`: 0 only for confirmed RUNNING; 1 for bounded confirmed ABSENT; 2 for UNKNOWN, invalid arguments or wait failure.
- `scutclient.sh` and `mentohust.sh` now require a bounded post-launch RUNNING observation after `start-stop-daemon -S -b` before reporting startup success. The original nonzero launcher result is preserved, and no signal/cleanup is attempted when inventory is UNKNOWN.
- The shared shell regression covers immediate/delayed RUNNING, ABSENT timeout, UNKNOWN, invalid arguments and interrupted sleep. `scripts/check_optional_app_start_status.py` exercises both clients end-to-end with deterministic stubs.
- The helper is installed into ROMFS, so `.github/workflows/xy-c1-firmware-validation.yml` and `scripts/tests/test_xy_c1_firmware_inputs.py` now include it as a firmware production input.

## Verification boundary
- Parent `66d64ca44f939851f24c68ba17d864c1b7b38db9`: Process lifecycle run `44253310572` PASS; AI handoff run `44253310676` PASS.
- Parent `180ac810ea6d9c4551512ce7aa2d5bc599d60935`: WAN route run `44254662647` PASS; AI handoff run `44254662816` PASS.
- Exact-head SHA `777d291bfad084cb7d6c7a3a28e8ed5d609c5acb`: Process workflow `44256159139` FAILED only at the optional-client checker because its final SCUT logout count still expected one after the newly added delayed-start/stop test correctly performs a second logout. The Process inventory workflow `44256159035` and Legacy wrapper pidfile workflow `44256158977` PASS.
- AI handoff workflow `44256158997` FAILED because this handoff lacked the protocol-required conditional-document-edit sentence and exact section names. This repair adds those contract strings and corrects the one stale SCUT test expectation.
- XY-C1 run `44256159232` is `in_progress` at last poll; Build XY-C1, TRX verification and artifact upload have not yet been confirmed. Verify the run on the same production source SHA; do not claim L3 while any of those stages is pending.
- No local shell run or cross-build is claimed in this connector-only environment. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch live `main`; query its exact-head push Actions for Process lifecycle, AI handoff docs and XY-C1. Inspect failed job logs. Require Build XY-C1, TRX verification and artifact upload on the same SHA for L3.
2. Confirm this repair SHA's Process optional-app checker and AI handoff contract are PASS. If either fails, fetch its log and repair the causal assertion/doc contract on current main with a complete Git tree/commit and expected-SHA non-force ref update. If pending, work independently.
3. Continue the P1 whole-userland service-start audit after this CI repair: look for enabled detached/background fork paths that return success without a bounded child-presence check. Do not repeat XUPnPD, APCLI/WISP, SCUT, MentoHUST, Adbyby or ttyd absent a fresh exact-head failure.
4. Preserve the frozen backup and keep physical-router L4 marked NOT VERIFIED.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional. Every session must inspect live main, exact-head Actions and the first next task before ending. If these facts and the first action remain accurate, leave the handoff untouched; do not create a no-op commit.

# Cross-session execution mandate
Before context exhaustion, leave a precise continuation checkpoint. Deliver one atomic main commit per logical source/test milestone, verify exact-SHA Actions, then continue to the next independent ranked defect. Never force-push, mutate the backup, reopen the retired PR, or claim unperformed tests/build/hardware checks.
