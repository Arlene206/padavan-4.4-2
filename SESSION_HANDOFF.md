# Cross-session execution mandate

Recover live repository state once, then continue the active engineering task. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task: create all blobs, one complete Git tree and one commit parented to freshly fetched main; update main with exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Preserve the frozen backup and archived log shards/index. Mandatory handoff checkpoint: The checkpoint is mandatory; a document edit is conditional.
Only edit it when the live facts or first next action change materially.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Current source commit: `0e27e66a60d7fda0e5136022ca1270426e931e57` (parent `d444b7bbe4fe2aaacd424f97640a05672ec60b16`).
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; unchanged/read-only.
- Parent SHA `3236230989b599b5d34ae7cdd8aa5e44178cf0d6`: Process lifecycle #43772230234 PASS and AI handoff docs #43772230258 PASS. XY-C1 #43772230305 was still building at last poll.
- Exact d444 SHA `d444b7bbe4fe2aaacd424f97640a05672ec60b16`: Process #43773527020 failed first at the over-escaped newline extraction; XY-C1 #43773526843 rejected an accidental pull_request trigger (XY-C1 is push-only); AI docs #43773527100 rejected missing exact checkpoint wording. The 0e27 correction fixed the extraction delimiter and removed the PR trigger.
- Exact 0e27 SHA `0e27e66a60d7fda0e5136022ca1270426e931e57`: Process #43773995752 got through every GNU89 policy binary and most integration assertions, then failed because the checker expected direct aggregation of `start_networkmap(1)` while production intentionally captures that status in `networkmap_start_status` for diagnostics before aggregation. This commit corrects the checker contract.
- Exact 0e27 AI docs #43773995692 failed because the handoff phrase was extended with `on material status changes`, breaking the validator's exact punctuation. This commit restores the exact required sentence. XY-C1 #43773995690 remained in progress at the last poll; the corrected workflow is push-only and retains its new helper/test path filters.
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

- Source SHA `d444b7bbe4fe2aaacd424f97640a05672ec60b16` added the boot-service status source/test/checker/workflow slice atomically. Correction SHA `0e27e66a60d7fda0e5136022ca1270426e931e57` fixed the initial checker extraction, restored XY-C1's push-only trigger and adjusted checkpoint wording.
- Exact 0e27 Process #43773995752 FAIL at the `networkmap_start_status` aggregation assertion after all GNU89 tests PASS. Exact 0e27 AI docs #43773995692 FAIL on the exact checkpoint sentence. Exact 0e27 XY-C1 #43773995690 was still in progress at last inspection.
- THIS COMMIT fixes the source-checker model of the captured networkmap status and restores the exact handoff contract sentence. New exact-head Process, AI docs and XY-C1 Actions are required; no L3/L4 claim.
- Exact-head Process lifecycle must execute the new boot-service reducer and integration assertions. Exact-head XY-C1 Build, TRX verification and artifact upload are required for L3.
- No local cross-build or physical router test is claimed.

## Exact first action for the next GPT

1. Fetch live `main` and frozen backup once; inspect exact-head Process lifecycle and AI handoff docs Actions for THIS correction. The source checker must reach final PASS and the handoff validator must pass.
2. Inspect the exact-head XY-C1 run. It must retain push-only workflow semantics, pass pre-build staged-header checks, then Build XY-C1, TRX verification and artifact upload on the same SHA before L3 can be claimed.
3. If an Action is PENDING, continue broad P1 work without waiting: residual legacy `pids()` consumers/name-wide signals, followed by `stop_services()`/shutdown and Samba/storage/NVRAM postconditions.
4. Do not repeat the completed Aria/Transmission storage regression (#43771300948) or RC logger policy tests; these passed on earlier exact SHAs. No L4 without physical-router evidence.

1. Fetch live `main`, the frozen backup and exact-head Actions once; confirm the corrective commit SHA.
2. Inspect corrected Process lifecycle # and AI handoff docs #; the Python source checker must reach its final PASS and the docs validator must pass.
3. Inspect corrected XY-C1 Build, TRX verification and artifact upload; only claim L3 if all pass on the same source SHA.
4. If either workflow is pending, continue independent P1 work: residual direct legacy `pids()` consumers and name-wide signals, then `stop_services()`/shutdown status propagation and Samba/storage/NVRAM postconditions.
5. Preserve frozen backup and log archive inventory. No L4 without physical-router evidence.
