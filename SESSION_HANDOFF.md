# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create all source/test/doc blobs, one complete Git tree and one commit parented to freshly fetched main; update main with exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Preserve frozen backup and archived log shards/index. Before context exhaustion, finish a safe bounded slice and leave the exact first action for the next session.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session records exact main/backup SHAs, source changes, exact-SHA Action IDs/statuses, validation boundary and first next action before ending. Do not create a no-op handoff commit when the checkpoint is accurate.

## Current repository state

- Repository: Arlene206/padavan-4.4-2.
- Parent main for the current Transmission staging-suffix repair: c0d287c2e1cacbf93dd20dccea9daf39e84d9324.
- Frozen backup branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; preserved and read-only.
- Exact parent c0 Actions: USB storage #43767296645 FAIL in the pre-existing stage-collision regression; AI handoff docs #43767296578 PASS; Process inventory #43767296590 PASS; Process lifecycle #43767296606 PASS; infosvr #43767296612 PASS; AliDDNS #43767296621 PASS; AliyunDrive #43767296670 PASS; AdGuardHome #43767296717 PASS. XY-C1 #43767296597 and NVRAM #43767296583 were still running at last inspection.
- Root cause of the c0 storage failure: the settings temp assignment had accidentally regressed from a PID-qualified `tmp.$ ` suffix to a single literal `tmp.# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create all source/test/doc blobs, one complete Git tree and one commit parented to freshly fetched main; update main with exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Preserve frozen backup and archived log shards/index. Before context exhaustion, finish a safe bounded slice and leave the exact first action for the next session.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session records exact main/backup SHAs, source changes, exact-SHA Action IDs/statuses, validation boundary and first next action before ending. Do not create a no-op handoff commit when the checkpoint is accurate.

. Therefore the collision fixture's exact `tmp.$ ` rewrite did not match, and the launcher created a different temp file and started. This also reintroduced cross-process staging-name collision risk.
- THIS COMMIT restores `CONFIG_TMP="${CONFIG_FILE}.tmp.$"`. The already-present collision regression now rewrites the actual source staging path to the pre-created fixture file and verifies its bytes survive and the daemon is not invoked.
- Transmission config generation checks expected umask 18, transformed umask 0, generator/sed/rename status, and required directory/permission/ownership setup. Cleanup only removes a stage owned by the current invocation.
- The exact XY-C1 Build/TRX/artifact PASS #43765742001 is for 4d2 only; do not transfer it to c0 or this repair. No L4 hardware evidence.

## Active task

### Verify the PID-unique Transmission settings stage and then audit stop postconditions

The c0 exact-head USB storage workflow #43767296645 failed at the pre-existing stage-collision test. Production source used `settings.json.tmp.# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create all source/test/doc blobs, one complete Git tree and one commit parented to freshly fetched main; update main with exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Preserve frozen backup and archived log shards/index. Before context exhaustion, finish a safe bounded slice and leave the exact first action for the next session.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session records exact main/backup SHAs, source changes, exact-SHA Action IDs/statuses, validation boundary and first next action before ending. Do not create a no-op handoff commit when the checkpoint is accurate.

## Current repository state

- Repository: Arlene206/padavan-4.4-2.
- Parent main for the current Transmission staging-suffix repair: c0d287c2e1cacbf93dd20dccea9daf39e84d9324.
- Frozen backup branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; preserved and read-only.
- Exact parent c0 Actions: USB storage #43767296645 FAIL in the pre-existing stage-collision regression; AI handoff docs #43767296578 PASS; Process inventory #43767296590 PASS; Process lifecycle #43767296606 PASS; infosvr #43767296612 PASS; AliDDNS #43767296621 PASS; AliyunDrive #43767296670 PASS; AdGuardHome #43767296717 PASS. XY-C1 #43767296597 and NVRAM #43767296583 were still running at last inspection.
- Root cause of the c0 storage failure: the settings temp assignment had accidentally regressed from a PID-qualified `tmp.$ ` suffix to a single literal `tmp.# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create all source/test/doc blobs, one complete Git tree and one commit parented to freshly fetched main; update main with exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Preserve frozen backup and archived log shards/index. Before context exhaustion, finish a safe bounded slice and leave the exact first action for the next session.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session records exact main/backup SHAs, source changes, exact-SHA Action IDs/statuses, validation boundary and first next action before ending. Do not create a no-op handoff commit when the checkpoint is accurate.

. Therefore the collision fixture's exact `tmp.$ ` rewrite did not match, and the launcher created a different temp file and started. This also reintroduced cross-process staging-name collision risk.
- THIS COMMIT restores `CONFIG_TMP="${CONFIG_FILE}.tmp.$"`. The already-present collision regression now rewrites the actual source staging path to the pre-created fixture file and verifies its bytes survive and the daemon is not invoked.
- Transmission config generation checks expected umask 18, transformed umask 0, generator/sed/rename status, and required directory/permission/ownership setup. Cleanup only removes a stage owned by the current invocation.
- The exact XY-C1 Build/TRX/artifact PASS #43765742001 is for 4d2 only; do not transfer it to c0 or this repair. No L4 hardware evidence.

 (one dollar) while the fixture replaces only the intended PID-qualified `settings.json.tmp.$` path. The test consequently did not create a collision and correctly detected that Transmission started. THIS COMMIT restores the PID-qualified suffix without changing the ownership-aware cleanup policy or weakening the regression.

After this exact-head storage regression passes, inspect `func_stop()` in `trunk/user/transmission/transmission.sh`. Its current forced-stop path sends `kill -9 "$tr_pid"` to a whitespace-separated `pidof` result and does not re-check the daemon afterward; confirm the behavior with injectable tests and fix any verified status/multi-PID defect.

## Completed source slice — MSD Lite configuration publication

The APP_MSD_LITE branch in `start_udpxy()` previously truncated `/etc/storage/msd_lite.conf` before validating the template and ignored read/write/flush/close failures. It now uses checked same-directory staged publication and aborts before enabling or launching the daemon on failure.

Changed: `trunk/user/rc/src/net.c`; new `trunk/user/rc/src/msd_lite_config_file.h` and its host regression; Process lifecycle integration and XY-C1 workflow filters. GNU89 ASan/UBSan regression passed. The exact Process lifecycle host regression and XY-C1 Build/TRX/artifact workflow passed for source SHA `b488a561bb91df62aca810a6ff2c191c17f82b2b`; L3 applies only to that exact source revision. No L4.

## Verification boundary

- Parent main: `c0d287c2e1cacbf93dd20dccea9daf39e84d9324`; this source/handoff/log repair is one atomic commit.
- Exact parent c0 storage run #43767296645 FAIL: the collision test proved the current production suffix did not match the PID-qualified pattern, allowing start. Repair is included in THIS COMMIT; exact-head storage host and BusyBox ash reruns are required.
- Exact parent c0 AI handoff run #43767296578 PASS, confirming the batch-100 archive rotation and root-log retention. Parent XY-C1 #43767296597 and NVRAM #43767296583 were pending at last inspection; parent XY result cannot validate this new SHA.
- Exact parent c0 Process inventory #43767296590, Process lifecycle #43767296606, infosvr #43767296612, AliDDNS #43767296621, AliyunDrive #43767296670 and AdGuardHome #43767296717 PASS.
- Previous XY-C1 Build/TRX/artifact run #43765742001 passed only for source SHA 4d2afeddba949b77dcbe1a8f6f950e9093438910. The current shell-source correction must receive its own exact-head XY-C1 run; no L3 claim until Build, TRX verification and artifact upload all pass on that SHA.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`. Existing archive shards and index are preserved. No L4 claim.

## Exact first action for the next GPT

1. Inspect the exact-head USB storage host + BusyBox ash regression and AI handoff workflow for THIS COMMIT. Confirm the collision fixture now reaches the PID-qualified path and preserves the pre-existing file; repair the first causal failure.
2. Inspect the exact-SHA XY-C1 workflow for Build XY-C1, TRX verification and artifact upload. L3 applies only if all phases pass for this exact source SHA.
3. Once validation is green, audit Transmission `func_stop()` failure handling and post-kill process verification; add multi-PID and failed-signal injection tests where they demonstrate the behavior.
4. Continue the highest-priority broader user-space stability/security item in the roadmap; preserve the frozen backup and existing log archives. No L4 without physical-router evidence.

