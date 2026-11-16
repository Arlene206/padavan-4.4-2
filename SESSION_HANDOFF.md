# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST perform an end-of-session handoff checkpoint before ending. The checkpoint is mandatory; a document edit is conditional. Continue the recorded engineering task across conversations: recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using a non-forced compare-and-swap. Never force-push, rewrite history, or create partial commits. Preserve the immutable backup and archive shards/index. Before context exhaustion, record precise live refs, exact Action results, and the next concrete task.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`.
- Parent `main` for THIS COMMIT: `d4616682786618314044bcb29ef3c103905247b1`. Fetch the live ref again before each subsequent write.
- Parent `d461668` includes the early `init_router()` status aggregation from `760ecfa9d1ff115e62b2df45afb9fda398229ba6`. The exact-head Process correction run `43832485163`, job `133180887500`, completed SUCCESS after fixing the stale inline-declaration source assertions. The parent `760ecfa` exact-head XY-C1 run `43831778029`, job `133178574224`, completed SUCCESS including Build XY-C1, TRX verification and artifact upload.
- The prior USB modem profile change remains L3 proven: `CONFIG_USB_SERIAL_OPTION=m` and its 11-module mapping check passed on `7630958f12938ae6014ee859d9eccfd21cdf7503`, run `43827946528`.
- Frozen archival ref: `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. Root `dev_log.md` has 78 records before THIS COMMIT and will have 79 after the new entry.

## Active task

The early `init_router()` failure accounting is now present on main. Exact-head Process run `43832485163` passes after the stale source checker fix, and the exact parent firmware run `43831778029` also passed Build/TRX/artifact.

A remaining concrete defect is in `start_services_once()`:

- In router mode when LAN STP is enabled, code calls `br_set_stp(IFNAME_BR, 1)` and `br_set_fd(IFNAME_BR, 15)` but discards both return values.
- Both wrappers return the underlying `br_set_param_int()` / sysfs file-write status. Losing a failed STP enable can conceal a bridge loop-prevention configuration failure. Forward-delay programming is independent and must still be attempted even if STP programming fails.
- THIS COMMIT captures/logs both results and merges them into `start_status` with first-nonzero semantics; the second write remains unconditional within the existing STP-enabled block.
- The GNU89 boot service status regression now covers STP-fails/forward-delay-succeeds and STP-succeeds/forward-delay-fails. The Process integration checker binds the assertions to actual production call sites. Existing Process and XY-C1 workflow filters/steps cover the source, checker and test.

## Files and caller chain reviewed

- `trunk/user/rc/src/rc.c::init_router()`: early boot result aggregation for both logger paths, detect_link, detect_internet, DNSMasq, WINS and the final service group is in parent `760ecfa`.
- `trunk/user/rc/src/services.c::start_services_once()`: router-mode LAN bridge STP and forward-delay sysfs writes; THIS COMMIT captures both.
- `trunk/user/shared/src/netutils.c::br_set_stp()` / `br_set_fd()`: return `br_set_param_int()` / sysfs file-write errors.
- `trunk/user/rc/src/service_start_status.h` and `tests/service_start_status.c`: first-nonzero policy and regression.
- `scripts/check_process_lifecycle_integration.py`: repaired stale boot-status declaration assumptions in parent `d461668`; THIS COMMIT adds bridge result-source assertions.
- Existing Process and XY-C1 workflow path filters and compile/run steps cover `services.c`, the integration checker and the status regression.

## Compatibility and sequencing notes

- Keep boot best-effort and preserve the existing `!is_ap_mode && lan_stp != 0` gate. No change is made to AP mode or disabled-STP behavior.
- Preserve first nonzero status; later success must not erase an earlier STP error.
- Always attempt the forward-delay write after STP configuration, even when STP fails. Do not short-circuit unrelated services.
- No new API, NVRAM schema or device behavior is introduced.

## Verification boundary

- Parent `d4616682786618314044bcb29ef3c103905247b1`: Process run `43832485163`, job `133180887500`, SUCCESS for the corrected integration checker and full Process regression.
- Parent `760ecfa9d1ff115e62b2df45afb9fda398229ba6`: XY-C1 run `43831778029`, job `133178574224`, SUCCESS including Build XY-C1, TRX verification and artifact upload. This verifies the early `rc.c` boot-status slice but not THIS COMMIT's new `services.c` changes.
- Prior exact modem-profile firmware run `43827946528` remains SUCCESS on SHA `7630958...`; no L4 modem/router behavior is claimed.
- THIS COMMIT requires exact-head Process and XY-C1 validation. No local checkout execution or physical-router test is claimed.

## Exact first action for the next GPT

1. Fetch live `main` and inspect the exact-head Process run for THIS COMMIT. Confirm the bridge status source assertions and GNU89/Werror/ASan/UBSan policy regression pass.
2. Inspect the exact-head XY-C1 run. Confirm `services.c` cross-compiles and the TRX verification plus artifact upload complete.
3. After this slice passes, continue the next P1 modem launcher audit: identify `start_modem_ndis()` paths that discard miniupnpd interface-migration and DHCP-client launch statuses, then verify propagation through start/restart callers before changing behavior. Do not repeat the completed early `init_router()` status aggregation or checker repair.

## Governance constraints

Preserve security, architecture, compatibility, target-toolchain and mandatory testing. Use only `main`; do not touch the frozen backup. Update root `dev_log.md` for each code/documentation commit; keep it within 20–119 records and the size limits; preserve archived shards/indexes verbatim. Distinguish source, CI and firmware validation levels. No force-push, published history rewrite, or unrun-test/build/hardware claim.
