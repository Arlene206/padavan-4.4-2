# 2026-11-18 continuation checkpoint — AP-client reconnect wait contract

## Current repository state
- Repository `Arlene206/padavan-4.4-2`, branch `main`. Live main before THIS COMMIT: `0ac383096f1c92581f566492b002bf71ebc74fd8`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 remains closed/unmerged and AliDDNS security/bootstrap changes are already integrated. Do not mutate backup, PR #1 or GPT Session Rotator.
- Main includes EINTR-safe waits for WAN/PPP, native DHCPv6, PPPoE relay, NFSd, detached daemons, LAN PHY reset/readiness and LAN DHCP deconfig.

## Active task
- Production change at `0ac383096f1c92581f566492b002bf71ebc74fd8` replaces raw APCLI reconnect `usleep(300000)` with `service_wait_microseconds(300000U)`. It normalizes hard errors to errno/EIO, merges the wait error into the sticky wireless restart status, and still attempts the re-enable command so recovery is not skipped.
- Exact Wireless run `44327593252` failed only at its embedded declaration assertion: `net_wifi.c` correctly changed locals to `int status = 0, wait_status;`, but the checker still required `int status = 0;`. The shared helper, policy C tests, bridge membership tests, guest isolation sanitizer tests and other source contracts all passed before the Python source assertion. THIS COMMIT aligns that one source contract; no production behavior change.

## Verification boundary
- On exact `0ac383096f1c92581f566492b002bf71ebc74fd8`: Process inventory `44327593499`, Process fail-closed `44327593719`, HTTPD security `44327593507`, iNIC lifecycle `44327593416`, watchdog signal `44327593403`, and AI documentation `44327593386` PASS.
- Wireless run `44327593252` FAIL at the stale local-declaration assertion described above. The compiled wireless radio policy, bridge membership and guest-isolation regressions had already passed. The fixed contract in THIS COMMIT needs exact-head rerun.
- XY-C1 run `44327593330` and NVRAM audit `44327593806` remained IN_PROGRESS at last poll. No current-SHA L3 claim.
- SmartDNS Build/TRX/artifact PASS applies only to exact product SHA `9bb9fdaeff826c957b599cc45e695eb3fb8cd32c`; it does not transfer to current source. Physical AP-client association, BusyBox/uClibc and router L4 remain NOT VERIFIED.
- No local checkout-based tests were run in this connector-only session.

## Exact first action for the next GPT
1. Fetch live main and exact-head Wireless, AI docs, XY-C1 and NVRAM runs created by THIS COMMIT. Repair any first causal failure, without weakening the driver re-enable recovery path.
2. Confirm APCLI re-enable is attempted after a wait failure and the first wireless status remains sticky.
3. Confirm full XY-C1 Build/TRX/artifact on the exact current firmware SHA before claiming L3; keep physical wireless/L4 separate.
4. Continue the remaining stability/security queue in the roadmap.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect current main and exact-head Actions before ending. Leave this handoff unchanged if the task and verification boundary remain accurate.

## Cross-session execution mandate
One logical slice is one atomic main commit. Create a complete Git tree from the exact parent, then update main with non-forced expected-SHA compare-and-swap. Continue independent work while Actions are pending. Before context exhaustion, record exact first action, commit SHA, test results and CI state. Never claim unobserved local tests, firmware builds or physical-router validation.
