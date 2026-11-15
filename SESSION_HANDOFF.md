# Cross-session execution mandate (read before resuming)

Recover live state once, then continue the exact P1 task. Do not repeat completed audits, stop at a plan, or ask the user to select work. Continue independent engineering while CI is pending. Before context exhaustion, refresh the exact first action and verification boundary so the next GPT can resume without repeating completed work.

**Mandatory handoff checkpoint:** Every session must verify before ending that this file accurately records the meaningful source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If next-session-critical facts have changed, update this handoff; if it is already accurate, do not create a no-op documentation commit.

For each multi-file logical slice, publish source, tests/contracts and necessary docs in **one atomic main commit**: create all blobs, one tree, one commit parented to freshly fetched main, then non-forced update of main using that exact expected SHA. No branches/PRs, no force-push, no archive modifications.

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; IAPPD wrapper and host regression are committed at `37204498a3ce81d6dd077fd551a66e15a29be344`. The exact-head process-lifecycle run #43738581837 passed all earlier C/ASan/UBSan and optional-app steps, then failed only because the new shell fixture captured command stdout instead of its exit status; the correction below adds an explicit status-capture helper. XY-C1 #43738581861 remained in Build XY-C1 at last poll. NFS stop fixtures are green at 479c (#43738132578).
- Boot diagnostics cover networkmap, watchdog, infosvr, GPIO, LLTD, rstats, telnetd and ttyd. SCUT, vlmcsd, iperf3 and mentohust wrappers now preserve their daemon launch/stop statuses.
- LPD printer-removal PID identity hardening landed in `72ce1bb97323d4c417e8a5d891bbaf9c6ed3f2f3`; source-contract formatting correction is `022bc51507ca5f395c60a1f450cc461bf5403926`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only. No local checkout/MIPS toolchain or physical-router L4 result is claimed.

## Active task

### P1 — Ralink IAPP daemon lifecycle postconditions

- The C `ralinkiappd` binary forks its own long-running child, but `iappd.sh` also backgrounded its launcher, hiding fork/exec failures and allowing later `sysctl`/`iptables` status to mask them. Stop ignored both `killall` statuses and lacked a process-absence postcondition; the RC boot path's `iappd.sh test` action was unsupported.
- The wrapper now runs the self-forking parent in the foreground, requires an authoritative `pidof` result, waits for bounded child/stop postconditions, skips duplicate starts, propagates launcher/firewall/kill failures, gates restart on a confirmed stop, and implements the existing RC `test` action. Added mocked host coverage and process/XY-C1 workflow triggers. Exact SHA #43738581837 exposed only a status-capture bug in the test harness; source launch/stop behavior has not yet passed its full fixture.

- Networkmap `start_networkmap(1)`, watchdog `start_watchdog()`, infosvr `start_infosvr()`, and now `start_lltd()` / `start_rstats()` log enabled launcher failures at the call site and return the original status.
- `start_gpio_btn()` already logs UNKNOWN inventory, launcher failure, UNKNOWN postcondition and immediate exit. Crond has bounded post-launch verification and failure logs.
- LLTD and rstats still return 1 when their NVRAM enable setting is off; only enabled `eval()` failures are logged. The process-lifecycle checker asserts those sentinels, result capture, diagnostic and status propagation. Both the process-lifecycle and XY-C1 workflows filter `services.c`.
- DNS Forwarder already checks launcher status, logs failures and verifies process appearance. The iperf3 package includes bounded startup/stop checks; SCUT/vlmcsd/MentoHUST wrappers preserve daemon status. NFS `func_stop()` rejects missing `pidof` before state mutation. The NFS stop fixture newline escaping was corrected at `479c05be3bef946f892f81a296d0ca3e709a698f`; exact NFS #43738132578 PASS.

### P1 common_ex PID-file SIGKILL audit — LEGACY PID-ONLY HELPER REMOVED

- All 36 top-level trunk/user/rc/src/*.c files were searched for kill_process_pidfile(). It had no in-tree RC C caller; its only implementation used PID-only kill_pidfile() and kill_pidfile_s(SIGKILL). The unused helper and its rc.h declaration are removed in this source slice.
- The ten live kill_services_with_pidfiles() callsites across VPN server/client, WAN/LAN DHCP, USB modem, detect_internet, detect_link and INICD use process_pidfile_read_identity() through service_lifecycle_policy.h. Existing tests cover UNKNOWN preflight, replacement identity blocking KILL escalation and force-only PPPD targets.
- This does not close Linux 4.4's final identity-check-to-kill(2) race; no atomic pidfd-like primitive is available in this target baseline.

## Verification boundary

- Exact IAPPD source/test SHA `37204498a3ce81d6dd077fd551a66e15a29be344`: process lifecycle #43738581837 FAIL at the first IAPPD missing-pidof case because command substitution captured empty stdout, not the command's exit code; all preceding scanner, lifecycle C/ASan/UBSan and optional-app checks passed. This test-only follow-up adds an explicit `capture_status` helper. AI docs #43738581717 PASS; XY-C1 #43738581861 IN_PROGRESS at Build XY-C1; NVRAM #43738581771 IN_PROGRESS.
- Exact parent SHA: `479c05be3bef946f892f81a296d0ca3e709a698f`; NFS #43738132578 PASS; Process lifecycle #43738132620 PASS; Process artifact #43738132593 PASS; AI docs #43738132573 PASS; infosvr #43738132617 PASS; AdGuardHome #43738132606 PASS; AliDDNS #43738132690 PASS. AliyunDrive #43738132576 and NVRAM #43738132600 completed/pending respectively at last poll. No XY-C1 run was triggered on that parent.
- infosvr packet and signal lifecycle stability regression #43738132617: completed/success (exact parent).
- NFS daemon start and reload lifecycle regression #43738132578: completed/success (exact parent).
- AI handoff documentation contract #43738132573: completed/success (exact parent).
- Process inventory and lifecycle fail-closed regression #43738132620: completed/success (exact parent).
- Process lifecycle inventory - 479c05be3bef946f892f81a296d0ca3e709a698f #43738132593: completed/success (exact parent).
- NVRAM transaction boundary audit #43738132600: completed/success (exact parent).
- The four RC wrapper changes retain their NVRAM feature gates and only add enabled-launch failure observability. Fresh exact-head process-lifecycle and XY-C1 Actions are required; no current-source L3 or physical-router L4 claim is made without exact-SHA build, TRX/artifact and hardware evidence.
## Exact first action for the next GPT
1. Verify exact-head process-lifecycle after the test status-capture correction; check whether every IAPPD fixture case now passes and inspect any next failure in the wrapper.
2. Verify XY-C1 #43738581861 for Build XY-C1, TRX output and artifact upload; do not declare L3 while any stage is pending/failed.
3. Recheck NVRAM #43738581771 and the exact-head AI/docs/other Action statuses. NFS fixture correction is already green at #43738132578; do not repeat it.
4. Continue the remaining `start_services_once()` audit only after IAPPD and XY-C1 are verified; preserve backup/archive and expected-SHA commit invariants.


## Invariants

- A PENDING run does not block independent work; retain exact run ID/SHA and continue safe tasks.
- DNSMasq/VPN lock-order and scoped RC process-signal audits are complete; do not repeat without new evidence.
- Linux 4.4 PID/start_time validation-to-signal TOCTOU remains unresolved. FTP failed-upload cleanup, NFS durability/AUTH_SYS and physical-router behavior remain separate unverified areas.
- Preserve docs/dev-log archive shards and docs/dev-log/INDEX.md byte-for-byte.
