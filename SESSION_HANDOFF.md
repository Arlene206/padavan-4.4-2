# 2026-11-18 continuation checkpoint — ChinaDNS-ng startup readiness

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; active branch `main`. This ChinaDNS-ng source/test slice is based on live parent `e412a50bc2384b139161bda9a8cbdc9d57b7caa1`; refresh the branch tip before any further write.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`. PR #1 remains closed/unmerged.
- Shadowsocks ABSENT-under-errexit helper source fix is `cfbf00f9d9100daf82017403e63342658bbac29e`; XY-C1 run `44283616981` on that exact product source is SUCCESS, with Build XY-C1, TRX verification and artifact upload all SUCCESS.
- Exact-head checker-boundary correction `e412a50bc2384b139161bda9a8cbdc9d57b7caa1`: Shadowsocks lifecycle `44285579546` PASS and AI handoff docs `44285579386` PASS. The shell/process fixes and checker corrections are green on the exact e412 SHA; its source tree is the same firmware product source as cfbf except checker/docs.
 
## Active task
- Core Shadowsocks TCP/UDP/local, watchcat and DNS proxy launch paths now use bounded RUNNING/ABSENT/UNKNOWN process postconditions. The ABSENT pidof/pgrep status capture is safe under caller `set -e`.
- This slice adds ChinaDNS-ng process identity preflight and bounded launch readiness. Empty upstream DNS is rejected, UNKNOWN PID inventory fails closed, an existing daemon is not duplicated, and `chinadnsng_enable_flag` is set only after the new or existing daemon is confirmed. Router and GFW callers preserve the startup failure instead of masking it with the later DHCP restart.
- Host regression covers empty upstream, fresh detached child, duplicate prevention, immediate child exit and UNKNOWN inventory; static checker verifies the executable and process identity and both caller branches.

## Verification boundary
- The ChinaDNS-ng slice in this commit has not yet run exact-SHA Actions. Verify shell syntax, process lifecycle runtime, source checker, Process inventory and AI handoff after push.
- Existing dnsmasq configuration mutation and restart are still unchecked/non-transactional legacy operations after ChinaDNS readiness; the next independent source slice is atomic dnsmasq config publication and verified stop/restart rollback.
- XY-C1 product build evidence is successful on cfbf SHA `cfbf00f9...`; this commit changes ROMFS product source and therefore must have its own exact-SHA Build XY-C1, TRX and artifact pass before L3 can be claimed. No physical-router L4 is claimed.

## Exact first action for the next GPT
1. Fetch live `main` and inspect this ChinaDNS commit's exact-head Shadowsocks, Process inventory, AI docs and XY-C1 checks; fix the first causal failure directly on main.
2. Verify Build XY-C1, Verify TRX output and artifact upload all succeeded on the same source SHA.
3. Implement atomic dnsmasq config staging/publication and fail-closed dnsmasq stop/start readiness with rollback; add injected tests before changing behavior.
4. Continue broader RC/userland P1 reliability work while unrelated Actions are pending.

## Mandatory handoff checkpoint
Every session MUST inspect the live repository state and compare it with `SESSION_HANDOFF.md` before ending, verifying the active engineering baseline, exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all facts remain accurate, make no file change and do not create a no-op commit.

# Cross-session execution mandate
Complete each logical source/test slice in one atomic main commit. Continue independent work while CI is PENDING. Before context exhaustion, leave a precise next action and tested/unverified boundaries. Never force-push, mutate the frozen backup, reopen PR #1 or claim unperformed tests/builds.
