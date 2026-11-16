# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers, frozen backup and precise first action for the next session. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, explicitly conclude the check and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, do not wait for pending CI when independent work can proceed, and do not ask the user what to do next.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main, followed by a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, use a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Current production-source baseline on `main`: `eb566f346f63d3d6ed295c08170ac6d834ff7f04`. The follow-on handoff/documentation commit changes only these project-status documents; no production source changes are pending in that documentation slice.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, unchanged and read-only.
- `0b32b9111c38e2cf7588cf6605b2491f25e3ce31` fixed HW NAT 2.4/5 GHz radio-registration and IPv6 debugfs configuration: command wait status is normalized, each independent write is attempted, and the first failure is propagated. The HTTPD whitespace-sensitive source assertion was corrected in `e928181da3a52b4035d1ccf03307f212cafa2a4e`.
- `58318f53937622672e10d6bcea8b07f8a4753aa3` fixed WAN restart route cleanup: interface names are passed to `ip` as argv elements; both interface flushes are attempted and a failure aborts route/table/WAN reconfiguration. Host regression `wan_route_flush_policy.c` covers first-error retention and independent cleanup.
- `f2d9ccef50d3ebfc0f6f5dde94fae7893c7bb58d` corrected missing pull-request path-filter entries for the new policy/test. Its HTTPD source assertion was then found to be whitespace-sensitive and fixed in `eb566f346f63d3d6ed295c08170ac6d834ff7f04`.

## Active task

Verify the exact-current-source XY-C1 build for `eb566f346f63d3d6ed295c08170ac6d834ff7f04`. This is the first firmware build that includes the WAN interface-route flush change. Do not infer its result from the earlier firmware success on `e928181da3a52b4035d1ccf03307f212cafa2a4e`.

## Verification boundary

- Exact-head `eb566f3` HTTPD run `43961727463` passed, including the switch/route source contract and the compiled WAN route-flush regression. AI handoff contract run `43961726579` passed before this documentation update.
- Exact-head `eb566f3` XY-C1 run `43961726533`: all pre-build host/source checks passed, including `Compile and run WAN route flush failure regression`; `Build XY-C1` was still in progress at handoff. TRX verification and artifact upload had not yet run.
- Exact-head `f2d9cce` Firewall Startup run `43961249138` passed. Its HTTPD run `43961249141` failed only at the whitespace-sensitive source assertion for the multiline `rc_wan_route_flush_apply(...)` call; this is corrected in `eb566f3`, and the current-head HTTPD rerun is green.
- The same route regression passed in Process lifecycle run `43960589874` on source SHA `58318f5`; the WAN route-operation regression `43960590043` also passed. The earlier `e928181` XY-C1 run `43957697560` completed Build XY-C1, TRX verification and artifact upload successfully, but predates the WAN route cleanup source change.
- The initial HW NAT implementation SHA `0b32b91` had one HTTPD checker failure, fixed on `e928181`; the Firewall, Process and XY-C1 checks on `0b32b91` passed.
- Physical-router L4 remains **NOT VERIFIED**. No claim of live radio registration, route behavior, or router stability is made from host CI alone.

## Exact first action for the next GPT

1. Poll exact-SHA XY-C1 run `43961726533` for `eb566f346f63d3d6ed295c08170ac6d834ff7f04`. Inspect `Build XY-C1`, `Verify TRX output`, and artifact upload separately; fix any failure before marking this source L3 verified.
2. Recheck main and the frozen backup before the next write. Continue source-level audits of remaining dropped network/RC command results only where the operation is required for a correct state transition; add fault-injection regression and exact-SHA Actions for each confirmed defect.
3. Keep main-only atomic full-tree commits with expected-SHA non-forced compare-and-swap. Never force-push, create a work branch/PR, or modify the frozen backup. Physical-router L4 remains NOT VERIFIED.
