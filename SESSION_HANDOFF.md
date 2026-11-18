# 2026-11-18 continuation checkpoint — Adbyby managed rule-pair recovery

## Current repository state
- Repository `Arlene206/padavan-4.4-2`, branch `main`. Parent SHA: `3f8e4f789c3d4eaa41f43e527a36aaeafa49940c`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 remains closed/unmerged.

## Active task
- Adbyby's managed `lazy.txt` / `video.txt` update previously used two atomic renames and rolled the first back if the second failed, but an abrupt shell termination between renames could leave a mixed rule generation.
- This slice adds a private same-filesystem transaction journal containing both old file states and backups before either live rename. Recovery validates the marker and private stage, restores both files idempotently, then removes the marker. Refresh runs recovery before inspecting cache digests; the EXIT trap attempts recovery on ordinary error/termination and preserves journal files if restoration cannot complete.
- Regression coverage includes the existing second-rename failure, a simulated interruption after publishing only lazy.txt, restoring a file that was previously absent, and rejecting symlinked transaction markers.
- Exact-head Actions are not yet known at commit creation. Check the new SHA's Adbyby workflow and any triggered AI/Process/XY-C1 workflows; do not infer target firmware or hardware success from host tests.

## Verification boundary
- Parent `3f8e4f789c3d4eaa41f43e527a36aaeafa49940c`: Shadowsocks `44308412124`, AI docs `44308412085`, Process inventory `44308412166` and Process fail-closed `44308412156` all PASS.
- ChinaDNS exact-directive and SmartDNS preservation source fix is in `38a2e1b80c78d250c892377647a43a454623228b`; its XY-C1 run `44307638792` was IN_PROGRESS at the last poll. Earlier `ef855ea69acec37a6c0b5ddefaab3214da26fbf7` XY-C1 run `44306582528` completed PASS, but that older source SHA does not verify the later ChinaDNS production change.
- Local checkout-based shell execution was NOT RUN. Target BusyBox/uClibc runtime and physical-router L4 are NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch live main and inspect the exact new commit SHA's Adbyby workflow logs. Fix any real failure before proceeding.
2. Query exact-head AI/Process/XY-C1 Actions; mark L3 PASS only after Build, TRX verification and artifact upload all succeed on the same source SHA.
3. Continue whole-userland stability fixes; do not reopen the completed Shadowsocks exact-cleanup source/test slice.

## Mandatory handoff checkpoint
Every session MUST inspect live repository state and end with an accuracy checkpoint before context exhaustion. The checkpoint is mandatory; a document edit is conditional. If all facts remain accurate, leave the handoff untouched and do not create a no-op commit.

## Cross-session execution mandate
Use small source/test slices, one complete Git tree and one atomic main commit per logical task. Before context exhaustion, leave the next GPT the exact source SHA, tests actually run, exact-head CI status, unverified levels and first action. Never force-push, change the frozen backup or claim validation not run on the same SHA.
