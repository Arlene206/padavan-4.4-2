# Cross-session execution mandate (read before resuming)

Recover live state once, then continue the exact P1 task. Do not repeat completed audits, stop at a plan, or ask the user to select work. Continue independent engineering while CI is pending. Before context exhaustion, refresh the exact first action and verification boundary so the next GPT can resume without repeating completed work.

**Mandatory handoff checkpoint:** Every session must verify before ending that this file accurately records the meaningful source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If next-session-critical facts have changed, update this handoff; if it is already accurate, do not create a no-op documentation commit.

For each multi-file logical slice, publish source, tests/contracts and necessary docs in **one atomic main commit**: create all blobs, one tree, one commit parented to freshly fetched main, then non-forced update of main using that exact expected SHA. No branches/PRs, no force-push, no archive modifications.

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; exact parent for this IPTV proxy diagnostic slice is `a3aa716b5205b0b48f0aa656234fa125e5251542`. IAPPD wrapper/test at `37204498a3ce81d6dd077fd551a66e15a29be344` and fixture status-capture correction at `443c23f2f814e69549a49b42c0347f0fd42cfe9e` pass Process lifecycle #43738886473. 802.1X diagnostics at `ac28f793fe7cf897edfbc26ec6766d21d1d2c70d` pass Process #43739228138, inventory #43739227948 and wireless radio #43739228377. The iperf3 ROMFS fix at a3aa is under exact-head XY-C1 #43739572125.
- Exact a3aa Actions: process lifecycle #43739572150 PASS; inventory artifact #43739572120 PASS; AI docs #43739572235 PASS; infosvr #43739572142 PASS; AdGuardHome #43739572092 PASS; AliDDNS #43739572118 PASS; AliyunDrive #43739572281 PASS. NVRAM #43739572197 remained IN_PROGRESS at raw ioctl audit. XY-C1 #43739572125 remained IN_PROGRESS at Build; TRX verification and artifact upload were not yet observed. The previous #43737703922 ROMFS failure was caused by `$(THISDIR)` becoming empty; a3aa now uses `$(CURDIR)`.
- Optional RC boot wrappers for AdGuardHome, DDNS.to, Adbyby and AliDDNS retain/log launcher failures. NFS stop regression #43738132578 passed on source parent `479c05be3bef946f892f81a296d0ca3e709a698f`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only). No local checkout/MIPS toolchain or physical-router L4 result is claimed.

## Active task

### P2 — IPTV streaming proxy launch failure diagnostics

- `start_udpxy()` ignored the `eval()` result for both `/usr/sbin/udpxy` and the optional `/usr/bin/msd_lite` provider. `start_xupnpd()` ignored the foreground `eval()` and background-shell `system()` results. Enabled IPTV proxy launch command failures therefore had no launcher-specific diagnostic.
- This slice captures and logs nonzero results for each selected launch path, preserving NVRAM thresholds, optional-app compile gates, xupnpd support checks and existing foreground/background mode. The process integration checker covers all three launch branches; `net.c` is already filtered by process/XY-C1 workflows.
- This is command-result visibility only: `system("... &")` success does not prove child readiness, and configuration file creation/publish errors remain open.

### P1 — iperf3 ROMFS source fix (committed; firmware verification pending)

- The ROMFS source defect was fixed at `a3aa716b5205b0b48f0aa656234fa125e5251542`: `$(THISDIR)` was empty under `make -C iperf3 romfs`, causing `/iperf3.sh`; the install source now uses `$(CURDIR)/iperf3.sh`. `scripts/check_optional_app_start_status.py` asserts the package path.
- Exact a3aa Process #43739572150 PASS and inventory artifact #43739572120 PASS; XY-C1 #43739572125 remained IN_PROGRESS at Build at last poll. TRX and artifact upload are not yet verified; no firmware L3 claim.
- Continue remaining optional `start_services_once()` launch wrappers after confirming the current build; retain feature gates and disabled-service no-op behavior.

### P1 — 802.1X authenticator launch diagnostics (host contract PASS; XY-C1 pending)

- `start_8021x_wl()` and `start_8021x_rt()` now capture and log failures from `rt2860apd` / `rtinicapd` while preserving radio/auth-mode guards, ordering and the `void` ABI.
- Exact Process inventory/lifecycle #43739228138 and Process lifecycle inventory #43739227948 passed on source SHA `ac28f793fe7cf897edfbc26ec6766d21d1d2c70d`; XY-C1 #43739227968 was still running at the last poll.

### P2 — Ralink IAPP daemon lifecycle (host regression PASS; firmware build pending)

- The self-forking `ralinkiappd` parent now runs in foreground; startup and stop use authoritative process checks and bounded postconditions; launch/firewall/kill failures propagate, restart is gated on stop, and the existing RC `test` action is implemented.
- Exact Process lifecycle #43738886473 on SHA `443c23f2f814e69549a49b42c0347f0fd42cfe9e` passed the full IAPPD fixture plus scanner, lifecycle/ASan/UBSan and optional-app steps. XY-C1 #43738581861 on source SHA `37204498a3ce81d6dd077fd551a66e15a29be344` was still in progress at last poll; require TRX and artifact evidence before firmware L3.

- Boot diagnostics cover networkmap, watchdog, infosvr, GPIO, LLTD, rstats, telnetd and ttyd. Crond has bounded post-launch verification. DNS Forwarder and SCUT/vlmcsd/iperf3/MentoHUST wrappers preserve start/stop statuses. The NFS stop guard rejects missing `pidof` before state mutation.

### P1 common_ex PID-file SIGKILL audit — LEGACY PID-ONLY HELPER REMOVED

- All 36 top-level trunk/user/rc/src/*.c files were searched for kill_process_pidfile(). It had no in-tree RC C caller; its only implementation used PID-only kill_pidfile() and kill_pidfile_s(SIGKILL). The unused helper and its rc.h declaration are removed in this source slice.
- The ten live kill_services_with_pidfiles() callsites across VPN server/client, WAN/LAN DHCP, USB modem, detect_internet, detect_link and INICD use process_pidfile_read_identity() through service_lifecycle_policy.h. Existing tests cover UNKNOWN preflight, replacement identity blocking KILL escalation and force-only PPPD targets.
- This does not close Linux 4.4's final identity-check-to-kill(2) race; no atomic pidfd-like primitive is available in this target baseline.

## Verification boundary

- Exact parent `a3aa716b5205b0b48f0aa656234fa125e5251542`: Process inventory/lifecycle #43739572150 PASS; process inventory artifact #43739572120 PASS; AI docs #43739572235 PASS; infosvr #43739572142 PASS; AdGuardHome #43739572092 PASS; AliDDNS #43739572118 PASS; AliyunDrive #43739572281 PASS. NVRAM #43739572197 remained IN_PROGRESS at the raw userland ioctl/kernel API audit.
- Exact a3aa XY-C1 #43739572125 was IN_PROGRESS at Build XY-C1 at last poll; TRX verification and artifact upload were not yet observed. This build includes the iperf3 ROMFS fix, IAPPD wrapper and 802.1X source.
- Prior XY-C1 #43738581861 on SHA `37204498a3ce81d6dd077fd551a66e15a29be344` FAILED at ROMFS because the iperf3 Makefile then used `$(THISDIR)` and attempted `/iperf3.sh`; a3aa fixes this with `$(CURDIR)`. Require a fresh successful build, TRX and artifact before L3.
- IAPPD exact Process lifecycle #43738886473 PASS on SHA `443c23f2f814e69549a49b42c0347f0fd42cfe9e`; 802.1X exact Process #43739228138, inventory #43739227948 and wireless radio #43739228377 PASS on `ac28f793fe7cf897edfbc26ec6766d21d1d2c70d`.
- NFS start/reload #43737106642 and stop fixture #43738132578 PASS on their recorded exact source parents; do not repeat without new evidence.

## Exact first action for the next GPT

1. Verify exact-head XY-C1 #43739572125: inspect Build XY-C1, Verify TRX output and artifact upload. Diagnose the first causal compile/ROMFS failure if Build does not pass.
2. Recheck NVRAM #43739572197 and the current exact-a3aa workflow statuses; continue independent work instead of waiting for a PENDING job.
3. Verify this net.c proxy launch checker on the resulting exact-head Process lifecycle run; net.c already belongs to process and XY-C1 path filters.
4. Continue the remaining optional RC startup-wrapper audit only where a concrete swallowed status or unsafe postcondition remains.

## Invariants

- A PENDING run does not block independent work; record exact run ID/SHA and continue safe tasks.
- DNSMasq/VPN lock-order and scoped RC process-signal audits are complete; do not repeat without new evidence.
- Linux 4.4 PID/start_time validation-to-signal TOCTOU remains unresolved. FTP failed-upload cleanup, NFS durability/AUTH_SYS and physical-router behavior remain separate unverified areas.
- Preserve `backup/pre-plugin-overhaul-2026-11-14` and docs/dev-log archive shards / `docs/dev-log/INDEX.md` byte-for-byte.
