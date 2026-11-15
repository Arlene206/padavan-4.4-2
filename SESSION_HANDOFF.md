# Cross-session execution mandate (read before resuming)

Recover live state once, then continue the exact P1 task. Do not repeat completed audits, stop at a plan, or ask the user to select work. Continue independent engineering while CI is pending. Before context exhaustion, refresh the exact first action and verification boundary so the next GPT can resume without repeating completed work.

**Mandatory handoff checkpoint:** Every session must verify before ending that this file accurately records the meaningful source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If next-session-critical facts have changed, update this handoff; if it is already accurate, do not create a no-op documentation commit.

For each multi-file logical slice, publish source, tests/contracts and necessary docs in **one atomic main commit**: create all blobs, one tree, one commit parented to freshly fetched main, then non-forced update of main using that exact expected SHA. No branches/PRs, no force-push, no archive modifications.

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; Dropbear wrapper status propagation and mocked shell tests are present at `a71d73c0ff7776ae36dce19c5287ca3c141553cd`. The exact-head process-lifecycle follow-up `7e21f495aeaa60a85bdf839e9ee1bb514105c49a` corrected the telnetd/ttyd return-order contracts; process lifecycle #43736960883 and AI docs #43736960862 now PASS. Independent NFS review found missing `pidof` was interpreted as empty process inventory, allowing NFS stop to progress toward export withdrawal/unmount without confirmed daemon absence. This slice adds a fail-closed `pidof` preflight to NFS start/reload/stop.
- Boot diagnostics cover networkmap, watchdog, infosvr, GPIO, LLTD, rstats, telnetd and ttyd. SCUT, vlmcsd, iperf3 and mentohust wrappers now preserve their daemon launch/stop statuses.
- LPD printer-removal PID identity hardening landed in `72ce1bb97323d4c417e8a5d891bbaf9c6ed3f2f3`; source-contract formatting correction is `022bc51507ca5f395c60a1f450cc461bf5403926`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only. No local checkout/MIPS toolchain or physical-router L4 result is claimed.

## Active task

### P1 — Boot daemon startup error reporting

- Networkmap `start_networkmap(1)`, watchdog `start_watchdog()`, infosvr `start_infosvr()`, and now `start_lltd()` / `start_rstats()` log enabled launcher failures at the call site and return the original status.
- `start_gpio_btn()` already logs UNKNOWN inventory, launcher failure, UNKNOWN postcondition and immediate exit. Crond has bounded post-launch verification and failure logs.
- LLTD and rstats still return 1 when their NVRAM enable setting is off; only enabled `eval()` failures are logged. The process-lifecycle checker asserts those sentinels, result capture, diagnostic and status propagation. Both the process-lifecycle and XY-C1 workflows filter `services.c`.
- DNS Forwarder already checks launcher status, logs failures and verifies process appearance. The iperf3 package now includes its missing service script, with bounded startup/stop checks and fail-closed tool preflight. APP_MENTOHUST's RC wrapper already preflights process presence, but `mentohust.sh` previously printed failures while unconditionally exiting 0; this slice preserves launcher and stop statuses through the shell dispatcher.

### P1 common_ex PID-file SIGKILL audit — LEGACY PID-ONLY HELPER REMOVED

- All 36 top-level trunk/user/rc/src/*.c files were searched for kill_process_pidfile(). It had no in-tree RC C caller; its only implementation used PID-only kill_pidfile() and kill_pidfile_s(SIGKILL). The unused helper and its rc.h declaration are removed in this source slice.
- The ten live kill_services_with_pidfiles() callsites across VPN server/client, WAN/LAN DHCP, USB modem, detect_internet, detect_link and INICD use process_pidfile_read_identity() through service_lifecycle_policy.h. Existing tests cover UNKNOWN preflight, replacement identity blocking KILL escalation and force-only PPPD targets.
- This does not close Linux 4.4's final identity-check-to-kill(2) race; no atomic pidfd-like primitive is available in this target baseline.

## Verification boundary

- On exact a71 SHA, process lifecycle #43736811716 failed at the telnetd order contract after all C and optional-app tests passed; AI docs #43736811598, process inventory artifact #43736811577, infosvr #43736811544, AdGuardHome #43736811570, AliyunDrive #43736811565 and AliDDNS #43736811539 passed. Follow-up `7e21f495aeaa60a85bdf839e9ee1bb514105c49a` corrected the checker; exact process lifecycle #43736960883 and AI docs #43736960862 PASS. AdGuardHome #43736960909, infosvr #43736961052, AliDDNS #43736960859 and AliyunDrive #43736961032 PASS; NVRAM #43736960951 was IN_PROGRESS.
- This slice adds an explicit `pidof` availability gate before NFS start/reload/stop, plus host regressions proving each returns 127 when the process inventory tool is missing and stop leaves daemon/mount state untouched. Exact-head NFS workflow must pass; no NFS target build or physical-router storage test is claimed.
- Dropbear source/wrapper changes are in a71; no L3 claim until the exact product-source SHA has successful Build, TRX verification and artifact upload. Physical-router L4 remains NOT VERIFIED.
## Exact first action for the next GPT

1. Verify exact-head NFS start/reload workflow for the missing-`pidof` fail-closed behavior; confirm start/reload/stop return 127 and stop makes no killall/exportfs/umount call and leaves daemons/mounts intact.
2. Confirm NFS workflow's inline RC/storage-lock/export ACL contracts still pass. Check exact-head process lifecycle #43736960883 and AI docs #43736960862 (both PASS) remain the latest green baseline.
3. Recheck NVRAM #43736960951 for the exact source SHA; inspect XY-C1 status for the product SHA that includes Dropbear. No L3 claim until Build, TRX verification and artifact upload all pass; physical L4 remains NOT VERIFIED.
4. Continue the tracked `start_services_once()` and optional service-wrapper audit. Keep atomic main commits and the backup/archive invariants.

## Invariants

- A PENDING run does not block independent work; retain exact run ID/SHA and continue safe tasks.
- DNSMasq/VPN lock-order and scoped RC process-signal audits are complete; do not repeat without new evidence.
- Linux 4.4 PID/start_time validation-to-signal TOCTOU remains unresolved. FTP failed-upload cleanup, NFS durability/AUTH_SYS and physical-router behavior remain separate unverified areas.
- Preserve docs/dev-log archive shards and docs/dev-log/INDEX.md byte-for-byte.
