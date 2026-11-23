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
- The workflow-only trigger rerun at 7c556be9eb1caa343d720bcecd3df74c5461262a has exact XY-C1 run 45106983817 IN_PROGRESS; AI docs run 45106983832 and Process inventory run 45106983853 PASS. The workflow change is explicitly classified as a firmware validation input, so this run is intended to test Build XY-C1, TRX and artifact upload for the current BusyBox PID1 source tree. Check its completed steps and result before claiming L3.
- This commit modifies the BusyBox init test file only; the XY workflow should run host tests but the detector should classify this as tests-only and skip Build/TRX/artifact. The next real production-source change under init/** triggers the corrected full target path.

# Cross-session execution mandate

The user delegates one continuing engineering job across conversations. The repository is the project memory; resume from live GitHub state, not copied chat summaries.

- Recover once, read the protocol and current handoff, confirm main and the frozen backup, then execute the active engineering task. Do not repeat completed audits or ask the user to select the next task.
- Stability defect discovery and repair is the primary mission; security work continues in parallel. Confirmed critical vulnerabilities, data corruption, boot failures and severe system failures take immediate priority.
- Use the loop: modify production source/tests → run focused checks → create one atomic commit on main → inspect Actions for that exact SHA. A failed exact-head run must be diagnosed from its first causal error; a pending run does not block independent engineering.
- For each multi-file logical slice, create all blobs, one complete Git tree and one commit with the freshly fetched main as its only parent. Update main with force:false and the expected parent SHA. Never create feature/checkpoint branches, per-file partial commits, PRs, force-pushes, amended commits or rewritten history.
- Preserve the frozen backup branch permanently and read-only. Do not alter GPT Session Rotator state or workflow.
- Do not claim tests, commits, Actions, firmware images or hardware behavior that were not actually verified. Keep L1/L2 host evidence separate from L3 MIPS/TRX/artifact validation and L4 hardware validation.
- Use bounded, reviewable slices. Finish and publish the current safe slice before beginning work that cannot be completed safely. Maintain exact resumable facts below; no follow-up commit solely to replace THIS COMMIT with its SHA.

## Current repository state

- Main at the parent of this checkpoint: 7c556be9eb1caa343d720bcecd3df74c5461262a. Fetch live main again at the next takeover and immediately before any write.
- Frozen backup: backup/pre-plugin-overhaul-2026-11-14 at 72f4f157a084f8184b9597ca5bf334b8080e5807, read-only.
- Production PID1 wait/reap baseline: 1f9d39bc59f9882b9f0d0efae2f09e908110a1ac; exact host/source regression follow-up 84060ba4f52823b396487b24cdfe66254baff0d0 passed.
- XY-C1 workflow path correction: 11090e80b126fa1665758026d2163fd9e2f09e6a; detector fixture correction: a71253cc463befcab865afb8a90407afa7975e33; workflow-only full-build rerun: 7c556be9eb1caa343d720bcecd3df74c5461262a.

## Active task

### BusyBox PID 1 wait/reap integration regression

- Primary files: trunk/user/busybox/busybox-1.24.x/init/init.c, init_reap_queue.h, init/tests/init_reap_queue.c, scripts/check_busybox_init_reap_queue.py, .github/workflows/busybox-init-reap-queue.yml.
- Existing implementation and callback tests are on main. This checkpoint adds a real-fork test ensuring init_wait_for_child(target_pid,waitpid,...) cannot reap a different zombie child while blocking on the target.
- The focused BusyBox workflow compiles under -std=gnu89 -Wall -Wextra -Werror -fsanitize=address,undefined, executes the C tests, then checks the PID1 wiring/source contract. The new test must be registered in both main() and the Python source contract.
- After this commit, inspect its exact-head BusyBox PID1 run, Process inventory/lifecycle, AI handoff docs and XY-C1 host run. If a check fails, fetch job logs and fix the first causal issue without weakening coverage.
- Once the real waitpid test passes, continue the PID1 review at the next concrete failure/race discovered from stop_handler(), pending queue drain, reload_inittab(), main-loop child reaping and recovery-shell respawn. Do not introduce speculative signal-handler rewrites. If no concrete PID1 issue remains, select the next high-risk module from the roadmap rather than polishing this component indefinitely.

### XY-C1 production-build gate

- XY-C1 firmware validation now watches trunk/user/busybox/busybox-1.24.x/init/**; this closes the prior trigger omission for BusyBox PID1 source.
- Exact run 45106741184 on workflow-fix SHA 11090e80 failed only because the detector test fixture encoded literal delimiters; corrected by a71253cc.
- Exact run 45106798326 on a71253cc passed detector/host checks but skipped Build/TRX/artifact because that commit changed only host tests/docs. This did not verify L3 for the production PID1 source.
- Exact workflow-only run 45106983817 on 7c556be is still IN_PROGRESS. Its purpose is to execute Build XY-C1, TRX verification and artifact upload for the current PID1 source baseline. Verify that exact run's terminal state; do not substitute a host-only pass.
- The next actual change to init.c or init_reap_queue.h must result in a push-triggered XY-C1 build. Do not manually dispatch and misrepresent it as same-SHA main push validation.

## Verification boundary

- L1: source review of the exact-child waitpid migration and bounded SIGTSTP deferred-reap path completed.
- L2: callback tests and source contract PASS on exact SHA 84060ba4f52823b396487b24cdfe66254baff0d0. An isolated GNU89 ASan/UBSan fork/waitid peer-preservation harness passed 10/10 locally; the repository-integrated variant is in THIS COMMIT and awaits exact-head Actions.
- XY detector tests and AI docs PASS on exact test-only SHA a71253cc; its target build was intentionally skipped. Workflow-only run 45106983817 on 7c556be remains pending at the time of this checkpoint.
- L3: NOT VERIFIED until 45106983817 completes Build XY-C1, TRX verification and artifact upload successfully on the same exact main SHA. L4: NOT VERIFIED; no physical router boot/recovery test has been performed.
- No complete local MIPS/uClibc toolchain or target PID1 binary is available in this session.

## Exact first action for the next GPT

1. Fetch the new live main SHA and verify this commit contains the fork/waitid peer-preservation test. Confirm the frozen backup remains unchanged.
2. Inspect exact-head BusyBox PID1 reap queue regression and its C host-regression/source-contract steps. If failed, fetch the job log and fix the first causal error. Also inspect exact-head AI docs and Process inventory/lifecycle results.
3. Inspect exact-head XY-C1 run 45106983817. If Build, TRX and artifact upload all passed on 7c556be, record that L3 is verified for this product-source content; otherwise diagnose the first causal build failure and retry on a later production-source SHA.
4. Resume PID1 from the pending queue and wait/reap call graph; select only a source-demonstrable reliability defect, then add a regression before changing production behavior. The next production PID1 edit should trigger the corrected XY-C1 build path.

## Mandatory handoff checkpoint

Before ending a conversation, compare the live repository with this record: engineering/source baseline, active task, exact first action, relevant Actions status, validation boundary, blockers and frozen backup identity. If a fact or first action materially changes, update this file in the same logical engineering commit when possible. If it remains accurate, leave it untouched; do not chase the latest SHA with no-op commits.

## End-of-session accuracy checkpoint

- Parent main SHA for this atomic test/documentation checkpoint: 7c556be9eb1caa343d720bcecd3df74c5461262a. This commit contains the real fork/waitid peer-preservation test, source-contract registration, updated PID1 roadmap and resumable handoff.
- Previous exact-head PID1 source/test run 45106412054 on 84060ba4f52823b396487b24cdfe66254baff0d0: PASS.
- Workflow-fix SHA 11090e80: XY-C1 run 45106741184 FAIL only at the detector mock delimiter fixture; corrected by a71253cc. Process inventory 45106741192 PASS; AI docs 45106741330 PASS.
- Test-fixture correction SHA a71253cc: XY-C1 host run 45106798326 PASS, Build/TRX/artifact SKIPPED; AI docs 45106798279 PASS.
- Workflow-only rerun SHA 7c556be: XY-C1 run 45106983817 IN_PROGRESS; Process inventory 45106983853 and AI docs 45106983832 PASS.
- The new commit's Actions have not yet been created; fetch them by the resulting exact main SHA after publication. No L3/L4 result is claimed until the corresponding gates pass.

