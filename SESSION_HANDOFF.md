# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create every source/test/workflow/doc blob, one complete Git tree and one commit parented to freshly fetched main; update `main` with that exact expected SHA and `force:false`. Use one atomic main commit, never partial multi-file commits or force-push. Keep the frozen backup and archived log shards/index unchanged. Before context exhaustion, finish a safe slice and leave this file accurate.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session must record exact main/backup SHAs, source changes, exact-head Action IDs/statuses, validation boundary and the first next action before ending. Do not create a no-op commit when the checkpoint is already accurate.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; main before the current source/test slice: `76e6e351e5bbbf17a6baf000bf76f56009f36ab3`.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only.
- Exact 51898 XY-C1 #43745391043 was IN_PROGRESS at Build; TRX verification and artifact upload were not confirmed.
- Exact 51898 Process #43745391109 failed only at the final integration checker; all preceding compile/host tests passed. The failure found the undeclared local `launch_status` in `run_dms()`; this batch adds the declaration.
- Exact 51898 AI docs #43745391151 failed because the handoff used lowercase "the checkpoint..." while the validator requires the exact capitalized sentence; this batch corrects it.
- Other 51898 exact-head Actions passed: NFS #43745390987, Samba #43745391093, AdGuardHome #43745391044, vsftpd #43745391060, AliDDNS #43745391035, HTTPD #43745391083, Process inventory #43745390995, AliyunDrive #43745391068, USB storage #43745391098 and NVRAM #43745391162.
- The FTP existing-STOR cleanup-eligibility defect is fixed and exact vsftpd host CI passed on #43744355353. Only successful exclusive creation grants rollback rights; the fstatat-to-unlinkat replacement race remains unresolved on Linux 4.4.
- No physical-router L4 evidence is claimed.

## Active task

### P2 — IGMP proxy configuration publication failure handling

`trunk/user/rc/src/net.c:start_igmpproxy()` used `fopen("/etc/igmpproxy.conf", "w")`, ignored `fprintf()` and `fclose()` errors, and could invoke `/bin/igmpproxy` after a failed open or partial config write.

This batch adds `trunk/user/rc/src/igmpproxy_config_file.h`: checked writes, flush, `fsync`, close and same-directory atomic rename; CR/LF values are rejected. Failure removes the staged file, preserves the last-good config, logs a nonzero status and prevents daemon launch. The production output retains quickleave, selected/default altnet, WAN upstream and bridge downstream directives.

The GNU89 ASan/UBSan test covers exact output, quickleave/default altnet, private 0600 mode, injection rejection, missing-parent and rename failures, buffered-write failure and stage cleanup. The Process workflow builds/runs it; Process/XY-C1 source filters and semantic contracts include the helper.

### Adjacent MiniDLNA correction

The exact 51898 Process checker also exposed that `run_dms()` assigned `launch_status` but had no local declaration. This batch adds `int launch_status;`; it does not change the recently added MiniDLNA status behavior.

## Verification boundary

- The new helper is host L2 only until the exact-head Process run passes. Do not infer host PASS from source creation.
- XY-C1 L3 is pending until Build XY-C1, TRX verification and artifact upload all pass on the exact current source SHA.
- No physical router/L4 check is claimed.
- Do not modify `backup/pre-plugin-overhaul-2026-11-14`, `docs/dev-log/` archive shards, `docs/dev-log/INDEX.md`, `docs/handoffs/` or `docs/roadmap-history/`.

## Exact first action for the next GPT

1. Fetch live main and exact-head Actions. Inspect the new Process regression, AI handoff-docs check and XY-C1 run; diagnose any real failure rather than claiming success.
2. Confirm XY-C1 Build, TRX verification and artifact upload for the exact source SHA.
3. If those checks are pending, continue the next independent source-level stability/security item from the live roadmap. Do not redo the completed FTP cleanup, udpxy/xupnpd launch logging, IAPPD caller status or MiniDLNA diagnostics.
4. Before ending, refresh this checkpoint only if a status, blocker, source baseline or first next action materially changed.

