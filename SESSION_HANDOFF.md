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
- Parent main for this checker-only correction: `310e7c8668db0eda362a0400775d5670cf7a2242`; product-source baseline remains `60ad278e7056ace912066b090fdf20d0fd874f3e` (fail-closed iNIC guest-state probe).
- Exact push SHA `310e7c8668db0eda362a0400775d5670cf7a2242`: Wireless radio state #44897978738 PASS; AI handoff docs #44897978824 PASS; iNIC #44897978753 FAIL in `scripts/check_inicd_signals.py` after its GNU89/Werror/ASan/UBSan policy binary passed. The failure was an incorrect fixture path: the `wireless_guest_isolation_link_state(-1, EACCES, ENODEV,` invocation is in `tests/wireless_guest_isolation_policy.c`, not in the policy header.
- This atomic follow-up changes only that checker assertion to read the actual test source. The prior ordering correction remains in place.
- Exact-head on preceding source SHA `60ad278e7056ace912066b090fdf20d0fd874f3e`: AI docs #44897419640, Process umbrella #44897419609, Process inventory #44897419663, watchdog #44897419667, HTTPD #44897419727, NVRAM #44897419737 PASS; Wireless #44897419595 and iNIC #44897419703 failed on the stale ordering assertion corrected in the intermediate checker commit. XY-C1 #44897419694 for the same production source is still in progress; Build was running, TRX and artifact pending.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, immutable. Legacy PR #1 remains closed/unmerged.

## Active task
- Verify exact-head iNIC signal and AI documentation Actions for this correction; Wireless passed on the parent and the embedded Wireless checker uses the correct test-file path.
- If iNIC fails again, inspect the first causal log and update only the mismatched contract; retain the UNKNOWN-state fail-closed regression.
- Inspect XY-C1 #44897419694 Build, TRX verification and artifact upload. L3 is allowed only if all three succeeded on production-source SHA `60ad278e7056ace912066b090fdf20d0fd874f3e`.
- After the iNIC slice closes, rotate to another distinct unfinished P1 reliability/security issue from the roadmap, avoiding repeat work in HTTPD/WAN/SkipDB.

## Verification boundary
- No local checkout/runtime; no local tests, sanitizers or firmware build are claimed for this checker-only follow-up.
- L2: C regression binaries passed on both preceding exact-head attempts, but the last iNIC source-contract check failed before this correction; exact-head rerun pending.
- L3: pending XY-C1 Build + TRX + artifact for the current production-source SHA. L4 hardware/router validation: NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch live main and inspect iNIC signal lifecycle and AI handoff push runs matching its exact SHA.
2. Triage the first causal failure, if any, and make a focused atomic main correction without weakening the regression.
3. Inspect Build/TRX/artifact steps in XY-C1 #44897419694 before making any L3 claim.
4. Continue with a distinct P1 stability/security issue once the radio slice is closed.

Before context exhaustion, update this handoff only when active task, exact-head status or validation boundary changes; no no-op commit.
