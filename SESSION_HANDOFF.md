# 2026-11-18 continuation checkpoint — complete MiniUPnP lifecycle contracts

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; parent main before THIS correction: `654c146b2d0dcf5cbb1187a544ddaf429edc8c55`.
- Development remains main-only. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- MiniUPnP production lifecycle serialization is implemented in `03a993e558b23f5f163ca1a904cbf7b54f6ef7e9`: stable private locking covers public start/stop/WAN check/update/restart wrappers, which invoke internal `*_locked` operations.

## Active task
- Exact e74 Process `44194600337` and MiniUPnP `44194600347` both passed their prior source checks but failed because their public restart contract still required a direct `return ret;`; the lock-owning wrapper must return `miniupnpd_lifecycle_lock_finish(lock_fd, ret, "restart")`.
- Commit `654c146` adjusted the restart error predicate in the dedicated workflow but did not remove the stale direct-return assertion. Exact 654 MiniUPnP run `44195030330` consequently failed at the same assertion. THIS correction updates it and the duplicated Process checker contracts: all public start and restart checks now inspect the locked implementation/callback and the wrapper's lock-release result.
- The dedicated `scripts/check_miniupnpd_lifecycle_lock.py` independently verifies acquire → locked operation → release for start, stop, WAN check, update and restart. No production runtime source is changed in THIS slice.

## Verification boundary
- Exact 654 MiniUPnP run `44195030330` FAIL at the direct-return assertion. Exact 654 AI docs run `44195030309` was still running when last queried.
- Exact e74 AI docs `44194600326` PASS; exact e74 Process `44194600337` failed at the same stale direct-return assertion. On exact `88770d7`, NVRAM `44193165743` completed SUCCESS; XY-C1 `44193165931` had passed all pre-build gates and entered Build XY-C1, with TRX/artifact pending at last recorded poll.
- Do not claim L3 until the exact product-source SHA completes Build, TRX verification and artifact upload. Physical-router L4 remains NOT VERIFIED. No local checkout/toolchain is available for local compilation.

## Exact first action for the next GPT
1. Inspect exact-head Process, dedicated MiniUPnP and AI handoff runs for THIS correction; require both source contracts and the dedicated lock checker to pass.
2. Recheck XY-C1 `44193165931` for Build/TRX/artifact completion.
3. Continue the next independent P1 reliability/security slice without repeating the MiniUPnP audit.

## Mandatory handoff checkpoint
Mandatory handoff checkpoint: inspect live main, active tasks, exact-SHA Actions and validation limits before ending. The checkpoint is mandatory; a document edit is conditional. If accurate, do not create a no-op documentation commit.

# Cross-session execution mandate

Continue one engineering effort across GPT conversations. Recover once, then execute. Do not repeat completed audits, wait idly on PENDING Actions, or ask the user what to do next. Implement source fixes with regression tests and validate exact main SHAs.

Every logical multi-file change must use Git data APIs to create all blobs, one complete tree and one atomic main commit, parented to freshly fetched main and published with non-forced compare-and-swap using the expected SHA. Never force-push, create branches/PRs, or modify the frozen backup.

Before context exhaustion, finish safe work, commit atomically, inspect exact-SHA Actions and leave a precise next action.
