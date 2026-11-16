# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers, frozen backup and precise first action for the next session. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, explicitly conclude the check and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, do not wait for pending CI when independent work can proceed, and do not ask the user what to do next.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main, followed by a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, use a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Live main before this HTTPD contract repair: `595bc6b6d78501ac833e721a51d9364063683e04` (`test(firewall): assert core and UPnP first-error status`). Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`; its preservation marker was read and the branch is read-only.
- Exact-head 595bc6 push Actions: ten runs were returned; eight passed and two were IN_PROGRESS at last poll: XY-C1 firmware validation `43940508628` and NVRAM transaction audit `43940508621`.
- Exact-head 595bc Firewall Startup `43940508528`, Process inventory/regression `43940508454`, AI handoff docs `43940508597`, infosvr `43940508523`, USB modem `43940509016`, AliDDNS `43940508780`, AdGuardHome `43940508442` and other listed runs passed. In particular, the old `return upnp_status;` checker failure is now fixed.
- The prior HTTPD run `43939886670` on edeaf28 passed snapshot-descriptor testing and failed only because its embedded source checker still searched for a bare `return;`; the implementation now returns `return status ? status : EIO;`. This commit corrects the inline contract and triggers a fresh HTTPD run. PR #1 remains closed/unmerged; do not resume it.

## Active task

- `start_firewall_ex()` returns EBUSY/EINVAL/EIO for unresolved recovery, unsafe interface tokens, core restore/rollback/snapshot-cleanup and Adbyby extension-restore failures. Incomplete rollback disables IPv4/IPv6 forwarding and retains recovery snapshots. `restart_firewall()` always attempts UPnP refresh, returns the first nonzero firewall/UPnP result and restores errno from the core failure.
- The core transaction fault-injection test passed in exact-head edeaf28 Firewall Startup `43939885812`. The canonical WAN/RC checker now verifies firewall-status capture and first-error propagation; current-head Firewall Startup `43940508528` and Process regression `43940508454` pass.
- This commit repairs the remaining HTTPD inline assertion to require the actual error return before forwarding can be enabled, and asserts the transaction status accessor in the same source-contract block.
- Next engineering task: audit the remaining Shadowsocks, persistent post-firewall and Optware hook execution/staging/cleanup results. They are currently logged but not merged into `start_firewall_ex()` status. Decide and test whether failure should be surfaced while allowing other independent hooks to run. Then continue NAT module unload/postcondition review.

## Verification boundary

- Exact-head 595bc: eight of ten returned push runs passed; XY-C1 `43940508628` and NVRAM `43940508621` were pending at last observation. XY-C1 Build/TRX/artifact is NOT VERIFIED until that same SHA completes those steps.
- The previous HTTPD run `43939886670` failed only at the stale bare-return source anchor after its firewall snapshot descriptor regression passed. This commit triggers a fresh HTTPD check; its result is not yet known.
- No local full firmware build or physical-router validation is claimed. Physical-router L4 remains NOT VERIFIED; frozen backup unchanged.

## Exact first action for the next GPT

1. Inspect exact-head HTTPD security, XY-C1 Build/TRX/artifact, NVRAM audit, Firewall Startup, Process regression and AI handoff docs for this contract-repair commit. Do not wait on pending jobs before independent engineering work.
2. Continue at `trunk/user/rc/src/firewall_ex.c`: inspect result propagation for Shadowsocks, persistent and Optware hooks, including staging and temporary-file cleanup. Add deterministic failure-injection coverage before changing the shared behavior; retain the fail-closed core rollback guarantees.
3. Then audit NAT module unload/load postconditions in `trunk/user/rc/src/net.c`. Keep commits on main, each one complete-tree atomic with non-forced expected-SHA CAS; backup is read-only and router L4 is NOT VERIFIED.
