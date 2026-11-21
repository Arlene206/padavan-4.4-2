# Cross-session execution mandate — 2026-11-21

## Execution rules

- Continue on `main` only. One complete Git tree and atomic commit per logical task; update main non-forcibly with the exact expected parent SHA.
- Never amend, force-push, create development branches/PRs, or alter the frozen backup `backup/pre-plugin-overhaul-2026-11-14` (`72f4f157a084f8184b9597ca5bf334b8080e5807`).
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Do not weaken regression contracts.
- PENDING workflows do not block independent fixes. Diagnose exact-SHA FAIL causes and continue; don't make unverified CI claims.
- Maintain root `dev_log.md` at 20–119 newest-first records; preserve November archive `part-021.md` and its index.

## Current repository state

- Current parent/main before the shared NVRAM fixture correction: `8700539128d7865df7d4e706fe710603e772c2c9`.
- Exact-head `8700539128d7865df7d4e706fe710603e772c2c9` CI:
  - WAN DHCP callback run `#44895295803` FAIL at GNU89/ASan test `wan_nvram_status_policy.c`, `fake_nvram_get()` line 660: newly added interface-name test asked for `wan0_man_ifname` and `wan0_ifname_t` but the fixture had only hardcoded legacy address/mask names.
  - WAN route `#44895295721` FAIL at its checked WAN interface-publication test for the same fake-reader name mismatch.
  - USB modem identity `#44895295781` FAIL at the shared NVRAM policy test for the same mismatch.
  - Firewall startup `#44895295888` FAIL at `scripts/check_modem_wan_start_status.py:162`: stale checker still requires `rc_wan_nvram_parse_int_value(modem_type_value, &modem_type)` inside `write_pppd_ras_conf()`, although the modem credentials/type now use a transactional snapshot.
  - HTTPD security `#44895295713` FAIL at inline Python assertion line 230 in DHCP lease/deconfig contract; inspect the embedded workflow to locate the precise assertion and compare current `man_down_internal()`.
  - Other exact-head runs that completed successfully include watchdog signal, wireless radio, detect_link, detect_internet, WAN teardown, WAN ARP probe, VPN lifecycle, ZeroTier, miniupnpd, DNSMasq and AI handoff docs (`#44895295660`). XY-C1, Process inventory and NVRAM transaction were still in progress/queued at the last recorded inspection.
- WAN DHCP renewal checked-read fix is in main commit `8700539128d7865df7d4e706fe710603e772c2c9`: physical and IPTV renewers now use checked reads for previous `err/route_err/mtu_err` state and stored MTU; a policy helper preserves first read failure and refuses partial retry tuples. New source-contract and GNU89 regressions are present but not yet reached in the failed callback/route runs.
- Earlier source fixes in the current chain include transactional modem credential snapshot (`f268eab8f685b67a59736ce716a9bf88e031f69c`) and copying cache-backed interface names before further NVRAM reads (`a68bac734fd3f232fb16f095bed9bba83813da24`).
- Last established XY-C1 L3 baseline remains `20e139d230b73eabc5f25a2b17d56a3a5768af11`, run `#44887656164`, successful Build + TRX + artifact upload on that exact source SHA. No current-source L3 claim; physical router L4 NOT VERIFIED.

## Immediate next actions

1. Finish the current atomic fixture correction. It adds optional per-call expected keys to `fake_nvram_reader`; existing tests retain legacy defaults and the interface-name regression now supplies `wan0_man_ifname` / `wan0_ifname_t`.
2. Fix the RAS credential checker to validate the new transactional snapshot contract (strict modem type parse and fail-closed-before-open semantics, snapshot-before-open, no unsafe direct cache getters), not the removed old call site.
3. Diagnose the HTTPD embedded assertion around line 230 in run `#44895295713`; distinguish a stale whitespace/ordering assumption from a real DHCP/MAN teardown ordering bug.
4. Fetch live main and exact-head workflows for each new commit. Once DHCP callback runs reach the new tests, verify the checked-renew source contract passes; repair any genuine failures.
5. After these fail causes converge, choose next independent P1 stability/security issue across the broader Padavan userland. Do not stay confined to WAN.

## Verification boundary

- No local checkout/runtime exists in this environment; no local Python/C tests or firmware compile are claimed.
- The exact failures above are observed Actions logs, not inferred.
- Host CI = L2; XY-C1 L3 needs Build, TRX verification and artifact upload passing on a single exact production-source SHA; hardware/router L4 remains unverified.
