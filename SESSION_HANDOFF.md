## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main, the engineering baseline, exact-head Actions, validation boundaries and the first next action before ending. Update this file only when those facts materially change; do not create a no-op commit or chase documentation-only SHAs. Each multi-file logical slice must land as one atomic main commit. Before context exhaustion, finish the active safe slice and leave an exact first action.

# Cross-session execution mandate

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- Last meaningful product-source baseline: `6e67a918cf560634fa597fcdfd2ea35df96a78c3` (`fix(firewall): use checked OpenVPN protocol cache`). The live `main` ref is fetched independently on each takeover; later checker/documentation commits do not change this source baseline.
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

## Active task

- IPv4/IPv6 filter listener preflight, UDPXY 1024..65535 range validation and VTS count/protocol safety are implemented in current main ancestry.
- A new LWF audit found two real policy defects: the rule count used unchecked `foreach_x("filter_lw_num_x")` despite the UI's hard cap of 64, and `nvram_match("filter_lw_default_x", "DROP")` collapsed backend/close errors into false. That could silently switch a configured whitelist (`DROP`) to blacklist behavior, removing the whitelist's terminal drop.
- This slice snapshots the exact `DROP` mode and bounded rule count through checked NVRAM reads before filter rule staging; missing mode retains the legacy false/blacklist default, while backend errors abort the restore transaction before publication. The rule builder consumes only the cached mode/count.
- Exact-head validation of the preceding VTS slice on `fc208c6baf1956855799c3d3320b5d88ae440d7a`: Firewall Startup #44951481093 PASS; HTTPD #44951481068 PASS; HTTPD lifecycle #44951481171 PASS; FULLCONENAT #44951481035 PASS; WAN DHCP #44951481067 PASS; WAN route #44951481066 PASS; Process lifecycle #44951481091 PASS; Process inventory fail-closed #44951481191 PASS; AI docs #44951481072 PASS. NVRAM #44951481074 and XY-C1 #44951481069 were IN_PROGRESS; Build XY-C1 was running and TRX/artifact pending.

## Verification boundary

- Parent product SHA `fc208c6baf1956855799c3d3320b5d88ae440d7a`: Firewall Startup #44951481093 PASS (GNU89/Werror/ASan/UBSan tests and source contracts); HTTPD auth #44951481068 PASS; HTTPD lifecycle #44951481171 PASS; FULLCONENAT #44951481035 PASS; WAN DHCP #44951481067 PASS; WAN route #44951481066 PASS; Process lifecycle #44951481091 PASS; Process inventory fail-closed #44951481191 PASS; AI docs #44951481072 PASS. NVRAM #44951481074 and XY-C1 #44951481069 were still running at last query. XY-C1 Build was IN_PROGRESS; TRX verification and artifact upload pending.
- VTS count parents `79cfb79099f40e3b67b6c23a520c623c422c2d16` and `b60f5d95776c8f7cc78d30edc7c6c2c6bfe032a3` failed Firewall Startup only at the signature/body contract. This was corrected in `fc208c6...`; the descendant exact-head check passes.
- No local C tests or firmware build were run. No L3 claim without Build XY-C1, TRX verification and artifact upload PASS on one SHA; hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main` and exact-head Actions for the LWF mode/count fix. Require Firewall Startup and NVRAM checks to pass; inspect the earliest failure log if not.
2. Re-query XY-C1 #44951481069 and NVRAM #44951481074 for parent `fc208c6...` only as historical evidence.
3. Continue reviewing LWF raw values (protocol flags, ICMP list and schedule) after the checked mode/count slice validates. Preserve absent-key defaults and never change whitelist/blacklist semantics.

## Workflow and preservation rules

- Develop only on `main`. Each logical slice is one complete Git tree, one commit and one non-forced expected-SHA update. Never force-push, amend history or write a multi-file slice as successive partial commits.
- Re-fetch main immediately before writing. If the expected SHA changed, stop and re-evaluate the new head; never blindly retry a rejected lease.
- Preserve GNU89, MIPS/uClibc, BusyBox, Linux 4.4, ABI, legacy NVRAM defaults, security and device compatibility. Never claim tests, builds, artifacts or hardware checks that were not actually performed.
- Root `dev_log.md` remains a newest-first 20–119-record rolling window; archive the oldest 100 complete records only when the next entry would reach 120. Preserve existing archive bytes and indexes. Update handoff/roadmap only when active task, module status or verification facts materially change.
