# Cross-session execution mandate (read before resuming)

Recover live state once, then continue the exact P1 task. Do not repeat completed audits, stop at a plan, or ask the user to select work. Continue independent engineering while CI is pending. Before context exhaustion, refresh the exact first action and verification boundary so the next GPT can resume without repeating completed work.

**Mandatory handoff checkpoint:** Every session must verify before ending that this file accurately records the meaningful source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If next-session-critical facts have changed, update this handoff; if it is already accurate, do not create a no-op documentation commit.

For each multi-file logical slice, publish source, tests/contracts and necessary docs in **one atomic main commit**: create all blobs, one tree, one commit parented to freshly fetched main, then non-forced update of main using that exact expected SHA. No branches/PRs, no force-push, no archive modifications.

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; current main parent for the checker correction is `c1987b2520611dd1adb1a54e655832c4c61df8b0`. RC now captures/logs restart, xt_TPROXY module-load and IAPPD post-start test wait statuses; exact c1987 Process #43740697227 failed only because the negative source contract matched assigned `system()` calls by substring. All earlier source, C/ASan/UBSan and IAPPD host steps passed. XY-C1 #43740697118 was at pre-build setup; AI docs #43740696916 PASS; NVRAM #43740696902 remained IN_PROGRESS.
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

- Exact c1987 SHA `c1987b2520611dd1adb1a54e655832c4c61df8b0`: Process inventory/lifecycle #43740697227 FAILED only in the final Python integration checker. All prior C/ASan/UBSan binaries, optional-app checks, IAPPD host lifecycle tests and earlier source contracts passed. The negative contract searched for `system("...");` as a substring and falsely matched assigned `int status = system(...);` statements.
- Exact c1987 AI docs #43740696916 PASS; NVRAM #43740696902 and XY-C1 #43740697118 were still running/queued at their last observed stages; no firmware/TRX/artifact result is claimed.
- Exact db661 Process #43740074825, AI docs #43740074768, NVRAM #43740074736, AdGuardHome #43740074816, WAN route #43740074841, AliyunDrive #43740074721, AliDDNS #43740074783 and infosvr #43740074867 PASS; XY-C1 #43740074742 was still at Build with TRX/artifact pending.
- IAPPD shell lifecycle #43738886473 PASS on `443c23f2f814e69549a49b42c0347f0fd42cfe9e`; 802.1X Process #43739228138, inventory #43739227948 and wireless radio #43739228377 PASS on `ac28f793fe7cf897edfbc26ec6766d21d1d2c70d`; NFS stop fixture #43738132578 PASS on its recorded exact source parent.

## Exact first action for the next GPT

1. Verify exact-head Process lifecycle after replacing the overly broad negative assertions with anchored standalone-statement regexes; check every integration sub-contract through the IAPPD caller step.
2. Recheck XY-C1 #43740697118 and NVRAM #43740696902 for exact c1987 SHA; do not report firmware success without Build, TRX and artifact completion.
3. Continue independent service-wrapper work while remaining jobs are PENDING; do not repeat green IAPPD wrapper or NFS tests without changed source.
4. Keep the backup/archive read-only and all logical changes atomic with expected-SHA, non-forced updates.

## Invariants

- A PENDING run does not block independent work; record exact run ID/SHA and continue safe tasks.
- DNSMasq/VPN lock-order and scoped RC process-signal audits are complete; do not repeat without new evidence.
- Linux 4.4 PID/start_time validation-to-signal TOCTOU remains unresolved. FTP failed-upload cleanup, NFS durability/AUTH_SYS and physical-router behavior remain separate unverified areas.
- Preserve `backup/pre-plugin-overhaul-2026-11-14` and docs/dev-log archive shards / `docs/dev-log/INDEX.md` byte-for-byte.
