## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main, the engineering baseline, exact-head Actions, validation boundaries and the first next action before ending. Update this file only when those facts materially change; do not create a no-op commit or chase documentation-only SHAs. Each multi-file logical slice must land as one atomic main commit. Before context exhaustion, finish the active safe slice and leave an exact first action.

# Cross-session execution mandate

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; develop and validate on `main` only.
- Live main before THIS checker-only correction: `71c4a9e9111d70caa62dd45213acb82844eee717`. Product source contains checked IPv6 service-state handling in LAN/LANv6, firewall, FTP/NFS, WAN VLAN/WISP, router hosts, DHCP 6rd and all three OpenVPN protocol-generation paths.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify/delete. PR #1 remains closed/unmerged.
- GitHub Git data API is the edit/push path; no local firmware checkout/toolchain is available.

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

1. Fix the two exact-head test-contract failures on OpenVPN source commit `71c4a9e9111d70caa62dd45213acb82844eee717`. Product code, the new GNU89 ASan/UBSan protocol-policy executable and VPN credential staging regression passed their preceding steps.
2. IPv6 service checker #44990566402 failed because its three source locators required a single-line exact string containing operation text, but C wraps `openvpn_resolve_protocol_family()` over multiple lines. The corrected source locators match the call token, while function scoping and ordering assertions continue to prove fail-closed behavior.
3. VPN lifecycle checker #44990566379 still demanded the deleted `get_ipv6_type()` fallback in exported-profile endpoint handling. It now requires the checked resolver before endpoint selection and separately checks resolver status propagation plus the pure policy's EIO/EACCES preservation tests. No weakening of dual-stack endpoint fallback ordering.

## Verification boundary

- Exact source SHA `71c4a9e9111d70caa62dd45213acb82844eee717`: IPv6 service-type #44990566402 FAILED only at `Verify checked IPv6 service-type integration`; GNU89 ASan/UBSan service-type C policy test passed. First cause was three whitespace-sensitive single-line source locators.
- Exact source SHA `71c4a9e9111d70caa62dd45213acb82844eee717`: VPN server lifecycle #44990566379 FAILED only at `Verify VPN lifecycle serialization and caller integration`; new OpenVPN IPv6 policy ASan/UBSan test and preceding lifecycle C tests passed. First cause was the checker retaining the removed legacy IPv6 getter expectation.
- Same SHA `71c4a9e9111d70caa62dd45213acb82844eee717`: XY-C1 Build #44990566341 IN_PROGRESS at last poll; NVRAM audit #44990566362 PASS; Process inventory #44990566399 PASS; OpenVPN client credential staging #44990566378 PASS; AI docs #44990566350 PASS; HTTPD #44990566325 PASS. Recheck XY after it finishes.
- No local C compile or firmware build claimed. This correction changes Python assertions and handoff only; exact-head source/VPN workflows must be rerun. L3 requires same-source-SHA Build XY-C1, TRX verification and artifact upload all PASS; hardware L4 NOT VERIFIED.

## Exact first action for the next GPT

1. Check exact-head IPv6 and VPN lifecycle Actions for THIS checker-only commit; require both the sanitizer-backed policy test and both Python source contracts to pass.
2. Recheck XY-C1 for the product source SHA `71c4a9e9111d70caa62dd45213acb82844eee717`; its result does not transfer to later product-source changes.
3. Scan the full rc/shared C/H source trees for any remaining direct `get_ipv6_type()` call, classify compatibility/read-only wrappers separately, and ensure every state-changing caller uses the checked API.
4. If migration is complete, select the next high-priority stability/security defect from the governance ledger and implement a bounded regression-backed fix. L3/L4 remain unverified until their explicit gates pass.
