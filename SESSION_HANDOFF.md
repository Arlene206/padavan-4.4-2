## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main, the engineering baseline, exact-head Actions, validation boundaries and the first next action before ending. Update this file only when those facts materially change; do not create a no-op commit or chase documentation-only SHAs. Each multi-file logical slice must land as one atomic main commit. Before context exhaustion, finish the active safe slice and leave an exact first action.

# Cross-session execution mandate

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch is `main`.
- Live `main` at this recovery: `feb5dfe55704a253cea979454cdad439d715596a`; the active product-source baseline is the Samba checked-credential snapshot in that commit. THIS follow-up adds the missing Samba workflow trigger for a regression source file and updates the active checkpoint; fetch live `main` on the next recovery instead of chasing this workflow/documentation-only descendant.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains preserved at `72f4f157a084f8184b9597ca5bf334b8080e5807`. PR #1 remains closed/unmerged. Do not touch backup or revive PR #1.
- GitHub Git data API remains the editing path; full local firmware checkout/toolchain is unavailable here.

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

- MAC-filter hardening complete: supported WebUI modes are enforced, indexed MAC/date/time values use checked NVRAM reads, address tokens are validated, and absent schedule keys preserve legacy defaults. Exact source commit `8dd8a5bcb515b752dc9c4fd07e11440e929d5c49` passed HTTPD, NVRAM, Firewall Startup, WAN DHCP, WAN route, FULLCONENAT, inventory and AI docs; exact XY-C1 #44955441811 passed Build/TRX/artifact (L3 only for that SHA).
- Static-route hardening complete through source commit `182cfee52473b9f072816c6e5f67ef727945143e`: fixed WebUI 64-row Add off-by-one, checked NVRAM enable/count/entry reads, fail-closed handling, strict metric and gateway validation, preserved the historical invalid-gateway fallback, and retained compatibility up to the generic 128-row storage limit. Maximum serialized snapshot is 7,552 bytes < 8,192-byte buffer. Source commits `1ace1932a59c91e5e313ec53fb1a8d2560d90717`, `06e243f66c34949cf77603479e6135c925422f2c`, `182cfee52473b9f072816c6e5f67ef727945143e`.
- Static-route Delete checker now matches the escaped quotes emitted in the JavaScript string (`37d0b7b96ee8dcd0b654c4e2e514d36fb4e6fdb7`); storage IPTV dispatch check ignores formatting-only whitespace (`f3a1cdda6e36c3014809f276c31c84a119016158`). WAN-route result-propagation contract was corrected in `923f8c64dc7598f92ce454a0fc2f71beb8f5a227`; exact run #44956642473 passed.
## Active task

- Firewall checked boot gate, LAN DHCP rollback snapshot, HTTPD boot-policy contract, OpenVPN key/certificate failure propagation + digest fix, and FTP passive-port range policy are completed source slices with exact workflow results in the dev log.
- Samba `acc_num` status guard is committed in `45d052982bd51efcbdc47e5c818544f95d4d5689`; checker correction `909a0095de86a88f7dfb8343923e641f466ac923`. Exact Samba #44976705529 PASS and AI docs #44976705516 PASS on `909a009`.
- Product-source baseline `feb5dfe55704a253cea979454cdad439d715596a` snapshots `http_username`, `acc_usernameN` and accepted `acc_passwordN` via copied, status-bearing NVRAM reads before process inventory, cleanup, config or passdb staging. Read failure, truncation, unsafe password or duplicate identity aborts and preserves last-good state; passdb generation consumes only validated snapshots and wipes credential buffers on locked exits.
- THIS workflow-contract correction adds `trunk/user/rc/src/tests/wan_nvram_status_policy.c` to both Samba `push` and `pull_request` path filters. The Samba workflow's Python contract reads that test and asserts its EIO/ENAMETOOLONG coverage, but neither event previously triggered on a change to that file. Product source is unchanged in THIS follow-up.
- No local MIPS toolchain or physical router; firmware L3/L4 NOT VERIFIED.

## Verification boundary

- Product-source baseline: `feb5dfe55704a253cea979454cdad439d715596a`; THIS follow-up is workflow/documentation only.
- Exact Samba #44976705529 PASS and AI docs #44976705516 PASS on parent `909a009`, validating the earlier account-count guard only. The checked-credential product change still needs exact-head Samba, Process inventory, NVRAM and XY-C1 validation.
- The GitHub combined-status connector returned no status rows for `feb5dfe`; its available workflow-runs helper is PR-filtered, so push-triggered exact-head Actions could not be enumerated from this session. Do not treat this as PASS or as evidence that no run exists; inspect GitHub Actions for THIS commit SHA on the next recovery.
- No same-product-SHA XY-C1 Build/TRX/artifact completion is recorded; no L3 claim. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`; no open PR was found. Hardware L4 NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect the push-triggered Samba share stability and AI handoff docs runs for the current `main` SHA (which includes THIS workflow filter correction). Match each run's `head_sha` exactly; diagnose/fix any failure before declaring the checked-credential slice validated.
2. While Actions are pending, continue the next highest-priority independent stability/security item from `REFACTOR_ROADMAP.md`; do not repeat completed audits or wait idly.
3. Require Build XY-C1, TRX verification and artifact upload all PASS on one exact product-source SHA before claiming L3; keep frozen backup untouched.

## Workflow and preservation rules

- Develop only on `main`. Each logical slice is one complete Git tree, one commit and one non-forced expected-SHA update. Never force-push, amend history or write a multi-file slice as successive partial commits.
- Re-fetch main immediately before writing. If the expected SHA changed, stop and re-evaluate the new head; never blindly retry a rejected lease.
- Preserve GNU89, MIPS/uClibc, BusyBox, Linux 4.4, ABI, legacy NVRAM defaults, security and device compatibility. Never claim tests, builds, artifacts or hardware checks that were not actually performed.
- Root `dev_log.md` remains a newest-first 20–119-record rolling window; archive the oldest 100 complete records only when the next entry would reach 120. Preserve existing archive bytes and indexes. Update handoff/roadmap only when active task, module status or verification facts materially change.
