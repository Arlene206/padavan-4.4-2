# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect live repository state and complete an end-of-session accuracy checkpoint. Record exact main SHA, exact-SHA Actions, test/build boundaries and the next precise task. If facts remain accurate, explicitly conclude with no file change and no no-op commit. Use one atomic main commit for material changes only. Before context exhaustion, leave an accurate handoff and complete any safe bounded engineering slice.

- Work only on `main`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update `main` only with `force:false` and a freshly fetched expected SHA.
- PASS continues; FAIL requires diagnosis/correction; PENDING does not block independent work.
- Do not claim local tests, firmware builds, TRX/artifacts or hardware checks that were not performed.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; parent main SHA: `c32d2d3955b2f986494f070c1272d9fac4add50e`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807` remains immutable.
- p910nd transient I/O, EAPoL orphan-tail cleanup, ttyd child readiness and WSDD2 XML UTF-8 validation remain committed.
- AliyunDrive HTTP listener readiness is committed; current fixture correction resets the termination mock before HTTP probes. Check its exact regression.
- AliDDNS uses a NUL-delimited configuration fingerprint including a separate digest of the persistent custom script. This commit fixes test root resolution and removes a redundant second hash.

## Active task

Two exact-head regressions need closure while independent work continues. AliyunDrive's host lifecycle regression passed on c32d2d3, but its source checker failed because `require()` was defined below its first call; this commit moves the helper above that call. AliDDNS's fingerprint test root and redundant digest were fixed in 3556d29; verify its exact SHA action as well.
## Verification boundary

- Ancestor SHA 129372718e1241823d785da6455abb46d03869c1: Process inventory #45043414729 PASS; AI handoff #45043414763 PASS; HTTPD auth #45043414724 PASS; AliDDNS #45043414686 FAILED at test path resolution; XY-C1 #45043414723 was pending then.
- AliyunDrive diagnostic #45042972255 on f8176758 FAILED with `marker=absent probes=0 sleeps=10`. Follow-up fixture changes on c32d2d3955b2f986494f070c1272d9fac4add50e address the missing mock process; exact run #45043595742 was still in progress at last inspection.
- The AliDDNS test-root correction and single-digest contract have not yet run. No local shell tests, production build, TRX/artifact or hardware checks were performed.
- L3 requires XY-C1 Build, TRX verification and artifact upload all PASS on one identical product-source SHA. L3/L4 remain NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect this commit's exact AliDDNS API transport run and fix only a demonstrated remaining failure.
2. Inspect AliyunDrive lifecycle #45043595742 after fixture isolation; use any diagnostic output to close the timeout regression without weakening bounded HTTP readiness.
3. Inspect current exact-SHA XY-C1 Build/TRX/artifact statuses, then continue an independent userland stability/security slice. Keep frozen backup read-only and work only on main.
