## 2026-11-28 continuation checkpoint

- Engineering source baseline: `ac811878adf63c4686af0acb59c0d11a26076133` introduced the PID 1 `init_mdev` bounded fsync migration. The correction in THIS COMMIT removes its stale unused `sync_result` local (reported by exact-head GNU89/Werror compile) and prevents the persistent-interruption test mode from driving its transient counter negative.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. Its warning file has been read.
- PR #1 is closed and unmerged (head `ba48e22609e0c81ad757b06756fe91d05f39cbf6`). Its 13 changed paths exist on main; two AliDDNS Makefiles remain byte-identical, while the other paths have evolved on main. No missing file path requires a blind port.
- Branch inventory contains only `main` and the named frozen backup. All development is main-only.
- The DHCPv6 bounded-fsync task in the former handoff is complete: source `5a8a417a975547d6537e7e566ee97e36c2c910f1`, test follow-up `73a2a4a151b97524ade60b68a85bcd6072621209`.
- The recent MiniUPnP/vsftpd malformed-YAML issue was fixed in `9811a97ddfc7531dbf55750c72cdb1c20fb222e7`; exact SHA MiniUPnP `46134521844`, vsftpd `46134521842`, and AI docs `46134521904` PASS.

## Current repository state

- Last fetched `main` parent for this correction: `9ffa705ebf992cb803b17cee327e013313112350` (`fix(l2tp): bound config fsync EINTR retries`). Live main must be fetched again on takeover.
- At source SHA `ac811878...`: mdev Actions `46134597261` failed before running the test because the unused local triggered `-Werror`; Process inventory `46134597289` PASS; AI docs `46134597323` PASS; NVRAM `46134597288` and XY-C1 `46134597343` were still running at last poll.
- At concurrent main `9ffa705ebf992cb803b17cee327e013313112350`, L2TP config durability `46134722346`, VPN server lifecycle `46134722365`, Process inventory `46134722339`, AI docs `46134722336`, and XY-C1 `46134722369` were in progress; NVRAM `46134722355` was pending. These run IDs apply only to 9ffa.
- No local checkout/host build was available. `REFACTOR_ROADMAP.md` remains near its hard size limit; avoid editing unless global priorities materially change.

## Active task

Finish validating PID 1 `mdev.conf` publication durability:
- `trunk/user/rc/src/init_mdev_config_file.h`: the implementation uses shared `config_file_sync_fd(fileno(fp))`; remove the obsolete local left from the previous raw fsync retry loop.
- `trunk/user/rc/src/tests/init_mdev_config_file.c`: retain the injectable `CONFIG_FILE_SYNC_FSYNC`; in persistent EINTR mode, only decrement the transient retry counter when it is positive.
- The host test checks two transient interruptions, persistent EINTR bounded at initial call + eight retries, no rename/parent sync on file-sync failure, stage cleanup, and active policy preservation.

## Exact first action for the next GPT

1. Fetch live `main` and confirm the result SHA of THIS COMMIT.
2. Inspect exact-head `PID 1 mdev hotplug configuration security regression`, Process inventory, NVRAM audit, AI docs and XY-C1 validation. Do not count the parent mdev failure as a result for the corrected source SHA.
3. Repair any new exact-head failure causally in a separate atomic main commit; PENDING runs do not block independent work.
4. Once mdev passes, continue another P1 stability/security task from the whole-repository roadmap; avoid repetitive fsync cleanup if a higher-impact unaddressed module is ready.
5. XY-C1 L3 is NOT VERIFIED until the matching source build, TRX validation and artifact upload all PASS. Physical router L4 is NOT VERIFIED.

## Verification boundary

- L1: exact compiler failure was the unused local; the source fix removes only that declaration. Test fixture change preserves a clean separation between transient retries and persistent EINTR mode.
- L2: NOT VERIFIED for this correction until exact-head mdev Actions run to completion. No local host compile was performed.
- L3: NOT VERIFIED; exact-source XY-C1 target build/TRX/artifact evidence is still required.
- L4: NOT VERIFIED; no physical router validation was performed.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Before ending, inspect the live repository state, exact-head Actions, active task, validation boundary and first next action; update this file only when those facts materially change. Do not create a no-op checkpoint commit.

Before ending, compare live main, exact-head Actions, active task, validation boundary and exact first action. Update this file only when those facts materially change; no no-op checkpoint commit.

# Cross-session execution mandate

- Recover once, then execute; do not repeat completed audits or ask the user what to do next.
- One logical multi-file task, one atomic main commit: create all blobs, one tree and one commit, then update main with a non-forced expected-SHA compare-and-swap. Never force-push or publish per-file intermediate commits.
- Before context exhaustion, finish the current safe slice, publish it, inspect exact-head Actions and leave the next GPT a precise first action and truthful L1–L4 validation boundary.
- Inspect exact pushed-main Actions. PASS permits the next slice; FAIL requires causal repair; PENDING permits independent work.
- Complete a bounded safe slice before starting another. Do not claim tests, firmware artifacts or hardware validation without evidence.
