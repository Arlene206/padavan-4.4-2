## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main, the engineering baseline, exact-head Actions, validation boundaries and the first next action before ending. Update this file only when those facts materially change; do not create a no-op commit or chase documentation-only SHAs. Each multi-file logical slice must land as one atomic main commit. Before context exhaustion, finish the active safe slice and leave an exact first action.

# Cross-session execution mandate

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- Parent main SHA before the current inverted firewall-selector read slice: `bbd199ab0458296efacd741b473d0ef6d3853f93`. The VPN/NAT checked-read source and workflow contract are in this parent; the current inverted-selector reader, tests and records are one atomic commit.
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

- `ipt_filter_rules()` also contained inverted-boolean policy reads using unchecked `nvram_invmatch(key, "0")`. A backend error was converted to an empty string, which compares unequal to `"0"` and therefore enables the option. This affected `misc_ping_x` (inbound ping/traceroute), `udpxy_enable_x`/optional `xupnpd_enable_x` multicast acceptance, remote SSH/FTP enable gates, and `upnp_enable_x` forwarding-chain selection.
- This slice introduces `firewall_nvram_not_zero_status()` plus `firewall_read_not_zero_setting()`. Missing-key behavior intentionally retains legacy `nvram_invmatch(..., "0")` semantics (enabled), but NVRAM backend/read/close errors abort before rules are staged and trigger the existing rollback path. The checked values now replace all inverted reads for those keys in the IPv4 filter builder; optional compile-time flags remain guarded and UPnP is queried only when NAT is enabled.
- `firewall_nvram_match_policy.c` now tests zero/nonzero, unexpected legacy nonzero text, absent-key defaults, and backend/API failures with output preservation. The firewall workflow compiles/runs it under GNU89/Werror/ASan/UBSan and asserts each new checked gate precedes `firewall_open_rule_file()` and no old inverted getter remains.
## Verification boundary

- Firewall/VPN source commit `bbd199ab0458296efacd741b473d0ef6d3853f93`: Firewall Startup #44948667798 PASS (including its C policy tests and source contracts); Process inventory #44948667779 PASS; AI handoff #44948667780 PASS. NVRAM transaction audit #44948667802 and XY-C1 #44948667816 were still running at last poll. The earlier XHCI source SHA `692c260...` had Process integration #44948258619 fail only on a single-line checker match; checker-only commit `a727f2e78a42fac20d2558dee63ac78673551c79` later passed Process integration #44948463340 and AI docs #44948463272.
- The current inverted-selector slice requires a fresh exact-head Firewall Startup run and XY-C1 Build/TRX/artifact for L3. No local test execution or hardware L4 claim.
## Exact first action for the next GPT

1. Fetch current main and inspect exact-head Firewall Startup, Process inventory, AI docs, NVRAM and XY-C1 runs. The `firewall_read_not_zero_setting()` change must pass its GNU89/ASan/UBSan policy regression and source-order assertions; diagnose any failures. L3 requires same-SHA XY-C1 Build, TRX verification and artifact upload.
2. Close out the earlier XHCI source SHA `bbd199ab0458296efacd741b473d0ef6d3853f93` XY-C1 Build/TRX/artifact and NVRAM audit if still pending, without attributing those results to this new current commit.
3. Continue the firewall review of numeric listener ports and remaining direct `nvram_get_int()` calls only where they alter exposure or malformed ruleset publication. Preserve documented default/range semantics and use checked reads where backend failure changes policy.
## Workflow and preservation rules

- Develop only on `main`. Each logical slice is one complete Git tree, one commit and one non-forced expected-SHA update. Never force-push, amend history or write a multi-file slice as successive partial commits.
- Re-fetch main immediately before writing. If the expected SHA changed, stop and re-evaluate the new head; never blindly retry a rejected lease.
- Preserve GNU89, MIPS/uClibc, BusyBox, Linux 4.4, ABI, legacy NVRAM defaults, security and device compatibility. Never claim tests, builds, artifacts or hardware checks that were not actually performed.
- Root `dev_log.md` remains a newest-first 20–119-record rolling window; archive the oldest 100 complete records only when the next entry would reach 120. Preserve existing archive bytes and indexes. Update handoff/roadmap only when active task, module status or verification facts materially change.
