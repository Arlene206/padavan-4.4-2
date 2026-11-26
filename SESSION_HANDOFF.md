# Session Handoff — 2026-11-26

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main` only.
- Current documentation parent: `21822988b5b299e4f931d6659c175515952bdc57`. Latest production-source behavior is in `15274181953dc258d6cec4e47d9038d2e3187b47`; the intervening `e49e86262257c592ad1e0be1b68cccd49c476517` preserves the MAN wrapper signature. The documentation commit that follows this checkpoint does not change production code.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; verified unchanged. PR #1 is closed and unmerged.
- Bootstrap/protocol restoration was already completed. Do not repeat it. Do not reopen PR #1 or modify the frozen backup.
- Use full multi-file Git trees and one atomic commit per logical task. Update `main` only with a non-forced expected-SHA compare-and-swap. Never force-push.

## Completed production work — LAN DHCP resolver transaction

- Confirmed that the automatic LAN DHCP resolver publisher could return failure after `rename()` had already exposed the candidate `/etc/resolv.conf` (for example, a parent-directory fsync failure). The outer LAN DHCP transition could then restore the prior kernel address/routes while leaving the candidate resolver visible.
- Production fix `0c897b7ee4fd80bddad936b7d5bc3d8433917676`: expose the rename-completed boundary from the internal staged-file publisher while preserving the original API; add rollback-aware publication using the existing hard-link snapshot/restore primitives; route the automatic LAN DHCP resolver update through the rollback-aware helper.
- Deterministic GNU89 C regression injects a failure after successful rename and verifies restoring an existing resolver and removing a newly created resolver when no previous file existed. Follow-up diagnostics in `72225a52c467a09dcdd5a4e252451d8d9df7eaf1` distinguish rollback failure/durability uncertainty without falsely claiming a recovery link still exists.
- Checker/fixture follow-ups: `57bd20e60f73f20d41fed7f08fee5fc5bdde7448`, `24ec9ce37430eaadcdfd3edc3f325ed4737f25b4`, `e428e722c2fbabde120a832bf93316bcf513d7ea`, `8f96eb6ecee967d448667072393aa99758079e7e`, `8477a7777fbd49717ae89a93011671500ce1a9c4`, and `b25835d921e2f3d9b3ae8fec39cb93549aa26e5e`.

## Completed production work — WAN / IPTV DHCP BOUND callback status

- Confirmed that DHCP BOUND callbacks consumed the aggregate return from `wan_up_internal()` / `man_up_internal()`. Those aggregates include ancillary service startup failures after address/route setup (firewall, multicast/IGMP, DDNS, watchdog, monitoring and other follow-up services). A nonzero ancillary status could therefore make BusyBox treat an otherwise applied lease ACK as a failed script callback and unnecessarily retry/rebind.
- Production fix `c36ab9c49ebd68fde9b07454da65e19212c182fd`: introduce `wan_up_internal_with_route_status()` and `man_up_internal_with_route_status()`, which report the DHCP-critical address/IPv4-route result before ancillary services, while retaining old aggregate wrappers for existing non-DHCP callers. `udhcpc_bound()` consumes the scoped status.
- Production fix `15274181953dc258d6cec4e47d9038d2e3187b47`: generalize the lease callback selector to `dhcp_bound_callback_status(route_setup_status, mtu_status)` and use it for both normal WAN and IPTV DHCP. IPTV multicast startup remains separately logged/managed and no longer rejects a lease ACK; route, MTU and required NVRAM/retry-state publication errors remain visible to BusyBox. The MAN wrapper source-signature compatibility adjustment is `e49e86262257c592ad1e0be1b68cccd49c476517`.
- Added/extended GNU89 policy regression `test_bound_callback_status_ignores_auxiliary_service_failures()` and source contracts for the route-only boundary. Checker follow-ups: `45cb6f12843035d47ce61dd2d4fa6dd58cd39e79`, `3f54cf2ba737835e9ea1a028fbe0fd229fe113e9`, `cbb470d0054854b3d8469a699ce78601f537c881`, `2377c6094e9acc5358a8abc6873f039eb057438f`, `f14f2b4d4d450f8dec403ed17ca74533ba31d204`, `e41af1199ab8279421fbed098557181767b92f01`, `405bb48aad75b5ea89aaec668a1b1f79fb140f97`, and `21822988b5b299e4f931d6659c175515952bdc57`.
- WAN callback follow-up on exact SHA `405bb48aad75b5ea89aaec668a1b1f79fb140f97`: WAN DHCP callback #45824870841 PASS. The route-status workflow failed on stale assertions against the old callback source shape; those assertions were aligned in `21822988b5b299e4f931d6659c175515952bdc57`; exact rerun #45825386107 was QUEUED at the last poll.

## Exact-SHA validation boundary

- Current source-checker parent before this documentation commit: `21822988b5b299e4f931d6659c175515952bdc57`. Exact-head Actions observed: AI handoff docs #45825386055 QUEUED; WAN route status #45825386107 QUEUED; XY-C1 validation #45825386117 QUEUED.
- On prior checker/source descendant `405bb48aad75b5ea89aaec668a1b1f79fb140f97`, WAN DHCP callback #45824870841 PASS. WAN route #45824870995 FAIL was caused by stale workflow assertions and is superseded by the source-contract update in `21822988`; the exact rerun is pending as above.
- XY-C1 run #45824870811 reported overall SUCCESS, but its `Build XY-C1`, `Verify TRX output`, and artifact-upload steps were all SKIPPED. This is not a firmware build pass and does not establish L3. The exact current run #45825386117 was queued at checkpoint; recheck its build, TRX and artifact steps independently.
- No local full firmware build or physical-router verification is claimed. Hardware/L4, real WAN/modem operation and live DHCP/DNS behavior remain NOT VERIFIED. Do not infer L3 from a green workflow whose product build steps were skipped or from a different source SHA.

## DHCP callback / BusyBox follow-up

- The callback now returns nonzero for route/MTU errors and required NVRAM lease/retry-state publication errors, while not treating ancillary service startup as DHCP ACK failure. Candidate lease tuple publication occurs before the WAN-up route operation; the callback records `route_err`, `mtu_err`, and general error state after that operation.
- Prior bounded event-flow review found `leasefail` non-destructive before ACK, NAK separate from active-lease `deconfig`, `DECONFIG_PENDING` retries, and stop/release paths wait rather than force-kill while deconfig is pending. No independent state-machine defect was established in that review.
- The next investigation is deliberately narrower: trace what BusyBox does after a nonzero `bound`/`renew` script result when the new NVRAM lease tuple has already been published but route or MTU application failed. Verify REQUESTING/BOUND/RENEWING/REBINDING transitions, T1/T2/lease-deadline handling and repeated ACK behavior. Build a focused GNU89 state-machine regression before making any further production change; preserve fail-closed retry semantics unless a concrete inconsistent state is demonstrated.

## Exact first action for the next GPT

1. Recheck live `main`, frozen backup, and exact-head Actions only; do not rerun bootstrap or re-audit the completed LAN resolver work.
2. Inspect BusyBox `udhcpc` handling of a nonzero `bound`/`renew` script status in `trunk/user/busybox/busybox-1.24.x/networking/udhcp/dhcpc.c`, alongside the WAN/IPTV callbacks. Add a focused GNU89 regression for failure/retry/rebind/lease-deadline behavior before changing source.
3. Do not wait on pending CI; pursue independent source work and validate each atomic commit against its exact SHA.

## Mandatory handoff checkpoint

Every session MUST perform an end-of-session accuracy checkpoint. Check the live `main` SHA, frozen backup SHA, exact-head Actions, completed/failed/pending tests, unresolved blockers, and the precise first action for the next session. Earlier-SHA PASS results do not validate a later SHA. **The checkpoint is mandatory; a document edit is conditional.** When all checkpoint facts and the first action are already accurate, conclude the check without a no-op file change or commit.

## Mandatory development loop

- Modify source → focused regression → atomic commit → check GitHub Actions for the corresponding exact SHA. PASS proceeds; FAIL is diagnosed and fixed; PENDING never blocks independent work.
- For multi-file work, create all blobs and one complete Git tree before one commit. Update `main` by expected-SHA non-forced compare-and-swap only. Never leave partial-file commits, force-push, or modify the backup branch.
- At each handoff, record the production-source SHA separately from documentation/checker-only descendants and accurately state which exact-SHA product build steps ran or were skipped.
- Preserve hardware/L4 NOT VERIFIED unless actual hardware validation has been performed.

## 2026-11-26 GPT-6 continuation

- Repository: `Arlene206/padavan-4.4-2`; branch: `main`; HEAD at handoff: `1e3154270732eef0314244dec117dd325971638a`.
- Frozen backup branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (not modified).
- This session's source work before this handoff:
  - Samba paired passdb/config journal directory durability barriers; committed recovery-state logging is now truthful.
  - Ralink IAPP: bound driver-returned event-table length/count, copied packed event payloads to aligned storage, and corrected Linux TCP send/connect to use absolute monotonic deadlines.
  - DNSMasq: staged config fsync/parent sync and durable rollback links; failed start/restart restores the previous config and static-ARP snapshot only after daemon absence is confirmed. When process state is RUNNING/UNKNOWN, preserve the current config and retain rollback links for manual recovery.
  - Inadyn: introduced a private, inode-checked config hard-link snapshot; stage and publish keeps the previous config available through startup confirmation, and a confirmed-absent failed launch restores the previous config and restarts the prior instance. If daemon presence remains RUNNING/UNKNOWN, retain the snapshot instead of replacing a config that may still be in use. Host regressions cover restore-after-publish, first-file removal, successful commit, and symlink rejection.
- Inadyn follow-up hardening: snapshot restore now explicitly unlinks the extra backup hard link when destination and backup still reference the same inode. Plain `rename(backup,destination)` is a no-op for two hard links to one inode; without this branch the backup link could remain and violate the daemon's single-link config validation. A regression covers restore before any new config is published.
- Recent main commits (newest first):
- 1e3154270732eef0314244dec117dd325971638a fix(inadyn): release unchanged rollback hardlink safely
- 07bd0e547e92e9b6604f548872c489631afd693f test(inadyn): expect committed config after snapshot regression
- 5fe2ed6d4c9e79c011f7c141a96b1f8744a040be fix(inadyn): restore last-known-good config after failed launch
- 89e7b85aebfeefa52496b4ef5480cf46c2c1372e docs(ai): checkpoint DNSMasq and QMI stability work
- 992bdf6da9ba45f217c88bdf42afea69932b45e7 docs(dnsmasq): clarify retained rollback link recovery
- 3147c02f2bddba93a6034e529c5d8935b2768a05 test(dnsmasq): verify rollback before success commit
- 9166a3d4f836612e0545cb0b255ba252674323e2 test(dnsmasq): distinguish stop-only and launch commits
- Targeted Actions attached to handoff HEAD:
  - WAN route operation status regression: completed/failure (run 45816775778, SHA 1e3154270732eef0314244dec117dd325971638a)
  - Process lifecycle inventory - 1e3154270732eef0314244dec117dd325971638a: completed/success (run 45816775719, SHA 1e3154270732eef0314244dec117dd325971638a)
  - NVRAM transaction boundary audit: in_progress (run 45816775763, SHA 1e3154270732eef0314244dec117dd325971638a)
  - DNSMasq lifecycle stability regression: completed/failure (run 45816775693, SHA 1e3154270732eef0314244dec117dd325971638a)
  - Inadyn fail-closed lifecycle regression: completed/failure (run 45816775662, SHA 1e3154270732eef0314244dec117dd325971638a)
- Inadyn checks for initial integration SHA `5fe2ed6d4c9e79c011f7c141a96b1f8744a040be`:
  - XY-C1 Firmware Validation: in_progress (run 45816170557, SHA 5fe2ed6d4c9e79c011f7c141a96b1f8744a040be)
  - HTTPD Authentication and Security Regression: completed/success (run 45816170578, SHA 5fe2ed6d4c9e79c011f7c141a96b1f8744a040be)
  - Process lifecycle inventory - 5fe2ed6d4c9e79c011f7c141a96b1f8744a040be: completed/success (run 45816170449, SHA 5fe2ed6d4c9e79c011f7c141a96b1f8744a040be)
  - miniupnpd atomic configuration publication regression: completed/success (run 45816170951, SHA 5fe2ed6d4c9e79c011f7c141a96b1f8744a040be)
  - Samba share configuration stability regression: completed/success (run 45816170687, SHA 5fe2ed6d4c9e79c011f7c141a96b1f8744a040be)
  - DNSMasq lifecycle stability regression: completed/failure (run 45816170586, SHA 5fe2ed6d4c9e79c011f7c141a96b1f8744a040be)
  - WAN DHCP callback state stability regression: completed/success (run 45816170308, SHA 5fe2ed6d4c9e79c011f7c141a96b1f8744a040be)
  - Inadyn fail-closed lifecycle regression: completed/failure (run 45816170673, SHA 5fe2ed6d4c9e79c011f7c141a96b1f8744a040be)
  - WAN route operation status regression: completed/failure (run 45816170314, SHA 5fe2ed6d4c9e79c011f7c141a96b1f8744a040be)
  - Process inventory and lifecycle fail-closed regression: completed/success (run 45816170451, SHA 5fe2ed6d4c9e79c011f7c141a96b1f8744a040be)
  - NVRAM transaction boundary audit: completed/success (run 45816170429, SHA 5fe2ed6d4c9e79c011f7c141a96b1f8744a040be)
- Local/host test and firmware build results are only those explicitly represented by the Actions records above; do not infer a full XY-C1 build from host policy tests. Any PENDING or FAIL state above must be resolved or superseded by an exact-SHA rerun.
- First next task: inspect the exact-SHA Inadyn lifecycle workflow for the latest snapshot commits, resolve any failed host regression/compile step before expanding scope. Then review `notify_ddns_update_locked()`'s write-plus-SIGHUP failure semantics and decide whether its config should be snapshotted and rolled back; preserve the current contract unless evidence shows a defect.
- Keep commits atomic: complete all blobs, create one Git tree, one commit, then non-force update `main` with `expected_sha`. Do not touch the frozen backup branch.

