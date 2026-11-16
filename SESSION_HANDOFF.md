# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect live repository state and compare it with this file before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers and precise first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave this file untouched and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task here.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, and do not ask the user to choose the next task.

Every multi-file logical change must be created as one complete Git tree and one atomic main commit parented to freshly fetched main, then update main with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, or modify the frozen backup branch.

## Current repository state

- Latest main commit: `c1aca046300ec7738a3455c779afb988f5d71cb6`. Targeted-eject product code and GNU89 policy test passed USB storage steps 18–24 on exact SHA; run `43907888057` failed only at step 25 because the checker compared a wrapped C helper call without whitespace normalization. THIS COMMIT fixes the checker contract. On c1aca, Process lifecycle `43907888085`, AI docs `43907888032`, HTTPD security `43907888445`, Samba `43907888436`, NFS `43907888144`, vsftpd `43907888072`, modem identity `43907888115`, infosvr `43907888362`, AdGuardHome `43907887961`, AliDDNS `43907888339` and AliyunDrive `43907888387` passed. XY-C1 `43907888399` and NVRAM `43907888090` were in progress at last poll. Prior product SHA d6 passed XY-C1 Build/TRX/artifact; physical-router L4 remains unverified.

- Repository: `Arlene206/padavan-4.4-2`; current source parent before this slice: `e05ccbcb8c458ee2bc07c6020ed5af52b3d4cff8`.
- Mount-link errno fix, GNU89 policy regression, storage source contracts, and workflow corrections are included together in THIS COMMIT.
- Frozen backup branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable and read-only.
- Storage application startup returns first errors from all seven configured services; media/download daemon launch and readiness errors flow through the common startup result, and deferred hotplug preserves retry events until success.
- `create_mp_link()` retains its boolean API but now initializes errno, normalizes mount inventory/path/directory/symlink/cleanup failures, keeps the first actionable error, and exposes `create_mp_link_status()` before all four media callers. MiniDLNA/Firefly retain the 0→1 directory fallback; Transmission/Aria retain mode 0 only.
- USB modem hotplug preserves coalesced unplug/replug events and retries failed reconnects; WAN IPv6 and LAN/Wi-Fi startup paths propagate failures.

## Active task


The current targeted-eject product slice is in parent `c1aca046300ec7738a3455c779afb988f5d71cb6`: helper policy and regression steps 18–24 passed, while USB storage run `43907888057` failed only because the source checker expected a helper call on one line although the C source wraps the argument list. THIS COMMIT normalizes whitespace in the checker assertion; product source is unchanged. Re-run the USB storage workflow on THIS COMMIT and verify the policy, source contract and full caller graph all pass.

The earlier checker hardening and all other exact-head checks listed above passed on parent c1aca. XY-C1 run `43907888399` was in progress and NVRAM `43907888090` was in progress at last poll. Recheck the relevant exact-SHA results rather than carrying results to this checker-only commit.

## Verification boundary


- Parent c1aca: USB storage `43907888057` passed BusyBox and storage policy/graph steps 1–24, then failed at step 25 solely on exact whitespace in the new source assertion; this commit normalizes whitespace and needs exact-head rerun.
- Parent c1aca: Process `43907888085`, AI docs `43907888032`, HTTPD `43907888445`, Samba `43907888436`, NFS `43907888144`, vsftpd `43907888072`, modem `43907888115`, infosvr `43907888362`, AdGuardHome `43907887961`, AliDDNS `43907888339`, AliyunDrive `43907888387` passed. XY-C1 `43907888399` and NVRAM `43907888090` were in progress at last poll.
- Product SHA d6 exact USB storage and XY-C1 Build/TRX/artifact both passed, but later c1aca product changes invalidate L3 carry-forward. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT


1. Inspect exact-head USB storage for THIS COMMIT and confirm the whitespace-normalized targeted-eject assertions pass. If another failure appears, read full logs and fix its first cause.
2. Inspect XY-C1 Build, TRX and artifact upload for product source SHA `c1aca046300ec7738a3455c779afb988f5d71cb6` (run `43907888399`) and NVRAM run `43907888090`; do not carry d6 L3 across source changes.
3. After exact-head checks are green, continue storage/hotplug lifecycle work on a new concrete defect. Preserve backup branch and physical-router L4 boundary.
