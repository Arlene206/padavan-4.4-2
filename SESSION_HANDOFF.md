# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state and compare it with `SESSION_HANDOFF.md` before ending or handing off. Verify the engineering/source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave the file untouched and make no no-op commit.

Before context exhaustion, finish or reduce the current safe slice, check exact-head Actions, commit any completed work atomically and preserve the precise first next action here.

Continue this engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active ref: `main`.
- Parent main for this IPv6 teardown follow-up: `ee722b0f0c475812c3fad536944fb521c4bf97d7`; use fresh-tree commits and non-forced expected-SHA updates.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only. Startup recovery complete; do not repeat.
- Root dev log has 35 records after this entry; part-012 keeps the oldest 100 records.
- Static IPoE rejects malformed IPv4/netmask before `ifconfig()` (EINVAL); WAN/LAN overlap returns EADDRINUSE. PPP ip-up rejects missing IPCP endpoints before state publication.
- IPv6 teardown is generation-safe: stop DHCPv6 and invalidate callbacks before any route/address/SIT cleanup; saved runtime `ifname6` identifies the previous live interface until reset.

## Active task

A legacy `else { wan6_ifname = wan_ifname; }` inside `wan6_down()` overwrote the previous runtime interface selected by `wan6_teardown_select_ifname()` whenever it was not `sit0`. That defeated the fix when transitioning between native/PPP/SIT modes and flushed the new interface instead of the old interface's routes/addresses. The overwrite is removed; the selected live runtime interface now reaches route and address cleanup. WAN DHCP source contracts are updated to cover disabled mode, prior-interface selection, ordering and first-error aggregation.

## Verification boundary

Exact-head validation for `ee722b0f0c475812c3fad536944fb521c4bf97d7`:
- WAN route `43872288931`: PASS, including the GNU89/Werror/ASan/UBSan IPv6 selection regression and route workflow source contracts.
- AI handoff docs `43872288772`: PASS; full module dashboard was restored.
- Process inventory `43872289006`, Process lifecycle `43872288846`, HTTPD `43872289253`, NVRAM `43872289074` (still running at last poll), AdGuardHome `43872289035`, AliDDNS `43872288789`, infosvr `43872289116`, AliyunDrive `43872288716`: pass except NVRAM pending.
- WAN DHCP callback `43872288966`: FAIL because its source checker still expected the removed `IPV6_DISABLED` early-return. It also encodes the old six-accumulator count; current additional previous-interface handling has eight.
- XY-C1 firmware `43872288829`: IN_PROGRESS at last poll; Build active, TRX/artifact pending. No same-SHA L3 claim.
- Physical-router L4: NOT VERIFIED.

## Exact first action for the next GPT

1. Verify exact-head WAN DHCP callback run after replacing stale IPv6 teardown assertions; inspect any first causal test/source error.
2. Verify WAN route and WAN teardown runs on the same new SHA.
3. Inspect XY-C1 Build, TRX verification and artifact upload on that identical SHA before claiming L3.
4. Continue bounded IPv6 callback work only on a concrete defect; preserve DHCPv6 generation/lock fencing. Physical-router L4 remains unverified.
