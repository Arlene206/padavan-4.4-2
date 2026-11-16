# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect live repository state and complete an accuracy checkpoint before ending. The checkpoint is mandatory; a document edit is conditional.

Continue the recorded engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched `main`; update `main` with exact expected SHA using a non-forced compare-and-swap. Never force-push, rewrite history, create partial commits, or touch the frozen backup. Before context exhaustion, record precise live refs, exact Action results and the next concrete task.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`.
- Parent main for THIS TTY identity slice: `37f00465dec9af78f3e366a16e903ddc600603c2`. Refetch live `main` before every later write and use expected-SHA non-forced compare-and-swap.
- Frozen archive: `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable.
- Startup protocol, root governance, archive index, backup marker and PR #1 recovery are complete; do not repeat that audit.
- Root `dev_log.md` has 86 newest-first records after this entry; keep 20–119 and preserve archive shards/indexes.

## Active task

The NDIS USB modem WAN control-TTY fallback now has a same-physical-device selection policy:
- The NDIS interface is resolved to its USB port ID; its kernel devnum is already supplied by NDIS discovery.
- Candidate TTY metadata must contain one well-formed `pref=0|1`, `devnum=1..127` and `portid`, and both identity fields must match the NDIS interface's physical identity.
- A configured `modem_node` index is a hard pin, but only if that TTY node belongs to the same physical USB device. A mismatched manual choice fails closed instead of switching silently.
- Without a manual index, the selector chooses a same-device preferred node first and then any same-device TTY fallback.
- WDM lookup remains constrained to the same port ID + devnum and does not inherit the TTY manual-selection behavior.

This closes the concrete cross-modem control-TTY selection risk at source/policy level. Do not call it verified until the exact-head GNU89/Werror/ASan/UBSan and source-contract Actions pass.

## Verification boundary

- Last fully observed checker-corrected parent before this slice: `37f00465dec9af78f3e366a16e903ddc600603c2`. Its AI handoff documentation run `43844536085` passed; Process `43844536051` and XY-C1 `43844536067` were still in progress when the TTY slice began.
- Parent `f0902af4861fdbf496958b22871ae43e51bdcfd5`: USB modem identity `43844254366`, WAN DHCP `43844254326`, miniupnpd `43844254468`, WAN teardown `43844254482`, WAN route `43844254309`, AI docs `43844254359`, Process inventory `43844254197` SUCCESS. Process WAN-start run `43844256457` failed its then-new exact-line RAS WANX checker assertion, which was replaced with a whitespace-tolerant regex in the child.
- XY-C1 `43844254098` was in progress at last inspection; no TRX/artifact L3 result is recorded.
- HTTPD Authentication `43844254517` failed its independent switch-port ioctl contract, unrelated to modem/WAN changes.
- This new TTY slice must be validated on its own SHA; prior identity PASS does not cover it.

## Exact first action for the next GPT

1. Fetch live main and inspect exact-head USB modem identity, Process, XY-C1 and AI handoff runs. Read failure logs. XY-C1 must reach Build, TRX verification and artifact upload before claiming L3.
2. If identity and source checks pass, inspect `reload_modem_modules()` and its boot/RC restart callers: distinguish module already loaded, missing module, busy unload, failed unload, parameter/readiness and optional alternative drivers. Add an explicit result contract only where semantics are unambiguous, plus regression tests.
3. Broaden whole-repository stability/security governance beyond this modem slice; do not keep returning to a completed modem path absent fresh evidence.

## Governance constraints

Preserve security, architecture, compatibility, target-toolchain and mandatory tests. Use only `main`; frozen backup is read-only. Update root `dev_log.md` for every code/documentation commit; keep the root record window at 20–119 and preserve archives/indexes verbatim. Distinguish source, CI and firmware validation levels. No force-push, history rewrite, or unrun-test/build/hardware claim.
