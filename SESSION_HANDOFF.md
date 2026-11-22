## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main, the engineering baseline, exact-head Actions, validation boundaries and the first next action before ending. Update this file only when those facts materially change; do not create a no-op commit or chase documentation-only SHAs. Each multi-file logical slice must land as one atomic main commit. Before context exhaustion, finish the active safe slice and leave an exact first action.

# Cross-session execution mandate

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch is `main`.
- Live `main` before THIS inline-contract correction: `de582a224e9bb20c3e6dbfe311d926f5096de19d`; exact WAN-route #44979536359 failed at the final route-result source contract because it required standalone `int route_snapshot_status;`, while production declares the four WAN route status locals together. Shared checker fix was already passing; XY-C1 #44979536417 status was in progress at last poll.
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

- Firewall checked boot gate, LAN DHCP rollback snapshot, HTTPD boot-policy contract, OpenVPN key/certificate failure propagation + digest fix, and FTP passive-port range policy are completed source slices.
- Samba ACL writer source SHA `5b93812bd53fff8a0a4bdb71e9e85b8a23543b9c` has same-SHA XY-C1 Build, TRX and artifact upload PASS (run #44978563487; artifact #14737425993). This is L3 only for `5b93812`; hardware L4 not verified.
- WAN route-up source fix `06604d0e6261efbf264fbe16c1eca5a6dc90cb13` now uses a coherent checked gateway/address/netmask transaction plus a separately checked gateway-only fallback. Exact de582a2 WAN-route #44979536359 passed the shared umbrella checker update and policy binaries, then failed at the final inline source contract because it required standalone `int route_snapshot_status;` while production uses a combined declaration. THIS commit corrects that matcher only; same-SHA reruns remain required.
- Exact `de582a2` AI docs #44979536437 PASS. XY-C1 #44979536417 was running at last poll. No L3 claim for the WAN change until Build, TRX and artifact all pass on one exact source SHA; hardware L4 NOT VERIFIED.

## Verification boundary

- SHA `5b93812bd53fff8a0a4bdb71e9e85b8a23543b9c`: XY-C1 Build, TRX verification and artifact upload all PASS (artifact #14737425993).
- SHA `06604d0e6261efbf264fbe16c1eca5a6dc90cb13`: Samba #44979351865, Process inventory #44979351880, WAN teardown #44979351818, miniupnpd #44979351879, USB modem #44979351908 and AI docs #44979354664 PASS. WAN route #44979351874 and XY-C1 #44979351755 stopped before firmware build due the old shared checker; NVRAM transaction boundary #44979351940 was still running at last poll.
- SHA `de582a224e9bb20c3e6dbfe311d926f5096de19d`: AI docs #44979536437 PASS; WAN route #44979536359 failed only at an inline combined-declaration matcher after shared checker and policy binaries passed; XY-C1 #44979536417 had not yet completed. THIS commit is workflow/docs only; exact reruns required.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 closed/unmerged. Hardware L4 NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head WAN route and XY-C1 Actions for current `main`; check matching `head_sha` and inspect any further failed contract assertions.
2. Require Build XY-C1, TRX verification and artifact upload all PASS on the same source SHA before claiming L3 for WAN route-up.
3. Continue independent stability/security work across RC userland while Actions run; the first full RC source scan found only legacy best-effort WAN counter telemetry via void wrappers, with all critical WAN state publication paths using status-returning helpers. Keep frozen backup untouched.

## Workflow and preservation rules

- Develop only on `main`. Each logical slice is one complete Git tree, one commit and one non-forced expected-SHA update. Never force-push, amend history or write a multi-file slice as successive partial commits.
- Re-fetch main immediately before writing. If the expected SHA changed, stop and re-evaluate the new head; never blindly retry a rejected lease.
- Preserve GNU89, MIPS/uClibc, BusyBox, Linux 4.4, ABI, legacy NVRAM defaults, security and device compatibility. Never claim tests, builds, artifacts or hardware checks that were not actually performed.
- Root `dev_log.md` remains a newest-first 20–119-record rolling window; archive the oldest 100 complete records only when the next entry would reach 120. Preserve existing archive bytes and indexes. Update handoff/roadmap only when active task, module status or verification facts materially change.
