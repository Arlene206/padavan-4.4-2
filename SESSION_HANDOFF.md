# SESSION HANDOFF — Padavan 4.4

## 2026-11-23 BusyBox PID 1 exact-child wait checkpoint

- The live development branch is main. The parent SHA for this test/handoff slice is 7c556be9eb1caa343d720bcecd3df74c5461262a; fetch live main again at takeover and immediately before any future write.
- The explicitly preserved branch backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807. It is an immutable archival snapshot: never write, delete, rename, merge, rebase, cherry-pick, prune or use it for development.
- The previous active handoff task (libdisk share-folder transaction tests and watchdog signal source-checker repair) is complete in later main commits. Do not repeat those audits. The current engineering thread is BusyBox PID 1 child reaping and exact-child waiting.

### Production change already on main

- Production source commit: 1f9d39bc59f9882b9f0d0efae2f09e908110a1ac, parent e3d9935d58d255d089392a1c8861779a2b6b1be9.
- Files: trunk/user/busybox/busybox-1.24.x/init/init.c, init_reap_queue.h, the matching test/checker, and lossless dev-log rotation.
- Defect: old waitfor(pid) probed with kill(pid,0), then used generic blocking wait(NULL). PID reuse could make PID 1 block on an unrelated live process; generic wait could also consume any other exited child instead of the requested action child.
- Fix: init_wait_for_child() calls waitpid(target_pid,NULL,0) for the exact PID, retries EINTR, accepts ECHILD as the SIGTSTP handler having already reaped the child into the bounded queue, and propagates other failures. waitfor() marks the exact reaped child, collects only already-exited peers with waitpid(-1,NULL,WNOHANG), retries EINTR for that nonblocking sweep, then drains queued PIDs in normal context.
- The SIGTSTP handler does not touch utmp or the mutable inittab list; it reaps only while the bounded queue has room and queues the PID. The main context drains while SIGTSTP is blocked. Inittab reload blocks SIGTSTP and drains queued children before changing the action list.
- Existing callback regressions cover exact PID/options, EINTR retry, ECHILD, errno propagation, unexpected zero return and invalid inputs. This commit adds an OS-level fork/waitid/WNOWAIT regression: one child is made an exited-but-unreaped zombie, PID 1's helper waits for a second delayed child, and the first child must remain waitable afterward. The test therefore checks actual kernel waitpid semantics, not only a mocked callback.

### Exact-head development and CI facts

- Exact source-follow-up SHA 84060ba4f52823b396487b24cdfe66254baff0d0:
  - BusyBox PID1 reap queue regression 45106412054: PASS (GNU89, ASan/UBSan host binary and full source contract).
  - Process inventory 45106412151: PASS.
  - AI handoff documentation contract 45106411973: PASS.
  - USB storage hotplug 45106411983: PASS.
  - NVRAM transaction audit 45106412065: PASS.
- The production source SHA 1f9d39bc did not start XY-C1 validation because, at that SHA, .github/workflows/xy-c1-firmware-validation.yml did not watch the BusyBox init/** subtree. That trigger omission was corrected in 11090e80b126fa1665758026d2163fd9e2f09e6a by adding trunk/user/busybox/busybox-1.24.x/init/** to its push path filter.
- The first workflow-fix run 45106741184 failed before firmware-input detection: the new mocked Git fixture had literal backslash-n/backslash-zero rather than actual newline/NUL delimiters. Process inventory 45106741192 and AI handoff contract 45106741330 passed. The literal fixtures were corrected in a71253cc463befcab865afb8a90407afa7975e33.
- Exact-head XY run 45106798326 and AI docs run 45106798279 PASS on a71253cc. Detector tests executed successfully, while Build XY-C1/TRX/artifact were correctly SKIPPED because that commit changed only tests/docs. It is not L3 evidence.
- The workflow-only trigger rerun at 7c556be9eb1caa343d720bcecd3df74c5461262a still has exact XY-C1 run 45106983817 IN_PROGRESS; AI docs run 45106983832 and Process inventory run 45106983853 PASS. Its Build XY-C1 step was in progress at the last check, with TRX/artifact pending. The later test-only commit 8331df0c690aafa00a7d46263dacea8ab8bfbb01 has BusyBox 45107256450, Process 45107256374, NVRAM 45107256356 and USB 45107256428 PASS; AI handoff run 45107256439 failed only because this live handoff lacked the explicit validator-required sentence now added in this correction. XY-C1 run 45107256442 passed its host/detector steps but intentionally skipped Build/TRX/artifact because this commit changed tests/docs only. Do not treat it as L3.
- This commit modifies the BusyBox init test file only; the XY workflow should run host tests but the detector should classify this as tests-only and skip Build/TRX/artifact. The next real production-source change under init/** triggers the corrected full target path.

# Cross-session execution mandate

- Every logical source/test slice must be delivered as one atomic main commit, built from one complete Git tree and published by a non-forced expected-SHA update.
- Before context exhaustion, finish and publish the current safe slice, inspect its exact-head Actions, and leave the next concrete task and validation boundary here.

The user delegates one continuing engineering job across conversations. The repository is the project memory; resume from live GitHub state, not copied chat summaries.

- Recover once, read the protocol and current handoff, confirm main and the frozen backup, then execute the active engineering task. Do not repeat completed audits or ask the user to select the next task.
- Stability defect discovery and repair is the primary mission; security work continues in parallel. Confirmed critical vulnerabilities, data corruption, boot failures and severe system failures take immediate priority.
- Use the loop: modify production source/tests → run focused checks → create one atomic commit on main → inspect Actions for that exact SHA. A failed exact-head run must be diagnosed from its first causal error; a pending run does not block independent engineering.
- For each multi-file logical slice, create all blobs, one complete Git tree and one commit with the freshly fetched main as its only parent. Update main with force:false and the expected parent SHA. Never create feature/checkpoint branches, per-file partial commits, PRs, force-pushes, amended commits or rewritten history.
- Preserve the frozen backup branch permanently and read-only. Do not alter GPT Session Rotator state or workflow.
- Do not claim tests, commits, Actions, firmware images or hardware behavior that were not actually verified. Keep L1/L2 host evidence separate from L3 MIPS/TRX/artifact validation and L4 hardware validation.
- Use bounded, reviewable slices. Finish and publish the current safe slice before beginning work that cannot be completed safely. Maintain exact resumable facts below; no follow-up commit solely to replace THIS COMMIT with its SHA.

## Current repository state

- Live main at the parent of this queue-header retry/atomic-install fix: 9de5f37d550244590158b302b6ac9935bff765b3. The frozen backup remains backup/pre-plugin-overhaul-2026-11-14 at 72f4f157a084f8184b9597ca5bf334b8080e5807.
- Frozen backup: backup/pre-plugin-overhaul-2026-11-14 at 72f4f157a084f8184b9597ca5bf334b8080e5807, read-only.
- Production PID1 wait/reap baseline: 1f9d39bc59f9882b9f0d0efae2f09e908110a1ac; exact host/source regression follow-up 84060ba4f52823b396487b24cdfe66254baff0d0 passed.
- XY-C1 workflow path correction: 11090e80b126fa1665758026d2163fd9e2f09e6a; detector fixture correction: a71253cc463befcab865afb8a90407afa7975e33; workflow-only full-build rerun: 7c556be9eb1caa343d720bcecd3df74c5461262a.

## Active task

### BusyBox PID 1 delayed-signal event preservation

- Immediate compile review found a declaration-scope defect in the initial source patch: signal_error was accidentally declared inside the optional DEBUG_SEGV_HANDLER block but used in the normal init signal setup. This commit removes the misplaced declaration and declares it in the ordinary setup block. Exact-head CI for both the original source slice and this correction must confirm the compile path.

- Source review confirmed a lost-event race in check_delayed_sigs(): the main loop snapshots the single shared bb_got_signal value and then clears it, so a different delayed handler can overwrite that global between those operations. The source comment already acknowledged SIGHUP/Ctrl-Alt-Del collision; a shutdown request could also be overwritten by another delayed event.
- This slice replaces the lossy single-signal slot for PID1 delayed events with independent sig_atomic_t event bits in init_signal_pending.h. A separate field retains the most recently delivered shutdown mode. Delayed handler sigaction masks are full to serialize the accumulator's read-modify-write; the main loop blocks the exact delayed signal set while taking/clearing a batch. A failed mask restore requeues the snapshot rather than discarding it.
- Existing bb_got_signal/record_signo remains dedicated to SIGCONT inside stop_handler(); no other BusyBox applet or global libbb signal behavior is changed.
- Focused tests cover different signals in one batch, repeated/coalesced requests, shutdown mode retention, snapshot restoration and invalid inputs. The source-contract script and BusyBox workflow compile/check this policy. Exact-head C test, source contract, Process inventory, AI docs and XY-C1 build are pending for the resulting SHA.
- Next, inspect the interaction between delayed events, WAIT/SYSINIT actions and main-loop sleeps/waits. Preserve shutdown priority and SIGCONT stop handling; make no broader signal changes without a testable defect.

### XY-C1 production-build gate

- The XY-C1 workflow watches trunk/user/busybox/busybox-1.24.x/init/**; the BusyBox init trigger omission is closed.
- Exact XY-C1 run 45106983817 on workflow-only SHA 7c556be9eb1caa343d720bcecd3df74c5461262a completed SUCCESS: Build XY-C1, TRX verification and artifact upload all passed. Compare 1f9d39bc..7c556be confirms no intervening production-source changes; L3 is VERIFIED for exact-child wait/reap.
- The delayed-signal policy unit/source tests passed in the focused BusyBox job on 352c47fe and 9de5f37d. The ordinary signal_error declaration scope was corrected in 9de5f37d; exact-head BusyBox, Process, AI handoff, USB and NVRAM checks passed on 9de5f37d.
- Exact XY-C1 run 45108415640 on 9de5f37d was still IN_PROGRESS at the latest verified poll; its job 137476531958 had completed detector checks and was IN_PROGRESS in Build XY-C1, with TRX and artifact steps pending. The earlier curl error 56 log came from a different/unmatched job and must not be attributed to this run.
- This commit retries all curl errors, writes queue.h to a unique temporary file in the destination directory, rejects empty results, normalizes mode to 0644 and atomically renames only a complete file. Existing valid headers remain intact on failure; empty stale headers no longer bypass installation. A local HTTP regression returns a truncated first response and valid second response to verify retry and final file integrity. XY path filters now watch root Makefile/helper/test and the workflow runs this regression.
- Exact XY-C1 run 45109162782 on ca609c08 is IN_PROGRESS. Its host detector and new atomic queue-header regression steps PASS; firmware input detection PASS; Build XY-C1 is IN_PROGRESS, TRX and artifact steps PENDING. The older 9de XY run 45108415640 is also still IN_PROGRESS at the last poll (Build stage), not failed. Do not infer L3 until a same-SHA Build/TRX/artifact run completes.

## Verification boundary

- L1: source review of exact-child waitpid and bounded SIGTSTP deferred-reap path completed; delayed single-slot signal overwrite defect identified and replaced by independent sig_atomic_t event bits and a blocked snapshot/clear window.
- L2: callback/source-contract checks and real fork/waitid peer-preservation regression passed on predecessor SHAs. The delayed-signal accumulator C test passed GNU89 with -Wall -Wextra -Werror and ASan/UBSan on 352c47fe and 9de5f37d; Process, AI docs, USB and NVRAM passed on 9de5f37d.
- L3: exact-child wait/reap source from 1f9d39bc is VERIFIED by XY-C1 run 45106983817 on 7c556be, with Build, TRX and artifact upload success and no intervening product-source change. Delayed-signal source remains NOT YET L3 VERIFIED. Exact XY-C1 9de run 45108415640 and ca609 run 45109162782 were both IN_PROGRESS in Build XY-C1 at the last verified polls; no terminal firmware result exists yet. The ca609 helper regression itself PASSED.
- L4: NOT VERIFIED; no physical XY-C1 router boot/recovery test has been performed.
- Do not attribute curl error 56 to 9de XY run 45108415640; the matching run/job lookup showed it remained in progress in Build XY-C1. Claim L3 only after the exact product-source SHA passes Build, TRX and artifact upload.

## Exact first action for the next GPT

1. Fetch live main and verify backup/pre-plugin-overhaul-2026-11-14 remains frozen at 72f4f157a084f8184b9597ca5bf334b8080e5807.
2. Inspect exact-head BusyBox run for this checker correction. The prior 81be run's C accumulator test PASSED; its source checker failed at Python SyntaxError because the newly added assertion used JavaScript syntax. Confirm the corrected Python checker and all source-contract assertions pass.
3. Inspect exact-head AI docs, Process inventory, USB/NVRAM and XY-C1. The full XY run on 81be remains IN_PROGRESS in Build; ca609 XY is also in Build. Do not claim L3 until Build/TRX/artifact pass for the same source SHA.
4. Continue PID1 signal handling review from mask restoration and delayed-event ordering, preserving shutdown priority and the independent SIGCONT stop-handler path unless another concrete, regression-testable defect is found.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state before ending, compare the active task, engineering baseline, exact-head Actions, validation boundary, blockers and exact next action, and update this handoff when one of those facts materially changes. If all facts and the first action remain accurate, leave this file untouched and do not create a no-op commit.

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state before ending, compare the active task, engineering baseline, exact-head Actions, validation boundary, blockers and exact next action, and update this handoff when one of those facts materially changes. If all facts and the first action remain accurate, leave this file untouched and do not create a no-op commit.

Before ending a conversation, compare the live repository with this record: engineering/source baseline, active task, exact first action, relevant Actions status, validation boundary, blockers and frozen backup identity. If a fact or first action materially changes, update this file in the same logical engineering commit when possible. If it remains accurate, leave it untouched; do not chase the latest SHA with no-op commits.

## End-of-session accuracy checkpoint

- Main parent for the current checker correction: 81be0de7b68467468789d7a67c590b5eb737a449.
- Exact 81be results: BusyBox run 45109466477 FAILED in source checker at line 127 with Python SyntaxError because a JavaScript const/.indexOf block was inserted into the Python file. Both C binaries compiled and ran before that step: init reap-queue and delayed-signal accumulator tests PASS. AI docs 45109466503 and Process inventory 45109466511 PASS. USB 45109466582 and NVRAM 45109466594 remained IN_PROGRESS at last poll.
- Exact XY-C1 run 45109466474 on 81be is IN_PROGRESS in Build; TRX/artifact pending. Exact XY-C1 runs 45109162782 on ca609 and 45108415640 on 9de were also still in Build at their last matching polls; neither was confirmed failed. ca609 helper regression and detector PASS.
- The earlier curl error 56 log was misattributed: it does not belong to exact XY run 45108415640. The atomic queue installer remains a defensive reliability improvement; do not report a proven failed 9de build prerequisite.
- Exact-child wait/reap L3 remains verified by XY run 45106983817 on 7c556be (Build, TRX and artifact PASS; no intervening production-source changes from 1f9d39bc). The delayed-signal source is not yet L3 verified.
- This correction only fixes the Python source checker. The fail-closed signal-mask restoration source remains committed on 81be and awaits its rerun; L4 hardware is NOT VERIFIED.
