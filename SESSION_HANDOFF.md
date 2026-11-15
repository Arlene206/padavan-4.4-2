# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create every source/test/workflow/doc blob, one complete Git tree and one commit parented to freshly fetched main; update `main` with that exact expected SHA and `force:false`. Use one atomic main commit, never partial multi-file commits or force-push. Keep the frozen backup and archived log shards/index unchanged. Before context exhaustion, finish a safe slice and leave this file accurate.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session must record exact main/backup SHAs, source changes, exact-head Action IDs/statuses, validation boundary and the first next action before ending. Do not create a no-op commit when the checkpoint is already accurate.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; main before the current checker/RC correction: `e8b4c0cc9f41df3092a4529a2fc8816d1d7b7438`.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only.
- Exact e8b4 XY-C1 #43746069488 was IN_PROGRESS at Build; TRX verification and artifact upload were not confirmed.
- Exact e8b4 Process #43746069534 failed in the integration checker with IndexError because the new logger extractor used over-escaped newline literals. Host tests preceding the checker passed, including IGMP proxy atomic configuration publication.
- Exact e8b4 AI docs #43746069612 PASS. Other e8b4 exact-head Actions passed: WAN route #43746069491, AliDDNS #43746069482, infosvr #43746069600, Process inventory #43746069494, vsftpd #43746069587, NFS #43746069498, Samba #43746069667, AdGuardHome #43746069487, AliyunDrive #43746069460, HTTPD #43746069568, USB storage #43746069468 and NVRAM #43746069497.
- e8b4 also introduced a duplicate `int launch_status;` declaration in run_dms(); the follow-up removes the duplicate and scopes the helper/checker only when relevant storage daemons are compiled.
- The FTP existing-STOR cleanup-eligibility defect is fixed and exact vsftpd host CI passed on #43744355353. Only successful exclusive creation grants rollback rights; the fstatat-to-unlinkat replacement race remains unresolved on Linux 4.4.
- No physical-router L4 evidence is claimed.

## Active task

### P2 — IGMP proxy configuration publication failure handling

`trunk/user/rc/src/net.c:start_igmpproxy()` used `fopen("/etc/igmpproxy.conf", "w")`, ignored `fprintf()` and `fclose()` errors, and could invoke `/bin/igmpproxy` after a failed open or partial config write.

This batch adds `trunk/user/rc/src/igmpproxy_config_file.h`: checked writes, flush, `fsync`, close and same-directory atomic rename; CR/LF values are rejected. Failure removes the staged file, preserves the last-good config, logs a nonzero status and prevents daemon launch. The production output retains quickleave, selected/default altnet, WAN upstream and bridge downstream directives.

The GNU89 ASan/UBSan test covers exact output, quickleave/default altnet, private 0600 mode, injection rejection, missing-parent and rename failures, buffered-write failure and stage cleanup. The Process workflow builds/runs it; Process/XY-C1 source filters and semantic contracts include the helper.

### Adjacent MiniDLNA correction

The exact 51898 Process checker exposed a missing `launch_status` declaration; 76 added it. The next e8b4 commit accidentally added a second declaration, which this follow-up removes.

## Verification boundary

- The new helper is host L2 only until the exact-head Process run passes. Do not infer host PASS from source creation.
- XY-C1 L3 is pending until Build XY-C1, TRX verification and artifact upload all pass on the exact current source SHA.
- No physical router/L4 check is claimed.
- Do not modify `backup/pre-plugin-overhaul-2026-11-14`, `docs/dev-log/` archive shards, `docs/dev-log/INDEX.md`, `docs/handoffs/` or `docs/roadmap-history/`.

## Exact first action for the next GPT

1. Verify the checker/duplicate-declaration correction: exact Process #43746069534 failed at helper-extraction IndexError; this commit changes extraction to chr(10) boundaries, removes duplicate run_dms declaration, and guards the shared logger when all related APP_* features are disabled.
2. Poll exact e8b4 XY-C1 #43746069488 for Build, TRX verification and artifact upload; investigate a real cross-build failure before claiming L3.
3. IGMP proxy configuration helper/source and host regression are committed in e8b4; its host test passed before the Process checker failure. Re-run the exact Process workflow to obtain an overall PASS.
4. Continue the next independent source-level stability/security item from the live roadmap. Do not redo completed FTP cleanup, udpxy/xupnpd launch logging or IAPPD caller status.
4. Before ending, refresh this checkpoint only if a status, blocker, source baseline or first next action materially changed.

