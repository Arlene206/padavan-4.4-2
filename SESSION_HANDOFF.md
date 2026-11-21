# Cross-session execution mandate — 2026-11-21

## Execution rules

- Continue on `main` only. Use one atomic main commit for each logical task: create all blobs, one complete Git tree and one commit, then update the ref non-forcibly with the exact expected parent SHA.
- Never amend, force-push, create development branches or PRs, or modify the frozen backup.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Add regression tests; do not weaken checks.
- PENDING Actions do not block independent work. Diagnose the first causal failure and fix it.
- Before context exhaustion, finish the current safe slice, publish atomically, and leave the exact first next action here.

## Mandatory handoff checkpoint — end-of-session accuracy

The checkpoint is mandatory; a document edit is conditional. At every session end, compare live `main`, relevant exact-SHA Actions, product-source baseline, verification boundary, blockers and first next action with this record. Update it only when resumable facts materially change; do not create a no-op commit.

## Current repository state

- Parent main SHA before this WAN DNS read-failure slice: `f2920b4d320be4bbb7340673f1b29a9c7c0d2b62`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only and unchanged.
- XY-C1 L3 baseline: `20e139d230b73eabc5f25a2b17d56a3a5768af11`, exact run `#44887656164` SUCCESS for Build XY-C1, TRX verification and artifact upload. No later production-source SHA has a newer confirmed L3 result in this checkpoint; physical-router L4 is NOT VERIFIED.
- Process inventory passed on exact source SHA `421042f3a436e82eb73305ebab77161e6c3d6f17` (run `#44889271901`).
- Storage static checker and AI handoff wording/path/byte-index fixes are committed on main at `f2920b4d320be4bbb7340673f1b29a9c7c0d2b62`. Prior exact run #44889854793 had AI handoff PASS; Storage #44889854813 and XY-C1 #44889854764 were pending/in progress when last polled. The docs-only XY run `#44889614321` skipped Build/TRX/artifact and is host-only.
- Root `dev_log.md` retains a rolling newest-first history; 100 archived records live in `docs/dev-log/2026/11/part-021.md` (100 entries, 107,650 bytes, blob `f80bf2d087fa1c313cb671f2108e3279092db234`).

## Active task

The current source slice adds `rc_wan_nvram_parse_int_value()` and `rc_wan_nvram_read_int_status()` to `wan_nvram_status_policy.h`, keeping missing-key defaults separate from backend errors, and migrates DNS-mode resolution:
- `get_wan_dns_static_status()` uses checked `modem_dnsa` and `wan0_dnsenable_x` reads;
- public `get_wan_dns_static()` retains old boolean fallback on errors for API compatibility while setting errno;
- `update_resolvconf()` releases its lock and returns before resolver staging/publication when DNS mode cannot be verified, protecting the last-known-good resolver file;
- GNU89 policy tests, a Python source checker and workflow path/step wiring are included.

This is intentionally a narrow first WAN slice. Subsequent tasks: migrate PPPoE `pppoe_man` and `pppoe_idletime`, interface `mtu` / `man_mtu`, and EAPOL/Kabinet `auth_mode` reads one component per commit; then audit MAN DNS-route setup/teardown callsites. Preserve absent defaults and use fail-closed status where decisions affect service activation.

## Verification boundary

- No local checkout or C toolchain was available; do not claim local host tests or firmware compilation.
- Exact-head WAN route regression must run the GNU89/ASan/UBSan policy test and Python source checker. Process and AI handoff Actions also need exact-SHA validation.
- Host regression success is not firmware L3. XY-C1 L3 is proven only on a SHA where Build, TRX verification and artifact upload all pass on that exact SHA; current recorded L3 baseline is `20e139d2...`.
- Physical router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch live `main` and frozen backup SHA; inspect exact-head WAN route, Process, AI docs, Storage and XY-C1 Actions for this commit. Fix the first causal CI failure without weakening checks. If WAN CI passes, continue with PPPoE manual-mode/demand reads in `start_wan()`: ensure backend errors abort startup and restore WAN state while a truly absent key retains the old zero behavior. Then separately migrate MTU and authentication reads. Pending Actions must not block independent work.

## Handoff maintenance

- Root `dev_log.md` should retain at most 20 newest-first complete records; move the oldest 100 only when the next entry would make 120.
- Archive `docs/dev-log/2026/11/part-021.md` and its index metadata must remain intact.
- Update `REFACTOR_ROADMAP.md` only when repository-wide module status/priority materially changes.
- Keep the frozen backup permanently unchanged.
