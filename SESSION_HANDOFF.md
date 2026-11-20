# 2026-11-20 continuation checkpoint

## Cross-session execution mandate

- Continue development on main only. Every logical task MUST be one atomic main commit, with all blobs and one complete Git tree created before one commit and a non-forced expected-SHA update.
- Never amend, force-push, create development branches/PRs, or modify the frozen backup branch.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Implement real source fixes with focused regressions; never weaken checks to manufacture PASS.
- A PENDING Action does not block independent work. Diagnose exact-SHA failures from the earliest causal log.
- Before context exhaustion, finish a safe slice and leave the exact first action for the next GPT. Do not ask the user what to do next.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. At the end of every session, inspect live main, verify the frozen backup hash, and inspect relevant exact-SHA Actions. Update this document only when the task, source baseline, verification evidence or first next action materially changes. Do not create a no-op commit.

## Current repository state

- Repository Arlene206/padavan-4.4-2; parent for this checker/handoff repair: ce8daa92ec5a57bcef2d8a662be32c00c6b369c1.
- Frozen backup branch backup/pre-plugin-overhaul-2026-11-14 remains at 72f4f157a084f8184b9597ca5bf334b8080e5807, read-only and unchanged.
- Production commit 30af417ed8d7665e5da0f630e79b38bbb9f92296 moved nat_scan_completed out of the pre-loop location and placed it after the complete NAT rule enumeration. Missing NAT chain remains a fail-closed state.
- Workflow-only commit 72b914095e14c099f60d6ac8f20150b4ca5d29e9 passed MiniUPnP #44803703734 and AI handoff #44803703663; it corrected the duplicate inline lease-append contract to match the transactional append helper.
- Exact-head MiniUPnP #44804291730 on 30af417 failed at check_miniupnpd_iptcrdr_delete.py. The checker normalizes whitespace but searched for escaped newlines; this repair matches the normalized loop-close marker before the completion assignment.
- Exact-head AI handoff #44804291590 on 30af417 failed because the exact required phrase "one atomic main commit" was missing from the current handoff; this has been restored.
- The production caller integration runner scripts/test_miniupnpd_redirect_delete_integration.py now extracts the actual _upnp_delete_redir() code, wraps its return type/signature, compiles it with injected firewall/lease callbacks and covers auxiliary cleanup failure, primary-state preservation and lease errno propagation. Its fixture fix is in ce8daa92ec5a57bcef2d8a662be32c00c6b369c1; this commit triggers the next exact-head result.
- Exact-SHA XY-C1 #44804291706 for product SHA 30af417 was still in progress at last poll; L3 is not verified. The older XY run on 8f884 failed because the scan marker was undeclared in the lookup function; corrected by the source commits 35e and 30af.

## Active task

1. Validate this commit's exact-head MiniUPnP and AI handoff workflows. The primary delete checker must enforce successful full enumeration before setting the scan-complete flag. The injected production caller test must compile under GNU89/Werror/ASan/UBSan and pass.
2. Inspect XY-C1 #44804291706 for source SHA 30af, including Build XY-C1, TRX verification and artifact upload. Count L3 only if all three actually ran and passed on that same SHA.
3. After this integration test passes, consider injected backend coverage of the lower-level NAT/filter deletion and rollback operation ordering, or move to a separate demonstrated stability/security defect elsewhere in the userland.
4. Keep following source → test → atomic commit → exact-SHA Actions. Pending unrelated workflows do not block development.

## Verification boundary

- Parent 30af: MiniUPnP #44804291730 failed at the normalized scan-loop close assertion; all preceding compile/regression steps passed. AI handoff #44804291590 failed the exact atomic-commit wording assertion.
- Parent cbe: MiniUPnP #44804109446 failed only in the injected fixture compile; generated code used literal backslash-n delimiters and omitted function signature. These fixture issues are fixed by ce8.
- Parent 72b: MiniUPnP #44803703734 PASS and AI handoff #44803703663 PASS.
- XY-C1 #44804291706 on source SHA 30af was still in progress at last poll. Firmware L3 and physical-router L4 remain NOT VERIFIED until their separate gates pass.

## Exact first action for the next GPT

Fetch live main and inspect exact-head MiniUPnP and AI documentation runs for this commit. If the injection test fails, inspect its compiler/runtime output and correct the generated translation unit or mock contract. Poll XY-C1 #44804291706, and verify Build XY-C1, TRX validation and artifact upload step conclusions on exact source SHA 30af. Then continue independent implementation.

## Handoff maintenance

- Keep dev_log.md newest-first; archive only at the 120-record rotation threshold, retaining the newest 20 records.
- Update REFACTOR_ROADMAP.md only when the module status, evidence or priority changes materially.
- Keep host regressions, exact-source firmware L3 and physical hardware L4 as distinct evidence.
