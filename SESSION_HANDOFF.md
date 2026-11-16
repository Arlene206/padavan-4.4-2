# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect live repository state and complete an accuracy checkpoint before ending. The checkpoint is mandatory; a document edit is conditional.

Continue the recorded engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched `main`; update `main` with exact expected SHA using a non-forced compare-and-swap. Never force-push, rewrite history, create partial commits, or touch the frozen backup. Before context exhaustion, record precise live refs, exact Action results and the next concrete task.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`.
- Parent for THIS RAS manual-node hard-pin correction: `1c1fa9679e625a468b04fc12cbd2794683e810fc`. Re-fetch main before each later write and use expected-SHA non-forced CAS.
- Frozen archive: `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only.
- Startup protocol/archive/backup/PR recovery are already complete; do not repeat them.
- `dev_log.md` contains 90 newest-first records before THIS entry and will contain 91 afterward. Keep 20–119 and preserve archived shards/indexes.

## Active task

A remaining manual-index fallback defect existed in the legacy RAS selector:
- `find_modem_node()` honored a configured `modem_node` when that exact index existed, but if absent returned `node_valid_last`. Since `get_modem_node_ras()` tries `ttyACM` and `ttyUSB` through this finder, a missing configured index could silently select a different device.
- THIS commit makes a configured index a hard pin: only the exact candidate is examined; if it is absent, the finder returns `-1`. Automatic selection remains unchanged.
- A small GNU89 policy helper and regression cover auto mode, exact match, mismatch and absent-candidate rejection; the USB identity workflow checks production integration and forbids the old last-node fallback.
- The earlier NDIS selector already validates physical port ID + devnum for WDM/TTY and preserves manual/preferred choices only within that identity.

## Verification boundary

Exact-head observations on parent `1c1fa9679e625a468b04fc12cbd2794683e810fc`:

- HTTPD Authentication `43846110941`: SUCCESS.
- AI handoff docs `43846110910`: SUCCESS.
- Process lifecycle `43846111322`: SUCCESS.
- AdGuardHome `43846110895`, infosvr `43846111062`, AliDDNS `43846111039`, AliyunDrive `43846111006`, NVRAM transaction boundary `43846110999`: SUCCESS.
- XY-C1 firmware validation `43846110844`: IN_PROGRESS at last inspection. Policy/host regression steps had passed and target build remained active; no TRX/artifact result or L3 pass yet.
- USB modem WDM/NDIS identity test `43844807532` on ancestor `9e284e758526ac281228e86b81dc349adbeff783`: SUCCESS. The new generic RAS manual-index behavior has not yet been validated by exact-head Actions.
- No physical-router/L4 result is established.

## Exact first action for the next GPT

1. Fetch live main and inspect this commit's exact-head USB modem identity and XY-C1 Actions. Verify the hard-pin regression and the full Build XY-C1 → TRX verification → artifact upload sequence before claiming L3.
2. Finish the `reload_modem_modules()` review. `module_smart_load()` returns zero both for already-loaded and failed loads, so post-load `/sys/module` state—not the return code—is authoritative. Keep alternative modem drivers optional; verify device readiness only after USB host initialization.
3. Resume broad RC/service-start result propagation outside modem-only work. The roadmap's remaining P1 is to identify enabled launch paths whose failures are still discarded; add caller-level regressions, not just helper tests.

## Governance constraints

Preserve security, architecture, compatibility, target-toolchain and mandatory tests. Use only `main`; the frozen backup is immutable. Update root `dev_log.md` for each source or documentation commit, preserve archives verbatim, and distinguish source, CI, firmware and physical-router validation. Never claim an unrun test/build/hardware result.
