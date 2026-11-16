# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST perform an end-of-session handoff checkpoint before ending. The checkpoint is mandatory; a document edit is conditional. Continue the recorded engineering task across conversations: recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using a non-forced compare-and-swap. Never force-push, rewrite history, or create partial commits. Preserve the immutable backup and archive shards/index. Before context exhaustion, record precise live refs, exact Action results, and the next concrete task.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`.
- Parent `main` immediately before THIS COMMIT: `d0d75c6925c2640df348f9a16e55f612356a88ac`. Refetch live main before future writes and use expected-SHA non-forced updates.
- Parent `d0d75c6925c2640df348f9a16e55f612356a88ac` closes router-mode bridge startup status loss: both `br_set_stp()` and `br_set_fd()` return values are captured with first-error semantics while forward-delay programming is still attempted after STP failure.
- Exact-head parent Actions: Process lifecycle run `43833861533` SUCCESS and XY-C1 Firmware Validation run `43833861360` SUCCESS. Both run `head_sha` values equal `d0d75c6925c2640df348f9a16e55f612356a88ac`; XY-C1 completed the build and TRX/artifact steps.
- Prior modem build-profile L3 baseline: `7630958f12938ae6014ee859d9eccfd21cdf7503`, XY-C1 run `43827946528` passed `CONFIG_USB_SERIAL_OPTION=m`, build, TRX verification and artifact upload. This does not verify later source edits.
- Frozen archive: `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable.
- Startup protocol, root governance, archive index, backup marker and PR #1 recovery are complete; do not repeat that audit.
- Root `dev_log.md` had 79 complete newest-first records before THIS COMMIT; this entry brings it to 80, within the 20–119 limit.

## Active task

THIS COMMIT fixes the NDIS modem WAN result-loss path:

- `launch_wan_usbnet()` previously ignored `check_upnp_wanif_changed(ndis_ifname)` and `start_udhcpc_wan(ndis_ifname, unit, 0)`, then reported success even when interface migration or DHCP launch failed.
- It now stops before interface publication/migration if miniupnpd cannot stop, records WAN error state and returns the migration failure. After successful migration it captures the DHCP launch result, updates the WAN error flag and returns that result.
- `start_wan()` aggregates NDIS and RAS modem migration/launch failures with first-error semantics while continuing unrelated best-effort setup, including PPPoE passthrough.
- Results propagate through full WAN restart, immediate/deferred/automatic/manual reconnect wrappers, `init_router()` boot-service health and PID1 notification action results. Existing stop-before-reload and WAN reset/cache ordering is unchanged.
- A GNU89/Werror/ASan/UBSan policy regression covers migration failure suppressing DHCP, DHCP status preservation and first-error retention across best-effort work. A source-contract checker covers the production caller chain; both are wired into Process and XY-C1 workflows.

## Files and caller chain

- `trunk/user/rc/src/usb_modem.c`: NDIS miniupnpd migration gate, DHCP launch result and per-WAN error state.
- `trunk/user/rc/src/modem_wan_start_policy.h` / `tests/modem_wan_start_policy.c`: migration/DHCP policy and host regression.
- `trunk/user/rc/src/net_wan.c`: NDIS/RAS modem status aggregation in `start_wan()`; result propagation through restart/reconnect/deferred entry points.
- `trunk/user/rc/src/rc.c` / `rc.h`: boot aggregation, PID1 notification result propagation and updated public prototypes.
- `scripts/check_modem_wan_start_status.py`: checks migration ordering, DHCP return, first-error aggregation and caller edges.
- `.github/workflows/process-lifecycle-regression.yml` and `.github/workflows/xy-c1-firmware-validation.yml`: new test/checker path filters and execution.

## Compatibility and sequencing

- Nonzero miniupnpd migration status prevents NDIS interface publication and DHCP launch; that original status is preserved.
- DHCP launch failure is returned and recorded in the WAN `err` value. Interface teardown behavior remains unchanged so a later reconnect may retry.
- `start_wan()` exposes modem-related failures; unrelated existing best-effort setup continues. Callback-link creation, WAN VLAN setup and AP-client migration failures now return their actual status rather than appearing successful.
- RAS/NDIS module selection, modem protocol, NVRAM schema, WAN stop-before-reload and reset/cache sequencing remain unchanged.
- A separate P1 multi-modem issue remains queued: NDIS `cdc-wdm` selection enforces physical USB port ID + devnum, but the generic `ttyUSB` control fallback can ignore physical identity and a manual `modem_node` can point to another modem. Hotplug TTY metadata already publishes `portid` and `devnum`; constrain manual/preferred TTY selection to that same identity in the next slice.

## Verification boundary

- Host policy regression: PASS under GNU89, `-Wall -Wextra -Werror`, AddressSanitizer and UndefinedBehaviorSanitizer.
- Full-tree source checker is wired into both workflows; there is no full local checkout, so its production-source result must come from exact-head Actions.
- Parent `d0d75c6925c2640df348f9a16e55f612356a88ac`: Process run `43833861533` and XY-C1 run `43833861360` completed SUCCESS at exact head. These validate the preceding bridge status change, not THIS COMMIT.
- Exact-head Process and XY-C1 Build/TRX/artifact for THIS COMMIT are pending its push Actions. No new L3/L4 or physical-router result is claimed.

## Exact first action for the next GPT

1. Fetch live `main`, then inspect push-triggered Process and XY-C1 runs whose `head_sha` equals the new main SHA. Confirm the policy test, semantic source checker, full Process suite, MIPS build, TRX verification and artifact upload.
2. If exact-head validation passes, fix the queued NDIS control-TTY cross-device issue. Match the TTY metadata's exact physical USB port ID + devnum to the selected NDIS interface; preserve manual `modem_node` and `pref > 0` selection only among same-identity nodes.
3. Then resume the device-aware `reload_modem_modules()` result/readiness contract. Keep host/source, exact-SHA Actions, firmware and physical-hardware evidence distinct. Preserve the frozen backup and do not repeat the bridge or NDIS launcher status fixes.

## Governance constraints

Preserve security, architecture, compatibility, target-toolchain and mandatory testing. Use only `main`; do not touch the frozen backup. Update root `dev_log.md` for each code/documentation commit; keep it within 20–119 records and the size limits; preserve archived shards/indexes verbatim. Distinguish source, CI and firmware validation levels. No force-push, published history rewrite, or unrun-test/build/hardware claim.
