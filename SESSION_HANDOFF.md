# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers, frozen backup and precise first action for the next session. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, explicitly conclude the check and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, do not wait for pending CI when independent work can proceed, and do not ask the user what to do next.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main, followed by a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, use a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Parent main before THIS COMMIT: `11e166fc85af30acb12165a198943b0be04f5117`. It contains the manual PPPoE physical-WAN status fix `4e1d18dd4d95ab16163a2373d70a674e00990db3` and the repaired WANX checker/handoff contract in `11e166fc85af30acb12165a198943b0be04f5117`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable and read-only.
- On parent `11e166fc85af30acb12165a198943b0be04f5117`, Process `43930702272` PASS and AI docs `43930702155` PASS; XY-C1 `43930702412` remained IN_PROGRESS at last poll. Previous completed parent `59681e0b5edf6803a47280c0c0256f596d217392` had Process `43927478287`, AI docs `43927478510`, USB modem identity `43927478496`, and XY-C1 `43927478225` all PASS, including Build/TRX/artifact.
- On parent `4e1d18dd4d95ab16163a2373d70a674e00990db3`, Process `43928812921` and XY-C1 `43928812622` failed at the new double-escaped WANX regex; the policy binary passed. AI docs `43928812779` failed because the shortened handoff dropped its mandatory checkpoint contract. Both issues were fixed in `11e166f`.
- PR #1 is historical closed/unmerged work, superseded by mainline port `74b66ff960457379518d13c723c44a3515e6a228`; do not resume that PR head.

## Active task

THIS COMMIT fixes the generic firewall-refresh UPnP status propagation gap. `restart_firewall()` previously called int-returning `update_upnp()` then discarded its result behind a `void` API. It now returns/logs the refresh status. Four RC notification paths merge it into `action_result` so a failed refresh keeps the incomplete marker; the direct CLI path returns it; SSH/HTTPD restart paths return it after preserving earlier start/postcondition errors. The existing RC notification policy tests normalization of positive EBUSY/EIO failures; the semantic WAN checker now binds the production return to every consumer. Static-IPoE and PPPoE-MAN startup fixes remain present.

## Verification boundary

- Source-level and C policy checks for THIS COMMIT run in existing Process and XY-C1 workflows. The checker verifies all four RC notification consumers, CLI, SSH and HTTPD status paths; existing RC notification policy tests positive errno normalization.
- A full local repository checkout/build is unavailable in this session; no local full C test is claimed. Previous GNU89/Werror/ASan/UBSan smoke coverage applied only to first-error aggregation helper semantics.
- Exact-head Process and AI docs must pass. L3 requires Build XY-C1, TRX verification and artifact upload together on THIS COMMIT's exact SHA. Physical-router L4 remains NOT VERIFIED.
- Frozen backup remains unchanged.

## Exact first action for the next GPT

1. Inspect exact-head Process, XY-C1 and AI docs runs for THIS COMMIT. Confirm the checker and notification policy tests, then confirm same-SHA Build/TRX/artifact separately. Diagnose failures from the first causal step.
2. Continue the P1 RC status audit around `reload_nat_modules()` and `start_firewall_ex()`: trace enabled module load/unload results and callers; distinguish required configured services from optional work before aggregating.
3. Keep the main-only atomic commit/CAS workflow, leave the frozen backup read-only, and keep physical-router L4 NOT VERIFIED.

