# Cross-session execution mandate — 2026-11-21

## Execution rules

- Continue on main only. Publish each logical task as one complete tree/commit and update main with a non-forced expected-SHA ref update.
- Never amend, force-push, create development branches/PRs, or modify frozen backup backup/pre-plugin-overhaul-2026-11-14 (72f4f157a084f8184b9597ca5bf334b8080e5807).
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4. Do not weaken regression checks.
- PENDING workflows do not block independent tasks. Diagnose exact-SHA failures and continue.
- Preserve November archive part-021 and index; root dev_log.md remains 20–119 newest-first records.

## Current repository state

- Parent main: c6c1d7a78213f9525560a5fe8602a6ec07957357; frozen backup remains unchanged at 72f4f157a084f8184b9597ca5bf334b8080e5807.
- Exact 8700539 CI exposed four independent problems:
  - WAN DHCP callback #44895295803, WAN route #44895295721, USB modem identity #44895295781: shared fake reader test aborted because the new interface-name case used custom NVRAM keys while the fake callback required legacy fixed names. Fixed on main by c6c1d7a78213f9525560a5fe8602a6ec07957357: optional key overrides added to test fixture; old defaults retained.
  - Firewall startup #44895295888: stale source-contract whitespace in checker line 162; corrected in this current slice.
  - HTTPD security #44895295713: inline DHCP-renew source contract still requires old dhcp_wan_bound_retry_required() and three unchecked nvram_get_int() calls after production moved to checked status reads. This remains the next failure to fix.
- DHCP production slice in 8700539128d7865df7d4e706fe710603e772c2c9: physical and IPTV renewers use checked err/route_err/mtu_err status reads, preserve first read error through dhcp_wan_bound_retry_decide(), and use checked stored-MTU reads; added GNU89 policy tests and extended WAN DHCP callback checker. Exact-head Actions still need to reach/validate those new test stages.
- The modem RAS snapshot in f268eab8f685b67a59736ce716a9bf88e031f69c safely copies all four fields under one NVRAM transaction; strict integer parsing and both fail-closed gates precede PPP peer file creation. This commit fixes only checker whitespace and asserts this sequence.
- Current c6c1d7a exact-head actions were still in progress/queued at last inspection after fake-reader fixture correction. Check exact SHA runs afresh after this commit; don't assume previous failures are resolved until rerun.
- Last proven XY-C1 L3 remains 20e139d230b73eabc5f25a2b17d56a3a5768af11, run #44887656164, successful Build+TRX+artifact on that exact source SHA. No current SHA L3; hardware L4 unverified.

## Exact next actions

1. Inspect new current-head exact Actions for WAN DHCP callback, WAN route, Firewall, USB modem, AI docs, Process inventory/lifecycle, and XY-C1.
2. Correct the HTTPD inline DHCP-renew contract in .github/workflows/httpd-security.yml to check the checked-read helper/order in both WAN and IPTV renewers. Its failure is at the assertion requiring dhcp_wan_bound_retry_required(lease_error, route_error, mtu_error), followed by three old direct getters.
3. Verify DHCP callback and WAN route workflows reach the new policy binary and check_wan_dhcp_dltime_publication.py; repair any causal new failure.
4. After convergence, choose a distinct P1 stability/security defect across the broader Padavan userland rather than remaining confined to WAN.

## Verification boundary

- No local checkout/runtime was available; no local C/Python tests or firmware compile are claimed.
- Exact CI failures observed: WAN DHCP, WAN route, USB modem fixture failures; Firewall stale compact-string assertion; HTTPD stale renew assertion. No newer current-head pass is asserted until Actions on the new SHA complete.
- Host CI = L2; XY-C1 Build, TRX, artifact must pass on one exact production-source SHA for L3; hardware/router L4 NOT VERIFIED.
