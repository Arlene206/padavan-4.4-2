# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state and complete an end-of-session accuracy checkpoint. Record exact main SHA, exact-SHA Actions, test/build boundaries, and the next precise task. If facts remain accurate, explicitly conclude with no file change and no no-op commit. Use one atomic main commit for material changes only. Before context exhaustion, leave an accurate handoff and complete any safe bounded engineering slice.

- Work only on `main`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update `main` only with `force:false` and a freshly fetched expected SHA.
- Check Actions for the exact SHA. PASS continues; FAIL requires diagnosis/correction; PENDING does not block independent work.
- Do not claim local tests, target builds, artifacts or hardware checks that were not performed.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; source parent SHA: `cf3727b33b4c8900a080cac08e4825c6e0812d35`; this slice adds ttyd detached-child readiness validation.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable.
- EAPoL credential configuration is quoted, staged privately and atomically published; daemon-stop function and helper scope restored.
- Rstats periodic MTD save remains due and retries after save failure; workflow push/pull filters include its policy header.

## Active task

ttyd's detached `start-stop-daemon -S -b` launcher returned success and was treated as a confirmed service start without checking whether the daemon child materialized. This is now guarded by shared `pl_wait_started()` bounded process probes: only RUNNING returns success; ABSENT and UNKNOWN/interrupted probes propagate distinct failures.

## Verification boundary

- On source parent SHA cf3727b33b4c8900a080cac08e4825c6e0812d35: Process lifecycle #45039929135 PASS; process lifecycle inventory #45039929132 PASS; EAPoL #45039929140 PASS; AI handoff docs #45039929005 PASS.
- XY-C1 #45039929086 remains in Build XY-C1 at last check. NVRAM transaction audit #45039929010 remains in progress. Neither L3 nor hardware L4 is verified.
- This ttyd change and new regression cases have not yet run under exact-head Actions. No local checkout/tests or target build were performed.

## Exact first action for the next GPT

1. Inspect exact-head process-lifecycle and XY-C1 workflow results for this ttyd change. Fix demonstrated failures only.
2. Check whether XY-C1 Build, TRX verification and artifact upload all PASS on one identical product-source SHA; L3 remains NOT VERIFIED otherwise.
3. Inspect completion of NVRAM transaction audit #45039929010 and continue a separate, not-already-covered userland stability/security issue. Keep the frozen backup read-only and work only on main.
