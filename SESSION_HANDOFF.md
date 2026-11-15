# Cross-session execution mandate (read before resuming)

Recover live state once, then continue the exact P1 task. Do not repeat completed audits, stop at a plan, or ask the user to select work. Continue independent engineering while CI is pending. Before context exhaustion, refresh the exact first action and verification boundary so the next GPT can resume without repeating completed work.

**Mandatory handoff checkpoint:** Every session must verify before ending that this file accurately records the meaningful source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If next-session-critical facts have changed, update this handoff; if it is already accurate, do not create a no-op documentation commit.

For each multi-file logical slice, publish source, tests/contracts and necessary docs in **one atomic main commit**: create all blobs, one tree, one commit parented to freshly fetched main, then non-forced update of main using that exact expected SHA. No branches/PRs, no force-push, no archive modifications.

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; current parent before the storage-backed media launcher status slice is `4f2986624dc72c87f5453e47e822cef8a8e2d667`. IAPPD caller status fix `c1987b2520611dd1adb1a54e655832c4c61df8b0` is committed; exact Process checker correction #43740923564 and AI docs #43740923576 PASS.
- Exact 4f298 Process #43740923564 and AI docs #43740923576 PASS. NVRAM #43740923560 remained IN_PROGRESS at last poll. Exact c1987 XY-C1 #43740697118 remained IN_PROGRESS at Build at last poll.
- The preceding IPTV proxy launch status fix is in `db6615af1928b3c7ea8cd564830d6ccc05d431c7`; exact Process #43740074825 and process inventory artifact #43740074931 PASS. Its XY-C1 #43740074742 was still running at Build at last poll.
- The iperf3 ROMFS source fix is in `a3aa716b5205b0b48f0aa656234fa125e5251542`; exact Process #43739572150, inventory artifact #43739572120 and AI docs #43739572235 PASS. The prior `$(THISDIR)` source defect attempted to install `/iperf3.sh`; source now uses `$(CURDIR)`.
- IAPPD exact Process #43738886473 PASS; 802.1X exact Process #43739228138, inventory #43739227948 and wireless radio #43739228377 PASS. PID1 exact Process #43739381121 and docs #43739381168 PASS. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only.
- No physical-router L4 result is claimed.
## Active task

### P2 — IAPPD boot caller status visibility

- RC `start_services_once()` called `iappd.sh restart`, `modprobe xt_TPROXY` and `iappd.sh test` using `system()` without observing any result. The IAPPD wrapper already propagates launch/stop failures and implements `test`, but RC hid all three statuses. This slice logs nonzero wait statuses in the original call order and retains the best-effort startup policy; the integration checker asserts each assignment, diagnostic and ordering.

### P2 — Storage-backed application launch failure diagnostics

- `run_itunes()`, `run_torrent()` and `run_aria()` discarded the `eval()` result from Firefly, Transmission and Aria launch commands. An enabled service could fail its launch command without an RC diagnostic even though each function already has an enable guard and fail-closed process-presence preflight.
- This slice captures the launcher status and logs the app name plus numeric failure for all three services. Existing NVRAM gates, support checks, RUNNING idempotency and UNKNOWN-inventory refusal remain unchanged.
- Regression assertions in `scripts/check_process_lifecycle_integration.py` verify result capture, failure-log order, enable guards and process-presence gates. The Process workflow now triggers on `services_stor.c`; XY-C1 already includes this source path.
- This is launch-command status visibility only; no new daemon-readiness guarantee is claimed.
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

- Boot diagnostics cover networkmap, watchdog, infosvr, GPIO, LLTD, rstats, telnetd and ttyd. Crond has bounded post-launch verification. DNS Forwarder and SCUT/vlmcsd/iperf3/MentoHUST wrappers preserve start/stop statuses. The NFS stop guard rejects missi## Verification boundary

- Exact parent before this storage-app status slice: `4f2986624dc72c87f5453e47e822cef8a8e2d667`. Fresh Process inventory/lifecycle and Process lifecycle inventory Actions are required for this source/test change. XY-C1 must complete Build, TRX verification and artifact upload on the resulting exact source SHA.
- Exact 4f298 Process #43740923564 and AI docs #43740923576 PASS. NVRAM #43740923560 remained IN_PROGRESS at last poll. Exact c1987 Process #43740697227 failed only in a negative checker assertion that falsely matched assigned `system()` calls; the anchored-regex correction in 4f298 is now PASS.
- Exact c1987 XY-C1 #43740697118 remained IN_PROGRESS at Build at last poll. Exact db661 XY-C1 #43740074742 also remained pending at Build. Neither had verified TRX/artifact evidence at last poll.
- Exact db661 Process #43740074825, process inventory artifact #43740074931, AI docs #43740074768, NVRAM #43740074736, WAN route #43740074841, HTTPD #43740074706, infosvr #43740074867, AdGuardHome #43740074816, AliyunDrive #43740074721, AliDDNS #43740074783 and USB storage #43740074717 completed SUCCESS.
- PID1 exact Process #43739381121 and docs #43739381168 PASS on `d2eebe4401dd9c7968334b71e748ad6c4312b20f`; IAPPD wrapper Process #43738886473 PASS; 802.1X Process #43739228138, inventory #43739227948 and wireless radio #43739228377 PASS. The iperf3 source-path guard passed exact a3aa Process #43739572150.
- Earlier IAPPD XY-C1 #43738581861 and 802.1X XY-C1 #43739227968 failed on pre-iperf-fix source; later db661/c1987 builds include the `$(CURDIR)` correction. No physical-router L4 evidence is claimed.

## Exact first action for the next GPT

1. Check exact-head Process inventory/lifecycle and Process artifact Actions triggered by the storage-app launcher status change; confirm all three source contracts and all workflow steps pass.
2. Verify the exact-head XY-C1 run and the preceding c1987 #43740697118 / db661 #43740074742. Require Build XY-C1, TRX verification and artifact upload before claiming firmware L3.
3. Continue the whole-repository RC/service launch and shutdown audit; candidates include storage media configuration publication failure handling and miniupnpd startup postconditions. Do not repeat green IAPPD, PID1, IPTV proxy or 802.1X source contracts.
4. Re-fetch main before every write; keep backup and archive shards read-only and use one complete Git tree with a non-forced expected-SHA update.
; 802.1X Process #43739228138, inventory #43739227948 and wireless radio #43739228377 PASS on `ac28f793fe7cf897edfbc26ec6766d21d1d2c70d`; NFS stop fixture #43738132578 PASS on its recorded exact source parent.

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
