# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the repository once, then execute the next verified engineering task; do not repeat completed audits or spend sessions on planning alone.

- one atomic main commit per logical slice. Create all changed blobs and one complete tree based on live main, parent the commit to the live SHA, and update with force=false plus exact expected_sha.
- Never force-push, amend published commits, develop on branches/PRs, or modify the frozen backup.
- Match Actions to exact head_sha. Diagnose failures from causal logs. A pending run is not a stop condition; work on independent tasks.
- Preserve GNU89, MIPS/uClibc, BusyBox/Linux 4.4 compatibility, fail-closed behavior, ABI/config compatibility and meaningful regressions.
- Before context exhaustion, finish the current safe slice, verify the exact SHA and leave one precise next action.

## Current repository state

- Live main SHA: ee8ef67dcd17b24ff79027e8dbbd66cb2f0dcbb1.
- WAN DHCP (11-field), virtual-IPTV DHCP (7-field), ZeroConf WAN (4-field), and ZeroConf IPTV (3-field) bound callbacks use a shared serialized NVRAM snapshot publisher with previous-value rollback and network/interface compensation.
- New source commit ee8ef67dcd17b24ff79027e8dbbd66cb2f0dcbb1 adds a length predicate for prior IP/netmask/gateway text before copying snapshot values into fixed INET_ADDRSTRLEN rollback buffers. This prevents a corrupt overlong value such as 255.255.255.255x from being truncated to a different valid address. The same guard is used in all four DHCP/ZeroConf callbacks.
- The commit adds focused tests for empty/exact-fit/overlong-prefix/NULL/zero-capacity cases and an 11-field fake NVRAM write failure: all eleven writes must be attempted and all old fields restored in reverse order after an after-write ENOSPC.
- Exact-SHA Actions on ee8ef67 were queued/in progress at last poll: WAN callback 44693084363, WAN route 44693084041, HTTPD 44693084106, AI handoff 44693084139, NVRAM boundary 44693084087, XY-C1 44693084076. No PASS/FAIL is claimed until each job completes.
- Previous source-equivalent checks: WAN route 44692804478, HTTPD 44692804337, AI docs 44692804357 passed on 5e2c84586842bffc46238d491b042dc1b9ee4741; the WAN callback on that SHA failed on a ZeroConf source-body extraction assertion, corrected by the prototype/call-site-aware parser in ee8ef67.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807, read-only. Legacy PR #1 remains CLOSED / NOT MERGED; AliDDNS hardening is already on main.

## Active task

1. Inspect exact-SHA Actions for ee8ef67. Diagnose any failing checker or C test from the failed job log and preserve transaction/full-rollback assertions.
2. Ensure the bounded-copy helper and 11-field rollback test compile and execute under GNU89/Werror/ASan/UBSan.
3. XY-C1 must reach and pass the MIPS build, TRX verification and artifact upload on the same production SHA before L3 can be claimed. Earlier f20b907 detector run passed host checks but skipped build/TRX/artifact because it saw no production input changes; inspect the current detector logs and fix the path classifier only if it is demonstrably wrong.
4. Once this boundary is green, continue another independent stability defect from the whole-repository governance ledger.

## Verification boundary

- PASS on 5e2c84586842bffc46238d491b042dc1b9ee4741: WAN route 44692804478, HTTPD 44692804337, AI docs 44692804357. WAN callback 44692804497 failed at a source-body assertion after the ZeroConf rollback helper was centralized; corrected parser selection and assertions are included in ee8ef67.
- EXACT-SHA ee8ef67dcd17b24ff79027e8dbbd66cb2f0dcbb1: WAN callback 44693084363, WAN route 44693084041, HTTPD 44693084106, AI docs 44693084139, NVRAM boundary 44693084087, XY-C1 44693084076 were queued or in progress at last poll.
- No same-source SHA has yet completed MIPS build + TRX verification + artifact upload after the WAN/IPTV/ZeroConf snapshot changes. L3 remains NOT VERIFIED. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch live main and exact-SHA run statuses for ee8ef67. Open any failed job log, then repair only the causal source/test/contract defect. Confirm the two new policy tests really execute under GNU89/Werror/ASan/UBSan, and check each of the four callback pre-copy guards. Keep running independent tasks while Actions are pending. Do not claim L3 until the same SHA passes MIPS build, TRX verification and artifact upload; do not claim physical L4 without device evidence.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state. Update this handoff only when resumable facts or the next action materially change. Otherwise make no no-op documentation commit.

## Evidence discipline

Keep host/source regressions, exact-head Actions, MIPS build/TRX and physical-device evidence separate. A same-production-SHA detector, MIPS build, TRX verification and artifact upload are required for L3. Never infer L4 from CI.
