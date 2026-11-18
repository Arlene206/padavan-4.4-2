# 2026-11-18 continuation checkpoint — LAN PHY and DHCP readiness waits

## Current repository state
- Repository `Arlene206/padavan-4.4-2`, branch `main`. Live main immediately before THIS COMMIT: `07ef69dbac678786f287f5aa35b929af62f4c9e4`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 remains closed/unmerged. AliDDNS security/bootstrap changes are integrated in main. Do not mutate backup, PR #1 or GPT Session Rotator.
- Main includes EINTR-safe waits for WAN/PPP, native DHCPv6, PPPoE relay, NFSd, detached daemons, LAN PHY full restart and LAN DHCP deconfig. The latest independent source slice, `07ef69d`, makes pre-flash settling EINTR-safe and restores watchdog operation if that wait fails.

## Active task
- `start_lan()` still had a raw `sleep(1)` after validating the LAN DHCP callback script link and before launching `udhcpc`. A signal could shorten the PHY readiness interval and launch DHCP too early.
- THIS COMMIT replaces it with `service_wait_seconds(1)` under `do_wait`. A hard wait failure is normalized to errno/EIO, logged and preserved through the existing first-status accumulator; the DHCP client is not launched unless the wait succeeded. The 60-second detect-internet monitor wakeup remains in place.
- WAN-route source contract pins the do_wait guard, shared wait, error propagation, detect wakeup, DHCP launch gate and absence of raw sleep.

## Verification boundary
- Main before THIS COMMIT: `07ef69dbac678786f287f5aa35b929af62f4c9e4`.
- Previous exact-head source `65d783f785157fb2c88b002f287e251d45707eeb`: WAN route `44326655340`, WAN DHCP `44326654812`, detect-internet `44326655047`, Process fail-closed `44326655033`, Process inventory `44326655043` and AI docs `44326654440` PASS. XY-C1 `44326654807` and NVRAM audit `44326655048` were still in progress at last poll.
- `8c4b3684e3386cbe63fc07ed46e5f77bf120ad37`: WAN DHCP `44326368451`, detect-internet `44326368532`, AI docs `44326368662` and XY-C1 `44326371312` PASS.
- The `07ef69d` run fan-out (XY-C1 `44326957144`, Process inventory `44326957121`, Process fail-closed `44326958586`, WAN route `44326956790`, detect-internet `44326956656`, plus broad WAN, DNS, USB, firewall, service and security checks) was queued/in progress at last observation.
- The preceding LAN PHY full-restart fix at `65d783f` passed exact-head WAN route, WAN DHCP, detect-internet, Process inventory/fail-closed and AI docs. Its XY-C1 `44326654807` and NVRAM `44326655048` were pending at last poll.
- Native DHCPv6/LAN PHY/this DHCP readiness change still needs complete exact-SHA Build/TRX/artifact evidence. SmartDNS L3 remains proven only for `9bb9fdaeff826c957b599cc45e695eb3fb8cd32c`, not later SHAs. Live BusyBox/uClibc, PHY timing and router L4 remain NOT VERIFIED.
- No local checkout-based tests were run in this connector-only session.

## Exact first action for the next GPT
1. Fetch live main and inspect exact-head WAN-route, Process, WAN-DHCP, detect-internet and XY-C1 runs created by THIS COMMIT; fix any first causal failure.
2. Verify failed PHY readiness wait blocks udhcpc launch, returns the wait error, and leaves detect-internet monitor notification intact.
3. Confirm exact current SHA Build/TRX/artifact before any L3 claim; retain L4 as unverified until physical router tests.
4. Continue the remaining stability/security queue in the roadmap.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect current main and exact-head Actions before ending. Leave this handoff unchanged if the active task and verification boundary remain accurate.

## Cross-session execution mandate
One logical slice is one atomic main commit. Create a complete Git tree from the exact parent, then update main with non-forced expected-SHA compare-and-swap. Continue independent tasks while Actions are pending. Before context exhaustion, record exact first task, commit SHA, tests and CI status. Never claim unobserved local tests, firmware builds or physical-router validation.
