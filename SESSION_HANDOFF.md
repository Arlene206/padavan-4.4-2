# 2026-11-20 continuation checkpoint

## Cross-session execution mandate

- Continue development on main only. Every logical task MUST be one atomic main commit: create all blobs and one complete Git tree, then one commit and a non-forced expected-SHA update.
- Never amend, force-push, create development branches/PRs, or modify the frozen backup branch.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Fix actual source defects with focused regressions; never weaken checks to obtain PASS.
- A PENDING Action does not block independent work. Diagnose exact-SHA failures from the earliest causal log.
- Before context exhaustion, finish a safe slice, commit it and leave an exact first action for the next GPT. Do not ask the user what to do next.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. At session end inspect live main, verify frozen backup SHA, and inspect relevant exact-SHA Actions. Update this file only when task, source baseline, verification evidence or first next action materially changes. Do not create a no-op commit.

## Current repository state

- Repository Arlene206/padavan-4.4-2; parent for this peer-cleanup state gate fix: 7d9b12c46927b2fbb3aba6ea0346de0cc1b7d0a1.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807, read-only.
- Prior commit 7d9b12c46927b2fbb3aba6ea0346de0cc1b7d0a1 added a testable primary-metadata cleanup finalizer. It invokes del_redirect_desc only when primary mapping is removed and preserves aggregate status. Its exact-head tests are queued at last observation.
- Prior verified host baseline e085c6fb89a2906d234c27248d4daebf232e4fcc: MiniUPnP #44804604643 PASS, AI docs #44804604650 PASS, including the injected _upnp_delete_redir caller regression. e085 XY #44804604713 succeeded with product Build/TRX/artifact skipped (test/checker/docs-only).
- New source review found that delete_redirect_and_filter_rules_ex() still removed the peer SNAT and DSCP rules even when primary DNAT/filter deletion had failed or NAT rollback had restored it. This risks breaking a still-live redirect. The current change wraps the entire peer delete + mangle + rollback sequence in a tested primary-mapping gate; the first primary error is kept and no secondary firewall mutation occurs on that failure.

## Active task

1. Validate this commit's exact-head MiniUPnP, AI, Process and NVRAM Actions. The peer cleanup policy unit test, dedicated peer checker and duplicate workflow contract must all pass.
2. Inspect exact-head XY-C1 and verify Build XY-C1, TRX and artifact upload on this same product source SHA; only then count L3.
3. If verified, add fault-injected coverage for the lower NAT/filter backend deletion/rollback stages (NAT init/delete failure, filter init/delete failure, successful/failed restoration, no matching primary rule), or continue with another demonstrated userland stability/security defect.
4. Keep source → test → atomic commit → exact-SHA Actions loop running; pending work does not block independent tasks.

## Verification boundary

- Parent 7d9b12c: new exact-head Actions were queued at the last poll. The immediately preceding e085 source-freeze MiniUPnP and AI runs passed; e085 XY Build/TRX/artifact were skipped, so it provides no L3.
- The concurrent product-source SHA 30af417 process inventory and NVRAM audit passed; its XY build was still in progress at the last observation. This current peer cleanup code change requires a fresh current-SHA XY-C1 Build/TRX/artifact.
- Firmware L3 and physical-router L4 remain NOT VERIFIED until their separate exact gates pass.

## Exact first action for the next GPT

Fetch live main and inspect exact-head MiniUPnP, AI, Process, NVRAM and XY-C1 runs. Confirm peer SNAT/mangle/rollback are scoped under the primary-removal gate and the new GNU89 test passes. Confirm Build XY-C1, TRX validation and artifact upload individually on this exact SHA. Diagnose the earliest failure, then continue lower-level delete fault injection or another concrete userland defect.

## Handoff maintenance

- Keep dev_log.md newest-first; archive only at the 120-record threshold, retaining the newest 20.
- Update REFACTOR_ROADMAP.md only when module status, evidence or priority materially changes.
- Host regression, exact-source firmware L3 and physical hardware L4 are distinct verification levels.
