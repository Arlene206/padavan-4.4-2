# 2026-11-18 continuation checkpoint — Shadowsocks detached daemon readiness

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; development remains on `main`. The pre-change main SHA for this atomic source/test milestone is `d2c47b7c9629950ff235659c14d08ba92a5a760e` (`test(shadowsocks): include startup readiness in bounded wait audit`).
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` is preserved; its DO-NOT-TOUCH marker was read. The previously recorded backup SHA is `72f4f157a084f8184b9597ca5bf334b8080e5807`; never modify/delete it. PR #1 is closed and no open PR was found.
- The last historically recorded XY-C1 L3 PASS is `a0cb9aad4844f7d044ce79efad6c417857c95972`; it does not validate subsequent Shadowsocks source changes. Physical-router L4 remains NOT VERIFIED.

## Completed engineering on this slice
- `shadowsocks.sh::run_bin()` previously returned immediately after scheduling a background subshell. Callers could log success even when the proxy daemon exited before becoming available. Multi-thread launches also needed to distinguish a new worker from an already-running sibling.
- The shared lifecycle helper now validates PID lists, counts instances, and waits with a bounded tri-state policy for the process count to increase by one. The launcher uses `exec` so the daemon replaces the wrapper.
- TCP, UDP (including the formerly unchecked Trojan UDP launch) and local SOCKS startup propagate launch/readiness failures. The Lua generator's actual long-lived child is `ipt2socks`, so its readiness check tracks that child rather than the short-lived Lua interpreter. The `ss-monitor` SSR-server and V2Ray/XRay/Trojan replacement paths use the same postcondition and log failed relaunch attempts.
- The regression extracts the production launcher and exercises an existing sibling plus new child, the Lua-spawned daemon, immediate child exit, and UNKNOWN process inventory. Python source contracts cover count helper, bounded wait and caller integration.

## Validation boundary
- No local shell test, host compile, MIPS/uClibc target build, firmware/TRX inspection or physical-router test was performed in this connector-only session.
- The available combined-status lookup returned no status contexts for the preceding head, and the commit-workflow-runs connector only exposes pull-request-triggered runs. Exact-head `push` Actions are therefore NOT VERIFIED; do not infer PASS from the main ref update. Inspect GitHub Actions for the new live-main SHA, especially Shadowsocks lifecycle, Process inventory/umbrella, AI handoff documentation, and XY-C1 if triggered. For XY-C1, inspect Build XY-C1, TRX verification and artifact upload separately before claiming L3.

## Exact first action for the next GPT
1. Fetch live `main` and inspect exact-head push Actions for this detached-launch change. Repair failed regression checks directly on `main`; verify XY-C1 Build, TRX and artifact state on that same SHA if the workflow was triggered.
2. If checks pass, continue the Shadowsocks audit at `start_chinadns()` and `start_watchcat()`: both still launch long-lived children without readiness verification, and dnsmasq is restarted asynchronously. Add bounded tri-state readiness and failure rollback before DNS publication/success reporting, with fault-injection regressions.
3. Continue broader RC/userland P1 reliability work. Do not repeat the completed cgroup, DNS-proxy or `run_bin()` audits without new evidence.

## Mandatory handoff checkpoint
Every session MUST compare live `main`, exact-head Actions, source baseline, validation boundary, blockers and first next action before ending. Update this file only when those facts materially change; do not create a no-op SHA-refresh commit.

## Cross-session execution mandate
Complete each logical source/test slice in one atomic `main` commit, then inspect exact-SHA Actions. Continue independent work while CI is pending. Never force-push, alter the frozen backup, reopen PR #1, or claim unperformed checks.
