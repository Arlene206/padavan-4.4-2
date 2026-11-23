# SESSION HANDOFF — Padavan 4.4

## 2026-11-22 implementation checkpoint

- Source checkpoint before this handoff update: `3a2c51ff5cd05110da6e0edc568799dd1e88c2e4` (branch `main`). Backup branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; this session did not modify it.
- This session continued the libdisk account-management stability slice and made these changes on main:
  - Reworked `mod_account()` so all volume metadata paths are preflighted and renamed as a compensating batch before NVRAM account identity/password publication; attempted writes are restored on ambiguous setter/commit failures, and filesystem moves are rolled back when publication fails. Regression coverage includes partial rename rollback and failed compensation.
  - Reworked `add_account()` to preflight the complete storage inventory and permission-file destinations, create volume metadata before publishing the account in NVRAM, snapshot tail keys, and compensate created files / NVRAM values on failure.
  - Added `nvram_transaction_commit()`, which commits through `nvram_commit_unlocked()` while the existing exclusive transaction lease is held.
  - Reworked `del_account()` to validate a complete storage inventory before mutation and compact usernames/passwords plus last-account modes in one exclusive NVRAM transaction. It revalidates the account snapshot, snapshots overwritten password/mode values, checks every setter and commit, and restores the prior state after setter/commit failures. Regression cases cover stale inventory, missing passwords, setter/commit failure, rollback failure, manual-commit mode, and last-account mode rollback.
- Exact-SHA Actions observed for current source checkpoint:
  - Process lifecycle inventory - 3a2c51ff5cd05110da6e0edc568799dd1e88c2e4 #45097953532: in_progress (head 3a2c51ff5cd05110da6e0edc568799dd1e88c2e4)
  - libdisk share-list output safety #45097953581: in_progress (head 3a2c51ff5cd05110da6e0edc568799dd1e88c2e4)
  - NVRAM transaction boundary audit #45097953535: in_progress (head 3a2c51ff5cd05110da6e0edc568799dd1e88c2e4)
- Local firmware compilation was not independently run in this environment. Treat the GNU89 sanitizer regressions, NVRAM transaction-boundary audit, HTTPD/Samba checks, and XY-C1 firmware/TRX/artifact workflow as unverified unless an exact-head run above shows `completed / success`.
- First action for the next GPT: poll the exact source checkpoint SHA `3a2c51ff5cd05110da6e0edc568799dd1e88c2e4` for all jobs; inspect and fix any failed GNU89/libdisk, NVRAM transaction-boundary, HTTPD/Samba, or XY-C1 build/TRX/artifact step. Do not restart completed work or touch the frozen backup branch. Then continue with checked/compensating NVRAM writes in the remaining account-management paths and keep this handoff current.


# Cross-session execution mandate

This repository is one continuous engineering project across GPT conversations. Take over from the live `main` tree and the exact handoff checkpoint; do not repeat completed audits or ask the user to choose the next task.

- Implement concrete source fixes with focused regression coverage while preserving security, architecture, compatibility and required testing constraints.
- Follow the loop: modify source → test → one atomic main commit per logical task → inspect Actions for that exact commit SHA. A PENDING run does not block independent work; diagnose and fix failures from their first causal logs.
- For each multi-file logical slice, create all blobs, one complete Git tree and one commit. Publish using non-forced compare-and-swap against the freshly fetched main SHA. Never create per-file half-state commits or force-push.
- Do not ask the user what to do next. Do not claim tests, commits, firmware artifacts or hardware checks that were not actually performed.
- Before context exhaustion, finish the current safe slice, commit it, and record the exact SHA, tests, exact-head CI state, validation boundary and first next action below.

## Current repository state

- Development branch: `main` only. Do not create feature branches or PRs; never force-push.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Preserve it permanently; do not modify, rename, delete or check it out.
- Current main checkpoint: THIS COMMIT records PID 1 recovery-shell respawn throttle fix `3d0917068e0311d38f6e4a01c5ec0c9c3bab3d0c`; its parent is `7a4280c7b5de32dca77aed16a7c256551d16147b`. Earlier product-source fixes remain Samba `a3a7d64b` and MiniUPnP `4ecc2877`. Fetch live main on takeover; do not chase a docs-only SHA.
- Continue implementation rather than repeating completed audits or asking what to do next.

## Active task

New PID 1 stability defect fixed in source commit `3d0917068e0311d38f6e4a01c5ec0c9c3bab3d0c`:

- `init_main_loop()` previously stored `uptime()` immediately before `run_shell(0, 1)`, then treated time until the child was reaped as shell runtime. The forked child still had to reset signal handlers, initialize/attach the console, map timezone and prepare the environment before `execve(SYS_SHELL)`. Slow pre-exec setup could therefore exceed the two-second threshold even when the actual recovery shell exited immediately; the retry throttle disappeared and PID 1 could enter a repeated fork/exec loop.
- Removed that misleading parent-side elapsed-time estimate. `pid1_console_retry_delay_seconds()` now returns a one-second delay for every reaped recovery-shell child, independent of pre-exec setup latency. The existing shutdown-aware wait loop still cancels promptly on a shutdown request. Updated policy regression and workflow source contract; the path contract now explicitly includes the console policy header/test.
- The small trade-off is a one-second pause after an intentional long-lived recovery shell exits; this is preferable to an unbounded respawn loop under slow console setup.

Previously completed service-start fixes remain:
- Samba checked workgroup/hostname/WINS snapshots in `a3a7d64ba1fbaab7cc6f6d74d9c9650c017242ee`.
- MiniUPnP checked LAN IP/netmask/MAC snapshots in `4ecc28778b3605743262514dbce53b7ffbebda2d`.

The PID 1 source slice was committed atomically across `init.c`, `pid1_console_policy.h`, its regression test and the workflow contract. Exact-head CI results are recorded below; XY-C1 Build/TRX/artifact are still running or pending.

## Verification boundary

Exact-head Actions on PID 1 source commit `3d0917068e0311d38f6e4a01c5ec0c9c3bab3d0c`:
- PID 1 mdev/security workflow `45093300454`: PASS. This includes compilation and execution of the PID 1 console recovery retry policy regression and the inline PID 1 source contract.
- Process inventory/lifecycle umbrella `45093300665`: PASS, including the PID 1 required-boot-environment contract and the broader GNU89/ASan/UBSan lifecycle suite.
- AI handoff docs `45093300644`: PASS; root log archive integrity and the required handoff structure validate.
- PID 1 notification queue `45093300520`, DNSMasq lifecycle `45093300505`, DNS forwarder `45093300535`, HTTPD security `45093300612`, and complete process inventory artifact `45093300456`: PASS.
- USB storage hotplug `45093300506` and NVRAM transaction boundary audit `45093300474`: IN_PROGRESS at last poll; these are independent of the PID 1 code change.
- XY-C1 firmware validation `45093300583`: host contracts and pre-build checks PASS; actual `Build XY-C1` step IN_PROGRESS. TRX verification and artifact upload remain PENDING. This is the relevant product-source commit and must not be called L3 until all three complete.

Earlier exact-head results on `7dd05bff`:
- Switch CLI `45091738973`: PASS.
- XY-C1 `45091739058`: workflow SUCCESS; Build XY-C1, TRX verification and artifact upload SKIPPED because the commit changed only workflow/docs. Not L3.
- Process lifecycle `45091738946`: FAIL at the Samba whitespace-sensitive checker assertion, fixed in descendant `b803d7a5`.

MTK ESW's link-mode policy host regression passed on exact-head Switch CLI, but same-SHA kernel Build/TRX/artifact is still NOT VERIFIED. The XY-C1 push filter now watches the MTK driver/header/test; the next relevant source change must confirm the target build. Hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main` and verify frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify the backup.
2. Check exact XY-C1 run `45093300583` for commit `3d0917068e0311d38f6e4a01c5ec0c9c3bab3d0c`: record Build XY-C1, TRX verification and artifact upload independently; only all three PASS permit the L3 claim.
3. Check independent NVRAM audit `45093300474` and USB hotplug run `45093300506` for their final status; diagnose only a real failure, do not wait before independent work.
4. PID 1 mdev/security `45093300454`, Process lifecycle `45093300665` and AI handoff docs `45093300644` all PASS on the exact source SHA. Do not rerun these unless relevant files change.
5. Continue the next high-priority, independently testable stability/security issue across the Padavan userland. Preserve complete-tree atomic commits with `force:false` and exact `expected_sha`.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

At the end of every session, inspect the live repository state and compare it with this handoff. Verify the current main SHA, frozen backup SHA, engineering baseline, active task, relevant exact-head Actions results (including transitions from PENDING to PASS/FAIL), validation boundaries, unresolved blockers, and the exact first action for the next GPT. Update this handoff and dev log when those facts materially change. If the facts and first action remain accurate, leave the files untouched and do not create a no-op commit.

## End-of-session accuracy checkpoint

- Current implementation source checkpoint: `3d0917068e0311d38f6e4a01c5ec0c9c3bab3d0c`, parent `7a4280c7b5de32dca77aed16a7c256551d16147b`.
- `# Cross-session execution mandate` and `## Mandatory handoff checkpoint` remain present with exact required wording.
- Root `dev_log.md` remains a 108-record (<120) rolling window; this source change will be added as the 109th complete record in the next docs commit. No archive rotation required.
- Exact-head PID 1 mdev/security `45093300454`, Process lifecycle `45093300665`, AI docs `45093300644`, notification queue `45093300520`, DNSMasq `45093300505`, DNS forwarder `45093300535`, HTTPD `45093300612` and process inventory `45093300456`: PASS.
- XY-C1 `45093300583` is IN_PROGRESS at Build XY-C1; TRX and artifact steps are PENDING. USB hotplug `45093300506` and NVRAM audit `45093300474` were still in progress. L3/L4 are NOT VERIFIED.
