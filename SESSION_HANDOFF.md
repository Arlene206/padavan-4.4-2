# 2026-11-18 continuation checkpoint — native DHCPv6 EINTR-safe readiness gate

## Current repository state
- Repository `Arlene206/padavan-4.4-2`, branch `main`. Live main immediately before this checkpoint: `34cdc9e12ce6d782da38340e5a98426e38122b5c`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 remains closed/unmerged. Do not mutate the backup, PR #1 or GPT Session Rotator.
- Recent production work includes EINTR-safe WAN DHCP/PPP settling, NFSd restart waits, RC detached-daemon readiness checks, logger wait/error aggregation, and PPPoE relay readiness. Main also includes a current process-checker follow-up (`592c0fd`).

## Active task
- Fix an unhandled interruptible delay in native DHCPv6 setup. In `trunk/user/rc/src/net_wan6.c::wan6_up()`, after route, privacy, autoconfiguration and RA setup, DHCPv6 launch was preceded by raw `sleep(2)` whose remaining time/result was ignored. A signal could shorten the settle interval and still launch `dhcp6c` before the interface readiness window completed.
- THIS COMMIT replaces the raw sleep with tested `service_wait_seconds(2)`. Any non-EINTR wait failure captures errno (or EIO), preserves errno, and jumps to the existing `native_failed` rollback path before marking DHCPv6 start attempted or launching the client. The existing shared wait regression covers EINTR retry and hard errors; the WAN route workflow now pins the wait/failure gate and launch ordering.
- This is a source-level reliability fix; it retains the existing cleanup order and no DHCPv6 runtime result is assumed.

## Verification boundary
- The prior current head before THIS COMMIT was `34cdc9e12ce6d782da38340e5a98426e38122b5c`. Its PPPoE relay update changed the relay readiness wait to `service_wait_seconds(1)` with error propagation, and updated the Process source contract.
- Process lifecycle run `44324719773` on `592c0fd` was still running at the previous poll. The new main head `34cdc9e` had Process inventory `44324943589`, Process regression `44324943624`, WAN route `44324943404`, firewall startup `44324943654`, USB storage `44324943687`, HTTPD `44324943429`, NVRAM `44324943786` and XY-C1 `44324943401` pending/in progress at last poll.
- WAN DHCP callback `44323940802` and AI handoff docs `44323940948` on `9cd6446` PASS.
- SmartDNS exact production SHA `9bb9fdaeff826c957b599cc45e695eb3fb8cd32c` has XY-C1 Build/TRX/artifact PASS; no current net_wan6 source SHA build is yet verified. Physical BusyBox/uClibc runtime, live IPv6/DHCPv6 traffic and L4 remain NOT VERIFIED.
- No local checkout-based tests were run in this connector-only session.

## Exact first action for the next GPT
1. Fetch live main and exact-head Actions; inspect the new WAN-route regression and XY-C1 build for THIS COMMIT. Verify the DHCPv6 source-contract passes and the full Build/TRX/artifact chain on the exact product SHA before any L3 claim.
2. Inspect Process inventory/fail-closed runs from the preceding RC wait work; follow any next failure to its source/contract cause.
3. Continue P1 WAN/firewall and other userland stability work only where a concrete gap remains; keep physical-router L4 explicitly separate.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main and exact-head Actions before ending. Leave this handoff unchanged if task, verification boundary and next action remain current.

## Cross-session execution mandate
One logical slice is one atomic main commit. Use the exact live tree, create a complete commit, and update main through non-forced expected-SHA compare-and-swap. Continue independent work while Actions are pending. Before context exhaustion, record precise next task, SHA, test results and CI status. Never claim unobserved local tests, firmware or physical-device validation.
