# 2026-11-20 continuation checkpoint

## Cross-session execution mandate

- Continue main-only engineering. One logical source slice per atomic commit; create all blobs and one complete Git tree, then update main with non-forced compare-and-swap against the exact parent SHA.
- Never amend, force-push, create development branches/PRs, or modify the frozen backup.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Add focused regressions and inspect exact-SHA Actions logs.
- Pending Actions are not a reason to wait; continue independent fixes. Finish safe slices and leave an accurate next task.

## Current repository state

- Parent main for the active VPN source slice: `84199a2fe17d774b8bcdba0a1918e7fa10a33160`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only).
- WAN route callback status source-contract correction: commit `2e3fffd9ef63b596f0e7f076bc7ab9e5a16d47ce`; exact-head WAN route #44762541362 and AI handoff #44762541303 passed.
- HTTPD Authentication/Security #44761848875 and AI handoff #44761848633 passed on `c84d26a6`.
- Parent `84199a2f` added MiniUPnP IPv6 pinhole validation; its AI handoff #44762854261 and MiniUPnP stability #44762854186 passed. XY-C1 #44762854295, Process #44762854209 and NVRAM #44762854323 were in progress at last check.
- This VPN change alters product source; new exact-SHA VPN and XY-C1 validation is required. Do not transfer firmware L3 from any earlier SHA. Hardware L4 remains NOT VERIFIED.

## Latest source task — VPN domain runtime snapshot lifecycle

- `restore_dns_from_vpnc()` previously removed `vpnc_dns_t` but left `vpnc_dom_t`; OpenVPN client-up omitted the domain write when the current session lacked a DOMAIN option. A stale domain could survive disconnect/reconnect.
- Added checked domain-only cleanup, attempts it even after DNS snapshot read/write failure, retains the first error, and rebuilds resolver state only after DNS snapshot removal succeeds.
- OpenVPN client-up now always writes the current domain or an explicit empty value before writing the up-state marker.
- A GNU89 policy regression and source-integration checks extend the VPN stability workflow. Poll exact-head Actions after commit and diagnose failures from logs.

## Exact first action for the next GPT

1. Fetch live main and the frozen backup; inspect exact-SHA VPN stability, WAN route, HTTPD, AI, Process, NVRAM and XY-C1 Actions for the newest source SHA.
2. Fix any failing source contracts from causal job logs without weakening coverage.
3. Continue high-severity reliability/security work across Padavan userland. Claim firmware L3 only when detector, Build XY-C1, TRX and artifact upload all pass on one exact product-source SHA.

## Handoff maintenance

- Keep `dev_log.md` newest-first; do not archive until the 100-record threshold is reached.
- Update this file only when active task, SHA, verification evidence, or exact next action changes.
- Host policy tests, source-contract CI, exact-source firmware L3 and physical hardware L4 are separate verification levels.
