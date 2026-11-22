## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main, the engineering baseline, exact-head Actions, validation boundaries and the first next action before ending. Update this file only when those facts materially change; do not create a no-op commit or chase documentation-only SHAs. Each multi-file logical slice must land as one atomic main commit. Before context exhaustion, finish the active safe slice and leave an exact first action.

# Cross-session execution mandate

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; development and validation target is `main` only.
- Live main before THIS storage IPv6 service slice: `0673248bc999e9e06f4f69c8d1b0bb2d05c6bea3`. Product source has checked IPv6 migration through LAN hostname/hosts, LANv6 policy and firewall filters/NAT.
- Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`; never modify/delete. PR #1 remains closed/unmerged.
- GitHub Git data APIs are the edit/push path; no local firmware checkout/toolchain is available.

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

- THIS slice migrates both generated storage-service IPv6 mode reads. `write_vsftpd_conf()` checks service type after ordinary FTP option validation but before opening a staged config; a read/close failure preserves the active vsftpd config rather than silently downgrading to IPv4-only listener mode.
- `write_nfsd_exports()` leaves operator-supplied custom exports unchanged. For generated exports it checks service type after override validation but before opening `/proc/mounts` or a staged file. Failure returns before resource acquisition/publication; the successfully checked type is reused for IPv6 ACL generation.
- Dedicated IPv6 source checker pins both read-before-stage/side-effect boundaries and rejects legacy getters in these function bodies. Storage-hotplug workflow already tracks `services_stor.c`; the dedicated IPv6 workflow filters now include it too.
- Remaining direct calls after this slice: `net_wan.c` (VLAN/WISP, host generation, 6rd DHCP option) and `vpn_openvpn.c` (protocol fallback). Review state/transaction semantics before each conversion.

## Verification boundary

- Previous main SHA `0673248bc999e9e06f4f69c8d1b0bb2d05c6bea3`: IPv6 service-type #44989677353 PASS; AI docs #44989677501 PASS; DNSMasq #44989677293 PASS; watchdog #44989677335, detect-internet #44989677332, wireless #44989677260, WAN DHCP #44989677276, detect-link #44989677304 and Firewall Startup #44989677298 PASS.
- On that same SHA, XY-C1 #44989677285, NVRAM #44989677366, Process inventory/lifecycle #44989677317/#44989677371, HTTPD #44989677261 and WAN route #44989677288 were IN_PROGRESS at last poll. These are not current validation for THIS storage commit.
- THIS storage IPv6 source/checker SHA awaits exact-head Actions. No local C compile, firmware build, L3 or L4 result is claimed.
- L3 requires same-SHA XY-C1 Build, TRX verification and artifact upload all PASS; hardware L4 NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect THIS storage commit's exact-head IPv6 service-type and storage-hotplug Actions; require the source-order checker and existing storage regression/build jobs to pass.
2. Check XY-C1 Build + TRX verification + artifact upload and NVRAM/Process/WAN jobs by exact `head_sha`; older source results do not validate this commit.
3. Migrate remaining callers in `net_wan.c` and `vpn_openvpn.c` with state/transaction review and one regression-backed atomic commit per slice.
4. Keep L3/L4 unverified until exact target/device gates pass.
