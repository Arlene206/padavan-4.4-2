# Cross-session execution mandate (read before resuming)

Recover live state once, then continue the exact P1 task. Do not repeat completed audits, stop at a plan, or ask the user to select work. Continue independent engineering while CI is pending. Before context exhaustion, refresh the exact first action and verification boundary so the next GPT can resume without repeating completed work.

**Mandatory handoff checkpoint:** Every session must verify before ending that this file accurately records the meaningful source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If next-session-critical facts have changed, update this handoff; if it is already accurate, do not create a no-op documentation commit.

For each multi-file logical slice, publish source, tests/contracts and necessary docs in **one atomic main commit**: create all blobs, one tree, one commit parented to freshly fetched main, then non-forced update of main using that exact expected SHA. No branches/PRs, no force-push, no archive modifications.

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; Dropbear wrapper status propagation and shell tests are committed at `a71d73c0ff7776ae36dce19c5287ca3c141553cd`. The process integration checker now preserves C diagnostic spacing, but the exact-head run found that return-order checks matched an earlier UNKNOWN-inventory return instead of the later launch-failure return; this follow-up scopes the search to the launcher branch.
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

- Exact a71 process inventory/lifecycle workflow run #43736811716 failed at the final source checker only after all C sanitizer/identity tests and optional-app shell tests passed. The failure was the checker using the first `return start_status;` in each function (UNKNOWN-inventory rejection) instead of the launch-failure return. This slice fixes the source contract only.
- Exact a71 AI docs #43736811598 PASS; Process lifecycle inventory #43736811577 PASS; infosvr #43736811544 PASS; AdGuardHome #43736811570 PASS; AliyunDrive #43736811565 PASS; AliDDNS #43736811539 PASS. XY-C1 #43736811566 and NVRAM #43736811552 were IN_PROGRESS at last poll.
- Dropbear source/wrapper changes are in a71; no L3 claim until the exact product-source SHA has successful Build, TRX verification and artifact upload. Physical-router L4 remains NOT VERIFIED.
## Exact first action for the next GPT

1. Verify exact-head Process lifecycle and AI docs after the corrected telnetd/ttyd return-order contract; check XY-C1 #43736811566 through Build, TRX verification and artifact upload, and NVRAM #43736811552 if still pending.
2. When CI is green, continue the tracked `start_services_once()`/service-script audit. The NFS wrapper exists at `trunk/user/nfs-utils/nfsd.sh`; review its stop failure and postcondition tests before claiming broader NFS compatibility.
3. Keep multi-file changes in one atomic main commit, using freshly fetched main and expected-SHA non-forced ref update. Preserve the frozen backup and archive shards.
## Invariants

- A PENDING run does not block independent work; retain exact run ID/SHA and continue safe tasks.
- DNSMasq/VPN lock-order and scoped RC process-signal audits are complete; do not repeat without new evidence.
- Linux 4.4 PID/start_time validation-to-signal TOCTOU remains unresolved. FTP failed-upload cleanup, NFS durability/AUTH_SYS and physical-router behavior remain separate unverified areas.
- Preserve docs/dev-log archive shards and docs/dev-log/INDEX.md byte-for-byte.
