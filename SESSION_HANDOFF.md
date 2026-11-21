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

- Parent `main` SHA before this checker/continuity correction: `7af88e6e0cf704717511efd808a758611d2fca2b`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only and unchanged.
- Latest WAN production-source commit is `7af88e6e0cf704717511efd808a758611d2fca2b` (`fix(wan): fail closed on modem type read errors`). It adds checked modem transport selection for full stop, manual disconnect and `get_wan_ifname()`; unreadable state is propagated instead of being guessed as legacy RAS.
- Exact-head parent WAN route run `#44893837491` was in progress when inspected; its job had already failed at step 29, `Verify WAN startup and reload network sysctl status propagation`. The checker still expected `tcp_syncookies_enabled = nvram_get_int("fw_syn_cook")` although production uses `get_nvram_int_checked_status()` and returns on a read failure before sysctl writes. The same stale assertion was the confirmed cause of WAN run `#44893645481` on the preceding parent. This commit updates the integration contract.
- On parent `7af88e6e0cf704717511efd808a758611d2fca2b`, AI handoff docs run `#44893837451` and XY-C1 run `#44893837554` were in progress. Other source-dependent workflows were also pending; inspect their exact-head status rather than waiting idly.
- XY-C1 L3 baseline remains `20e139d230b73eabc5f25a2b17d56a3a5768af11`, run `#44887656164` SUCCESS for Build XY-C1, TRX verification and artifact upload on that exact source SHA. Later production-source changes have no newer confirmed same-SHA L3 result in this checkpoint. Physical-router L4 is NOT VERIFIED.
- The November development-log archive index and `docs/dev-log/2026/11/part-021.md` remain preserved. Root `dev_log.md` remains a newest-first rolling window; the new record is within the 20–119 entry rule and far below the size ceiling.

## Active task

Complete exact-head validation of the WAN checked-NVRAM decision paths and integration contracts.

The current WAN source migration includes:
- strict checked integer parsing/status preservation through `wan_nvram_status_policy.h`; absent-key defaults remain compatible, while malformed/out-of-range values and backend errors remain distinct;
- checked DNS-mode reads, with `update_resolvconf()` failing closed before resolver staging/publication when the mode cannot be verified;
- checked PPPoE manual-mode/demand, physical WAN/MAN MTU, EAPOL/Kabinet authentication-mode, and MAN DNS-route setup/teardown reads;
- checked global SYN-cookie policy read before WAN setup side effects, with the exact sysctl write status retained separately;
- checked USB modem transport-type reads in startup and shutdown/interface-selection paths, including defensive teardown when the mode cannot be read.

Legacy public accessors remain for compatibility; internal WAN decisions have been migrated away from error-collapsing integer reads. The current slice only aligns the remaining network-sysctl source contract with the implemented checked-read semantics. Do not repeat completed migrations.

## Verification boundary

- Parent WAN run `#44893837491` had reached the network-sysctl source checker after the GNU89/Werror/ASan/UBSan policy binaries and earlier WAN DNS, PPPoE, MTU, auth, MAN route and associated service contracts passed; step 29 failed on the stale `nvram_get_int()` expectation. The integration assertion is now updated to require checked `fw_syn_cook` read, fail-closed return ordering, and exact status capture for the sysctl write.
- Exact-head WAN run `#44893645481` on the preceding parent independently confirmed the same stale assertion. The new commit must trigger a fresh WAN route run.
- No local checkout or local build environment is available in this session; no local test or target compile is claimed.
- Host regression success is L2 only. Current WAN production code is not covered by the older L3 baseline as a same-SHA build. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch the live `main` head and inspect WAN route, AI handoff docs, XY-C1 and the other relevant push Actions for that exact SHA. Fix the first causal WAN route failure if one remains; do not wait for pending independent checks. Once the WAN route contract passes, resume the first still-open P1 roadmap item: verify and fix full-WAN-restart recovery of static LAN routes when the checked runtime WAN-protocol snapshot fails after the earlier route flush. Inspect the current implementation and callers, preserve the primary protocol-staging error while aggregating route-recovery failure, add/retain a meaningful regression, then publish one atomic main commit and inspect its exact-head Actions.

## Handoff maintenance

- Root `dev_log.md` should retain 20–119 newest-first complete records; move the oldest 100 only when the next entry would make 120.
- Keep `docs/dev-log/2026/11/part-021.md` and its index metadata byte-for-byte intact.
- Update `REFACTOR_ROADMAP.md` only when repository-wide module status, evidence or priority materially changes.
- Keep the frozen backup permanently unchanged.
