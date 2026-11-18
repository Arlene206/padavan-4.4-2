# 2026-11-18 continuation checkpoint — Shadowsocks watchcat startup readiness

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; active branch `main`. The parent source/test correction is `361d4b33ef8801d94cd13d949f693974b46af1d3` (`test(shadowsocks): yield to detached daemon fixtures`); the next atomic slice adds watchcat readiness and coverage.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at recorded SHA `72f4f157a084f8184b9597ca5bf334b8080e5807`. PR #1 is closed; do not reopen it.
- Historical XY-C1 L3 PASS applies only to `a0cb9aad4844f7d044ce79efad6c417857c95972`; physical-router L4 remains NOT VERIFIED.

## Active task
- Core `run_bin()` readiness and bounded count+1 process launch checks cover Shadowsocks TCP, UDP (including Trojan UDP), local SOCKS, and ss-monitor replacement launches. The Lua generator is checked via its actual `ipt2socks` child, not the short-lived Lua wrapper.
- `start_watchcat()` previously launched `/usr/bin/ss-monitor` without confirming it survived. This slice adds validated `pgrep -f` count+1 readiness, duplicate prevention, fail-closed UNKNOWN handling, a bounded timeout, and caller-visible scheduling/readiness errors.
- Parent `361d4b33ef8801d94cd13d949f693974b46af1d3` exact-head checks: Process inventory `44281846459` PASS; full inventory `44281846403` PASS; AI docs `44281846410` FAIL because the exact `Mandatory handoff checkpoint` heading was missing. This section restores that required contract text.
- XY-C1 was not triggered for the test/docs-only parent; the last historically recorded product L3 remains the older `a0cb9aad...` SHA. Do not infer current firmware validation from it.

## Verification boundary
- This source/test slice has not been locally executed in the connector-only environment. No shell test, host compile, MIPS/uClibc build, TRX validation, artifact upload or router runtime is claimed.
- Exact-head Shadowsocks lifecycle, Process inventory, AI handoff docs and any triggered XY-C1 product checks must be inspected after publishing this slice. For any L3 claim, Build XY-C1, TRX verification and artifact upload must all pass on the same exact SHA.

## Exact first action for the next GPT
1. Fetch live `main` and inspect exact-head Actions for this watchcat commit. Verify the Shadowsocks process lifecycle suite, Process inventory suite and AI handoff documentation contract; repair any FAIL directly on `main`.
2. Inspect XY-C1 Build/TRX/artifact statuses for the current source SHA if the firmware-input detector triggered it.
3. Continue the remaining Shadowsocks startup queue with `start_chinadns()` and dnsmasq restart readiness/rollback, then rotate to broader RC/userland P1 stability work.

## Mandatory handoff checkpoint
Every session MUST inspect live repository state, exact-head Action results, validation boundary, blockers and the precise first next action before ending. The checkpoint is mandatory; a document edit is conditional. If those facts are already accurate, explicitly conclude the checkpoint with no file change and no no-op commit.

# Cross-session execution mandate
Complete each logical slice in one atomic main commit and inspect its exact-SHA Actions. Continue independent work while CI is pending. Never force-push, alter the frozen backup, reopen PR #1, or claim unperformed tests/builds. Before context exhaustion, leave the next GPT a precise first action and verified/unverified boundaries.
