# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect live repository state and complete an accuracy checkpoint before ending. The checkpoint is mandatory; a document edit is conditional.

Continue the recorded engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using a non-forced compare-and-swap. Never force-push, rewrite history, create partial commits, or touch the frozen backup. Before context exhaustion, record precise live refs, exact Action results and the next concrete task.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; active branch: main.
- Parent for THIS bridge configuration status slice: 47f1534749d11be908c9af57d5f1d31e28029ff8. Refetch live main before any subsequent write and use expected-SHA non-forced CAS.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 = 72f4f157a084f8184b9597ca5bf334b8080e5807; immutable. Startup governance/archive/PR recovery are complete; do not repeat.
- Root dev_log.md has 97 newest-first records before THIS entry; this entry makes 98. Keep 20–119 and preserve archived shards/indexes.

## Active task

Propagate status from LAN bridge multicast setup without short-circuiting independent operations:
- config_bridge() returned void and discarded return values from four br_set_param_int() writes, brport_set_m2u(), phy_igmp_static_port() and phy_igmp_snooping(). All APIs return int.
- New bridge_config_policy.h invokes all seven operations even when one fails, returns the first nonzero status, and exposes failure count/first operation for aggregate logging. The GNU89/Werror/ASan/UBSan regression verifies exact ordering/values, multiple injected failures, and fail-before-side-effects argument validation.
- config_bridge() now returns this status; start_lan() folds it into its LAN-start first error, and restart_iptv() folds it with proxy stop/start results for both router and AP paths. Follow-up proxy starts are still attempted when bridge setup fails; a failed AP proxy stop still blocks replacing a live proxy.
- This records setup failure; it does not abort PID 1 or suppress unrelated service start. Board-specific optional sysfs/switch feature support still requires hardware validation.

## Verification boundary

Exact-parent observations on 47f1534749d11be908c9af57d5f1d31e28029ff8:
- Process inventory run 43849407767 and Process regression 43849407856: SUCCESS. The latter included prior LAN startup error propagation.
- DNSMasq lifecycle 43849407985, AI handoff docs 43849407655, infosvr 43849407642, AdGuardHome 43849407657, AliDDNS 43849407685, AliyunDrive 43849407661, HTTPD 43849408102, WAN DHCP callback 43849408056, detect_internet 43849407600, detect_link 43849407631 and NVRAM 43849408107: SUCCESS.
- XY-C1 43849407925 was IN_PROGRESS at Build XY-C1; no TRX/artifact completion recorded yet.
- Previous driver-array checker correction passed its XY-C1 check before Build on parent be64a1990828e92895b4ce507216c509e1b35ff3, run 43849012639; do not reuse the rest of that run as current-parent firmware evidence.
- No physical-router/L4 result is established.

## Exact first action for the next GPT

1. Fetch live main and inspect this bridge-policy commit's exact-head Process regression, XY-C1 Build/TRX/artifact and NVRAM. Confirm GNU89 bridge regression and production source wiring. Only claim L3 if Build XY-C1, TRX verification and artifact upload all pass on the same SHA.
2. Audit remaining LAN bridge and IPTV operations for status handling, including unsupported-feature capability semantics; do not turn optional board feature absence into a service abort.
3. Continue broad RC stability/security coverage. Selected NDIS interface-to-driver binding is separate from module-family presence under /sys/module.

## Governance constraints

Preserve security, architecture, compatibility, target toolchain and mandatory tests. Main only; frozen backup read-only. One complete Git tree and one atomic commit per logical task, non-forced expected-SHA update. Update dev_log.md for every source/documentation change; keep 20–119 records and preserve archives/indexes verbatim. Distinguish host tests, source contracts, exact-SHA CI, firmware/TRX and physical-router evidence. Never claim an unrun result.
