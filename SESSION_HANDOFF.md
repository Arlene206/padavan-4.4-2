## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main, the engineering baseline, exact-head Actions, validation boundaries and the first next action before ending. Update this file only when those facts materially change; do not create a no-op commit or chase documentation-only SHAs. Each multi-file logical slice must land as one atomic main commit. Before context exhaustion, finish the active safe slice and leave an exact first action.

# Cross-session execution mandate

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; develop on `main` only, with complete atomic Git trees/commits and non-forced expected-SHA updates.
- Current source baseline before the checked getter follow-up: `2b2c2e6c1ef1ca9fc41dd39ebb88858e33cf1bcb`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only. PR #1 remains closed/unmerged.
- P1 WAN startup fix: `start_wan()` now uses checked `wan0_ifname` read, preserves an empty legacy no-op, validates the token and aggregates read/validation errors. Its source contracts were aligned after the observed stale/whitespace assertion failures.
- Exact 2b WAN DHCP callback #44995928887 PASS; WAN route status #44995928825 PASS; AI handoff docs #44995928838 PASS. The WAN interface-name policy executable and its expanded source contract passed inside WAN DHCP. Process/source-related tests on previous e344 also passed where recorded.
- Exact 2b XY-C1 #44995928841 was still IN_PROGRESS at last poll (host preflight/setup completed; firmware-input/build/TRX/artifact steps had not yet run). Do not claim L3 until same-SHA Build, TRX and artifact steps execute and pass.
- A second lossy read remains in `get_wan_ifname()`: it calls `get_man_ifname()`/`nvram_safe_get()` and wraps the potentially empty value as if it were successfully read. This is now being changed to a checked read only on the manual-interface fallback path, so modem/PPP overrides do not gain an unnecessary dependency on `wan0_ifname`.
- No local full MIPS toolchain and no hardware test. L3 requires Build XY-C1, TRX verification and artifact upload all execute and pass on an identical product-source SHA; hardware L4 NOT VERIFIED.

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

- `start_wan()` checked `wan0_ifname` read is complete. Exact 2b WAN DHCP callback #44995928887 and WAN route #44995928825 passed, along with AI handoff #44995928838.
- Current source slice closes the same `nvram_safe_get()` error-loss path in `get_wan_ifname()`. It reads/validates `wan0_ifname` via checked NVRAM APIs only when the manual interface is actually selected; active modem/PPP overrides do not acquire an unrelated manual-interface read dependency.
- Extend `scripts/check_wan_ifname_bounds.py` to pin checked fallback read, errno preservation and absence of `get_man_ifname()` from `get_wan_ifname()`; reuse existing `wan_ifname_copy` and checked-NVRAM tests.

## Verification boundary

- Exact 7bd storage and AI handoff checks passed. Exact 7bd XY-C1 was host-only and skipped Build/TRX/artifact (no L3).
- Exact 6ec/e344 WAN DHCP/route/XY failures and their precise stale checker causes are recorded above in the current state and dev log.
- Exact 2b WAN DHCP #44995928887 PASS; exact 2b WAN route #44995928825 PASS; exact 2b AI handoff #44995928838 PASS. Exact 2b XY-C1 #44995928841 was still IN_PROGRESS at last poll; no L3 claim.
- The checked `get_wan_ifname()` follow-up has not run exact-head Actions. No local MIPS build and no hardware test; L3/L4 remain unverified.

## Exact first action for the next GPT

1. Check exact-head WAN DHCP callback, WAN route and XY-C1 on the checked `get_wan_ifname()` SHA; fix any real source or checker failure.
2. Verify the checker asserts read only on the manual-interface fallback and preserves modem/PPP override behavior; ensure GNU89/ASan/UBSan interface and checked-NVRAM tests pass.
3. Check Build XY-C1, TRX verification and artifact upload on the identical product-source SHA before declaring L3.
4. Continue P1 WAN startup result-propagation review; backup remains read-only and hardware L4 unverified.