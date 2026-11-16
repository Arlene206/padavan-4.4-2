# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify live main/source baseline, active task, exact-head Actions, validation boundary, blockers, frozen backup and the first next action. The checkpoint is mandatory; a document edit is conditional. If all facts remain accurate, do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task. Continue one continuous engineering effort across GPT conversations: recover once, avoid repeated audits, do not wait for pending CI when independent work is available, and do not ask the user what to do next.

Every logical multi-file change must be one complete Git tree and one atomic main commit, parented to freshly fetched main, followed by a non-forced expected-SHA compare-and-swap. Never force-push, split a logical change into half-commits, create a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- Source parent for the current module-state slice: `1554a5f3fbb1bf13c3325ad765b7e806d74d1537`. d287 exact-head XY-C1 `43967039935` passed Build, TRX output verification and artifact upload; its WAN `43967040074`, Process `43967040068`, AI handoff `43967040010`, NVRAM `43967039938`, AdGuardHome `43967039819`, infosvr `43967039889`, AliyunDrive `43967040044` and AliDDNS `43967040108` all passed. This establishes L3 for d287 only.
- The separate PPPoE sysctl source slice is in `1e10514145025ceb0ce9e0a0cd5468e28e6b7fa0`; its first Process run `43969335166` failed only on a whitespace-sensitive assertion for a multiline first-error helper call. This commit removes all whitespace before matching that helper call. AI handoff `43969335259` on d837 passed.
- Active source defect: `net.c` used lossy Boolean module probes to gate HW NAT disable, FTP helper configuration and SFE enable/disable. The new slice migrates those callsites to strict tri-state state handling and carries probe errno instead of suppressing unknown state.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains recorded at `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only. Never alter it. Physical-router L4 remains **NOT VERIFIED**.

## Active task

Complete and validate the strict tri-state module state transition changes in HW NAT, FTP and SFE. A negative sysfs probe means UNKNOWN, not ABSENT: propagate its errno; do not skip unload; do not reconfigure FTP conntrack parameters while the module state is unknown; and do not touch SFE sysfs unless load is verified. Check all caller contracts and exact-head Actions after commit.

## Verification boundary

- d287 exact-head firmware L3 is verified by Build XY-C1, TRX output and artifact upload in run `43967039935`; no later source commits inherit that result.
- d837 AI handoff `43969335259` passed; Process `43969335166` failed at the formatting-sensitive sysctl status contract, and this commit corrects that assertion. Current-main (`1554a5f`) Process, XY-C1, Firewall Startup and AI jobs were in progress at last poll.
- Existing injected module policy tests already verify direct load probe errors, unload pre/post probe errors, busy modules and first-error retention. The new production source contract extends `scripts/check_modem_wan_start_status.py` to assert strict HW NAT, FTP and SFE handling; same-SHA Process, Firewall Startup and XY-C1 checks are required.
- Physical-router L4 remains **NOT VERIFIED**.

## Exact first action for the next GPT

1. Fetch live `main`; inspect the exact-head Process run for the sysctl checker fix and Process/Firewall Startup/XY-C1/AI jobs for the HW NAT/FTP/SFE slice. Fix failures from exact job logs.
2. Continue P1 RC/network result-propagation work. The currently queued separate candidate is `flush_conntrack_table()` under `RCN_RESTART_NETFILTER`; trace its return contract and all callers before changing the API.
3. Use complete-tree atomic main commits with non-forced expected-SHA updates; keep the backup read-only and never claim physical-router L4 without hardware evidence.
