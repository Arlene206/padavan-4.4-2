# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume concrete engineering tasks. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning.

- Create all changed blobs, one complete tree and one commit with the current main parent; update main with non-forced compare-and-swap using that exact SHA.
- Verify Actions by exact `head_sha`. A pending run does not block independent work; a failed relevant run must be triaged before claiming validation.
- Preserve security, architecture, GNU89/MIPS/uClibc compatibility, regression coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` is archival only; never alter or delete it. Develop on main.

## Current repository state

- Product-source checkpoint: `3488f129babe376bccf27e132776c45910614dd7` (mass-storage notification status fix). THIS checker repair is based on that exact parent; always re-fetch main before writing.
- PPP physical-WAN firewall recovery hardening: `5878c9d11404d1bd4a329b59430a7c1279025dd4`. Exact WAN route checker run `44625039862` PASS on `ccf419d1dcff6aff70080d37c90be707dec9fe88`; current-source XY-C1 L3 still needed.
- APCLI WAN recovery result propagation: `de73223da74d29a93236f29d8214fe66221b498c`. On `f2e814729e9152102811b4be4b069efe4924bd4f`, AI docs `44625501779` PASS, while Wireless `44625501907` and Process `44625501771` failed only because the guest checker referenced undefined lowercase `rc` instead of its defined `RC` path. THIS commit fixes that test-only NameError.
- SD/MMC checked PID-1 notification source fix landed at `3488f129babe376bccf27e132776c45910614dd7`: add/remove paths use `notify_rc_status()`, preserve queue errno, and return failure if operation or queue creation fails. Existing swapoff-before-unplug-notify and notify-after-successful-mount order is retained.
- On exact `3488f129...`: NVRAM audit `44625809632` PASS; Process inventory `44625809765` PASS; AI docs `44625809759` PASS. Storage hotplug `44625809558` failed a stale `tune_status == 0` source assertion despite the source deliberately logging tuning failure and continuing mount, then reporting that failure in final mdev status. HTTPD security `44625809482` failed a stale boolean return assertion because mdev now uses the shared two-input completion policy. THIS commit updates both contracts; exact reruns are pending.
- XY-C1 firmware run `44625809599` on product source `3488f129...` was in progress at last query. It is the current product-source L3 candidate and includes PPP, APCLI and SD/MMC source changes; verify Build, TRX and artifact upload on this exact SHA.
- Frozen backup remains `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify. PR #1 remains closed/unmerged. Root `dev_log.md` has 30 dated entries before THIS commit, well inside the 20–119 window.

## Active task

### Close downstream regression contracts after APCLI and storage result propagation

- APCLI's production callers intentionally capture `check_apcli_wan()` into `wan_status`, then merge it into the sticky guest-isolation status. The count assertion is already corrected; the final checker failed solely due the lowercase `rc` typo. THIS commit corrects it to `RC`.
- Storage's `mdev_sd_main()` and `mdev_mmc_main()` now return `storage_hotplug_completion_status(operation_status, notification_status)`. The HTTPD workflow still expected the pre-fix boolean expression and now verifies both operation/queue status inputs plus the tested policy. The storage workflow also had an obsolete `tune_status == 0` assertion even though tuning failure is intentionally diagnostic/non-blocking for mount and is returned as an applet error; this commit pins that behavior and final error aggregation.
- No product source changes in THIS checker repair.

## Exact first action for the next GPT

1. Fetch live main and exact-head Actions for THIS checker repair: Wireless radio, Process lifecycle, Storage hotplug, HTTPD security and AI docs. Inspect each failed job's exact log and repair the actual contract issue, not the product logic.
2. Inspect XY-C1 run `44625809599` for product SHA `3488f129...`. L3 is PASS only if Build XY-C1, TRX verification and artifact upload all succeed on that exact SHA; a skipped build is not L3.
3. Once those contracts pass, resume another independently evidenced P1 stability issue across Padavan userland. Do not repeat the PPP recovery, APCLI result path, SD/MMC notification fix, SCUT/Crond/Process status or OpenVPN audits without new evidence.

## Verification boundary

- L1: PPP fail-closed physical-WAN recovery at `5878c9d`; APCLI WAN error propagation at `de73223`; SD/MMC queue-completion error propagation at `3488f12`.
- L2: WAN route `44625039862` PASS; exact f2 Wireless/Process runs failed only on the lowercase checker variable, fixed here. On `3488f12`, NVRAM audit, Process inventory and AI docs PASS; Storage/HTTPD/Wireless/Process contract reruns are pending after THIS commit.
- L3: XY-C1 run `44625809599` on `3488f12` was in progress at checkpoint. Do not claim until Build, TRX verify and artifact upload all pass.
- L4: physical WAN fail-closed behavior, live APCLI/WISP reconnection and physical SD/MMC/PID-1 hotplug remain NOT VERIFIED.

## Mandatory handoff checkpoint

At session end, compare live main, product-source baseline, exact-head Actions, module state and this first action. Update this file only when resumable facts materially change; do not create no-op SHA-refresh commits.

## Development invariants

- Main-only; never modify the frozen backup or develop on a legacy branch/PR.
- One complete Git tree and one atomic commit per logical task; `force=false`, exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits or weaken regressions.
- Report exact SHA Actions and keep L1/L2/L3/L4 distinct. Host success is not firmware/hardware success.
