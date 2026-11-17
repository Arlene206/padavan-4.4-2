# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify main/source baseline, active task, exact-head Actions, validation boundary, blockers, frozen backup and first next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task. Continue one continuous engineering effort across GPT conversations: recover once, avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next.

Every logical multi-file change must be one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical change into partial commits, create a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`. Current source parent for this test/documentation repair: `243c2514e56474ed779a4057a1277e2ae178091c`.
- `net6.c` IPv6 per-interface cleanup, all `net_wan6.c` SIT/6RD/native route commands, `net_wan.c` dynamic PPP/WAN hook arguments and `net_ppp.c` L2TP/mwan static control calls use argv-based `eval()`.
- Exact-head `71c95adc`: Firewall `43991464263`, WAN route `43991464458`, Process inventory `43991464459`, USB modem `43991464382`, infosvr `43991464327`, AdGuardHome `43991464383`, AliDDNS `43991464312`, AliyunDrive `43991464319` SUCCESS. AI handoff `43991464381` failed because the next commit truncated `dev_log.md` to one entry. XY-C1 `43991464349` and NVRAM `43991464305` passed host checks; XY-C1 was still at Build at last poll.
- Exact-head `243c2514`: WAN DHCP `43991768681`, WAN route `43991768587`, USB modem `43991768563`, infosvr `43991768585`, AdGuardHome `43991768660`, AliDDNS `43991768674`, standalone process inventory `43991768704` SUCCESS. Firewall `43991768509` and Process `43991768591` failed on malformed regex; VPN `43991768846` and HTTPD `43991768656` failed on stale shell-command assertions; AI handoff `43991768626` found only two complete log entries. This commit repairs those causes.
- XY-C1 `43991768599`, NVRAM `43991768624` and AliyunDrive `43991768665` were IN_PROGRESS at last poll. Frozen backup remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## Active task

Re-run the corrected Firewall, Process, VPN, HTTPD and AI handoff checks; inspect exact-head XY-C1 Build/TRX/artifact, NVRAM and AliyunDrive outcomes. After these regressions clear, continue adjacent PPP startup/stop review across L2TP and multi-WAN lifecycle gates.

## Verification boundary

- The checker fixes in this commit have not yet run on its own SHA. Exact-head 243c failures exposed test/documentation drift rather than a C compile/runtime failure in the new PPP argv calls.
- Product-source MIPS/uClibc XY-C1 L3 (Build, TRX verification and artifact upload on one SHA) and physical-router L4 remain **NOT VERIFIED**.

## Exact first action for the next GPT

1. Fetch exact-head runs for the corrected PPP/Firewall/Process/VPN/HTTPD/AI checks and resolve any residual first failure from the job log.
2. Confirm same-SHA XY-C1 Build, TRX verification and artifact upload; continue PPP lifecycle status review while build work is pending.
3. Preserve the restored developer-log archive, keep the backup frozen, and use one-tree atomic commits with non-forced expected-SHA updates.
