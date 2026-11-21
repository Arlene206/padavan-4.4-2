## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main, the engineering baseline, exact-head Actions, validation boundaries and the first next action before ending. Update this file only when those facts materially change; do not create a no-op commit or chase documentation-only SHAs. Each multi-file logical slice must land as one atomic main commit. Before context exhaustion, finish the active safe slice and leave an exact first action.

# Cross-session execution mandate

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- Parent main SHA before the current USB/XHCI boot-read slice: `5f61f0345e3bdddf7fa21219eb16a316801920ea`. The USB/XHCI source and integration-contract update below are included in one atomic commit; resolve the resulting exact SHA from live `refs/heads/main` during recovery.
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

- `load_usb_modules()` still read the MT7621 XHCI `usb3_disable` module parameter through unchecked `nvram_get_int()`. A backend ioctl/read/close error, malformed decimal or integer overflow could be collapsed into zero and silently change the USB 3.0 host-controller configuration.
- This slice switches the parameter to `rc_boot_nvram_read_int_status()`, preserving the legacy absent-key default of 0 and the exact integer values accepted previously. When the checked read fails, boot logs and aggregates its errno-style status, applies the old zero fallback, and still attempts `xhci_hcd` / `xhci_mtk` so an NVRAM fault does not unnecessarily remove USB host service.
- The Process integration source contract now pins the checked read, failure aggregation, fallback, and module-load ordering. The existing GNU89/ASan/UBSan `wan_nvram_status_policy.c` regression covers missing/default values, valid signed decimal values, malformed/overflow input and backend failures. No local checkout is available in this execution environment; exact-head Process and XY-C1 Actions provide new integration/build evidence.
## Verification boundary

- Parent main `5f61f0345e3bdddf7fa21219eb16a316801920ea` exact-SHA checks before this USB/XHCI slice: DNSMasq #44947511996, NVRAM #44947511851, HTTPD #44947511845, Samba #44947512135, WAN route #44947511905, MiniUPnP #44947511912, Process inventory #44947511867, WAN DHCP #44947512035, Process lifecycle #44947511882, Inadyn #44947511860 and AI handoff #44947512112 all PASS. XY-C1 #44947512124 had completed prerequisites and was still in Build XY-C1; TRX verification and artifact upload were not yet complete, so no L3 claim applies to that parent.
- The current USB/XHCI commit requires its own exact-head Process lifecycle, Process inventory, AI handoff and XY-C1 checks. For L3, require Build XY-C1, TRX verification and artifact upload to all pass on the same source SHA. Hardware L4 remains NOT VERIFIED.

- Local C policy regression: PASS under GNU89, `-Wall -Wextra -Werror`, ASan and UBSan in an isolated temporary workspace. It covers short writes plus EINTR, permanent error after a sent prefix, zero progress, impossible send count and invalid arguments. The full repository integration checker has not been run locally because no complete checkout is present; the exact-head Actions workflow is authoritative for integration.
- The WSD stream-send workflow #44946503639 passed on source 9a0ed45. Parent XY-C1 #44946503534 was still building (TRX/artifact pending); no L3 claim applies to that source SHA. The new Samba source checkpoint must obtain its own exact-head Samba/Process/XY-C1 checks; do not claim L3 until Build, TRX verification and artifact upload all pass on that exact SHA.
- WSD metadata source `edc9d594...` has exact HTTP, XML, Process inventory, NVRAM and XY-C1 Build/TRX/artifact PASS. The earlier WSD framing source `1c0ac607...` also has same-SHA XY-C1 Build/TRX/artifact PASS.
- Parent main source `f56fe999363645a737233a6721261f82ceb1cd2b`: NVRAM #44945662395 PASS and HTTPD security #44945662341 PASS; XY-C1 #44945662439 was still in progress (Build in progress, TRX/artifact pending) at last poll. Documentation parent `a293665472f8f5cef0ce80a928730b6c9ab66980`: AI handoff check #44945838044 PASS. These parent checks do not validate the new WSD source checkpoint.
- L3 is established only for the exact named source SHAs: WSD framing `1c0ac607`, WSD metadata/XML `edc9d594`, p910nd `23248f7`, MiniUPnP checked configuration `be4952e`, and earlier MiniUPnP redirect-description `4a5139f`. Hardware L4 is NOT VERIFIED.

## Exact first action for the next GPT

1. Resolve the live `refs/heads/main` SHA after this commit and inspect `/actions/runs?head_sha=<exact SHA>`. Close out Process lifecycle, Process inventory, AI handoff documentation and XY-C1 on that exact source; inspect failed job logs. For L3, require Build + TRX verification + artifact upload on one SHA. Treat pending runs as pending and move to independent work.
2. Once those checks are closed, continue the cross-userland audit with the Switch/PHY/MDIO module in `REFACTOR_ROADMAP.md`: strict numeric parsers and source-checker correction are present, but the row still records exact-head Switch CLI/XY-C1 verification as incomplete. Inspect current `switch_cli_parse_policy` tests and checker state before deciding whether a real defect remains; do not repeat parser work that has already passed.
3. Keep the explicitly preserved backup branch immutable. Do not create branches/PRs, force-push, amend history, run/modify Session Rotator or replay its old SHA.
## Workflow and preservation rules

- Develop only on `main`. Each logical slice is one complete Git tree, one commit and one non-forced expected-SHA update. Never force-push, amend history or write a multi-file slice as successive partial commits.
- Re-fetch main immediately before writing. If the expected SHA changed, stop and re-evaluate the new head; never blindly retry a rejected lease.
- Preserve GNU89, MIPS/uClibc, BusyBox, Linux 4.4, ABI, legacy NVRAM defaults, security and device compatibility. Never claim tests, builds, artifacts or hardware checks that were not actually performed.
- Root `dev_log.md` remains a newest-first 20–119-record rolling window; archive the oldest 100 complete records only when the next entry would reach 120. Preserve existing archive bytes and indexes. Update handoff/roadmap only when active task, module status or verification facts materially change.
