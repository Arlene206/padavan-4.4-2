# Cross-session execution mandate

Recover live repository state once, then continue the active engineering task. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task: create all blobs, one complete Git tree and one commit parented to freshly fetched main; update main with exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Preserve the frozen backup and archived log shards/index. Mandatory handoff checkpoint: The checkpoint is mandatory; a document edit is conditional on material status changes.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Current source commit: `d444b7bbe4fe2aaacd424f97640a05672ec60b16` (parent `3236230989b599b5d34ae7cdd8aa5e44178cf0d6`).
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; unchanged/read-only.
- Parent SHA `3236230989b599b5d34ae7cdd8aa5e44178cf0d6`: Process lifecycle #43772230234 PASS and AI handoff docs #43772230258 PASS. XY-C1 #43772230305 was still building at last poll.
- Exact current source SHA `d444b7bbe4fe2aaacd424f97640a05672ec60b16`: Process lifecycle #43773527020 FAIL only at the new source-integration extraction assertion (Python string contained an over-escaped newline); the new GNU89 boot-service reducer and logger tests had already passed. XY-C1 #43773526843 FAIL at the pre-build Samba shared-header contract because the workflow must remain push-only; the accidental pull_request trigger is removed. AI docs #43773527100 FAIL because the handoff missed the validator's required checkpoint wording; that exact wording is now restored.
- Exact USB storage #43771300948 PASS on parent 828; Transmission and Aria lifecycle tests passed under host shell and firmware BusyBox 1.24.2 ash. Do not repeat those source slices absent a new regression.
- The preceding logger fix in SHA 323 makes `start_logger()` attempt both syslogd/klogd, logs both launch failures and returns the first nonzero status. Exact Process #43772230234 PASS includes its reducer and source-integration regression.
- No physical-router/L4 validation is claimed.

## Active task

### Preserve boot service startup failures across the RC init boundary

`start_services_once()` previously ignored return values from multiple status-returning start functions (HTTPD, telnetd, optional SSHD, VPN, watchdog, GPIO button, infosvr, UPnP, LLTD, crond, networkmap and rstats), then always returned zero. Its only RC caller ignored the return value too. Some service helpers use 1 as a disabled-feature sentinel, so blindly aggregating every result would falsely report disabled LLTD, crond, rstats or infosvr as failures.

THIS COMMIT adds a GNU89 first-nonzero policy, preserves the first actual failure while still attempting all later startup actions, aggregates existing optional-app/command statuses, and has the RC caller log a single summary when any enabled boot action failed. LLTD, crond, rstats and infosvr are aggregated only when their matching NVRAM feature setting is enabled. Void-returning 802.1X, udpxy, xupnpd and watchdog-CPU calls remain outside this slice.

Changed source: `trunk/user/rc/src/services.c` and `trunk/user/rc/src/rc.c`. Added `service_start_status.h` and a GNU89 policy regression. Extended `check_process_lifecycle_integration.py` and both Process lifecycle / XY-C1 workflow path filters.

## Completed source slice — RC logger startup results

`start_logger()` previously discarded both `start_syslogd()` and `start_klogd()` results and always returned zero, making existing syslog-restart/time-update failure gates ineffective. It now attempts both daemons, logs each failure and returns the first nonzero launch status. The reducer policy and production integration regression passed in exact Process lifecycle run #43772230234 on SHA 323.

## Completed source slice — Aria2 and Transmission stop lifecycle

Aria and Transmission stop logic validates PID inventories, rejects malformed process IDs, sends SIGKILL one PID at a time, verifies final process absence and prevents restart after failed stop. Exact USB storage run #43771300948 passed host shell and firmware BusyBox 1.24.2 ash regressions. Do not repeat without a new failure.

## Completed source slice — IGMP proxy config-token validation

IGMP proxy config publication uses checked same-directory staging/atomic replacement and rejects whitespace/control/comment injection in interface and altnet tokens. Exact Process lifecycle #43770376409 passed on parent 662. Preserve the Linux 4.4 PID/start_time check-to-kill race as an explicit residual limit; don't claim atomic kernel identity signaling.

## Completed source slice — MSD Lite configuration publication

The APP_MSD_LITE branch in `start_udpxy()` now stages config and checks publication before enabling or launching the daemon. GNU89 sanitizer regression and Process lifecycle/XY-C1 Build/TRX/artifact passed on exact source SHA `b488a561bb91df62aca810a6ff2c191c17f82b2b`. This is historical L3 evidence for that SHA only, not for current main.

## Verification boundary

- Source SHA `d444b7bbe4fe2aaacd424f97640a05672ec60b16` is parented to `3236230989b599b5d34ae7cdd8aa5e44178cf0d6`; all boot-service source, policy, tests, checker/workflows and docs were one atomic commit.
- Exact d444 Process lifecycle #43773527020 FAIL at the newly appended source-extraction assertion (the policy binary itself passed). Exact d444 XY-C1 #43773526843 FAIL at the Samba pre-build check because XY-C1 is contractually push-only. Exact d444 AI docs #43773527100 FAIL only for missing exact checkpoint wording.
- This corrective commit fixes those three integration/contract failures; new exact-head Process, AI docs and XY-C1 checks remain pending. No L3/L4 claim.
- Exact-head Process lifecycle must execute the new boot-service reducer and integration assertions. Exact-head XY-C1 Build, TRX verification and artifact upload are required for L3.
- No local cross-build or physical router test is claimed.

## Exact first action for the next GPT

1. Fetch live `main`, the frozen backup and exact-head Actions once; confirm the corrective commit SHA.
2. Inspect corrected Process lifecycle # and AI handoff docs #; the Python source checker must reach its final PASS and the docs validator must pass.
3. Inspect corrected XY-C1 Build, TRX verification and artifact upload; only claim L3 if all pass on the same source SHA.
4. If either workflow is pending, continue independent P1 work: residual direct legacy `pids()` consumers and name-wide signals, then `stop_services()`/shutdown status propagation and Samba/storage/NVRAM postconditions.
5. Preserve frozen backup and log archive inventory. No L4 without physical-router evidence.
