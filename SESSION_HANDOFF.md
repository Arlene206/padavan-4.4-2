# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create all source/test/doc blobs, one complete Git tree and one commit parented to freshly fetched main; update main with exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Preserve frozen backup and archived log shards/index. Before context exhaustion, finish a safe bounded slice and leave the exact first action for the next session.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session records exact main/backup SHAs, source changes, exact-SHA Action IDs/statuses, validation boundary and first next action before ending. Do not create a no-op handoff commit when the checkpoint is accurate.

## Current repository state

- Repository: Arlene206/padavan-4.4-2.
- Parent main for the Aria stop lifecycle slice: 662c8d279a8bea874997e1d5fb74623d3a322640.
- Frozen backup branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; unchanged and read-only.
- Exact parent 662 Process lifecycle #43770376409 PASS, including IGMP token-validation and the corrected last-good fixture; Process inventory #43770376403 PASS; AI handoff #43770376449 PASS; NVRAM #43770376543 PASS; AdGuardHome #43770376442 PASS; infosvr #43770376527 PASS; AliDDNS #43770376582 PASS; AliyunDrive #43770376415 PASS.
- Exact parent 662 XY-C1 #43770376467 was still at Build XY-C1 (TRX and artifact upload pending) at last inspection. No L3 for this SHA.
- Exact prior abd40 USB storage #43769497238 PASS covers Transmission host and firmware BusyBox 1.24.2 ash. It predates Aria stop changes.
- THIS COMMIT fixes Aria stop/restart parity: pidof tri-state, PID 0/1 and malformed-output rejection, per-PID SIGKILL, verified post-kill absence and restart fail-closed. Transmission's matching stop validator now also rejects PID 0/1 and successful-empty pidof output.
- The common storage test exercises both launchers in host shell and BusyBox ash, including absent, graceful, multi-PID, signal failure, malformed/unsafe PID, pidof failure and restart gate scenarios.

## Active task

### Verify Aria2 and Transmission stop postconditions under host shell and BusyBox ash

Aria `func_stop()` passed a whitespace-separated `pidof aria2c` result as one quoted `kill -9` argument, printed success without checking post-SIGKILL daemon absence, and allowed `restart` to proceed regardless of stop result. It also treated pidof errors or successful-empty output as daemon absence.

THIS COMMIT validates pidof's status/output consistency, rejects malformed and unsafe PID 0/1 targets, signals every PID separately, verifies final process absence after SIGKILL and gates restart on successful stop. The shared storage regression now runs equivalent checks for both Aria and Transmission under host shell and firmware BusyBox 1.24.2 ash.

After exact-head storage validation, continue the next uncompleted broad P1 user-space service lifecycle/security finding from the roadmap. Avoid re-auditing these stop paths or the IGMP config publication.

## Completed source slice — IGMP proxy config-token validation

The IGMP proxy publisher already staged its configuration and checked writes/flush/fsync/close/rename. It now rejects whitespace/control/comment markers in interface tokens and restricts altnet values to digits, dots and slashes, preventing same-line token/comment injection from NVRAM values. Exact Process lifecycle #43770376409 PASS on parent SHA 662; exact 662 XY-C1 #43770376467 was still in progress at last poll.

## Completed source slice — Transmission launcher lifecycle

Transmission's shell stop now validates each PID, signals individual processes, confirms absence after forced termination and gates restart. Its PID-unique config stage and ownership-aware cleanup remain. Exact abd40 USB storage #43769497238 PASS across host and firmware BusyBox ash; no L3 claim for newer source SHA.

## Completed source slice — MSD Lite configuration publication

The APP_MSD_LITE branch in `start_udpxy()` previously truncated `/etc/storage/msd_lite.conf` before validating the template and ignored read/write/flush/close failures. It now uses checked same-directory staged publication and aborts before enabling or launching the daemon on failure.

Changed: `trunk/user/rc/src/net.c`; new `trunk/user/rc/src/msd_lite_config_file.h` and its host regression; Process lifecycle integration and XY-C1 workflow filters. GNU89 ASan/UBSan regression passed. The exact Process lifecycle host regression and XY-C1 Build/TRX/artifact workflow passed for source SHA `b488a561bb91df62aca810a6ff2c191c17f82b2b`; L3 applies only to that exact source revision. No L4.

## Verification boundary

- Parent main: `662c8d279a8bea874997e1d5fb74623d3a322640`; Aria/Transmission shell sources, shared regression and handoff/roadmap/log update are a single atomic commit.
- Exact parent 662 Process lifecycle #43770376409 PASS (all scanner, PID, Firefly/MiniDLNA, optional-app, IAPP and IGMP checks); Process inventory #43770376403 PASS; AI docs #43770376449 PASS; NVRAM #43770376543 PASS; AdGuardHome #43770376442 PASS; infosvr #43770376527 PASS; AliDDNS #43770376582 PASS; AliyunDrive #43770376415 PASS.
- Exact parent 662 XY-C1 #43770376467 was still building; TRX and artifact upload were pending. Prior abd40 storage #43769497238 PASS covers Transmission only.
- Exact-head USB storage host + firmware BusyBox 1.24.2 ash tests are required for both launchers. Exact-head XY-C1 Build/TRX/artifact is required for L3. No L3/L4 claim yet.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; archived log shards and index are untouched.

## Exact first action for the next GPT

1. Inspect exact-head USB storage Action for POSIX shell and firmware BusyBox 1.24.2 ash runs: both launchers' absent/graceful/multi-PID/failed-signal/malformed PID/pidof-unknown/restart tests, config-generation faults and the complete storage caller graph must pass.
2. Inspect exact-head XY-C1 Build, TRX verification and artifact upload; claim L3 only if all stages pass on this exact source SHA.
3. Continue the next uncompleted P1 broad user-space stability/security task in the roadmap; do not repeat the completed Aria/Transmission stop audit, IGMP input-token validation or prior MSD Lite slice.
4. Preserve the frozen backup and archive inventory. No L4 without physical-router evidence.

