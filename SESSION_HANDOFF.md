## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main, the engineering baseline, exact-head Actions, validation boundaries and the first next action before ending. Update this file only when those facts materially change; do not create a no-op commit or chase documentation-only SHAs. Each multi-file logical slice must land as one atomic main commit. Before context exhaustion, finish the active safe slice and leave an exact first action.

# Cross-session execution mandate

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; develop on `main` only, with complete atomic Git trees/commits and non-forced expected-SHA updates.
- Current source baseline before this checker-alignment slice: `e34444ae55651085a157996f4c6b5699b20066c9`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only. PR #1 remains closed/unmerged.
- Storage retry durability remains covered: exact 7bd storage #44995310447 PASS and AI handoff #44995310444 PASS. Exact 7bd XY-C1 #44995310496 completed host-only SUCCESS, but Build XY-C1/TRX/artifact were skipped; it is not an L3 build.
- P1 product fix: `start_wan()` now checks `wan0_ifname` using `get_wan_unit_value_copy_status()`, preserves an empty legacy no-op, validates non-empty interface tokens, and propagates read/validation errors into WAN startup status.
- Exact 6ec WAN DHCP #44995722211 FAILED only in the newly added checker locator (`\\n` literal); its GNU89/ASan/UBSan WAN interface policy executable passed. Exact 6ec WAN route #44995722139 and XY-C1 #44995722192 FAILED at `scripts/check_wan_firewall_status.py:1170`, whose old source contract still required `get_man_ifname()` after the new checked read had removed it.
- Follow-up e344 corrected the newline locator and diagnostic varargs, but exact e344 WAN DHCP #44995792863 FAILED at `check_wan_ifname_bounds.py:26` because two whitespace-sensitive assertions omitted the actual spaces after `(`. Exact e344 WAN route #44995792838 and XY-C1 #44995792759 FAILED at the same stale line 1170. AI handoff #44995792742 and Process inventory #44995792750 PASS.
- No local full MIPS toolchain and no hardware test. For L3, Build XY-C1, TRX verification and artifact upload must all execute and pass on one identical product-source SHA; hardware L4 NOT VERIFIED.

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

- Product source fix is in `net_wan.c`: checked `wan0_ifname` read, legacy-empty no-op preservation, strict `wan_ifname_copy()` validation and first-error aggregation.
- Aligning both source contracts with the new behavior: `check_wan_ifname_bounds.py` must normalize the whitespace in the checked-reader/validator calls; `check_wan_firewall_status.py` must assert checked read → failed-read gate → empty-key no-op → validated token → failed-validation gate → first network side effect, without requiring the old lossy getter.

## Verification boundary

- Exact 7bd storage and AI handoff checks passed; exact 7bd XY-C1 was host-only and skipped Build/TRX/artifact (no L3).
- Exact 6ec WAN DHCP failed at its new checker locator; the corresponding C policy executable passed. Exact 6ec WAN route and XY-C1 failed on the stale `get_man_ifname()` assertion. These are recorded as FAIL, not PASS.
- Exact e344 WAN DHCP still failed at whitespace-sensitive checker assertions; exact e344 WAN route and XY-C1 still failed on the stale getter assertion. AI handoff and Process inventory passed. The current follow-up aligns both checkers; its exact-head Actions remain to be checked.
- No MIPS firmware build/trx/artifact has yet passed on the new product-source SHA; L3/L4 unverified. Backup remains read-only.

## Exact first action for the next GPT

1. Check exact-head WAN DHCP callback, WAN route and XY-C1 on the checker-alignment SHA; fix the first actual assertion/compile failure and do not weaken product behavior.
2. Verify the WAN interface checker reaches its order/error assertions and the C GNU89/ASan/UBSan tests pass.
3. Confirm Build XY-C1, TRX verification and artifact upload all execute and pass on that same product-source SHA before declaring L3.
4. Continue P1 WAN startup result-propagation review after the interface-name path; keep backup read-only and hardware L4 unverified.