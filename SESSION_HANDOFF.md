# 2026-11-18 continuation checkpoint — PPPoE-MWAN probe regression verified

## Repository state
- Repository: `Arlene206/padavan-4.4-2`; work on `main` only.
- Parent main for THIS documentation/test-contract commit: `ffa7f6d07522d8a51b53a9c1fede22c42d3588b2`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`. PR #1 is closed and unmerged; do not reopen it.
- The development-log archive rotation is complete: `part-016.md` stores 100 records oldest-first; the exact-head archive contract on `808cff7a22b147e928a6fae47142e3016ade685a` passed AI docs run `44262705464`.

## PPPoE-MWAN probe fix completed
- Source: `trunk/user/pppoe/detect.sh`. `mwan_get_ppp_ipv4()` treats a missing `/sys/class/net/pppN` entry as normal during WAN reconvergence, but preserves the exact nonzero `ifconfig` status when the sysfs interface still exists. If the device disappears between the sysfs check and ifconfig, the second sysfs check prevents reporting a false error.
- The detector continues independent peer reconciliation after an existing-interface probe fails and keeps the first failure in `MWAN_STATUS`. Both policy-building loops use the shared probe helper.
- Regression in `trunk/user/pppoe/tests/detect_status.sh`: existing ppp0 failure status 44 must be retained while ppp1 still contributes a route; missing ppp1 sysfs entry skips ifconfig and remains status 0.
- Exact-head source/test/checker validation on `ba893707c6f5cb939980cf4eecb5cd2d5dd14871`: WAN route `44263552770` PASS; Process lifecycle inventory `44263552790` PASS; Process inventory fail-closed `44263552757` PASS. XY-C1 `44263552808` was IN_PROGRESS at last poll (firmware Build active, TRX and artifact pending). Do not claim L3 for this SHA until Build, TRX and upload all pass on the same SHA.
- The prior failed WAN run `44263088331` on `66dbe404` was caused by malformed test tail at line 123 before the detector regression executed. Commit `4bc9d87c` repaired the test structure; `ba893707` then corrected the ifconfig status capture to happen inside the failing `else`. The exact-head WAN PASS validates the repaired shell contract.
- Current main before THIS commit is descendant `ffa7f6d07522d8a51b53a9c1fede22c42d3588b2`, which also contains deferred WAN watchdog error propagation in `ffa7f6d07522d8a51b53a9c1fede22c42d3588b2`. On that exact SHA, WAN route run `44264230949`, Process inventory `44264231061`, and several independent lifecycle jobs were still running; XY-C1 run `44264231022` was PENDING. Recheck latest Actions after this commit.
- Historical product-source SHA `a0cb9aad4844f7d044ce79efad6c417857c95972` has L3 VERIFIED from XY-C1 run `44260005493` (Build, TRX, upload). This verification does not transfer automatically to newer source SHAs.
- No physical-router test is claimed. L4 remains NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch live `main` and inspect exact-head AI docs, WAN route and XY-C1 results for THIS commit. For XY-C1, inspect Build, TRX verification and artifact upload; do not infer success from the top-level workflow alone.
2. Continue the next P1 userland reliability slice: audit required-stage error propagation in `trunk/user/shadowsocks/scripts/shadowsocks.sh` (`ssp_start()` and its start/restart entry points). Keep optional/disabled service behavior intact; do not report success if required proxy, DNS, local-proxy, firewall-policy or PPPoE detector stages fail. First add a focused host regression and exact source contract, then modify behavior.
3. Pending independent workflows must not block source work. Use one complete tree and one atomic commit per logical source/test slice; never force push or modify the frozen backup.
4. Preserve the physical-router L4 status as NOT VERIFIED.

## Mandatory handoff checkpoint
At session end, fetch live main, exact-head Actions and the next task. Update this file only when the facts or first task materially change; no no-op documentation commits.

# Cross-session execution mandate
Continue source fixes, regressions, atomic commits and exact-SHA Actions. Never force-push, mutate the backup, reopen the retired PR, or claim unperformed tests/build/hardware checks.
