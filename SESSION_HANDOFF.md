# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state and compare it with `SESSION_HANDOFF.md` before ending or handing off. Verify the engineering/source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave the file untouched and make no no-op commit.

Before context exhaustion, finish or reduce the current safe slice, check exact-head Actions, commit any completed work atomically and preserve the precise first next action here.

Continue this engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active development ref: `main`.
- Parent main for this IPv6 teardown slice: `fbd9a1554d30f394e9e0090b9148bdb574cc9f6a`; all writes use a freshly fetched tree and a non-forced expected-SHA compare-and-swap.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only. Startup recovery is complete and must not be repeated.
- Root dev log has 34 records including this slice; part-012 retains the oldest 100 records.
- WAN reliability fixes cover required physical IFUP, IPv6 controls, MAN routes, PPP/IPoE/USB RAS transitions, IPTV and PPPoE passthrough. Static IPoE rejects malformed address/netmask with EINVAL; WAN/LAN overlap remains EADDRINUSE.
- PPP ip-up fails closed unless both negotiated IPCP IPv4 endpoints are present before interface/NVRAM publication.

## Engineering baseline

- Stop order invalidates PPP/DHCPv6 callback generations before service/device teardown; preserve lock and generation fencing.
- `reset_wan_vars()` invokes `reset_wan6_vars()` only after `stop_wan()` completes, so `ifname6` still identifies the last live interface during teardown.
- L3 firmware and L4 physical-router claims require exact evidence on the same source SHA; no L4 result is currently established.

## Active task

`wan6_down()` previously returned immediately when the current IPv6 configuration was disabled. During a full WAN restart, `stop_wan()` runs before `reset_wan_vars()` clears the previous runtime `ifname6`; NVRAM may already describe the next mode. The early return skipped RA/autoconfiguration disable, route/address cleanup and SIT teardown. Teardown now selects the still-live previous runtime interface first, falls back to a surviving SIT device when disabled, and clears `ifname6` only after successful cleanup. GNU89 policy tests and source contracts cover disabled/native/SIT/reconfiguration cases.

## Verification boundary

Exact-head validation for parent `fbd9a1554d30f394e9e0090b9148bdb574cc9f6a`:
- WAN route `43870829310`: PASS.
- WAN DHCP callback `43870829604`: PASS.
- WAN interface teardown `43870829555`: PASS.
- Process lifecycle `43870829665`, NVRAM `43870829549`, HTTPD `43870829629`, miniupnpd `43870829330`, DNSMasq `43870829617`, AliDDNS `43870829627`, VPN `43870829584`, AdGuardHome `43870829532`, AliyunDrive `43870829550`, infosvr `43870829594`, detect_internet `43870829447`, detect_link `43870829377`, USB modem identity `43870829507`, WAN ARP `43870829586`: PASS.
- AI handoff docs `43870829566`: FAIL because the previous commit removed the required `## Whole-repository module status` dashboard heading; this slice restores the complete prior dashboard and heading.
- XY-C1 firmware `43870829606`: IN_PROGRESS at last query, Build XY-C1 active and TRX/artifact pending. No exact-SHA L3 claim.
- Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head WAN route, WAN teardown and XY-C1 runs for this IPv6 teardown commit; repair the first causal failure only.
2. Confirm the AI handoff docs workflow passes after restoring the complete module-status dashboard.
3. Continue bounded PPP IPv6/DHCPv6 review only for concrete stale-state or error-propagation defects; preserve generation fencing.
4. Claim L3 only after Build, TRX verification and artifact upload pass on one identical SHA. Physical-router L4 remains unverified.
