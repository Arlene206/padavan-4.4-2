# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect live repository state and complete an end-of-session accuracy checkpoint. Record exact main SHA, exact-SHA Actions, test/build boundaries and the next precise task. If facts remain accurate, explicitly conclude with no file change and no no-op commit. Use one atomic main commit for material changes only. Before context exhaustion, leave an accurate handoff and complete any safe bounded engineering slice.

- Work only on `main`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update `main` only with `force:false` and a freshly fetched expected SHA.
- PASS continues; FAIL requires diagnosis/correction; PENDING does not block independent work.
- Do not claim local tests, firmware builds, TRX/artifacts or hardware checks that were not performed.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; main at this checkpoint: `72ffcb355cff70e8dc7b54f708b3806e3eac7e66`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807` remains immutable.
- p910nd transient I/O, EAPoL orphan-tail cleanup, ttyd child readiness and WSDD2 XML UTF-8 validation remain committed.
- AliyunDrive HTTP listener readiness is committed. The c32d2d3 fixture now keeps the simulated PID alive during the bounded HTTP probes; its source-checker `require()` ordering fix is committed at 72ffcb3. Exact run #45043816997 is pending.
- AliDDNS NUL-delimited configuration fingerprint and shell regression are committed at 1293727/3556d29. The exact fingerprint regression passed on #45043730798, but the final Python contract failed at line 542 because three new assertions refer to undefined uppercase `SOURCE` rather than the initialized lowercase `source`; this commit corrects the checker.

## Active task

Correct the AliDDNS semantic checker variable typo, then verify its exact-head workflow. In parallel, inspect the AliyunDrive source-checker rerun on 72ffcb3; its prior host regression passed but the source checker failed because `require()` was defined below its first call, now fixed.
## Verification boundary

- Ancestor SHA 129372718e1241823d785da6455abb46d03869c1: Process inventory #45043414729 PASS; AI handoff #45043414763 PASS; HTTPD auth #45043414724 PASS; AliDDNS #45043414686 FAILED at test path resolution; XY-C1 #45043414723 was pending then.
- AliyunDrive diagnostic #45042972255 on f8176758 FAILED with `marker=absent probes=0 sleeps=10`. Follow-up fixture changes on c32d2d3955b2f986494f070c1272d9fac4add50e address the missing mock process; exact run #45043595742 was still in progress at last inspection.
- The AliDDNS test-root correction and single-digest contract have not yet run. No local shell tests, production build, TRX/artifact or hardware checks were performed.
- L3 requires XY-C1 Build, TRX verification and artifact upload all PASS on one identical product-source SHA. L3/L4 remain NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect the exact AliDDNS Actions rerun for this checker correction; all fingerprint shell assertions already passed, so only a demonstrated remaining checker failure should be changed.
2. Inspect AliyunDrive lifecycle #45043816997 on 72ffcb3 and diagnose its first failed step if any.
3. Recheck XY-C1 Build/TRX/artifact on exact product-source SHA, then continue the next independent stability/security defect from the roadmap. Keep frozen backup read-only and work only on main.
