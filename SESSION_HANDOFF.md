# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state and complete an end-of-session accuracy checkpoint before ending. Record the current source SHA, exact-SHA Action state, test/build boundaries, and next concrete task. If all checkpoint facts and the first action remain accurate, explicitly conclude the check with no file change and no commit. Make one atomic main commit for a material checkpoint change; never create a no-op commit. Before context exhaustion, leave the next GPT a precise first action and truthful verification status.

- Work only on `main`. Never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice is one complete Git tree and one atomic commit; update `main` only with a non-forced expected-SHA compare-and-swap.
- Inspect exact product-source-SHA Actions. PASS permits continued work; FAIL requires diagnosis and correction; PENDING does not block independent work.
- Do not claim unrun local tests, target builds, artifacts or hardware checks.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch `main`.
- Parent for the current PPP contract normalization: `1e6238117d62087ea9eb5e99384b3ad2c65d1511`; fetch live `main` after this atomic commit for current SHA.
- Immutable backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Legacy PR #1 is closed/unmerged.
- PPP launch mode is read using one checked NVRAM transaction for `pppoemwan_enable` and (APP_RPL2TP L2TP only) `wan_l2tpd`; lock, daemon stop/start, multi-WAN start and rollback consume its immutable result.
- Downstream WAN DHCP, VPN, HTTPD, XY-C1, PPPoE-MWAN and PPP option contracts have been migrated to the updated worker signature and mode fields in the current series.
- The IPv6 service-type standalone source checker was corrected and its exact-head run #45031159848 PASS; AI handoff run #45031159852 PASS.

## Active task

This small slice corrects two remaining PPP dependent contracts. The VPN lifecycle checker now tests the RPL2TP start-result helper using substring presence rather than a brittle escaped-newline comparison. The PPPoE-MWAN checker now uses the three-argument locked-worker signature and verifies partial-session cleanup is gated by the same immutable multi-WAN flag.

## Verification boundary

- Local tests were not run in this connector-only session.
- The PPP integration parent `52a8c60d1e9aa324bb7f020fdeafda99a03319e9` had AI handoff #45031084508 PASS and HTTPD #45031084503 PASS; process inventory #45031084490 PASS.
- On that parent VPN lifecycle #45031084455 failed at the whitespace-fragile launch-result contract; WAN route #45031084451 failed on the outdated worker signature. These are addressed by this commit.
- On that parent check_modem-related source contracts still failed because `reload_lan_addr6()` now uses the checked `lan6_read_config_value_status()` plus `rc_lan_addr6_reload_from_snapshot()`, not the retired helper expected by the checker. LAN DHCP #45031084413 also failed because its hosts contract expects the obsolete unchecked `get_lan_addr6_host()` path; current production code reads via `get_lan_addr6_host_status()` before staging. Further source-contract repairs are pending.
- NVRAM transaction audit #45031084347 was still in progress at last observation. XY-C1 #45031084406 failed in the same stale checked-LAN-IPv6 source-contract stage before target build. No full firmware Build/TRX/artifact or hardware result is claimed.

## Exact first action for the next GPT

1. Inspect exact-head Actions for this correction SHA and update any source-contract failures.
2. Fix the checked LAN IPv6 reload and LAN DHCP hostname source contracts to assert the actual status-bearing snapshot APIs and their read/error-before-stage order. Preserve the current product-code behavior.
3. Re-check the NVRAM audit and XY-C1 job states on their exact SHA; continue broader stability/security tasks while any unrelated jobs are pending.
4. XY-C1 L3 requires Build, TRX validation and artifact upload all PASS on the same source SHA. Do not claim hardware L4 without hardware validation.
