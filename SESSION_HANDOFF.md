# Cross-session execution mandate — 2026-11-21

## Execution rules

- Continue on `main` only. Use one atomic main commit per logical task: create all blobs, one complete Git tree and one commit, then update the ref non-forcibly with the exact expected parent SHA.
- Never amend, force-push, create development branches or PRs, or modify the frozen backup.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Add meaningful regression coverage; do not weaken checks.
- PENDING Actions do not block independent work. Diagnose relevant exact-SHA FAIL results before declaring a slice validated.
- Before context exhaustion, finish the current safe slice, publish atomically, and leave the exact first next action here.

## Mandatory handoff checkpoint — end-of-session accuracy

The checkpoint is mandatory; a document edit is conditional. At every session end, compare live `main`, relevant exact-SHA Actions, product-source baseline, verification boundary, blockers and first next action with this record. Update it only when resumable facts materially change; do not create a no-op commit.

## Current repository state

- Parent `main` before this DHCP renewal checked-read slice: `a68bac734fd3f232fb16f095bed9bba83813da24`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only and unchanged.
- Latest production-source commits: `912fbff13b652c09563159f3357a21732db35f4b` fails closed on unreadable modem type while staging RAS peer config; `f268eab8f685b67a59736ce716a9bf88e031f69c` snapshots modem type/user/password/ISP under one NVRAM transaction; `a68bac734fd3f232fb16f095bed9bba83813da24` copies interface names before shared NVRAM cache reuse.
- Exact `a68bac7` WAN DHCP callback `#44895192116`, WAN route `#44895192094`, AI docs `#44895192006`, Firewall `#44895192001`, USB modem identity `#44895192081`, Process inventory `#44895192082`, Process lifecycle `#44895192033`, HTTPD `#44895192029`, XY-C1 `#44895192025` and other userland lifecycle workflows were pending at last inspection.
- Prior WAN route regression `#44894394702` PASS on exact SHA `b107e60a4c30b838ab963d3dc8ccdabeb4fe62e8`. This slice changes `net_wan.c` and WAN DHCP route-transition policy/tests, so fresh exact-head callback and route Actions are required.
- XY-C1 L3 is not verified for current source. Previous proven Build XY-C1 + TRX + artifact SHA remains `20e139d230b73eabc5f25a2b17d56a3a5768af11`, run `#44887656164`. Current a68bac7 XY-C1 `#44895192025` was pending; do not claim L3 until all stages pass on the same production-source SHA. Physical-router L4 is NOT VERIFIED.
- November archive index and `docs/dev-log/2026/11/part-021.md` remain preserved. Root dev log remains within its 20–119 record window.

## Active task

Close the WAN DHCP renewal status-read failure window.

Both `udhcpc_renew()` and `udhcpc_viptv_renew()` previously used error-collapsing `nvram_get_int()` for prior lease/route/MTU error flags; both also used unchecked stored-MTU reads. A backend error could look like zero and suppress the bound/rebind repair. This slice:
- reads the three previous error flags through `get_nvram_int_checked_status()`;
- calculates a retry decision only from a complete read tuple, preserving the first read error;
- logs and returns read failures before lease comparison/publication;
- checks stored WAN and IPTV MTU reads;
- adds GNU89 tests for each read failure, first-error precedence, complete tuples and retry conditions;
- extends the WAN DHCP callback source checker to enforce checked reads and ordering.

The first roadmap items for full-WAN-restart LAN-route restoration and `ifname_t` runtime identity publication already have policy tests/source-contract coverage on prior exact Actions. Do not duplicate those implementations absent new evidence.

## Verification boundary

- Parent a68bac7 WAN DHCP callback `#44895192116` and WAN route `#44895192094` were pending; this slice must trigger fresh exact-head runs.
- Parent AI docs `#44895192006` was pending.
- The patch was preflighted against fetched source. No local C/Python tests or firmware compilation were run because no local checkout/runtime is available.
- Host Actions are L2 only. XY-C1 Build/TRX/artifact must all pass on one exact production-source SHA for L3; physical-router L4 is NOT VERIFIED.

## Exact first action for the next GPT

Fetch live `main`, frozen backup and exact-SHA WAN DHCP callback, WAN route, AI docs, Firewall, USB modem identity, Process inventory/lifecycle, HTTPD and XY-C1 Actions. Fix the first causal current-head failure; continue independent work while any unrelated run is pending. If DHCP and WAN route pass, select a distinct P1 stability/security defect from the roadmap across Padavan userland and privilege boundaries rather than remaining confined to WAN. For every fix, add source/regression coverage, publish one atomic main tree/commit using the expected parent SHA with a non-forced ref update, then inspect Actions for that exact SHA.

## Handoff maintenance

- Root `dev_log.md` should retain 20–119 newest-first complete records; move the oldest 100 only when the next entry would make 120.
- Keep `docs/dev-log/2026/11/part-021.md` and its index metadata byte-for-byte intact.
- Update `REFACTOR_ROADMAP.md` only when module status, evidence or priority materially changes.
- Keep the frozen backup permanently unchanged.
