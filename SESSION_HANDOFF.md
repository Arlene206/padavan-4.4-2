# Cross-session execution mandate

Recover live repository state once, then continue the active engineering task. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task: create all blobs, one complete Git tree and one commit parented to freshly fetched main; update main with exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Preserve the frozen backup and archived log shards/index. Mandatory handoff checkpoint: The checkpoint is mandatory; a document edit is conditional.
Before context exhaustion, record exact refs, Action states and the first next action.
Only edit it when the live facts or first next action change materially.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Parent for THIS COMMIT: `2c16e60838bcc066e34d5e7e8b70fae2f02a088d`; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Exact parent 2c16 Actions (source/test CI-contract repair): GPIO #43775134023, Crond #43775133995, watchdog #43775134027, Process #43775134102, AI docs #43775134055, infosvr #43775134174, AliDDNS #43775134052, AdGuardHome #43775134050, NVRAM #43775134029 and AliyunDrive #43775134000 were all IN_PROGRESS at last poll.
- The 2c16 commit changes only three integration assertions: GPIO and Crond helper scripts now assert their production boot calls are passed through `rc_first_service_start_status()`; watchdog inline contract does the same. Their exact-head Actions will establish PASS/FAIL.
- Prior exact 5cb AI docs #43774559261 and Process #43774559266 PASS. Prior 5cb service runs including infosvr #43774559287, AliDDNS #43774559327, AliyunDrive #43774559248, AdGuardHome #43774559217 and NVRAM #43774559233 PASS.
- The P1 Transmission/Aria start-state defect below is not yet fixed; this commit contains the first source/test implementation. Exact-head USB storage workflow run is required to exercise host and firmware BusyBox 1.24.2 ash.
- SkipDBv2 #43773526771 remains unresolved in its large inline source-contract step; output was truncated before the final failure text, and CMake/Build/CTest were skipped. No source or C/CTest root cause has been established.
- Earlier parent 323 XY-C1 #43772230305 passed Build, TRX verification and artifact upload; that L3 evidence applies only to SHA 323. No L3 for current sources and no physical-router/L4 validation are claimed.
## Active task

### Fail closed on unknown Transmission and Aria startup process state

Both launchers previously treated empty `pidof` output as proof that the daemon was absent. A failed process query or contradictory status/output pair could therefore launch a duplicate daemon. THIS COMMIT captures both the `pidof` exit status and output: only status 1 with empty output is ABSENT; status 0 is RUNNING only with a nonempty list of valid positive canonical PIDs. Query errors, status/output contradictions, malformed PID tokens, whitespace-only output and PID 0/1 fail closed before any storage setup or daemon launch.

The shared `storage_app_start_status.sh` regression covers both launchers for each state under the existing host-shell and firmware BusyBox 1.24.2 ash workflow. Exact-head USB storage CI is required before claiming L2.
## Completed source slice — Transmission and Aria fail-closed startup process-state checks

Previously both scripts trusted captured `pidof` stdout while ignoring command status; an unavailable query could be misinterpreted as daemon absence. This commit makes only status 1 plus empty output permit startup. Status 0 requires one or more positive canonical PID tokens and returns as already running; every contradictory, malformed or error state exits before storage configuration or launch. The shared test now covers error, empty-success, error-with-output, absent-with-output, malformed/whitespace/unsafe PID and valid existing multi-PID states for both services. Exact-head USB storage must pass on both host shell and firmware BusyBox 1.24.2 ash before L2 is claimed.

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

- Parent for THIS COMMIT: `2c16e60838bcc066e34d5e7e8b70fae2f02a088d`; source changes are limited to the Aria/Transmission startup prechecks and their shared host/BusyBox regression.
- The preceding Aria/Transmission stop-postcondition slice passed exact USB storage #43772230269 on parent 323, including POSIX host and firmware BusyBox 1.24.2 ash.
- Exact parent 2c16 GPIO #43775134023, Crond #43775133995, watchdog #43775134027, Process #43775134102 and AI docs #43775134055 were IN_PROGRESS at last poll; these are for the prior validation-contract commit, not this source change.
- This commit has not yet run exact-head USB storage; no test result is claimed until that workflow runs on its SHA. No firmware build/TRX/artifact or L3, and no L4 physical-router test, is claimed.
## Exact first action for the next GPT

1. Inspect exact-head USB storage # for THIS COMMIT, including POSIX host and firmware BusyBox 1.24.2 ash startup-process-state regressions for both launchers; diagnose any failure before claiming L2.
2. Inspect exact-head Actions for the prior 2c16 CI-contract commit and close any failure in GPIO, Crond, watchdog, Process or AI docs.
3. Isolate SkipDBv2 #43773526771 with a targeted reproduction or a split checker that preserves all assertions and exposes the complete failure output; do not bypass the unknown assertion.
4. Continue broad user-space P1 work after these checks; preserve the frozen backup, and do not claim L3/L4 without exact firmware artifact or physical-router evidence.