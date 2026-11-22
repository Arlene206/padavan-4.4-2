## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main, the engineering baseline, exact-head Actions, validation boundaries and the first next action before ending. Update this file only when those facts materially change; do not create a no-op commit or chase documentation-only SHAs. Each multi-file logical slice must land as one atomic main commit. Before context exhaustion, finish the active safe slice and leave an exact first action.

# Cross-session execution mandate

## Current repository state
- Repository Arlene206/padavan-4.4-2; active branch main; parent for this storage firewall-refresh slice: 005d1e340cfc7a68b0686833c8ab96d30122fe73. Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807, read-only; PR #1 remains closed/unmerged.
- Firewall checked-IP snapshot fix is on main at cea789c171d0698f32cbe1d188ce8dda79c43353; its source-contract locator correction 005d1e340cfc7a68b0686833c8ab96d30122fe73 exact Firewall Startup Stability #45005183052 PASS and AI docs #45005183110 PASS.
- Exact cea NVRAM audit #45005080676 PASS. Exact cea XY-C1 #45005080673 was still IN_PROGRESS at last poll; no newer source L3 claim.
- USB printer checked-NVRAM fix at a5777a906c785d296e8612cf4954fc2a5dbb6749: USB/printer #45004808849, Process umbrella #45004808803, Process inventory #45004808838, NVRAM #45004808813, USB storage #45004808805, AI docs #45004808939 PASS; a577 XY-C1 #45004808822 still IN_PROGRESS at last poll.
- IAPP duplicate-start firewall repair at e29431f47e51c48bcbe0e8c4cf1adec51b55f36b: IAPP #45004292701, Process umbrella #45004292730, Process inventory #45004292778, AI docs #45004292741 and e294 XY-C1 #45004292784 PASS.
- Previous product L3 evidence remains only for a5d5bb332a7b18db1676f8e182a7ddaae92bd2ff; hardware L4 NOT VERIFIED.

## Completed source slices

- USB/storage hotplug: physical-unplug marker is retained until eject cleanup and marker acknowledgement both succeed; an action-completion latch avoids replaying completed cleanup when only marker ack is retried (source commit `d414a788`). `safe_remove_stor_device()` now queues the storage restart via `notify_rc_status("on_hotplug_mass_storage")` after releasing the storage operation lock, so a short-lived rc applet does not rely on its own `alarm(5)` to wake PID 1 (source commit `89a6e740`). The exact-head storage workflow is being reconciled with the actual source ordering.

- VPN client state policy: PPP VPN and OpenVPN up callbacks use checked `vpn_client_nvram_read_bool_status()`; a follow-up removed a retained legacy argument tail from `on_client_ifup()`. OpenVPN server callbacks prefer event-supplied `dev`, use checked topology fallback only when absent, and preserve disconnect hooks. Client DNS policy errors preserve last-good DNS/domain state while mandatory state/hook work continues.
- OpenVPN exported profiles activate `remote-cert-tls server` to match the generated certificate's `serverAuth` EKU. Dual-stack modes 4/5 use a validated WAN IPv4 endpoint only as final fallback after DDNS and both IPv6 sources fail; IPv6-only modes remain unchanged.
- WSD HTTP POST framing source `1c0ac607c66179ab58c3ae04c9cda2691c4dd91e`: bounded CRLF/exact-body reads, strict non-zero decimal Content-Length, duplicate/overflow rejection and 413 mapping. WSD framing #44942356801, AI docs #44942356848, Process inventory #44942356815, Process lifecycle #44942356893 and NVRAM #44942356787 passed. Exact XY-C1 #44942356809 passed Build, TRX verification and artifact upload; L3 applies to that exact source SHA.
- WSD LLMNR parsing: bounded uncompressed DNS question parsing, label and wire/text bounds, rejection of ambiguous NUL/dot bytes, and forced hostname NUL termination. The regression fixture and source-contract locator were corrected; #44945105615 passed on source/checker SHA `63854309f6c8b877563062119d0e169b7a7fd95f`.
- WSD XML output: request-derived RelatesTo and all seven configured GetResponse metadata values are escaped before XML insertion. On source `edc9d59456fe40d5c991b3a07507515ddf2c5eaf`, HTTP framing #44945383956, XML reflection #44945383982, Process inventory #44945383886 and NVRAM #44945383856 passed. Exact XY-C1 #44945383870 also passed Build, TRX verification and artifact upload; L3 is verified for this exact metadata source SHA.
- p910nd bidirectional SIGPIPE fix: source `23248f7df1575885e991fe518629bb974fc05c35`; host socketpair regression, p910nd, Process inventory, Process lifecycle and AI checks passed. Exact XY-C1 #44941685839 passed Build/TRX/artifact. L3 applies to that SHA; physical printer-disconnect L4 is NOT VERIFIED.
- MiniUPnP checked configuration preflight: source `be4952e90111ddea8f91684f6c3fe5ecf25be785`; checked protocol, secure mode, HTTP port, cleanup timers and mapping ranges. MiniUPnP, NVRAM, Process inventory, Samba and AI checks passed; exact XY-C1 #44941839623 passed Build/TRX/artifact. L3 applies to that SHA. The earlier redirect-description source `4a5139f484943abe3a5ac9cf0f6d56bc70067a56` also passed #44940761694. Hardware L4 remains NOT VERIFIED.
- RC AP/router-mode and LAN/WAN checked-read gates were expanded in commits `74e0ecc4`, `dbce98c2`, `ae3b0eb0`, `3e3ba126` and `202266c5`. These have mixed exact-SHA results; do not infer current-head PASS from older source or checker-only commits.
- HTTPD security workflow offset correction is in `c9005f49bb7edf6ac9efad6bfa51b82f6b902cc4`. The original checker run #44945277626 failed; the later exact-head HTTPD run #44945662341 on source descendant `f56fe999...` completed/success, validating the correction. Do not confuse this with the cancelled intermediate run #44945615000.

- MAC-filter hardening complete: supported WebUI modes are enforced, indexed MAC/date/time values use checked NVRAM reads, address tokens are validated, and absent schedule keys preserve legacy defaults. Exact source commit `8dd8a5bcb515b752dc9c4fd07e11440e929d5c49` passed HTTPD, NVRAM, Firewall Startup, WAN DHCP, WAN route, FULLCONENAT, inventory and AI docs; exact XY-C1 #44955441811 passed Build/TRX/artifact (L3 only for that SHA).
- Static-route hardening complete through source commit `182cfee52473b9f072816c6e5f67ef727945143e`: fixed WebUI 64-row Add off-by-one, checked NVRAM enable/count/entry reads, fail-closed handling, strict metric and gateway validation, preserved the historical invalid-gateway fallback, and retained compatibility up to the generic 128-row storage limit. Maximum serialized snapshot is 7,552 bytes < 8,192-byte buffer. Source commits `1ace1932a59c91e5e313ec53fb1a8d2560d90717`, `06e243f66c34949cf77603479e6135c925422f2c`, `182cfee52473b9f072816c6e5f67ef727945143e`.
- Static-route Delete checker now matches the escaped quotes emitted in the JavaScript string (`37d0b7b96ee8dcd0b654c4e2e514d36fb4e6fdb7`); storage IPTV dispatch check ignores formatting-only whitespace (`f3a1cdda6e36c3014809f276c31c84a119016158`). WAN-route result-propagation contract was corrected in `923f8c64dc7598f92ce454a0fc2f71beb8f5a227`; exact run #44956642473 passed.

## Active task
- Storage daemon lifecycle code in services_stor.c gated firewall refreshes with unchecked nvram_match("ftpd_wopen","1") and nvram_match("fw_enable_x","1"). Backend errors collapse to false, so a presence transition can leave stale FTP/Transmission/Aria port mappings. FTP additionally omitted ordinary RUNNING-to-ABSENT refresh and skipped cleanup when its port toggle was disabled.
- THIS FIX calls restart_firewall() for every actual FTP/Transmission/Aria process-presence transition and lets the firewall's own checked configuration decide whether to publish or clear rules. Storage start/stop app groups now include FTP even with its port toggle off and conservatively refresh for UNKNOWN daemon presence.
- Added source-contract regression assertions to scripts/check_storage_safe_remove.py; the existing storage-hotplug workflow already path-filters both modified files and runs the checker.

## Verification boundary
- Exact 005 Firewall Startup Stability #45005183052 PASS; AI handoff #45005183110 PASS. Exact cea NVRAM audit #45005080676 PASS. Exact cea XY-C1 #45005080673 was IN_PROGRESS at last poll.
- Exact a577 USB/printer #45004808849, Process umbrella #45004808803, Process inventory #45004808838, NVRAM #45004808813, USB storage #45004808805 and AI docs #45004808939 PASS; a577 XY-C1 #45004808822 was IN_PROGRESS at last poll.
- This storage firewall integration fix is not yet validated on its own SHA. No L3 claim unless same-SHA XY-C1 Build, TRX and artifact upload all pass; hardware L4 NOT VERIFIED.

## Exact first action for the next GPT
1. Check exact current-head storage-hotplug regression, Process umbrella/inventory, AI docs and XY-C1 Actions; diagnose any failure from its exact job log.
2. Claim L3 only if Build XY-C1, TRX verification and artifact upload all pass on the same exact current source SHA.
3. Continue an independent stability/security source fix outside completed WAN route, account snapshot, IAPP, USB printer and firewall IP work. Hardware L4 remains NOT VERIFIED.
