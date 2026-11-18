# 2026-11-18 continuation checkpoint — watchdog-notification radio settling

## Current repository state
- Repository `Arlene206/padavan-4.4-2`, branch `main`. Live main immediately before THIS COMMIT: `0b8acba5190a4765f643090a852cbc53d216b97d`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 remains closed/unmerged, with AliDDNS security/bootstrap changes integrated. Do not mutate backup, PR #1 or GPT Session Rotator.
- Main has shared EINTR-safe waits across WAN/PPP, native DHCPv6, PPPoE relay, NFSd, detached RC daemons, LAN PHY restart/readiness, LAN DHCP deconfig, forced AP-client reconnect and iNIC reset.

## Active task
- Four manual radio/guest enable paths in `net_wifi.c` signal the watchdog via `notify_watchdog_wifi()`, then used raw `usleep(300000)` before mutating WLAN NVRAM and applying the radio/guest state. If interrupted, the radio action could race the watchdog's own Wi-Fi state notification/health handling.
- THIS COMMIT factors the pause into `wait_wifi_watchdog_update()`, backed by `service_wait_microseconds(300000U)`. On hard failure it preserves errno/EIO, logs the target radio/guest path and returns before changing NVRAM or invoking the radio controller. The watchdog notification is already sent, but the requested state transition is fail-closed rather than racing an incomplete settle interval.
- The Wireless workflow pins helper error handling and, for all four 2.4/5GHz main/guest paths, requires notify → full wait → failure return → NVRAM update → control call. It rejects raw 300ms usleep.

## Verification boundary
- AP-client forced reconnect commit `0ac383096f1c92581f566492b002bf71ebc74fd8` had one stale declaration assertion; checker correction `ea1d3722bcbbcc7f31806b3c8902500a2e8f228e` passed exact Wireless run `44328000618` and AI docs `44328000570`.
- On exact source SHA `0b8acba5190a4765f643090a852cbc53d216b97d`: Wireless `44328293451`, iNIC stability `44328293059`, watchdog signal `44328291899`, and Process inventory `44328294449` PASS. Process inventory integration `44328292174`, XY-C1 `44328292088`, NVRAM audit `44328294043`, Process fail-closed `44328292174` and HTTPD security `44328293427` were still in progress at last poll.
- On previous exact SHA `0ac3830`, NVRAM audit `44327593806` PASS; XY-C1 `44327593330` was still running at last observation.
- New exact-head Wireless, Process, AI docs and XY-C1 checks are required for THIS COMMIT. Full current-SHA Build/TRX/artifact and real AP-client/watchdog timing remain unverified. SmartDNS L3 applies only to exact product SHA `9bb9fdaeff826c957b599cc45e695eb3fb8cd32c`.
- No local checkout-based tests were run in this connector-only session.

## Exact first action for the next GPT
1. Fetch live main and exact-head Wireless, Process, AI docs, NVRAM, HTTPD and XY-C1 runs from THIS COMMIT; repair any first causal failure.
2. Verify failed watchdog settle waits block NVRAM/control mutations for all four manual radio/guest paths.
3. Confirm full XY-C1 Build/TRX/artifact on exact current source SHA before claiming L3; keep hardware timing/L4 separate.
4. Continue the remaining cross-userland stability/security queue in the roadmap.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect current main and exact-head Actions before ending. Leave this handoff unchanged if task and verification boundary remain accurate.

## Cross-session execution mandate
One logical slice is one atomic main commit. Create a complete Git tree from the exact parent, then update main with non-forced expected-SHA compare-and-swap. Continue independent tasks while Actions are pending. Before context exhaustion, record exact first action, commit SHA, regression checks and CI state. Never claim unobserved local tests, firmware builds or physical-router validation.
