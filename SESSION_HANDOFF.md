# Cross-session execution mandate

## Workflow
- Develop on `main` only. One logical slice, one atomic main commit: create all blobs, one tree and one commit, then use a non-forced expected-SHA compare-and-swap update. Never force-push, amend, create branches/PRs, or edit the immutable backup.
- Preserve security, architecture, compatibility and required tests. Target GNU89, MIPS/uClibc, BusyBox and Linux 4.4. Do not weaken tests.
- Exact-head push Actions only validate the matching main SHA. PENDING is not a gate on independent work; inspect a relevant FAIL, fix its cause, and move forward.
- Before context exhaustion, finish the safe slice, publish the commit, inspect exact-SHA Actions and leave a truthful next action.
- Keep root `dev_log.md` at 20–119 newest-first entries; preserve `docs/dev-log/2026/11/part-021.md` and its index unchanged. Do not create handoff/roadmap snapshot archives.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect the live repository state before ending, verify this handoff's source baseline, active task, exact-head Actions, validation boundary, blockers and first next action. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no no-op commit. Update this handoff only when next-session-critical facts materially change.

## Current repository state
- Live main before this atomic checker repair: `60ad278e7056ace912066b090fdf20d0fd874f3e`; this commit is a checker/documentation child and does not change product source.
- The iNIC guest-link-state production fix on `60ad278e7056ace912066b090fdf20d0fd874f3e` uses `wireless_guest_interface_state_checked()` and defers recovery when the state is UNKNOWN. No production code change is included in this follow-up.
- Exact-head Wireless radio state #44897419595 failed in the embedded contract at the guest-state ordering assertion; exact-head iNIC signal lifecycle #44897419703 failed at the equivalent assertion in `scripts/check_inicd_signals.py:181`. Both failures incorrectly matched the earlier force-disable `if (status != 0)` rather than the later probe error branch. The C policy tests had passed in both workflows.
- This commit scopes both assertions from the checked guest-state call and preserves the required UNKNOWN-state return-before-recovery behavior.
- On `60ad278e7056ace912066b090fdf20d0fd874f3e`, AI handoff docs #44897419640, Process umbrella #44897419609, Process inventory #44897419663, watchdog #44897419667, HTTPD #44897419727 and NVRAM #44897419737 passed. XY-C1 #44897419694 was IN_PROGRESS at the last check; its Build/TRX/artifact steps must be inspected before any L3 claim.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, immutable. Legacy PR #1 is closed and unmerged; it is not a development target.

## Active task
- Verify the new exact-head iNIC signal, Wireless radio state and AI handoff documentation push Actions.
- Triage any first causal failure from its job log; do not relax the UNKNOWN-state fail-closed contract.
- Inspect XY-C1 #44897419694 for Build, TRX verification and artifact upload. Its source SHA is `60ad278e7056ace912066b090fdf20d0fd874f3e`; checker/docs-only changes in this commit do not alter that firmware input.
- Once this radio slice is validated, rotate to the next unfinished P1 reliability issue outside the repeatedly worked HTTPD/WAN/SkipDB areas.

## Verification boundary
- No local checkout/runtime was available. No local tests, sanitizer run or local firmware build are claimed for this checker-only repair.
- Exact-head remote C policy regressions passed on the parent SHA before the source-contract assertions failed; the corrected contracts require new exact-head push runs.
- L3: pending inspection of XY-C1 #44897419694 Build + TRX + artifact on production-source SHA `60ad278e7056ace912066b090fdf20d0fd874f3e`. Hardware/router L4: NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch live main and inspect the iNIC signal, Wireless radio state and AI handoff push runs whose `head_sha` exactly matches it.
2. Read the first causal failure log if any check is red; repair the specific contract/source defect in one atomic main commit.
3. Inspect XY-C1 run #44897419694 job steps for Build XY-C1, TRX verification and artifact upload; only claim L3 if all three completed successfully for `60ad278e7056ace912066b090fdf20d0fd874f3e`.
4. Continue with the next distinct P1 stability/security issue from the roadmap; do not repeat the already completed radio probe review.

Before context exhaustion, update this handoff only when active task, exact-head status or validation boundary changes; no no-op commit.
