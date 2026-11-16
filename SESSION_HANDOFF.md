# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state and compare it with `SESSION_HANDOFF.md` before ending or handing off. Verify source baseline, active task, exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave the file untouched and make no no-op commit.

Before context exhaustion, finish or reduce the current safe slice, check exact-head Actions, commit completed work atomically and preserve the precise first next action here.

Continue this engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active ref: `main`.
- Parent main for the current USB printer error-propagation correction: `0409c2b90d8da55531b8370dbb16e7a1ed978d67`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only. Startup recovery is complete; do not repeat.
- Root dev log has 42 records before this entry; archive part-012 retains older entries.
- PPP ip-up validates canonical local/peer IPCP IPv4 addresses and contiguous netmask before interface setup or lease publication.
- WAN IPv6 teardown preserves the last live runtime `ifname6` through RA/autoconfiguration and route/address cleanup; DHCPv6 callback generation fencing remains required.
- Optional WAN MTU ioctl failures remain diagnostic-only and do not set DHCP, interface or route failure state.

## Active task

USB printer launch failures now propagate from p910nd and direct u2ec/LPRng exec. The latest `services_usb.c` slice replaced `system("/usr/sbin/{u2ec,lpd} &")`, whose shell background status masked exec failures, with a fork/exec helper using an FD_CLOEXEC pipe to report exec errors to the caller. The helper does not wait for daemon runtime readiness; it only confirms successful `execv` handoff or returns the precise exec/setup error.

The exact-head storage workflow on `0409c2b90d8da55531b8370dbb16e7a1ed978d67` failed at its last path-filter assertion: `service_start_status.h` and `tests/service_start_status.c` were present in push filters but absent in pull_request filters. The current correction adds both PR paths. Review also found a remaining production omission: the optional `on_hotplug_printer.sh` hook's `doSystem()` status was logged but dropped; this correction aggregates it without skipping p910nd or shared spooler starts. A focused GNU89 error/continue test and source contract cover the hook.

## Verification boundary

Exact-head Actions for `0409c2b90d8da55531b8370dbb16e7a1ed978d67`:
- USB storage hotplug `43880100890`, job `133343396256`: FAIL at source-contract line 138 because `service_start_status.h` and `tests/service_start_status.c` were missing from pull_request filters. All steps through RC service-start GNU89 compile/run (step 22) passed, including BusyBox 1.24.2 ash and storage regressions. This commit adds the missing PR paths and asserts the optional hook result is aggregated.
- XY-C1 Firmware Validation `43880100797`: IN_PROGRESS at last inspection; no same-SHA Build/TRX/artifact outcome available yet.
- NVRAM transaction `43880100873`: IN_PROGRESS at last inspection.
- AI handoff documentation `43880101001`, process inventory/lifecycle `43880100870`, process lifecycle `43880100789`: PASS on this parent.
- Product-source firmware L3 was previously verified on `f6fba229aba497988d2308c8c99879aca5da1214` with same-SHA Build, TRX verification and artifact upload. Do not transfer that claim to this SHA until the current exact-head XY-C1 run passes all stages. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head storage-hotplug and XY-C1 Actions on the resulting main SHA; fix the first causal failure without weakening contracts.
2. Require current-SHA XY-C1 Build, TRX and artifact upload to pass before claiming firmware L3.
3. Continue P1 broad RC startup-result propagation across userland modules. Trace every actual caller and preserve lock/process identity contracts.
4. Keep physical-router L4 explicitly NOT VERIFIED until hardware validation evidence exists.
