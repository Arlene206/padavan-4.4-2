# Cross-session execution mandate

Recover live state once, then continue the exact first action below. Do not repeat completed audits, stop at planning, or wait on PENDING CI when independent engineering is available. Before ending a session, refresh this file's exact source SHA, Action IDs/statuses, validation boundary and next action.

**Mandatory handoff checkpoint:** verify the meaningful source baseline, active work, exact-head CI, blockers, and first action before context exhaustion. The checkpoint is mandatory; a document edit is conditional. An edit is required only when the record is stale.

For each logical task, publish source, regression tests/contracts and necessary documentation as one atomic main commit. Create every blob, one complete tree, and one commit parented to freshly fetched `main`, then update `main` with that exact `expected_sha` and `force:false`. Do not create partial multi-file commits, force-push, change the frozen backup, or mutate dev-log archive shards. Before context exhaustion, finish the current safe slice, validate it, commit it and leave a usable handoff.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Optional-service caller status source: `8da4da031fa9e0c4fa51ae1208a65be99386c1cd`; checker/docs repair: `1c9ef914b9d27396e90976d6eb0fdfa9ba961cf4`. Exact Process #43741972950 PASS; AI docs #43741972993 still failed because the handoff omitted the required phrase `one atomic main commit` and explicit `Before context exhaustion` rule. This commit adds both. Exact 8da XY-C1 #43741567717 remained in Build XY-C1 at last poll. The preceding 28ccec XY-C1 #43741160797 has since completed SUCCESS, including TRX verification and artifact upload.
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

- Exact optional-service source SHA `8da4da031fa9e0c4fa51ae1208a65be99386c1cd`: Process run #43741567690 failed at a malformed checker splice; the correction in `1c9ef914b9d27396e90976d6eb0fdfa9ba961cf4` makes the complete Process workflow #43741972950 PASS.
- AI handoff docs run #43741567622 first failed because the checkpoint literal was absent; after adding that literal, #43741972993 failed because the handoff omitted the exact `one atomic main commit` and `Before context exhaustion` requirements. This commit restores those required rules; exact-head docs rerun required.
- Exact 8da XY-C1 #43741567717 was still building at last poll. Its predecessor 28ccec XY-C1 #43741160797 completed SUCCESS, including Build XY-C1, Verify TRX output and artifact upload. This does not establish L3 for 8da or later source SHA.
- Exact 1c9f Process #43741972950 PASS; AI docs #43741972993 is the only known failure on that SHA. Backup remains `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## Exact first action for the next GPT

1. Verify exact-head AI handoff docs after restoring the atomic-main-commit and before-context-exhaustion rules.
2. Check XY-C1 #43741567717 on 8da through Build, TRX verification and artifact upload; do not infer those results from the older 28ccec pass.
3. Continue the Firefly configuration publication failure-handling fix: `update_firefly_conf()` still ignores writes/close/rename and `run_itunes()` still starts mt-daapd after that void helper.
4. Preserve frozen backup and archived log shards; use one atomic tree/commit and non-forced expected-SHA update.
## Invariants and unresolved boundaries

- A PENDING run does not block independent work. Record exact SHA/run ID and continue safe tasks.
- Preserve enabled/disabled feature no-op semantics and existing best-effort boot policy unless the repository contract explicitly requires otherwise.
- DNSMasq/VPN lock-order and scoped RC process-signal audits are complete; do not repeat without new evidence.
- Linux 4.4 PID/start_time validation-to-signal TOCTOU remains unresolved. FTP failed-upload cleanup, NFS durability/AUTH_SYS, configuration publication and physical-router behavior are separate open or unverified items.
- Keep `backup/pre-plugin-overhaul-2026-11-14` and docs/dev-log archive shards / `docs/dev-log/INDEX.md` byte-for-byte unchanged.