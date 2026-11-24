## Current repository state

- Live branch: `main` at `bc17758dcb25dc349c9304a6d3f671600fc7f5c7`. Recent RC firewall source correction `7ca46fc7049f3380755f6b2fe5073d86727c6382`, checker fixes `03ca0dbf`, `77dc29c`, `1c4cdd6`, `65c6051`; RAETH includes Clause45 address bounds, Clause22 PHY address bounds and shared MDIO entry guards.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. Legacy PR #1 is closed/unmerged.
- Each logical task uses one complete Git tree and one commit, then a non-forced expected-SHA update of `main`. No force push or side branches.

## Active task

### Bound Clause 45 MDIO write payload width at the shared kernel entry point

- `mii_mgr_write_cl45()` accepted a 32-bit `write_data` and ORed it directly into the MDIO control word. Unlike the address fields, the 16-bit Clause45 data payload had no bound check. Values above `0xffff` can spill into control/address bits, potentially issuing a malformed MDIO command.
- Added `raeth_mdio_cl45_write_data_status()`; the kernel entry returns failure before acquiring the MDIO lock or touching hardware for an out-of-range payload. The standalone GNU89 policy test covers zero, max 16-bit, first invalid value and UINT32_MAX. Switch CLI workflow source contract enforces the guard before MDIO lock/command writes.
- A previous production compile issue was fixed at `a650ed8`: include `raeth_mdio_cl45_address_policy.h` in `mii_mgr.c`, required for the address checker prototype.

## Verification boundary

- Firewall startup status propagation in RC source SHA `7ca46fc7049f3380755f6b2fe5073d86727c6382` is L3 VERIFIED: exact XY-C1 run #45434352284 Build, TRX and artifact upload PASS. Physical-device L4 remains NOT VERIFIED.
- RAETH shared Clause45 source SHA `a08b86e` XY-C1 #45436447371 failed compilation for a missing policy-header include; fixed in `a650ed8`. Switch CLI #45436447892 on `a08b86e` passed; AI docs #45436446803 passed.
- XY-C1 #45436853853 on checker-only SHA `65c6051` concluded success but Build/TRX/artifact steps were SKIPPED, so it is not L3 evidence.
- Exact SHA `a650ed8`: Switch CLI #45437415025 failed one independent IRQ source-boundary checker because it sliced ESW function up to legacy `void sw_ioctl`; later checker-only commit `472064b` changed that boundary to `int sw_ioctl`, and Switch CLI #45437706525 on exact `472064b` PASS. AI handoff #45437414952 on `a650ed8` PASS. NVRAM #45437414867 and XY-C1 #45437415030 were still running at last poll; inspect final Build/TRX/artifact.
- XY-C1 #45434537034 on RAETH Clause45 address source SHA `a76f771` passed Build, TRX and artifact upload. Do not transfer that result to newer source SHA.
- New MDIO payload-width patch requires exact-head Switch CLI, NVRAM and XY-C1 checks. L3 needs same-SHA Build/TRX/artifact PASS.

## Exact first action for the next GPT

1. Fetch the commit SHA created for the Clause45 payload guard and inspect its exact-head Switch CLI, NVRAM and XY-C1 runs; verify Build, TRX and artifact steps individually.
2. Close XY-C1 #45437415030 for `a650ed8` and NVRAM #45437414867; if XY build failed, inspect the compiler log and fix the source.
3. Continue broad RC startup-result propagation P1 or a high-risk unfinished userland/kernel stability defect only after this MDIO safety slice is validated. Pending checks do not block independent work.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. At session end inspect live main, frozen backup, exact source SHA, exact-head Actions, L2/L3/L4 evidence and next action. If accurate, make no handoff commit.

# Cross-session execution mandate

Recover once, implement, test, commit atomically and hand off. Never force-push or overstate tests, firmware builds or hardware validation.
