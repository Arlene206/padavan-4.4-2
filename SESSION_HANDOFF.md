# Cross-session execution mandate

Recover live state once, then continue the exact first action below. Do not repeat completed audits, stop at planning, or wait on PENDING CI when independent engineering is available. Before ending a session, refresh this file's exact source SHA, Action IDs/statuses, validation boundary and next action.

**Mandatory handoff checkpoint:** verify the meaningful source baseline, active work, exact-head CI, blockers, and first action before context exhaustion. An edit is required only when the record is stale.

For each logical slice, create every blob, one complete tree, and one commit parented to freshly fetched `main`, then update `main` with that exact `expected_sha` and `force:false`. Do not create partial multi-file commits, force-push, change the frozen backup, or mutate dev-log archive shards.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Current parent before the optional-service caller status slice: `28ccec681975863be1dc59580166fdd3a7774655`. This slice captures and logs six previously ignored startup results in `start_services_once()`; its result is the current commit following the atomic push.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`. It is read-only.
- No physical-router L4 result is claimed. Do not claim XY-C1 firmware L3 without exact-SHA Build, TRX verification and artifact upload.

## Active task

### P2 — Optional RC boot service status visibility

- `start_services_once()` discarded the return values of `doSystem("/usr/sbin/skipd -d /etc/storage/db")`, `start_scutclient()`, `start_dnsforwarder()`, `start_vlmcsd()`, `start_iperf3()` and `start_mentohust()`. Their helpers already return command/start status and have their own feature/enable guards; the boot caller hid failures.
- This slice captures each result and logs nonzero status while preserving the existing compile-time `APP_*` guards, order and best-effort boot semantics. It deliberately does not turn optional-service failure into a boot abort.
- The process-lifecycle integration checker asserts assignment, feature-guard containment, nonzero branch, diagnostic and original order; it also rejects standalone fire-and-forget calls. `services.c` is already included in Process and XY-C1 path filters.

### Previously completed source slices

- NFS `pidof` fail-closed gates on start/reload/stop: `c736d4a8eccfe58f3106cdb63acc8b47963e6b98`; fixture newline correction: `479c05be3bef946f892f81a296d0ca3e709a698f`; exact NFS start/reload and stop fixture runs #43737106642 / #43738132578 PASS.
- Ralink IAPPD self-fork wrapper, tri-state process inventory, bounded start/stop postconditions and RC `test`: `37204498a3ce81d6dd077fd551a66e15a29be344`; test status-capture correction `443c23f2f814e69549a49b42c0347f0fd42cfe9e`; Process lifecycle #43738886473 PASS.
- 802.1X 5GHz/2.4GHz authenticator launch status logging: `ac28f793fe7cf897edfbc26ec6766d21d1d2c70d`; Process #43739228138, inventory #43739227948 and wireless radio #43739228377 PASS.
- iperf3 ROMFS install path uses `$(CURDIR)`: `a3aa716b5205b0b48f0aa656234fa125e5251542`; Process #43739572150 and inventory artifact #43739572120 PASS. Require a current exact-head firmware run for L3.
- IPTV proxy launch status logging for udpxy, MSD Lite and xupnpd: `db6615af1928b3c7ea8cd564830d6ccc05d431c7`; Process #43740074825 PASS. Background `system("... &")` only reports shell status, not child readiness.
- IAPPD RC caller observes restart, xt_TPROXY load and post-start test status: `c1987b2520611dd1adb1a54e655832c4c61df8b0`; checker false-positive fixed by anchored regex in `4f2986624dc72c87f5453e47e822cef8a8e2d667`.
- Storage-backed media launch status logging for Firefly/Transmission/Aria: `28ccec681975863be1dc59580166fdd3a7774655`.

## Verification boundary

- Exact parent `28ccec681975863be1dc59580166fdd3a777465`: Process inventory/lifecycle #43741160702 PASS; AI handoff docs #43741160792 PASS; process inventory artifact #43741160766 PASS; NVRAM #43741160737 PASS; USB storage #43741160723 PASS; infosvr #43741160726 PASS; NFS #43741160739 PASS; AdGuardHome #43741160924 PASS; HTTPD #43741160902 PASS; Samba #43741160807 PASS; AliDDNS #43741160758 PASS; AliyunDrive #43741160680 PASS; vsftpd #43741160663 PASS.
- Exact 28ccec XY-C1 #43741160797 was IN_PROGRESS at `Build XY-C1`; TRX verification and artifact upload had not yet completed at last poll. This optional-service caller change requires a new exact-head XY-C1 run.
- Exact Process #43740923564 and AI docs #43740923576 on `4f2986624dc72c87f5453e47e822cef8a8e2d667` PASS, including the anchored IAPPD raw-call contract correction.

## Exact first action for the next GPT

1. Fetch live `main`, then check the exact-head Process inventory/lifecycle run created by this commit. Confirm all six optional service caller contracts pass and inspect any first failing assertion.
2. Check the new exact-head XY-C1 run through Build XY-C1, TRX verification and artifact upload; the prior 28ccec run #43741160797 was still building and cannot verify this new source SHA.
3. Continue independent RC launch/shutdown auditing while CI is pending. Next likely targets: `start_watchdog_cpu()` module-load status, `start_crond()` postcondition, and miniupnpd startup/readiness; do not re-audit NFS, IAPPD, 802.1X, IPTV or storage media launch contracts without new source changes.
4. Preserve the backup and archive invariants; use a complete Git tree and expected-SHA non-forced main update for every logical change.

## Invariants and unresolved boundaries

- A PENDING run does not block independent work. Record exact SHA/run ID and continue safe tasks.
- Preserve enabled/disabled feature no-op semantics and existing best-effort boot policy unless the repository contract explicitly requires otherwise.
- DNSMasq/VPN lock-order and scoped RC process-signal audits are complete; do not repeat without new evidence.
- Linux 4.4 PID/start_time validation-to-signal TOCTOU remains unresolved. FTP failed-upload cleanup, NFS durability/AUTH_SYS, configuration publication and physical-router behavior are separate open or unverified items.
- Keep `backup/pre-plugin-overhaul-2026-11-14` and docs/dev-log archive shards / `docs/dev-log/INDEX.md` byte-for-byte unchanged.