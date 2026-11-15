# Cross-session execution mandate

Recover live repository state once, then continue the active engineering task. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task: create all blobs, one complete Git tree and one commit parented to freshly fetched main; update main with exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Preserve the frozen backup and archived log shards/index. Mandatory handoff checkpoint: The checkpoint is mandatory; a document edit is conditional.
Before context exhaustion, record exact refs, Action states and the first next action.
Only edit it when the live facts or first next action change materially.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Live main before THIS COMMIT: `5cbf3a52a89a98646f195be34d9fb162c72b2bb3`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; unchanged/read-only.
- Exact 5cb Actions PASS: AI handoff docs #43774559261, Process inventory/lifecycle #43774559266, infosvr #43774559287, AliDDNS #43774559327, AliyunDrive #43774559248, AdGuardHome #43774559217 and NVRAM #43774559233.
- Boot-service aggregation source fix is `d444b7bbe4fe2aaacd424f97640a05672ec60b16`; correction commits `0e27e66a60d7fda0e5136022ca1270426e931e57`, `118008748d0a076d58b7886ab71fb370482a71ef` and `5cbf3a52a89a98646f195be34d9fb162c72b2bb3` repaired the process checker, captured networkmap-status expectation, XY-C1 trigger and handoff contract. Exact 5cb Process and AI docs checks both PASS.
- Exact 0e27 XY-C1 Build workflow #43773995690 was still IN_PROGRESS at the last inspection; the workflow is now push-only. No exact-5cb Build/TRX/artifact result is available yet. Earlier parent 323 XY-C1 #43772230305 passed Build, TRX verification and artifact upload, but that evidence applies only to SHA 323.
- Exact d444 GPIO #43773527047, Crond #43773526834 and watchdog #43773527200 failed only because their source-contract assertions still expected bare start calls. The actual service starters are wrapped by `rc_first_service_start_status()`; THIS COMMIT aligns those assertions with the real aggregated call form.
- Exact d444 SkipDBv2 #43773526771 failed in its large inline source-contract step after the configured-path checks printed OK. The run log is truncated before the failing post-loop assertion; CMake configure, build and CTest were skipped. Keep this isolated as unresolved and do not claim a C/CTest failure.
- No exact-current-branch L3 firmware evidence or physical-router/L4 validation is claimed.
## Active task

### Preserve boot service startup failures across the RC init boundary

`start_services_once()` previously ignored return values from multiple status-returning start functions (HTTPD, telnetd, optional SSHD, VPN, watchdog, GPIO button, infosvr, UPnP, LLTD, crond, networkmap and rstats), then always returned zero. Its only RC caller ignored the return value too. Some service helpers use 1 as a disabled-feature sentinel, so blindly aggregating every result would falsely report disabled LLTD, crond, rstats or infosvr as failures.

THIS COMMIT adds a GNU89 first-nonzero policy, preserves the first actual failure while still attempting all later startup actions, aggregates existing optional-app/command statuses, and has the RC caller log a single summary when any enabled boot action failed. LLTD, crond, rstats and infosvr are aggregated only when their matching NVRAM feature setting is enabled. Void-returning 802.1X, udpxy, xupnpd and watchdog-CPU calls remain outside this slice.

Changed source: `trunk/user/rc/src/services.c` and `trunk/user/rc/src/rc.c`. Added `service_start_status.h` and a GNU89 policy regression. Extended `check_process_lifecycle_integration.py` and both Process lifecycle / XY-C1 workflow path filters.

## Completed validation slice — boot-service aggregation call contracts

GPIO, Crond and watchdog integration checks now require their boot start calls to appear as arguments to `rc_first_service_start_status(start_status, ...)`, matching the production implementation while still requiring each service to start. Exact 5cb Process lifecycle #43774559266 PASS already covers the production aggregate and GNU89 reducer; THIS COMMIT adds the three corrected checker contracts, whose own exact-head Actions must pass.

## Completed source slice — RC logger startup results

`start_logger()` previously discarded both `start_syslogd()` and `start_klogd()` results and always returned zero, making existing syslog-restart/time-update failure gates ineffective. It now attempts both daemons, logs each failure and returns the first nonzero launch status. The reducer policy and production integration regression passed in exact Process lifecycle run #43772230234 on SHA 323.

## Completed source slice — Aria2 and Transmission stop lifecycle

Aria and Transmission stop logic validates PID inventories, rejects malformed process IDs, sends SIGKILL one PID at a time, verifies final process absence and prevents restart after failed stop. Exact USB storage run #43771300948 passed host shell and firmware BusyBox 1.24.2 ash regressions. Do not repeat without a new failure.

## Completed source slice — IGMP proxy config-token validation

IGMP proxy config publication uses checked same-directory staging/atomic replacement and rejects whitespace/control/comment injection in interface and altnet tokens. Exact Process lifecycle #43770376409 passed on parent 662. Preserve the Linux 4.4 PID/start_time check-to-kill race as an explicit residual limit; don't claim atomic kernel identity signaling.

## Completed source slice — MSD Lite configuration publication

The APP_MSD_LITE branch in `start_udpxy()` now stages config and checks publication before enabling or launching the daemon. GNU89 sanitizer regression and Process lifecycle/XY-C1 Build/TRX/artifact passed on exact source SHA `b488a561bb91df62aca810a6ff2c191c17f82b2b`. This is historical L3 evidence for that SHA only, not for current main.

## Verification boundary

- Parent for THIS COMMIT: `5cbf3a52a89a98646f195be34d9fb162c72b2bb3`; this slice updates GPIO, Crond and watchdog integration contracts plus the handoff/roadmap/dev log only.
- Exact parent 5cb AI handoff docs #43774559261 and Process lifecycle #43774559266 PASS. infosvr #43774559287, AliDDNS #43774559327, AliyunDrive #43774559248, AdGuardHome #43774559217 and NVRAM #43774559233 PASS.
- GPIO #43773527047, Crond #43773526834 and watchdog #43773527200 previously failed on stale bare-call assertions; THIS COMMIT changes those expectations to the actual aggregator call. Exact-head runs are required before closing these checks.
- Corrected XY-C1 push-only run #43773995690 is for SHA 0e27, not current main; it was still in progress at last inspection. Parent 323 XY-C1 #43772230305 PASS is historical Build/TRX/artifact evidence only.
- SkipDBv2 #43773526771 remains unresolved in its large inline source-contract step; output was truncated before the final failure text and all later build/test steps were skipped.
- No exact-current L3 or L4 evidence is claimed.
## Exact first action for the next GPT

1. Fetch live main and the frozen backup once; inspect exact-head GPIO, Crond, watchdog and AI handoff Actions for THIS COMMIT.
2. Recheck corrected XY-C1 Build/TRX/artifact run #43773995690 on SHA 0e27; do not claim current-main L3 from this older run.
3. Isolate SkipDBv2 source-contract failure #43773526771 from its truncated log, preferably by a targeted reproduction or by splitting the oversized inline checker into a normal script with complete failure output. Do not guess or bypass the assertion.
4. Continue the P1 Transmission/Aria startup process-state defect and shared host + BusyBox 1.24.2 ash regression; preserve the frozen backup. No L4 without physical-router evidence.