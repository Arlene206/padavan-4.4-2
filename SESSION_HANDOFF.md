# 2026-11-20 continuation checkpoint

# Cross-session execution mandate

- Continue main-only development. Every logical source slice is one atomic main commit: create all blobs, one complete Git tree and one commit, then update main using a non-forced compare-and-swap against the exact parent SHA.
- Never amend, force-push, create development branches/PRs, or modify the frozen backup branch.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Implement actual fixes with focused regressions, then inspect Actions for the exact pushed SHA.
- A PENDING Action does not block independent work. Diagnose failures from causal job logs; do not weaken tests to obtain a pass.
- Before context exhaustion, finish a safe slice and leave a factual next task. Do not ask the user what to do next.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

At the end of every session, inspect live main, verify that the frozen backup is unchanged, and check the exact-head Actions relevant to the latest source commit. Update this file only when the active task, source SHA, verification evidence, or first next action has materially changed. If the checkpoint is already accurate, leave it untouched and do not create a no-op commit.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; authorized development branch: `main`. Live main at this recovery was `b90cdf4f6cd8ce0725bdee19dc7b605c0163a775`. The only other branch is the immutable `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; never modify or delete it.
- Legacy PR #1 is closed and unmerged. Its relevant AliDDNS CA trust-store bootstrap, stdin-only HMAC helper, tests and CI changes are already present on main; there is no remaining PR port for those changes.
- MiniUPnP source fixes now cover allocation-safe rule snapshots, lease record key matching and persistence error propagation, descriptor metadata staging, NAT/filter compensation, IPTC ownership, and common teardown on fatal event-loop select failure.
- THIS COMMIT fixes a build-blocking duplicate `static int` specifier before `lease_file_remove()` in `trunk/user/miniupnpd/miniupnpd-2.x/upnpredirect.c` and adds a source-contract assertion to `scripts/check_miniupnpd_lease_file_cleanup.py`.
- Parent source baseline `545d8ca3ddbe85e95271ed5e21a3990e81f2bc76` had exact-head MiniUPnP #44798143654 PASS and XY-C1 #44798143782 PASS, including Build, TRX verification and artifact upload. Subsequent main commit `d1e668dc294601fcc4ce5ecd45241aa470236409` changed infosvr production code; AI handoff #44798836961 PASS, while infosvr #44798836931, Process #44798836880 and NVRAM #44798836998 were in progress at last check.
- The XY-C1 workflow currently has no path filter entry for `trunk/user/infosvr/src/infosvr.c`. The file changed on `d1e668dc` but no XY-C1 push run was scheduled for that SHA. This is an identified validation-trigger gap; fix it in a separate workflow/checker milestone.
- Exact-head MiniUPnP and XY-C1 runs are triggered by this commit's MiniUPnP production and checker paths. No result on an older SHA proves this commit. Hardware L4 remains NOT VERIFIED.

## Active task

1. Close the missing XY-C1 firmware-validation path coverage for the infosvr production source changed on `d1e668dc`. Inspect `scripts/check_xy_c1_firmware_inputs.py`, its tests, workflow path filters and build-input manifest so the workflow both triggers and correctly classifies `trunk/user/infosvr/src/infosvr.c` as a product firmware input. Add a regression proving this file schedules the firmware build. Avoid modifying Session Rotator.
2. Continue MiniUPnP persistence transaction review in `upnp_redirect()` and `_upnp_delete_redir()`: firewall changes currently precede lease-file writes/removals. Read all callers and rollback helpers, then implement only if both runtime and persistent state can be restored safely on failure. Add an injectable GNU89 regression and source-order contract.
3. Continue teardown/resource reliability work across Padavan userland; do not repeat the listed completed NAT/filter rollback, lease metadata, OOM snapshot, or handle-ownership fixes.

## Verification boundary

- Local focused declaration tests PASS: the duplicated declaration is rejected by GCC GNU89/Werror; the corrected declaration compiles. The new checker regex positive/negative fixtures PASS. The full MiniUPnP production translation unit was not built locally.
- Parent `545d8ca3ddbe85e95271ed5e21a3990e81f2bc76`: MiniUPnP #44798143654 SUCCESS and XY-C1 #44798143782 SUCCESS (Build, TRX verification, artifact upload). This does not validate later main commits.
- Parent `d1e668dc294601fcc4ce5ecd45241aa470236409`: AI handoff #44798836961 SUCCESS; infosvr #44798836931, Process #44798836880 and NVRAM #44798836998 were PENDING/in progress at last observation. No XY-C1 run was triggered for its infosvr source change.
- MiniUPnP and XY-C1 exact-head push Actions must complete successfully on THIS COMMIT before closing this source slice at L2/L3. L3 means actual Build XY-C1 + TRX verification + artifact upload.
- MIPS/uClibc runtime outside the firmware build and physical-router L4 remain NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch current main and inspect the exact-head MiniUPnP regression and XY-C1 Build/TRX/artifact runs triggered by THIS COMMIT. Diagnose any failing job from its causal log.
2. Implement the standalone infosvr-to-XY-C1 path-filter/input-classifier regression. Confirm the workflow dispatches and classifies `trunk/user/infosvr/src/infosvr.c` as a firmware input, not merely as a host-only regression path.
3. Investigate `upnp_redirect()` and `_upnp_delete_redir()` lease/firewall transaction ordering, with callers, failure edges and rollback behavior in scope before any code change.
4. Continue other independent stability/security slices while CI is pending. Preserve main-only atomic CAS commits and keep physical L4 explicitly unverified.

## Handoff maintenance

- Keep `dev_log.md` newest-first. Archive only when the 120-record rotation threshold is reached, retaining the newest 20 records.
- Update the roadmap only when module priority/status/evidence changes materially; do not copy it into history on routine updates.
- Host regressions, exact-source firmware L3 and physical hardware L4 are separate verification levels.
