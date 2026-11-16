# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect live repository state and complete an accuracy checkpoint before ending. The checkpoint is mandatory; a document edit is conditional.

Continue the recorded engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched `main`; update `main` with exact expected SHA using a non-forced compare-and-swap. Never force-push, rewrite history, create partial commits, or touch the frozen backup. Before context exhaustion, record precise live refs, exact Action results and the next concrete task.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`.
- Parent main for THIS slice: `f0902af4861fdbf496958b22871ae43e51bdcfd5`. Fetch live `main` before every later write and use expected-SHA non-forced compare-and-swap.
- Frozen archive: `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable.
- Startup protocol, root governance, archive index, backup marker and PR #1 recovery are complete; do not repeat that audit.
- Root `dev_log.md` has 85 newest-first records before the next entry; keep it between 20 and 119 records and preserve archived shards/indexes.

## Active task

The WAN modem error path has been extended through the RAS physical WAN preparation:
- NDIS startup stops before interface migration/DHCP if miniupnpd cannot stop; returns real DHCP launch status and records WAN error.
- RAS startup aggregates miniupnpd migration, physical WAN `launch_wanx()`, and modem/RAS launch errors with first-error semantics while retaining best-effort setup ordering.
- WAN results propagate through restart/reconnect wrappers, boot service health and PID1 notification action results.
- Full WAN restart retains original ordered stop, WAN-bridge teardown and LAN bridge setup gates.

The current Process regression on `f0902af4861fdbf496958b22871ae43e51bdcfd5` reached the new RAS physical setup contract and failed only because its source checker expected a one-line aggregation call while the C source wraps its arguments. The next checker correction uses whitespace-tolerant regex without weakening the three NDIS/RAS aggregation checks. The host policy tests preceding the source checker passed.

## Verification boundary

Exact-head Action observations for `f0902af4861fdbf496958b22871ae43e51bdcfd5`:
- Process run `43844256457`: job `133220324178` FAIL at the RAS WANX aggregation source assertion; no later Process checks passed.
- XY-C1 run `43844254098`: still IN_PROGRESS at last inspection. Do not claim build/TRX/artifact until run and steps complete.
- USB modem WDM/NDIS physical identity run `43844254366`: SUCCESS.
- WAN DHCP callback `43844254326`: SUCCESS.
- miniupnpd atomic configuration publication `43844254468`: SUCCESS.
- WAN interface teardown `43844254482`: SUCCESS.
- WAN route `43844254309`: SUCCESS.
- AI handoff docs `43844254359`: SUCCESS.
- Process inventory `43844254197`: SUCCESS.
- AdGuardHome `43844254305`, NVRAM `43844254330`, AliDDNS `43844254220`, AliyunDrive `43844254407`, and XY-C1 remain in progress or unrelated; re-fetch exact states.
- HTTPD authentication run `43844254517` failed its independent switch-port ioctl contract, unrelated to this WAN slice.

No L3 firmware TRX/artifact or physical-router/L4 result is yet established for `f0902af4861fdbf496958b22871ae43e51bdcfd5`.

## Exact first action for the next GPT

1. Check this correction's exact-head Process, XY-C1 and AI docs Actions. Read failed logs and do not claim success from a queued or in-progress run.
2. Continue P1 NDIS control-TTY cross-device selection: bind the `ttyUSB` metadata to the selected NDIS interface's exact physical USB port ID + devnum. Preserve manual index selection only if its metadata belongs to that same modem. Otherwise select a preferred TTY on the same device first, then any same-device fallback. Add GNU89/Werror/ASan/UBSan regression cases and source contracts.
3. Then resume the device-aware `reload_modem_modules()` return/readiness contract without making alternative drivers mandatory without evidence.

## Governance constraints

Preserve security, architecture, compatibility, target-toolchain and mandatory tests. Use only `main`; frozen backup is read-only. Update root `dev_log.md` for every code/documentation commit; keep 20–119 root records and preserve archive shards/indexes verbatim. Distinguish source, CI and firmware validation levels. No force-push, history rewrite, or unrun-test/build/hardware claim.
