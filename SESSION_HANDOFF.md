# Cross-session execution mandate — 2026-11-21

## Execution rules

- Continue on `main` only. Use one atomic main commit for each logical task: create all blobs, one complete Git tree and one commit, then update the ref non-forcibly with the exact expected parent SHA.
- Never amend, force-push, create development branches or PRs, or modify the frozen backup.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Add meaningful regression coverage; do not weaken checks.
- PENDING Actions do not block independent work. Diagnose relevant exact-head FAIL results before declaring a slice validated.
- Before context exhaustion, finish the current safe slice, publish atomically, and leave the exact first next action here.

## Mandatory handoff checkpoint — end-of-session accuracy

The checkpoint is mandatory; a document edit is conditional. At every session end, compare live `main`, relevant exact-SHA Actions, product-source baseline, verification boundary, blockers and first next action with this record. Update it only when resumable facts materially change; do not create a no-op commit.

## Current repository state

- Parent `main` immediately before this deferred-watchdog checker correction: `2c0f9ce2b316798109289cdabcdcb20c1d22c21d`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only and unchanged.
- Latest WAN production-source commit remains `7af88e6e0cf704717511efd808a758611d2fca2b` (`fix(wan): fail closed on modem type read errors`). Current commits since then align source contracts with checked production semantics; no WAN production code changed in the latest two corrections.
- Exact-head parent WAN route run `#44894284811` FAILED at step 32, `Verify deferred WAN reconnect watchdog launch status`, after preceding WAN policy binaries and source contracts (including checked SYN-cookie read and initial physical MTU checked-read ordering) passed. The stale checker required literal `return launch_status;`; actual `start_watchdog()` returns the rollback helper for launcher failures with the original launch status/errno and returns the rollback helper on readiness timeout. This commit aligns the checker with that fail-closed status behavior.
- Exact-head parent AI documentation run `#44894284801` PASS. The new commit must trigger fresh WAN/AI exact-head validation.
- WAN teardown source contract already passed on parent `751cb64ff459a1d5afebc836996dd43f810172f6` in run `#44894158502`; it validates checked modem-type selection before monitor pause and the NDIS interface-selection guard.
- XY-C1 firmware Build/TRX/artifact is not verified on a current production-source SHA. The last proven L3 baseline remains `20e139d230b73eabc5f25a2b17d56a3a5768af11`, exact run `#44887656164` SUCCESS for Build XY-C1, TRX verification and artifact upload. Physical-router L4 is NOT VERIFIED.
- The November development-log archive index and `docs/dev-log/2026/11/part-021.md` remain preserved; root `dev_log.md` retains its newest-first 20–119 entry window.

## Active task

Finish exact-head validation of the checked WAN NVRAM decision-path migration and the related source-contract suite.

Production already uses checked integer NVRAM reads for DNS, PPPoE/manual mode and demand, physical WAN/MAN MTU, EAPOL/Kabinet auth mode, MAN DNS routes, SYN-cookie policy and USB modem transport. Production source contracts have been brought into line for those migrations. The deferred reconnect flow checks deadline publication and propagates watchdog startup failure; the watchdog itself treats unknown process inventory as fatal, rolls back failed launcher/readiness startup, and retains the real status/error rather than silently returning success.

The outstanding work is exact-head CI convergence, followed by choosing the next still-open source defect. The first roadmap items for full-WAN-restart LAN-route restoration and `ifname_t` fail-closed runtime identity publication appear to have corresponding current source/test coverage; verify semantic checker and exact-head evidence before reopening either item.

## Verification boundary

- Parent WAN route run `#44894284811` reached step 32 after earlier source contracts and GNU89/Werror/ASan/UBSan policy binaries passed. It failed only because the watchdog checker demanded a direct `return launch_status;` instead of the production rollback helper. Updated requirement preserves the launcher error arguments and timeout rollback path, alongside unknown-inventory fail-closed behavior.
- Parent AI documentation run `#44894284801` PASS.
- Parent WAN teardown run `#44894158502` PASS.
- No local checkout/build runtime was available; no local Python/C test execution or firmware compile is claimed.
- Host regression success is L2 only. XY-C1 L3 requires Build, TRX verification and artifact upload to pass on one exact production-source SHA. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch live `main`, frozen backup and exact-SHA WAN route/AI documentation Actions. Fix the first causal current-head failure. If WAN route passes, verify the remaining WAN startup candidate (`ifname_t` publication/recovery/abort ordering) against the current source and its source contracts; if it is already complete, do not duplicate it and select a concrete independent P1 stability/security defect from the roadmap. For every source task, add regression coverage, commit one complete atomic main tree with expected-SHA non-forced update, and inspect Actions for that exact SHA. Do not wait on independent PENDING runs.

## Handoff maintenance

- Root `dev_log.md` should retain 20–119 newest-first complete records; move the oldest 100 only when the next entry would make 120.
- Keep `docs/dev-log/2026/11/part-021.md` and its index metadata byte-for-byte intact.
- Update `REFACTOR_ROADMAP.md` only when repository-wide module status, evidence or priority materially changes.
- Keep the frozen backup permanently unchanged.
