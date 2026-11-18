# 2026-11-18 continuation checkpoint — report ZeroCD mode-switch failures

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; parent main before THIS ZeroCD status fix: `2e01dae1878bdc984326895d5f464789bdcc3d76`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; untouched.
- Exact parent `2e01dae1` USB modem workflow `44213177117` PASS. The missing SG policy include fixed in `2e01dae` is now verified together with the SG/CD sanitizer tests, source contract and event filters.
- Exact parent AI handoff `44213177260`, AdGuardHome `44213177186`, infosvr `44213177162`, AliDDNS `44213177125`, AliyunDrive `44213177140`, HTTPD Security `44213177111`, Process lifecycle `44213177094` and Process inventory `44213177132` PASS. NVRAM audit `44213177187` remained IN_PROGRESS at last poll.
- Found another false-success path in `mdev_sg_main()`: when `modem_zcd == 0` and USB port/VID/PID discovery succeeds, the handler invokes `/sbin/zerocd` with `_eval()` but discards the return value, then prints success. `_eval()` returns the child's exit code or a nonzero errno on launch/wait failure. THIS slice captures the result, maps it through a pure status policy, logs and releases the hotplug lock before returning failure. The legacy no-op when USB identity cannot be resolved remains unchanged.
- Extends the GNU89/Werror/ASan/UBSan SG policy regression and the USB source contract for ZeroCD success/failure propagation.
- No exact-current-SHA XY-C1 Build/TRX/artifact evidence; firmware L3 and physical-router L4 NOT VERIFIED.

## Active task
- Prevent `mdev_sg_main()` from claiming success after an attempted `zerocd` command fails; preserve the existing no-op when the USB path or VID/PID cannot be resolved.

## Verification boundary
- Exact parent USB modem, AI handoff, AdGuardHome, infosvr, AliDDNS, AliyunDrive, HTTPD Security, Process lifecycle and Process inventory workflows PASS.
- NVRAM audit `44213177187` was IN_PROGRESS at last observation.
- THIS ZeroCD source/policy/workflow slice's exact-head Actions are pending. Firmware L3 and physical L4 NOT VERIFIED.

## Exact first action for the next GPT
1. Inspect the exact-head USB modem workflow; require the SG readiness and ZeroCD policy cases, CD policy regression, WDM identity source contract and push/PR path filters to PASS.
2. Check NVRAM audit `44213177187` and any current exact-SHA reruns.
3. Continue an independent P1 userland storage/security issue while Actions run; check XY-C1 L3 only if Build, TRX verification and artifact upload all pass on one SHA.
4. Keep the frozen backup unchanged and publish multi-file changes atomically to main with a non-forced expected-SHA update.


## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional.
At session end inspect live main, relevant exact-SHA Actions, current handoff/roadmap, validation boundary and first actionable task. If all remain accurate, do not create a no-op documentation commit.

# Cross-session execution mandate

Continue one engineering effort across GPT conversations. Recover once, implement concrete defects, test and commit atomically; do not wait idly on pending CI.

Every logical slice is one atomic main commit: create all blobs, one complete tree and one commit parented to freshly fetched main, then publish with non-forced expected-SHA compare-and-swap. Never force-push, create development branches/PRs, or modify frozen backup.

Before context exhaustion, finish a safe slice, inspect exact-SHA Actions and leave the next GPT a precise first action.
