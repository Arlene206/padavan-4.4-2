## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main, the engineering baseline, exact-head Actions, validation boundaries and the first next action before ending. Update this file only when those facts materially change; do not create a no-op commit or chase documentation-only SHAs. Each multi-file logical slice must land as one atomic main commit. Before context exhaustion, finish the active safe slice and leave an exact first action.

# Cross-session execution mandate

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; development and validation target: `main` only.
- Live main before THIS LAN hostname/IPv6 slice: `c29cb02e1536627abb824379ba92c8dc3d9cd8af`. Product source includes the checked IPv6 service-type reader, LAN IPv6 state-preservation fix (`net_lan6.c`) and fail-closed firewall builders (`firewall_ex.c`).
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify or delete it. PR #1 remains closed/unmerged.
- GitHub Git data APIs are the editing path; local firmware checkout/toolchain is unavailable.

## Completed source slices

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

- THIS slice migrates both remaining `net_lan.c` service-type reads. `create_hosts_lan()` checks `ip6_service` before opening either staged hostname/hosts file; a read failure returns without replacing the last-known-good dual-stack host files. A valid disabled setting still emits the legacy IPv4-only hosts configuration.
- `start_lan()` checks the service type before `reload_lan_addr6()`. Read failure is returned through the existing first-service-status aggregator while independent bridge setup continues; disabled IPv6 skips reload without reusing a stale prior operation status.
- The IPv6 source checker pins publication/reload ordering and rejects legacy getters in both function bodies. Push and PR path filters now include `net_lan.c` symmetrically.
- Remaining direct callers after this slice are in `net_wan.c`, `services_stor.c` and `vpn_openvpn.c`; continue with side-effect/transaction review, not blanket substitutions.

## Verification boundary

- Checker correction `c29cb02e1536627abb824379ba92c8dc3d9cd8af`: IPv6 service-type #44989518974 PASS and AI handoff docs #44989518977 PASS on that exact push SHA.
- Firewall source commit `a056dd3ef7b04fd3ebe5ca3f5a576a57f5d1dbb5`: Firewall Startup #44989409981, HTTPD security #44989409980, WAN route #44989409993, WAN DHCP #44989410029, AI docs #44989410105, NVRAM audit #44989409959 and Process inventory #44989409985 PASS on that exact SHA. XY-C1 #44989409968 remained IN_PROGRESS at last poll.
- THIS LAN hostname/IPv6 source commit has not yet run its exact-head checks. No local C compile or firmware build is claimed. L3 NOT VERIFIED until Build XY-C1, TRX verification and artifact upload all PASS on the same source SHA; hardware L4 NOT VERIFIED.

## Exact first action for the next GPT

1. Check exact-head IPv6 service regression; require GNU89 ASan/UBSan policy test and LAN/LAN6/firewall source-order contracts to PASS.
2. Check XY-C1 Build, TRX validation, artifact upload and relevant NVRAM/Process/WAN jobs by exact `head_sha`. Do not claim L3 from older source commits.
3. Migrate remaining checked service reads in `net_wan.c`, `services_stor.c` and `vpn_openvpn.c`, one bounded, transaction-safe change with regression per commit.
4. Hardware L4 remains unverified pending real-device validation.
