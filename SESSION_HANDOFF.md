# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume the concrete engineering task. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning. Before context exhaustion, complete the safe slice and preserve its exact source/CI boundary.

- One atomic main commit per logical task: create all blobs, one complete tree and one commit with the current main parent, then update main using a non-forced compare-and-swap with that exact SHA.
- Verify push-triggered Actions by exact head_sha; a pending run does not block independent work, while a failed relevant run must be triaged before the slice is called validated.
- Preserve security, architecture, GNU89/MIPS/uClibc compatibility, regression coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- The frozen backup branch `backup/pre-plugin-overhaul-2026-11-14` is archival only and must never be modified or deleted. Development stays on main.

## Current repository state

- Parent main for THIS PPP recovery source fix: `52cd91c1c7f0d5c297b157a9ffb636844363f605`. It contains the SCUT EPERM/status-collision correction; exact-head Process umbrella `44622196808` and Process inventory `44622196753` PASS, AI docs `44622196858` PASS. NVRAM run `44622196798` was pending at checkpoint.
- SCUT boot crontab correction at `69430c238e26c0de0b9210a5d090512bb4570f50` establishes 0=unchanged, 1=appended, 2+=script error; `system()`-level errno is negative so `EPERM=1` cannot be confused with successful append. Crond checker run `44621067912` PASS at `69d64f4...`; Process contract run `44621537460` PASS and AI docs `44621537441` PASS at `c2fe476...`.
- XY-C1 run `44620322487` for the SCUT product source `69430c...` was still in progress on the last query. Its final Build, TRX verification and artifact upload must be checked before L3 can be called PASS.
- OpenVPN credential staging run `44613650285` PASS on `f4fbd093e934fda5304aa796cba30d59eb29b3a4`; VPN server lifecycle `44613173654` PASS and XY-C1 Build/TRX/artifact `44613173721` PASS on OpenVPN product source `e01f70a766c4e628593e272192723f6add59ffca`.
- Frozen backup remains `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify. PR #1 remains closed/unmerged. Root `dev_log.md` had 25 dated records before THIS COMMIT; the new record keeps it inside the 20–119 window.

## Active task

### Fail closed when PPP recovery cannot restore the physical WAN firewall

- In `trunk/user/rc/src/net_wan.c`, `restore_wan_after_ppp_start_failure()` restores physical-WAN UPnP binding and rebuilds firewall rules, returning a failure if either step fails. The existing `restore_wan_after_firewall_failure()` wrapper takes the physical interface down if restoration fails and preserves the original recovery error.
- Three normal PPP failure branches (PPTP/L2TP physical-interface setup, pppd launch, demand-PPP readiness) previously bypassed the fail-closed wrapper and called the lower-level helper directly. These paths could leave the physical WAN administratively up with incomplete UPnP/firewall recovery.
- THIS COMMIT routes all three branches through the wrapper and extends `check_wan_firewall_status.py` to assert those exact callers cannot bypass the force-down path. No successful PPP startup or recovery behavior changes.

## Exact first action for the next GPT

1. Query exact-head WAN route-status and XY-C1 Actions for THIS COMMIT. Inspect all step results and logs for failures; do not claim L3 until Build XY-C1, TRX verification and artifact upload pass on one exact product-source SHA.
2. If those runs pass, continue the next independent P1 stability issue in the wider Padavan userland. Do not repeat SCUT crontab/Crond/Process checker, OpenVPN, DNSMasq resolver, `create_file()` close-status, USB modem notifier, WAN monitor-pause or the PPP recovery call graph already closed by this commit.
3. Keep host L2, XY-C1 L3 and physical-router L4 separate. No physical-router WAN recovery validation is claimed.

## Verification boundary

- L1: all three ordinary PPP failure callers now use fail-closed physical-WAN recovery.
- L2: a source-semantic regression is added to the existing WAN firewall checker, already covered by the WAN route workflow. This session's local brace-aware caller assertions passed; exact-head WAN route Actions remain pending.
- L3: XY-C1 Build/TRX/artifact for the current SCUT product source `69430c...` was pending at last query; current PPP source change needs its own exact-head firmware validation.
- L4: forcing miniupnpd/firewall recovery failures on a physical XY-C1 and verifying the WAN interface remains down is NOT VERIFIED.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session must perform an end-of-session accuracy checkpoint. Compare live main, source baseline, exact-head Actions, module status and this first action. Update this file only when those resumable facts materially change; do not create a SHA-refresh or no-op commit.

## Development invariants

- Main-only; never modify the frozen backup or develop on a legacy branch/PR.
- One complete Git tree and one atomic commit per logical task; use `force=false` with the exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits, or weaken regression coverage.
- Report exact SHA Actions and separate L1/L2/L3/L4. Host success is not firmware/hardware success.
