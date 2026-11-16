# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify live main/source baseline, active task, exact-head Actions, validation boundary, blockers, frozen backup and the first next action. The checkpoint is mandatory; a document edit is conditional. If all facts remain accurate, do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task. Continue one continuous engineering effort across GPT conversations: recover once, avoid repeated audits, do not wait for pending CI when independent work is available, and do not ask the user what to do next.

Every logical multi-file change must be one complete Git tree and one atomic main commit, parented to freshly fetched main, followed by a non-forced expected-SHA compare-and-swap. Never force-push, split a logical change into half-commits, create a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; current parent for this source slice: d28729af83c9692f376539c31d9578f129774d20.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807, unchanged/read-only.
- Source status: IPv6 global route/address/neighbor cleanup failure propagation and strict tri-state module loading are implemented. The last wireless Boolean module-load caller was migrated to is_module_loaded_status() in b6ff015.
- Exact-head d28729a results: WAN route 43967040074 PASS; AI handoff 43967040010 PASS; Process inventory 43967040068 PASS; NVRAM transaction audit 43967039938 PASS; AliDDNS 43967040108 PASS; AliyunDrive 43967040044 PASS; infosvr 43967039889 PASS; AdGuardHome 43967039819 PASS. XY-C1 run 43967039935 was compiling at last poll; TRX and artifact steps were pending.

## Active task

Propagate the errno-style status from set_passthrough_pppoe(1) through restart_all_sysctl() and the RCN_RESTART_SYSCTL PID 1 notification. The setter returns a status, but the void sysctl helper discarded it and its only caller ignored the helper result. This commit returns/logs the PPPoE relay status, merges it into the notification result, and adds a regression plus a production source contract.

## Verification boundary

- Parent d28729af: WAN route regression 43967040074 PASS; AI handoff 43967040010 PASS; Process inventory 43967040068 PASS; NVRAM 43967039938 PASS; several service/security regressions also PASS.
- Parent d28729af XY-C1 run 43967039935 was in progress with Build XY-C1 running and TRX/artifact pending at last poll. Do not report L3 until Build, TRX verification and artifact upload pass on one exact SHA.
- Current sysctl/PPPoE source, unit regression and integration contract require exact-head Process lifecycle and XY-C1 verification after this commit.
- Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head Process lifecycle and XY-C1 Actions for this sysctl notification commit. Check the host regression/source contract and separately verify same-SHA Build XY-C1, TRX and artifact upload.
2. Once verified, continue auditing concrete dropped network/RC results. The next candidate is flush_conntrack_table() in RCN_RESTART_NETFILTER, which currently has its result ignored; trace the writer's contract and every caller before changing any API.
3. Keep complete-tree atomic main commits with freshly fetched expected-SHA non-forced CAS. Frozen backup remains read-only; do not claim physical-router L4 without device evidence.
