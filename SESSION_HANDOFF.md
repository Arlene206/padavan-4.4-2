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

- Firewall IPv4 and IPv6 URL filters now share the bounded, checked keyword reader. A URL keyword backend/API/close error aborts the staged family-specific restore rather than publishing a partial policy.
- Parent main SHA `4f59f6c72e1d35b323ee4c622bfdbf8f8be23da9` exposed an IPv6 integration defect: the helper signature had changed to require the cached entry count, but the IPv6 call still passed one argument; its function declaration had also become `static intip6t_filter_rules`. This atomic main checkpoint restores the proper symbol and adds the IPv6 checked-read abort path.
- The same parent caused a stale HTTPD source-contract assertion: it compared the VPN lease abort against the first firewall abort globally, which now belongs to earlier URL/LWF staging. The contract now scopes the abort search to after the VPN lease include call; VPN lease regression itself passed.
- Next engineering target after exact-head verification: audit Lightweight Filter schedule reads for fail-open NVRAM errors, then continue MAC-filter and static-route count/value review.

## Verification boundary

- Exact parent `4f59f6c72e1d35b323ee4c622bfdbf8f8be23da9` push Actions: Process inventory #44952467224 PASS; AI handoff #44952467200 PASS; WAN DHCP #44952467197 PASS; FULLCONENAT #44952467198 PASS; NVRAM audit #44952467177 PASS; WAN route #44952467210 PASS.
- Firewall Startup #44952467207 failed at the IPv6 URL source contract (Python line 500): the IPv6 function still called the changed helper with the old arity and did not propagate the checked-read failure. The three preceding compiled firewall regression steps passed. The source integration and regression contract are corrected in this checkpoint.
- HTTPD #44952467237 reached the VPN lease policy regression successfully, then failed because its source-order assertion selected earlier, unrelated URL/LWF abort calls. The checker now anchors the abort lookup after `include_vpns_clients()`.
- XY-C1 #44952467208 was still in progress at last query; Build XY-C1, TRX verification and artifact upload had not started. No L3 claim. No local C test or firmware build was available in this Git-data-only workspace. Hardware L4 remains NOT VERIFIED.
- The exact SHA for this checkpoint is the live main ref created by the commit containing this file; fetch `refs/heads/main` before any further work and validate only its push-event runs.

## Exact first action for the next GPT

1. Fetch live `main`; inspect exact-head Firewall Startup, HTTPD, NVRAM and XY-C1 Actions for this checkpoint. Diagnose any new failure from its first causal log; do not borrow results from parent SHA `4f59f6c...`.
2. Confirm same-SHA XY-C1 Build, TRX and artifact-upload completion before assigning L3.
3. Continue the checked LWF schedule audit: `timematch_conv()` currently uses unchecked `nvram_get()` for `filter_lw_date_x` and `filter_lw_time_x`, where backend failure can be mistaken for legacy defaults. Preserve missing-key defaults while aborting on actual read/close errors. Then audit MAC-filter and static-route loops.

## Workflow and preservation rules

- Develop only on `main`. Each logical slice is one complete Git tree, one commit and one non-forced expected-SHA update. Never force-push, amend history or write a multi-file slice as successive partial commits.
- Re-fetch main immediately before writing. If the expected SHA changed, stop and re-evaluate the new head; never blindly retry a rejected lease.
- Preserve GNU89, MIPS/uClibc, BusyBox, Linux 4.4, ABI, legacy NVRAM defaults, security and device compatibility. Never claim tests, builds, artifacts or hardware checks that were not actually performed.
- Root `dev_log.md` remains a newest-first 20–119-record rolling window; archive the oldest 100 complete records only when the next entry would reach 120. Preserve existing archive bytes and indexes. Update handoff/roadmap only when active task, module status or verification facts materially change.
