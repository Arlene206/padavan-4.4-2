## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main, the engineering baseline, exact-head Actions, validation boundaries and the first next action before ending. Update this file only when those facts materially change; do not create a no-op commit or chase documentation-only SHAs. Each multi-file logical slice must land as one atomic main commit. Before context exhaustion, finish the active safe slice and leave an exact first action.

# Cross-session execution mandate

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; develop and verify on `main` only.
- Live main before THIS WAN regression-checker correction: `66944d56ea2da8a129f557f1d385a875f74e0333`. Product source has checked IPv6 reads for WAN VLAN/WISP setup and LAN router-hosts generation, plus LAN/LANv6, firewall, FTP and generated NFS paths.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`; never modify/delete. PR #1 remains closed/unmerged.
- GitHub Git data APIs are the edit/push path; no local firmware checkout/toolchain available.

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

1. Repair the stale WAN source assertions in `scripts/check_modem_wan_start_status.py`. The first source commit `80f478f76dfb6caf2c73ba774358201757a065e8` caused Firewall Startup #44989981636 and USB modem WDM/NDIS #44989981642 to FAIL only because the checker still searched for the old `if (get_ipv6_type() != IPV6_DISABLED)` guard. Product now uses checked preflight + `if (ipv6_type != IPV6_DISABLED)`; assert that checked read/error return happens before VLAN/WISP side effects and retain the control-if and interface-publication assertions.
2. Once fixed, continue the final direct `get_ipv6_type()` calls in `net_wan.c` (6rd DHCP option) and `vpn_openvpn.c` (protocol fallback) with regression coverage.

## Verification boundary

- Exact source SHA `80f478f76dfb6caf2c73ba774358201757a065e8`: IPv6 service-type #44989981542 PASS; WAN route #44989981485 PASS; WAN teardown #44989981553 PASS; HTTPD #44989981516 PASS; DNSMasq #44989981686 PASS; miniupnpd #44989981611 PASS; watchdog #44989981627 PASS; detect-link #44989981481 PASS; detect-internet #44989981458 PASS; ZeroTier #44989981539 PASS; wireless #44989981457 PASS.
- Two non-product checker failures on that SHA: Firewall Startup #44989981636 and USB modem WDM/NDIS #44989981642 both passed all preceding compile/runtime steps but failed when invoking `scripts/check_modem_wan_start_status.py` at obsolete assertion `parent_ipv6_guard >= 0`. Root cause: checker expected legacy getter; product now uses checked type/status guard. No C test failures reported in either job.
- XY-C1 #44989981487, WAN DHCP #44989981651, VPN lifecycle #44989981591, Process #44989981564 and ARP probe #44989981574 were IN_PROGRESS at last poll; follow up by exact SHA.
- Main `66944d56ea2da8a129f557f1d385a875f74e0333` has pending Firewall, IPv6, WAN route/teardown, NVRAM, AI docs, XY-C1 and other exact-head Actions.
- No local C build. L3 NOT VERIFIED until same-SHA XY-C1 Build, TRX verification and artifact upload pass; L4 hardware NOT VERIFIED.

## Exact first action for the next GPT

1. Check exact-head IPv6 service, Firewall Startup and USB modem WDM/NDIS Actions on THIS checker correction; confirm the stale-source checks are fully green.
2. Continue WAN DHCPv4 6rd option with checked service-type status before starting udhcpc; then finish OpenVPN protocol fallback so NVRAM read failure cannot silently downgrade IPv6 transport to IPv4.
3. Recheck XY-C1/NVRAM/Process and WAN jobs by exact `head_sha`; do not infer current results from older SHA.
4. L3/L4 remain unverified until explicit target/device gates pass.
