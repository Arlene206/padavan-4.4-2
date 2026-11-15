# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create every source/test/workflow/doc blob, one complete Git tree and one commit parented to freshly fetched main; update main with that exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Keep the frozen backup and archived log shards/index unchanged. Before context exhaustion, finish a safe slice and leave this file accurate.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session must record exact main/backup SHAs, source changes, exact-head Action IDs/statuses, validation boundary and the first next action before ending. Do not create a no-op commit when the checkpoint is already accurate.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; main before this checker follow-up: 30e184833ffd8c43c1b22cdb544e0740545bdb43.
- Frozen backup: backup/pre-plugin-overhaul-2026-11-14 at 72f4f157a084f8184b9597ca5bf334b8080e5807; read-only.
- Exact 30e Process #43746805300 passed the IGMP proxy atomic config host test (step 19) and all earlier host steps, then failed because the MiniDLNA logger contract omitted the explicit enum cast used by production. The updated assertion verifies the typed process_state argument and call ordering.
- Exact 30e AI handoff docs #43746805306 PASS. Exact source-identical XY-C1 run #43746522702 on be7fd was IN_PROGRESS at last check; confirm Build, TRX and artifact upload before claiming L3. NVRAM #43746805297 was IN_PROGRESS.
- Other exact 30e Actions PASS: AdGuardHome #43746805244, AliDDNS #43746805299, AliyunDrive #43746805254, infosvr #43746805329, vsftpd #43746522533, NFS #43746522567, Process inventory #43746522504 and Samba #43746522547.
- The duplicate launch_status local and over-escaped storage helper extraction were fixed on be7fd. The FTP fstatat-to-unlinkat replacement window remains unresolved on Linux 4.4. No physical-router L4 evidence is claimed.

## Active task

### Complete Process verification for IGMP proxy atomic configuration publication

The IGMP config publisher and regression test are committed. Its GNU89 ASan/UBSan host test passed on exact 30e Process step 19. The remaining integration contract now checks the production MiniDLNA helper call including the explicit enum process_state cast and verifies it follows the post-launch process scan.

## Verification boundary

- Re-run Process on the checker-corrected commit; do not infer overall PASS from the host regression step alone.
- Check source-identical XY-C1 #43746522702 on be7fd through the Build, TRX verification and artifact upload steps. No newer production source has changed since be7fd.
- L3 requires successful exact-source Build, TRX and artifact upload. No physical-router/L4 result is claimed.
- Keep the frozen backup and all indexed dev-log, handoff and roadmap archives unchanged.

## Exact first action for the next GPT

1. Fetch live main and exact-head Actions for this checker follow-up; inspect Process logs for any remaining static-contract mismatch.
2. Confirm whether XY-C1 #43746522702 completed Build, TRX and artifact upload for the same production source tree; if it failed, diagnose before claiming L3.
3. If CI is pending, continue an independent source-level security/stability item from the live roadmap. Do not redo FTP cleanup, udpxy/xupnpd launch logging, IAPPD status propagation, storage daemon diagnostics or IGMP config publication.
4. Before ending, refresh this checkpoint only if the source SHA, relevant Action status, verification boundary or first next action materially changes.

