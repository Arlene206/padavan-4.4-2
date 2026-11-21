# Cross-session execution mandate — 2026-11-21

## Workflow
- Main only, atomic Git tree/commit per logical task, expected-SHA non-forced ref updates. No force push, amend, development branches, PRs or frozen backup edits.
- Preserve GNU89, MIPS/uClibc, BusyBox, Linux 4.4; do not weaken tests. Pending independent Actions do not block development.
- Root dev_log.md stays 20–119 newest-first entries; preserve November part-021 archive/index.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect the live repository state before ending, verify this handoff's source baseline, active task, exact-head Actions, validation boundary, blockers and first next action. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no no-op commit. Update this handoff only when next-session-critical facts materially change.

## Current state

- Parent main for this validation repair: `21bf595a0ecc81b2cf8e5500c93f93c03f4811e6` (commit `fix(wan): preserve IPTV settings across NVRAM cache reuse`); it adds checked IPTV mode reads and copies every cache-backed VLAN/static-IP value to local storage before the next NVRAM lookup.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`, immutable.
- The physical/IPTV DHCP renewal implementation on main uses checked `get_nvram_int_checked_status()` reads for lease/route/MTU error state and `dhcp_wan_bound_retry_decide()`. New checked-state policy tests exist. Stale workflow contracts still need alignment.
- Exact push Actions on `21bf595a0ecc81b2cf8e5500c93f93c03f4811e6`:
  - HTTPD security `#44895865685` FAIL: embedded source checker still expects six WAN route-text validation calls; actual production callback validates exactly three distinct route option values.
  - AI handoff docs `#44895865686` FAIL: handoff is missing the mandatory end-of-session accuracy checkpoint.
  - WAN DHCP callback `#44895865642` FAIL: embedded checker still expects the removed unchecked retry helper and direct `nvram_get_int()` status reads.
  - WAN route status `#44895865688` FAIL: `scripts/check_wan_vlan_setup.py:294` still expects `viptv_mode = nvram_get_int(...)` and borrowed static-option NVRAM pointers; production now uses checked reads and explicit bounded copies.
  - Process lifecycle inventory `#44895865634` PASS. NVRAM audit `#44895865623`, MiniUPnP `#44895865555`, XY-C1 `#44895865716`, and Process inventory/lifecycle regression `#44895865762` were PENDING at last poll.
  - USB modem identity `#44895865719`, Firewall startup `#44895865687`, Wireless `#44895865597`, watchdog/detect_link/detect_internet and WAN ARP checks `#44895865747`, `#44895865690`, `#44895865672`, `#44895865679`, and WAN teardown `#44895865582` PASS on this exact SHA.
- Last established XY-C1 L3 baseline remains `20e139d230b73eabc5f25a2b17d56a3a5768af11`, run `#44887656164`; no current-source L3 or hardware L4 claim.

## First actions
1. Inspect live main and exact-SHA Actions for HTTPD, WAN DHCP, WAN route, Firewall, USB modem, Process lifecycle/inventory, AI docs and XY-C1.
2. Fix the first new causal CI failure. The DHCP policy binary and callback checker must complete on this SHA; HTTPD embedded contract must pass.
3. After these integrations converge, move to a distinct P1 stability/security defect elsewhere in Padavan userland.

## Verification boundary
- No local checkout/runtime available; no local C/Python tests or firmware build are claimed.
- Host tests are L2 only. XY-C1 Build+TRX+artifact must pass on one exact production-source SHA for L3; hardware L4 NOT VERIFIED.