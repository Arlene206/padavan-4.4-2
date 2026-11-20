# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Restore exact live source and CI state once, then resume concrete engineering tasks. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits or stop at planning.

- Create all changed blobs, one complete Git tree and one commit with current main as parent; update main using non-forced compare-and-swap with that exact expected SHA.
- Verify Actions by exact `head_sha`. Pending runs do not block independent work; inspect and repair real failures.
- Preserve security, architecture, GNU89/MIPS/uClibc compatibility, regression coverage, lifecycle identity checks, bounded I/O and fail-closed UNKNOWN handling.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` is archival only; never alter it. Develop on main.

## Current repository state

- PID 1 degraded-readiness implementation is committed on main as `9ad1570aa0bc8a9e1c5ff26a85266d53043c526a`. Its GNU89 ASan/UBSan policy binary and existing `dev_init.sh` shell regression passed in the Process job; the subsequent first-attempt Python source contract failed on the C-function parser buffer reference.
- CI-contract follow-up `e8fb566f3a122f75c4f648543c2a7aa516b00316` corrected that parser and the HTTPD three-state boot-result source assertions. Exact-head Process `44643316833`, HTTPD `44643316983` and AI handoff docs `44643316847` all PASS on this SHA.
- XY-C1 firmware run `44643316929` on the e8 checker/workflow SHA remained IN_PROGRESS at the last query. The prior production-source SHA `9ad1570` XY run `44642485538` was also still IN_PROGRESS; re-query exact source SHA before reporting L3.
- Earlier rp-L2TP product SHA `2b646cccdcb24da5d447586893a500fdcf3cfe48` passed XY-C1 Build/TRX/artifact in `44639411601`; artifact `14584429568`. This does not verify the newer PID 1 source.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, archival and untouched.

## Active task

- P1 PID 1 startup-result propagation: `init_router()` maps the four mandatory NVRAM/configuration aborts to FATAL, clean service startup to READY and accumulated module/service failure to DEGRADED. PID 1 keeps notifications/deferred hotplug active for READY/DEGRADED only.
- The next concrete caller-side defect is at the end of `init_router()`: it previously logged but discarded `copyscripts.sh`'s explicit CA trust-store failure status, and also discarded the wait/launch statuses for the persistent user script and firmware `autostart.sh`. The in-progress follow-up merges all three into `boot_services_status` and moves the aggregate diagnostic after these launch attempts. The background script statuses describe launcher/wait status only, not child-service readiness.
- A shell regression exercises CA archive extraction success, corrupt archive, missing CA bundle and uncreatable destination; source contracts verify all three statuses reach the first-error boot aggregate. Local shell regression passed against the extracted production helper.
- Source/test commit `61cbc6b842fdaaafc3f2fcab269bb11292fa8514` is on main. Its CA helper regression passed in the Process job, including corrupt archive, missing bundle and uncreatable destination cases. Exact-head Process `44644448788` then failed only because its checker normalized whitespace but expected a non-normalized call; AliDDNS `44644448746` retained the prior `system() != 0` expression; Storage `44644448682` expected the previous inline return rather than the new, separately tested `storage_hotplug_completion_status()` helper. This checker-only follow-up repairs those three contracts without weakening test coverage.
- On `61cbc6`, HTTPD `44644448461`, AI handoff docs `44644448860`, WAN route `44644448877` and other completed subsystem regressions are PASS; Process `44644448788`, AliDDNS `44644448746` and Storage `44644448682` are the three identified contract failures. XY-C1 `44644448761` and NVRAM `44644448560` were IN_PROGRESS at last poll.
- Checker repair commit `5ebdf7977d3abf53ca5f3514b76f8a59b64af294`: exact-head Process `44645368007` and AliDDNS `44645368089` PASS. Storage `44645368071` exposed a stale direct-`notify_rc()` assertion, corrected on `479516c` to the actual checked queue API.
- NVRAM audit `44645672301` on `133ccb9ed03ebd2812ba76b3cb5e0eb175582d56` PASS, including lock/snapshot/ioctl/device-node and the explicit PID1 FATAL prerequisite / READY-DEGRADED status source contract. AI docs `44645672267` also PASS.
- Storage rerun `44645869851` on `479516c` passed the broad caller graph and queue-status contracts, then failed only because `check_storage_safe_remove.py` searched for literal backslash-n instead of a real newline when locating the checked queue helper. This commit fixes the parser needle.
- Production-source XY-C1 `44644448761` on exact source SHA `61cbc6b842fdaaafc3f2fcab269bb11292fa8514` completed SUCCESS: detector, Build XY-C1, TRX verification and artifact upload all PASS. L3 is VERIFIED for this source SHA; it does not imply physical-router L4. Checker-only commits after it do not change the built production source.

## Exact first action for the next GPT

1. Validate the storage parser fix on its exact-head rerun; NVRAM `44645672301` and XY-C1 Build/TRX/artifact `44644448761` are now PASS on their stated SHAs.
2. Continue the broad RC startup-result propagation P1 queue at the next independent caller defect; do not repeat the closed CA-preparation/RC caller or SD/MMC queued-notification fixes.
3. Require a fresh exact-source XY-C1 run for any subsequent production-source change; current `61cbc6b` L3 evidence does not transfer to later source commits.

## Verification boundary

- L1: PID 1 distinguishes FATAL/READY/DEGRADED and continues event processing after degraded service boot. Late CA preparation, persistent user-script and autostart launcher failures are now included in the boot aggregate.
- L2: PID 1 policy binary and `dev_init` shell regression passed on `9ad1570`; the new CA status shell regression passed on `61cbc6`. Exact-head Process, AliDDNS and Storage jobs initially failed only at three stale source-contract assertions; their corrections are in this checker follow-up. HTTPD, AI docs and all other completed current-head regression runs were PASS.
- L3: XY-C1 `44644448761` on `61cbc6` was IN_PROGRESS at last poll; no Build/TRX/artifact result is claimed until all three exact-head steps succeed.
- L4: target MIPS/uClibc runtime and physical-router boot/recovery behavior remain NOT VERIFIED.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST perform a handoff checkpoint before ending. At session end, compare live main, production-source baseline, exact-head Actions, module state and the next task. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit; do not create a no-op commit.

## Before context exhaustion

- Finish the current safe source/test slice and commit it atomically before handoff; do not publish related multi-file changes as sequential partial commits.
- Re-fetch live `main`, create one complete Git tree and one commit with that exact parent, then update main only with `force=false` and the exact parent as `expected_sha`. Never force-push.
- Query relevant Actions by exact source SHA and record PASS/FAIL/PENDING with run IDs. Pending validation does not block independent work and must not be reported as complete.
- Record the production-source baseline, test commands/results, exact CI state, unresolved limitations and the next concrete task. Distinguish host L2, firmware L3 and device L4 evidence.
- Confirm all work is committed and remotely present before claiming it is preserved across conversations.

## Development invariants

- Main-only; never alter the frozen backup or develop on a legacy PR.
- One complete tree and one atomic commit per logical task; `force=false`, exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits or weaken tests.
- Keep L1/L2/L3/L4 distinct. Host tests are not firmware or hardware validation.
