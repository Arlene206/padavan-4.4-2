## 2026-11-28 continuation checkpoint

- Repository: `Arlene206/padavan-4.4-2`. Continue on `main`; startup recovery was completed once. Do not repeat the completed bootstrap/inventory audit.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14`, SHA `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. No force-push, reset, rename, CI, or development work on this branch.
- The latest source parent for this checkpoint is `19d86adc5f57d17800a687435050b294affb6ade`; this commit extends its firmware-stop first-errno work. Fetch live `main` to resolve the resulting exact commit SHA before checking Actions.
- PR #1 remains closed and unmerged. Its AliDDNS CA trust-store postcondition source, shell regression and workflow integration are present on main; do not resume development on the closed PR branch or assume its legacy branch is current.

## Exact-SHA validation already established

- DDNSTO production SHA `3f1cab20589e858b2db951b18821194249424957`: XY-C1 run #46141285573 completed SUCCESS; firmware Build, TRX verification and artifact upload all SUCCESS; artifact ID `15261971334`.
- Aria2 production SHA `76646cffc52d09fcb9b7a67052d5ae64b1b14462`: XY-C1 run #46141026720 completed SUCCESS; firmware Build, TRX verification and artifact upload all SUCCESS; artifact ID `15261729486`.
- PPPoE-MWAN detector commit SHA `300e78b2a36ceea51d508d18bc3cf09add6d633d`: WAN route-status run #46141847656 SUCCESS; XY-C1 run #46141847644 was IN_PROGRESS at the last check. Its PPPoE-MWAN shell tests, static checker and other host pre-build tests were SUCCESS; Build XY-C1 was still in progress, TRX and artifact steps pending.
- Firmware first-errno policy commit SHA `19d86adc5f57d17800a687435050b294affb6ade`: PID 1 notification run #46142248483 SUCCESS; Process lifecycle regression #46142248545 SUCCESS (including the first-errno policy test and firmware notification checker); Process inventory #46142248530 SUCCESS. XY-C1 #46142248550 remained IN_PROGRESS; its first-errno test and firmware notification checker completed SUCCESS; Build XY-C1 was in progress, TRX and artifact pending.
- No local MIPS/kernel build or router hardware is available in this session. Physical L4 remains NOT VERIFIED. Do not claim L3 for a SHA until its exact-SHA Build, TRX check and artifact upload all succeed.

## Current repository state

- Main-only development. The frozen backup remains unchanged.
- `flash_firmware()` now routes all ten independent pre-flash stop operations through `firmware_update_stop_first_errno()`: VPN client, VPN server, residual L2TP/PPP process cleanup, VPN lifecycle-lock release, MiniUPnPd, checked service stop, EAPOL, LAN authentication, 802.1X and PPPoE relay.
- Each operation clears errno before it runs; the first nonzero failure errno is retained, defaulting to EIO when a failing operation leaves errno unset. Independent stop attempts continue; failure is logged and errno restored before returning. Persistence/MTD write remains unreachable after any stop failure.
- The GNU89 policy regression covers successful operations with stale errno, operation errno capture, EIO fallback and sticky first error. The Process lifecycle and XY-C1 workflows compile/run it with sanitizers and execute the extended firmware-update source checker.
- Root `dev_log.md` has 98 records before this commit and is below archive thresholds; roadmap is near its size cap and was not edited.

## Active task

- Validate this firmware-stop errno follow-up on its exact main SHA. Require the GNU89 policy test, source checker, Process lifecycle regression, PID 1 notification workflow and XY-C1 pre-build checks to pass.
- Complete the exact-SHA XY-C1 firmware Build, TRX verification and artifact upload before claiming L3.
- In parallel, finish or diagnose the PPPoE-MWAN detector XY-C1 run #46141847644 on SHA `300e78b2a36ceea51d508d18bc3cf09add6d633d`; its host tests already passed.

## Exact first action for the next GPT

1. Fetch live `main`, identify this commit's exact SHA, and inspect all Actions triggered by that SHA.
2. Confirm `firmware_update_stop_policy.c` and `check_firmware_update_notification.py` pass in Process/XY-C1; confirm the PID 1 notification workflow remains green.
3. Inspect XY-C1 Build, TRX and artifact steps for the exact current SHA. A pending firmware build does not block other independent fixes; a failure requires its exact job log and causal repair.
4. Poll #46141847644 independently for the PPPoE-MWAN detector SHA 300e; DDNSTO 3f and Aria2 766 have already completed full exact-SHA XY-C1 validation.
5. Continue main-only atomic changes. Do not modify the frozen backup; do not force-push or publish per-file partial commits.

## Verification boundary

- DDNSTO SHA 3f and Aria2 SHA 766: exact-SHA XY-C1 Build, TRX and artifact upload PASS.
- PPPoE-MWAN detector SHA 300e: source/host checks PASS; exact-SHA firmware build/TRX/artifact not complete at last poll.
- Firmware-stop first-errno SHA 19d86: PID 1 notification, Process lifecycle, Process inventory and pre-build policy checks PASS; exact-SHA XY-C1 firmware Build/TRX/artifact still pending at last poll.
- This follow-up commit: Actions are not yet available at handoff authoring time; fetch exact-head results.
- Physical router/hardware validation L4: NOT VERIFIED.

## Mandatory end-of-session handoff checkpoint

Every session MUST inspect live repository state and exact-head Actions before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. Update this file when the active task or first action materially changes; do not create a documentation-only commit merely to record routine CI transitions.

## Cross-session execution mandate

- One logical task, one atomic main commit: create all blobs, the complete tree, one commit, then a non-forced expected-SHA update. Never force-push or publish per-file partial commits.
- Before context exhaustion, finish safe source slices, inspect exact-head Actions, and leave the next GPT an accurate first action and L1-L4 verification boundary.
- Keep working while independent Actions are pending; PASS continues, FAIL requires causal repair, PENDING permits independent work.
- Recover once, implement continuously, do not repeat completed audits, and do not ask what to do next.
