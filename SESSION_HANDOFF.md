# Cross-session execution mandate

Recover live state once, then continue the exact first action below. Do not repeat completed audits, stop at planning, or wait on PENDING CI when independent engineering is available. Before ending a session, refresh this file's exact source SHA, Action IDs/statuses, validation boundary and next action.

**Mandatory handoff checkpoint:** verify the meaningful source baseline, active work, exact-head CI, blockers, and first action before context exhaustion. The checkpoint is mandatory; a document edit is conditional. An edit is required only when the record is stale.

For each logical task, publish source, regression tests/contracts and necessary documentation as one atomic main commit. Create every blob, one complete tree, and one commit parented to freshly fetched `main`, then update `main` with that exact `expected_sha` and `force:false`. Do not create partial multi-file commits, force-push, change the frozen backup, or mutate dev-log archive shards. Before context exhaustion, finish the current safe slice, validate it, commit it and leave a usable handoff.

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; exact parent before the MiniDLNA config publication slice is `d3531270e9d3cf9523f2c0b8f0dc577b687acafb`.
- Firefly atomic config publication is committed at `d3531270e9d3cf9523f2c0b8f0dc577b687acafb`; exact Process #43742499910 passed all steps, including the GNU89 ASan/UBSan publication regression and static source contract. AI handoff docs #43742501048 and process inventory artifact #43742499867 also PASS.
- Exact d353 XY-C1 #43742499742 was still at dependency installation at the last poll; Build XY-C1, TRX verification and artifact upload remain pending.
- Optional RC caller status source is `8da4da031fa9e0c4fa51ae1208a65be99386c1cd`; exact Process was repaired and passed at #43741972950. The older 28ccec XY-C1 #43741160797 completed full Build/TRX/artifact SUCCESS.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only. No physical-router L4 result is claimed.
## Active task

### P2 — MiniDLNA config publication failure handling

- `update_minidlna_conf()` opened `/etc/minidlna.conf` with `"w"`, truncating the active config immediately; it ignored all `fprintf()` and `fclose()` results. `run_dms()` then launched minidlnad even when the config write had failed or left partial content.
- This slice stages a complete generated config in a unique same-directory file and checks each write, flush, `fsync`, close and final rename. Failure removes the stage and leaves the previous file intact; `run_dms()` logs and refuses daemon launch if publication fails.
- The host regression checks all config branches, media-source omission, root-container/sort settings, replacement of old content, restrictive 0600 mode and cleanup after a failed rename. Process runs it under ASan/UBSan, and the XY-C1 path filter tracks the helper header.
- The guarantee is atomic config publication only, not DLNA socket/discovery readiness.
### P2 — Firefly configuration publication failure handling

- `update_firefly_conf()` was `void` and ignored source-open, temporary-file open, write, close and rename errors. `run_itunes()` launched mt-daapd regardless, so a failed update could leave stale or partial configuration.
- This slice adds a unique same-directory temporary-file publisher. It checks source reads, writes, flush, `fsync`, close and final rename; failed publication removes the staged file and preserves the previous config. `run_itunes()` checks directory creation and legacy migration, logs failures and returns before starting mt-daapd.
- A host C regression covers existing-config rewrite/preservation, default creation, last-good preservation on read/rename failure, 0600 permissions and temporary-file cleanup. Process runs it under ASan/UBSan; static contracts and XY-C1 path filters cover the new helper.
- This guarantees config publication only, not daemon runtime readiness.
### P2 — Optional RC boot service status visibility

- `start_services_once()` discarded the return values of `doSystem("/usr/sbin/skipd -d /etc/storage/db")`, `start_scutclient()`, `start_dnsforwarder()`, `start_vlmcsd()`, `start_iperf3()` and `start_mentohust()`. Their helpers already return command/start status and have ## Verification boundary

- Exact parent before MiniDLNA atomic config publication: `d3531270e9d3cf9523f2c0b8f0dc577b687acafb`. Require fresh exact-head Process regression and static contracts, plus XY-C1 Build, TRX verification and artifact upload.
- Firefly atomic config publication at exact SHA `d3531270e9d3cf9523f2c0b8f0dc577b687acafb`: Process #43742499910 PASS; all 20 job steps completed SUCCESS, including `Compile and run Firefly atomic configuration publication regression`. Process inventory artifact #43742499867 and AI docs #43742501048 PASS.
- Exact d353 XY-C1 #43742499742 remained IN_PROGRESS at dependency installation at last poll; no build/TRX/artifact result yet. Do not infer current firmware L3 from the historical 28ccec pass.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; no physical-router L4 evidence is claimed.

## Exact first action for the next GPT

1. Check the exact-head Process workflow for the MiniDLNA atomic publisher ASan/UBSan regression and checker; fix the first actual test or source failure.
2. Check exact-head XY-C1 through Build, TRX verification and artifact upload; keep it distinct from the older passing 28ccec build.
3. Continue independent user-space stability/security work while XY-C1 is pending. Do not repeat Firefly source/test contracts without new evidence.
4. Keep the frozen backup and archive shards read-only; use one complete tree/commit and non-forced expected-SHA update.
 predecessor 28ccec XY-C1 #43741160797 completed SUCCESS, including Build XY-C1, Verify TRX output and artifact upload. This does not establish L3 for 8da or later source SHA.
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