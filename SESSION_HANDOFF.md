## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main, the engineering baseline, exact-head Actions, validation boundaries and the first next action before ending. Update this file only when those facts materially change; do not create a no-op commit or chase documentation-only SHAs. Each multi-file logical slice must land as one atomic main commit. Before context exhaustion, finish the active safe slice and leave an exact first action.

# Cross-session execution mandate

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development and validation target: `main` only.
- Live `main` immediately before this workflow correction: `250ebfd2babb558957924e18033db434976fdbfe`. The product-source baseline includes the checked IPv6 service-type API and boot/WAN migrations from the preceding source commit.
- Branch inventory: `main` plus the preserved immutable archive `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Its `BACKUP_BRANCH_DO_NOT_TOUCH.md` was read; do not modify or delete it.
- PR #1 is closed and unmerged. Its closing comment records that valid CA trust-store/HMAC work was ported directly to main; no PR is used for development.
- GitHub Git data APIs are the available editing path. A local firmware checkout/toolchain is unavailable here.

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

1. Repair the exact-head Firewall Startup workflow failure before advancing the IPv6 migration. On `250ebfd2babb558957924e18033db434976fdbfe`, Firewall Startup run #44988923723 failed in the inline source-contract step with Python `NameError: name 'rc_source' is not defined`. The step already loads `trunk/user/rc/src/rc.c` into `rc`; its new IPv6 assertion mistakenly referenced the nonexistent `rc_source`. The workflow correction in this checkpoint changes only that assertion to inspect the already-loaded `rc`. No production C source changes in this correction.
2. After the corrected exact-head Firewall Startup run passes, continue the fail-closed `get_ipv6_type()` migration. The checked `get_ipv6_type_status(int *)` API and pure mapping policy are implemented; boot/cache publication, IPv6 global init/restart, WAN6 up/down and DHCPv6 callback paths use it. Remaining legacy callers identified by the current source record are in LAN policy, firewall rule setup, WAN interface setup, storage and OpenVPN. Inspect callers and side effects, then migrate bounded state-changing paths with regression tests; retain the legacy wrapper only where compatibility/read-only semantics justify it.
3. Do not infer target-build validation from host regressions. The product-source baseline still needs same-SHA XY-C1 Build, TRX verification and artifact upload to establish L3.

## Verification boundary

- Exact `main` push SHA `250ebfd2babb558957924e18033db434976fdbfe`: 40 workflow runs were visible. 36 completed successfully, including IPv6 service-type regression #44988923829 and Process lifecycle inventory #44988923596.
- Firewall Startup #44988923723 FAILED only at the inline checker step: `rc_source` is undefined. Its four earlier compile/run steps completed successfully; later steps were skipped because the source-contract step failed. The current workflow correction addresses that checker bug without weakening its assertion.
- Still PENDING at last inspection on exact SHA `250ebfd2babb558957924e18033db434976fdbfe`: NVRAM transaction boundary audit #44988923766; XY-C1 firmware validation #44988923611; USB storage hotplug #44988923719. Recheck the new SHA's relevant workflows; parent-SHA results do not validate a later commit.
- IPv6 service-type regression #44988923829 PASSED on `250ebfd2babb558957924e18033db434976fdbfe`; this is host L2 evidence for the checked-read policy, not an L3 firmware result.
- No same-SHA XY-C1 Build + TRX + artifact completion is verified for the current product-source baseline. L3 NOT VERIFIED; hardware L4 NOT VERIFIED.
- No local host test or firmware build is claimed; validation above is from GitHub Actions.

## Exact first action for the next GPT

1. Fetch live `main` and inspect push-triggered Actions for this workflow-correction SHA. Confirm Firewall Startup reran on that exact SHA and passed the previously failing source-contract step. If it fails, inspect and fix its first causal error.
2. Recheck the NVRAM, XY-C1 and USB runs by exact `head_sha`; do not reuse parent-SHA results.
3. Search all remaining `get_ipv6_type()` callsites and resume the next state-changing LAN/firewall/WAN-interface caller with a fail-closed regression, one atomic commit on main.
4. Claim L3 only after Build XY-C1, Verify TRX output and upload-artifact all pass on the same product-source SHA. Hardware L4 remains a separate device validation.
