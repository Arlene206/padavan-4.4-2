# 2026-11-20 continuation checkpoint

# Cross-session execution mandate

- Continue main-only development. Every logical source slice is one atomic main commit: create all blobs, one complete Git tree and one commit, then update main using a non-forced compare-and-swap against the exact parent SHA.
- Never amend, force-push, create development branches/PRs, or modify the frozen backup branch.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Implement actual fixes with focused regressions, then inspect Actions for the exact pushed SHA.
- A PENDING Action does not block independent work. Diagnose failures from causal job logs; do not weaken tests to obtain a pass.
- Before context exhaustion, finish a safe slice and leave a factual next task. Do not ask the user what to do next.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

At the end of every session, inspect live main, verify that the frozen backup is unchanged, and check the exact-head Actions relevant to the latest source commit. Update this file only when the active task, source SHA, verification evidence, or first next action has materially changed. If the checkpoint is already accurate, leave it untouched and do not create a no-op commit.

## Current repository state

- Current main before this dynamic-capacity source slice: 2ce64bd716a56fc3dafee0ab022de9f55e29c10e; branch inventory has only main and the frozen backup.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14: 72f4f157a084f8184b9597ca5bf334b8080e5807 (read-only; unchanged).
- MiniUPnP allocation OOM fix 2ffb847 and checker normalization 8dbb096: exact-head MiniUPnP #44789878471 PASS (GNU89 ASan/UBSan cleanup regression and full source contract); AI handoff #44789878451 PASS.
- XY-C1 #44790179844 on 1ac1d7e failed in a pre-build source checker at an obsolete direct httpd_started NVRAM assertion; Build/TRX/artifact were skipped. The assertion was aligned to status writers in efebf98. Current XY-C1 run #44790573472 on 2ce64bd716a56fc3dafee0ab022de9f55e29c10e was IN_PROGRESS at last poll.
- Confirmed P1 memory-corruption defect in get_portmappings_in_range(): after 128 results it reallocates the same size and never increases capacity, so the next mapping writes beyond the allocated array.
- Physical-router L4 remains NOT VERIFIED.

## Active task

Fix MiniUPnP port-mapping enumeration buffer growth:

- Replace the no-op realloc-at-the-same-size branch in get_portmappings_in_range() with bounded capacity doubling.
- Check unsigned-int capacity overflow and size_t byte-size overflow before realloc; update capacity only after realloc succeeds.
- On overflow or realloc failure, free the array, reset *number, preserve errno and return the existing failure result.
- Add a GNU89 ASan/UBSan regression that grows beyond 128 entries and verifies all stored ports, plus a source contract that prevents the original fixed-capacity realloc from returning.
- Extend MiniUPnP and XY-C1 workflow triggers for the policy header/test/checker and add an exact-head sanitizer step.

After validation, continue the MiniUPnP update_portmapping() NAT/filter two-table commit rollback audit; the first review indicates NAT can be committed before a later filter failure leaves mismatched port state.

## Verification boundary

- MiniUPnP rule-allocation fix 2ffb847 and checker normalization 8dbb096: exact-head MiniUPnP #44789878471 PASS, including all GNU89 policy binaries, ASan/UBSan allocation cleanup and Python source contract.
- AI handoff #44789878451 PASS. Process lifecycle #44789544079 and NVRAM transaction #44789543972 PASS on earlier exact SHA 2ffb847.
- XY-C1 #44790179844 on 1ac1d7e failed before Build because of an outdated HTTPD direct-NVRAM source assertion; efebf98 aligns that checker. XY-C1 #44790573472 on current parent 2ce64bd716a56fc3dafee0ab022de9f55e29c10e was still running at last check. Do not call L3 complete unless Build, TRX and artifact all pass.
- New capacity regression/source checker are added in this commit and require exact-head MiniUPnP Actions after push. Physical hardware L4: NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live main and verify the backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807.
2. Check the exact-head MiniUPnP capacity regression and XY-C1 run for Build XY-C1, TRX verification and artifact upload. Diagnose first causal failures from logs.
3. Continue the update_portmapping() NAT/filter two-table failure-rollback audit; confirm transaction semantics and add regression coverage before changing it. Physical-router L4 remains unverified.

## Handoff maintenance

- Keep `dev_log.md` newest-first. Archive only when the 120-record rotation threshold is reached, retaining the newest 20 records.
- Update the roadmap only when module priority/status/evidence changes materially; do not copy it into history on routine updates.
- Host regressions, exact-source firmware L3 and physical hardware L4 are separate verification levels.
