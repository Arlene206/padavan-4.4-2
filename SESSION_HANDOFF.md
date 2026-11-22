## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main, the engineering baseline, exact-head Actions, validation boundaries and the first next action before ending. Update this file only when those facts materially change; do not create a no-op commit or chase documentation-only SHAs. Each multi-file logical slice must land as one atomic main commit. Before context exhaustion, finish the active safe slice and leave an exact first action.

# Cross-session execution mandate

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; development and validation target `main` only.
- Live main before THIS IPv6 checker correction: `a056dd3ef7b04fd3ebe5ca3f5a576a57f5d1dbb5`. Product code now includes the checked LAN IPv6 state-preservation change from `e85eaf114556844668acaaea9fe4d9d7afd60406` and the firewall filter/NAT fail-closed migration from `a056dd3ef7b04fd3ebe5ca3f5a576a57f5d1dbb5`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify/delete. PR #1 is closed/unmerged.
- GitHub Git data APIs are the editing path; no local firmware checkout/toolchain is available.

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

1. Correct the IPv6 source-contract checker before advancing. Exact run #44989410008 passed the GNU89 ASan/UBSan policy test and failed at line 125 because the checker demanded exact `if (status != 0)`, while LAN predicates validly use combined `if (status != 0 || ipv6_type == IPV6_DISABLED)`. Change the assertion locator to the condition prefix without weakening the checked-reader and error-return requirements.
2. The same checker also contained three malformed function locators written as literal `\\n` sequences for the firewall builders. Replace them with explicit function-signature prefixes (`ipt_filter_rules(char *man_if`, `ip6t_filter_rules(char *man_if`, `ipt_nat_rules(char *man_if`) so the firewall contracts execute against their actual functions instead of failing at a later substring lookup.
3. Firewall product source migration is already in `a056dd3ef7b04fd3ebe5ca3f5a576a57f5d1dbb5`: IPv4/IPv6 filter generation now returns on service-type read error before opening rule files; IPv4 NAT only requires the service type when DMZ is active and reuses the checked type for the protocol-41 exception. Once the checker passes, continue remaining callers in `net_lan.c`, `net_wan.c`, storage and OpenVPN.

## Verification boundary

- Exact IPv6 service-type run #44989410008 on `a056dd3ef7b04fd3ebe5ca3f5a576a57f5d1dbb5`: GNU89 ASan/UBSan policy test PASS; integration checker FAIL at line 125 with strict substring mismatch. First cause is checker-only; no C compiler/test failure was reported.
- Firewall Startup #44989409981, HTTPD security #44989409980, WAN route #44989409993, WAN DHCP #44989410029 and AI docs #44989410105 all PASS on exact push SHA `a056dd3ef7b04fd3ebe5ca3f5a576a57f5d1dbb5`.
- XY-C1 firmware validation #44989409968 and NVRAM boundary audit #44989409959 remained IN_PROGRESS; Process lifecycle inventory #44989409985 remained IN_PROGRESS. L3 is not verified until Build XY-C1, TRX verification and artifact upload all pass on the same source SHA.
- No local firmware toolchain or local checkout is available. Hardware L4 NOT VERIFIED.

## Exact first action for the next GPT

1. Check the exact new main SHA's IPv6 service-type run. Require the ASan/UBSan policy executable and the full integration checker (LAN and firewall assertions) to PASS.
2. Inspect exact-SHA XY-C1, NVRAM and Process runs. L3 only after all three target build/TRX/artifact steps pass on that exact product SHA.
3. Continue the remaining checked service-type callsites in `net_lan.c`, `net_wan.c`, `services_stor.c` and `vpn_openvpn.c`, inspecting ordering and adding regressions before each atomic main commit.
4. Keep L4 as NOT VERIFIED until actual hardware boot/runtime testing.
