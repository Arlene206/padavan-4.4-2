# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect live repository state and complete an end-of-session accuracy checkpoint. Record exact main SHA, exact-SHA Actions, test/build boundaries and the next precise task. If facts remain accurate, explicitly conclude with no file change and no no-op commit. Use one atomic main commit for material changes only. Before context exhaustion, leave an accurate handoff and complete any safe bounded engineering slice.

- Work only on `main`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update `main` only with `force:false` and a freshly fetched expected SHA.
- PASS continues, FAIL requires diagnosis/correction, PENDING does not block independent work.
- Do not claim local tests, production builds, TRX/artifacts or hardware checks that were not performed.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Parent main SHA immediately before this atomic slice: `b794d511f0a70d77cc046090b39c416f89c64006`. This commit advances only `main`; fetch the live head before resuming.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; it remains immutable and must never be modified or deleted.
- The only remote branches are `main` and that explicitly preserved backup. Legacy PR #1 is closed and unmerged; do not use its branch as a workspace.
- Exact parent-main push checks: Process lifecycle inventory #45058824811 PASS and Adbyby configuration stability #45058824804 PASS on `b794d511f0a70d77cc046090b39c416f89c64006`.
- XY-C1 run #45058706199 is for product-source commit `2b41f5aa8d322def2cc22621b09fc2fdf4687367`, an ancestor of the parent main. The firmware detector passed and Build XY-C1 was in progress at last inspection; TRX and artifact upload were pending. The next main commit below changes production storage script input, so re-check the fresh run for the new exact source SHA.
- Historical XY-C1 run #45057859268 passed Build, TRX verification and artifact upload on `ae53025f281eef66057fde793731ee06477822a2`; this is not current L3 evidence because later product-source changes landed in `2b41f5a`.
- Storage workflow #45058110639 passed on ancestor `ca8e79ec28c4f2fe4a20be62b1d0766f9c4ba283`, including the filesystem-module lock, storage caller-graph and mount-inventory regressions. This new swap-inventory regression requires a fresh exact-head storage run.
- No local checkout or hardware device is available in this execution. L4 hardware validation remains NOT VERIFIED.

## Active task

The prior passwd/group/shadow two-phase publication task is already implemented on main. `recreate_passwd_unix()` retains hard-link recovery points across `change_passwd_unix()`, rolls back in reverse order on password-install failure and finalizes the links only after success. The GNU89/ASan/UBSan regression and Process source contract passed on exact SHA `0502c7ab3b3ad32c0682a0c372b79789174bfe7e` in run #45052382237. Do not repeat that task.

This slice fixes a separate USB storage stability gap in `trunk/user/scripts/files/sbin/automount.sh`: the swap hotplug path previously interpreted every nonzero `awk` result from `/proc/swaps` as “target absent,” so unreadable, empty or malformed inventory could still reach `swapon`. It now uses `automount_swap_status()` to validate the five-column header and every complete row, returning ACTIVE, INACTIVE or UNKNOWN. Only a complete INACTIVE result permits activation; UNKNOWN refuses it, and a failed `swapon` is logged. `storage_mountpoint_inventory.sh` covers active/inactive/exact-path and malformed/empty/unreadable cases plus the caller's lock and swapon ordering.

First verification step: inspect exact-head storage, AI-handoff and XY-C1 Actions for this commit; resolve any failure using its first causal log. Do not infer L3 until Build XY-C1, TRX verification and artifact upload all pass on the same production-source SHA.

## Verification boundary

- Parent main immediately before this source/test/documentation slice: `b794d511f0a70d77cc046090b39c416f89c64006`; the resulting main SHA is the live commit containing this entry and must be resolved from GitHub on the next recovery.
- Source change: `automount.sh` now fails closed on incomplete or malformed `/proc/swaps`; the existing storage-hotplug workflow already includes the source and regression test paths in both push and pull_request filters, so no trigger semantics were changed.
- Regression coverage: `storage_mountpoint_inventory.sh` exercises active, inactive, exact-path, malformed header/row, invalid numeric fields, empty inventory, missing file and non-file read failure; it also checks that only INACTIVE reaches `swapon`.
- Local test state: NOT RUN; no repository checkout was available. Exact-head Actions after this commit are the L2 evidence source.
- Before this slice, exact parent checks #45058824811 (Process inventory) and #45058824804 (Adbyby) passed on `b794d511f0a70d77cc046090b39c416f89c64006`; neither verifies the new automount code.
- XY-C1 #45058706199 was pending on older product-source `2b41f5aa8d322def2cc22621b09fc2fdf4687367`. It cannot verify this new automount source. The new exact-head XY workflow must pass detector, Build XY-C1, TRX and artifact upload before L3 can be marked VERIFIED.
- L4 device behavior is NOT VERIFIED.

## Exact first action for the next GPT

Fetch live `main` and query Actions runs for that exact SHA. Inspect the storage-hotplug regression, AI handoff documentation and XY-C1 workflow; match every run's `head_sha`, inspect any failed job's first causal log, and do not wait idly on pending checks. Confirm Build XY-C1, TRX verification and artifact upload all succeeded on the same new production-source SHA before claiming L3. Once those checks are clear, continue the storage lifecycle review at `trunk/user/libdisk/src/disk_initial.c`: trace `try_device_swapoff()` and the global cleanup path through their hotplug/eject callers, focusing on incomplete inventory and swapoff failure handling; add a regression only for a new independent defect.

