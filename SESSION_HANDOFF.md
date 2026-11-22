# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect live repository state and complete an end-of-session accuracy checkpoint. Record exact main SHA, exact-SHA Actions, test/build boundaries and the next precise task. If facts remain accurate, explicitly conclude with no file change and no no-op commit. Use one atomic main commit for material changes only. Before context exhaustion, leave an accurate handoff and complete any safe bounded engineering slice.

- Work only on `main`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update `main` only with `force:false` and a freshly fetched expected SHA.
- PASS continues; FAIL requires diagnosis/correction; PENDING does not block independent work.
- Do not claim local tests, firmware builds, TRX/artifacts or hardware checks that were not performed.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; parent main SHA: `0d7275cc91b2ab922a57c142c688c43f442fd25b`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807` remains immutable.
- p910nd transient I/O, EAPoL orphan-tail cleanup, ttyd child readiness, WSDD2 strict XML validation and AliyunDrive HTTP readiness are committed.
- AliDDNS uses a NUL-delimited config digest including the custom script digest. The test-root path was corrected; this commit corrects lowercase Python checker bindings.

## Active task

Rerun AliDDNS API transport validation after fixing its Python source-contract checker. New fingerprint assertions previously referenced undefined uppercase SOURCE/WORKFLOW identifiers, but the existing checker binds lowercase `source` and `workflow`. These references are now corrected. Confirm the test script extracts production source from the repository root, then diagnose any actual regression failures.

## Verification boundary

- Exact actions on parent 0d7275cc91b2ab922a57c142c688c43f442fd25b: none returned.
- AliyunDrive fixture-isolation run #45043595742 on c32d2d3955b2f986494f070c1272d9fac4add50e: completed/failure.
- AliDDNS fingerprint action #45043730798 on 3556d297e887ad81b5457e16a33b32eafadafa9a: completed/failure; its prior checker execution raised a binding NameError, now corrected. Rerun required.
- No local Python/shell tests, firmware build, TRX/artifact or hardware checks were performed. L3 requires Build, TRX verification and artifact upload all PASS on one identical product-source SHA; L3/L4 remain NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head AliDDNS API transport Action after this checker fix. Correct real source/test failure if any; do not weaken the NUL-boundary collision regression.
2. Inspect AliyunDrive HTTP readiness Action after fixture isolation. Confirm the test mock reports the daemon running through all bounded HTTP probes and verify delayed-listener and unhealthy-duplicate cases.
3. Inspect current XY-C1 same-SHA Build/TRX/artifact statuses, then continue another independent userland stability/security task. Keep backup immutable and work only on main.
