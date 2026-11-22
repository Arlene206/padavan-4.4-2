# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state and complete an end-of-session accuracy checkpoint. Record exact main SHA, exact-SHA Actions, test/build boundaries, and the next precise task. If facts remain accurate, explicitly conclude with no file change and no no-op commit. Use one atomic main commit for material changes only. Before context exhaustion, leave an accurate handoff and complete any safe bounded engineering slice.

- Work only on main; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update main only with force:false and a freshly fetched expected SHA.
- Check Actions for the exact SHA. PASS continues; FAIL requires diagnosis/correction; PENDING does not block independent work.
- Do not claim local tests, target builds, artifacts or hardware checks that were not performed.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; parent main SHA: 764042628a2ccd7b90a722d408c10a3d4cfb9056.
- Frozen backup: backup/pre-plugin-overhaul-2026-11-14 at 72f4f157a084f8184b9597ca5bf334b8080e5807; immutable.
- EAPoL credential configuration is quoted, staged privately and atomically published; its daemon-stop function has been restored.
- Rstats periodic MTD save remains due and retries after a failed storage write.
- ttyd detached startup now waits for shared process-inventory confirmation before returning success.

## Active task

The exact-SHA Process lifecycle regression #45040236371 on 764042628a2ccd7b90a722d408c10a3d4cfb9056 failed only because scripts/check_optional_app_start_status.py still expected the old ttyd test success line. The test was expanded to cover detached-child absence, UNKNOWN inventory, and interrupted readiness waits, so its success line now includes detached-child readiness. The script and test themselves returned success; this is a stale output-string contract, not a production or shell-test failure. This commit aligns the checker.

## Verification boundary

- On ancestor cf3727b33b4c8900a080cac08e4825c6e0812d35: EAPoL regression #45039929140 PASS; process inventory #45039929132 PASS; process lifecycle #45039929135 PASS; AI handoff #45039929005 PASS. XY-C1 #45039929086 was still at Build XY-C1 and NVRAM audit #45039929010 still running at the last check.
- Current main 764042628a2ccd7b90a722d408c10a3d4cfb9056: process lifecycle #45040236371 FAILED at the checker output assertion (line 1098); ttyd_lifecycle.sh returned 0 and emitted the newer success marker. Exact-head run after this checker fix is pending.
- No local checkout or local C/Python tests were run in this connector-only session. XY-C1 TRX/artifact completion is not verified on the current source SHA. L3 and hardware L4 remain NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect the exact-head process-lifecycle rerun after the ttyd marker correction, and check EAPoL, rstats, detect-link and NVRAM audits for any new failures.
2. Check XY-C1 Build, TRX verification and artifact upload on one identical product-source SHA; do not infer L3 from ancestor runs.
3. Continue a separate userland stability/security slice after those checks. Keep the frozen backup read-only and work only on main.
