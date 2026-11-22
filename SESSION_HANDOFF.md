## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main, the engineering baseline, exact-head Actions, validation boundaries and the first next action before ending. Update this file only when those facts materially change; do not create a no-op commit or chase documentation-only SHAs. Each multi-file logical slice must land as one atomic main commit. Before context exhaustion, finish the active safe slice and leave an exact first action.

# Cross-session execution mandate

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch is `main`.
- Live `main` before THIS WAN-up route snapshot fix: `9dfdae39490d7486f5dd33ff2b3ad68f23f1a0b0`; Samba stability #44979109639 and AI docs #44979110109 PASS on this exact SHA. The prior exact-head Samba contract mismatch was fixed; the product-source XY-C1 run #44978563487 for `5b93812` remained in progress at last poll.
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
- Samba account-count and credential snapshots are in product-source baseline `feb5dfe55704a253cea979454cdad439d715596a`. ACL writer fix `5b93812bd53fff8a0a4bdb71e9e85b8a23543b9c` passes checked router identity through all private-share ACL lists; exact Samba #44979109639 and AI docs #44979110109 PASS on the workflow/docs descendant `9dfdae3`.
- P1 WAN NVRAM audit found a real WAN-up route-consistency defect: `wan_up_internal()` used three independent unchecked `get_wan_unit_value()` calls for gateway/address/netmask while teardown already used the transaction-backed snapshot. THIS commit changes WAN-up to use the checked transactional tuple, keeps only a checked single-gateway fallback after tuple failure, and adds transaction-close regression plus workflow source-contract assertions.
- Product-source Samba/ACL correction SHA `5b93812bd53fff8a0a4bdb71e9e85b8a23543b9c` has exact firmware-independent PASS: VSFTPD #44978563499, AI docs #44978563541, NFS #44978563620, USB modem #44978563518, Firewall #44978563485, Process inventory #44978563472, USB storage #44978563480, Process fail-closed #44978563479 and NVRAM #44978563483. XY-C1 #44978563487 remained in progress at last poll; recheck.
- No local MIPS toolchain or physical router; L3/L4 NOT VERIFIED.

## Verification boundary

- Product-source baseline `feb5dfe55704a253cea979454cdad439d715596a`: Samba #44977071563 PASS, Process inventory #44977071505 PASS, Process fail-closed #44977071916 PASS, NVRAM #44977071500 PASS, USB storage #44977071530 PASS, NFS #44977071428 PASS, Firewall #44977071404 PASS, USB modem #44977071499 PASS, VSFTPD #44977071617 PASS and AI docs #44977071668 PASS.
- Samba ACL product-source correction `5b93812bd53fff8a0a4bdb71e9e85b8a23543b9c`: firmware-independent VSFTPD/NFS/firewall/USB modem/USB storage/Process/NVRAM/AI Docs checks PASS. Exact Samba and HTTPD initially failed stale source contracts; contract fixes in `32394e6` and `9dfdae3` are now validated by Samba #44979109639 PASS and HTTPD #44978784728 PASS, AI Docs #44979110109 PASS. XY-C1 #44978563487 still had status IN_PROGRESS at last poll.
- WAN route-up transactional snapshot fix in THIS commit: C regression now tests coherent 3-key snapshot read errors and transaction-close failure; workflow contract pins checked snapshot ordering, fail-closed address/netmask, checked gateway fallback, and absence of unchecked route-tuple getters. Exact WAN-route Actions pending. No same-product-SHA XY-C1 Build/TRX/artifact completion or L3 claim. Hardware L4 NOT VERIFIED.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 closed/unmerged.

## Exact first action for the next GPT

1. Inspect exact push Actions on current `main` for WAN route-status and AI docs; match each `head_sha` to main. Fix a source or checker failure without weakening the checked tuple/fallback contract.
2. Recheck XY-C1 #44978563487 on product source `5b93812`; it is independent of this WAN follow-up. Require Build, TRX and artifact upload on one exact source SHA before L3.
3. Continue P1 Init/PID1 reliability auditing from `REFACTOR_ROADMAP.md` while Actions run; keep the frozen backup untouched.

## Workflow and preservation rules

- Develop only on `main`. Each logical slice is one complete Git tree, one commit and one non-forced expected-SHA update. Never force-push, amend history or write a multi-file slice as successive partial commits.
- Re-fetch main immediately before writing. If the expected SHA changed, stop and re-evaluate the new head; never blindly retry a rejected lease.
- Preserve GNU89, MIPS/uClibc, BusyBox, Linux 4.4, ABI, legacy NVRAM defaults, security and device compatibility. Never claim tests, builds, artifacts or hardware checks that were not actually performed.
- Root `dev_log.md` remains a newest-first 20–119-record rolling window; archive the oldest 100 complete records only when the next entry would reach 120. Preserve existing archive bytes and indexes. Update handoff/roadmap only when active task, module status or verification facts materially change.
