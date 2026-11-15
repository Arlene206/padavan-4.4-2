# Cross-session execution mandate (read before resuming)

Recover live state once, then continue the exact P1 task. Do not repeat completed audits, stop at a plan, or ask the user to select work. Continue independent engineering while CI is pending. Before context exhaustion, refresh the exact first action and verification boundary so the next GPT can resume without repeating completed work.

**Mandatory handoff checkpoint:** Every session must verify before ending that this file accurately records the meaningful source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If next-session-critical facts have changed, update this handoff; if it is already accurate, do not create a no-op documentation commit.

For each multi-file logical slice, publish source, tests/contracts and necessary docs in **one atomic main commit**: create all blobs, one tree, one commit parented to freshly fetched main, then non-forced update of main using that exact expected SHA. No branches/PRs, no force-push, no archive modifications.

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; IAPPD lifecycle source/test are committed at `37204498a3ce81d6dd077fd551a66e15a29be344`, with its status-capture fixture correction at `443c23f2f814e69549a49b42c0347f0fd42cfe9e`; exact Process lifecycle #43738886473 and AI docs #43738886520 PASS. XY-C1 #43738581861 for the IAPPD product-script source remained IN_PROGRESS at Build XY-C1 at last poll. Latest parent before this 802.1X fix is `c0999a97f07435c11903dac04524a07844cc60a8`. NFS stop fixtures pass at 479c (#43738132578).
- Boot diagnostics cover networkmap, watchdog, infosvr, GPIO, LLTD, rstats, telnetd and ttyd. SCUT, vlmcsd, iperf3 and mentohust wrappers now preserve their daemon launch/stop statuses.
- LPD printer-removal PID identity hardening landed in `72ce1bb97323d4c417e8a5d891bbaf9c6ed3f2f3`; source-contract formatting correction is `022bc51507ca5f395c60a1f450cc461bf5403926`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only. No local checkout/MIPS toolchain or physical-router L4 result is claimed.

## Active task

### P1 — 802.1X authenticator launch failure visibility

- `start_8021x_wl()` and `start_8021x_rt()` invoked `rt2860apd` / `rtinicapd` with `eval()` but discarded the returned status. A configured WPA/WPA2/RADIUS authenticator launch failure therefore produced no radio-specific boot diagnostic.
- Both functions now retain and log nonzero launcher statuses while preserving radio-enabled/auth-mode guards, call ordering and the existing `void` interface. The process-lifecycle source contract checks both launch result assignments and error-log ordering; `net_wifi.c` is already filtered by the process and XY-C1 workflows.

### P2 — Ralink IAPP daemon lifecycle postconditions (host regression PASS; firmware build pending)

- The C `ralinkiappd` binary forks its own long-running child, but `iappd.sh` also backgrounded its launcher, hiding fork/exec failures and allowing later `sysctl`/`iptables` status to mask them. Stop ignored both `killall` statuses and lacked a process-absence postcondition; the RC boot path's `iappd.sh test` action was unsupported.
- The wrapper runs the self-forking parent in the foreground, requires authoritative `pidof`, verifies bounded child/stop postconditions, skips duplicate starts, propagates launcher/firewall/kill failures, gates restart on confirmed stop, and implements the RC `test` action. The corrected mocked host regression passes exact Process lifecycle #43738886473; AI docs #43738886520 PASS. XY-C1 #43738581861 for this product source remains pending at Build XY-C1.

- Networkmap `start_networkmap(1)`, watchdog `start_watchdog()`, infosvr `start_infosvr()`, and now `start_lltd()` / `start_rstats()` log enabled launcher failures at the call site and return the original status.
- `start_gpio_btn()` already logs UNKNOWN inventory, launcher failure, UNKNOWN postcondition and immediate exit. Crond has bounded post-launch verification and failure logs.
- LLTD and rstats still return 1 when their NVRAM enable setting is off; only enabled `eval()` failures are logged. The process-lifecycle checker asserts those sentinels, result capture, diagnostic and status propagation. Both the process-lifecycle and XY-C1 workflows filter `services.c`.
- DNS Forwarder already checks launcher status, logs failures and verifies process appearance. The iperf3 package includes bounded startup/stop checks; SCUT/vlmcsd/MentoHUST wrappers preserve daemon status. NFS `func_stop()` rejects missing `pidof` before state mutation. The NFS stop fixture newline escaping was corrected at `479c05be3bef946f892f81a296d0ca3e709a698f`; exact NFS #43738132578 PASS.

### P1 common_ex PID-file SIGKILL audit — LEGACY PID-ONLY HELPER REMOVED

- All 36 top-level trunk/user/rc/src/*.c files were searched for kill_process_pidfile(). It had no in-tree RC C caller; its only implementation used PID-only kill_pidfile() and kill_pidfile_s(SIGKILL). The unused helper and its rc.h declaration are removed in this source slice.
- The ten live kill_services_with_pidfiles() callsites across VPN server/client, WAN/LAN DHCP, USB modem, detect_internet, detect_link and INICD use process_pidfile_read_identity() through service_lifecycle_policy.h. Existing tests cover UNKNOWN preflight, replacement identity blocking KILL escalation and force-only PPPD targets.
- This does not close Linux 4.4's final identity-check-to-kill(2) race; no atomic pidfd-like primitive is available in this target baseline.

## Verification boundary

- IAPPD test run #43738581837 initially failed because the fixture captured stdout instead of command exit status; test-only correction `443c23f2f814e69549a49b42c0347f0fd42cfe9e` then passed Process lifecycle #43738886473 and AI docs #43738886520. XY-C1 #43738581861 for IAPPD source SHA `37204498a3ce81d6dd077fd551a66e15a29be344` remained IN_PROGRESS at Build; TRX and artifact were not yet observed.
- Exact parent SHA: `479c05be3bef946f892f81a296d0ca3e709a698f`; NFS #43738132578 PASS; Process lifecycle #43738132620 PASS; Process artifact #43738132593 PASS; AI docs #43738132573 PASS; infosvr #43738132617 PASS; AdGuardHome #43738132606 PASS; AliDDNS #43738132690 PASS. AliyunDrive #43738132576 and NVRAM #43738132600 completed/pending respectively at last poll. No XY-C1 run was triggered on that parent.
- infosvr packet and signal lifecycle stability regression #43738132617: completed/success (exact parent).
- NFS daemon start and reload lifecycle regression #43738132578: completed/success (exact parent).
- AI handoff documentation contract #43738132573: completed/success (exact parent).
- Process inventory and lifecycle fail-closed regression #43738132620: completed/success (exact parent).
- Process lifecycle inventory - 479c05be3bef946f892f81a296d0ca3e709a698f #43738132593: completed/success (exact parent).
- NVRAM transaction boundary audit #43738132600: completed/success (exact parent).
- The four RC wrapper changes retain their NVRAM feature gates and only add enabled-launch failure observability. Fresh exact-head process-lifecycle and XY-C1 Actions are required; no current-source L3 or physical-router L4 claim is made without exact-SHA build, TRX/artifact and hardware evidence.
## Exact first action for the next GPT
1. Verify exact-head process-lifecycle and XY-C1 for the 802.1X status-logging change; confirm status capture and error-log ordering for both `rt2860apd` and `rtinicapd`.
2. Recheck XY-C1 #43738581861 for IAPPD source SHA 37204498; require Build, TRX verification and artifact upload before L3.
3. Continue independent `start_services_once()` wrapper audits only once these checks are recorded; candidates include udpxy/xupnpd and watchdog module-load status. Do not repeat NFS/IAPPD checks already green.
4. Preserve the frozen backup and dev-log archive; use complete atomic trees and expected-SHA non-forced main updates.


## Invariants

- A PENDING run does not block independent work; retain exact run ID/SHA and continue safe tasks.
- DNSMasq/VPN lock-order and scoped RC process-signal audits are complete; do not repeat without new evidence.
- Linux 4.4 PID/start_time validation-to-signal TOCTOU remains unresolved. FTP failed-upload cleanup, NFS durability/AUTH_SYS and physical-router behavior remain separate unverified areas.
- Preserve docs/dev-log archive shards and docs/dev-log/INDEX.md byte-for-byte.
