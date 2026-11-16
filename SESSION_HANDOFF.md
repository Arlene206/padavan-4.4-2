# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state and compare it with `SESSION_HANDOFF.md` before ending or handing off. Verify the engineering/source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave the file untouched and make no no-op commit.

Before context exhaustion, finish or reduce the current safe slice, check exact-head Actions, commit any completed work atomically and preserve the precise first next action here.

Continue this engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; active development ref is main.
- Main baseline for this correction: d8d59acfb40c8d439682d8764e872283bed4a23c. Refetch before every write and use non-forced expected-SHA update.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14: 72f4f157a084f8184b9597ca5bf334b8080e5807; read-only. Startup branch recovery is complete and not to be repeated.
- Root dev_log.md remains below 30 records; part-012 preserves the 100 oldest entries and is indexed.
- Main includes the PPP/IPoE startup-status refactor and its errno-preserving demand PPP error contract.

## Engineering baseline

- Required LAN bridge/VLAN/switch and iNIC MII failures propagate; optional hw_nat/LED remain best-effort.
- WAN startup returns the first required failure across physical IFUP, IPv6 controls, MAN address/routes/gateway, PPP migration/launch/demand readiness, IPoE address/EAPOL/DHCP, and USB modem RAS. Optional tuning, UPnP notification and cleanup remain best-effort.
- check_modem_wan_start_status.py verifies the demand PPP helper's errno-normalized interface timeout, socket error, route preinstall failure, and the production first-error aggregation call sites.

## Active task

The miniupnpd workflow now expects two captured PPP migration calls in start_wan(): USB RAS and ordinary PPP. It also checks one captured IPoE migration call and retains the independent rollback-helper stop gate. Parent d8d59 failed only because the old contract asserted one PPP call.

## Verification boundary

- Parent d8d59 exact-SHA successes: AI docs 43868343106, Process inventory/lifecycle 43868343273, infosvr 43868343256, AdGuardHome 43868343268, WAN route 43868343214, HTTPD security 43868343265, AliyunDrive 43868343283 and AliDDNS 43868343245.
- miniupnpd 43868343344 failed at the exact PPP call count (expected 1, source has 2: RAS plus PPP); no product source failure.
- XY-C1 43868343338 was IN_PROGRESS at Build XY-C1, with TRX and artifact pending. NVRAM 43868343257 was IN_PROGRESS. No same-SHA firmware L3 is claimed yet.
- Hardware L4 is unverified.

## Exact first action for the next GPT

1. Inspect exact-head miniupnpd workflow repair, XY-C1 Build/TRX/artifact and NVRAM run.
2. Continue the bounded WAN adjunct-service status pass (IPTV startup and PPPoE passthrough), preserving WAN core availability while returning the first configured-service error to PID 1.
3. Preserve best-effort status only for optional notification, tuning and cleanup paths.
4. Claim L3 only after same-SHA Build, TRX verification and artifact upload all pass. Physical-router L4 remains unverified.

## Governance

Main only; frozen backup read-only. One full tree and atomic commit per logical task, then non-forced expected-SHA update. Update `dev_log.md` for source changes and preserve archive history. Distinguish host tests, exact-SHA Actions, firmware/TRX L3 and physical-router L4. Never claim an unrun result.
