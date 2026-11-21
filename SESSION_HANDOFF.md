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

- The IPv4 and IPv6 filter builders preflight HTTP/HTTPS, SSH, FTP, UDPXY, Transmission and Aria flags and listener ports through checked NVRAM wrappers before opening their filter rule files. HTTPS/SSH/FTP-only caches are feature-guarded; rule generation reuses the snapshot. Existing Transmission/Aria invalid-but-readable port defaults in the 1024..65535 range are retained.
- Firewall Startup #44950535060 on parent `4cd3b21733ad373d2cb0f9adb418bef2df277458` failed at the brace-aware source-contract parser: `ip6t_filter_rules()` lacked a closing brace for the HTTP listener block before the HTTPS section. This also accidentally nested later HTTPS/SSH/FTP/UDPXY/Transmission/Aria/VPN rules under HTTP enablement. The block is now closed; CI explicitly asserts its boundary.
- After exact-head validation passes, continue inspecting `include_vts_nat()` and associated virtual-server/port-forward value conversion for malformed numeric ranges, port-zero/65536 handling and unchecked reads. Preserve valid range behavior.

## Verification boundary

- Parent SHA `4cd3b21733ad373d2cb0f9adb418bef2df277458`: Firewall Startup #44950535060 FAIL at Python stdin line 314 (`unterminated function: ip6t_filter_rules`) due the missing HTTP-block brace now fixed. HTTPD #44950534996 PASS; AI docs #44950535073 PASS; Process #44950535022 PASS; WAN route #44950534986 PASS; WAN DHCP #44950534973 PASS; FULLCONENAT #44950534983 PASS. NVRAM #44950535055 and XY-C1 #44950534974 were IN_PROGRESS at last poll.
- Listener-preflight SHA `8a7cf932879931c7df25708a9290dcc8eba2ed89`: Process #44950485777, WAN DHCP #44950485629, AI docs #44950485620, WAN route #44950485619 and FULLCONENAT #44950485662 PASS. Firewall Startup #44950485651 FAIL on the same missing-brace check; NVRAM #44950485650 and HTTPD #44950485750 were cancelled after newer pushes; XY-C1 #44950485679 remained IN_PROGRESS at last poll.
- Macro-guard cleanup SHA `4cd3b21733ad373d2cb0f9adb418bef2df277458` is the direct parent of the brace fix. No exact-head firewall/build pass may be inferred from an earlier SHA.
- Checker-correction SHA `13a229e14833708e70f0da7d541fb4aee8396459`: HTTPD Security #44950209297 PASS; AI docs #44950209330 PASS.
- OpenVPN cache SHA `6e67a918cf560634fa597fcdfd2ea35df96a78c3`: Firewall Startup #44950120695, AI docs #44950120745, Process #44950120902, WAN route #44950120777, FULLCONENAT #44950120775, WAN DHCP #44950120688 and NVRAM #44950120698 PASS; HTTPD #44950120693 failed only the old DMZ predicate literal fixed in `13a229...`; XY-C1 #44950120665 remained IN_PROGRESS at last query.
- No local checkout or local workflow execution is available. Do not claim L3 unless Build XY-C1, TRX verification and artifact upload all pass on one source SHA. Hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main` and query exact-head Firewall Startup, HTTPD, NVRAM, Process, AI docs, FULLCONENAT and XY-C1 Actions for the brace-fix commit. Inspect the first causal log on any failure.
2. Re-query XY-C1 for `4cd3b217...`, `8a7cf932...` and `6e67a918...` only as parent evidence; the newest source SHA requires its own Build/TRX/artifact gates.
3. Continue virtual-server/port-forward value audit only after the IPv4/IPv6 listener-preflight slice compiles and its source contract passes.

## Workflow and preservation rules

- Develop only on `main`. Each logical slice is one complete Git tree, one commit and one non-forced expected-SHA update. Never force-push, amend history or write a multi-file slice as successive partial commits.
- Re-fetch main immediately before writing. If the expected SHA changed, stop and re-evaluate the new head; never blindly retry a rejected lease.
- Preserve GNU89, MIPS/uClibc, BusyBox, Linux 4.4, ABI, legacy NVRAM defaults, security and device compatibility. Never claim tests, builds, artifacts or hardware checks that were not actually performed.
- Root `dev_log.md` remains a newest-first 20–119-record rolling window; archive the oldest 100 complete records only when the next entry would reach 120. Preserve existing archive bytes and indexes. Update handoff/roadmap only when active task, module status or verification facts materially change.
