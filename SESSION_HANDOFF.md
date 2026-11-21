## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main, the engineering baseline, exact-head Actions, validation boundaries and the first next action before ending. Update this file only when those facts materially change; do not create a no-op commit or chase documentation-only SHAs. Each multi-file logical slice must land as one atomic main commit. Before context exhaustion, finish the active safe slice and leave an exact first action.

# Cross-session execution mandate

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- Parent main SHA for this Samba local-master NVRAM source/test checkpoint: `9a0ed45f2be0f67807b53c0b8e6d33723436b0ac`. The new source, regression, workflow and handoff changes are included in one atomic commit. Resolve its exact SHA from live `refs/heads/main` after the commit is installed.
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

- Fix the unchecked Samba local-master configuration input in `trunk/user/rc/src/services_ex.c`. `write_smb_conf_header()` formerly called `nvram_get_int("st_samba_lmb")`, silently treating NVRAM backend/close errors as zero and accepting malformed values outside 0..2.
- This checkpoint uses `nvram_get_checked()`, `rc_wan_nvram_read_int_status()`, and the new `samba_local_master_value_status()` policy to preserve missing-key default 0 while rejecting backend errors and out-of-range values before writing staged smb.conf content. Adds GNU89 boundary/error tests and Samba inline source-contract assertions. Valid 0/1/2 behavior is unchanged.

## Verification boundary

- Local C policy regression: PASS under GNU89, `-Wall -Wextra -Werror`, ASan and UBSan in an isolated temporary workspace. It covers short writes plus EINTR, permanent error after a sent prefix, zero progress, impossible send count and invalid arguments. The full repository integration checker has not been run locally because no complete checkout is present; the exact-head Actions workflow is authoritative for integration.
- The WSD stream-send workflow #44946503639 passed on source 9a0ed45. Parent XY-C1 #44946503534 was still building (TRX/artifact pending); no L3 claim applies to that source SHA. The new Samba source checkpoint must obtain its own exact-head Samba/Process/XY-C1 checks; do not claim L3 until Build, TRX verification and artifact upload all pass on that exact SHA.
- WSD metadata source `edc9d594...` has exact HTTP, XML, Process inventory, NVRAM and XY-C1 Build/TRX/artifact PASS. The earlier WSD framing source `1c0ac607...` also has same-SHA XY-C1 Build/TRX/artifact PASS.
- Parent main source `f56fe999363645a737233a6721261f82ceb1cd2b`: NVRAM #44945662395 PASS and HTTPD security #44945662341 PASS; XY-C1 #44945662439 was still in progress (Build in progress, TRX/artifact pending) at last poll. Documentation parent `a293665472f8f5cef0ce80a928730b6c9ab66980`: AI handoff check #44945838044 PASS. These parent checks do not validate the new WSD source checkpoint.
- L3 is established only for the exact named source SHAs: WSD framing `1c0ac607`, WSD metadata/XML `edc9d594`, p910nd `23248f7`, MiniUPnP checked configuration `be4952e`, and earlier MiniUPnP redirect-description `4a5139f`. Hardware L4 is NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `refs/heads/main`, then query `/actions/runs?head_sha=<new exact SHA>`. Inspect the new WSD stream-send workflow and XY-C1 run; require send regression PASS, and for L3 require Build XY-C1 + TRX verification + artifact upload all PASS on that same SHA. Diagnose the first causal failure and repair atomically; do not wait idle for pending runs.
2. Recheck WSD metadata XY-C1 #44945383870 only if its state changes; it was PASS at this checkpoint. Parent-source XY-C1 #44945662439 was still building and must be closed out separately, without attributing it to the new source SHA.
3. Continue to the next concrete userland security/stability item. The roadmap currently calls out MiniUPnP SOAP input validation: reject internal port 0 for AddPortMapping/AddAnyPortMapping while retaining permitted external-port zero semantics. Inspect implementation and caller expectations before editing.
4. Keep backup branch immutable. Do not create branches/PRs, force-push, run/modify Session Rotator or replay its old SHA.

## Workflow and preservation rules

- Develop only on `main`. Each logical slice is one complete Git tree, one commit and one non-forced expected-SHA update. Never force-push, amend history or write a multi-file slice as successive partial commits.
- Re-fetch main immediately before writing. If the expected SHA changed, stop and re-evaluate the new head; never blindly retry a rejected lease.
- Preserve GNU89, MIPS/uClibc, BusyBox, Linux 4.4, ABI, legacy NVRAM defaults, security and device compatibility. Never claim tests, builds, artifacts or hardware checks that were not actually performed.
- Root `dev_log.md` remains a newest-first 20–119-record rolling window; archive the oldest 100 complete records only when the next entry would reach 120. Preserve existing archive bytes and indexes. Update handoff/roadmap only when active task, module status or verification facts materially change.
