# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect live repository state and compare it with this file before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers and precise first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave this file untouched and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task here.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, and do not ask the user to choose the next task.

Every multi-file logical change must be created as one complete Git tree and one atomic main commit parented to freshly fetched main, then update main with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, or modify the frozen backup branch.

## Current repository state

- Latest main commit: `528da87938e245b3c45c0bf95839b9a0d9676ec1` (checker-only whitespace normalizer for targeted eject). Parent c1aca USB storage `43907888057` passed policy/graph steps 1–24 and failed only at step 25 on a wrapped C-call assertion. This commit replaces the first correction's unavailable `re.sub()` dependency with `" ".join(text.split())`. Run `43908320511` is the exact-head USB storage test on 528da; inspect its result/log. Parent c1aca XY-C1 `43907888399` and NVRAM `43907888090` were in progress at last poll; verify same-SHA Build/TRX/artifact.

- Repository: `Arlene206/padavan-4.4-2`; current source parent before this slice: `e05ccbcb8c458ee2bc07c6020ed5af52b3d4cff8`.
- Mount-link errno fix, GNU89 policy regression, storage source contracts, and workflow corrections are included together in THIS COMMIT.
- Frozen backup branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable and read-only.
- Storage application startup returns first errors from all seven configured services; media/download daemon launch and readiness errors flow through the common startup result, and deferred hotplug preserves retry events until success.
- `create_mp_link()` retains its boolean API but now initializes errno, normalizes mount inventory/path/directory/symlink/cleanup failures, keeps the first actionable error, and exposes `create_mp_link_status()` before all four media callers. MiniDLNA/Firefly retain the 0→1 directory fallback; Transmission/Aria retain mode 0 only.
- USB modem hotplug preserves coalesced unplug/replug events and retries failed reconnects; WAN IPv6 and LAN/Wi-Fi startup paths propagate failures.

## Active task

The targeted-eject policy and product source changes are in parent c1aca. USB storage `43907888057` passed the policy and full storage graph through step 24, then step 25 failed only on a multiline C-call assertion. Parent 528da queued USB storage run `43908320511`; its checker correction used `re.sub` without importing `re`. THIS COMMIT changes normalization to built-in `str.split()`; product source is unchanged. Inspect the exact-head rerun and check the replacement workflow on THIS COMMIT.
## Verification boundary

- Product parent c1aca: USB storage `43907888057` passed steps 1–24, failed step 25 on checker formatting. XY-C1 `43907888399` and NVRAM `43907888090` were in progress at last poll.
- Checker-only parent 528da: USB storage `43908320511` was queued after an incomplete regex-based whitespace fix. THIS COMMIT uses built-in whitespace normalization and needs its own exact-head storage run.
- The previous product L3 on d6 does not transfer to later source changes. Physical-router L4 remains NOT VERIFIED.
## Exact first action for the next GPT

1. Inspect exact-head USB storage for THIS COMMIT; verify checker step 25 and the retry policy/source contracts pass. Read full logs if not.
2. Check XY-C1 and NVRAM runs on parent c1aca (`43907888399` and `43907888090`), and confirm whether Build/TRX/artifact completed on the same SHA.
3. Resume targeted storage/hotplug audit at automount/notification semantics only after confirming `notify_rc()` and `_eval()` contracts. Do not revisit finished mount-link/eject retry work.
4. Preserve the frozen backup branch; L4 remains unverified.
