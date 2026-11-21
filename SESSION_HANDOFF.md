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

- The IPv6 URL rule terminator is corrected, and Lightweight Filter schedule gating now uses presence-aware checked NVRAM reads. Missing keys preserve the legacy all-days/all-day defaults; present-empty/invalid/overlong entries remain inactive; backend/API/close errors abort staged IPv4 publication.
- Exact-head CI on `ef324cc77a34420655ab63cb856120c5fad59dc7`: Firewall Startup #44953373884 failed in its source contract at the new reader-body substring count; HTTPD #44953373906 failed in its private-rule cleanup-count contract after the fail-closed URL/LWF staging paths added cleanup sites. Both are workflow-contract brittleness, not failures in the preceding compiled tests.
- The firewall contract now slices the LWF helper by its actual source boundaries rather than the generic function-body parser. HTTPD now requires at least the original cleanup count rather than a brittle exact count.
- Next source target: harden MAC filtering. The WebUI is expected to cap `macfilter_num_x` at 64; confirm the input and count contract, then make per-entry MAC/schedule reads checked and propagate errors before publishing rules. Follow with static routes after confirming their UI cap.

## Verification boundary

- Exact parent `ef324cc77a34420655ab63cb856120c5fad59dc7`: WAN route #44953373909 PASS; Process inventory #44953373841 PASS; AI handoff #44953373949 PASS; FULLCONENAT #44953373928 PASS; WAN DHCP #44953373881 PASS. Firewall Startup #44953373884 failed at source-contract line 511; HTTPD #44953373906 failed in the later firewall private-rule cleanup-count contract. NVRAM #44953373840 and XY-C1 #44953373845 remained IN_PROGRESS at last poll (XY-C1 had reached toolchain setup before Build).
- Earlier C regression steps within Firewall Startup completed PASS before the source-contract assertion. HTTPD's URL sanitizer, firewall token-validation and VPN lease contracts preceding the cleanup check completed PASS.
- No local test/build and no completed same-SHA XY-C1 build/TRX/artifact evidence. No L3 claim; hardware L4 remains NOT VERIFIED.
- This checkpoint's exact SHA is the new main commit made from this parent. Re-fetch main and only use its exact push-event Actions.

## Exact first action for the next GPT

1. Re-fetch main; query its exact-head Firewall Startup, HTTPD, NVRAM and XY-C1 runs. Confirm both repaired source-contract suites pass; inspect logs if not.
2. Continue MAC filter audit: inspect WebUI cap, checked mode/count and per-entry address/date/time reads. Preserve absent-key defaults while failing closed on actual read errors.
3. Audit static-route UI input maximum and all per-entry conversions only after the limit is proven. Do not modify the frozen backup branch.

## Workflow and preservation rules

- Develop only on `main`. Each logical slice is one complete Git tree, one commit and one non-forced expected-SHA update. Never force-push, amend history or write a multi-file slice as successive partial commits.
- Re-fetch main immediately before writing. If the expected SHA changed, stop and re-evaluate the new head; never blindly retry a rejected lease.
- Preserve GNU89, MIPS/uClibc, BusyBox, Linux 4.4, ABI, legacy NVRAM defaults, security and device compatibility. Never claim tests, builds, artifacts or hardware checks that were not actually performed.
- Root `dev_log.md` remains a newest-first 20–119-record rolling window; archive the oldest 100 complete records only when the next entry would reach 120. Preserve existing archive bytes and indexes. Update handoff/roadmap only when active task, module status or verification facts materially change.
