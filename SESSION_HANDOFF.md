## 2026-11-28 continuation checkpoint

- Repository: `Arlene206/padavan-4.4-2`. Continue on `main`; bootstrap and branch inventory were restored once. Do not repeat that audit.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. No work, reset, rename, CI or force-push on that branch.
- Latest main before this CI repair: `5fc78cc0e312610ef27c61e42fb80d42b0de520a`, parent `670e513d99fa0e2d48dfc382ea795ab5c8f6ba1a`. Latest production-source commit is `670e513d`: V2 Samba journal with EINTR-safe state read and O_RDWR regular-file fsync. `5fc78cc` adds the COMMITTED-before-cleanup test only.
- PR #1 remains closed and unmerged; the AliDDNS CA trust-store work is already on main.

## Active task

- Complete Samba V2 four-file journal tests and align static source contracts with the current implementation. `run_samba()` journals `/etc/passwd`, `/etc/group`, private passdb and `smb.conf`; V2 PREPARED rollback restores all four and COMMITTED recovery retains the new generation.
- Exact-head 5fc action #46145701231 failed in the Samba host C regression: the newly added COMMITTED-before-cleanup scenario leaves the marker generation active, while later first-boot and V1 recovery assertions still expected the older `committed` strings. Fix those expected baselines. The same run's exact XY-C1 #46145701246 failed on the same C test before firmware build.
- Samba exact-head #46145600741 also exposed a stale inline assertion expecting two `write_smb_conf_unlocked()` calls inside `run_samba()`; current function contains exactly one transactional call. The dce5 HTTPD guard also still searched for the V1 `recover()` API inside `run_samba()`. The standalone durability checker retained V1 state-read/abort/begin assumptions. Fix tests/contracts; do not revert production V2.

## Exact first action for the next GPT

1. Fetch live main and exact-SHA Actions for the checker/test repair. Require Samba host test, embedded source guard, standalone durability checker, HTTPD identity guard, and XY-C1 transaction regression to pass.
2. Check exact-SHA XY-C1 Build XY-C1, TRX validation and artifact upload; claim L3 only if all three succeed.
3. Poll exact-head NVRAM transaction audit #46145701242 (was in progress); Process inventory #46145701238 and AI handoff #46145701248 passed on 5fc. Do not repeat completed broad inventories.
4. Once Samba transaction CI is green, continue a separate source-backed stability/security fix. Keep the frozen backup immutable.

## Exact-SHA verification boundary

- Main `5fc78cc0e312610ef27c61e42fb80d42b0de520a`: Samba share #46145701231 FAIL (C transaction regression, marker generation baseline mismatch); XY-C1 #46145701246 FAIL at `Compile and run Samba parent-directory security regression` because it runs the same C transaction regression. AI handoff #46145701248 PASS; Process inventory #46145701238 PASS; NVRAM transaction audit #46145701242 was IN_PROGRESS at last check.
- Main `670e513d99fa0e2d48dfc382ea795ab5c8f6ba1a`: Samba share #46145600741 FAIL in stale writer-call count guard; AI handoff #46145600769 PASS; Process inventory #46145600796 and Process lifecycle inventory #46145600779 were running; NVRAM audit #46145600813 was running; XY-C1 #46145600745 was in progress before the newer 5fc run.
- `dce5f27b1787abbf83a712159358215839c3cadc` introduced V2 four-file journal; `670e513d` hardened V2 state-read EINTR handling and file fsync descriptor mode. No L3 claim for either until exact-SHA Build, TRX and artifact steps pass.
- Existing earlier exact-SHA results remain historical: Samba snapshot SHA `9ffd787f...` had XY-C1 #46143284111 PASS; DDNSTO `3f1cab20...`, Aria2 `76646cff...`, PPPoE-MWAN `300e78b2...`, firmware first-errno `19d86adc...` and all-stop `80562419...` had the specific exact-SHA CI results documented in dev_log.
- Physical router/hardware L4: NOT VERIFIED. No local MIPS/kernel build or router hardware is available.

## Mandatory end-of-session handoff checkpoint

Mandatory handoff checkpoint. The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live repository state and exact-head Actions before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. Update this file when the active task or first action materially changes; do not create a documentation-only commit merely to record routine CI transitions.

## Cross-session execution mandate

- One logical task, one atomic main commit: create all blobs, the complete tree, one commit, then a non-forced expected-SHA update. Never force-push or publish per-file partial commits.
- Before context exhaustion, finish safe source slices, inspect exact-head Actions, and leave the next GPT an accurate first action and L1-L4 verification boundary.
- Keep working while independent Actions are pending; PASS continues, FAIL requires causal repair, PENDING permits independent work.
- Recover once, implement continuously, do not repeat completed audits, and do not ask what to do next.
