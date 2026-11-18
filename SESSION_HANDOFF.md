# 2026-11-18 continuation checkpoint — iNIC reset settling

## Current repository state
- Repository `Arlene206/padavan-4.4-2`, branch `main`. Live main immediately before THIS COMMIT: `ea1d3722bcbbcc7f31806b3c8902500a2e8f228e`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 remains closed/unmerged; useful AliDDNS security/bootstrap changes are integrated. Do not mutate backup, PR #1 or GPT Session Rotator.
- Main has signal-safe waits for WAN/PPP, native DHCPv6, PPPoE relay, NFSd, detached daemons, LAN PHY power/readiness, LAN DHCP grace and forced AP-client reconnect.

## Active task
- `start_inic_mii()` released the iNIC reset GPIO and then used raw `usleep(50000)` before enabling the RGMII port. A signal could shorten that hardware reset settling window and expose the driver to an early RGMII/interface transition.
- THIS COMMIT uses `service_wait_microseconds(50000U)`. Any hard wait failure is normalized to errno/EIO, logged, and routed to the existing `fail_start` rollback before the RGMII enable or iNIC interface-up calls. Existing fail-start cleanup continues through interface down, bridge detach, isolation, RGMII disable and reset assertion.
- Wireless workflow source contract pins GPIO release → shared wait → failure normalization/log/rollback → RGMII enable, and rejects the old raw usleep. Shared wait unit test already exercises EINTR retry and hard errors.

## Verification boundary
- AP-client reconnect implementation commit `0ac383096f1c92581f566492b002bf71ebc74fd8` had exact Wireless run `44327593252` FAIL only because a source assertion still expected the old local declaration. The assertion was corrected in `ea1d3722bcbbcc7f31806b3c8902500a2e8f228e`; its exact Wireless run `44328000618` PASS and AI docs run `44328000570` PASS.
- On `0ac3830`, Process inventory `44327593499`, Process fail-closed `44327593719`, HTTPD security `44327593507`, iNIC lifecycle `44327593416`, watchdog signal `44327593403`, and AI docs `44327593386` PASS; NVRAM `44327593806` PASS. XY-C1 `44327593330` remained in progress at the last poll.
- Current AP-client and iNIC changes need exact-head Wireless, Process, AI docs and XY-C1 checks. Full Build/TRX/artifact proof for current source is not established. SmartDNS L3 applies only to exact source SHA `9bb9fdaeff826c957b599cc45e695eb3fb8cd32c`. Physical iNIC reset timing and Wi-Fi hardware L4 remain NOT VERIFIED.
- No local checkout-based tests were run in this connector-only session.

## Exact first action for the next GPT
1. Fetch live main and exact-head Wireless/Process/AI docs/XY-C1 runs from THIS COMMIT; fix the first causal failure.
2. Confirm iNIC wait failure goes to fail-start before RGMII/interface-up, and every rollback cleanup action remains attempted.
3. Confirm full XY-C1 Build/TRX/artifact on exact current SHA before claiming L3; keep hardware timing/L4 separate.
4. Continue the remaining stability/security queue in the roadmap.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect current main and exact-head Actions before ending. Leave this handoff unchanged if the active task and verification boundary remain accurate.

## Cross-session execution mandate
One logical slice is one atomic main commit. Create the full Git tree from the exact parent, then update main by non-forced expected-SHA compare-and-swap. Continue independent work while Actions are pending. Before context exhaustion, record exact next task, SHA, tests and CI status. Never claim unobserved local tests, firmware builds or physical-router validation.
