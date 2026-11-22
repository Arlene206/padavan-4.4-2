## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main, the engineering baseline, exact-head Actions, validation boundaries and the first next action before ending. Update this file only when those facts materially change; do not create a no-op commit or chase documentation-only SHAs. Each multi-file logical slice must land as one atomic main commit. Before context exhaustion, finish the active safe slice and leave an exact first action.

# Cross-session execution mandate

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; develop and validate on `main` only. Use one complete Git tree/commit and a non-forced expected-SHA ref update for each multi-file slice.
- Parent main SHA before this physical storage-unplug slice: `289c6fcb28553e78b3aa7c3f8a4e251d2544450c`. The source baseline for `trunk/user/rc/src/services_stor.c` is blob `ea52db4350e8211afe3d41d7216bccc182c04825`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only, do not merge, modify or delete. Main has intentionally diverged from the archival checkpoint.
- PR #1 remains closed/unmerged; its historical changes were already ported. Do not reopen it as the development path.
- On parent SHA `289c6fcb28553e78b3aa7c3f8a4e251d2544450c`, USB storage run #44993522407 was IN_PROGRESS at last inspection; XY-C1 #44993522386, process inventory #44993522352/#44993522416, USB modem #44993522402 and other triggered checks were also IN_PROGRESS. No exact-parent PASS is inferred from these statuses.
- GitHub Git Data and Actions REST endpoints are available through the connected integration. Host `cc` is available; there is no local full firmware checkout/MIPS toolchain.

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

- The product fix at parent source SHA `d414a788abc66e0bf5d8711b4d0584de505964c9` retains `usb_unplug_ms` until eject cleanup succeeds and the NVRAM marker acknowledgement succeeds. A process-local completion latch prevents duplicate eject cleanup when only marker acknowledgement is retried.
- Exact-parent USB storage run #44993693581 exposed a stale embedded workflow assertion requiring the old, unsafe clear-before-cleanup ordering. This checker-only slice updates it to assert checked read → completion gate → cleanup → marker acknowledgement → latch handling, and explicitly rejects early clearing.
- The GNU89/ASan/UBSan policy unit passed locally. The parent workflow stopped at the stale embedded assertion, so the storage checker and later tests remain unverified until the corrected workflow runs.

## Verification boundary

- Local standalone policy regression passed with GNU89, `-Wall -Wextra -Werror`, ASan and UBSan. It covers failed acknowledgement without repeating cleanup, cleanup failure remaining retryable, absent marker reset and error normalization.
- Parent SHA `d414a788abc66e0bf5d8711b4d0584de505964c9`: USB storage #44993693581 failed only at the embedded source contract that still mandated pre-cleanup marker clearing; preceding storage checks passed. Corrected workflow requires a fresh exact-head rerun. Other parent checks (Process, USB modem, firewall, NFS, Samba, vsftpd, HTTPD, IPv6 service type and AI docs) completed/success; XY-C1 #44993693537 and NVRAM #44993693562 were in progress at last inspection.
- No full MIPS firmware build was run locally. L3 requires Build XY-C1, TRX verification and artifact upload all PASS on one identical product-source SHA. Hardware L4 is NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-main-SHA USB storage Actions for this workflow-only correction. Confirm the whole storage caller graph, storage safe-remove checker, deferred cleanup/ack ASan+UBSan binary and all following regression steps pass.
2. Inspect XY-C1 on the same product-source SHA. Claim L3 only if Build, TRX verification and artifact upload all pass on that identical SHA.
3. If either run is pending, continue an independent userland stability/security fix; if failed, repair the first actual failing contract or regression without undoing marker retention.
4. Keep the frozen backup read-only; hardware L4 remains unverified.
