# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect live repository state and complete an end-of-session accuracy checkpoint. Record exact main SHA, exact-SHA Actions, test/build boundaries and the next precise task. If facts remain accurate, explicitly conclude with no file change and no no-op commit. Use one atomic main commit for material changes only. Before context exhaustion, leave an accurate handoff and complete any safe bounded engineering slice.

- Work only on `main`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update `main` only with `force:false` and a freshly fetched expected SHA.
- PENDING does not block independent work; no local/test/build claims without evidence.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; main parent SHA: 4b01a3e8638bb0fd94f7ca7f38d5556bb61a2c27.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807` remains immutable.
- AliyunDrive WebDAV start and duplicate-start now require both a validated process and bounded HTTP response; an initial checker/template typo was fixed on the current parent.
- p910nd I/O, EAPoL syntax, ttyd and WSDD2 slices remain committed; see dev_log.md for their exact verification boundaries.

## Active task

Diagnose the AliyunDrive WebDAV no-listener timeout regression. Exact run #45042609844 on parent 4b01a3e failed at the generic final check after an attempted start with a running PID and no HTTP listener. A new test revision emits marker presence, HTTP probe-line count and sleep count, so the next exact run can distinguish harness timing from production behavior.

## Verification boundary

- Parent #45042609844: AliyunDrive lifecycle FAILED at the HTTP timeout assertion. The earlier `*}` shell expansion error was fixed in the parallel parent, but the timeout case is still unresolved.
- No same-SHA XY-C1 evidence for the new AliyunDrive policy is recorded here. Require Build, TRX verification and artifact upload all PASS on one identical product SHA for L3.
- No local test/build was performed.

## Exact first action for the next GPT

1. Inspect the diagnostic AliyunDrive lifecycle rerun. Use actual marker/probe/sleep values to correct the source or test (not weaken the contract).
2. Inspect exact-SHA Process inventory, AI and XY-C1 Build/TRX/artifact results for the current source tip.
3. Continue an independent stability/security slice if AliyunDrive Actions are pending. Keep backup immutable and work only on main.
