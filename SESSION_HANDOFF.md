# Cross-session execution mandate (read before resuming)

Recover live state once, then continue the exact P1 task. Do not repeat completed audits, stop at a plan, or ask the user to select work. Continue independent engineering while CI is pending. Before context exhaustion, refresh the exact first action and verification boundary so the next GPT can resume without repeating completed work.

**Mandatory handoff checkpoint:** Every session must verify before ending that this file accurately records the meaningful source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If next-session-critical facts have changed, update this handoff; if it is already accurate, do not create a no-op documentation commit.

For each multi-file logical slice, publish source, tests/contracts and necessary docs in **one atomic main commit**: create all blobs, one tree, one commit parented to freshly fetched main, then non-forced update of main using that exact expected SHA. No branches/PRs, no force-push, no archive modifications.

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; main parent before the iperf3 ROMFS fix is `d2eebe4401dd9c7968334b71e748ad6c4312b20f`. IAPPD lifecycle code/test is at `37204498a3ce81d6dd077fd551a66e15a29be344`, with corrected fixture exit-status capture at `443c23f2f814e69549a49b42c0347f0fd42cfe9e`. The 802.1X diagnostics are at `ac28f793fe7cf897edfbc26ec6766d21d1d2c70d`; exact Process lifecycle #43739228138 and process inventory #43739227948 passed.
- Current XY-C1 run #43739227968 for the 802.1X/IAPPD source stack was IN_PROGRESS at Build XY-C1 at last poll. Earlier exact XY-C1 #43737703922 on SHA `776b47eea3728e44944e04aeef67e4e0fe750f9a` exposed the iperf3 ROMFS install-path failure detailed below.
- Optional RC boot wrappers for AdGuardHome, DDNS.to, Adbyby and AliDDNS retain/log launcher failures. NFS stop regression #43738132578 passed on source parent `479c05be3bef946f892f81a296d0ca3e709a698f`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only). No local checkout/MIPS toolchain or physical-router L4 result is claimed.

## Active task

### P1 — iperf3 ROMFS package install path blocks XY-C1 firmware

- Exact XY-C1 push run #43737703922 on SHA `776b47eea3728e44944e04aeef67e4e0fe750f9a` compiled through package ROMFS, then failed at `trunk/user/iperf3/Makefile`: `$(THISDIR)` was empty under `make -C iperf3 romfs`, so install attempted to copy `/iperf3.sh` and failed with “cannot stat”. TRX verification and artifact upload were skipped. The earlier groff HTML warning and SmartDNS mirror 404s were nonfatal; the SmartDNS Git fallback completed.
- This atomic slice changes the script install source to GNU make's `$(CURDIR)` and adds a package-path regression assertion to `scripts/check_optional_app_start_status.py`. That checker runs in the process-lifecycle workflow; the Makefile is in the XY-C1 path filter. Require fresh exact-head process and complete XY-C1/TRX/artifact results.
- Continue the remaining `start_services_once()` / optional service launch audit after this build blocker is closed; preserve enable guards and disabled-service no-op semantics.

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

- IAPPD test run #43738581837 initially failed because the fixture c## Verification boundary

- Exact parent before this fix: `d2eebe4401dd9c7968334b71e748ad6c4312b20f`. XY-C1 run #43737703922 on source SHA `776b47eea3728e44944e04aeef67e4e0fe750f9a` failed in iperf3 ROMFS at `install: cannot stat '/iperf3.sh'`; TRX verification and artifact upload were skipped. This fix changes the source path to `$(CURDIR)/iperf3.sh`; exact-head XY-C1 must pass Build, Verify TRX output and upload before L3 can be claimed.
- 802.1X source SHA `ac28f793fe7cf897edfbc26ec6766d21d1d2c70d`: Process inventory and lifecycle regression #43739228138 PASS; Process lifecycle inventory #43739227948 PASS. XY-C1 #43739227968 was IN_PROGRESS at the last poll; inspect its outcome, but do not treat it as validation of this iperf3 change.
- IAPPD fixture correction: Process lifecycle #43738886473 and AI docs #43738886520 PASS on SHA `443c23f2f814e69549a49b42c0347f0fd42cfe9e`. XY-C1 #43738581861 for source SHA `37204498a3ce81d6dd077fd551a66e15a29be344` was IN_PROGRESS at the last poll.
- PID 1 workflow on SHA `c0999a97f07435c11903dac04524a07844cc60a8` failed at the stale telnetd assertion expecting `return eval("telnetd");`; the implementation now captures/logs/returns `start_status`. This checker correction is in `d2eebe4401dd9c7968334b71e748ad6c4312b20f`; exact-head rerun required.
- Existing NFS stop regression #43738132578, process lifecycle #43738132620 and their source-parent results remain PASS on their recorded exact SHAs. No full-firmware L3 or physical-router L4 claim is made without corresponding evidence.

## Exact first action for the next GPT

1. Verify the process-lifecycle and XY-C1 push runs whose `head_sha` exactly matches the commit containing this iperf3 fix. Inspect Build XY-C1, TRX verification and artifact upload; diagnose the first causal failure.
2. Recheck PID 1 notification queue and AI handoff docs Actions triggered by `d2eebe4401dd9c7968334b71e748ad6c4312b20f`; diagnose the next contract failure if the workflow does not pass.
3. Inspect the result of XY-C1 #43739227968 on `ac28f793fe7cf897edfbc26ec6766d21d1d2c70d`, but keep it distinct from the new exact-head build.
4. Once the build blocker is closed, continue the whole-repository RC startup-wrapper audit; do not repeat IAPPD, 802.1X or NFS host regressions without new evidence.

ntinue safe tasks.
- DNSMasq/VPN lock-order and scoped RC process-signal audits are complete; do not repeat without new evidence.
- Linux 4.4 PID/start_time validation-to-signal TOCTOU remains unresolved. FTP failed-upload cleanup, NFS durability/AUTH_SYS and physical-router behavior remain separate unverified areas.
- Preserve docs/dev-log archive shards and docs/dev-log/INDEX.md byte-for-byte.
