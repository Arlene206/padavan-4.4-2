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

- Parent `main` immediately before this checker correction: `751cb64ff459a1d5afebc836996dd43f810172f6`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only and unchanged.
- Latest WAN production-source commit remains `7af88e6e0cf704717511efd808a758611d2fca2b` (`fix(wan): fail closed on modem type read errors`). The current correction is checker/documentation only.
- Exact-head parent WAN route run `#44894158533` FAILED at `Verify physical WAN initial MTU status and static override`, after policy binaries, the DNS/PPPoE/MTU/auth/MAN route contracts, VLAN/firewall/DDNS/ZeroTier/APCLI checks and the network-sysctl checker passed. Its root cause was an index-space bug in `scripts/check_wan_initial_mtu_status.py`: a position from uncompressed source was used against a compacted function string. This commit uses raw function-body offsets consistently and verifies checked MTU read/error/continue ordering before default-MTU setup.
- Exact-head parent WAN teardown run `#44894158502` PASS; AI handoff docs run `#44894158457` PASS. New exact-head route and AI runs are required for this correction.
- The earlier teardown checker mismatch was fixed in the parent: run `#44894158502` verifies and passes checked USB modem transport selection before monitor pause and NDIS USBnet interface teardown. Do not re-audit the fixed stale getter assertion.
- XY-C1 firmware Build/TRX/artifact is not verified on a current production-source SHA in this checkpoint. The last proven L3 baseline remains `20e139d230b73eabc5f25a2b17d56a3a5768af11`, exact run `#44887656164` SUCCESS for Build XY-C1, TRX verification and artifact upload. Physical-router L4 is NOT VERIFIED.
- The November development-log archive index and `docs/dev-log/2026/11/part-021.md` remain preserved; root `dev_log.md` retains its newest-first 20–119 entry window.

## Active task

Finish exact-head validation of the checked WAN NVRAM decision-path migration and its corresponding source contracts.

The current production slice includes strict checked integer parsing/status preservation; fail-closed DNS-mode resolution before resolver publication; checked PPPoE manual-mode/demand, physical WAN/MAN MTU, EAPOL/Kabinet auth-mode and MAN DNS-route reads; checked global SYN-cookie policy before WAN activation; and checked modem transport selection across WAN startup, stop, manual disconnect and interface lookup. Missing-key defaults and legacy public API behavior remain compatible.

The recent CI-only corrections keep the source-contract suite aligned with those already implemented production decisions:
- network sysctl contract checks the checked `fw_syn_cook` read and fail-closed return before sysctl writes;
- WAN teardown contract checks checked modem transport selection before monitor pause and the NDIS interface-selection guard;
- initial physical MTU contract now uses raw source offsets consistently while requiring checked NVRAM read/error ordering before physical default MTU application.

Do not repeat those completed migrations or weaken their tests.

## Verification boundary

- Exact-head parent WAN route run `#44894158533` ran the GNU89/Werror/ASan/UBSan policy binaries successfully and passed the network-sysctl assertion; it failed at the initial-MTU Python source checker because of the coordinate-space defect now corrected.
- Exact-head parent WAN teardown run `#44894158502` PASS and AI documentation run `#44894158457` PASS.
- No local checkout/build runtime was available; no local Python/C test execution or firmware build is claimed.
- Host regression success is L2 only. XY-C1 L3 is valid only when Build, TRX verification and artifact upload all pass on the exact same production-source SHA. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch live `main`, frozen backup and exact-SHA WAN route, WAN teardown and AI documentation Actions. Fix the first causal current-head failure. If WAN route passes, inspect the first still-open P1 item in `REFACTOR_ROADMAP.md` against current source, tests and exact-head CI before choosing new work: do not duplicate the full-WAN-restart LAN-route restoration or `ifname_t` fail-closed work if their current code/tests already cover them. Then select a concrete independent stability/security defect, implement source plus regression, make one atomic main commit and inspect its exact-head Actions. Pending independent runs must not block other work.

## Handoff maintenance

- Root `dev_log.md` should retain 20–119 newest-first complete records; move the oldest 100 only when the next entry would make 120.
- Keep `docs/dev-log/2026/11/part-021.md` and its index metadata byte-for-byte intact.
- Update `REFACTOR_ROADMAP.md` only when repository-wide module status, evidence or priority materially changes.
- Keep the frozen backup permanently unchanged.
