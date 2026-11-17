# 2026-11-18 continuation checkpoint — factory reset status propagation and CI verification

## Current repository state
- Repository `Arlene206/padavan-4.4-2`; latest implementation commit: `0d58928ee6ab852cbbc37f4a2201a685c8252e1e`.
- The immutable backup `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify it.
- Factory-reset defect fixed in `0d58928e`: propagate MTD erase status, gate NVRAM reset marker on successful storage erase, gate PID 1 reboot request on both persistent operations, attempt configured rstats recovery on abort, and restore the reset LED when the physical button path fails.
- Exact-head GPIO button workflow `44197451556`: PASS — GNU89 daemon policy, new fail-closed factory-reset policy under ASan/UBSan, hold timing regression, and Python source/call-order contracts all succeeded.
- Exact-head Process lifecycle `44197451701`: PASS. Exact-head MiniUPnP configuration/lifecycle `44197451163`: PASS. AliDDNS `44197451708`, Inadyn `44197451699`, NFS `44197451136`, VPN server `44197451509`, iNIC `44197451350`, WAN ARP `44197451378`, firewall `44197451483`, FRP `44197451512`, HTTPD security/lifecycle `44197451148`/`44197451349`, AdGuardHome `44197451198`, infosvr `44197451180`, Samba `44197451302`, and MiniUPnP `44197451163` were PASS at the last exact-SHA poll.
- Exact-head AI handoff run `44197451610`: FAIL only because this handoff had not yet included the mandatory end-of-session checkpoint clause required by the current development protocol. THIS documentation update adds the missing explicit rule and records exact statuses; no product source change is made.
- Exact-head XY-C1 firmware run `44197454090` was QUEUED at the last poll. It must complete Build XY-C1, TRX verification and artifact upload before L3 can be claimed for `0d58928e`. Earlier `88770d79` firmware success is not evidence for this source SHA.
- Local host/MIPS compilation was NOT RUN because no full checkout/toolchain is available in this session. Physical-router L4 remains NOT VERIFIED.

## Active task
- Factory-reset source change `0d58928e` is committed. Its new dedicated GPIO workflow is green, along with Process and MiniUPnP integration.
- Close the exact-head documentation check and remaining queued/in-progress Actions. Inspect any first-causal failure; do not conflate an AI handoff-contract failure with firmware source failure.
- Continue an independent high-priority reliability/security slice while XY-C1 is queued or building.

## Verification boundary
- `0d58928e`: GPIO `44197451556` PASS; Process `44197451701` PASS; MiniUPnP `44197451163` PASS; AI handoff `44197451610` FAIL on missing literal end-of-session checkpoint clause, fixed here; XY-C1 `44197454090` QUEUED.
- Other exact-head runs above were PASS as listed; remaining run set includes queued/in-progress independent module regressions. Query exact `head_sha` before relying on any result.
- New factory-reset GNU89 ASan/UBSan test passed in Actions. No local compilation or hardware L4 claim.

## Exact first action for the next GPT
1. Fetch live `main` and the exact-`head_sha` push runs for this documentation commit, especially AI handoff and XY-C1. Require Build, TRX and artifact completion on the exact product SHA.
2. If AI still fails, inspect the first failing assertion and preserve the exact checkpoint requirements from `scripts/check_ai_handoff_docs.py`; no no-op commit if already accurate.
3. Repair any product CI failure in a separate atomic main commit. While XY-C1 remains queued/in progress, continue another independent P1 stability/security issue.
4. Keep the backup immutable; hardware L4 remains NOT VERIFIED.

## Mandatory handoff checkpoint
Every session MUST inspect the live repository state, exact-SHA Actions that materially changed, current handoff/roadmap facts, and the first actionable next task before ending. The checkpoint is mandatory; a document edit is conditional. Record new PASS/FAIL/PENDING state accurately. If the facts and first action remain accurate, leave the handoff untouched and do not create a no-op commit.

# Cross-session execution mandate

Continue one engineering effort across GPT conversations. Recover once, then implement concrete defects. Do not wait idly on PENDING Actions or ask the user what to do next.

Every logical multi-file change must use Git data APIs to create all blobs, one complete tree and one atomic main commit, parented to freshly fetched main and published with non-forced expected-SHA compare-and-swap. Never force-push, create development branches/PRs or modify the frozen backup.

Before context exhaustion, finish a safe slice, commit atomically, inspect exact-SHA Actions and leave a precise first action.
