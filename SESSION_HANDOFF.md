# Cross-session execution mandate

Mandatory handoff checkpoint: The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live repository state and compare it with SESSION_HANDOFF.md before ending or handing off. Verify source baseline, active task, exact-head Actions, validation boundary, blockers and precise first action. If the checkpoint remains accurate, do not make a documentation-only commit.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup.

Before context exhaustion, finish the current safe slice, check exact-head Actions and preserve the precise first next action here.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; product development remains on `main` only.
- This storage retry fix is one atomic commit parented at `7b8b3129261fc846cb972cee4fdc73dfca0344bd`; fetch live `main` after publication for this commit's exact resulting SHA.
- Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only, never modify or delete.
- PR #1 (`fix/aliddns-ca-bundle-bootstrap`) is closed and unmerged, 17 commits ahead of its base. Current main has independently changed the CA bootstrap implementation; the PR branch is archival, not a development base.

## Active task — USB storage integrated-service startup retry

- `start_stor_apps()` returns the first required failure while continuing independent FTP/Samba/NFS and optional media starts. Its duplicate declaration defects were fixed in `0111177585699cfacbf7878d1554eeb6bc861537`; exact-head XY-C1 run `43888486194` passed Build, TRX verification and artifact upload on that then-current source SHA. Subsequent source changes mean this result does not establish L3 for the current code.
- Remaining defect fixed here: `try_start_stor_apps()` formerly cleared `usb_hotplug_ms` even when the integrated daemon startup result was nonzero. The deferred callback then scheduled `alarm(5)` but had already discarded its retry event. The new path clears the flag only on status 0 and keeps failure pending.
- The Optware child result is consumed once before retrying integrated daemons. If the asynchronous child is still running, the callback returns before restarting FTP/Samba/NFS/media daemons; the Optware script is still launched only after releasing `storage.lock`.
- Regression coverage: `trunk/user/rc/src/tests/service_start_status.c`, `trunk/user/rc/src/service_start_status.h`, and `.github/workflows/storage-hotplug-stability.yml`.

## Verification boundary and unresolved failures

- Local host service-start policy regression compiled with GNU89, `-Wall -Wextra -Werror`, AddressSanitizer and UndefinedBehaviorSanitizer: PASS, including positive/negative failure retention.
- Latest parent exact-head Actions on `7b8b3129261fc846cb972cee4fdc73dfca0344bd`: AI handoff docs `43890908405` PASS; Process inventory/lifecycle fail-closed `43890908528` PASS. This parent did not rerun storage or XY-C1 for the final retry fix.
- Last exact-head storage run before this fix: `72064974e7b099112e587d1c5aad1c410c2b3789`, run `43890786101` PASS. It covers the prior media start/stop API, not this new retry gate.
- XY-C1 run `43890644424` on source commit `a706e2470bbb381cbd8acd2afb150db30e069839` was still IN_PROGRESS at the last query. The prior syntax failure on `847089c17ac34880436527384f73f60d7b7b74e6` (run `43887049459`) was diagnosed and repaired in `0111177`; do not extend the older `0111177` build success to later source commits.
- WAN route status run `43890645171` on `a706e247` FAILED its source-verification step with an AssertionError in the embedded checker. Process-lifecycle regression run `43890644968` also FAILED with an IndexError in its source checker; checker edits were made in `7b8b312`, but an exact-head rerun of that specific workflow is still needed. Treat these as open CI blockers, not source PASS.
- No same-SHA current MIPS firmware L3 evidence yet; physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Query push-triggered Actions for the exact SHA now at `main`. Inspect the storage workflow added by this commit, XY-C1 Build/TRX/artifact stages, and process-lifecycle regression. Repair first causal failures without weakening contracts.
2. Re-run WAN route status on the current SHA and isolate the AssertionError from run `43890645171`; also confirm the process-lifecycle checker IndexError is resolved after `7b8b312`.
3. Require same-SHA XY-C1 Build, TRX verification and artifact upload before claiming firmware L3. Physical-router L4 remains NOT VERIFIED.
4. Then continue the P1 cross-userland RC startup-result audit, preserving process identity, lock ordering and first-error semantics.
