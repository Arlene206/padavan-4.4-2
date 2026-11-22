# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state and complete an end-of-session accuracy checkpoint. Record exact main SHA, exact-SHA Actions, test/build boundaries, and the next precise task. If facts remain accurate, explicitly conclude with no file change and no no-op commit. Use one atomic main commit for material changes only. Before context exhaustion, leave an accurate handoff and complete any safe bounded engineering slice.

- Work only on `main`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update `main` only with `force:false` and a freshly fetched expected SHA.
- Check Actions for the exact SHA. PASS continues; FAIL requires diagnosis/correction; PENDING does not block independent work.
- Do not claim local tests, target builds, artifacts or hardware checks that were not performed.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; parent main SHA: `d2e76ea7bbc5e73cd71716ca9e929ce2b9c2e963`.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable.
- EAPoL stop body/helper file scope restored. ttyd uses bounded child-readiness probes; exact Process regression PASS after updating its expected shell-test output marker.
- WSDD2 XML metadata policy now validates strict UTF-8 and XML 1.0 legal code points before escaping. Its compiler/sanitizer test binary passed; source checker required updating.
- Rstats periodic MTD persistence retries failed periodic writes; push/pull path filters include its policy header.

## Active task

The WSDD2 XML reflection policy change is implemented and its GNU99 ASan/UBSan binary completed successfully, but its exact Action then failed because the Python source checker still searched for the deleted old C0-only validation expression. The checker is updated to require the strict UTF-8/XML 1.0 validator, including sequence bounds, EILSEQ/EINVAL error classification and validation-before-allocation.

## Verification boundary

- Exact parent SHA d2e76ea7bbc5e73cd71716ca9e929ce2b9c2e963: WSDD2 XML run #45040593359 FAILED only at the stale checker assertion after the compile/run step; process inventory #45040593394 PASS; AI docs #45040593406 PASS; NVRAM #45040593403 and XY-C1 #45040593342 were in progress at last inspection.
- ttyd readiness source SHA 764042628a2ccd7b90a722d408c10a3d4cfb9056: the first process regression failed only on the checker’s old expected success line; correction commit 30a1e775fef4ab92288e81a9f4eb1ade048993a4 reran Process #45040464894 PASS and AI docs #45040464926 PASS.
- No local checkout/tests were run. WSDD2 exact-source Action rerun is required.
- L3 requires Build XY-C1, TRX verification and artifact upload all PASS on one identical product-source SHA. L3 and hardware L4 are NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head WSDD2 XML Action and the XY-C1 / NVRAM runs after this checker correction; fix demonstrated failures only.
2. Confirm whether the XY-C1 Build, TRX verification and artifact upload all PASS on one identical source SHA; do not infer L3 from a pending or failed build.
3. Continue an independent userland stability/security slice outside completed WAN/PPP, USB modem, Inadyn, DNSMasq, EAPoL and ttyd work. Keep backup immutable and work only on main.
