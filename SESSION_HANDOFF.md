# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume concrete engineering tasks. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning. Before context exhaustion, complete the safe slice and preserve its exact source/CI boundary.

- Create all changed blobs, one complete tree and one commit with the current main parent; update main with non-forced compare-and-swap using that exact SHA.
- Verify Actions by exact `head_sha`. A pending run does not block independent work; a failed relevant run must be triaged before claiming validation.
- Preserve security, architecture, GNU89/MIPS/uClibc compatibility, regression coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` is archival only; never alter or delete it. Develop on main.

## Current repository state

- Current parent for the shared service-lock retry slice: `a9f3c2030c917043642aad02574da2ada180b473` (`docs(ai): restore cross-session checkpoint rule`).
- MiniUPnP bounded EINTR source commit `e736dbfcff830e6ac6baa32026fff7a6334787d1`: Process lifecycle `44628661363`, MiniUPnP config `44628661018`, Process inventory `44628661358`, NVRAM audit `44628661528`, and XY-C1 `44628661156` all PASS. XY-C1 completed Build XY-C1, TRX verification and artifact upload on the exact e736 source SHA.
- AI handoff documentation run `44628661127` failed on e736 because the mandatory literal phrase `Before context exhaustion` was absent; docs-only follow-up `a9f3c20` restored it and AI docs run `44629051861` PASS.
- Newly confirmed cross-service issue: Inadyn, DNSMasq and VPN-server lifecycle lock releases also had unbounded `EINTR` loops. This slice standardizes their retries with MiniUPnP under one shared helper.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; archival only. PR #1 remains closed/unmerged.
- No physical-router result is available. Exact e736 Build/TRX/artifact is L3 for that SHA; device behavior is L4 NOT VERIFIED.

## Active task

- Confirmed unbounded `EINTR` retries in `inadyn_lifecycle_lock_release()`, `dnsmasq_lifecycle_lock_release()` and `vpn_server_lifecycle_lock_release()`; MiniUPnP had just acquired a dedicated eight-retry guard.
- Added shared `lifecycle_lock_retry_policy.h` with an eight-retry ceiling and common errno behavior. MiniUPnP acquisition/release and the other three service release functions now use it; Inadyn/DNSMasq/VPN acquisition contention behavior remains unchanged.
- The GNU89 MiniUPnP lock regression covers retry success, exhaustion (`EINTR`) and invalid arguments. Its semantic checker now pins helper use across all four daemon lock-release paths. Existing per-service workflows compile the corresponding lock tests, and XY-C1 filters include the common production helper.
- No local compilation or router L4 result is claimed for this new shared-helper slice.

## Exact first action for the next GPT

1. Fetch live `main` and inspect exact-head Process lifecycle, Inadyn lifecycle, DNSMasq lifecycle, VPN-server stability, MiniUPnP config, AI-doc and XY-C1 runs for the new shared-helper commit.
2. Fix real assertion/build failures from their exact job logs without weakening the eight-retry bound or lock identity checks. Confirm XY-C1 Build, TRX verification and artifact upload on the same source SHA.
3. Continue an independently evidenced cross-userland stability/security issue after validation. Do not reopen the already-fixed PPP, APCLI or SD/MMC notification paths without new evidence.

## Verification boundary

- L1: MiniUPnP, Inadyn, DNSMasq and VPN-server lock release paths now share an eight-retry EINTR limit; MiniUPnP acquisition uses the same policy.
- L2: GNU89 retry-policy regression and four-daemon semantic source contract are extended; exact-head per-service workflow outcomes remain pending.
- L3: Previous MiniUPnP source SHA e736 passed XY-C1 Build/TRX/artifact. The shared-helper production-source change still needs its own exact-head Build/TRX/artifact run.
- L4: target MIPS/uClibc behavior under repeated signal interruption and physical-router service transitions remain NOT VERIFIED.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST perform a handoff checkpoint before ending. At session end, compare live main, product-source baseline, exact-head Actions, module state and this first action. Update this file only when resumable facts materially change; do not create no-op SHA-refresh commits.

## Development invariants

- Main-only; never modify the frozen backup or develop on a legacy branch/PR.
- One complete Git tree and one atomic commit per logical task; `force=false`, exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits or weaken regressions.
- Report exact SHA Actions and keep L1/L2/L3/L4 distinct. Host success is not firmware/hardware success.
