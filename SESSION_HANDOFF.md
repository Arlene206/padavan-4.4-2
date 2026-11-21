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

- The IPv4/IPv6 filter listener preflight and NAT/DMZ hardening are in the current main ancestry. The next firewall review found one remaining policy ambiguity in Virtual Server (VTS) mappings: `proto_conv("vts_proto_x", i)` used the legacy safe getter and defaulted unknown/empty strings to TCP. An NVRAM backend read/close failure is also surfaced as an empty string by the legacy API, so a transient fault could synthesize a TCP DNAT/ACCEPT mapping for a rule that was intended to be UDP or another protocol.
- This slice adds a tested classifier for `nvram_get_checked()` results and updates `proto_conv()` to distinguish absent from backend failure. Missing or unrecognized values preserve the historical TCP fallback; backend/API/close failures return NULL. Both VTS filter and NAT loops now log and skip only the affected mapping instead of publishing the wrong TCP rule.
- The Firewall Startup workflow now asserts the checked protocol reader, both fail-closed call sites, and the C regression that distinguishes absent-key defaults from backend failures.
## Verification boundary

- Current main parent before VTS protocol-read hardening: `79cfb79099f40e3b67b6c23a520c623c422c2d16` (`fix(firewall): bound virtual server rule count`). The IPv4/IPv6 filter listener source must be considered unverified until this exact descendant passes Firewall Startup and XY-C1 Build/TRX/artifact checks.
- Earlier exact-head listener parent `8a7cf932879931c7df25708a9290dcc8eba2ed89` had Firewall Startup parser failure due the extra IPv6 HTTP block opener; fixed in descendant `e1e48877d521ef9b4559ade13568ff4d97de59ad`, which the current main ancestry contains. The immediate parent and current workflows must be polled from live Actions; no L3/hardware L4 claim is made.
- No local C test/build execution is claimed. The new NVRAM status unit regression runs only through exact-head Firewall Startup CI.
## Exact first action for the next GPT

1. Query exact-head Firewall Startup and XY-C1 for the VTS protocol hardening. Require the GNU89/Werror/ASan/UBSan helper regression and the two VTS checked-read assertions to pass; inspect the first failing source/test line rather than weakening the assertion.
2. Track XY-C1 Build, TRX verification and artifact upload on the same exact SHA. Re-query the current parent's Actions only as parent evidence.
3. Continue the firewall audit of VTS enable/count/port/address parsing and `include_vts_filter`/`include_vts_nat` semantics. Preserve legacy absent defaults; ensure backend errors or malformed WAN-facing fields cannot synthesize a more permissive mapping.
## Workflow and preservation rules

- Develop only on `main`. Each logical slice is one complete Git tree, one commit and one non-forced expected-SHA update. Never force-push, amend history or write a multi-file slice as successive partial commits.
- Re-fetch main immediately before writing. If the expected SHA changed, stop and re-evaluate the new head; never blindly retry a rejected lease.
- Preserve GNU89, MIPS/uClibc, BusyBox, Linux 4.4, ABI, legacy NVRAM defaults, security and device compatibility. Never claim tests, builds, artifacts or hardware checks that were not actually performed.
- Root `dev_log.md` remains a newest-first 20–119-record rolling window; archive the oldest 100 complete records only when the next entry would reach 120. Preserve existing archive bytes and indexes. Update handoff/roadmap only when active task, module status or verification facts materially change.
