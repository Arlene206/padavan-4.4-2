# 2026-11-18 continuation checkpoint — Internet-monitor control status reporting

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; develop on `main` only.
- Current source baseline before this atomic slice: `4989579534f712f54126629b2a3323ef50436ad9`; the preceding SCUT/MentoHUST source milestone is `777d291bfad084cb7d6c7a3a28e8ed5d609c5acb` (parent `aab4a9a3eccc2ceb25bd584a8b2fb16952656534`).
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains archival at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not check out, modify, merge, delete, rename or push it.
- PR #1 is closed and unmerged; do not reopen or use its feature branch.
- Recent relevant source commits: APCLI/WISP timeout correction `d486aba51fc1614a4e63836f1d192e2c7899ca24`, XUPnPD detached-child readiness `4388a1cca473a89255ba0ab938f19db450ee1ba5`, WAN sysctl propagation `49f1ab1d4c921306ed67a34c45cadd84a21f17ae`, and follow-ups `180ac810ea6d9c4551512ce7aa2d5bc599d60935` / `aab4a9a3eccc2ceb25bd584a8b2fb16952656534`.

## Active task
- `notify_run_detect_internet()` ignored the return status from `start_detect_internet()` and the SIGUSR1 `process_pidfile_signal()` request. `notify_pause_detect_internet()` likewise ignored the SIGUSR2 result, hiding launcher, PID-file and process-identity failures.
- This slice normalizes status/errno pairs in the existing detection-notification policy, logs failed launch/resume/pause operations, and treats ENOENT/ESRCH on pause as a benign no-monitor/race condition. Existing void APIs and caller ordering remain unchanged.
- The GNU89 host policy regression and detect-internet source contract cover result capture, normalization and the benign-pause boundary. The XY-C1 trigger watches the production policy header.

## Verification boundary
- Exact source baseline `4989579534f712f54126629b2a3323ef50436ad9`: Process lifecycle run `44256754447` PASS and AI handoff documentation run `44256754667` PASS, repairing the stale SCUT counter expectation and missing checkpoint wording from `777d291bfad084cb7d6c7a3a28e8ed5d609c5acb`.
- Source SHA `777d291bfad084cb7d6c7a3a28e8ed5d609c5acb`: Process inventory `44256159035` and Legacy wrapper pidfile `44256158977` PASS. XY-C1 run `44256159232` was still IN_PROGRESS at the last poll; Build was running while TRX and artifact remained PENDING. Do not claim L3 until those steps pass on this production-source SHA.
- The Internet-monitor control-status slice is included in this commit tree; its detect-internet host regression and XY-C1 run require exact-head verification after publication. A passing host test does not establish L3.
- No local shell run or cross-build is claimed in this connector-only environment. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch live `main`; query exact-head Actions for the detect-internet signal regression, AI handoff docs and XY-C1 validation. Check Build, TRX and artifact results for production-source SHA `777d291bfad084cb7d6c7a3a28e8ed5d609c5acb` and for the new current SHA separately.
2. If an exact-head check fails, inspect its first causal job log and repair on current main with a complete tree/commit and expected-SHA non-force ref update. If pending, continue independent work.
3. Continue the P1 whole-userland startup/lifecycle audit after this notification-result slice. Do not repeat APCLI/WISP, XUPnPD, sysctl, SCUT/MentoHUST, Adbyby or ttyd source reviews without a new exact-head failure.
4. Preserve the frozen backup and keep physical-router L4 marked NOT VERIFIED.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional. Every session must inspect live main, exact-head Actions and the first next task before ending. If these facts and the first action remain accurate, leave the handoff untouched; do not create a no-op commit.

# Cross-session execution mandate
Before context exhaustion, leave a precise continuation checkpoint. Deliver one atomic main commit per logical source/test milestone, verify exact-SHA Actions, then continue to the next independent ranked defect. Never force-push, mutate the backup, reopen the retired PR, or claim unperformed tests/build/hardware checks.
