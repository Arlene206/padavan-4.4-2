# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify main/source baseline, active task, exact-head Actions, validation boundary, blockers, frozen backup and first next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task. Continue one continuous engineering effort across GPT conversations: recover once, avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next.

Every logical multi-file change must be one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical change into partial commits, create a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`. Parent for this IPv6 global sysctl slice: `8291da3892430ca915c5414c609aca246f78c7aa`.
- Exact parent Actions: WAN DHCP callback `43984154569` PASS; AI handoff `43984154581` PASS; Process inventory `43984154609` PASS; AdGuardHome `43984154572` PASS; AliDDNS `43984154561` PASS; infosvr `43984154692` PASS; AliyunDrive `43984154565` PASS; NVRAM audit `43984154656` PASS.
- LAN IPv6 cleanup/reload status propagation is implemented. Dynamic SIT/native WAN IPv6 address-adds in `net_wan6.c` use argv-based `eval()`; the WAN DHCP checker correction is in this parent.
- This slice makes `control_if_ipv6_all()` return first sysctl failure while attempting all applicable writes, skips only per-device entries for interfaces not yet registered, checks neighbour-cache writes, and propagates status through IPv6 init/enable/disable.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Physical-router L4 remains **NOT VERIFIED**.

## Active task

Validate this commit's IPv6 global sysctl policy: first-error retention while every applicable operation continues, safe skipping of absent optional devices, checked neighbour-cache sysctl output, and result aggregation in IPv6 initialization and runtime enable/disable. Inspect exact Process, Firewall, WAN route, AI docs and XY-C1 Actions.

## Verification boundary

- Exact parent `8291da3892430ca915c5414c609aca246f78c7aa`: WAN DHCP callback, AI handoff, Process inventory, AdGuardHome, AliDDNS, infosvr, AliyunDrive and NVRAM audit passed.
- This commit adds global IPv6 sysctl result propagation and ASan/UBSan tests. Its exact-head workflows must complete before validation claims. Physical-router L4 remains **NOT VERIFIED**.

## Exact first action for the next GPT

1. Inspect this commit's exact-head Process, Firewall, WAN route, AI docs, WAN DHCP callback, NVRAM audit and XY-C1 runs; resolve the first causal failure.
2. Confirm XY-C1 Build, TRX verification and artifact upload on one exact SHA; product-source L3 remains NOT VERIFIED until all pass. Physical-router L4 remains NOT VERIFIED.
3. Continue the IPv6 route/tunnel command and interface-provenance audit; preserve frozen backup and use non-forced expected-SHA updates.
