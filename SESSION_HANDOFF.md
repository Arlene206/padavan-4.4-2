# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers, frozen backup and precise first action for the next session. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, explicitly conclude the check and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, do not wait for pending CI when independent work can proceed, and do not ask the user what to do next.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main, followed by a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, use a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Main parent for the strict module-probe slice: `1aca5ab3a776a66138814769b84d5e5ceb43b46c`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`, unchanged/read-only.
- Recent source changes verify configured NAT helper loads, HW NAT WAN-VID configuration and SFE sysfs configuration. A fail-open edge remains in the legacy module probe: `is_module_loaded()` returns 0 for both ENOENT and other `opendir("/sys/module/<name>")` errors. This slice adds a tri-state probe, preserves the legacy boolean API, and makes verified unload propagate probe errors.
- Exact-head 1aca Actions: Process lifecycle regression `43952107815`, HTTPD `43952107789`, and AI handoff `43952107818` passed. XY-C1 `43952107870` status is recorded below; these runs do not include this new strict-probe commit.
- ec923 previously passed XY-C1 module-policy regression, firmware build, TRX and artifact upload. That build predates later HW NAT/SFE source changes and must not be treated as current-source L3 evidence.

## Active task

Harden NAT module unload postconditions. The previous verifier treated any zero-valued presence probe as "absent", while `is_module_loaded()` conflates ENOENT and sysfs permission/resource failures. The strict probe returns 1 for loaded, 0 only for a missing module after confirming the sysfs module root is readable, and negative errno for lookup failures. Existing boolean callers keep their legacy API.

Injectable policy regressions prove an initial probe failure prevents unload and a post-unload probe failure is returned rather than reported as success. Configured helper unloads must still be attempted independently, retaining the first failure.

## Verification boundary

- Exact-head 1aca Process lifecycle regression `43952107815`, HTTPD `43952107789`, and AI handoff `43952107818` passed. XY-C1 `43952107870` was `in_progress` at the last poll.
- Earlier ec923 XY-C1 run `43948787188` passed the module policy test, firmware build, TRX verification and artifact upload. It predates subsequent HW NAT/SFE source modifications and is not current-source L3 evidence.
- The strict module-probe source changes and two injected failure cases are in this commit; no new-SHA CI or firmware result is claimed until exact-head Actions complete. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect new-SHA Process, Firewall Startup, HTTPD and XY-C1 Actions. Require the strict-probe regressions to pass and inspect Build/TRX/artifact before claiming firmware validation.
2. Continue the cross-userland RC startup-result audit: trace enabled service launchers through actual callers and PID 1 action-result merges; fix only demonstrated dropped errors.
3. Keep main-only complete-tree commits with non-forced expected-SHA CAS. Frozen backup read-only; physical-router L4 NOT VERIFIED.
