# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state and compare it with `SESSION_HANDOFF.md` before ending or handing off. Verify the engineering/source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave the file untouched and make no no-op commit.

Before context exhaustion, finish or reduce the current safe slice, check exact-head Actions, commit any completed work atomically and preserve the precise first next action here.

Continue this engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; active development ref is main.
- Main baseline for this WAN adjunct-service status slice: 168527e76b4ba78f45d2268ecf8992563d93fbf6. Refetch main before each write and use a non-forced expected-SHA update.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14: 72f4f157a084f8184b9597ca5bf334b8080e5807; read-only. Startup branch recovery is complete.
- Root dev_log.md remains far below the 120-entry archive threshold; part-012 preserves the oldest 100 complete records.

## Engineering baseline

- Required LAN bridge/VLAN/switch and iNIC MII errors propagate; optional hw_nat/LED remain best-effort.
- WAN startup now preserves required physical IFUP, IPv6 controls, MAN static address/routes/gateway, PPP/IPoE start failures, USB RAS launch and cleanup results.
- This slice adds first-error status return for configured IPTV setup and PPPoE pass-through/relay setup, without preventing the main WAN protocol and other adjunct services from starting. Optional MTU and UPnP notification failures remain nonfatal.

## Active task

launch_viptv_wan() and set_passthrough_pppoe(1) previously logged startup errors but discarded them. Both results now feed wan_start_status. C policy tests cover first-error retention; check_modem_wan_start_status.py checks both production call sites and aggregation ordering.

## Verification boundary

- Parent d8d59 exact-SHA successes: WAN route 43868343214, Process inventory/lifecycle 43868343273, HTTPD security 43868343265, AI docs 43868343106, infosvr 43868343256, AdGuardHome 43868343268, AliyunDrive 43868343283 and AliDDNS 43868343245.
- miniupnpd 43868343344 failed only because the test asserted one captured PPP migration call; the source correctly has two (USB RAS and ordinary PPP). The expected count is corrected on 168527.
- XY-C1 43868343338 was still in Build XY-C1, TRX and artifact pending. NVRAM 43868343257 was still auditing. No L3/L4 claim.
- The current adjunct-service source changes require fresh exact-SHA Process/XY-C1/WAN route validation.

## Exact first action for the next GPT

1. Inspect exact-head Process, XY-C1, WAN route, miniupnpd and NVRAM Actions for the IPTV/PPPoE passthrough status slice.
2. Continue the targeted WAN callback review for any remaining required startup operation whose failure is logged but not returned; preserve core WAN availability while surfacing configured-service failures.
3. Keep optional MTU, UPnP notification and cleanup paths best-effort.
4. Claim firmware L3 only after same-SHA Build, TRX verification and artifact upload all pass. Physical-router L4 remains unverified.

## Governance

Main only; frozen backup read-only. One full tree and atomic commit per logical task, then non-forced expected-SHA update. Update `dev_log.md` for source changes and preserve archive history. Distinguish host tests, exact-SHA Actions, firmware/TRX L3 and physical-router L4. Never claim an unrun result.
