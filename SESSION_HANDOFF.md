# Cross-session execution mandate

Recover live repository state once, then continue the active engineering task. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task: create all blobs, one complete Git tree and one commit parented to freshly fetched main; update main with exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Preserve the frozen backup and archived log shards/index. Mandatory handoff checkpoint: The checkpoint is mandatory; a document edit is conditional.
Before context exhaustion, record exact refs, Action states and the first next action.
Only edit it when the live facts or first next action change materially.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Current main parent for this validation repair: `aebfd65ccf7cef183e6a2432c45866d273c17b8b`; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only).
- Exact aebfd65 Actions PASS: USB storage #43775390744 (including Transmission/Aria PID-state host + firmware BusyBox 1.24.2 ash regressions), Process inventory #43775390816, Process lifecycle #43775390764, AI handoff docs #43775390851, infosvr #43775390910, AliDDNS #43775390953 and AliyunDrive #43775390850.
- Exact aebfd65 AdGuardHome #43775390873 PASS. NVRAM #43775390849 and XY-C1 firmware validation #43775390773 were IN_PROGRESS at the last poll; no L3 for aebfd65 until its own XY-C1 Build, TRX and artifact steps pass.
- Exact 2c16 Actions for GPIO #43775134023, Crond #43775133995, watchdog #43775134027, Process inventory/lifecycle #43775134102 and AI docs #43775134055 PASS. Boot-service first-failure source fix is d444 `d444b7bbe4fe2aaacd424f97640a05672ec60b16`; follow-up checker corrections through 2c16 are now validated.
- Transmission and Aria startup process-state fix landed in aebfd65: only `pidof` status 1 + empty output is ABSENT; status 0 requires a nonempty list of valid positive canonical PIDs; contradictory, malformed and command-error states fail closed. Exact USB storage #43775390744 PASS on host and BusyBox 1.24.2 ash; L2 verified for that shell regression.
- SkipDBv2 #43773526771 previously failed its pre-build source contract because the workflow still expected bare `start_gpio_btn();` after boot startup status aggregation. This commit updates the expected call to the actual reducer wrapper; CMake/build/CTest were never reached on that historical failed run.
- No physical-router/L4 validation is claimed.
## Active task

### Repair the SkipDBv2 integration contract after boot-service status aggregation

The SkipDBv2 pre-build source map still required the obsolete direct `start_gpio_btn();` call. Production now wraps the GPIO starter in `rc_first_service_start_status(start_status, start_gpio_btn())` so its failure is propagated with the first enabled boot-service failure. This commit updates the workflow assertion to verify that actual call shape without weakening the requirement that GPIO startup remains part of the SkipDB firmware integration. The Process lifecycle workflow's accidental pull_request trigger is also removed; exact main-push workflows remain the validation path.
## Verification boundary

- Exact aebfd65 USB storage #43775390744 PASS. This includes Transmission and Aria startup process-state regressions under host shell and firmware BusyBox 1.24.2 ash, alongside the rest of the storage regression suite.
- Exact aebfd65 Process inventory #43775390816, Process lifecycle #43775390764, AI docs #43775390851, infosvr #43775390910, AdGuardHome #43775390873, AliDDNS #43775390953 and AliyunDrive #43775390850 PASS.
- Exact aebfd65 NVRAM #43775390849 and XY-C1 #43775390773 were in progress at last inspection. No current-source L3 until that XY run completes Build, TRX verification and artifact upload on the same SHA.
- This repair changes only the SkipDB workflow contract, Process workflow trigger and status documentation. Exact-head SkipDB, Process, and AI docs Actions are required. No physical-router/L4 validation is claimed.

## Exact first action for the next GPT

1. Inspect exact-head SkipDBv2, Process lifecycle and AI handoff Actions after this commit. The SkipDB preflight must pass on the reducer-wrapped GPIO call and proceed to its actual configured CMake/build/CTest stages.
2. Recheck exact aebfd65 NVRAM #43775390849 and XY-C1 #43775390773; claim L3 only if that same source SHA passes Build XY-C1, TRX verification and artifact upload.
3. Continue broad P1 work after these gates without re-auditing completed storage, boot reducer or service-state slices. Preserve the frozen backup and archived log shards/index. No L4 without physical-router evidence.
