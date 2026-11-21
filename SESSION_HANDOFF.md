# Cross-session execution mandate — 2026-11-21

## Workflow
- Main only, atomic Git tree/commit per logical task, expected-SHA non-forced ref updates. No force push, amend, development branches, PRs or frozen backup edits.
- Preserve GNU89, MIPS/uClibc, BusyBox, Linux 4.4; do not weaken tests. Pending independent Actions do not block development.
- Root dev_log.md stays 20–119 newest-first entries; preserve November part-021 archive/index.

## Current state
- Parent main before HTTPD contract correction: c14651353b1096db701f560729c0052d0275f560.
- Fake NVRAM fixture fix on c6c1d7a78213f9525560a5fe8602a6ec07957357; RAS snapshot checker whitespace/order fix on c14651353b1096db701f560729c0052d0275f560.
- WAN DHCP production fix on 8700539128d7865df7d4e706fe710603e772c2c9: checked lease/route/MTU error-state reads in physical and IPTV renew functions, first-error policy helper, checked stored MTU. C6/72b actions ran on direct descendants but exact-head test convergence is not yet proven.
- HTTPD run #44895295713 failed because inline workflow assertions still required old retry helper and unchecked integer getters. This current slice replaces those assertions with checked-read, helper, checked MTU, and fail-closed ordering checks for both renewers.
- RAS source contract fix in c146... requires snapshot -> strict modem type parse -> open and ensures both errors abort before opening peer file. New exact-head Firewall validation pending.
- Last confirmed frozen backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807. Last proven XY-C1 L3 baseline is 20e139d230b73eabc5f25a2b17d56a3a5768af11 (#44887656164); current-source L3 and hardware L4 are unverified.

## First actions
1. Inspect live main and exact-SHA Actions for HTTPD, WAN DHCP, WAN route, Firewall, USB modem, Process lifecycle/inventory, AI docs and XY-C1.
2. Fix the first new causal CI failure. The DHCP policy binary and callback checker must complete on this SHA; HTTPD embedded contract must pass.
3. After these integrations converge, move to a distinct P1 stability/security defect elsewhere in Padavan userland.

## Verification boundary
- No local checkout/runtime available; no local C/Python tests or firmware build are claimed.
- Host tests are L2 only. XY-C1 Build+TRX+artifact must pass on one exact production-source SHA for L3; hardware L4 NOT VERIFIED.