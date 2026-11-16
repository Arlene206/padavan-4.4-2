# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect live repository state and compare it with this file before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers and precise first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave this file untouched and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task here.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, and do not ask the user to choose the next task.

Every multi-file logical change must be created as one complete Git tree and one atomic main commit parented to freshly fetched main, then update main with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, or modify the frozen backup branch.

## Current repository state

- Latest main commit: `6acfd5cc088bb3eafc5116a7ab017dc53845ae13`. USB storage xtrace run `43912334160` exposed the AWK END-block behavior: an unreadable directory yielded status 1 and incorrectly proceeded to `modprobe`. THIS COMMIT now validates a `cat` snapshot before and after taking the module lock and adds the post-lock read-error regression. HTTPD lock-assertion fix is in parent `6acfd5cc088bb3eafc5116a7ab017dc53845ae13`; exact-head HTTPD run pending. Product parent eff340 XY-C1 `43911560375` was still in progress at last poll.

- Repository: `Arlene206/padavan-4.4-2`; current source parent before this slice: `e05ccbcb8c458ee2bc07c6020ed5af52b3d4cff8`.
- Mount-link errno fix, GNU89 policy regression, storage source contracts, and workflow corrections are included together in THIS COMMIT.
- Frozen backup branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable and read-only.
- Storage application startup returns first errors from all seven configured services; media/download daemon launch and readiness errors flow through the common startup result, and deferred hotplug preserves retry events until success.
- `create_mp_link()` retains its boolean API but now initializes errno, normalizes mount inventory/path/directory/symlink/cleanup failures, keeps the first actionable error, and exposes `create_mp_link_status()` before all four media callers. MiniDLNA/Firefly retain the 0→1 directory fallback; Transmission/Aria retain mode 0 only.
- USB modem hotplug preserves coalesced unplug/replug events and retries failed reconnects; WAN IPv6 and LAN/Wi-Fi startup paths propagate failures.

## Active task

An exact-head test exposed the subtle AWK behavior in `func_load_module()`: when an input file cannot be opened, `END { exit !found }` can override the AWK input error and return 1, indistinguishable from `module absent`. Parent `8795f568cf1c53d8aa9b045b5a1074d3634e8f42` xtrace run `43912334160` proved the directory case passed through to `modprobe`. THIS COMMIT fixes this by taking a checked `cat` snapshot before AWK both outside and inside the lock. It also tests inventory becoming unreadable during lock wait; xtrace has been removed.

Parent `6acfd5cc088bb3eafc5116a7ab017dc53845ae13` aligned the HTTPD cross-workflow checker; confirm its exact-head run before treating that contract as green.
## Verification boundary

- Parent 8795 USB storage `43912334160`: all earlier checks passed; new shell test failed at unreadable-directory case because AWK status 1 masked an input-open error.
- HTTPD lock-checker parent `6acfd5cc088bb3eafc5116a7ab017dc53845ae13`: exact-head run `43912642221` was queued at last poll.
- Product parent eff340 XY-C1 `43911560375` was in progress; this commit changes product shell code and needs its own Build/TRX/artifact gate.
- Frozen backup remains unchanged. Physical-router L4 NOT VERIFIED.
## Exact first action for the next GPT

1. Inspect exact-head USB storage for THIS COMMIT; verify inventory read failures both before and after lock do not reach modprobe, and all existing storage checks pass.
2. Inspect HTTPD run `43912642221` and its exact-head rerun triggered by this commit; confirm the refreshed lock assertion passes.
3. Inspect XY-C1 Build, TRX verification and artifact upload for THIS product SHA.
4. Continue independent storage hotplug work only after those gates; preserve the frozen backup and L4 boundary.
