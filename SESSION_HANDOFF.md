## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main, the engineering baseline, exact-head Actions, validation boundaries and the first next action before ending. Update this file only when those facts materially change; do not create a no-op commit or chase documentation-only SHAs. Each multi-file logical slice must land as one atomic main commit. Before context exhaustion, finish the active safe slice and leave an exact first action.

# Cross-session execution mandate

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; active branch `main`; atomic complete-tree commits and non-forced expected-SHA updates only.
- Current source/checker baseline before this documentation rotation: `a3b7e95b71776808fbb6e6bddf273db8cf0c98f4`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only; PR #1 closed/unmerged.
- Checked manual WAN identity is used by `start_wan()`, `get_wan_ifname()`, `get_wan_ip4()`, `stop_wan_locked()`, `config_apcli_wisp()`, `check_apcli_wan()` and `full_restart_wan()`. The latest full restart fix validates the route-flush interface before router-mode refresh and WAN teardown, then reuses the checked bounded string.
- Exact a3 Actions: WAN route #44997377580 PASS; WAN DHCP #44997377452 PASS; WAN teardown #44997377521 PASS; wireless radio #44997377474 PASS; Process lifecycle #44997377507 PASS; Process inventory #44997377633 PASS.
- Exact a3 AI handoff #44997377383 FAILED because root `dev_log.md` reached 121 complete records. Resolved here by losslessly archiving the 101 oldest into `docs/dev-log/2026/11/part-025.md`, preserving newest 20 root entries and adding this checkpoint (21 root entries total). Index row and Git blob SHA are updated together.
- Exact a3 XY-C1 #44997377413 was still running `Build XY-C1` at last poll; TRX verification and artifact upload had not completed. NVRAM transaction audit #44997377584 and independent suites remained in progress. No L3 claim.
- L3 requires Build XY-C1, TRX verification and artifact upload all execute and pass on one identical product-source SHA. Hardware L4 NOT VERIFIED.

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

- State-changing WAN manual-interface read paths in startup, IP lookup, stop, AP-client WISP migration, AP-client radio restart and full WAN restart are now checked and validated before dependent state transitions.
- `full_restart_wan()` reads and validates `wan0_ifname` before `update_router_mode()` or `stop_wan()`, reuses the bounded local name for route flush, preserves empty legacy configuration and returns actual read/token errors without starting teardown.
- Current correction is documentation integrity: root development log reached 121 complete entries. Archived the 101 oldest verbatim in `part-025.md`, retained the newest 20 original entries, prepended one checkpoint and updated `docs/dev-log/INDEX.md` with exact size/count/Git blob SHA.

## Verification boundary

- Exact a3 WAN route #44997377580 PASS; WAN DHCP #44997377452 PASS; WAN teardown #44997377521 PASS; wireless radio #44997377474 PASS; Process lifecycle #44997377507 PASS; Process inventory #44997377633 PASS.
- Exact a3 AI handoff #44997377383 FAILED only because `dev_log.md` contained 121 entries. The current lossless rotation reduces root to 21 entries and archives 101 oldest records; exact-head AI handoff rerun is required.
- Exact a3 XY-C1 #44997377413 was running Build XY-C1 at last poll; TRX/artifact not completed. NVRAM audit #44997377584 also in progress.
- No same-SHA Build/TRX/artifact completion verified yet; L3 not claimed. No hardware test; L4 not verified.

## Exact first action for the next GPT

1. Check exact-head AI handoff after the root dev-log rotation; validate `part-025.md` checksum/count/size through the archive checker.
2. Poll XY-C1 #44997377413 and require Build, TRX verification and artifact upload all pass on the identical source SHA before L3.
3. Continue independent in-progress NVRAM/process/security stability suites; repair only the first concrete failure.
4. Continue WAN transition reliability work only where an unchecked state-changing read can cause wrong cleanup/restart; backup remains read-only and hardware L4 unverified.