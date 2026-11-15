# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create every source/test/workflow/doc blob, one complete Git tree and one commit parented to freshly fetched main; update main with that exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Keep the frozen backup and archived log shards/index unchanged. Before context exhaustion, finish a safe slice and leave this file accurate.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session must record exact main/backup SHAs, source changes, exact-head Action IDs/statuses, validation boundary and the first next action before ending. Do not create a no-op commit when the checkpoint already accurate.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; main before this contract correction: be7fd92337812a28ddc5b275c7898915be114987.
- Frozen backup: backup/pre-plugin-overhaul-2026-11-14 at 72f4f157a084f8184b9597ca5bf334b8080e5807; read-only.
- Exact be7fd Process #43746522677 passed every host compile/regression, including IGMP proxy atomic config step 19, then failed at the final source checker because the MiniDLNA-specific contract still expected inline error logging after the storage code moved reporting into storage_log_daemon_start_result(). This update changes the contract to require the shared logger after the process scan.
- Exact be7fd AI handoff docs #43746522557 PASS. XY-C1 #43746522702 was IN_PROGRESS at Build; TRX verification and artifact upload were not confirmed. NVRAM #43746522646 and USB hotplug #43746522545 were IN_PROGRESS.
- Other be7fd Actions PASS: infosvr #43746522641, AdGuardHome #43746522610, AliyunDrive #43746522583, NFS #43746522567, vsftpd #43746522533, AliDDNS #43746522528, HTTPD #43746522505, Process inventory #43746522504 and Samba #43746522547.
- The previous duplicate MiniDLNA local declaration and over-escaped storage helper extraction were fixed in be7fd. FTP pre-existing STOR cleanup protection is complete; Linux 4.4's separate fstatat-to-unlinkat race remains open. No physical-router L4 evidence is claimed.

## Active task

### P2 — Complete exact-head Process verification for IGMP proxy config publication

The IGMP proxy config publisher and regression test are already committed. The dedicated host test passed on e8b4, but the Process job's final source checker was stale: its MiniDLNA-specific contract demanded inline logging even though the shared storage-result helper handles launch exit and process-presence states for all four optional storage daemons. The updated contract verifies the command, process scan and shared logger sequence.

## Verification boundary

- Re-run Process on this contract-corrected source SHA; do not infer PASS from the e8b4 test step alone.
- Exact be7fd XY-C1 #43746522702 was in Build. Recheck its terminal result, including TRX verification and artifact upload, then require a fresh exact-head XY-C1 run for this follow-up commit.
- L3 requires the exact current source SHA to complete Build, TRX and artifact upload successfully. No physical-router L4 check is claimed.
- Do not modify the frozen backup, docs/dev-log archives/index, docs/handoffs or docs/roadmap-history.

## Exact first action for the next GPT

1. Fetch live main and exact-head Actions for this checker correction. Verify Process and inspect any remaining failure logs.
2. Check the exact-source XY-C1 build through TRX validation and artifact upload; also check whether the prior be7fd build completed.
3. If CI is pending, continue an independent source-level stability/security item from the live roadmap. Do not repeat completed FTP existing-STOR cleanup, udpxy/xupnpd logging, IAPPD status propagation, storage service logger or IGMP configuration publication.
4. Before ending, refresh this checkpoint only when the source SHA, relevant Action status, validation boundary or next action materially changes.

