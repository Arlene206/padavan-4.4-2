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

- Live main at the parent of this compile-scope correction: 352c47fef554d64ad8c536d1ed8227a3af261d96. The previous signal-coalescing commit contained the new event accumulator but its signal_error local was inserted in the DEBUG_SEGV_HANDLER block rather than the normal signal setup block; this correction moves the declaration next to its use.
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

- The XY-C1 workflow watches trunk/user/busybox/busybox-1.24.x/init/**; the prior trigger omission is closed.
- Exact XY-C1 run 45106983817 on workflow-only SHA 7c556be9eb1caa343d720bcecd3df74c5461262a completed SUCCESS: detector tests, Build XY-C1, TRX verification and artifact upload all passed.
- Compare 1f9d39bc59f9882b9f0d0efae2f09e908110a1ac..7c556be9eb1caa343d720bcecd3df74c5461262a confirms the intervening commits changed only workflow, test, script and dev-log paths; no production source changed after the BusyBox PID1 exact-child implementation. Therefore L3 is verified for that production-source baseline.
- Exact XY-C1 run 45107256442 on test-only SHA 8331df0c690aafa00a7d46263dacea8ab8bfbb01 passed detector/host checks and correctly skipped Build/TRX/artifact. It is not L3 evidence for the new delayed-signal source change.
- The next production init.c/header edit in this slice must trigger the full XY-C1 path; verify Build, TRX and artifact upload on the exact product-source SHA.

## Verification boundary

- L1: source review of the exact-child waitpid migration and bounded SIGTSTP deferred-reap path completed.
- L2: callback tests and source contract PASS on exact SHA 84060ba4f52823b396487b24cdfe66254baff0d0. The repository-integrated GNU89/ASan/UBSan fork/waitid peer-preservation test passed in exact-head BusyBox run 45107256450 on 8331df0c690aafa00a7d46263dacea8ab8bfbb01; Process inventory 45107256374, USB hotplug 45107256428 and NVRAM audit 45107256356 also PASS on that SHA. The AI handoff contract 45107256439 failed only because this handoff omitted the validator-required explicit end-of-session checkpoint sentence; this correction adds it without weakening the validator.
- XY detector tests and AI docs PASS on exact test-only SHA a71253cc; its target build was intentionally skipped. Workflow-only run 45106983817 on 7c556be remains pending at the time of this checkpoint.
- L3: VERIFIED for the exact-child wait/reap production source from 1f9d39bc: exact XY-C1 run 45106983817 on 7c556be passed Build XY-C1, TRX verification and artifact upload; the intervening compare diff contains no production-source changes. The delayed-signal accumulator introduced in THIS COMMIT is not yet L3 verified. L4: NOT VERIFIED; no physical router boot/recovery test has been performed.
- No complete local MIPS/uClibc toolchain or target PID1 binary is available in this session.

## Exact first action for the next GPT

1. Fetch live main and verify the frozen backup still matches 72f4f157a084f8184b9597ca5bf334b8080e5807.
2. Inspect exact-head Actions for the declaration-placement correction; also inspect the prior 352c47fe run and diagnose any failure from its first causal log line. Check BusyBox unit/source-contract, AI handoff, Process inventory, NVRAM/USB and XY-C1 results.
3. The existing exact-child wait/reap source baseline has L3 proof from XY run 45106983817 on 7c556be (Build, TRX and artifact PASS). The new delayed-signal source has L2/L3 pending until the corresponding exact-head tests and XY build pass.
4. Continue PID1 only after the delayed-signal implementation compiles: inspect WAIT/SYSINIT action interactions and delayed-signal retry behavior; preserve shutdown precedence and the independent SIGCONT stop handler.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state before ending, compare the active task, engineering baseline, exact-head Actions, validation boundary, blockers and exact next action, and update this handoff when one of those facts materially changes. If all facts and the first action remain accurate, leave this file untouched and do not create a no-op commit.

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state before ending, compare the active task, engineering baseline, exact-head Actions, validation boundary, blockers and exact next action, and update this handoff when one of those facts materially changes. If all facts and the first action remain accurate, leave this file untouched and do not create a no-op commit.

Before ending a conversation, compare the live repository with this record: engineering/source baseline, active task, exact first action, relevant Actions status, validation boundary, blockers and frozen backup identity. If a fact or first action materially changes, update this file in the same logical engineering commit when possible. If it remains accurate, leave it untouched; do not chase the latest SHA with no-op commits.

## End-of-session accuracy checkpoint

- Parent main SHA for the delayed-signal event accumulator source/test slice: 056edb68a0c9987d24cf397da377024bf5e77c2e; commit 352c47fef554d64ad8c536d1ed8227a3af261d96 added init_signal_pending.h, its tests, init.c integration, the source contract and focused workflow.
- Review immediately after publication caught that signal_error had been inserted in the DEBUG_SEGV_HANDLER block while used in the normal signal setup. This correction moves the declaration into the correct scope. Do not claim that 352 compiled successfully.
- A local GNU89 ASan/UBSan run of the standalone event-policy tests passed. It does not validate the full BusyBox init.c translation unit.
- Previous exact-child source baseline L3: XY-C1 run 45106983817 on 7c556be passed Build XY-C1, TRX verification and artifact upload; compare 1f9d39bc..7c556be contains no intervening production-source changes.
- Exact-head Actions for 352c47fe were IN_PROGRESS at the last check. The correction's Actions have not yet been created. New signal-fix L2/L3 remain pending; L4 hardware is not verified.
