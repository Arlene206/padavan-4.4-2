# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state and complete an end-of-session accuracy checkpoint before ending. Record the current source SHA, exact-SHA Action state, test/build boundaries, and next concrete task. If all checkpoint facts and the first action remain accurate, explicitly conclude the check with no file change and no commit. Make one atomic main commit for a material checkpoint change; never create a no-op commit. Before context exhaustion, leave the next GPT a precise first action and truthful verification status.

- Work only on `main`. Never force-push, use side branches/PRs, or modify the frozen backup.
- Every logical slice creates all blobs, one full Git tree and one commit; update `main` only with `force:false` and a fetched expected SHA.
- Check push Actions for the exact source SHA. PASS continues; FAIL must be diagnosed and fixed; PENDING never blocks unrelated work.
- Do not claim tests, firmware builds, artifacts or hardware results that were not performed.

## Current repository state

- Repository `Arlene206/padavan-4.4-2`, development branch `main`.
- Parent for this LAN IPv6 source-contract correction: `592ce6b42c4c1f3ff473cdfb8839959c2fb4b5da`; fetch live main after the commit for the current SHA.
- Immutable backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`. Legacy PR #1 is closed/unmerged.
- PPP launch mode is a checked immutable NVRAM transaction snapshot for `pppoemwan_enable` and APP_RPL2TP L2TP `wan_l2tpd`; all lifecycle decisions reuse it.
- The router-hosts IPv6 checker now distinguishes address preflight and hosts emission; exact IPv6 service-type #45031159848 and AI handoff #45031159852 passed on ancestor `1e6238117d62087ea9eb5e99384b3ad2c65d1511`.
- `SESSION_HANDOFF.md` and `dev_log.md` remain the sole root handoff/history records.

## Active task

Two shared source contracts are being updated for status-bearing IPv6 APIs already used by production:
- `reload_lan_addr6()` reads a checked NVRAM snapshot before invoking `rc_lan_addr6_reload_from_snapshot()`; that helper returns read errors before clearing/publishing a live IPv6 address. Its regression test covers zero callbacks on read failure.
- `create_hosts_lan()` reads `get_lan_addr6_host_status()` before staging files and returns immediately on error; emitted IPv6 host rows use the captured address, not a later unchecked getter.

This changes test contracts only, not runtime code.

## Verification boundary

- Local tests were not run in this connector-only session.
- Exact parent `592ce6b42c4c1f3ff473cdfb8839959c2fb4b5da` was reached after PPP contract fixes. The earlier PPP integration source `52a8c60d1e9aa324bb7f020fdeafda99a03319e9` had AI handoff #45031084508 PASS, HTTPD #45031084503 PASS and process lifecycle inventory #45031084490 PASS.
- Ancestor WAN DHCP #45031084413 failed on an outdated unchecked LAN IPv6 getter contract; modem/USB/firewall/process/XY Actions #45031084464/#45031084376/#45031084483/#45031084406 failed on an outdated LAN IPv6 reload call contract. The source code already uses the checked snapshot APIs; these checker fixes must rerun.
- Ancestor PPP VPN lifecycle #45031084455 and WAN route #45031084451 failed at stale dependent contracts, now fixed in `592ce6b42c4c1f3ff473cdfb8839959c2fb4b5da`.
- NVRAM audit #45031084347 was still in progress at last observation. XY-C1 had not reached target firmware build after its earlier source-contract failure. No Build/TRX/artifact or hardware result is claimed.

## Exact first action for the next GPT

1. Fetch live main and inspect exact-head WAN DHCP, USB modem, firewall, process inventory, VPN, WAN route, XY-C1, AI and NVRAM Actions for this checker correction. Fix any new concrete failures.
2. Continue the wider Padavan stability/security rotation after exact-head feedback; do not redo startup audits.
3. XY-C1 L3 requires firmware Build, TRX validation and artifact upload all PASS on the same source SHA. Hardware L4 is unverified without device testing.
