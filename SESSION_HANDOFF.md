# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state and complete an end-of-session accuracy checkpoint. Record exact main SHA, exact-SHA Actions, test/build boundaries, and the next precise task. If facts remain accurate, explicitly conclude with no file change and no no-op commit. Use one atomic main commit for material changes only. Before context exhaustion, leave an accurate handoff and complete any safe bounded engineering slice.

- Work only on `main`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update `main` only with `force:false` and a freshly fetched expected SHA.
- Check Actions for the exact SHA. PASS continues; FAIL requires diagnosis/correction; PENDING does not block independent work.
- Do not claim local tests, target builds, artifacts or hardware checks that were not performed.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; parent main SHA: `30a1e775fef4ab92288e81a9f4eb1ade048993a4`.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable.
- EAPoL stop function/file-scope config helper restored; ttyd detached startup now waits for confirmed child readiness; WSDD2 XML escaping now rejects malformed UTF-8 and XML 1.0-forbidden text.
- Rstats periodic MTD save remains due and retries after save failure; push/pull workflow filters include its persistence policy header.

## Active task

WSDD2 reflects NVRAM/configuration and hostname metadata into XML 1.0 SOAP discovery responses. The prior escaping helper removed markup characters and most C0 controls, but did not validate UTF-8 or the XML 1.0 Char production; malformed encodings, surrogate encodings, out-of-range code points and U+FFFE could produce invalid response XML. Strict validation now runs before escaping, with distinct EILSEQ/EINVAL failure status.

## Verification boundary

- Parent SHA 30a1e775fef4ab92288e81a9f4eb1ade048993a4: ttyd checker output-marker correction committed; exact process and AI runs #45040464894/#45040464926 were in progress at last inspection.
- Previous ttyd source SHA 764042628a2ccd7b90a722d408c10a3d4cfb9056: AI handoff #45040236379 PASS; process inventory #45040236380 PASS; process lifecycle regression #45040236371 failed only on the old output-marker assertion, now corrected in parent 30a1; exact rerun pending. XY-C1 #45040236370 remained in progress.
- This WSDD2 policy/test change has not yet run its exact-head GNU99 ASan/UBSan workflow. No local checkout/tests were run.
- L3 requires Build XY-C1, TRX verification and artifact upload all PASS on one identical product-source SHA. L3 and hardware L4 are NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head WSDD2 XML regression, process-lifecycle regression and XY-C1 results after this commit; fix only demonstrated failures.
2. Confirm whether ttyd readiness tests pass on source SHA 764042628a2ccd7b90a722d408c10a3d4cfb9056 and whether XY-C1 Build/TRX/artifact all PASS on one current exact SHA.
3. Continue an independent userland stability/security slice outside completed WAN/PPP, USB modem, Inadyn, DNSMasq, EAPoL and ttyd work. Keep backup immutable and work only on main.
