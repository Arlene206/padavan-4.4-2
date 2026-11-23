# SESSION HANDOFF — Padavan 4.4

## 2026-11-23 watchdog signal / libdisk stability checkpoint

- Current `main` before this atomic checker/docs correction: `5892a4229e9683979bc1e97fca71b2e8528d2413`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; no backup writes.
- The libdisk folder metadata slice remains complete at source level: deletion and rename use precomputed ACL file images, reverse-order permission compensation, directory/snapshot restoration, and exclusive hidden staging-directory creation. A concurrent follow-up commit `1ccfaed9ba04feaa5c1d6802e6708d788e16f953` also fixed an `add_folder()` rollback defect: ACL rows are no longer deleted when directory removal fails and the directory remains live. Its exact-head libdisk run `45104177783` passed.
- Current watchdog run `45104213963` on `5892a4229e9683979bc1e97fca71b2e8528d2413` failed only the final Python source-wiring contract. Its GNU89 ASan/UBSan signal, NTP/service-lifecycle, and CPU-watchdog policy tests all passed. The failure was a `ValueError: substring not found`: the checker searched source for literal backslash-n sequences instead of matching formatted C whitespace. This commit replaces the brittle substring checks with one whitespace-normalized source view; no watchdog runtime logic is changed.
- Same-head current Actions at last poll:
  - XY-C1 Firmware Validation `45104213876`: all host/pre-build stages PASS; `Build XY-C1` IN_PROGRESS; TRX verification and artifact upload PENDING.
  - NVRAM transaction boundary audit `45104213886`: policy/lock regressions PASS; raw userland ioctl/kernel caller audit IN_PROGRESS.
  - Process lifecycle inventory `45104213913`: inventory upload/job still in progress.
  - HTTPD authentication/security `45104213964`: PASS.
  - Watchdog signal lifecycle `45104213963`: FAIL only at final source-wiring checker; this commit must be checked at its own exact SHA.
  - DNSMasq lifecycle `45104213901`, HTTPD daemon lifecycle `45104213888`, and AI handoff contract `45104213904`: PASS.
- Exact current-head libdisk regression `45104177783` for the preceding libdisk change `1ccfaed9`: PASS. The prior folder transaction commit and test history are retained below.
- No local MIPS/uClibc firmware build or hardware test is claimed. L3 requires XY-C1 Build, TRX and artifact upload PASS on one exact product-source SHA; L4 remains NOT VERIFIED.
- First next action: inspect the watchdog workflow on this commit's exact SHA; then continue polling XY-C1 and NVRAM runs without waiting on unrelated pending work. Next engineering slice is function-level failure injection around real `del_folder()` / `mod_folder()` filesystem callbacks, because current libdisk tests still exercise shared policy helpers more deeply than their filesystem integration.

# Cross-session execution mandate

This repository is one continuous engineering project across GPT conversations. Take over from the live `main` tree and the exact handoff checkpoint; do not repeat completed audits or ask the user to choose the next task.

- Implement concrete source fixes with focused regression coverage while preserving security, architecture, compatibility and required testing constraints.
- Follow the loop: modify source → test → one atomic main commit per logical task → inspect Actions for that exact commit SHA. A PENDING run does not block independent work; diagnose and fix failures from their first causal logs.
- For each multi-file logical slice, create all blobs, one complete Git tree and one commit. Publish using non-forced compare-and-swap against the freshly fetched main SHA. Never create per-file half-state commits or force-push.
- Do not ask the user what to do next. Do not claim tests, commits, firmware artifacts or hardware checks that were not actually performed.
- Before context exhaustion, finish the current safe slice, commit it, and record the exact SHA, tests, exact-head CI state, validation boundary and first next action below.

## Current repository state

- Development branch: `main` only. This commit's parent is `5892a4229e9683979bc1e97fca71b2e8528d2413`; the source checkpoint includes the libdisk and watchdog work already committed there.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only.
- Never force-push, rewrite history, or create feature branches/PRs. Build a complete Git tree for each logical change and update `main` with `force:false` and the freshly read expected SHA.

## Active task

### Libdisk share-folder integrity

- `del_folder()` and `mod_folder()` precompute account + anonymous permission-file replacements before mutating directories. The generic batch applies replacements and rolls back successful earlier writes in reverse order, preserving retry state if compensation itself fails.
- Delete moves the directory under an exclusively-created dot-prefixed staging directory, refreshes the visible folder list, applies ACL batch, then removes the staged folder. Failure restores ACLs/directory/list. Exclusive `mkdir()` retries only `EEXIST`, eliminating the old lstat/rename destination race.
- Rename precomputes exact old-to-new ACL images and restores original bytes plus directory/list if publication fails.
- Concurrent main-line fix `1ccfaed9ba04feaa5c1d6802e6708d788e16f953` closes an additional `add_folder()` rollback defect: permission rows are preserved when `rmdir()` fails and the directory survives. Its exact-head libdisk regression run `45104177783` passed.
- Remaining coverage gap: exercise the full directory+permission orchestration with injected filesystem callbacks; current batch and staging tests are policy-level.

### Watchdog signal lifecycle

- Signal handlers only record pending bits; the main loop dispatches SIGHUP/Wi-Fi/timer work in normal context with managed masks and termination priority.
- On `5892a4229e9683979bc1e97fca71b2e8528d2413`, workflow `45104213963` compiled and passed all GNU89 ASan/UBSan signal, NTP/service-lifecycle, and CPU module restart regression binaries. Its final source contract failed with `ValueError: substring not found` because three function-call checks used escaped newlines rather than a whitespace-normalized view of C source.
- THIS COMMIT updates that CI contract to normalize C whitespace once and check event dispatch order, unblock-before-dispatch and reblock-after-dispatch in the same normalized coordinate space. No runtime code is changed.

## Verification boundary

- Libdisk exact-head run `45104177783` on `1ccfaed9ba04feaa5c1d6802e6708d788e16f953`: PASS.
- Parent current-head Actions `5892a4229e9683979bc1e97fca71b2e8528d2413`: XY-C1 `45104213876` host checks PASS, Build IN_PROGRESS, TRX/upload PENDING; NVRAM `45104213886` policy/lock tests PASS and raw ioctl/kernel audit IN_PROGRESS; Process inventory `45104213913` IN_PROGRESS; HTTPD auth `45104213964` PASS; watchdog `45104213963` source-checker FAIL (runtime policy binaries PASS); DNSMasq `45104213901` PASS; HTTPD daemon lifecycle `45104213888` PASS; handoff contract `45104213904` PASS.
- No L3 claim until same-SHA Build XY-C1, TRX verification and artifact upload all pass. No hardware L4 test claimed.

## Exact first action for the next GPT

1. Read live `main` and frozen backup branch; confirm no concurrent update before any write.
2. Check this commit's exact-head watchdog workflow. If the normalized source contract passes, proceed to the next independent task; if it fails, fix the first causal assertion.
3. Poll XY-C1 `45104213876` and NVRAM audit `45104213886` to their actual terminal states. Do not wait before doing independent development.
4. Add function-level failure injection for folder delete/rename staging + permission publication + folder snapshot rollback, retaining the current generic policy tests.
5. Continue broad userland stability/security review after the above bounded work. Avoid repeating finished audits.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

At the end of every session, inspect the live repository state and compare it with this handoff. Verify the current main SHA, frozen backup SHA, engineering baseline, active task, relevant exact-head Actions results (including transitions from PENDING to PASS/FAIL), validation boundaries, unresolved blockers, and the exact first action for the next GPT. Update this handoff and dev log when those facts materially change. If the facts and first action remain accurate, leave the files untouched and do not create a no-op commit.

## End-of-session accuracy checkpoint

- Parent main SHA for this checkpoint: `5892a4229e9683979bc1e97fca71b2e8528d2413`; the resulting docs/workflow change is the next main commit. Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Libdisk run `45104177783` on preceding source commit `1ccfaed9`: PASS.
- Parent SHA watchdog workflow `45104213963`: policy regression binaries PASS, final source-wiring checker FAIL on escaped-newline `ValueError`; this commit normalizes whitespace to repair the check.
- Parent SHA XY-C1 `45104213876`: host checks PASS, Build IN_PROGRESS, TRX/upload PENDING. NVRAM audit `45104213886`: policy/lock tests PASS, raw caller audit IN_PROGRESS. Process inventory `45104213913` IN_PROGRESS; HTTPD auth `45104213964`, DNSMasq lifecycle `45104213901`, HTTPD daemon lifecycle `45104213888`, handoff contract `45104213904` PASS.
- L3/L4 not verified. No local firmware build or hardware test claimed.
