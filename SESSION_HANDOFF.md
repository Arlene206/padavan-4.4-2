# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect live repository state and complete an accuracy checkpoint before ending. The checkpoint is mandatory; a document edit is conditional.

Continue the recorded engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched `main`; update `main` with exact expected SHA using a non-forced compare-and-swap. Never force-push, rewrite history, create partial commits, or touch the frozen backup. Before context exhaustion, record precise live refs, exact Action results and the next concrete task.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`.
- Parent main for THIS TTY identity slice: `37f00465dec9af78f3e366a16e903ddc600603c2`. Refetch live `main` before every later write and use expected-SHA non-forced compare-and-swap.
- Frozen archive: `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable.
- Startup protocol, root governance, archive index, backup marker and PR #1 recovery are complete; do not repeat that audit.
- Root `dev_log.md` contained 87 newest-first records before THIS commit; this entry makes 88, inside the required 20–119 window.

## Active task

The NDIS USB modem WAN control-TTY fallback now has a same-physical-device selection policy:
- The NDIS interface is resolved to its USB port ID; its kernel devnum is already supplied by NDIS discovery.
- Candidate TTY metadata must contain one well-formed `pref=0|1`, `devnum=1..127` and `portid`, and both identity fields must match the NDIS interface's physical identity.
- A configured `modem_node` index is a hard pin, but only if that TTY node belongs to the same physical USB device. A mismatched manual choice fails closed instead of switching silently.
- Without a manual index, the selector chooses a same-device preferred node first and then any same-device TTY fallback.
- WDM lookup remains constrained to the same port ID + devnum and does not inherit the TTY manual-selection behavior.

This closes the concrete cross-modem control-TTY selection risk at source/policy level. Do not call it verified until the exact-head GNU89/Werror/ASan/UBSan and source-contract Actions pass.

## Verification boundary

Exact-head `9e284e758526ac281228e86b81dc349adbeff783` observations:

- USB modem WDM/NDIS physical identity run `43844807532`: SUCCESS. It validates the new same-device TTY selection policy and host/source regression checks, not physical multi-modem hardware.
- Process lifecycle integration run `43844807348`: IN_PROGRESS at last observation; an earlier parent run failed because `check_process_lifecycle_integration.py` parsed the former `void start_wan()` signature. THIS corrective commit updates that parser to the current `int start_wan()` definition. Exact-head rerun required.
- XY-C1 Firmware Validation run `43844807421`: IN_PROGRESS at last observation. It must complete Build XY-C1, TRX verification and artifact upload before L3 is claimed.
- HTTPD Authentication/Security run `43844807351`: FAIL in its embedded `start_wan()` checker because it searched for bare `return;` in a function that now returns `int`. THIS corrective commit asserts the actual callback setup and VLAN error returns and preserves their ordering.
- AI handoff docs `43844807461`: SUCCESS; Infosvr packet regression `43844807516`: SUCCESS; WAN teardown `43844807410`: SUCCESS; AdGuardHome `43844807404`: SUCCESS; AliyunDrive `43844807369`: SUCCESS; AliDDNS `43844807524`: SUCCESS.
- Prior `f0902af` WAN DHCP callback `43844254326` and miniupnpd config `43844254468` passed. Earlier exact-head WAN route `43844254309` passed. Use each Action's exact `head_sha` when interpreting results.
- No current-source XY-C1 Build/TRX/artifact PASS or physical-router/L4 evidence is established at the `9e284e7` parent.

## Exact first action for the next GPT

1. Fetch live `main` and inspect exact-head Process lifecycle, XY-C1 Firmware Validation, HTTPD Security, USB modem WDM/NDIS identity, AI handoff and Infosvr Actions. Read failed job logs. Confirm Process and HTTPD are green; require XY-C1 Build, TRX verification and artifact upload before L3.
2. Then investigate `reload_modem_modules()` readiness and failure propagation. Map all modem driver alternatives, supported module configurations, module dependency ordering and all callers of `module_smart_load()` / `module_smart_unload()`. Separate optional-driver absence from a failure that makes the selected device unusable; add a small host policy and source-linked regression before changing behavior.
3. Continue whole-repository user-space stability/security governance after the modem readiness slice; do not return to already-completed NDIS TTY identity matching unless its exact-head workflow exposes a defect.

## Governance constraints

Preserve security, architecture, compatibility, target-toolchain and mandatory tests. Use only `main`; frozen backup is read-only. Update root `dev_log.md` for every code/documentation commit; keep the root record window at 20–119 and preserve archives/indexes verbatim. Distinguish source, CI and firmware validation levels. No force-push, history rewrite, or unrun-test/build/hardware claim.
