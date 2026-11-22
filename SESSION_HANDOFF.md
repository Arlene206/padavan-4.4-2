# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect live repository state and complete an end-of-session accuracy checkpoint. Record exact main SHA, exact-SHA Actions, test/build boundaries and the next precise task. If facts remain accurate, explicitly conclude with no file change and no no-op commit. Use one atomic main commit for material changes only. Before context exhaustion, leave an accurate handoff and complete any safe bounded engineering slice.

- Work only on `main`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update `main` only with `force:false` and a freshly fetched expected SHA.
- PASS continues, FAIL requires diagnosis/correction, PENDING does not block independent work.
- Do not claim local tests, production builds, TRX/artifacts or hardware checks that were not performed.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Parent main SHA before this regression-fixture correction: `a633cf309bd672b2c8111a9c09ba891223f8f1ba`.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; keep immutable. Only `main` and the backup branch exist.
- Exact parent Process inventory #45061097595 PASS and Process lifecycle fail-closed #45061097583 PASS.
- Exact parent Adbyby #45061097586 FAIL and HTTPD security #45061097680 FAIL at the shared `firewall_snapshot.sh` regression. All preceding host regressions passed. The failing fixture wrote two literal backslashes before `n`, so its later exact-line grep could not match; the repair uses `printf '%s\\n'` to write a real newline.
- Exact parent XY-C1 #45061097611 remained IN_PROGRESS at cross-build setup; Build, TRX and artifact upload are not verified. Storage #45060756356 on ancestor `790fb2a` PASS, including the swap inventory and final-LF tests.
- No local repo checkout or router hardware is available. Hardware L4 remains NOT VERIFIED.

## Active task

Fix the shared Adbyby firewall-snapshot regression fixture that failed both exact-parent Adbyby and HTTPD workflows. The production change in `a633cf3` correctly preserves a failed `adbyby_remove_rule_snapshot()` status after attempting kernel/ipset cleanup, refusing to log “closed” success when stale recovery state could survive. The new test override emitted `snapshot-remove-attempted\\n` with two backslashes, producing literal backslash-n bytes; the subsequent `grep -Fqx snapshot-remove-attempted` therefore failed. Replace the fixture with a single-format `printf '%s\\n' 'snapshot-remove-attempted'`. No production source change is needed for this test-only correction.

After exact Adbyby and HTTPD reruns pass, continue the PID 1 signal mask validity slice: `init_signals()` currently ignores `sigemptyset()` and `sigaddset()` return values while constructing handler masks. Add injectable set-build wrappers/tests and make these failures feed its first-error status, which already blocks router readiness.

## Verification boundary

- Parent main: `a633cf309bd672b2c8111a9c09ba891223f8f1ba`; resulting main SHA must be read from the live ref after publication.
- Exact parent Process inventory #45061097595 and Process fail-closed #45061097583 PASS.
- Exact parent Adbyby #45061097586 and HTTPD #45061097680 both fail in the same `firewall_snapshot.sh` test after its prior setup tests pass; root cause is the two-backslash test printf fixture, not the production cleanup behavior.
- Fix is test-only: emit the marker as one newline-terminated line, preserving all production code and the assertions that verify every firewall cleanup stage is attempted after snapshot removal fails.
- Local checkout is unavailable; fresh exact-head Adbyby and HTTPD actions are required. XY-C1 #45061097611 on the parent was still running; exact same-source Build XY-C1, TRX verification and artifact upload are required for L3. L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch live `main` and match exact-head Adbyby, HTTPD, Process and XY-C1 runs. Confirm the corrected firewall snapshot regression completes and that all kernel/IPSet cleanup assertions pass; diagnose any new failure from its first causal log. Confirm Build XY-C1, TRX and artifact upload on the same source SHA. Then implement checked PID 1 signal-set construction and its GNU89 injectable regression.

