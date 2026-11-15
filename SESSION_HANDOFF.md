# Cross-session execution mandate

Recover live state once, then continue the exact first action below. Do not repeat completed audits, stop at planning, or wait on PENDING CI when independent engineering is available. Before ending a session, refresh this file's exact source SHA, Action IDs/statuses, validation boundary and next action.

**Mandatory handoff checkpoint:** verify the meaningful source baseline, active work, exact-head CI, blockers, and first action before context exhaustion. The checkpoint is mandatory; a document edit is conditional. An edit is required only when the record is stale.

For each logical task, publish source, regression tests/contracts and necessary documentation as one atomic main commit. Create every blob, one complete tree, and one commit parented to freshly fetched `main`, then update `main` with that exact `expected_sha` and `force:false`. Do not create partial multi-file commits, force-push, change the frozen backup, or mutate dev-log archive shards. Before context exhaustion, finish the current safe slice, validate it, commit it and leave a usable handoff.

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; exact parent before Firefly config publication is `2bd4868069048e5470fcfd024678b0891575b12a`.
- Optional-service caller checker repair: `1c9ef914b9d27396e90976d6eb0fdfa9ba961cf4` exact Process #43741972950 PASS; handoff docs follow-up `2bd4868069048e5470fcfd024678b0891575b12a` exact Process #43742092499 and AI docs #43742092447 PASS.
- Optional RC boot status source: `8da4da031fa9e0c4fa51ae1208a65be99386c1cd`; its Process #43741567690 originally failed on checker syntax only, corrected at 1c9f. XY-C1 #43741567717 remained in Build XY-C1 at last poll.
- Previous storage media daemon launch status source: `28ccec681975863be1dc59580166fdd3a7774655`; exact XY-C1 #43741160797 completed SUCCESS through TRX verification and artifact upload.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only. No physical-router L4 result is claimed.
## Active task

### P2 — Firefly configuration publication failure handling

- `update_firefly_conf()` was `void` and ignored source-open, temporary-file open, write, close and rename errors. `run_itunes()` launched mt-daapd regardless, so a failed update could leave stale or partial configuration.
- This slice adds a unique same-directory temporary-file publisher. It checks source reads, writes, flush, `fsync`, close and final rename; failed publication removes the staged file and preserves the previous config. `run_itunes()` checks directory creation and legacy migration, logs failures and returns before starting mt-daapd.
- A host C regression covers existing-config rewrite/preservation, default creation, last-good preservation on read/rename failure, 0600 permissions and temporary-file cleanup. Process runs it under ASan/UBSan; static contracts and XY-C1 path filters cover the new helper.
- This guarantees config publication only, not daemon runtime readiness.
### P2 — Optional RC boot service status visibility

- `start_services_once()` discarded the return values of `doSystem("/usr/sbin/skipd -d /etc/storage/db")`, `start_scutclient()`, `start_dnsforwarder()`, `start_vlmcsd()`, `start_iperf3()` and `start_mentohust()`. Their helpers already return command/start status and have their own feature/enable guards; the boot caller hid failures.
- This slice captures each result and logs nonzero status while preserving the existing compile-time `APP_*` guards, order and best-effort boot semantics. It deliberately does not turn optional-service failure into a boot abort.
- The process-lifecycle integration checker asserts assignment, feature-guard containment, nonzero branch, diagnostic and original order; it also rejects standalone fire-and-forget calls. `services.c` is already included in Process and XY-C1 path filters.

### Previously completed source slices

- NFS `pidof` fail-closed gates on start/reload/stop: `c736d4a8eccfe58f3106cdb63acc8b47963e6b98`; fixture newline correction: `479c05be3bef946f892f81a296d0ca3e709a698f`; exact NFS start/reload and stop fixture runs #43737106642 / #43738132578 PASS.
- Ralink IAPPD self-fork wrapper, tri-state process inventory, bounded start/stop postconditions and RC `test`: `37204498a3ce81d6dd077fd551a66e15a## Verification boundary

- Exact parent before Firefly atomic config publication: `2bd4868069048e5470fcfd024678b0891575b12a`. The new helper/source contract and host regression require fresh exact-head Process PASS; XY-C1 must complete Build, TRX verification and artifact upload.
- Exact 2bd Process #43742092499 and AI handoff docs #43742092447 PASS; exact 1c9f Process #43741972950 PASS.
- Exact 8da XY-C1 #43741567717 remained IN_PROGRESS at Build XY-C1 at last poll; TRX and artifact upload were pending. Exact 28ccec XY-C1 #43741160797 completed SUCCESS through Build, TRX verification and artifact upload; that result does not validate 8da or later changes.
- No physical-router L4 evidence is claimed. Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## Exact first action for the next GPT

1. Check the exact-head Process workflow for the Firefly publisher regression under ASan/UBSan and source-contract checks; diagnose the first failure if any.
2. Check exact-head XY-C1 Build, TRX output verification and artifact upload. Do not infer current-source firmware validation from the older 28ccec pass.
3. Continue independent reliability/security work while CI is pending; after Firefly, review other storage-backed config publishers for concrete ignored write/close/rename failures. Do not repeat completed launcher-status checks.
4. Keep backup/archive shards read-only and use a complete tree/commit with non-forced expected-SHA main update.
n required.
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