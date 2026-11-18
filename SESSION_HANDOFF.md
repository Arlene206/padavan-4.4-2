# 2026-11-18 continuation checkpoint — wireless AP-client reconnect settling

## Current repository state
- Repository `Arlene206/padavan-4.4-2`, branch `main`. Live main before THIS COMMIT: `daf0a387b830c17312c5872fc8a6916b43c149f2`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 remains closed/unmerged. AliDDNS security/bootstrap work is already integrated. Do not mutate backup, PR #1 or GPT Session Rotator.
- Main now uses shared interrupt-safe wait handling for WAN/PPP, native DHCPv6, PPPoE relay, NFSd, RC service startup, LAN PHY reset/readiness and LAN DHCP deconfig.

## Active task
- In `reconnect_apcli()` forced reconnect, the driver is told to disable ApCli, then raw `usleep(300000)` is used before enabling it again. An asynchronous signal could shorten that transition window and issue re-enable before the driver has finished its disable operation.
- THIS COMMIT switches to `service_wait_microseconds(300000U)`. Hard errors are normalized to errno/EIO and fed into `wireless_radio_restart_status()` so the first failure is sticky. APCLI re-enable is intentionally attempted even when disable or wait fails, preserving the recovery behavior.
- Wireless workflow source contract now verifies disable → shared wait → hard-error normalization/aggregation → re-enable, confirms no raw usleep or early return in the recovery window, and adds shared wait header/unit-test paths to push and pull_request triggers.

## Verification boundary
- Previous main SHA: `daf0a387b830c17312c5872fc8a6916b43c149f2`. On `daf0a387b830c17312c5872fc8a6916b43c149f2`, WAN route `44327167484`, WAN DHCP `44327167692`, detect-internet `44327167749`, Process inventory `44327167674`, Process fail-closed `44327167818`, detect_link `44327167645`, DNSMasq `44327167535`, AI docs `44327168058`, Wireless `44327167983` and HTTPD `44327169366` were queued/pending at last poll; XY-C1 `44327167924` and NVRAM `44327167631` were pending.
- Previous source SHA `65d783f785157fb2c88b002f287e251d45707eeb` exact-head WAN route, WAN DHCP, detect-internet, Process fail-closed/inventory and AI docs all PASS. XY-C1 `44326654807` and NVRAM `44326655048` were in progress at the previous poll.
- This wireless wait is not target-build or runtime verified. SmartDNS L3 is established only for `9bb9fdaeff826c957b599cc45e695eb3fb8cd32c`; current source L3 and physical Wi-Fi/AP-client L4 remain NOT VERIFIED.
- No local checkout-based tests were run in this connector-only session.

## Exact first action for the next GPT
1. Fetch live main and exact-head Wireless, WAN-route, Process, AI docs and XY-C1 runs created by THIS COMMIT; repair any first failure without weakening the recovery path.
2. Verify APCLI re-enable is still attempted after a wait failure and first-error status remains sticky.
3. Confirm the full Build/TRX/artifact chain on the exact current SHA before any L3 claim; retain AP-client radio/association runtime as a separate L4 test.
4. Continue the remaining cross-userland stability/security queue in the roadmap.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect current main and exact-head Actions before ending. Leave this handoff unchanged if task and verification boundaries remain accurate.

## Cross-session execution mandate
One logical slice is one atomic main commit. Create a complete Git tree from the exact parent, then update main with non-forced expected-SHA compare-and-swap. Continue independent work while Actions are pending. Before context exhaustion, record exact first action, SHA, regression and CI status. Never claim unobserved local tests, firmware builds or physical-router validation.
