## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main, the engineering baseline, exact-head Actions, validation boundaries and the first next action before ending. Update this file only when those facts materially change; do not create a no-op commit or chase documentation-only SHAs. Each multi-file logical slice must land as one atomic main commit. Before context exhaustion, finish the active safe slice and leave an exact first action.

# Cross-session execution mandate

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- Parent main SHA before the current Process checker whitespace repair: `692c260ecf9d49ab44b2baefcb4895d1a4190d7f`. The source fix and initial checker are already in this parent; this checker-only correction normalizes whitespace and is committed atomically with the handoff/roadmap/log update.
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

- Source fix `692c260ecf9d49ab44b2baefcb4895d1a4190d7f` changes the MT7621 XHCI `usb3_disable` boot parameter from unchecked `nvram_get_int()` to `rc_boot_nvram_read_int_status()`, retains the legacy zero fallback, aggregates checked-read errors and still attempts the XHCI modules.
- Exact-source Process inventory (#44948258854), USB modem (#44948258515), Firewall Startup (#44948258642), WAN route (#44948258490) and AI handoff docs (#44948258596) all PASS. The Process lifecycle integration run #44948258619 FAILED only in its final source-contract checker: it searched for the checked-read call as a one-line literal, while production intentionally wraps its arguments across lines. The exact log shows all preceding C/shell policy tests and prior contracts passed; failure is at `scripts/check_process_lifecycle_integration.py:1932` (`assert usb3_read in usb_modules`).
- This checker-only repair normalizes whitespace on the extracted `load_usb_modules()` function before asserting the checked read and ordering. The source is unchanged. The exact-head NVRAM transaction audit #44948258687 and XY-C1 build #44948258645 were still in progress at the last poll; re-query them after this correction. The failed Process run does not yet validate the new contract. No local checkout/test execution is claimed.
## Verification boundary

- Source commit `692c260ecf9d49ab44b2baefcb4895d1a4190d7f`: Process lifecycle inventory #44948258854 PASS; USB modem #44948258515 PASS; Firewall Startup #44948258642 PASS; WAN route #44948258490 PASS; AI handoff #44948258596 PASS. Process lifecycle integration #44948258619 FAILS only because the new checker compared a wrapped C function call literally; see the failure above. The NVRAM transaction audit #44948258687 and XY-C1 #44948258645 were IN_PROGRESS at last poll. The exact-head vsftpd, WAN ARP, PID1 notification, DNS Forwarder, Adbyby, DDNSTO, detect_internet, and Crond regression runs also passed on the source SHA.
- This checker-only commit requires a new exact-head Process lifecycle run; all other source checks that already passed on `692c260...` remain evidence for that parent only and must not be promoted to current-head PASS. XY-C1 L3 requires Build, TRX verification and artifact upload all PASS on the same exact source SHA. Hardware L4 remains NOT VERIFIED.
## Exact first action for the next GPT

1. Fetch the live main SHA and query exact-head Actions. Confirm Process lifecycle reruns the whitespace-normalized USB/XHCI integration contract; inspect NVRAM audit #44948258687 and XY-C1 #44948258645 for the parent source SHA separately, and do not claim they validate this checker-only descendant.
2. Continue to the current Switch/PHY/MDIO audit only after closing the checker. Strict parser sources/tests are already present; investigate the outstanding Switch CLI workflow/checker state before touching production code. The workflow `switch-cli-stability.yml` supports `workflow_dispatch`.
3. Preserve immutable backup and all no-force/atomic main rules. No Session Rotator work or replay.
## Workflow and preservation rules

- Develop only on `main`. Each logical slice is one complete Git tree, one commit and one non-forced expected-SHA update. Never force-push, amend history or write a multi-file slice as successive partial commits.
- Re-fetch main immediately before writing. If the expected SHA changed, stop and re-evaluate the new head; never blindly retry a rejected lease.
- Preserve GNU89, MIPS/uClibc, BusyBox, Linux 4.4, ABI, legacy NVRAM defaults, security and device compatibility. Never claim tests, builds, artifacts or hardware checks that were not actually performed.
- Root `dev_log.md` remains a newest-first 20–119-record rolling window; archive the oldest 100 complete records only when the next entry would reach 120. Preserve existing archive bytes and indexes. Update handoff/roadmap only when active task, module status or verification facts materially change.
