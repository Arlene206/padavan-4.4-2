# 2026-11-20 continuation checkpoint

## Cross-session execution mandate

- Continue development on main only. Every logical task MUST be one atomic main commit: create all blobs and one complete Git tree, then one commit and a non-forced expected-SHA update.
- Never amend, force-push, create development branches/PRs, or modify the frozen backup branch.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Fix real defects and add focused regression tests; never weaken checks to obtain PASS.
- A PENDING Action does not block independent work. Diagnose exact-SHA failures from the earliest causal log.
- Before context exhaustion, complete a safe slice, commit it and leave an exact first action for the next GPT. Do not ask the user what to do next.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. At session end inspect live main, verify frozen backup SHA, and inspect relevant exact-SHA Actions. Update this file only when task, source baseline, verification evidence or first next action materially changes. Do not create a no-op commit.

## Current repository state

- Repository Arlene206/padavan-4.4-2; parent for this primary metadata cleanup test slice: e085c6fb89a2906d234c27248d4daebf232e4fcc.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807 and is read-only.
- Product source 30af417ed8d7665e5da0f630e79b38bbb9f92296 moves nat_scan_completed after full NAT rule enumeration and has missing-chain fail-closed handling. Its exact-head process inventory #44804291620 and NVRAM audit #44804291862 passed. Its XY-C1 #44804291706 was still building at last observation.
- Exact main e085c6fb89a2906d234c27248d4daebf232e4fcc MiniUPnP #44804604643 PASS; AI docs #44804604650 PASS. The injected production-caller regression passed, proving lease cleanup still occurs after secondary peer/DSCP failure when the primary mapping was removed.
- This commit adds a testable finalizer for in-memory redirect description cleanup. It invokes del_redirect_desc only when the primary mapping is gone and always preserves aggregate firewall status, including secondary cleanup failures. The policy test exercises mocked callback invocation and arguments. Both standalone and inline source contracts are updated.
- Exact XY-C1 #44804604713 on e085 completed SUCCESS but its Build XY-C1, TRX and artifact steps were all SKIPPED because e085 changed only tests/checkers/docs. This new commit changes production C and requires a fresh exact-source build.

## Active task

1. Inspect exact-head MiniUPnP and AI Actions for THIS COMMIT; the finalizer regression and both source contracts must pass.
2. Inspect the new exact-head XY-C1 run for THIS COMMIT. Count L3 only if Build XY-C1, Verify TRX output and artifact upload all ran and passed on this exact source SHA.
3. If green, continue injected lower-level primary NAT/filter backend coverage (NAT init/delete failure, filter init/delete failure, rollback success/failure, absent match), or move to a separate concrete userland reliability/security defect.
4. Keep source → regression → atomic commit → exact-SHA Actions loops running while unrelated jobs are pending.

## Verification boundary

- Parent e085: MiniUPnP #44804604643 PASS; AI docs #44804604650 PASS; injected _upnp_delete_redir caller step PASS. Exact XY #44804604713 overall SUCCESS but Build/TRX/artifact all SKIPPED, so no L3 from that SHA.
- Product source 30af: process inventory #44804291620 PASS, NVRAM audit #44804291862 PASS. MiniUPnP and AI failures on that SHA were source-checker whitespace and handoff wording issues, fixed and verified on e085. XY #44804291706 was still in progress at last poll.
- This commit changes netfilter/iptcrdr.c and the policy header and needs a new production-source firmware build.
- MIPS/uClibc L3 and physical-router L4 remain NOT VERIFIED until their gates pass.

## Exact first action for the next GPT

Fetch live main and inspect this commit's exact-head MiniUPnP and AI runs. Verify the finalizer policy regression, production delete checker and injected caller test. Poll exact-head XY-C1, and confirm Build XY-C1, TRX validation and artifact upload individually. If any fail, inspect the earliest causal log and fix without weakening assertions. Then continue lower-level delete backend test coverage or another demonstrated userland defect.

## Handoff maintenance

- Keep dev_log.md newest-first; archive only at the 120-record threshold, retaining the newest 20.
- Update REFACTOR_ROADMAP.md only when module status, evidence or priority materially changes.
- Host regression, exact-source firmware L3 and physical hardware L4 are distinct verification levels.
