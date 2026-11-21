# Cross-session execution mandate — 2026-11-21

## Execution rules

- Continue on main only. Use one complete Git tree/commit per logical task and update main with non-forced expected-parent SHA.
- Never amend, force-push, create development branches/PRs, or touch the frozen backup branch (backup/pre-plugin-overhaul-2026-11-14 at 72f4f157a084f8184b9597ca5bf334b8080e5807).
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility; do not weaken tests.
- PENDING Actions do not block independent work; diagnose exact-SHA failures before claiming validation.
- Root dev_log.md must retain 20–119 newest-first records; preserve November archive part-021 and its index.

## Current state

- Parent main immediately before this RAS contract update: 72b789bc6c027d1bb9d36fdbc9473a00de9bb13f.
- Shared NVRAM fixture mismatch fixed in c6c1d7a78213f9525560a5fe8602a6ec07957357: fake_nvram_reader now supports optional expected key names; legacy defaults remain for old tests; the new interface-name test sets wan0_man_ifname and wan0_ifname_t explicitly.
- DHCP renew fail-closed fix is in 8700539128d7865df7d4e706fe710603e772c2c9: both WAN/IPTV renew callbacks check prior err/route_err/mtu_err fields and stored MTU; dhcp_wan_bound_retry_decide preserves first read error and only evaluates a complete tuple. Its policy/source tests have not yet been confirmed passing at exact head.
- Firewall stale checker fix now corrects whitespace-compacted modem_type parse token and asserts transactional snapshot -> strict parse -> peer-file-open order; commit generated from this parent must rerun Firewall.
- HTTPD security run #44895295713 failed at the DHCP renewal inline source contract: it still expects dhcp_wan_bound_retry_required() and legacy nvram_get_int reads. Update workflow .github/workflows/httpd-security.yml next to require checked-read retry decision and fail-closed order in both WAN and IPTV renewers.
- On parent 72b789, exact workflow IDs #44895636746 (Firewall), #44895636794 (USB modem identity), #44895636775 (Process inventory/lifecycle) and #44895636822 (XY-C1), plus AI docs #44895636818 were in progress at last inspection.
- Frozen backup stays 72f4f157a084f8184b9597ca5bf334b8080e5807. Last proven XY-C1 L3 baseline is 20e139d230b73eabc5f25a2b17d56a3a5768af11, run #44887656164; no current exact-source L3 claim. Hardware L4 not verified.

## Next exact actions

1. Inspect exact-head results for current Firewall/USB/Process/XY-C1/AI workflows; don't reuse statuses from previous SHAs.
2. Patch HTTPD security's inline Python assertions for WAN and IPTV renew functions, replacing legacy status getters with the current checked-read tuple contract and preserving existing resolver/route tests.
3. Recheck WAN DHCP callback and WAN route after fixture correction; fix the first new causal failure if either doesn't reach/pass the checked-renew regression.
4. After CI convergence, continue a distinct P1 reliability/security fix across other Padavan userland components, not just WAN.

## Verification limits

- No local checkout/runtime available; do not claim local C/Python test or firmware compile.
- Host CI is L2 only. XY-C1 Build + TRX + artifact upload must all pass on one exact production-source SHA for L3. Hardware/router L4 remains NOT VERIFIED.
