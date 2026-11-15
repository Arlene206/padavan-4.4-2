# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create all source/test/doc blobs, one complete Git tree and one commit parented to freshly fetched main; update main with exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Preserve frozen backup and archived log shards/index. Before context exhaustion, finish a safe bounded slice and leave the exact first action for the next session.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session records exact main/backup SHAs, source changes, exact-SHA Action IDs/statuses, validation boundary and first next action before ending. Do not create a no-op handoff commit when the checkpoint is accurate.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Parent main for the RC logger startup-status slice: `828af0d6423994cfb798b6e473bc295bd009b1b8`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; unchanged/read-only.
- Exact-parent USB storage #43771300948 PASS: Aria and Transmission shell syntax/start-stop regressions passed under host shell and firmware BusyBox 1.24.2 ash, as did the complete storage caller/eject/persistence suite.
- Exact-parent Process #43771301002, Process inventory #43771301076, AI docs #43771300969, AdGuardHome #43771300992, AliyunDrive #43771300907, AliDDNS #43771300910, infosvr #43771301013 and NVRAM #43771300993 PASS.
- Exact-parent XY-C1 #43771300972 was in progress (Build XY-C1; TRX/artifact pending). No L3 for parent 828.
- THIS COMMIT fixes a separate P1 RC defect: `start_logger()` discarded syslogd/klogd launch results and returned zero, defeating existing syslog-restart/time-update failure gates.
- No physical-router/L4 validation is claimed.

## Active task

### Return truthful status from RC logger startup

`start_syslogd()` returns `_eval()` status and `start_klogd()` returns `eval()` status, but `start_logger()` ignored both and unconditionally returned zero. Existing syslog-restart and time-update notification paths gate on `start_logger(0) != 0`, so failures were never propagated.

THIS COMMIT records both statuses, attempts both daemons regardless of the first result, logs each nonzero status and returns the first failure. A GNU89 reducer test covers individual/combined and negative statuses; the process-lifecycle integration checker verifies the production call order and return contract. Exact-head Process lifecycle and XY-C1 Build/TRX/artifact Actions are required for this source.

The immediately preceding Aria/Transmission stop slice is L2 verified by exact USB storage run #43771300948 (host + firmware BusyBox 1.24.2 ash). Do not re-audit it.

## Completed source slice — IGMP proxy config-token validation

The IGMP proxy publisher already staged its configuration and checked writes/flush/fsync/close/rename. It now rejects whitespace/control/comment markers in interface tokens and restricts altnet values to digits, dots and slashes, preventing same-line token/comment injection from NVRAM values. Exact Process lifecycle #43770376409 PASS on parent SHA 662; exact 662 XY-C1 #43770376467 was still in progress at last poll.

## Completed source slice — Transmission launcher lifecycle

Transmission's shell stop now validates each PID, signals individual processes, confirms absence after forced termination and gates restart. Its PID-unique config stage and ownership-aware cleanup remain. Exact abd40 USB storage #43769497238 PASS across host and firmware BusyBox ash; no L3 claim for newer source SHA.

## Completed source slice — MSD Lite configuration publication

The APP_MSD_LITE branch in `start_udpxy()` previously truncated `/etc/storage/msd_lite.conf` before validating the template and ignored read/write/flush/close failures. It now uses checked same-directory staged publication and aborts before enabling or launching the daemon on failure.

Changed: `trunk/user/rc/src/net.c`; new `trunk/user/rc/src/msd_lite_config_file.h` and its host regression; Process lifecycle integration and XY-C1 workflow filters. GNU89 ASan/UBSan regression passed. The exact Process lifecycle host regression and XY-C1 Build/TRX/artifact workflow passed for source SHA `b488a561bb91df62aca810a6ff2c191c17f82b2b`; L3 applies only to that exact source revision. No L4.

## Verification boundary

- Parent main: `828af0d6423994cfb798b6e473bc295bd009b1b8`; this atomic commit contains RC source, GNU89 policy test, process-lifecycle checker, both workflow filters/test wiring and required handoff/roadmap/log changes.
- Exact-parent USB storage #43771300948 PASS: Aria and Transmission host + firmware BusyBox 1.24.2 ash launcher tests and full storage checks.
- Exact-parent Process #43771301002, inventory #43771301076, AI docs #43771300969, AdGuardHome #43771300992, AliyunDrive #43771300907, AliDDNS #43771300910, infosvr #43771301013 and NVRAM #43771300993 PASS.
- Exact-parent XY-C1 #43771300972 was still in progress at last inspection. Exact-head Process and XY-C1 Build/TRX/artifact must pass; no L3/L4 claim.

## Exact first action for the next GPT

1. Inspect exact-head Process lifecycle and XY-C1 Actions for THIS logger commit. Diagnose any failure; claim L3 only if Build XY-C1, TRX verification and artifact upload pass on the same SHA.
2. If Actions remain pending, continue the broad P1 service-lifecycle queue without waiting: remaining enabled RC/service startup status boundaries, then residual direct `pids()` consumers and firmware `stop_services()`/shutdown/Samba/storage/NVRAM postconditions.
3. Do not re-audit Aria/Transmission stop paths (exact USB #43771300948 PASS), IGMP input-token validation or completed MSD Lite publication.
4. Preserve the frozen backup and archived log inventory. No L4 without physical-router evidence.

1. Inspect exact-head USB storage Action for POSIX shell and firmware BusyBox 1.24.2 ash runs: both launchers' absent/graceful/multi-PID/failed-signal/malformed PID/pidof-unknown/restart tests, config-generation faults and the complete storage caller graph must pass.
2. Inspect exact-head XY-C1 Build, TRX verification and artifact upload; claim L3 only if all stages pass on this exact source SHA.
3. Continue the next uncompleted P1 broad user-space stability/security task in the roadmap; do not repeat the completed Aria/Transmission stop audit, IGMP input-token validation or prior MSD Lite slice.
4. Preserve the frozen backup and archive inventory. No L4 without physical-router evidence.

