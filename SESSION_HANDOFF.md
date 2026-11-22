# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository and complete an end-of-session accuracy checkpoint before ending. Record current source SHA, exact-SHA Actions, test/build boundaries, and the next precise task. If facts and first action remain accurate, explicitly conclude the check with no file change and no commit. Use one atomic commit for material changes only; never create a no-op commit. Before context exhaustion, leave the next GPT a precise first action and truthful verification status.

- Work only on `main`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update `main` only with `force:false` and a fetched expected SHA.
- Check Actions for the exact source SHA. PASS continues; FAIL requires diagnosis/correction; PENDING does not block independent work.
- Do not claim local tests, target builds, artifacts or hardware checks that were not performed.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; branch `main`.
- Current main source baseline: `33e87d4540698ef87226715fd3fa8e967243cb01` (checked modem WAN link-change reconnect NVRAM snapshot). PPP launch unit validation remains in `b3caebcbaba0924abcb83339d49a1968e237f042`; its XY-C1 run is still pending since the last observation.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. Legacy PR #1 is closed/unmerged.
- WAN PPP launch mode uses one checked NVRAM transaction for `pppoemwan_enable` and (APP_RPL2TP L2TP only) `wan_l2tpd`; same immutable flags drive lock selection, daemon stop/start, multi-WAN startup and rollback.
- WAN PPP source contracts, VPN lifecycle, WAN route, WAN DHCP, HTTPD, modem, firewall, IPv6 router-hosts and LAN DHCP checks have been reconciled with their checked APIs.
- `SESSION_HANDOFF.md` and `dev_log.md` are the only live root checkpoint/history records. Do not repeat startup audits or create routine history snapshots.

## Active task

The modem WAN ethernet-link callback previously used unchecked `nvram_get_int("modem_prio")`, silently suppressing recovery on a backend read error, and a legacy Boolean WISP query. Commit `33e87d4540698ef87226715fd3fa8e967243cb01` replaces those with one checked modem startup/readiness snapshot and `get_wan_wisp_active_status()`, adds a pure reconnect policy regression and enforces source ordering. The USB modem CI then failed in its embedded workflow source checker because it still asserted the old implementation; the checker-only fix in this next commit updates that contract to the checked snapshot/WISP/policy sequence. Product C regressions already compiled and passed on `33e87d4540698ef87226715fd3fa8e967243cb01`.

## Verification boundary

- Local tests were not run in this connector-only session.
- Parent `b443b54cf920195df1405e4a172ec0ed7f9b1f09`: USB modem #45031581333 PASS; firewall startup #45031581354 PASS; process inventory/lifecycle #45031581442 PASS; AI handoff #45031581408 PASS.
- Parent XY-C1 #45031581390 workflow SUCCESS: host C/Python regressions and source contracts passed, but Build XY-C1, TRX verification and artifact upload were intentionally skipped because that parent commit contained no firmware production input changes. This is not an L3 build.
- The WAN PPP selector source implementation and downstream contracts are committed in `d3805365ca95a17d5bd54736f29d5627580fa305` and `52a8c60d1e9aa324bb7f020fdeafda99a03319e9`. Their early XY host attempts stopped at stale source contracts before the cross-build. The next source-triggered XY run must complete Build, TRX and artifact upload on the same SHA before L3 can be claimed.
- Earlier exact runs: IPv6 service type #45031159848 PASS; WAN DHCP #45031431084 PASS; VPN lifecycle #45031330323 PASS; WAN route #45031330345 PASS; AI handoff #45031330331 PASS; NVRAM audit #45031084347 PASS. These validate their respective ancestor SHAs only.
- Exact SHA `b3caebcbaba0924abcb83339d49a1968e237f042`: WAN DHCP #45032049974 PASS; VPN #45032050106 PASS; WAN route #45032049989 PASS; HTTPD #45032050007 PASS; process #45032050136 PASS; NVRAM #45032049962 PASS. AI docs #45032049968 was corrected by `b0338e1be6b52f0f43cc644253565675fc243138` (AI handoff #45032198181 PASS). XY-C1 #45032049960 remained in_progress at last observation with production Build started; TRX/artifact have no completed evidence.
- Exact source SHA `33e87d4540698ef87226715fd3fa8e967243cb01`: USB modem #45032550398 FAILED only at embedded step `Verify WDM discovery, hotplug identity publication and NDIS matching`, an obsolete link-change source assertion; its GNU89/ASan/UBSan modem policy regression and all preceding C tests PASS. The checker is corrected in this documentation/checker follow-up. WAN teardown #45032550356, WAN DHCP #45032550362 and AI handoff #45032550424 PASS. Firewall #45032550381, XY-C1 #45032550417, NVRAM #45032550412, Process inventory #45032550504, Process lifecycle #45032550421 and HTTPD #45032550438 were still in progress at last observation.
- Hardware L4 remains unverified.

## Exact first action for the next GPT

1. Fetch live main and inspect exact-SHA USB modem, firewall, process, NVRAM, HTTPD and XY-C1 Actions for the corrected embedded checker. Fix any real failure; leave unrelated PENDING jobs running.
2. Recheck XY-C1 #45032049960 for the prior PPP change; production Build, TRX verification and artifact upload must all PASS on the same SHA for L3.
3. Continue the broad userland stability/security rotation outside the just-completed modem/PPP slices. Keep the backup immutable; hardware L4 remains unverified.
