# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state and complete an end-of-session accuracy checkpoint before ending. Record the current source SHA, exact-SHA Action state, test/build boundaries, and next concrete task. If all checkpoint facts and the first action remain accurate, explicitly conclude the check with no file change and no commit. Make one atomic main commit for a material checkpoint change; never create a no-op commit. Before context exhaustion, leave the next GPT a precise first action and truthful verification status.

- Work only on `main`. Never force-push, use side branches/PRs, or modify the frozen backup.
- For each logical engineering slice, change source + regression test + source contract together; create all blobs, one complete tree and one atomic commit; update `main` using a non-forced compare-and-swap with the fetched expected SHA.
- Inspect the exact product-source SHA's push Actions. PASS permits the next item; FAIL must be diagnosed and fixed; PENDING does not block independent work.
- Do not claim local tests, target builds, artifacts or hardware checks that were not performed.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- Parent SHA for the current IPv6 checker correction: `52a8c60d1e9aa324bb7f020fdeafda99a03319e9`. Fetch live `main` after this atomic commit for the current source SHA.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable/read-only. Legacy PR #1 is closed/unmerged.
- WAN PPP launch mode now uses a checked NVRAM transaction for `pppoemwan_enable` and, only for APP_RPL2TP L2TP, `wan_l2tpd`. Same immutable flags drive lifecycle lock choice, daemon stop/start, multi-WAN startup and rollback.
- All downstream contracts for WAN interface-name bounds, VPN lifecycle, modem WAN startup, WAN route and HTTPD have been changed to recognize the new PPP worker signature and checked mode snapshot.
- `safe_start_xl2tpd_locked()` already takes its VPN selector/credential snapshot before process scanning or rewriting the shared configuration file.
- Root files `SESSION_HANDOFF.md` and `dev_log.md` are the live checkpoint/history; avoid repeating initial audits or creating history snapshots.

## Active task

The standalone IPv6 service-type checker previously chose the wrong branch in `update_hosts_router()`. The valid production code first checks the IPv6 service type and snapshots the LAN IPv6 address before staging either hostname file, then later emits IPv6 host records after the stage files exist. The checker now explicitly verifies that two-phase order, without modifying product code.

The PPP selector integration correction is committed as `52a8c60d1e9aa324bb7f020fdeafda99a03319e9`; its exact-head Actions were all queued when last polled.

## Verification boundary

- Local C/Python tests were not run in this connector-only session.
- On ancestor `0af768201a45cf28482ffa4e8c42b6846e16081a`, IPv6 service-type #45030079720 passed the GNU89/ASan/UBSan policy binary and failed only in the stale standalone source-checker ordering assertion.
- On parent `d3805365ca95a17d5bd54736f29d5627580fa305`, AI handoff #45030597515 and process lifecycle inventory #45030597384 passed. WAN DHCP #45030597508, VPN lifecycle #45030597433, WAN route #45030597395, HTTPD #45030597411 and XY-C1 #45030597436 failed on dependent source contracts; the next commit `52a8c60d1e9aa324bb7f020fdeafda99a03319e9` corrected those contracts and fixed one real remaining undefined RPL2TP selector reference.
- Exact-head runs on `52a8c60d1e9aa324bb7f020fdeafda99a03319e9`: WAN route #45031084451, WAN DHCP #45031084413, USB modem #45031084464, VPN lifecycle #45031084455, XY-C1 #45031084406, NVRAM audit #45031084347, AI handoff #45031084508, firewall #45031084376 and process-inventory #45031084490/#45031084483 were queued at last observation. No result is claimed until they complete.
- No full XY-C1 firmware Build, TRX validation, artifact upload or hardware check is claimed for these commits. XY-C1 L3 requires all three CI steps to PASS on the same source SHA.

## Exact first action for the next GPT

1. Fetch live `main` and inspect exact-SHA IPv6 service-type Actions for this correction plus all PPP selector integration workflows for `52a8c60d1e9aa324bb7f020fdeafda99a03319e9`. Diagnose and fix concrete failures; treat pending checks as non-blocking.
2. Continue the wider Padavan userland stability/security rotation after those checks; do not reopen completed LAN/WAN IPv6 product code absent new evidence.
3. Before context exhaustion, update this handoff only when the resumable facts or first action materially change; if still accurate, explicitly conclude the check without a no-op commit.
