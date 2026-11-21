## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main, the engineering baseline, exact-head Actions, validation boundaries and the first next action before ending. Update this file only when those facts materially change; do not create a no-op commit or chase documentation-only SHAs. Each multi-file logical slice must land as one atomic main commit. Before context exhaustion, finish the active safe slice and leave an exact first action.

# Cross-session execution mandate

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- Main source SHA at this checkpoint: `1c0ac607c66179ab58c3ae04c9cda2691c4dd91e` (the handoff/log/roadmap update is committed as a documentation-only child).
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`. PR #1 is closed/unmerged; no feature branches/PRs, force-push or history rewriting. Do not touch GPT Session Rotator or replay SHA `9d5a5cb`.

## Completed source slices

- Shared VPN boolean policy: both PPP VPN and OpenVPN up callbacks use checked `vpn_client_nvram_read_bool_status()`. The follow-up commit `53e9c553994afe3917c3451dee9d73f116be1024` removes an accidentally retained old argument tail from `on_client_ifup()`, updates the checker to the shared helper, and corrects the part-023 archive index byte count from 109434 to the verified 109696 UTF-8 bytes (archive blob SHA `d896a53f5bd56915dbabb5ec4987a2dc069d8388` remains unchanged).
- OpenVPN server callbacks prefer event-supplied `dev`, use checked topology fallback only when absent, and preserve disconnect hook cleanup. Client DNS policy errors preserve last-good DNS/domain state while mandatory state/hook work continues.
- This commit hardens exported client profiles: activate `remote-cert-tls server` to match the generated server certificate's `serverAuth` EKU, and use a validated WAN IPv4 endpoint only as a final fallback for dual-stack modes 4/5 after DDNS and both IPv6 sources fail. IPv6-only modes remain unchanged. The VPN lifecycle checker cross-validates the directive against `openvpn-cert.sh` and checks profile option mapping/fallback order.

## Active task

- Current main before this documentation-only checkpoint: `f56fe999363645a737233a6721261f82ceb1cd2b`. Frozen backup branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (do not update it).
- WSD/HTTP request parser: source `1c0ac607c66179ab58c3ae04c9cda2691c4dd91e` fixes the NULL-CRLF dereference, fragmented-header cursor invalidation, short body reads and strict Content-Length bounds; `17ddd162a9d728f51df93d20beb511da5263fb95` adds the 413 status. HTTP framing rejects all Transfer-Encoding headers rather than accidentally accepting unsupported chunked or CL+TE request framing.
- WSD XML response hardening: `1dfdc01ad7a3c2338ec3e2232c62c658662cdd05` escapes request-derived RelatesTo; `edc9d59456fe40d5c991b3a07507515ddf2c5eaf` extends the same XML-text escaping to all seven configured GetResponse metadata fields (vendor, vendor URL, model, model URL, presentation URL, NetBIOS name and workgroup), with fail-closed cleanup, tests and source-contract checks.
- WSD XML parser hardening: `f9ea436523324a555fed0bd87c445246c49e881c` fixes NULL/undefined pointer and allocation-failure paths in tag/request parsing. LLMNR question parsing is bounded, rejects malformed/truncated labels and ambiguous embedded NUL/dot bytes; hostname acquisition is forcibly NUL-terminated. Regression fixture bounds/checker corrections followed the first test failure; re-check the newest LLMNR exact-head action before treating that slice green.
- p910nd bidirectional mode ignores SIGPIPE so a disconnected print client produces EPIPE for existing recovery rather than killing the daemon. Source `23248f7df1575885e991fe518629bb974fc05c35`; its host socketpair regression, p910nd, Process inventory, Process lifecycle and AI checks passed on earlier exact heads, but same-SHA XY-C1 L3 was still not verified at last recorded poll.
- MiniUPnP checked configuration preflight (protocol, secure mode, HTTP port, cleanup timers, mapping range bounds) remains on source `be4952e90111ddea8f91684f6c3fe5ecf25be785`. Only the earlier redirect-description source `4a5139f484943abe3a5ac9cf0f6d56bc70067a56` has recorded same-SHA XY-C1 Build, TRX and artifact upload PASS.

## Verification boundary

- HTTPD security contract run #44945277626 failed inside the inline Python checker, not the router code: the checker used a byte offset computed from whitespace-compacted text against the original un-compacted source string. The assertion then raised `ValueError: substring not found`. Commit `c9005f49bb7edf6ac9efad6bfa51b82f6b902cc4` fixes the offset by computing both the search and offset in `full_restart_routes` (same representation). Exact-head rerun still needs inspection.
- WSD configured-metadata XML source commit `edc9d59456fe40d5c991b3a07507515ddf2c5eaf`: source, unit-test and checker changes committed; verify the exact-head XML regression before marking green. Latest retrieved Actions for that SHA: #44945383982 wsdd2 WSD XML reflection regression: completed/success; #44945383956 wsdd2 bounded HTTP framing regression: completed/success; #44945383886 Process lifecycle inventory - edc9d59456fe40d5c991b3a07507515ddf2c5eaf: completed/success; #44945383870 XY-C1 Firmware Validation: in_progress; #44945383856 NVRAM transaction boundary audit: completed/success.
- HTTPD checker-only source commit `c9005f49bb7edf6ac9efad6bfa51b82f6b902cc4`: checker offset fix committed; exact-head workflow state: #44945615079 AI handoff documentation contract: completed/success; #44945615000 HTTPD Authentication and Security Regression: completed/cancelled.
- Current main exact-head Actions: #44945662490 WAN gateway ARP probe socket stability regression: completed/success; #44945662441 Wireless radio state regression: completed/success; #44945662439 XY-C1 Firmware Validation: in_progress; #44945662435 ZeroTier lifecycle stability regression: completed/success; #44945662429 DNSMasq lifecycle stability regression: completed/success; #44945662424 USB modem WDM/NDIS physical identity stability: completed/success; #44945662411 WAN DHCP callback state stability regression: completed/success; #44945662408 Firewall Startup Stability Regression: completed/success; #44945662406 VPN server lifecycle stability regression: completed/success; #44945662405 watchdog signal lifecycle stability regression: completed/success; #44945662400 detect_link signal lifecycle stability regression: completed/success; #44945662396 miniupnpd atomic configuration publication regression: completed/success.
- L3 is currently established only for MiniUPnP redirect-description SHA `4a5139f484943abe3a5ac9cf0f6d56bc70067a56` (#44940761694 Build, TRX and artifact PASS). Do not transfer that L3 claim to p910nd, later MiniUPnP configuration, WSD/LLMNR, or later WSD XML SHAs. Hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect latest main and exact-head Actions. In particular, verify the WSD configured-metadata XML regression on `edc9d59456fe40d5c991b3a07507515ddf2c5eaf`, the corrected HTTPD security workflow after `c9005f49bb7edf6ac9efad6bfa51b82f6b902cc4`, and the latest LLMNR fixture/checker run. Fix only first-causal failures.
2. For each pending source SHA, check its own XY-C1 Build, TRX verification and artifact upload; claim L3 only when all three are PASS on the same source SHA.
3. Continue with another concrete userland reliability/security defect; do not repeat completed audits or reopen green slices without same-SHA regression evidence.

## Workflow and preservation rules

- Develop only on `main`. Each logical slice is one complete Git tree, one commit and one non-forced expected-SHA update. Never force-push, amend history, create branches or PRs.
- Re-fetch main immediately before writes. Only exact-head push Actions count. Preserve GNU89, MIPS/uClibc, BusyBox, Linux 4.4, ABI, legacy NVRAM defaults, security and device compatibility. Never claim tests/builds not run.
- Root `dev_log.md` remains a newest-first 20–119-record rolling window; archive the 100 oldest complete records when the next entry would reach 120. Preserve existing archive bytes and index integrity.

