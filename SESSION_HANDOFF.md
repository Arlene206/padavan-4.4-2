# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state and compare it with `SESSION_HANDOFF.md` before ending or handing off. Verify the engineering/source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave the file untouched and make no no-op commit.

Before context exhaustion, finish or reduce the current safe slice, check exact-head Actions, commit any completed work atomically and preserve the precise first next action here.

Continue this engineering task across ChatGPT conversations. Recover repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup. Record exact Action results and the next concrete task before context exhaustion.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- Product-source baseline under validation: `b715097b8fdb76ed1e96bdeed6bedafde47eeefd` (`fix(lan): verify required bridge module loads`). The CI-contract repair now being committed has no firmware C change.
- Frozen archival branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable and read-only. One-time startup comparison is already recorded; do not repeat the archive recovery.
- The branch inventory contains only `main` and the frozen backup. Legacy PR #1 is closed and unmerged; its history was inspected, but it is not an active development target.

## Active task

The LAN bridge module-load slice is implemented on product-source baseline `b715097b8fdb76ed1e96bdeed6bedafde47eeefd`:

- `init_bridge_load_module_verified()` reuses `rc_module_load_verified()`, whose post-load `/sys/module` probe avoids treating `module_smart_load()`'s ambiguous zero return as proof of success.
- Required `iNIC_mii` load status is included in `init_bridge()` first-error aggregation and therefore PID 1 boot health on `USE_RT3352_MII` boards.
- Optional `hw_nat` bridge offload is post-load verified and logged accurately; its absence remains non-fatal.
- The Process workflow compiles/runs the existing GNU89/Werror/ASan/UBSan module-load policy regression and checks both required/optional production call sites.

The active continuation is to close the two exact-head CI contract failures below, then continue the RC initialization audit for unchecked required module loads and startup operations. Keep required hardware readiness separate from optional acceleration/LED behavior.

## Verification boundary

On product-source SHA `b715097b8fdb76ed1e96bdeed6bedafde47eeefd`, push-triggered Actions observed during recovery:

- Process lifecycle regression `43859397503`: FAILED at the final LAN startup/source-contract step (step 39, embedded Python line 107); all preceding process inventory, compile, sanitizer and policy regression steps passed. The failed assertion was the broad raw LAN-member-call scan; the workflow contract is being narrowed to count the two expected router/AP calls and require each to assign `operation_status`.
- AI handoff documentation contract `43859397894`: FAILED because `SESSION_HANDOFF.md` omitted the required end-of-session accuracy-checkpoint wording. The validator-required wording is restored in this checkpoint.
- XY-C1 firmware validation `43859397498`: IN_PROGRESS at `Build XY-C1`; TRX verification and artifact upload had not started at last inspection.
- NVRAM transaction audit `43859397571`: IN_PROGRESS at the raw userland ioctl/kernel API caller audit; preceding transaction and permission regressions passed.
- Other exact-head passing checks: Process inventory `43859397823`, AdGuardHome `43859397634`, infosvr `43859397632`, AliyunDrive `43859397583`, WAN DHCP `43859397570`, AliDDNS `43859397505`, detect_link `43859397835`, HTTPD security `43859397626`, DNSMasq `43859397688`, and detect_internet `43859397829`.

No same-source-SHA XY-C1 Build + TRX verification + artifact upload result was established at recovery, so L3 is not yet claimed. No physical-router L4 result is claimed.

## Exact first action for the next GPT

1. Inspect exact-head Process and AI handoff documentation runs created by the CI-contract repair commit; fix the first causal failure if either remains red.
2. Recheck XY-C1 run `43859397498` and NVRAM audit `43859397571` for product-source SHA `b715097b8fdb76ed1e96bdeed6bedafde47eeefd`. XY-C1 L3 requires Build, TRX verification and artifact upload on that same production-source SHA.
3. Continue auditing `init_router()` / `init_bridge()` for ignored required module-load and service-operation results. Keep optional hardware features non-fatal.
4. Keep development on `main`, create one complete Git tree and atomic commit per logical slice, and update main with a non-forced expected-SHA compare-and-swap.

