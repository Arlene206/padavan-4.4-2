# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state and compare it with `SESSION_HANDOFF.md` before ending or handing off. Verify the engineering/source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave the file untouched and make no no-op commit.

Before context exhaustion, finish or reduce the current safe slice, check exact-head Actions, commit any completed work atomically and preserve the precise first next action here.

Continue this engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active development ref: `main`.
- Parent main for this static IPoE overlap status fix: `68643ba87132dedc5945679d13e6b0e216861cdc`; each write must use a fresh main/tree read and non-forced expected-SHA compare-and-swap.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only. Initial branch/handoff recovery is complete; do not repeat.
- Root `dev_log.md` contains 27 entries before this source milestone.

## Engineering baseline

- Required LAN bridge/VLAN/switch and iNIC MII errors propagate; optional hw_nat/LED remain best-effort.
- WAN startup now preserves required physical IFUP, IPv6 controls, MAN static address/routes/gateway, PPP/IPoE start failures, USB RAS launch and cleanup results.
- This slice adds first-error status return for configured IPTV setup and PPPoE pass-through/relay setup, without preventing the main WAN protocol and other adjunct services from starting. Optional MTU and UPnP notification failures remain nonfatal.

## Active task

Static IPoE WAN/LAN subnet overlap was already detected and prevented from continuing to `wan_up()`, but the rejection only set a local `wan_err`; it did not update the return value consumed by PID 1. The conflict branch now accumulates `EADDRINUSE` in `wan_start_status`. The WAN DHCP callback workflow now asserts this fail-closed status contract.

## Verification boundary

- Parent main: `68643ba87132dedc5945679d13e6b0e216861cdc`; the preceding miniupnpd checker correction now expects both PPP setup branches to capture `check_upnp_wanif_changed()` into `vlan_result`.
- The latest parent exact-head Actions were still being triggered/settled when this slice began; no same-SHA XY-C1 Build + TRX + artifact L3 or hardware L4 is claimed.
- Earlier verified successes on the preceding source/checker SHA included Process lifecycle, WAN route, HTTPD security, AI handoff, infosvr, AdGuardHome, AliyunDrive and AliDDNS.
- Frozen backup remains unchanged and read-only.

## Exact first action for the next GPT

1. Inspect exact-head Process, XY-C1, WAN route, miniupnpd and NVRAM Actions for the IPTV/PPPoE passthrough status slice.
2. Continue the targeted WAN callback review for any remaining required startup operation whose failure is logged but not returned; preserve core WAN availability while surfacing configured-service failures.
3. Keep optional MTU, UPnP notification and cleanup paths best-effort.
4. Claim firmware L3 only after same-SHA Build, TRX verification and artifact upload all pass. Physical-router L4 remains unverified.

## Governance

Main only; frozen backup read-only. One full tree and atomic commit per logical task, then non-forced expected-SHA update. Update `dev_log.md` for source changes and preserve archive history. Distinguish host tests, exact-SHA Actions, firmware/TRX L3 and physical-router L4. Never claim an unrun result.
