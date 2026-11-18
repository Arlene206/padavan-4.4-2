# 2026-11-18 continuation checkpoint — LAN PHY reset interval

## Current repository state
- Repository `Arlene206/padavan-4.4-2`, branch `main`. Live main immediately before THIS COMMIT: `8c4b3684e3386cbe63fc07ed46e5f77bf120ad37`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 remains closed/unmerged. Useful AliDDNS security/bootstrap work is already in main. Do not mutate the backup, PR #1 or GPT Session Rotator.
- Main includes shared EINTR-safe waits for WAN/PPP, native DHCPv6, PPPoE relay, NFSd, detached daemons and LAN DHCP deconfig. Latest CI contract repair is `8c4b3684e3386cbe63fc07ed46e5f77bf120ad37`.

## Active task
- `full_restart_lan()` powered the LAN PHY ports down, called raw `sleep(1)`, then powered the ports back up. A signal could interrupt that sleep and shorten the mandatory hardware reset interval.
- THIS COMMIT replaces the delay with `service_wait_seconds(1)`, normalizes a hard wait failure to errno/EIO, records it via `lan_restart_record_status()`, and still unconditionally attempts PHY power-up. The existing first-error accumulator preserves the initial restart failure while allowing recovery to proceed.
- WAN-route source contract now asserts power-down → EINTR-safe wait → error normalization/recording → power-up → LAN service start, and rejects old raw sleep. The shared wait C suite already covers repeated EINTR and hard errors.

## Verification boundary
- Main immediately before THIS COMMIT: `8c4b3684e3386cbe63fc07ed46e5f77bf120ad37`. CI-only commit `131238bbac61f8a3f5de03fe88be923795fcd3b7` fixed the WAN-route workflow's missing shared-wait test push/PR filters; exact-head WAN-route run `44325745439` PASS.
- Commit `8c4b3684e3386cbe63fc07ed46e5f77bf120ad37` aligned LAN DHCP callback checks with the already-landed `service_wait_microseconds(300000U)` fail-closed grace wait. Its exact-head WAN-DHCP, detect-internet and AI handoff Actions `44326368451`, `44326368532`, `44326368662` were IN_PROGRESS at the last poll; XY-C1 run `44326371312` was also IN_PROGRESS.
- On `d125e60`, Process inventory `44325478354`, NVRAM `44325478415`, HTTPD security `44325478364`, Wireless `44325478342`, detect_link `44325478326` and DNSMasq `44325478218` PASS. XY-C1 `44325478321` had not yet completed at last poll.
- Native DHCPv6 wait from `be7ff1d` and LAN PHY wait from THIS COMMIT need exact-source XY-C1 Build/TRX/artifact evidence. SmartDNS L3 is verified only for older exact source SHA `9bb9fdaeff826c957b599cc45e695eb3fb8cd32c`; this does not prove current L3. Live BusyBox/uClibc, real WAN/DHCPv6/PHY operation and physical-router L4 remain NOT VERIFIED.
- No local checkout-based tests were run in this connector-only session.

## Exact first action for the next GPT
1. Fetch live main and exact-head WAN-route, WAN-DHCP, detect-internet, Process, NVRAM, HTTPD and XY-C1 runs created by THIS COMMIT; fix any first causal failure without weakening behavior contracts.
2. Verify the power-cycle failure path always executes PHY power-up and preserves the first failure return.
3. Confirm a complete Build/TRX/artifact pass on the exact current firmware source SHA before claiming L3.
4. Continue the remaining stability/security queue in the roadmap.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect current main and exact-head Actions before ending. Leave this handoff unchanged if task and verification boundaries remain accurate.

## Cross-session execution mandate
One logical slice is one atomic main commit. Create the complete Git tree from the exact parent, then update main with non-forced expected-SHA compare-and-swap. Continue independent work while Actions are pending. Before context exhaustion, record the exact next task, SHA, tests and CI state. Never claim unobserved local tests, firmware builds or physical-router validation.
