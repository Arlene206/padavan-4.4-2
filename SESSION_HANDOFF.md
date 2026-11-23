# SESSION HANDOFF — Padavan 4.4

## Current repository state

- Repository: Arlene206/padavan-4.4-2. The authorized development branch is main. The parent for the current FRP credential seeding source/test slice is 5378fccc2201f4359ea3b0b37f1043d2b813a883. This commit is intended to include source, regression, log and handoff as one atomic commit; resolve THIS COMMIT from Git history and do not create a follow-up commit just to replace its placeholder SHA.
- The immutable archival ref backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807. Never modify, delete, rename, merge, rebase, cherry-pick, reset, prune, or develop from it.
- Branch inventory at this checkpoint contains main and the frozen backup only. Do not create feature, fix, temporary or checkpoint branches. No pull request development.
- There is no local repository checkout available in this session. Source changes are prepared by GitHub Git Data API; do not claim local shell tests, compiler runs or firmware builds.
- Legacy PR #1 is closed and unmerged, head ba48e22609e0c81ad757b06756fe91d05f39cbf6 and base 9d5a5cb4ab574c0e610ed7a0815c0b1ad749da74. The inspected AliDDNS HMAC helper, OpenSSL 3 EVP_MAC code, CA trust-store bootstrap, tests and package/workflow integration are already present in main. Do not reopen or develop its old head.

## Active task

### Preserve USB printer unplug event when a spooler stop fails

- Confirmed source defect in `trunk/user/rc/src/services_stor.c`: the `usb_unplug_lp` handler discarded the return value from `stop_usb_printer_spoolers()`, then passed action status 0 to `usb_hotplug_event_ack_status()` and cleared the marker. `trunk/user/rc/src/services_usb.c` implements the stop API as an aggregate of U2EC, LPD and p910nd stop results and can return a failure.
- The new caller records the stop return in `printer_unplug_status`, skips NVRAM marker clear when it is nonzero, then passes the action status and marker-clear status to `usb_hotplug_event_ack_status()`. This preserves retry eligibility after failed stop. When stop succeeded but marker clear failed, the action latch prevents repeating the successful stop; subsequent retries only acknowledge the marker. If `usblp` is still needed by another port, the existing no-stop/no-op path continues to acknowledge the event.
- `trunk/user/rc/src/tests/usb_hotplug_event_policy.c` adds a positive EBUSY action-failure case; `scripts/check_storage_safe_remove.py` now contracts the stop result, acknowledgement gating, error latch and retry scheduler. `.github/workflows/storage-hotplug-stability.yml` already covers all changed paths and runs the policy binary plus checker.
- This is one source/test/checker atomic slice. No local checkout; exact-head storage workflow must provide L2 evidence. The new source SHA also requires XY-C1 MIPS/uClibc build, TRX and artifact stages for L3.

### Exact parallel baseline and Actions

- Libdisk unique ACL lookup source e757f86309b19f890797b2f36fb15f44f87a7e95 passed its descendant exact-head libdisk run 45114769360 on 5378fccc2201f4359ea3b0b37f1043d2b813a883, including both GNU89 sanitizer tests and the Python source contract.
- e757 exact XY-C1 run 45114634020 completed SUCCESS. The same-SHA firmware build, TRX verification and artifact upload completed; this closes L3 for the e757 ACL source only. It does not validate later source changes.
- e757 NVRAM transaction boundary audit 45114633929, HTTPD authentication/security 45114633840 and Process lifecycle inventory 45114634024 completed SUCCESS.
- d2c FRP source commit `d2c388a04f6125666043bf94672a8f4222655d4b` exact XY-C1 run 45115500761 was still IN_PROGRESS at its last lookup; Build XY-C1 was running after successful prerequisites, detector and earlier host checks. TRX/artifact are not yet verified. Exact-head d2c FRP and Process workflows exposed only stale source contracts; Process run 45115500771 ran the copyscripts private FRP cases successfully before the checker-only failure.
- Checker correction commit f9320f888b7ac284a4b657b1ae58feca40f0428f: Process lifecycle run 45115896497 completed SUCCESS; FRP run 45115896479 failed at its remaining second stale assertion in `scripts/check_frp_lifecycle.py` after all FRP shell tests passed. Checker-only commit 41285d396d7bd1f984f13c49699ed6dacbd27e92 fixes that last assertion; its exact FRP rerun 45116127095 was IN_PROGRESS at the last check and handoff validator 45116127034 completed SUCCESS.
- FRP source contracts are distinct from product validation. Do not transfer L3 evidence between e757, d2c and this new printer source commit.
## Verification boundary

- Prior e757 libdisk ACL change: L1 and L2 passed; L3 passed on exact e757 SHA 45114634020 (XY-C1 build, TRX and artifact upload); L4 NOT VERIFIED.
- d2c FRP source: all focused shell regressions, including private FRP staging and mode repair, passed on GitHub; its source contracts were being updated. Exact XY-C1 run 45115500761 was still in progress; do not claim L3 unless the full same-SHA target job concluded successfully.
- Checker-only f932: exact Process run 45115896497 SUCCESS; FRP run 45115896479 failed only in a second stale source contract after all shell tests passed. Commit 41285 corrects the last stale assertion; its exact FRP run 45116127095 was in progress. No local test is claimed.
- New printer unplug source slice in THIS COMMIT has L1 review only; exact storage-hotplug host/sanitizer/source-checker and new-SHA XY-C1 build/TRX/artifact are required. L4 NOT VERIFIED.

- L1: source-level issue and intended fix reviewed. The old FRP cp -f path is gone from the changed source; ordinary three-argument helper use remains unchanged and FRP gets the fourth private-mode flag.
- L2: NOT YET VERIFIED for this FRP slice. There is no local checkout; the expanded copyscripts_ca_status.sh was not executed locally. Require the exact-head Process lifecycle regression push Action to run and pass the shell regression and full source contracts.
- L3: NOT YET VERIFIED for the e757 ACL source until run 45114634020 completes Build XY-C1, TRX verification and artifact upload on e757. The FRP source change also needs its own exact new-SHA XY-C1 production-input detector, target Build, TRX and artifact upload. No different SHA's green check substitutes.
- L4: NOT VERIFIED; no physical router test is evidenced in this session.

## Exact first action for the next GPT

1. Fetch live main and confirm the frozen backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807. Resolve THIS COMMIT from history and inspect the entire printer unplug diff; do not assume the saved parent remains live.
2. Inspect the exact-head `storage-hotplug-stability` push run for THIS COMMIT. Require the USB hotplug policy GNU89 ASan/UBSan binary and `scripts/check_storage_safe_remove.py` to pass; read the first causal log and fix if not.
3. Inspect the exact same-SHA XY-C1 detector/build/TRX/artifact workflow. No L3 claim until firmware build, TRX and artifact all pass for this source SHA.
4. Revisit FRP checker exact-head run 45116127095 and d2c XY-C1 45115500761 if they remain material. e757 XY-C1 45114634020 is SUCCESS and NVRAM audit 45114633929 is SUCCESS; do not repeat those completed checks.
5. After storage validation, continue the next source-backed stability/security issue across the userland. Do not re-audit completed libdisk ACL lookup or BusyBox PID1 work without new evidence.

# Cross-session execution mandate

- The same maintainer continues the same repository mission. Recover live state once, resume the recorded task and implement; do not ask the user what to do next or repeat completed audits.
- Stability bug discovery and repair is primary; security hardening continues in parallel. Critical vulnerabilities, data corruption, boot failures and severe service faults take immediate priority. Cover the integrated Padavan userland rather than remaining in one subsystem.
- Each logical source/test slice is one atomic main commit: fetch the latest main parent, create every blob, one full tree and one commit, then update main through non-forced compare-and-swap using expected_sha. Do not create sequential partial per-file commits. Never force-push, amend, rewrite history, branch for development or use PRs.
- Verify the landed main SHA and complete diff. Inspect relevant push Actions only when head_sha exactly equals that commit. Diagnose the first causal failure. PASS closes that check; FAIL requires a fix; PENDING does not block independent work and is not a reason to poll idly.
- Preserve security, architecture, compatibility and necessary regression tests. Review callers/callees before source edits. Do not weaken checks for a green result.
- Distinguish L1 source review, L2 host test/build, L3 MIPS/uClibc target build + TRX + artifact on the same production-source SHA, and L4 physical router validation. A static checker or skipped build is not L3/L4.
- Keep GPT Session Rotator unchanged. Do not reset or replay SHA 9d5a5cb or alter its quiet-period/de-duplication behavior.
- Before context exhaustion, finish the current safe slice, inspect its diff, publish and check exact-head Actions. Leave an accurate first action for the next GPT.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect live GitHub before ending and compare the meaningful source baseline, active task, exact-head Actions, validation boundary, blockers, frozen backup identity and exact first action with this file. Update this handoff only if next-session-critical facts materially change. If it remains accurate, explicitly conclude the check with no file change and no commit. Do not chase documentation-only SHA changes or create a no-op commit.
