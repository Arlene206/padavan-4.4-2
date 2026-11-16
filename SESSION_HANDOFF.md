# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST perform an end-of-session handoff checkpoint before ending. The checkpoint is mandatory; a document edit is conditional. Continue the recorded engineering task across conversations: recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using a non-forced compare-and-swap. Never force-push, rewrite history, or create partial commits. Preserve the immutable backup and archive shards/index. Before context exhaustion, record precise live refs, exact Action results, and the next concrete task.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`.
- Parent `main` immediately before THIS COMMIT: `8c3772e33127c3870c4465c45a8fedc1142a06b3`; all subsequent writes must re-fetch live main and use expected-SHA, non-forced ref updates.
- Parent `8c3772e33127c3870c4465c45a8fedc1142a06b3` introduced NDIS WAN startup result propagation from miniupnpd migration and DHCP launch through `start_wan()`, restart/reconnect wrappers, `init_router()` and PID1 notifications. Its exact-head Actions were NOT green; see the active-task and verification sections below.
- Frozen archive: `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable.
- Startup protocol, root governance, archive index, backup marker and PR #1 recovery were already completed. Do not repeat that audit.
- Root `dev_log.md` had 80 complete newest-first records before THIS COMMIT; this entry brings it to 81, within the 20–119 limit.

## Active task

THIS COMMIT is a corrective follow-up to `8c3772e33127c3870c4465c45a8fedc1142a06b3`. The first attempt at returning the NDIS modem WAN startup result exposed several real integration gaps, so do not treat `8c3772e` as green:

1. The new Python checker used `r"\\s+"` instead of `r"\s+"`, so whitespace was not compacted and the Process/XY-C1 source-contract steps failed. It also exposed that the RAS branch recorded the miniupnpd migration failure nowhere; a misplaced merge had instead landed in the unrelated IPoE EAPOL error branch.
2. The int-return conversion of `full_restart_wan()` had replaced the established direct `stop_wan()`, bridge teardown and LAN bridge gates with status assignments. This broke existing WAN teardown and route regression contracts. THIS COMMIT restores the original gate structure/order and only converts abort results to `-1`, while forwarding `start_wan()`'s result.
3. Existing source-contract checks needed to follow the intentional int-return contract: AP-client early failure now returns its captured `vlan_result`; the miniupnpd checker now expects the RAS migration result to be captured once; IPTV callback assertion now expects both route and multicast status to be accumulated.

The modem startup source behavior remains:
- `launch_wan_usbnet()` blocks NDIS interface publication/DHCP if miniupnpd cannot stop, and returns the actual DHCP launch result.
- `start_wan()` preserves the first NDIS/RAS migration/launch error while continuing unrelated best-effort setup.
- Errors propagate through full restart, reconnect/deferred/manual/automatic wrappers, boot health and PID1 notification results.
- The RAS migration failure is now aggregated before its branch continues; the unrelated IPOE EAPOL failure no longer touches modem status.

## Files and caller chain

- `trunk/user/rc/src/net_wan.c`: RAS migration status retention, removal of stray IPoE status merge, restored full-restart bridge safety gates.
- `scripts/check_modem_wan_start_status.py`: proper whitespace regex and explicit RAS/EAPOL source contracts.
- `.github/workflows/wan-teardown-stability.yml`: expect the new int-return abort result while preserving original bridge-gate checks.
- `.github/workflows/miniupnpd-config-stability.yml`: check RAS result capture rather than requiring the pre-capture conditional spelling.
- `.github/workflows/wan-dhcp-callback-stability.yml`: align the AP-client return check and IPTV route+multicast accumulated result with current source behavior.
- Previous `8c3772e` policy header/test and Process/XY workflow wiring remain unchanged.

## Compatibility and sequencing

- RAS and NDIS module selection, modem protocol, NVRAM schema, WAN stop-before-reload and reset/cache sequencing remain unchanged.
- `full_restart_wan()` retains its legacy execution order and early abort gates; it now returns `-1` for the same stop/bridge setup abort paths and the final `start_wan()` status.
- A nonzero miniupnpd migration status still prevents NDIS interface publication/DHCP. DHCP launch failure remains recorded in the WAN `err` field and returned.
- Multi-modem NDIS TTY selection remains a separate P1 defect: the generic `ttyUSB` fallback does not enforce physical port ID + devnum, while a manual node index can select another modem.

## Verification boundary

- Parent `8c3772e33127c3870c4465c45a8fedc1142a06b3`: exact-head Actions were FAIL:
  - Process run `43838099251` and XY-C1 run `43838100294` failed at the new source checker because its whitespace regex was over-escaped and the RAS migration error was not aggregated. The host modem policy test itself passed in both jobs.
  - WAN teardown run `43838099259` failed because the existing checker searched for the former void-function `return;` spelling.
  - miniupnpd atomic publication run `43838099236` failed because its source contract expected two direct RAS migration condition strings after one was correctly converted to a captured status.
  - WAN DHCP callback run `43838099905` had two stale expectations: AP-client setup now returns `vlan_result`, and the IPTV callback returns the accumulated route+multicast status, not only route status.
  - WAN route status run `43838099930` exposed a real contract regression: the previous `full_restart_wan()` rewrite had removed the source-visible `config_soft_bridge_lan() != 0` gate. THIS COMMIT restores it.
- The preceding parent `d0d75c6` passed Process run `43833861533` and XY-C1 run `43833861360`; these do not validate either `8c3772e` or THIS COMMIT.
- Re-run the local GNU89/Werror/ASan/UBSan policy test and the exact-head Process + XY-C1 + WAN teardown + miniupnpd + WAN DHCP + WAN route checks for THIS COMMIT. All new results are pending. No new L3/L4 or physical-router result is claimed.

## Exact first action for the next GPT

1. Fetch live main and inspect exact-`head_sha` runs for Process, XY-C1, WAN teardown, miniupnpd atomic publication, WAN DHCP callback and WAN route status. Read any failing job logs; do not infer success from run completion alone.
2. If those checks are green, continue the P1 NDIS control-TTY cross-device selection fix: match TTY metadata's exact physical USB port ID + devnum to the selected interface, preserve manual `modem_node` and `pref > 0` behavior only within that identity, and add regression tests.
3. Then resume the device-aware `reload_modem_modules()` result/readiness contract. Preserve the frozen backup and do not repeat the completed startup-protocol/bridge work.

## Governance constraints

Preserve security, architecture, compatibility, target-toolchain and mandatory testing. Use only `main`; do not touch the frozen backup. Update root `dev_log.md` for each code/documentation commit; keep it within 20–119 records and the size limits; preserve archived shards/indexes verbatim. Distinguish source, CI and firmware validation levels. No force-push, published history rewrite, or unrun-test/build/hardware claim.
