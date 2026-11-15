# Cross-session execution mandate

Recover live repository state once, then continue the active engineering task. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task: create all blobs, one complete Git tree and one commit parented to freshly fetched main; update main with exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Preserve the frozen backup and archived log shards/index. A handoff checkpoint is mandatory at session end; document edits are conditional on material status changes.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Parent main for the current boot-service status aggregation fix: `3236230989b599b5d34ae7cdd8aa5e44178cf0d6`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; unchanged/read-only.
- Exact-parent Process lifecycle #43772230234 PASS. Its steps include GNU89/ASan/UBSan policy tests, production integration checks, and the RC logger status reducer regression. Exact-parent AI handoff docs #43772230258 PASS.
- Exact-parent XY-C1 #43772230305 was still building XY-C1 at last poll; TRX validation and artifact upload were pending. No L3 claim for SHA 323.
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

- Parent main for THIS boot-service slice: `3236230989b599b5d34ae7cdd8aa5e44178cf0d6`; source/test/checker/workflow and handoff/roadmap/log are one atomic tree and commit.
- Exact-parent Process lifecycle #43772230234 PASS; exact-parent AI handoff docs #43772230258 PASS.
- Exact-parent XY-C1 #43772230305 was in progress at last poll (Build XY-C1); TRX verification and artifact upload had not completed. No L3/L4 claim.
- Exact-head Process lifecycle must execute the new boot-service reducer and integration assertions. Exact-head XY-C1 Build, TRX verification and artifact upload are required for L3.
- No local cross-build or physical router test is claimed.

## Exact first action for the next GPT

1. Fetch live `main`, the frozen backup and exact-head Actions once. Confirm the SHA created by THIS COMMIT.
2. Inspect exact-head Process lifecycle run for the new boot-service policy, source checker and logger policy. Fix a real failure; do not repeatedly inspect already-passed slices.
3. Inspect exact-head XY-C1 Build, TRX verification and artifact upload; only claim L3 if all pass on the same source SHA.
4. If either workflow is pending, continue independent P1 work: residual direct legacy `pids()` consumers and name-wide signals, then `stop_services()`/shutdown status propagation and Samba/storage/NVRAM postconditions.
5. Preserve frozen backup and log archive inventory. No L4 without physical-router evidence.
