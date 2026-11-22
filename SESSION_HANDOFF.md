## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main, the engineering baseline, exact-head Actions, validation boundaries and the first next action before ending. Update this file only when those facts materially change; do not create a no-op commit or chase documentation-only SHAs. Each multi-file logical slice must land as one atomic main commit. Before context exhaustion, finish the active safe slice and leave an exact first action.

# Cross-session execution mandate

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch is `main`.
- Live `main` before this workflow-contract/handoff follow-up: `ac8436355e5081e81737ffa758bd78fc2d73112e`; active firewall product-source SHA `e9ef41bd5a103b1693d5f0044d3ec5e5532c1661` and checker-only forwarding-contract correction `ef20a36bf970ec3c3c487cb71c2e0bdc615bf073`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains preserved at `72f4f157a084f8184b9597ca5bf334b8080e5807`. PR #1 remains closed/unmerged. Do not touch backup or revive PR #1.
- GitHub Git data API remains the editing path; full local firmware checkout/toolchain is unavailable here.

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

- P1 firewall startup read-state fix: production source commit `e9ef41bd5a103b1693d5f0044d3ec5e5532c1661`. PID 1 reads `fw_enable_x`/`wan_nat_x` via status-bearing NVRAM reads and blocks WAN on unknown/error state; default NAT preflights copied LAN IP/netmask/NAT-loopback settings. Forwarding checker fix `ef20a36bf970ec3c3c487cb71c2e0bdc615bf073`; exact Firewall Startup #44974378538 PASS.
- P2 LAN DHCP rollback snapshot fix: production source `6ca55ecdaca95858aadbf77c4fd1af49b0e1644c`. `udhcpc_lan_bound()` reads seven last-good tuple fields plus configured netmask in one transaction; read/close/truncation errors reject callback before state/kernel publication. Exact source L2 workflows DNSMasq #44974691080, WAN DHCP #44974691075, NVRAM audit #44974691126, Firewall #44974691074, Process inventory #44974691083/#44974691059, watchdog #44974691117, WAN route #44974691107, detect-internet #44974691207, detect-link #44974691084, wireless #44974691077 and AI docs #44974691103 PASS.
- HTTPD firewall source-contract reconciliation: #44974951724 PASS and AI docs #44974951739 PASS on `a05cb570eea8deb76d67fece4994ee415924c67e`; checker pins a lower bound and direct default-NAT checks instead of a brittle exact token count.
- OpenVPN certificate script reliability: `ddef1929c2c2799a464d8d397597f1e7d9f64b51` first failed syntax due generated adjacent function delimiters; `456c8176cc81209f0b75c2bc5f7d156589810870` fixed this. Exact OpenVPN #44975381015 PASS on `456c817` (syntax plus mocked crypto/permissions regression), AI docs #44975380936 PASS, Process inventory #44975380867 PASS. Follow-up `9f98dbe48d0f4bf6ece4cf025741cbca68c35d23` removes generated CA/server keys on chmod failure; exact OpenVPN #44975466561 PASS and Process inventory #44975466543 PASS; AI docs #44975466572 PASS.
- VPN lifecycle #44975466566 on `9f98dbe` failed at an obsolete assertion expecting the old unquoted signer call. THIS commit updates that contract to test the current quoted call and failure propagation. Further audit found a real defect: `DGST_ALG` already contains `-sha256`/`-sha1`, while `sign_cert()` prepended a second dash; this commit passes `"$6"` verbatim and rejects `--sha256` in the fake crypto tool.
- No local MIPS toolchain or router hardware is available; no target build or physical L4 claim.

## Verification boundary

- Parent main before THIS commit: `9f98dbe48d0f4bf6ece4cf025741cbca68c35d23`.
- Exact `9f98dbe`: OpenVPN certificate workflow #44975466561 PASS, AI docs #44975466572 PASS and Process inventory #44975466543 PASS. VPN lifecycle #44975466566 FAILED at stale signer-call text; this commit refreshes the checker and corrects the actual OpenSSL digest flag.
- XY-C1 #44975466578 on exact `9f98dbe` was IN_PROGRESS at last poll; verify Build XY-C1, TRX and artifact on the same source SHA. Older #44974691162 is tied to `6ca55ec`; L3 remains unconfirmed.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 closed/unmerged. Hardware L4 NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head VPN lifecycle, OpenVPN certificate, AI docs, Process inventory and XY-C1 Actions from THIS commit; fix real failures without weakening the fail-fast/digest contracts.
2. Require Build, TRX verification and artifact upload all PASS on the same exact source SHA before recording L3. If pending, work on an independent P1/P2 module.
3. Continue whole-userland stability/security work from the roadmap; do not repeat resolved DHCP/firewall audits or touch the frozen backup.

## Workflow and preservation rules

- Develop only on `main`. Each logical slice is one complete Git tree, one commit and one non-forced expected-SHA update. Never force-push, amend history or write a multi-file slice as successive partial commits.
- Re-fetch main immediately before writing. If the expected SHA changed, stop and re-evaluate the new head; never blindly retry a rejected lease.
- Preserve GNU89, MIPS/uClibc, BusyBox, Linux 4.4, ABI, legacy NVRAM defaults, security and device compatibility. Never claim tests, builds, artifacts or hardware checks that were not actually performed.
- Root `dev_log.md` remains a newest-first 20–119-record rolling window; archive the oldest 100 complete records only when the next entry would reach 120. Preserve existing archive bytes and indexes. Update handoff/roadmap only when active task, module status or verification facts materially change.
