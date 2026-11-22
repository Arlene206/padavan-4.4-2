## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main, the engineering baseline, exact-head Actions, validation boundaries and the first next action before ending. Update this file only when those facts materially change; do not create a no-op commit or chase documentation-only SHAs. Each multi-file logical slice must land as one atomic main commit. Before context exhaustion, finish the active safe slice and leave an exact first action.

# Cross-session execution mandate

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; develop on `main` only, with complete atomic Git trees/commits and non-forced expected-SHA updates.
- Current product/checker baseline before the modem error assertion correction: `d018080e3ef848417c1969ba17adfd0b8a775624`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only. PR #1 remains closed/unmerged.
- `start_wan()` and `get_wan_ifname()` now use checked bounded `wan0_ifname` reads and validation only on manual-interface selection. `get_wan_ip4()` now selects modem/PPP first, then checks and validates manual interface only if no override is selected; `errno` preserves NVRAM/modem read failures while legacy `INADDR_ANY` remains the result sentinel.
- Exact d018 WAN DHCP #44996488482 PASS. Exact d018 WAN route #44996488531 FAILED at `scripts/check_wan_dns_read_status.py:515`: an older assertion still required a plain `if(status!=0)returnINADDR_ANY` branch after the code had been strengthened to preserve the specific error with `errno=status`. The preceding WAN DNS/manual-fallback source contract passed.
- Exact d018 AI docs #44996488511 PASS, HTTPD Authentication/Security #44996488357 was in progress in last poll, XY-C1 #44996488349 in progress in compiler setup/build prep. Many independent source suites on d018 had already passed. The WAN checker follow-up makes the modem identity error contract explicit rather than requiring the removed errno-less branch.
- No verified same-SHA Build XY-C1 + TRX + artifact result yet; L3 not claimed. No hardware test; L4 NOT VERIFIED.

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

- WAN checked manual-interface selection is implemented in `start_wan()`, `get_wan_ifname()` and `get_wan_ip4()`; exact d018 WAN DHCP #44996488482 passed.
- The exact d018 WAN route run failed only because `check_wan_dns_read_status.py` still searched for an errno-less return pattern after `get_wan_ip4()` was updated to preserve the real modem identity read failure in `errno`. The checker now locates the modem status read, requires its errno-preserving failure gate, and ensures it precedes transport selection.

## Verification boundary

- Exact d018 WAN DHCP #44996488482 PASS; exact d018 AI handoff #44996488511 PASS.
- Exact d018 WAN route #44996488531 FAIL at the stale errno-less modem-read assertion (`check_wan_dns_read_status.py:515`); earlier checker contracts through the new manual-fallback validation passed. This follow-up corrects that assertion only.
- Exact d018 XY-C1 #44996488349 was IN_PROGRESS at last poll; do not claim Build/TRX/artifact until each step passes on one SHA.
- No local MIPS build or hardware test. L3/L4 unverified.

## Exact first action for the next GPT

1. Check exact-head WAN route and WAN DHCP after the `get_wan_ip4()` modem-error assertion update.
2. Ensure the checker confirms modem identity read → errno-preserving failure gate → transport selection and manual fallback read/validation order.
3. Check whether d018 or a later product-source SHA completes Build XY-C1, TRX verification and artifact upload together; only that SHA can establish L3.
4. Continue WAN reliability review; leave backup branch untouched and hardware L4 unverified.