# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify live main/source baseline, active task, exact-head Actions, validation boundary, blockers, frozen backup and the first next action. The checkpoint is mandatory; a document edit is conditional. If all facts remain accurate, do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task. Continue one continuous engineering effort across GPT conversations: recover once, avoid repeated audits, do not wait for pending CI when independent work is available, and do not ask the user what to do next.

Every logical multi-file change must be one complete Git tree and one atomic main commit, parented to freshly fetched main, followed by a non-forced expected-SHA compare-and-swap. Never force-push, split a logical change into half-commits, create a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; current parent for this checker correction: 1e10514145025ceb0ce9e0a0cd5468e28e6b7fa0. Frozen backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807, read-only.
- Source slice: restart_all_sysctl() now returns the errno-style status of set_passthrough_pppoe(1), logs the failure, and its sole RCN_RESTART_SYSCTL caller merges the result into PID 1's notification status.
- The new checker caught and corrected a doubled-backslash whitespace regex before the queued Process job ran; it now checks the whitespace-compacted function signature. The host regression checks positive errno retention and notification failure mapping.

## Active task

Verify exact-head Process lifecycle and XY-C1 for the PPPoE sysctl result-propagation commit and this checker correction. The preceding source/checker baseline d28729a has a complete XY-C1 Build/TRX/artifact pass; that L3 result does not cover the new sysctl source change.

## Verification boundary

- d28729af exact-head WAN route 43967040074 PASS, AI handoff 43967040010 PASS, Process inventory 43967040068 PASS, NVRAM audit 43967039938 PASS, and XY-C1 Build/TRX/artifact run 43967039935 PASS.
- Commit 1e105141 introduced the sysctl PPPoE status propagation; its Process lifecycle job 43968920396 was queued and XY-C1 43968920456 was building when polled. The checker fix in this commit is now in place before Process execution; exact-head reruns are pending.
- Current sysctl commit Build/TRX/artifact must all pass on the same current SHA before claiming L3. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head Process and XY-C1 runs for this checker correction. Verify host regression/source integration and same-SHA Build XY-C1, TRX output and artifact upload.
2. If those pass, continue auditing remaining dropped RC/network statuses. The next candidate is flush_conntrack_table() under RCN_RESTART_NETFILTER; inspect return semantics and all callers before API changes.
3. Keep full-tree atomic commits and expected-SHA non-forced CAS; never alter the frozen backup or claim physical-router L4 without device evidence.
