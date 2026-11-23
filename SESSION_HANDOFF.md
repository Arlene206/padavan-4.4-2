# SESSION HANDOFF — Padavan 4.4

## Current repository state

- Development branch: `main` only. Do not create feature branches or PRs; never force-push.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Preserve it permanently; do not modify, rename, delete or check it out.
- Current source checkpoint: THIS COMMIT (trigger XY-C1 on MTK ESW kernel changes); parent main SHA: `a3a7d64ba1fbaab7cc6f6d74d9c9650c017242ee`. Fetch live `main` on takeover; do not chase a docs-only SHA.
- Continue implementation rather than repeating completed audits or asking what to do next.

## Active task

The MTK ESW link-mode validation is in parent `a905ff6c262f066cbba6c6cb2c704dee01cd3d9f`. Its Switch CLI host regression passed, but the XY-C1 workflow did not trigger on that driver change because `.github/workflows/xy-c1-firmware-validation.yml` omitted `ioctl_mt762x.c`, `link_mode_policy.h`, and the new policy test from its push-path filter. A descendant commit `a3a7d64ba1fbaab7cc6f6d74d9c9650c017242ee` happened to trigger XY-C1 through an unrelated Samba change and includes the MTK code, but that accidental trigger is not reliable coverage. This commit adds the three paths and makes the Switch CLI contract assert the firmware trigger allowlist.
## Verification boundary

Parent source SHA `a3a7d64ba1fbaab7cc6f6d74d9c9650c017242ee`:
- Previous MTK ESW source SHA `a905ff6c262f066cbba6c6cb2c704dee01cd3d9f`: Switch CLI `45091472374` PASS, AI docs `45091472379` PASS; NVRAM `45091472420` was cancelled after another main update.
- Descendant `a3a7d64ba1fbaab7cc6f6d74d9c9650c017242ee` triggered XY-C1 run `45091634154` through an unrelated Samba source change; at last poll the workflow job was IN_PROGRESS with Build XY-C1 still PENDING. The build includes the MTK code but is not proof that MTK changes themselves trigger the workflow.
- Descendant NVRAM run `45091634173` was IN_PROGRESS; other host checks were queued/running.

THIS COMMIT changes the XY-C1 workflow itself, which is in its push allowlist; inspect its new exact-head build and ensure Build, TRX and artifact steps execute.

## Exact first action for the next GPT

1. Fetch live `main` and verify frozen backup `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not touch it.
2. Inspect exact-head Switch CLI, confirming the MTK ESW GNU89/ASan/UBSan test, pre-MDIO validation source contract, and XY-C1 trigger-path assertions pass.
3. Inspect exact-head XY-C1 on the same SHA; Build XY-C1, TRX verification and artifact upload must all pass for L3.
4. Continue another independent stability issue while jobs are pending. Repair first causal failures from their logs.
5. Preserve complete-tree atomic commits on freshly fetched main, `force:false`, exact `expected_sha`.

## End-of-session accuracy checkpoint

- Current implementation checkpoint: THIS COMMIT; parent main SHA `a3a7d64ba1fbaab7cc6f6d74d9c9650c017242ee`.
- Parent `a905ff6c`: Switch CLI `45091472374` and AI docs `45091472379` PASS; NVRAM `45091472420` cancelled by concurrent main advancement.
- Parent `a3a7d64ba1fbaab7cc6f6d74d9c9650c017242ee` triggered XY-C1 `45091634154` via Samba changes; Build was pending at last poll. It includes MTK ESW link-mode hardening, but did not prove that the driver path itself is watched.
- THIS COMMIT adds MTK ESW driver/header/test paths to the XY-C1 push filter and pins them in the host workflow contract. Its own exact-head XY-C1 is required.
- No L3/TRX/artifact or physical hardware L4 claim until observed.
