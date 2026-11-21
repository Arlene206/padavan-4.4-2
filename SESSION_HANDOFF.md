## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main, the engineering baseline, exact-head Actions, validation boundaries and the first next action before ending. Update this file only when those facts materially change; do not create a no-op commit or chase documentation-only SHAs. Each multi-file logical slice must land as one atomic main commit. Before context exhaustion, finish the active safe slice and leave an exact first action.

# Cross-session execution mandate

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- Parent main SHA before the current firewall/VPN NVRAM fail-closed slice: `a727f2e78a42fac20d2558dee63ac78673551c79`. The XHCI source and its whitespace-normalized Process checker repair are included in this parent; the current firewall source/integration checks plus handoff records form one new atomic commit.
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

- A firewall ruleset review found `ipt_filter_rules()` still used unchecked NVRAM getters for security policy selectors `wan_nat_x`, `vpns_enable/type/actl` and `vpnc_enable/type/sfw`. In particular, a backend failure for `vpnc_sfw` collapsed to zero, the branch interpreted as ACCEPT for the VPN-client interface; a `vpns_actl` failure similarly defaulted to broad policy mode `0`.
- This slice moves those reads to existing `firewall_read_enabled_setting()` / `firewall_read_integer_setting()` wrappers before `firewall_open_rule_file()`, so read/close/parser failures abort ruleset generation and invoke the staged-restore rollback gate. Missing keys preserve their prior defaults (false/zero).
- OpenVPN-specific server mode/protocol/port and client TUN/TAP selectors are also checked only when the corresponding OpenVPN server/client is enabled and type 2, avoiding unnecessary reads for inactive features. The server port retains its default 1194 when outside 1..65535, matching its prior `nvram_safe_get_int(..., 1194, 1, 65535)` range behavior. Firewall workflow source-contract assertions now pin these checked reads before file creation; its existing GNU89/ASan/UBSan helper test covers strict parser and backend failures.
## Verification boundary

- XHCI source commit `692c260ecf9d49ab44b2baefcb4895d1a4190d7f` had a Process lifecycle integration failure at the new checker because it searched for a wrapped C call on one line. The checker-only correction is `a727f2e78a42fac20d2558dee63ac78673551c79`; its exact-head Actions had not appeared at the time of this firewall slice. Do not claim it PASS without rerun.
- Source SHA `692c260...` also passed Process inventory #44948258854, USB modem #44948258515, Firewall Startup #44948258642, WAN route #44948258490 and AI handoff #44948258596; these are parent SHA evidence only. XY-C1 #44948258645 and NVRAM audit #44948258687 were in progress at last poll.
- The current firewall/VPN slice requires exact-head Firewall Startup validation (GNU89/ASan/UBSan helper test plus source contract), and XY-C1 Build/TRX/artifact on one SHA for L3. No local build/test or hardware L4 claim is made.
## Exact first action for the next GPT

1. Fetch exact current main and inspect this commit's Firewall Startup run. Require its GNU89/ASan/UBSan helper regression and inline VPN/NAT source contract to pass; inspect failure logs and repair atomically. Track XY-C1 through Build, TRX and artifact upload; do not attribute parent evidence to this SHA.
2. Re-query Process lifecycle for the whitespace-normalized XHCI checker on its own SHA `a727f2e78a42fac20d2558dee63ac78673551c79`; it is a checker-only descendant of the XHCI source fix.
3. Continue reviewing firewall filter selectors (remote-admin/FTP/HTTP/UDPXY ports and enable flags) where unchecked NVRAM getters might turn backend faults into altered policy. Confirm call-site ranges and semantics before widening scope.
## Workflow and preservation rules

- Develop only on `main`. Each logical slice is one complete Git tree, one commit and one non-forced expected-SHA update. Never force-push, amend history or write a multi-file slice as successive partial commits.
- Re-fetch main immediately before writing. If the expected SHA changed, stop and re-evaluate the new head; never blindly retry a rejected lease.
- Preserve GNU89, MIPS/uClibc, BusyBox, Linux 4.4, ABI, legacy NVRAM defaults, security and device compatibility. Never claim tests, builds, artifacts or hardware checks that were not actually performed.
- Root `dev_log.md` remains a newest-first 20–119-record rolling window; archive the oldest 100 complete records only when the next entry would reach 120. Preserve existing archive bytes and indexes. Update handoff/roadmap only when active task, module status or verification facts materially change.
