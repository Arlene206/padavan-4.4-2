# Session Handoff — 2026-11-26

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main` only.
- Source checkpoint at this handoff: `72225a52c467a09dcdd5a4e252451d8d9df7eaf1` (`fix(lan): report resolver rollback uncertainty accurately`). The documentation checkpoint commit that appends this handoff will be its child; do not mistake the documentation-only delta for a new production-source build.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; verified unchanged. PR #1 is closed and unmerged.
- Bootstrap/protocol restoration was completed in the earlier session; do not repeat it. Continue from the exact next task below.
- Use complete multi-file Git trees and one commit per logical task. Update `main` only with non-forced expected-SHA compare-and-swap. Never modify the frozen backup or force-push.

## This session — LAN DHCP resolver transaction consistency

- Confirmed a concrete transaction-boundary defect: `lan_publish_resolver_auto()` participates in the recoverable LAN DHCP address/route transition, but the old single-file publisher could return an error after `rename()` had already made the new `/etc/resolv.conf` visible (a parent-directory fsync failure). The caller then rolled kernel address/routes back while leaving the candidate resolver published.
- Production fix `0c897b7ee4fd80bddad936b7d5bc3d8433917676`: expose the rename-completed boundary from the internal staged-file publisher while retaining the original API for other callers; add a rollback-aware publisher using the existing hard-link snapshot/restore primitives; switch the automatic DHCP resolver writer to the rollback-aware path. A deterministic C regression injects an error after successful rename and verifies restoration when the prior resolver existed and removal when it did not.
- Follow-up checker/workflow commits corrected stale source contracts and fixture initialization: `57bd20e60f73f20d41fed7f08fee5fc5bdde7448`, `24ec9ce37430eaadcdfd3edc3f325ed4737f25b4`, `e428e722c2fbabde120a832bf93316bcf513d7ea`, `8f96eb6ecee967d448667072393aa99758079e7e`, `8477a7777fbd49717ae89a93011671500ce1a9c4`, and `b25835d921e2f3d9b3ae8fec39cb93549aa26e5e`.
- Final diagnostic hardening `72225a52c467a09dcdd5a4e252451d8d9df7eaf1`: corrected the post-restore diagnostic so it does not claim a backup link is retained when the restore rename already consumed that link but the subsequent durability fsync failed. The message now reports rollback/durability-confirmation uncertainty and the observed recovery-link pathname accurately.
- The initial exact-SHA checker failures were stale assertions/fixture setup, not evidence that the injected C behavior failed. They were fixed in subsequent atomic commits; do not reopen those old assertions without new evidence.

## Exact-SHA verification boundary

- Latest production-source SHA: `72225a52c467a09dcdd5a4e252451d8d9df7eaf1`.
- Exact-head Actions on that SHA: DNSMasq lifecycle #45819215235 PASS (includes the resolver helper host regression and source contracts); WAN DHCP callback #45819215341 PASS; HTTPD security #45819215135 PASS; NVRAM transaction audit #45819215141 PASS; WAN route #45819215171 PASS; AI handoff docs #45819215253 PASS. The other non-product exact-head checks in the batch also completed PASS.
- XY-C1 Firmware Validation #45819215001 is still IN_PROGRESS on this exact SHA. At last poll, all host/pre-build checks passed; `Build XY-C1` was still running and `Verify TRX output` plus artifact upload were pending. Recheck this exact run. Do not claim product L3 until Build, TRX verification and artifact upload all PASS on the same production-source SHA.
- No local full firmware build or physical-router test is claimed. Hardware/L4, live modem, live WAN and live DHCP/DNS behavior remain NOT VERIFIED.
- Earlier exact-head DNSMasq failures were resolved with the follow-up changes above; the latest DNSMasq run #45819215235 is the authoritative result for the current source checkpoint.

## DHCP event-flow review

- Rechecked BusyBox `udhcpc` INIT/REQUESTING/BOUND/RENEWING/REBINDING transitions, NAK handling, `DECONFIG_PENDING` retry, SIGUSR2/release handling and WAN/LAN RC event handlers.
- The current `leasefail` handlers are intentionally non-destructive: before an ACK there is no committed lease to deconfigure. NAK handling invokes the separate noack notification and BusyBox itself runs `deconfig` for an active lease; failures remain pending and are retried. LAN/WAN stop paths request release and wait without force-killing a client whose deconfig is pending.
- No new defect was established in that bounded review, so no speculative source change was made. Existing callback tests/contracts intentionally require WAN route/MTU failures to remain visible to BusyBox and persist retry state.

## Exact first action for the next GPT

1. Read this handoff and verify live `main`, frozen backup and exact-SHA Actions only; do not repeat bootstrap or the completed LAN resolver work.
2. Continue with the WAN DHCP BOUND/RENEW failure contract: trace what happens when the callback has published the candidate NVRAM lease but `wan_up_internal()` reports route/MTU failure and the callback returns nonzero to BusyBox. Build a focused GNU89 state-machine regression around repeated ACK/rebind/lease-deadline behavior before modifying production logic. Preserve the current fail-closed retry contract unless a concrete inconsistent state is demonstrated.
3. Recheck XY-C1 #45819215001 on source SHA `72225a52c467a09dcdd5a4e252451d8d9df7eaf1`; do not wait on it before doing independent work.

## Mandatory checkpoint and cross-session rules

- Before each session ends, verify the live source baseline, exact-head tests, pending blockers, current active task and precise next action. Update this handoff and `dev_log.md` when engineering progress or validation state materially changes.
- Continue the loop: source change → focused regression → atomic commit → exact-SHA Actions. PASS proceeds; FAIL is diagnosed and repaired; PENDING never blocks independent work.
- Keep every logical milestone atomic: create all blobs, one complete Git tree, one commit, then expected-SHA non-forced update of `main`. No partial-file commits or force-push.
- PENDING runs and earlier-SHA results do not validate a later SHA. Product L3 requires Build XY-C1, TRX verification and artifact upload all PASS on one production-source SHA. Physical L4 requires actual hardware testing.
- Preserve the frozen backup and do not reopen PR #1. Routine snapshot archives are prohibited; update the roadmap only for material module/status changes.

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

