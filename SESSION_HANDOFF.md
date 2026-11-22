## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main, the engineering baseline, exact-head Actions, validation boundaries and the first next action before ending. Update this file only when those facts materially change; do not create a no-op commit or chase documentation-only SHAs. Each multi-file logical slice must land as one atomic main commit. Before context exhaustion, finish the active safe slice and leave an exact first action.

# Cross-session execution mandate

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; production development on `main`. Parent main SHA for the current ZeroTier slice: `587427b3e8952a8bdefcd2bf8ec32c1c6ca2669c`. Recent production changes include the WAN DHCP checked LAN-subnet snapshot (`452f666cd126e06284c0607fd6b6bbbcabc1bbe1`) and checked LAN teardown identity reads (`587427b3e8952a8bdefcd2bf8ec32c1c6ca2669c`).
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only and untouched.
- Exact WAN DHCP callback run #45010299513 and AI handoff validation #45010299473 both PASS on prior SHA `f475d1f3d10d8e7eb8cd028520aa57704a8d6062`; the inline hostname contract repair and all twenty preceding C/source regression steps succeeded.
- Same-product-SHA validation for WAN/RC changes is still distinct: XY-C1 #45010044568 had completed all preparatory steps and was in Build XY-C1 at last poll; NVRAM audit #45010044539 was in its raw ioctl/API inventory step; Process lifecycle #45010044404 completed PASS. Recheck exact run outcomes. The later `587427b` LAN teardown production change needs its own exact Process/XY-C1 fan-out.
- A remaining ZeroTier uploaded-planet consistency defect is being fixed in THIS COMMIT: `zerotier_import_planet_file()` previously Base64-encoded the upload and then separately copied it, allowing one logical transaction to persist different bytes to disk and NVRAM if the upload changed between reads. The fix stages the upload once and derives both outputs from the private snapshot, with a race-injection regression. Exact-head ZeroTier Actions pending.
- Closed PR #1 (`fix(aliddns): verify CA trust-store bootstrap completion`, head `ba48e22609e0c81ad757b06756fe91d05f39cbf6`) remains unmerged; current main already has its OpenSSL 3.0.21 EVP_MAC helper, CA bootstrap/readability postcondition and matching HMAC/CA tests, plus evolved package/shell/RC integration. No duplicate source port is needed.

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

- Complete the ZeroTier uploaded-planet transaction consistency fix in THIS COMMIT. The private import stage is now the single source for both published disk bytes and the Base64 NVRAM value; an injected test mutates the original upload after Base64 reads the stage and asserts the two committed forms still match.
- Then run the exact-head ZeroTier lifecycle workflow, inspect the shell regression and Python source-contract result, and resolve any real checker/test failure without weakening the snapshot invariant.

## Verification boundary

- Current parent `587427b3e8952a8bdefcd2bf8ec32c1c6ca2669c`: WAN DHCP validation on prior checker/source SHA `f475d1f` and AI handoff validation both PASS. The later LAN teardown product change `587427b` still needs exact Process and XY-C1 evidence; prior SHA `452f666` had XY-C1 still building and NVRAM audit still inventorying at last poll.
- ZeroTier slice in THIS COMMIT changes `zerotier.sh`, the host shell regression and semantic checker. Exact Actions have not run yet; do not claim L2/L3. Hardware L4 NOT VERIFIED.
- No local repo checkout exists in this session; no local test execution is claimed. Actions is the validation path.

## Exact first action for the next GPT

1. Fetch live `main`, then inspect the exact-head ZeroTier lifecycle run triggered by THIS COMMIT. Check both shell race-injection regression and Python source-order contract; repair the first failure if any.
2. Inspect exact-source XY-C1 Build/TRX/artifact and Process lifecycle for the most recent LAN teardown product SHA `587427b3e8952a8bdefcd2bf8ec32c1c6ca2669c`; also revisit the pending NVRAM audit and XY-C1 runs for `452f666cd126e06284c0607fd6b6bbbcabc1bbe1`.
3. Continue an independent high-priority whole-userland stability/security defect after ZeroTier, preserving the frozen backup branch and main-only atomic commit rules. Hardware L4 remains NOT VERIFIED.

