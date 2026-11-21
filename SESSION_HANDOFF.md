# Cross-session execution mandate

## Workflow
- Develop on `main` only. One logical slice, one atomic main commit: create all blobs, one tree and one commit, then use a non-forced expected-SHA compare-and-swap update. Never force-push, amend, create branches/PRs, or edit the immutable backup.
- Preserve security, architecture, compatibility and required tests. Target GNU89, MIPS/uClibc, BusyBox and Linux 4.4. Do not weaken tests.
- Exact-head push Actions only validate the matching main SHA. PENDING is not a gate on independent work; inspect a relevant FAIL, fix its cause, and move forward.
- Before context exhaustion, finish the safe slice, publish the commit, inspect exact-SHA Actions and leave a truthful next action.
- Keep root `dev_log.md` at 20–119 newest-first entries; preserve `docs/dev-log/2026/11/part-021.md` and its index unchanged. Do not create handoff/roadmap snapshot archives.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect the live repository state before ending, verify this handoff's source baseline, active task, exact-head Actions, validation boundary, blockers and first next action. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no no-op commit. Update this handoff only when next-session-critical facts materially change.

## Current repository state
- Parent main for this follow-up: `ed7c42675c8784123f1ad14cf098b2d31be6ddcc`.
- iNIC status-propagation fix landed in `215892b1acde4eea1be4be7785f4ef2b2b197e55`; iNIC signal and Wireless radio CI now pass on `07e68ffbe95b7d8954274207e4712997a3c1548a` / `#44896945147` and `#44896945159` after checker repairs. The iNIC daemon continues on transient poll failures and surfaces force-off/guest-up command errors.
- Further review found the iNIC callback still used `is_interface_up()`, which collapses `get_interface_flags()` socket/ioctl failure to false. This contradicted the same module's existing fail-closed tri-state helper `wireless_guest_interface_state_checked()`: an UNKNOWN state was being mistaken for DOWN and could trigger an unnecessary guest AP transition. This commit uses the checked probe, defers changes on unknown/error, and pins the existing `wireless_guest_isolation_link_state()` + EACCES regression in both iNIC/Wireless contracts.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, immutable.
- XY-C1 run `#44896853565` on source `215892b1acde4eea1be4be7785f4ef2b2b197e55` had completed the 36 pre-build checks and was still compiling XY-C1, with TRX and artifact steps pending; no L3 claim. Prior L3 source `20e139d230b73eabc5f25a2b17d56a3a5768af11` / `#44887656164`; hardware L4 NOT VERIFIED.

## Active task
- Verify exact-head iNIC and Wireless Actions for this checked guest-link-state probe. Their C policy tests already include UNKNOWN-vs-DOWN behavior; the new source contracts require this health callback to consume that result before recovery.
- Poll XY-C1 #44896853565; inspect Build, TRX and artifact individually before making L3 claims.
- After iNIC and Wireless converge, advance to a distinct P1 reliability/security issue outside WAN/HTTPD/SkipDB.

## Verification boundary
- No local checkout/runtime available; no local tests/sanitizer/firmware build is claimed.
- Remote Actions on exact SHA provide L2 evidence. XY-C1 Build + TRX + artifact on one exact production-source SHA is required for L3; hardware/router L4 NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch live main and inspect exact-head iNIC signal and Wireless radio state workflows for the checked-link-state change.
2. Fix any first causal failure without weakening UNKNOWN-state policy.
3. Inspect exact Build/TRX/artifact steps for XY-C1 run #44896853565; if it remains pending or is tied to an older SHA, trigger a source-matching run.
4. Choose another concrete P1 module reliability/security issue when this radio slice closes.

Before context exhaustion, update this handoff only when active task, exact-head status or validation boundary changes; no no-op commit.
