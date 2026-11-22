# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state and complete an end-of-session accuracy checkpoint. Record exact main SHA, exact-SHA Actions, test/build boundaries, and the next precise task. If facts remain accurate, explicitly conclude with no file change and no no-op commit. Use one atomic main commit for material changes only. Before context exhaustion, leave an accurate handoff and complete any safe bounded engineering slice.

- Work only on `main`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update `main` only with `force:false` and a freshly fetched expected SHA.
- Check Actions for the exact SHA. PASS continues; FAIL requires diagnosis/correction; PENDING does not block independent work.
- Do not claim local tests, target builds, artifacts or hardware checks that were not performed.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; parent main SHA: `e2a842d08bae9e4f0e64a519e10c13c0444888f7`; this slice removes an orphaned EAPoL source tail and extends its compile-boundary contract.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807` remains immutable.
- EAPoL temporary config creation and stop-helper file scope remain corrected; ttyd detached-child readiness has Process/AI PASS on checker descendant 30a1; WSDD2 strict XML UTF-8 validation has WSDD XML/AI PASS on checker descendant e2.

## Active task

The previous EAPoL compile repair restored the missing `stop_auth_eapol()` body and moved `write_wpa_config_temp()` to file scope but left a stale suffix of the former startup function after `start_auth_eapol()` had been closed. XY-C1 caught those orphan tokens at auth.c:292–298. They are removed, and the lifecycle checker now verifies the source gap before `wpacli_main()` contains no stray tokens.

## Verification boundary

- Parent exact checker SHA e2a842d08bae9e4f0e64a519e10c13c0444888f7: WSDD2 XML regression #45040760516 PASS; AI docs #45040760489 PASS.
- Parent product-source SHA d2e76ea7bbc5e73cd71716ca9e929ce2b9c2e963: XY-C1 #45040593342 FAILED at Build with compiler syntax errors caused by orphan `auth.c` lines 292–298. The build did not reach TRX verification/artifact upload; no L3 claim.
- Parent d2 source SHA: Process inventory #45040593394 PASS and NVRAM audit #45040593403 PASS. This EAPoL repair and updated Process checker have not yet run exact-head Actions. No local build was performed.
- L3 requires Build XY-C1, TRX verification and artifact upload all PASS on one identical product-source SHA. Hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect this commit's exact-head Process regression and XY-C1 run. Confirm the `auth.c` compilation succeeds and that Build, TRX verification and artifact upload all PASS on one SHA.
2. If any further compile error is reported, fix the actual source defect; do not weaken source assertions to hide the compiler result.
3. Continue a different, not-already-completed userland stability/security slice; backup remains read-only, development only on main.
