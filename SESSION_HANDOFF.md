# Cross-session execution mandate (read before resuming)

Recover live state once, then continue the exact P1 task. Do not repeat completed audits, stop at a plan, or ask the user to select work. Continue independent engineering while CI is pending. Before context exhaustion, refresh the exact first action and verification boundary so the next GPT can resume without repeating completed work.

**Mandatory handoff checkpoint:** Every session must verify before ending that this file accurately records the meaningful source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If next-session-critical facts have changed, update this handoff; if it is already accurate, do not create a no-op documentation commit.

For each multi-file logical slice, publish source, tests/contracts and necessary docs in **one atomic main commit**: create all blobs, one tree, one commit parented to freshly fetched main, then non-forced update of main using that exact expected SHA. No branches/PRs, no force-push, no archive modifications.

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; exact parent for this IAPPD RC-caller status slice is `db6615af1928b3c7ea8cd564830d6ccc05d431c7`. The preceding IPTV proxy launch-status change is also in db661; IAPPD wrapper/test at `37204498a3ce81d6dd077fd551a66e15a29be344` and status-capture correction at `443c23f2f814e69549a49b42c0347f0fd42cfe9e` pass Process #43738886473. 802.1X diagnostics at `ac28f793fe7cf897edfbc26ec6766d21d1d2c70d` pass Process #43739228138, inventory #43739227948 and wireless radio #43739228377. Exact db661 Process #43740074825, AI docs #43740074768 and NVRAM #43740074736 PASS; XY-C1 #43740074742 was still in Build at last poll.
- Exact a3aa Actions: process lifecycle #43739572150 PASS; inventory artifact #43739572120 PASS; AI docs #43739572235 PASS; infosvr #43739572142 PASS; AdGuardHome #43739572092 PASS; AliDDNS #43739572118 PASS; AliyunDrive #43739572281 PASS. NVRAM #43739572197 remained IN_PROGRESS at raw ioctl audit. XY-C1 #43739572125 remained IN_PROGRESS at Build; TRX verification and artifact upload were not yet observed. The previous #43737703922 ROMFS failure was caused by `$(THISDIR)` becoming empty; a3aa now uses `$(CURDIR)`.
- Optional RC boot wrappers for AdGuardHome, DDNS.to, Adbyby and AliDDNS retain/log launcher failures. NFS stop regression #43738132578 passed on source parent `479c05be3bef946f892f81a296d0ca3e709a698f`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only). No local checkout/MIPS toolchain or physical-router L4 result is claimed.

## Active task

### P2 — IAPPD boot caller status visibility

- RC `start_services_once()` called `iappd.sh restart`, `modprobe xt_TPROXY` and `iappd.sh test` using `system()` without observing any result. The IAPPD wrapper already propagates launch/stop failures and implements `test`, but RC hid all three statuses. This slice logs nonzero wait statuses in the original call order and retains the best-effort startup policy; the integration checker asserts each assignment, diagnostic and ordering.

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

- Exact parent `db6615af1928b3c7ea8cd564830d6ccc05d431c7`: Process inventory/lifecycle #43740074825 PASS (including source integration and IAPPD host tests); AI docs #43740074768 PASS; NVRAM #43740074736 PASS; AdGuardHome #43740074816 PASS; WAN route #43740074841 PASS; AliyunDrive #43740074721 PASS; AliDDNS #43740074783 PASS; infosvr #43740074867 PASS.
- Exact db661 XY-C1 #43740074742 remained IN_PROGRESS at Build XY-C1 at last poll; TRX and artifact upload were pending. This build covers the iperf3 ROMFS fix, IAPPD wrapper, 802.1X diagnostics and IPTV proxy status changes.
- IAPPD exact Process #43738886473 PASS on test-fix SHA `443c23f2f814e69549a49b42c0347f0fd42cfe9e`; 802.1X exact Process #43739228138, inventory #43739227948 and wireless radio #43739228377 PASS on `ac28f793fe7cf897edfbc26ec6766d21d1d2c70d`. NFS stop fixture #43738132578 PASS on its recorded source parent.

## Exact first action for the next GPT

1. Verify exact-head XY-C1 #43740074742 through Build, TRX output verification and artifact upload; investigate the first causal error if the build fails.
2. Verify this IAPPD RC-caller status checker on exact-head Process lifecycle; require all three results to be captured, logged and kept in call order.
3. Continue independent service/startup audits if XY-C1 or other workflows are PENDING; do not repeat green IAPPD, 802.1X or NFS regressions without source changes.
4. Preserve frozen backup/archive invariants and create each logical task as one full tree/commit with an expected-SHA non-forced main update.

## Invariants

- A PENDING run does not block independent work; record exact run ID/SHA and continue safe tasks.
- DNSMasq/VPN lock-order and scoped RC process-signal audits are complete; do not repeat without new evidence.
- Linux 4.4 PID/start_time validation-to-signal TOCTOU remains unresolved. FTP failed-upload cleanup, NFS durability/AUTH_SYS and physical-router behavior remain separate unverified areas.
- Preserve `backup/pre-plugin-overhaul-2026-11-14` and docs/dev-log archive shards / `docs/dev-log/INDEX.md` byte-for-byte.
