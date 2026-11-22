# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state and complete an end-of-session accuracy checkpoint. Record exact main SHA, exact-SHA Actions, test/build boundaries, and the next precise task. If facts remain accurate, explicitly conclude with no file change and no no-op commit. Use one atomic main commit for material changes only. Before context exhaustion, leave an accurate handoff and complete any safe bounded engineering slice.

- Work only on `main`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update `main` only with `force:false` and a freshly fetched expected SHA.
- PASS continues; FAIL requires diagnosis/correction; PENDING does not block independent work.
- Do not claim local tests, firmware builds, TRX/artifacts or hardware checks that were not performed.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; parent main SHA: `47e8da3e5f3dd72c7f93872a5dadde52fa27cbcd`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807` remains immutable.
- p910nd transient I/O, EAPoL orphan-tail cleanup, ttyd child readiness, WSDD2 XML UTF-8 validation and AliyunDrive HTTP listener readiness are committed; see dev_log.md for exact source/CI boundaries.

## Active task

AliDDNS previously computed its keeper restart status by raw concatenation of NVRAM values, without field separators. Adjacent fields could collide, e.g. `example.com` + `ab` and `example.coma` + `b`, so configuration edits could be missed and stale DDNS settings left active. The current change uses a NUL-delimited fingerprint and a separate digest of the custom script; missing or symlinked script input fails closed.

## Verification boundary

- Parent main SHA recorded by this slice: `${head}`. New AliDDNS fingerprint shell regression and source-contract checks are committed below but have not run yet.
- AliyunDrive prior lifecycle #45043377399 on `47e8da3e5f3dd72c7f93872a5dadde52fa27cbcd` failed with `probes=1 sleeps=10`: `TEST_TERM_BEHAVIOR=after-one-poll` leaked from the disabled-stop test and made the PID disappear after one sleep. This commit resets the mock to `stuck` before both HTTP readiness fixtures; production source is unchanged.
- No local shell tests, production target build, TRX/artifact or hardware run was performed. L3 requires all XY-C1 Build/TRX/artifact checks PASS on one exact product-source SHA; L3 and L4 remain NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head AliyunDrive lifecycle and AI handoff results for this fixture-state repair; verify 11 HTTP-unavailable probes and 3 delayed-listener probes.
2. Inspect the in-progress parent-129372 Actions: AI handoff #45043414763, Process inventory #45043414729, HTTPD security #45043414724, XY-C1 #45043414723 and AliDDNS #45043414686; they are historical parent results, not evidence for this new SHA.
3. Continue an independent confirmed userland defect while Actions are pending. Keep the frozen backup read-only and work only on main.
