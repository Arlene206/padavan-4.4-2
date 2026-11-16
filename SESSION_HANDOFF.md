# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers, frozen backup and precise first action for the next session. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, explicitly conclude the check and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, do not wait for pending CI when independent work can proceed, and do not ask the user what to do next.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main, followed by a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, use a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Parent main before THIS COMMIT: `4e1d18dd4d95ab16163a2373d70a674e00990db3`. It contains the manual PPPoE physical-WAN setup status fix in `4e1d18dd4d95ab16163a2373d70a674e00990db3`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; unchanged, immutable and read-only.
- Exact-head validation for parent `59681e0b5edf6803a47280c0c0256f596d217392`: Process `43927478287` PASS; AI docs `43927478510` PASS; USB modem identity `43927478496` PASS; XY-C1 `43927478225` PASS including Build XY-C1, TRX verification and artifact upload.
- Parent `4e1d18dd4d95ab16163a2373d70a674e00990db3` Actions: Process regression `43928812921` FAIL and XY-C1 `43928812622` FAIL because `scripts/check_modem_wan_start_status.py:209` used a double-escaped raw regex for the new WANX status assertion. The policy test itself printed PASS before the semantic checker failed. AI docs `43928812779` FAIL because the updated handoff removed the mandatory end-of-session checkpoint contract. Numerous independent parent-head suites passed.
- PR #1 is historical closed/unmerged work, superseded by mainline port `74b66ff960457379518d13c723c44a3515e6a228`; never resume its head.

## Active task

THIS COMMIT fixes both validation regressions found on parent `4e1d18dd4d95ab16163a2373d70a674e00990db3`. The WANX checker now uses proper single raw-regex whitespace escapes for the first-error aggregation added to configured PPPoE MAN setup. The full mandatory cross-session execution and accuracy-checkpoint preamble is restored in this handoff so the AI documentation contract can verify it. No production code is changed in THIS COMMIT.

## Verification boundary

- The static-IPoE UPnP result propagation landed in `1b06d3178c366ae764f894d6bb15847b8f5f6e21`; its checker regex was corrected in `59681e0b5edf6803a47280c0c0256f596d217392`. On `59681e0...`, exact-head Process, AI docs, USB modem and XY-C1 Build/TRX/artifact all passed.
- The manual PPPoE physical-start fix in parent `4e1d18d...` adds GNU89 policy coverage and preserves PPPD best-effort startup. On that parent, the WAN policy test ran and printed PASS, then the checker failed at its newly added WANX regex assertion; this is fixed in THIS COMMIT.
- The parent AI docs job failed specifically because the current handoff lacked the required "Mandatory handoff checkpoint" text and explicit conditional/no-op gate; both are restored above.
- Exact-head Process, XY-C1 and AI documentation checks for THIS COMMIT must pass. L3 is only verified if Build XY-C1, TRX output verification and artifact upload all pass on THIS COMMIT's exact SHA. Physical-router L4 remains NOT VERIFIED.
- No local full-tree checkout/test is available in this session; prior focused GNU89/Werror/ASan/UBSan first-error smoke passed. Do not claim that as a full product test.

## Exact first action for the next GPT

1. Inspect exact-`head_sha` Process, XY-C1 and AI docs runs for THIS COMMIT. Process must compile/run the modem WAN policy suite and execute the corrected checker; XY-C1 must complete Build, TRX verification and artifact upload on the same SHA. Diagnose any failed step from its logs.
2. Confirm configured PPPoE MAN failure is included in the returned WAN first-error status while PPPD is still attempted; confirm the exact source contract.
3. Continue the P1 RC startup-result audit, beginning with `restart_firewall()` / `update_upnp()`: verify all RC notification, CLI, SSH and HTTPD callers preserve UPnP refresh errors before implementing the next atomic slice.
4. Keep the frozen backup read-only and physical-router L4 marked NOT VERIFIED.

