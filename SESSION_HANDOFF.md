## Current repository state

- Active branch: `main`, source/test parent `82962240fea531c282def4176c5a0833ceeae676` (`fix(vsftpd): reject unterminated duplicate ACL entries`). The documentation checkpoint below is based on this parent and is a separate atomic commit. No open PRs.
- Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only and unchanged.
- No force-pushes. All multi-file changes use one complete Git tree and one commit, then non-forced compare-and-swap update of `main`.

## Active task

### VSFTPD share ACL parser hardening

- The earlier exact-record parser fixed newline-boundary matching, but review found two fail-open malformed-record cases.
- Commit `b3c91f1` now recognizes the exact `*folder=` key before validating total line length, so an overlong malformed record cannot be skipped in favour of a later valid duplicate.
- Commit `8296224` extends processing through the final unterminated byte range, but never accepts a matching ACL record without its required newline. Regressions cover malformed-only, malformed-before-valid, valid-before-malformed, and valid followed by overlong/exact unterminated records.
- The VSFTPD source contract checks key-first matching, strict record length, duplicate rejection, and EOF handling. Exact VSFTPD workflow #45408307661 passed on `8296224`.

## Previous task — MiniUPnP secure-mode delete authorization

- `DeletePortMapping()` fails closed with SOAP 501 when secure-mode owner lookup or IPv4 parsing fails, and returns 606 for a foreign owner.
- `DeletePortMappingRange()` checks all mapping owners before the first mutation, returning 501 for unresolved/invalid owner identity and 606 for foreign ownership. It also reports backend deletion failures as SOAP 501 rather than unconditional success.
- Exact MiniUPnP workflow #45406562259 and AI handoff #45406562291 passed on `eea1254`. This verification is not a firmware L3 claim.

## Previous task — XUPnP generated Lua configuration pair

- `eb0fbb1` stages and fsyncs both Lua configuration files before pair publication, restoring prior destinations if publication fails.
- DNSMasq validator #45402050207, Process lifecycle #45402861054, Process inventory #45403376647, and NVRAM #45401700436 passed their recorded exact SHAs.
- XY-C1 run #45401700209 passed Build, TRX verification, and artifact upload on the exact source SHA `eb0fbb1`. L3 evidence applies only to that source slice; physical-router L4 remains unverified.

## Verification boundary

- VSFTPD exact run #45408307661 on `8296224`: completed PASS, including the ACL GNU89 regression and source contract. AI handoff #45408307604 PASS; Process inventory #45408307674 PASS.
- NVRAM transaction audit #45408307596 was still `in_progress` at last poll.
- XY-C1 firmware run #45408308038 was still building at last poll; `Build XY-C1` was `in_progress`, while TRX verification and artifact upload were `pending`. No L3 claim for `8296224`.
- MiniUPnP source/test and checker verification above is exact for `eea1254`; do not transfer its test result to firmware L3 for this later SHA.
- Backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## Exact first action for the next GPT

1. Fetch live `main`, backup ref and Actions once; verify the exact-head VSFTPD and AI handoff results after this checkpoint commit.
2. Poll NVRAM #45408307596 and XY-C1 #45408308038. Claim L3 only if Build, TRX verification and artifact upload all PASS for the same product-source SHA.
3. Continue the next independent stability/security defect across Padavan userland; do not repeat the completed ACL, MiniUPnP, XUPnP, DHCPDECLINE or already-verified checks.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect the live repository state, relevant exact-head Actions, the validation boundary, blockers, and the first next action before context exhaustion. Record material source commits, real failures, and relevant exact-head Action transitions from PENDING to PASS or FAIL. If these facts and the first action remain accurate, explicitly conclude the checkpoint with no file change and no commit.

# Cross-session execution mandate

Recover once, then execute. An audit is a means, not the default endpoint. Continue source implementation, regression tests, small atomic changes, and exact-SHA verification across conversations. For a multi-file logical change, create all blobs and one complete tree, then one atomic main commit and a non-forced compare-and-swap update. Never force-push. Before context exhaustion, finish the current safe slice, checkpoint verification facts, and leave the next GPT one precise first action. Do not ask the user what to do next.
