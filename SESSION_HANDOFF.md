# SESSION HANDOFF — Padavan 4.4

## 2026-11-23 libdisk folder-metadata transaction checkpoint

- Current `main` before this documentation commit: `7bd561845032ed68084e53c609da8c0a22655a6a`. Latest product-source checkpoint: `b5458732188ea2ac1b6ab27ca8f1c73a87e3c06e`; descendants `44e3a626bebf255b8a08eb212a62c5fb07ebe74b` and `7bd561845032ed68084e53c609da8c0a22655a6a` only corrected the libdisk source checker. Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; it remains untouched.
- This continuous session completed the folder permission consistency slice:
  - `0e83450aa686d745f80547604bd4f08479a43538` added a reusable permission-file snapshot/apply/rollback batch and rewrote `del_folder()` to stage the directory, publish all ACL rewrites, and restore file images/directory/folder snapshot after failure.
  - `f00ed9b4ba735b1943887ccd95b4dcda75255e88` changed `mod_folder()` to precompute original file images for the rename and apply them as one compensating batch. Rollback restores original bytes rather than attempting a reverse textual rewrite.
  - `b5458732188ea2ac1b6ab27ca8f1c73a87e3c06e` removed the staging-path TOCTOU window: the service now reserves a private dot-prefixed staging parent with exclusive `mkdir()` semantics, retries only `EEXIST`, then renames the folder into that parent. Added policy regressions for collision retry, non-collision error propagation and truncated paths.
  - The source checker was repaired in `44e3a626` and `7bd56184`; current exact-head L2 validation passes.
- Exact-head Actions / current validation boundary:
  - Libdisk share-list L2 `45103728364` on `7bd561845032ed68084e53c609da8c0a22655a6a`: PASS. Both GNU89 ASan/UBSan host regressions and the public-API/Samba failure-cleanup contract passed.
  - XY-C1 firmware validation `45103585439` on product-source SHA `b5458732188ea2ac1b6ab27ca8f1c73a87e3c06e`: all host/pre-build checks PASS; `Build XY-C1` was IN_PROGRESS at the last poll; TRX verification and artifact upload were PENDING.
  - NVRAM transaction-boundary audit `45103585419` on the same product-source SHA: all NVRAM policy regressions PASS; the raw userland ioctl/kernel NVRAM caller audit was IN_PROGRESS.
  - HTTPD authentication/security `45103585429` and Process lifecycle inventory `45103585424` on the same product-source SHA: PASS.
- The earlier L2 failures on `b5458732` and `44e3a626` were source-checker contract/locator failures after the GNU89 sanitizer binaries had passed. They were corrected on descendants; exact current-head L2 `45103728364` is PASS.
- No local MIPS/uClibc firmware build or hardware test was run in this environment. Do not claim L3 until Build XY-C1, TRX verification and artifact upload all PASS for the same product-source SHA. L4 hardware validation remains NOT VERIFIED.
- First action for the next GPT: poll XY-C1 run `45103585439` and NVRAM audit `45103585419` to their final states. Diagnose any first-causal failure; if both pass, add function-level failure-injection coverage for the staged folder delete/rename paths, then inspect other userland stability work without repeating completed L2 checks.

# Cross-session execution mandate

This repository is one continuous engineering project across GPT conversations. Take over from the live `main` tree and the exact handoff checkpoint; do not repeat completed audits or ask the user to choose the next task.

- Implement concrete source fixes with focused regression coverage while preserving security, architecture, compatibility and required testing constraints.
- Follow the loop: modify source → test → one atomic main commit per logical task → inspect Actions for that exact commit SHA. A PENDING run does not block independent work; diagnose and fix failures from their first causal logs.
- For each multi-file logical slice, create all blobs, one complete Git tree and one commit. Publish using non-forced compare-and-swap against the freshly fetched main SHA. Never create per-file half-state commits or force-push.
- Do not ask the user what to do next. Do not claim tests, commits, firmware artifacts or hardware checks that were not actually performed.
- Before context exhaustion, finish the current safe slice, commit it, and record the exact SHA, tests, exact-head CI state, validation boundary and first next action below.

## Current repository state

- Development branch: `main` only. Current documentation parent SHA: `7bd561845032ed68084e53c609da8c0a22655a6a`; the source/product checkpoint remains `b5458732188ea2ac1b6ab27ca8f1c73a87e3c06e` because its descendants only changed the source checker.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Preserve it read-only.
- Do not create feature branches or PRs; never force-push or rewrite history. Publish each multi-file logical change as one complete Git tree and update `main` only with `force:false` and the freshly observed expected SHA.

## Active task

Libdisk share-folder metadata mutation now has a checked compensation path for both deletion and rename.

- `disk_share_permission_batch_prepare_rewrite()` reads each original account/anonymous metadata file and constructs every replacement image before any write. Missing files/records are treated as idempotent no-ops; other read/rewrite failures abort before publication.
- `disk_share_permission_batch_apply()` records successful file replacements and compensates them in reverse order after the first write failure. `disk_share_permission_batch_rollback()` retains per-entry state and can retry an incomplete restoration.
- `del_folder()` reserves a dot-prefixed staging parent with exclusive `mkdir()`, moves the source directory into it, rebuilds the visible folder snapshot, applies all permission-file changes, and removes the staged directory only after the metadata batch succeeds. Failure restores file images, moves the directory back, refreshes the snapshot and removes the empty staging parent. A failed directory restoration is logged and the staged data is left in place rather than deleted.
- `mod_folder()` precomputes original ACL images for old-name-to-new-name rewrites, renames the directory, atomically replaces the whole ACL batch, and restores original bytes/directory/snapshot on error.
- The policy-level test covers batch success, failure of a later write, rollback failure/retry, staging-name collision retry, hard mkdir errors and path truncation. The next coverage gap is full function-level failure injection through the directory operations themselves.

## Verification boundary

- Exact current-head libdisk L2 `45103728364` on `7bd561845032ed68084e53c609da8c0a22655a6a`: PASS, including GNU89 ASan/UBSan policy binaries, the `read_whole_file` sanitizer binary and public API/Samba cleanup source checks.
- Product-source SHA `b5458732188ea2ac1b6ab27ca8f1c73a87e3c06e`:
  - XY-C1 `45103585439`: all host preflight tests PASS; Build XY-C1 IN_PROGRESS at last poll; TRX verification and artifact upload PENDING.
  - NVRAM transaction-boundary audit `45103585419`: all compile/policy/lock regressions PASS; raw userland ioctl/kernel caller audit IN_PROGRESS.
  - HTTPD authentication/security `45103585429`: PASS.
  - Process lifecycle inventory `45103585424`: PASS.
- The failed L2 runs on `b5458732` and `44e3a626` were due to stale/malformed Python source-checker assertions; the C sanitizer binaries passed, and the checker defects were corrected by `44e3a626` / `7bd56184`. Do not treat these as unresolved product-source failures.
- No local MIPS/uClibc build or hardware validation is claimed. L3 requires Build XY-C1, TRX validation and artifact upload all PASS on one product-source SHA; L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Read the live `main` SHA and verify the frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify the backup.
2. Poll XY-C1 run `45103585439` and NVRAM audit `45103585419`. Diagnose the first causal failure rather than rerunning completed stages.
3. If the build succeeds, inspect the final TRX verification and artifact-upload steps before recording L3. Do not claim an artifact that was not uploaded.
4. Add function-level failure-injection tests for `del_folder()` / `mod_folder()` staging, permission publication and snapshot rollback; the existing tests currently validate the generic compensation policy, not every real filesystem boundary.
5. Continue broad Padavan userland stability/security work after that slice. Keep small atomic commits and exact-SHA Actions checks; a PENDING workflow does not block independent work.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

At the end of every session, inspect the live repository state and compare it with this handoff. Verify the current main SHA, frozen backup SHA, engineering baseline, active task, relevant exact-head Actions results (including transitions from PENDING to PASS/FAIL), validation boundaries, unresolved blockers, and the exact first action for the next GPT. Update this handoff and dev log when those facts materially change. If the facts and first action remain accurate, leave the files untouched and do not create a no-op commit.

## End-of-session accuracy checkpoint

- Documentation parent / main SHA: `7bd561845032ed68084e53c609da8c0a22655a6a`; latest product-source SHA: `b5458732188ea2ac1b6ab27ca8f1c73a87e3c06e`.
- Frozen backup: `72f4f157a084f8184b9597ca5bf334b8080e5807`; no backup writes.
- Exact current-head libdisk L2 `45103728364`: PASS.
- Product-source XY-C1 `45103585439`: host checks PASS; Build XY-C1 IN_PROGRESS; TRX and upload PENDING at the last poll. NVRAM audit `45103585419`: policy/lock regression steps PASS; raw userland/kernel caller audit IN_PROGRESS. HTTPD `45103585429` and Process inventory `45103585424`: PASS.
- L3 and L4 are NOT VERIFIED. No local firmware build or hardware test is claimed.
- Root `dev_log.md` had 109 records before this documentation update; this update adds one complete record for 110, below the 120-record rotation threshold.
