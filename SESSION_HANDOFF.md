## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main, the engineering baseline, exact-head Actions, validation boundaries and the first next action before ending. Update this file only when those facts materially change; do not create a no-op commit or chase documentation-only SHAs. Each multi-file logical slice must land as one atomic main commit. Before context exhaustion, finish the active safe slice and leave an exact first action.

# Cross-session execution mandate

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch is `main`.
- Live `main` before THIS source correction: `4f5fa861c2702ead3c3fcbe6a871fad6639f6089`; product-source commit `feb5dfe55704a253cea979454cdad439d715596a` has confirmed XY-C1 compile failures in four Samba ACL helper calls and a stale HTTPD Samba checker assertion. THIS commit fixes both; fetch live `main` on the next recovery and verify the exact source SHA.
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
- Samba account-count and credential snapshots are in product-source baseline `feb5dfe55704a253cea979454cdad439d715596a`; account reads fail closed before passdb/config staging and the passdb writer consumes only copied credentials.
- Exact parent-SHA Samba #44977071563, Process inventory #44977071505, Process fail-closed #44977071916, NVRAM #44977071500, USB storage #44977071530, NFS #44977071428, Firewall #44977071404, USB modem #44977071499, vsftpd #44977071617 and AI docs #44977071668 PASS on `feb5dfe`. HTTPD #44977071470 failed at its source contract because it still searched for removed unchecked `nvram_safe_get()` patterns. XY-C1 #44977071471 failed compile because four ACL loops in `write_smb_conf_unlocked()` still called the changed account-safety helper without the router identity argument.
- THIS commit threads the preflight `router_username` snapshot through all four private-share ACL lists. The public `write_smb_conf()` path passes a null sentinel; the writer then performs a checked router-identity copy only for private-user modes and before opening the staged config. Both Samba and HTTPD source contracts are updated to assert the new call signature and read-before-open ordering.
- The workflow-trigger follow-up `4f5fa861c2702ead3c3fcbe6a871fad6639f6089` passed exact Samba #44978305350 and AI docs #44978305586; those runs validate the filter/docs change only, not the Samba product compile.
- No local MIPS toolchain or physical router; firmware L3/L4 NOT VERIFIED.

## Verification boundary

- Product baseline `feb5dfe55704a253cea979454cdad439d715596a`: Samba #44977071563 PASS, Process inventory #44977071505 PASS, Process fail-closed #44977071916 PASS, NVRAM #44977071500 PASS, USB storage #44977071530 PASS, NFS #44977071428 PASS, Firewall #44977071404 PASS, USB modem #44977071499 PASS, vsftpd #44977071617 PASS and AI docs #44977071668 PASS.
- Exact HTTPD #44977071470 FAIL was a stale source-contract substring lookup after checked-credential migration; prior compile/tests in that job passed through step 27. Exact XY-C1 #44977071471 FAIL was a real source compile failure: four too-few-arguments diagnostics at `services_stor.c` lines 1075, 1090, 1117 and 1139. Build failed, TRX/artifact skipped.
- Workflow-only commit `4f5fa861c2702ead3c3fcbe6a871fad6639f6089` passed Samba #44978305350 and AI docs #44978305586. THIS commit changes production source plus both Samba/HTTPD checker contracts; fresh exact-head runs are required.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`; no open PR was found. No same-product-SHA XY-C1 Build/TRX/artifact completion or L3 claim. Hardware L4 NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect the exact push runs on current `main` for Samba share stability, HTTPD security, XY-C1, Process inventory/fail-closed and NVRAM. Require each run's `head_sha` to equal the current main SHA; fix any remaining failure before treating the checked-credential slice as verified.
2. Confirm Build XY-C1, TRX verification and artifact upload all PASS on the same exact product-source SHA before L3; do not infer target validation from host-only PASS.
3. While exact-head Actions are pending, continue an independent P1/P2 stability/security item from `REFACTOR_ROADMAP.md`; keep the frozen backup untouched.

## Workflow and preservation rules

- Develop only on `main`. Each logical slice is one complete Git tree, one commit and one non-forced expected-SHA update. Never force-push, amend history or write a multi-file slice as successive partial commits.
- Re-fetch main immediately before writing. If the expected SHA changed, stop and re-evaluate the new head; never blindly retry a rejected lease.
- Preserve GNU89, MIPS/uClibc, BusyBox, Linux 4.4, ABI, legacy NVRAM defaults, security and device compatibility. Never claim tests, builds, artifacts or hardware checks that were not actually performed.
- Root `dev_log.md` remains a newest-first 20–119-record rolling window; archive the oldest 100 complete records only when the next entry would reach 120. Preserve existing archive bytes and indexes. Update handoff/roadmap only when active task, module status or verification facts materially change.
