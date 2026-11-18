# 2026-11-18 continuation checkpoint — Internet-monitor control status reporting

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; develop on `main` only.
- Source implementation under verification: `8c272644dfc86a047eed50dd48aee42400607b72` (parent `4989579534f712f54126629b2a3323ef50436ad9`); checker repair is being prepared on that exact main head. The SCUT/MentoHUST source milestone is `777d291bfad084cb7d6c7a3a28e8ed5d609c5acb`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains archival at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not check out, modify, merge, delete, rename or push it.
- PR #1 is closed and unmerged; do not reopen or use its feature branch.
- Recent relevant source commits: APCLI/WISP timeout correction `d486aba51fc1614a4e63836f1d192e2c7899ca24`, XUPnPD detached-child readiness `4388a1cca473a89255ba0ab938f19db450ee1ba5`, WAN sysctl propagation `49f1ab1d4c921306ed67a34c45cadd84a21f17ae`, and follow-ups `180ac810ea6d9c4551512ce7aa2d5bc599d60935` / `aab4a9a3eccc2ceb25bd584a8b2fb16952656534`.

## Active task
- `notify_run_detect_internet()` ignored the return status from `start_detect_internet()` and the SIGUSR1 `process_pidfile_signal()` request. `notify_pause_detect_internet()` likewise ignored the SIGUSR2 result, hiding launcher, PID-file and process-identity failures.
- This slice normalizes status/errno pairs in the existing detection-notification policy, logs failed launch/resume/pause operations, and treats ENOENT/ESRCH on pause as a benign no-monitor/race condition. Existing void APIs and caller ordering remain unchanged.
- The GNU89 host policy regression and detect-internet source contract cover result capture, normalization and the benign-pause boundary. The XY-C1 trigger watches the production policy header.

## Verification boundary
- Exact-head source SHA `8c272644dfc86a047eed50dd48aee42400607b72`: Process inventory `44258150914` PASS; AI handoff docs `44258150879` PASS; NVRAM transaction boundary `44258150848` PASS. Process lifecycle `44258150936` FAILED only at an old checker that expected one-line SIGUSR1/SIGUSR2 source strings; detect-internet signal regression `44258150955` passed the GNU89/ASAN/UBSAN policy executable and failed at a brittle whitespace-sensitive assertion for the multiline error-normalizer call. This follow-up normalizes C whitespace in both checkers.
- Exact-head XY-C1 `44258150922` was still IN_PROGRESS at last poll; Build was running and TRX/artifact remained pending. Recheck exact-SHA status after the checker repair. L3 is not yet claimed for 8c.
- Source SHA `777d291bfad084cb7d6c7a3a28e8ed5d609c5acb`: XY-C1 `44256159232` has since completed SUCCESS. Cross-build, TRX verification and artifact upload all completed successfully on that same SHA; L3 is verified for the SCUT/MentoHUST product-source slice.
- No local shell run or independent cross-build is claimed beyond the cited Actions. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch live `main`; query exact-head Process lifecycle, detect-internet signal stability, AI handoff and XY-C1 Actions for the checker-repair SHA. Inspect causal logs if either checker still fails.
2. Recheck XY-C1 run `44258150922` for the source SHA `8c272644dfc86a047eed50dd48aee42400607b72`; Build, TRX and artifact must all pass on that same SHA for L3.
3. Once CI is green or remains pending, continue the P1 whole-userland service-start and error-propagation audit; do not repeat APCLI/WISP, XUPnPD, WAN sysctl, SCUT/MentoHUST, Adbyby or ttyd absent a fresh exact-head regression.
4. Preserve the frozen backup and keep physical-router L4 marked NOT VERIFIED.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional. Every session must inspect live main, exact-head Actions and the first next task before ending. If these facts and the first action remain accurate, leave the handoff untouched; do not create a no-op commit.

# Cross-session execution mandate
Before context exhaustion, leave a precise continuation checkpoint. Deliver one atomic main commit per logical source/test milestone, verify exact-SHA Actions, then continue to the next independent ranked defect. Never force-push, mutate the backup, reopen the retired PR, or claim unperformed tests/build/hardware checks.
