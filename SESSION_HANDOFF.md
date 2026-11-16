# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect live repository state and compare it with this file before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers and precise first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave this file untouched and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task here.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, and do not ask the user to choose the next task.

Every multi-file logical change must be created as one complete Git tree and one atomic main commit parented to freshly fetched main, then update main with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, or modify the frozen backup branch.

## Current repository state

- Latest main parent before THIS COMMIT: `a4daa0fd1ec33223b2d9bfaf6e3bff442079119a` (`fix(smartdns): preserve adblock parser error status`). The SmartDNS lifecycle test still references `$tmpdir` under `set -u` without creating it; ancestor Process inventory run `43916101921` demonstrated the exact failure. THIS COMMIT repairs the test fixture and cleanup.
- Frozen backup branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable and read-only.
- `automount.sh` now uses checked `/proc/modules` snapshots, checked `modprobe -l` catalog capture, and a checked tri-state `/proc/mounts` inventory for preflight, revalidation, safe unmount and final source-device ownership. Only confirmed foreign ownership permits fallback; UNKNOWN/malformed inventory aborts.
- The SmartDNS process/Adblock regression is invoked by the USB storage and Process inventory workflows. Its previous CI blocker was test-fixture setup, not a production SmartDNS or automount failure.
## Active task

Fixing an actual shared CI blocker: `trunk/user/smartdns/tests/smartdns_process_lifecycle.sh` runs with `set -u` but lacked a `tmpdir` initialization before using `$tmpdir/adblock/...`, causing exit at line 86. THIS COMMIT creates a private temporary workspace and cleans it on exit/signals. It also fixes a doubled-backslash `printf` in the invalid Adblock fixture. Product source is unchanged. Next inspect the exact-head Process and USB storage runs to confirm the shared regression completes, then resume independent product work.
## Verification boundary

- Ancestor `06bfebfa986d6b6b0c7c61a98ee389327019a7af`: Process inventory run `43916101921` FAILED at `smartdns_process_lifecycle.sh:86` with `tmpdir: parameter not set`, after the process-state assertions passed. The failure is reproducible from the test file and also blocks the embedded USB storage workflow step.
- Parent `a4daa0fd1ec33223b2d9bfaf6e3bff442079119a`: current source already includes the SmartDNS Adblock parser error-status correction; exact-head Process `43916362695`, USB storage `43916362699`, XY-C1 `43916362791`, NVRAM `43916362746`, AI docs `43916362841`, and Process lifecycle `43916362778` were IN_PROGRESS at last poll.
- THIS COMMIT changes only the SmartDNS shell regression fixture and handoff/log; exact-head Actions for THIS COMMIT pending. No local shell test or firmware build is claimed.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; physical-router L4 remains NOT VERIFIED.
## Exact first action for the next GPT

1. Inspect the exact-head Process inventory and USB storage Actions for THIS COMMIT. Confirm the SmartDNS lifecycle regression gets past Adblock fixture setup and the automount module-catalog/mount-owner tests then run.
2. Recheck XY-C1 Build, TRX verification and artifact upload on the latest product-source SHA; do not carry an earlier mount-owner result over a later source change.
3. If these jobs remain pending, continue the next independent P1 stability issue, not another audit of completed mount ownership/module-load work.
4. Keep the frozen backup unchanged and physical-router L4 marked NOT VERIFIED.
