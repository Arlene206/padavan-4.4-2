# 2026-11-18 continuation checkpoint — Adbyby managed rule-pair recovery

## Current repository state
- Repository `Arlene206/padavan-4.4-2`, branch `main`. Current live main SHA: `141bf15edf2d35f950e04c4d267ad8bd140795d4`. Adbyby product-source slice: `c957987ad9971cb3f029dfb6a054d6367d059fa0`; checker-only correction: `7e12500ebb45e32611cc0db7dc37e6f80f49e7f4` followed by `141bf15edf2d35f950e04c4d267ad8bd140795d4`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 remains closed/unmerged.

## Active task
- Adbyby's managed `lazy.txt` / `video.txt` update previously used two atomic renames and rolled the first back if the second failed, but an abrupt shell termination between renames could leave a mixed rule generation.
- This slice adds a private same-filesystem transaction journal containing both old file states and backups before either live rename. Recovery validates the marker and private stage, restores both files idempotently, then removes the marker. Refresh runs recovery before inspecting cache digests; the EXIT trap attempts recovery on ordinary error/termination and preserves journal files if restoration cannot complete.
- Regression coverage includes the existing second-rename failure, a simulated interruption after publishing only lazy.txt, restoring a file that was previously absent, and rejecting symlinked transaction markers.
- The product implementation is in `c957987ad9971cb3f029dfb6a054d6367d059fa0`: `.rules-transaction` records the private `/tmp/adbyby-rules.*` stage path, and per-file `.state` / `.previous` records allow idempotent restore. The marker is validated for owner, mode, link count and strict stage-path form; the stage must be owner-controlled mode 700. The EXIT trap recovers on ordinary termination; a later refresh recovers after SIGKILL before digest/cache use.
- Checker-only commits `7e12500` and `141bf15` aligned the static contract with the implementation's loop-generated `adbyby_rule_name.previous` / `.state` files. Exact-head Adbyby host regression `44310660331` on `141bf15` PASSes (shell syntax, `rules_download.sh`, related host regressions and Python source contract).
- This is process-interruption recovery while `/tmp` persists, not power-loss/reboot durability. No local shell run was performed in this connector-only session.

## Verification boundary
- Product-source SHA `c957987ad9971cb3f029dfb6a054d6367d059fa0`: Process inventory `44310230750` PASS; Process fail-closed `44310230811` PASS; AI handoff `44310230698` PASS; HTTPD security `44310230857` PASS. Adbyby `44310230655` initially failed only because the static checker expected literal `lazy.previous`; after checker alignment, `141bf15edf2d35f950e04c4d267ad8bd140795d4` Adbyby run `44310660331` PASS. The intervening `7e12500` Adbyby run `44310425340` still expected literal `lazy.state`; `141bf15` changed that contract to the actual loop-driven `adbyby_rule_name.state`.
- XY-C1 firmware run `44310230672` is for the exact product-source SHA `c957987ad9971cb3f029dfb6a054d6367d059fa0`; it had reached the Build XY-C1 step with earlier build checks passing, but Build was still IN_PROGRESS and TRX/artifact upload PENDING at the latest poll. Do not mark L3 PASS yet.
- Prior unrelated source SHA `38a2e1b80c78d250c892377647a43a454623228b` XY-C1 `44307638792` succeeded, but that does not verify this Adbyby source change.
- Local checkout-based execution was NOT RUN. Target BusyBox/uClibc/MIPS runtime and physical-router L4 remain NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch live main, poll XY-C1 run `44310230672` for product-source SHA `c957987ad9971cb3f029dfb6a054d6367d059fa0`, and inspect Build, TRX verification and artifact upload. Mark L3 only if all three steps succeed.
2. Keep the Adbyby host result `44310660331` tied to checker-corrected SHA `141bf15`; if source, test or workflow paths change, inspect those exact-head Actions before marking them verified.
3. Continue the next highest-risk stability defect in the whole-userland roadmap. Keep the frozen backup unchanged and do not repeat the completed startup audit.

## Mandatory handoff checkpoint
Every session MUST inspect live repository state and end with an accuracy checkpoint before context exhaustion. The checkpoint is mandatory; a document edit is conditional. If all facts remain accurate, leave the handoff untouched and do not create a no-op commit.

## Cross-session execution mandate
Use small source/test slices, one complete Git tree and one atomic main commit per logical task. Before context exhaustion, leave the next GPT the exact source SHA, tests actually run, exact-head CI status, unverified levels and first action. Never force-push, change the frozen backup or claim validation not run on the same SHA.
