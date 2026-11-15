# Cross-session execution mandate (read before resuming)

Recover live state once, then continue the exact P1 task. Do not repeat completed audits, stop at a plan, or ask the user to select work. Continue independent engineering while CI is pending. Before context exhaustion, refresh the exact first action and verification boundary so the next GPT can resume without repeating completed work.

**Mandatory handoff checkpoint:** Every session must verify before ending that this file accurately records the meaningful source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If next-session-critical facts have changed, update this handoff; if it is already accurate, do not create a no-op documentation commit.

For each multi-file logical slice, publish source, tests/contracts and necessary docs in **one atomic main commit**: create all blobs, one tree, one commit parented to freshly fetched main, then non-forced update of main using that exact expected SHA. No branches/PRs, no force-push, no archive modifications.

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; live parent before this optional-launch diagnostic slice: `56369810646910ccaf0f342b414dd09f83570843`. Earlier commits fixed NFS pidof fail-closed handling, the telnetd/ttyd launch-return source contracts, and Dropbear/OpenSSH startup status propagation.
- Boot diagnostics cover networkmap, watchdog, infosvr, GPIO, LLTD, rstats, telnetd and ttyd. SCUT, vlmcsd, iperf3 and mentohust wrappers now preserve their daemon launch/stop statuses.
- LPD printer-removal PID identity hardening landed in `72ce1bb97323d4c417e8a5d891bbaf9c6ed3f2f3`; source-contract formatting correction is `022bc51507ca5f395c60a1f450cc461bf5403926`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only. No local checkout/MIPS toolchain or physical-router L4 result is claimed.

## Active task

### P1 — NFS stop postcondition regression coverage

- Networkmap `start_networkmap(1)`, watchdog `start_watchdog()`, infosvr `start_infosvr()`, and now `start_lltd()` / `start_rstats()` log enabled launcher failures at the call site and return the original status.
- `start_gpio_btn()` already logs UNKNOWN inventory, launcher failure, UNKNOWN postcondition and immediate exit. Crond has bounded post-launch verification and failure logs.
- LLTD and rstats still return 1 when their NVRAM enable setting is off; only enabled `eval()` failures are logged. The process-lifecycle checker asserts those sentinels, result capture, diagnostic and status propagation. Both the process-lifecycle and XY-C1 workflows filter `services.c`.
- DNS Forwarder already checks launcher status, logs failures and verifies process appearance. The iperf3 package includes bounded startup/stop checks; SCUT/vlmcsd/MentoHUST wrappers preserve daemon status. NFS `func_stop()` now rejects missing `pidof` before state mutation, but the shell regression still lacks direct cases for surviving daemons, failed export withdrawal, failed unmount, and complete successful teardown; this slice adds those four cases without changing production code.

### P1 common_ex PID-file SIGKILL audit — LEGACY PID-ONLY HELPER REMOVED

- All 36 top-level trunk/user/rc/src/*.c files were searched for kill_process_pidfile(). It had no in-tree RC C caller; its only implementation used PID-only kill_pidfile() and kill_pidfile_s(SIGKILL). The unused helper and its rc.h declaration are removed in this source slice.
- The ten live kill_services_with_pidfiles() callsites across VPN server/client, WAN/LAN DHCP, USB modem, detect_internet, detect_link and INICD use process_pidfile_read_identity() through service_lifecycle_policy.h. Existing tests cover UNKNOWN preflight, replacement identity blocking KILL escalation and force-only PPPD targets.
- This does not close Linux 4.4's final identity-check-to-kill(2) race; no atomic pidfd-like primitive is available in this target baseline.

## Verification boundary

- Exact parent main: `56369810646910ccaf0f342b414dd09f83570843`.
- NVRAM transaction boundary audit #43737626810: in_progress (parent 5636981).
- NFS daemon start and reload lifecycle regression #43737626899: completed/failure (parent 5636981).
- AI handoff documentation contract #43737626900: completed/success (parent 5636981).
- Process lifecycle inventory - 56369810646910ccaf0f342b414dd09f83570843 #43737626854: completed/success (parent 5636981).
- infosvr packet and signal lifecycle stability regression #43737626880: completed/success (parent 5636981).
- Process inventory and lifecycle fail-closed regression #43737627168: completed/success (parent 5636981).
- Product source changes in the next commit modify only launch diagnostics for enabled AdGuardHome, DDNS.to, Adbyby and AliDDNS wrappers. Disabled-service guards remain unchanged; exact-head process-lifecycle and XY-C1 runs are required.
- No current-source L3 or physical-router L4 result is claimed unless exact-head Build, TRX verification, artifact upload and hardware validation are independently recorded.
## Exact first action for the next GPT

1. Verify exact-head `nfsd-start-stability` after the stop regression additions: killall failure must retain live-daemon and mount state; exportfs/unmount failure must retain `etab`; full success must remove both daemon markers and both pseudo-mounts.
2. Confirm the workflow's inline RC/storage-lock/export ACL contracts still pass; keep the c736 exact-head process lifecycle #43737106692 and AI docs #43737106611 as green baseline unless a newer SHA supersedes them.
3. After NFS stop coverage is green, continue the actual `start_services_once()` and optional service-wrapper audit; preserve the frozen backup, archive shards and exact-SHA commit discipline.
4. Continue the tracked `start_services_once()` and optional service-wrapper audit. Keep atomic main commits and the backup/archive invariants.

## Invariants

- A PENDING run does not block independent work; retain exact run ID/SHA and continue safe tasks.
- DNSMasq/VPN lock-order and scoped RC process-signal audits are complete; do not repeat without new evidence.
- Linux 4.4 PID/start_time validation-to-signal TOCTOU remains unresolved. FTP failed-upload cleanup, NFS durability/AUTH_SYS and physical-router behavior remain separate unverified areas.
- Preserve docs/dev-log archive shards and docs/dev-log/INDEX.md byte-for-byte.
