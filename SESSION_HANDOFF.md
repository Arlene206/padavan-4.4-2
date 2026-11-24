## Current repository state

- Live branch: `main` at `5ac14cdde2a18cf9edb8cfa3f116b42d11807b10`. RC firewall production fix `7ca46fc7049f3380755f6b2fe5073d86727c6382` is followed by firewall checker updates `03ca0dbf`, `77dc29c`, `1c4cdd6` and `65c6051`. RAETH Clause 45 bounds `a76f771`, Clause 22 bounds `7efbd10`, checker correction `2f95d1c`, and shared Clause 45 entry-point validation `a08b86e` are also present.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. Legacy PR #1 remains closed and unmerged.
- Each logical task is one atomic main commit built from all blobs + one Git tree + one commit, followed by non-forced expected-SHA update. No extra branches or force pushes.

## Active task

### Fix missing Clause 45 policy include in the shared MDIO path

- RAETH shared Clause 45 entry-point validation added a call to `raeth_mdio_cl45_address_status()` in `mii_mgr.c`, but omitted the same-directory `raeth_mdio_cl45_address_policy.h` include. The XY-C1 compiler treated the missing prototype as an error under `-Werror=implicit-function-declaration`.
- This source fix adds the missing header include. The existing standalone GNU89 address-policy test remains the relevant L2 regression; same-SHA XY-C1 Build/TRX/artifact is required for L3.
- Separate RC slice: `rc.c:init_router()` now preserves the first applicable firewall/NAT selector, VLAN, NAT restore and IPv6/filter prerequisite failure in `boot_services_status`, preventing PID 1 from reporting READY while WAN is intentionally blocked.

## Verification boundary

- XY-C1 #45436447371 for RAETH commit `a08b86e` failed during kernel compile: `mii_mgr.c:256` had an implicit declaration of `raeth_mdio_cl45_address_status`; TRX and artifact steps were skipped. This was a missing production include, not a host test failure.
- XY-C1 #45436853853 for exact SHA `65c6051` was still building at last poll and includes the same missing include. New include fix must be verified on its own exact SHA.
- RAETH Switch CLI #45436447892 on `a08b86e` PASS; AI handoff #45436446803 PASS; NVRAM #45436447356 was pending at last poll.
- RC firewall policy and checked-NVRAM host C regression binaries passed on source SHA `7ca46fc`. Later Firewall Startup runs were failing only at separate source-integration contracts; exact latest result is to be checked after this change. The forwarding-control contract was changed on `65c6051` to extract `if (firewall_startup_status != 0)` rather than the retired boolean helper.
- XY-C1 #45434352284 on exact firewall source SHA `7ca46fc`: Build, TRX verification and artifact upload all PASS. This verifies the firewall source fix at L3. XY-C1 #45434537034 on `a76f771` also completed Build/TRX/artifact PASS. Physical-device L4 is NOT VERIFIED.
- AI handoff contract #45436267737 on `1c4cdd6` PASS. RAETH Switch CLI #45435916372 and AI handoff #45435916408 on `2f95d1c` PASS.

## Exact first action for the next GPT

1. Inspect exact-head XY-C1 for the missing Clause 45 include fix; Build, TRX and artifact must all PASS on the same SHA.
2. Recheck the in-progress XY-C1 #45436853853 on `65c6051` (expected to fail for the pre-fix missing include) and close the run accurately; do not count it as L3.
3. Recheck NVRAM transaction audit #45436447356 and Firewall Startup Actions after the `65c6051` source-checker update. Fix any true failures with a separate atomic commit.
4. Then continue the broad RC startup-result propagation P1 queue or a higher-risk unfinished userland stability issue outside SmartDNS/DNS Forwarder. Pending CI does not block independent work.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. At each session ending, inspect main, the frozen backup, exact source SHA, exact-head Actions, L2/L3/L4 boundary and first next action. If accurate, do not create a no-op commit.

# Cross-session execution mandate

Recover once, implement, test, commit atomically and hand off. Before context exhaustion, record the exact commit, tests, CI results and first action. Never force-push or overstate firmware or hardware evidence.
