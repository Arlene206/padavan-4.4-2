# 2026-11-18 continuation checkpoint — detect-internet notification status propagation

## Current repository state
- Repository `Arlene206/padavan-4.4-2`, branch `main`. Parent main for THIS COMMIT: `8189e1ef1be11c637f57bda9a40d38f68752890d`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 remains closed/unmerged. Do not mutate the backup, PR #1 or GPT Session Rotator.
- Main contains the previously integrated signal-safe settling fixes for WAN/PPP, native DHCPv6, PPPoE relay, NFSd, detached daemons, LAN PHY reset/readiness, LAN DHCP grace, forced AP-client reconnect and iNIC reset.

## Active task
- Before THIS COMMIT, `notify_run_detect_internet()` logged monitor launch, PID-file signal and unknown-process-inventory failures but returned `void`, discarding the already-normalized status.
- THIS COMMIT changes it to return the normalized operation error; extends its unknown-inventory branch through the same errno/EIO mapping; and propagates results into WAN startup/up, LAN startup and PID 1's queued DI action. Existing status accumulators retain first error and independent setup continues.
- Teardown rollback/resume calls and the post-commit DHCP lease callback are explicitly best-effort; they must not overwrite the primary teardown error or invalidate an already committed lease.
- The detect-internet regression workflow now asserts return semantics and caller aggregation.

## Verification boundary
- Parent exact-head `0b8acba5190a4765f643090a852cbc53d216b97d`: iNIC regression `44328293059`, Wireless `44328293451`, watchdog signal `44328291899`, HTTPD security `44328293427`, Process inventory `44328294449`, Process fail-closed `44328292174` and AI docs `44328294526` PASS. NVRAM audit `44328294043` and XY-C1 `44328292088` were still in progress at last observation.
- Exact `0b8acba` source SHA includes iNIC reset settling; its physical reset timing remains unverified on hardware. This detect-internet API change needs exact-head detect-internet, WAN route, WAN DHCP, Process and XY-C1 Build/TRX/artifact evidence.
- No local checkout-based tests were run in this connector-only session. Full firmware L3 requires exact-SHA Build + TRX + artifact pass; physical WAN/LAN/iNIC/Wi-Fi runtime and L4 remain NOT VERIFIED.

## Exact next actions
1. Check the new live main SHA and exact-head detect-internet, WAN-route, WAN-DHCP, Process, AI-doc and XY-C1 runs.
2. Fix any causal contract/build failure; do not weaken first-error aggregation or best-effort rollback/lease behavior.
3. Implement the separately staged watchdog time-notification status propagation after this slice passes/while independent CI is pending.
4. Continue concrete reliability/security work across userland; do not repeat completed startup audits.

## Cross-session execution mandate
One logical slice is one atomic main commit. Build the complete Git tree from the exact parent and use a non-forced expected-SHA compare-and-swap to advance main. Continue independent work while Actions are pending. Leave the backup untouched. Never claim unobserved local tests, firmware builds or physical-router validation.
