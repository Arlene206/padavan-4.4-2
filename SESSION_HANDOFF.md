# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository and complete an end-of-session accuracy checkpoint before ending. Record current source SHA, exact-SHA Actions, test/build boundaries, and the next precise task. If facts and first action remain accurate, explicitly conclude the check with no file change and no commit. Use one atomic commit for material changes only; never create a no-op commit. Before context exhaustion, leave the next GPT a precise first action and truthful verification status.

- Work only on `main`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one commit; update `main` only with `force:false` and a fetched expected SHA.
- Check Actions for the exact source SHA. PASS continues; FAIL requires diagnosis/correction; PENDING does not block independent work.
- Do not claim local tests, target builds, artifacts or hardware checks that were not performed.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; branch `main`.
- Parent of the current PPP input-validation change: `b443b54cf920195df1405e4a172ec0ed7f9b1f09`; fetch live main after this atomic commit for the current SHA.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. Legacy PR #1 is closed/unmerged.
- WAN PPP launch mode uses one checked NVRAM transaction for `pppoemwan_enable` and (APP_RPL2TP L2TP only) `wan_l2tpd`; same immutable flags drive lock selection, daemon stop/start, multi-WAN startup and rollback.
- WAN PPP source contracts, VPN lifecycle, WAN route, WAN DHCP, HTTPD, modem, firewall, IPv6 router-hosts and LAN DHCP checks have been reconciled with their checked APIs.
- `SESSION_HANDOFF.md` and `dev_log.md` are the only live root checkpoint/history records. Do not repeat startup audits or create routine history snapshots.

## Active task

The final PPP review found the public `launch_wan_pppd()` wrapper returned `-1` for a negative WAN unit without setting `errno`, unlike the locked worker which returns EINVAL. It now validates the unit through `ppp_wan_launch_unit_status()`, sets EINVAL and aborts before NVRAM preflight. Added policy regression and source-contract coverage. This commit modifies `net_ppp.c` to trigger a real XY-C1 production build of the recently hardened PPP launch path.

## Verification boundary

- Local tests were not run in this connector-only session.
- Parent `b443b54cf920195df1405e4a172ec0ed7f9b1f09`: USB modem #45031581333 PASS; firewall startup #45031581354 PASS; process inventory/lifecycle #45031581442 PASS; AI handoff #45031581408 PASS.
- Parent XY-C1 #45031581390 workflow SUCCESS: host C/Python regressions and source contracts passed, but Build XY-C1, TRX verification and artifact upload were intentionally skipped because that parent commit contained no firmware production input changes. This is not an L3 build.
- The WAN PPP selector source implementation and downstream contracts are committed in `d3805365ca95a17d5bd54736f29d5627580fa305` and `52a8c60d1e9aa324bb7f020fdeafda99a03319e9`. Their early XY host attempts stopped at stale source contracts before the cross-build. The next source-triggered XY run must complete Build, TRX and artifact upload on the same SHA before L3 can be claimed.
- Earlier exact runs: IPv6 service type #45031159848 PASS; WAN DHCP #45031431084 PASS; VPN lifecycle #45031330323 PASS; WAN route #45031330345 PASS; AI handoff #45031330331 PASS; NVRAM audit #45031084347 PASS. These validate their respective ancestor SHAs only.
- Hardware L4 remains unverified.

## Exact first action for the next GPT

1. Fetch live main and inspect exact-SHA PPP/WAN route/VPN/XY-C1/AI Actions for this commit. XY-C1 must actually run Build, TRX verification and artifact upload; do not treat skipped steps as L3.
2. Fix any concrete exact-SHA failure; while jobs are pending, continue an independent high-priority Padavan stability/security source slice outside PPP/IPv6.
3. Keep the backup immutable and update this handoff only when facts or the next first action materially change.
