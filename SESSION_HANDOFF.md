# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Confirm the main/source baseline, exact-head Actions, validation boundary, blockers, frozen backup and precise next action. The checkpoint is mandatory; a document edit is conditional. Before context exhaustion, complete this accuracy checkpoint against the live main SHA and exact-head Actions; update this file only when a fact or next action materially changed. If all checkpoint facts and the first action are already accurate, do not create a no-op commit. This is the required end-of-session accuracy checkpoint.

Continue one engineering effort across GPT conversations. Avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next. Finish safe source slices, add regression coverage, inspect exact-SHA Actions, commit atomically, and preserve the next task.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, create a development branch/PR, or modify the frozen backup.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development only on `main`.
- Current source commit: THIS COMMIT; its parent main was `06ed43217d0e47b54f17fc0416a81e7341593824`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- P1 WAN-up DDNS fix `01bc69f3`: enabled Inadyn inventory/start/config-publication/HUP failures now flow into WAN-up's first-error status; disabled DDNS and AP mode remain successful no-ops before process scan/start.
- Integration correction `0c786821`: WAN regression compile includes the shared headers; HTTPD and MiniUPnD source contracts now assert the actual status-returning PPP rollback behavior. WAN route, MiniUPnD, HTTPD, infosvr, Process lifecycle, NVRAM, AliyunDrive, AdGuardHome and AliDDNS exact-head workflows passed on `0c786821`. The AI-handoff run on that SHA failed only on a missing literal checkpoint phrase; the next documentation commit corrected it.
- Documentation checkpoint commit `06ed4321`: AI handoff, infosvr, AliDDNS, AdGuardHome, NVRAM, Process inventory/lifecycle and AliyunDrive exact-head runs all passed. The previous same-source SHA `0c786821` XY-C1 run `44026236613` subsequently passed every cross-build regression gate, Build XY-C1, TRX verification and artifact upload. The documentation-only `06ed4321` did not itself trigger XY-C1.
- THIS COMMIT fixes another P1 WAN-up status drop: when ZeroTier is enabled, `wan_up_internal()` used `doSystem("/usr/bin/zerotier.sh start")` and discarded its result despite the existing checked `start_zerotier()` RC wrapper. WAN-up now calls the wrapper, accumulates/logs its status, and preserves the disabled no-op gate. The shell regression now injects a launch failure and asserts `start_zero()` does not report success. The WAN-route and ZeroTier workflows both cover the source caller and run a dedicated caller-contract checker.
- Exact-head Actions for THIS COMMIT are PENDING until run. Require WAN route, ZeroTier lifecycle, Process lifecycle, and XY-C1 to pass at this exact SHA. For XY-C1 require Build XY-C1, TRX output verification and artifact upload; the earlier `0c786821` artifact is not evidence for this source change.
- Local checkout/toolchain is unavailable; local compilation was NOT RUN. Physical-router L4 remains NOT VERIFIED.

## Active task

1. Fetch live `main`, inspect all exact-head `push` Actions for THIS COMMIT, and resolve any failure. Verify WAN route and ZeroTier lifecycle regressions, Process checks, and same-SHA XY-C1 Build/TRX/artifact steps.
2. If CI is pending, continue the next independent P1 WAN-up caller defect: `pppoemwan_enable` launches `/usr/bin/detect.sh` via `doSystem()` but discards the result. Do not simply aggregate it yet: the current `trunk/user/pppoe/detect.sh` lacks `set -e`, does not check many iptables/ip/route operations, and ends with lock cleanup, so its return value does not reliably represent configuration success. First make the script's command/cleanup status truthful with host-mocked failure-path regression, then propagate the RC result.
3. Preserve the frozen backup. Use one complete tree, one commit and non-forced expected-SHA main update per logical slice.

## Verification boundary

- Exact parent main verified before publishing this slice: `06ed43217d0e47b54f17fc0416a81e7341593824`; live main matched.
- On `0c786821fd1bb8b571c51c1c7abf32780f481157`: WAN route `44026236589`, MiniUPnD `44026236528`, HTTPD `44026236478`, infosvr `44026236468`, Process lifecycle `44026236421`, NVRAM `44026236411`, AliyunDrive `44026236509`, AdGuardHome `44026236508`, and AliDDNS `44026236477` all passed. AI handoff `44026236466` initially failed on the missing checkpoint literal; this was corrected in `06ed4321`.
- On `06ed43217d0e47b54f17fc0416a81e7341593824`: infosvr `44026666912`, AliDDNS `44026666907`, AdGuardHome `44026666843`, NVRAM `44026666835`, AI handoff `44026666834`, Process inventory/lifecycle `44026666833`, and AliyunDrive `44026666832` passed. The source-identical `0c786821` XY-C1 run `44026236613` completed successfully through Build, TRX verification and artifact upload.
- The ZeroTier source slice in THIS COMMIT is published as one atomic change with a source-level caller checker and shell launch-failure regression. It was not locally executed because this environment has no checkout/toolchain. Exact-head CI is pending at commit creation; no new firmware success is claimed yet.
- Physical-router testing (L4) remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch the live main SHA and query Actions filtered by that exact `head_sha` and `event=push`. Inspect WAN route, ZeroTier lifecycle, Process lifecycle, and XY-C1; resolve any exact-SHA failure and confirm XY-C1 reaches Build, TRX verification, and artifact upload. If any relevant checks remain pending, investigate and begin hardening `trunk/user/pppoe/detect.sh` with failure-injection tests rather than waiting. Keep the backup immutable and update this handoff only when facts or the next action materially change.