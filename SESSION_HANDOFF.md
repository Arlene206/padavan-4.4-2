# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect live repository state and complete an accuracy checkpoint before ending. The checkpoint is mandatory; a document edit is conditional.

Continue the recorded engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched `main`; update `main` with exact expected SHA using a non-forced compare-and-swap. Never force-push, rewrite history, create partial commits, or touch the frozen backup.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`.
- Parent for THIS HTTPD workflow contract correction: `70e20841ef6dc6198fda5036521760f99d43a46d`. Refetch live main before every subsequent write and use expected-SHA non-forced CAS.
- Frozen archive: `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only.
- Startup governance/archive/backup/PR recovery are already complete; do not repeat the startup audit.
- `dev_log.md` contains 88 records before this change; this entry will make 89. Keep the file within 20–119 records.

## Active task

The exact-parent HTTPD Authentication workflow run exposed one more stale source contract in its USB metadata step:
- Workflow `43845149417`, job `133223331331`, passed the standalone modem metadata policy binary and the USB hotplug metadata staging check.
- The embedded Python assertion then failed because it expected the removed generic `get_modem_node("ttyUSB", devnum, NULL)` selector to remain in `usb_modem.c`.
- The NDIS TTY control path now correctly uses `get_modem_node_by_usb_identity("ttyUSB", ndis_port_id, devnum)`; THIS commit updates the workflow's source assertion to require that identity-bound selection instead.
- This is a checker correction only; no production behavior is changed here.

## Verification boundary

Exact-head observations on parent `70e20841ef6dc6198fda5036521760f99d43a46d`:

- Process inventory/lifecycle run `43845149493`: SUCCESS after the current int-return `start_wan(void)` source-checker fix.
- XY-C1 run `43845149433`: IN_PROGRESS at the `Build XY-C1` step on last inspection. Earlier policy regression steps had passed; no TRX verification or artifact-upload result yet, so no L3 pass is claimed.
- AI handoff docs `43845149351`, infosvr `43845149279`, AdGuardHome `43845149408`, AliDDNS `43845149268`, AliyunDrive `43845149455`: SUCCESS.
- HTTPD Authentication `43845149417`: FAIL at `Verify USB modem node metadata staging`. The metadata policy binary and preceding hotplug metadata contract printed PASS; the only observed failure was the stale generic TTY getter assertion fixed by THIS commit.
- NVRAM transaction boundary `43845149529`: IN_PROGRESS at last inspection.
- USB WDM/NDIS identity test `43844807532` on parent `9e284e758526ac281228e86b81dc349adbeff783`: SUCCESS, including GNU89/Werror/ASan/UBSan test and the same-device TTY source contract.
- No exact-parent XY-C1 TRX/artifact L3 result or physical-router/L4 result is established.

## Exact first action for the next GPT

1. Fetch current main and inspect exact-head HTTPD, XY-C1, NVRAM, Process and AI handoff actions. Read failures, and do not count XY-C1 L3 until Build, TRX verification and artifact upload all complete.
2. Inspect the unresolved `reload_modem_modules()` contract carefully. `module_smart_load()` can return zero both for an already-loaded module and for a failed load; `/sys/module` post-load state is authoritative. Do not require every alternative NDIS driver to load. Determine which selected-device readiness condition can be tested after USB host initialization, and propagate only well-defined failures.
3. Broaden stability/security governance beyond USB modem work when the above integration gates are green.

## Governance constraints

Preserve security, architecture, compatibility, target-toolchain and mandatory tests. Use only `main`; the frozen backup remains read-only. Update `dev_log.md` for every code/documentation commit and preserve archived shards/indexes verbatim. Distinguish source, CI, firmware and physical-router validation. Never claim an unrun test/build/hardware result.
