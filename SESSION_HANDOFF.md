## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main, the engineering baseline, exact-head Actions, validation boundaries and the first next action before ending. Update this file only when those facts materially change; do not create a no-op commit or chase documentation-only SHAs. Each multi-file logical slice must land as one atomic main commit. Before context exhaustion, finish the active safe slice and leave an exact first action.

# Cross-session execution mandate

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- Latest product-source baseline before the current LAN QoS fix: `88054b9f272d9f673dabf45df60d497de441b2d2` (checker-only correction for WAN dynamic-route source contracts). The preceding source commit `c4a3bb279d23ca331572a91ab8ca85acfff8edc7` adds checked `dr_enable_x` selector reads to WAN/IPTV DHCP option setup and route additions while keeping cleanup unconditional. Exact-head WAN/HTTPD reruns for the checker repair must still be inspected. Re-fetch live `main` on takeover.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`. PR #1 is closed/unmerged. No feature branches/PRs, force-push or history rewriting. Do not touch GPT Session Rotator or replay SHA `9d5a5cb`.
- GitHub Git data APIs are being used; this execution workspace has no complete repository checkout.

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

- THIS source/test slice restores `vlan_pri[SWAPI_VLAN_RULE_WAN_LAN3] &= 0x07` immediately after the successful checked NVRAM read. The original `f20473f` implementation masked all six 802.1p priorities; the checked-read migration in `ebc799f` dropped only the LAN3 mask. Both RTL8367 and MTK ESW PHY encoders pack priority using an eight-bit field, so the omitted normalization could change LAN3 QoS values above seven.
- `scripts/check_wan_vlan_setup.py` now requires all six priority masks, each after its own checked read and before the first `phy_vlan_rule_set()`. Targeted preflight confirmed this contract rejects the parent source and passes the fix.
- Parent `c4a3bb279d23ca331572a91ab8ca85acfff8edc7` WAN route `#44962248260` and HTTPD `#44962248262` failed only on static contracts for the new dynamic-route selector. The checker-only fix is `88054b9f272d9f673dabf45df60d497de441b2d2`; its exact-head reruns had not appeared at the last poll. Parent XY-C1 `#44962248242` Build was pending, and NVRAM `#44962248222` was in progress.
- LAN checked-read work remains in place: WAN VLAN config, soft/core bridge multicast/router settings, switch mode/isolation/filter, six VIDs, six priorities/four tags, storm-control and jumbo/green/EEE preflights. Do not redo those audits.
- After exact-head verification of the new source commit, continue the ranked P1 PID-1 WAN NVRAM publication review at `set_wan_unit_value_int()` / `set_wan_unit_param()`.

## Verification boundary

- This LAN priority source/checker commit has not yet run its exact-head Actions. Inspect WAN route, HTTPD, Process, NVRAM and XY-C1 results for its live main SHA.
- The preceding checker correction `88054b9f272d9f673dabf45df60d497de441b2d2` had no exact-head run visible at the last poll; do not treat the c4 parent failures as resolved until the corrected SHA passes.
- L3 requires Bui## Exact first action for the next GPT

1. Fetch live `main` and inspect exact-head WAN route, HTTPD, NVRAM and XY-C1 runs for THIS source/test commit; inspect Build XY-C1, Verify TRX and artifact upload individually.
2. Repair only current-SHA failures. Also inspect the corrected WAN/HTTPD reruns for `88054b9f272d9f673dabf45df60d497de441b2d2`; do not claim the c4 failures resolved without those checks.
3. Continue the P1 `set_wan_unit_value_int()` / `set_wan_unit_param()` NVRAM publication audit after the LAN VLAN path validates.


## Workflow and preservation rules

- Develop only on `main`. Each logical slice is one complete Git tree, one commit and one non-forced expected-SHA update. Never force-push, amend history or write a multi-file slice as successive partial commits.
- Re-fetch main immediately before writing. If the expected SHA changed, stop and re-evaluate the new head; never blindly retry a rejected lease.
- Preserve GNU89, MIPS/uClibc, BusyBox, Linux 4.4, ABI, legacy NVRAM defaults, security and device compatibility. Never claim tests, builds, artifacts or hardware checks that were not actually performed.
- Root `dev_log.md` remains a newest-first 20–119-record rolling window; archive the oldest 100 complete records only when the next entry would reach 120. Preserve existing archive bytes and indexes. Update handoff/roadmap only when active task, module status or verification facts materially change.
