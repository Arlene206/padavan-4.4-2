## Current repository state

- Active branch: `main`, current production source commit `c28bc0011a34f04e45b27f25891b37824634e15e` (`fix(mtk-esw): require CAP_NET_ADMIN for switch ioctls`). The subsequent checkpoint below is documentation-only.
- Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only and unchanged. No open PRs.
- No force-pushes. Multi-file changes use one complete Git tree and commit, followed by a non-forced compare-and-swap update of `main`.

## Active task

### MTK ESW ioctl authorization hardening

- The MTK switch device node was created as `0666`, and the kernel `mtk_esw_ioctl()` handler had no caller authorization check. Any local unprivileged account able to execute code could open the node and issue bridge, VLAN, port-power, PHY and other switch-control ioctls.
- Commit `c28bc0011a34f04e45b27f25891b37824634e15e` creates `/dev/mtk_esw` as `0600` and checks `capable(CAP_NET_ADMIN)` on every ioctl before acquiring the switch-state mutex. This also blocks requests through a file descriptor inherited before a privilege drop.
- Added a host-testable policy and GNU89 regression for the positive capability case and fail-closed denial for zero, negative and unexpected values. Switch CLI CI includes ASan/UBSan execution and source/trigger contracts; XY-C1 workflow filters track both policy files.

## Previous task — VSFTPD share ACL parser hardening

- Commit `b3c91f1` recognizes an exact `*folder=` key before validating total record length, so an overlong malformed duplicate cannot be skipped in favour of a later valid record.
- Commit `8296224` evaluates the final unterminated byte range but never accepts a matching record without its required newline. Regression cases cover malformed-only, malformed-before-valid, valid-before-malformed and unterminated final duplicate records.
- Exact VSFTPD #45408307661, AI handoff #45408307604, Process inventory #45408307674, NVRAM #45408307596 and XY-C1 #45408308038 all completed successfully on `82962240fea531c282def4176c5a0833ceeae676`. This closes the previous pending verification; do not repeat those checks.

## Previous task — MiniUPnP secure-mode delete authorization

- `DeletePortMapping()` fails closed with SOAP 501 when secure-mode owner lookup or IPv4 parsing fails, and returns 606 for a foreign owner.
- `DeletePortMappingRange()` checks all mapping owners before the first mutation, returning 501 for unresolved/invalid owner identity and 606 for foreign ownership. It also reports backend deletion failures as SOAP 501 rather than unconditional success.
- Exact MiniUPnP workflow #45406562259 and AI handoff #45406562291 passed on `eea1254`. This verification is not a firmware L3 claim.

## Previous task — XUPnP generated Lua configuration pair

- `eb0fbb1` stages and fsyncs both Lua configuration files before pair publication, restoring prior destinations if publication fails.
- DNSMasq validator #45402050207, Process lifecycle #45402861054, Process inventory #45403376647, and NVRAM #45401700436 passed their recorded exact SHAs.
- XY-C1 run #45401700209 passed Build, TRX verification, and artifact upload on the exact source SHA `eb0fbb1`. L3 evidence applies only to that source slice; physical-router L4 remains unverified.

## Verification boundary

- Host policy regression on the source diff for `c28bc0011a34f04e45b27f25891b37824634e15e`: GNU89 `-Wall -Wextra -Werror` with ASan/UBSan passed locally.
- Exact Switch CLI run #45410754770 completed PASS on `c28bc0011a34f04e45b27f25891b37824634e15e`, including the MTK ESW capability policy regression and source/trigger contract.
- Process lifecycle #45410754891, Process inventory #45410754960, HTTPD authentication/security #45410754925 and NVRAM transaction boundary #45410754900 completed PASS on the same SHA.
- AI handoff documentation run #45410754763 failed because `REFACTOR_ROADMAP.md` was 131,220 bytes, 148 bytes above its 131,072-byte hard limit. The production source and Switch CLI job passed; this docs-only checkpoint compacts the new P1 entry and records the live state. The AI check must be rerun on this documentation SHA.
- XY-C1 run #45410754918 was still running `Cross-build XY-C1 firmware` at last poll. No L3 claim until Build, TRX verification and artifact upload all pass for the same product-source SHA. Hardware L4 remains unverified.
- The older VSFTPD parent SHA `8296224` now has exact VSFTPD, AI, Process, NVRAM and XY-C1 job-level success (#45408307661, #45408307604, #45408307674, #45408307596, #45408308038 respectively). Prior XUPnP L3 evidence remains scoped to `eb0fbb1`; MiniUPnP source/test checks remain scoped to `eea1254`.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## Exact first action for the next GPT

1. Verify the AI handoff documentation run triggered by this documentation checkpoint and confirm the new `REFACTOR_ROADMAP.md` size is below 131,072 bytes.
2. Poll XY-C1 run #45410754918. Claim L3 for `c28bc0011a34f04e45b27f25891b37824634e15e` only if Build, TRX verification and artifact upload all PASS on that exact production-source SHA; diagnose a failure, or continue independent work while it is pending.
3. Continue the next independent userland stability/security defect. Do not repeat the completed MTK ESW host regression, VSFTPD ACL, MiniUPnP or XUPnP checks.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect the live repository state, relevant exact-head Actions, the validation boundary, blockers, and the first next action before context exhaustion. Record material source commits, real failures, and relevant exact-head Action transitions from PENDING to PASS or FAIL. If these facts and the first action remain accurate, explicitly conclude the checkpoint with no file change and no commit.

# Cross-session execution mandate

Recover once, then execute. An audit is a means, not the default endpoint. Continue source implementation, regression tests, small atomic changes, and exact-SHA verification across conversations. For a multi-file logical change, create all blobs and one complete tree, then one atomic main commit and a non-forced compare-and-swap update. Never force-push. Before context exhaustion, finish the current safe slice, checkpoint verification facts, and leave the next GPT one precise first action. Do not ask the user what to do next.
