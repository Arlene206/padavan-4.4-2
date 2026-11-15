# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create all source/test/doc blobs, one complete Git tree and one commit parented to freshly fetched main; update main with exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Preserve frozen backup and archived log shards/index. Before context exhaustion, finish a safe bounded slice and leave the exact first action for the next session.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session records exact main/backup SHAs, source changes, exact-SHA Action IDs/statuses, validation boundary and first next action before ending. Do not create a no-op handoff commit when the checkpoint is accurate.

## Current repository state

- Repository: Arlene206/padavan-4.4-2.
- Parent main for this Transmission stop hardening slice: ba51619ceea8efc084d28f6f38bb5a8bc084e8d7.
- Frozen backup branch backup/pre-plugin-overhaul-2026-11-14: 72f4f157a084f8184b9597ca5bf334b8080e5807; read-only.
- Source `trunk/user/transmission/transmission.sh` now uses the PID-qualified settings stage `settings.json.tmp.$$` (exact source blob SHA f3b4260101bec87830c86ea96e7efd737b649354). Ownership-aware cleanup is retained.
- Exact parent b516 storage #43768122780 had passed host shell syntax and startup regression (including the fixed collision path) and was building BusyBox 1.24.2 ash at last poll. Exact parent AI docs #43768122829 PASS; Process lifecycle #43768122785 PASS; Process inventory #43768122777 PASS; infosvr #43768122801 PASS; AdGuardHome #43768122808 PASS; AliDDNS #43768122886 PASS; AliyunDrive #43768122908 PASS. XY-C1 #43768122787 and NVRAM #43768122782 were in progress at last poll.
- This slice audits the separate shell `func_stop()`: the old quoted `kill -9 "$tr_pid"` treats a whitespace-separated PID list as one argument and logs KILLED without a final presence check. The updated path validates PID tokens, signals each PID individually, and requires a final absent snapshot. New host regression injects graceful exit, two-PID forced exit, signal failure and malformed PID output.
- Root dev_log.md holds 22 records; part-011 remains the indexed 100-record chronological archive. This commit adds one root entry; archive shards/index remain unchanged.
- XY-C1 #43768122787 validates the prior b516 source SHA, not this new stop change. Exact-head XY-C1 and storage checks for THIS COMMIT are required. No L3/L4 claim for this slice.

## Active task

### Verify Transmission stop postconditions and per-PID forced termination

The prior `func_stop()` ignored the return status of name-wide `killall`, waited up to 25 seconds, then passed the entire `pidof` output as one quoted argument to `kill -9`. With multiple daemon PIDs, this is not a valid single PID argument; even when signaling failed, the function could print `[KILLED]` and return the logger status without checking that the process was gone.

The new implementation treats unexpected `pidof` errors and malformed PID tokens as failure, retains graceful name-wide termination, signals each validated PID separately on timeout, and confirms a final empty PID snapshot before reporting success. Regressions inject graceful removal, a two-PID forced path, failed SIGKILL (must return failure with PIDs remaining), and malformed PID output (must signal none).

## Completed source slice — MSD Lite configuration publication

The APP_MSD_LITE branch in `start_udpxy()` previously truncated `/etc/storage/msd_lite.conf` before validating the template and ignored read/write/flush/close failures. It now uses checked same-directory staged publication and aborts before enabling or launching the daemon on failure.

Changed: `trunk/user/rc/src/net.c`; new `trunk/user/rc/src/msd_lite_config_file.h` and its host regression; Process lifecycle integration and XY-C1 workflow filters. GNU89 ASan/UBSan regression passed. The exact Process lifecycle host regression and XY-C1 Build/TRX/artifact workflow passed for source SHA `b488a561bb91df62aca810a6ff2c191c17f82b2b`; L3 applies only to that exact source revision. No L4.

## Verification boundary

- Parent main: ba51619ceea8efc084d28f6f38bb5a8bc084e8d7; source/test/handoff/roadmap/log changes form THIS atomic commit.
- Parent exact storage #43768122780 had passed the host launcher syntax and startup behavior step, then was still running the BusyBox 1.24.2 ash stage. Exact parent AI docs #43768122829 PASS. Parent process lifecycle #43768122785, inventory #43768122777, infosvr #43768122801, AdGuardHome #43768122808, AliDDNS #43768122886 and AliyunDrive #43768122908 PASS. XY-C1 #43768122787 and NVRAM #43768122782 were in progress.
- This commit changes Transmission shell stop logic and its test harness. Exact-head storage host + BusyBox ash, plus XY-C1 Build/TRX/artifact, are pending. Do not transfer parent results to this SHA.
- Frozen backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807; log archive/index unchanged. No L3/L4 claim.

## Exact first action for the next GPT

1. Inspect exact-head USB storage Actions for THIS COMMIT: host shell and BusyBox 1.24.2 ash must pass Transmission startup/collision tests, all four stop cases (graceful, multi-PID, failed SIGKILL, malformed PIDs), Aria concurrency and the remainder of storage checks. Fix the first causal failure.
2. Inspect exact-head XY-C1 Build, TRX and artifact upload. L3 only if all phases pass on this source SHA; no L4 without physical-router evidence.
3. If both gates are green, continue the broader user-space stability/security roadmap and do not re-audit completed Transmission startup config/directory/permission work.
4. Preserve backup and archive shards/index; maintain the root log 20–119 entry window.

