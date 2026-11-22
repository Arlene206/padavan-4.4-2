## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main, the engineering baseline, exact-head Actions, validation boundaries and the first next action before ending. Update this file only when those facts materially change; do not create a no-op commit or chase documentation-only SHAs. Each multi-file logical slice must land as one atomic main commit. Before context exhaustion, finish the active safe slice and leave an exact first action.

# Cross-session execution mandate

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch is `main`.
- Parent main SHA for the deferred hotplug retry slice: `e26890edfef8dd47aa1bbb1d514a315933de2571`. This change makes failed temporary-NVRAM marker writes retryable without losing the original RC notification.
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

- USB modem-WAN identity is status-aware across `net_wan.c`, `hotplug_usb.c`, `services_usb.c`, `usb_modem.c`, `rc.c` and `detect_link.c`; parent exact-head modem identity and related WAN/Process checks passed.
- New stability defect: `handle_notifications()` removed the RC queue item and ignored `nvram_set_int_temp(..., 1)` results for printer add/remove, modem add/remove and mass-storage add. A temporary-NVRAM failure therefore discarded the only queued wakeup and incorrectly completed the RC notification. The existing `alarm(5)` only ran the deferred worker; it did not recreate the lost notification.
- `rc.c` now merges all five marker-write results into `action_result`, retains a retry bit for failed writes, and exposes a PID 1 retry worker. On SIGALRM, PID 1 recreates only fixed-name safe queue markers and re-signals itself; retry bits clear only after queue marker creation and wakeup succeed. The original incomplete marker remains until successful event handling.
- Added GNU89 policy regressions for all event mappings, unknown names, marker-write status recording and successful/failed wakeup completion. Extended the RC notification source contract. Corrected a stale USB storage checker assertion that incorrectly prohibited the single initial `storage_should_start = 0` initializer.
- Next: inspect exact-SHA RC notification, USB storage, AI handoff and XY-C1 Build/TRX/artifact results; then select a distinct whole-userland stability/security issue.

## Verification boundary

- Parent source SHA: `2f93266c59b9a10fec33589ada3e2e9c7b803b84`.
- Exact parent-SHA Actions: AI handoff #44972905746 PASS; Wireless radio #44972905459 PASS; Router mode NVRAM update #44972905764 PASS; VPN server #44972905495 PASS; vsftpd #44972905498 PASS; DNS Forwarder #44972905606 PASS; watchdog #44972905723 PASS; WAN gateway ARP #44972905645 PASS.
- PID 1 notification #44972905893 FAIL only because the two new policy regression functions had not actually been included/registered in the C test file. This commit inserts both function bodies and invokes them from `main()`.
- HTTPD auth/security #44972905619 failed on its existing exact-source expectation that `on_deferred_hotplug_dev()` remain under the original single `if (router_ready)` statement. This commit keeps retry before deferred dispatch but restores that source pattern.
- HTTPD daemon lifecycle #44972905858 failed on a stale explicit-UNKNOWN branch assertion. Startup correctly delegates this classification to `httpd_start_postcondition_requires_recovery(presence)`; checker now asserts that delegation and the policy's non-ABSENT condition.
- USB storage #44972905435 and XY-C1 #44972905721 were IN_PROGRESS at last poll; the storage workflow exercises the corrected output-initializer check. No same-SHA XY-C1 Build/TRX/artifact or L3 firmware pass has been observed. Hardware L4 remains NOT VERIFIED.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 remains closed/unmerged.

## Exact first action for the next GPT

1. Inspect exact-head PID 1 notification, HTTPD auth/security, HTTPD daemon lifecycle, USB storage, AI handoff, Process lifecycle and XY-C1 Build/TRX/artifact Actions for this commit. Fix every red result.
2. Verify the retry regression runs under GNU89/ASan/UBSan, that all five marker-write errors preserve incomplete RC status, and that PID 1 recreates safe markers before deferred NVRAM dispatch.
3. After verification, select a distinct P1/P2 whole-userland stability/security issue; do not reopen the modem identity audit or completed tasks.

## Workflow and preservation rules

- Develop only on `main`. Each logical slice is one complete Git tree, one commit and one non-forced expected-SHA update. Never force-push, amend history or write a multi-file slice as successive partial commits.
- Re-fetch main immediately before writing. If the expected SHA changed, stop and re-evaluate the new head; never blindly retry a rejected lease.
- Preserve GNU89, MIPS/uClibc, BusyBox, Linux 4.4, ABI, legacy NVRAM defaults, security and device compatibility. Never claim tests, builds, artifacts or hardware checks that were not actually performed.
- Root `dev_log.md` remains a newest-first 20–119-record rolling window; archive the oldest 100 complete records only when the next entry would reach 120. Preserve existing archive bytes and indexes. Update handoff/roadmap only when active task, module status or verification facts materially change.
