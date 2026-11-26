# Session Handoff — 2026-11-26

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main` only.
- Current `main` parent before the DHCPv6 config-writer hardening: `00dd6ef26b8f0e76d49fcd63d24b4ad61eebeccf`. The latest confirmed production firmware baseline is `43ba9c6d0f5fd86bb94ad45a6cb8bf6b6ce3dfe1` (NVRAM rehash fix); the WAN/IPTV retry-marker fix is `80db7072a08e98b80a7e8290040d8d2e910aff65`. Checker/workflow descendants include WAN MTU `14a0ec8`, USB modem `4e9b81a`, detect_internet `c21826e`, and the later AdGuardHome rollback fix `00dd6ef`.
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

## Active task

- Harden the DHCPv6 config writer: `start_dhcp6c()` interpolates a bare interface-name token into `/etc/dhcp6c.conf`; it now validates that token with the shared safe-name policy before reading settings, locking the callback session or emitting config. Source, checker, workflow filters and GNU89 adversarial-token test are committed atomically; inspect exact-SHA DHCPv6 IA and XY-C1 Actions before further changes.

## Verification boundary

- Documentation parent before this update: `fab825ac680bca65603b99f6c2dc0b16c7b3a102`. Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; unchanged.
- BusyBox repeated-REBINDING callback regression `7be435059c1f8ffbd757ea95ba41fe466557960f`: exact run #45825922148 PASS. It proves failed callbacks do not commit a new server/requested-IP/BOUND state or extend the existing lease deadline. No production `dhcpc.c` defect was demonstrated.
- Inadyn transaction fix `4c0b2a02b021c813845e3a5356cdfd0cf987c4ab`, fixture correction `916fbec0fcc6739548870888fdbb24216683955c`, checker descendants `2b2b6ec122e5b4e9c7f94c3599ffd0c4da60c526` and `ece28b515b546cd8539089975322cb79ecbe6395`: Inadyn #45826971290 PASS and DNSMasq #45826971254 PASS on exact `ece28b5`.
- VSFTPD config commits: `63972673ad970fb439a175b21499078d6433c09f` (staged-file + parent-directory fsync), `65d6e0030edcebdf568dd4dba3ba281a143654aa` (GNU89 prototypes), `7f9a288dd74d7cbfdecf03f3b7ea4709bccfd740` (/etc symlink compatibility), `0eaada3d45cc8c627a902a962927ad40a3ec5330` (checker definition matching), and `f0fbe839174fb6b569c036f21c34a55903ed5965` (XY-C1 path trigger).
- VSFTPD host regression #45828730341 PASS on exact `0eaada3`; Process inventory #45829100150 PASS on exact `f0fbe839`. XY-C1 #45829100040 PASS on exact `f0fbe839`: Build XY-C1, TRX verification and artifact upload all completed successfully. Artifact ID `15121837299`, SHA-256 `dfcbc1cd76623eacf6cf33c6f95267983af6ad236a3732927ae955d58366ad7e` (13 MB TRX). This verifies the FTP durability source baseline at L3; it does not verify the later DHCP marker change.
- The previous complete XY-C1 #45826697813 PASS was on `916fbec0`, before the VSFTPD helper change; it does not verify this newer source. No current FTP L3 claim until #45829100040 Build/TRX/artifact all PASS. No physical-router result is claimed; hardware/L4 remains NOT VERIFIED.

## BusyBox udhcpc repeated-REBINDING failure regression

- Added `test_repeated_failed_rebinding_acks_preserve_server_and_deadline()` and source checks ensuring repeated callback failures cannot commit the alternate server, requested IP or BOUND state, or extend the old lease deadline.
- Exact test #45825922148 PASS on `7be435059c1f8ffbd757ea95ba41fe466557960f`. Production source remained unchanged because the tested review did not establish a state-machine defect.

## Inadyn SIGHUP configuration transaction

- `notify_ddns_update_locked()` snapshots the last-known-good config before replacement, restores it on write failure, retains the recovery link when SIGHUP delivery is uncertain, and commits the snapshot only after successful signal delivery and parent-directory sync.
- GNU89 rollback regression added. Production commit `4c0b2a02b021c813845e3a5356cdfd0cf987c4ab`; fixture correction `916fbec0fcc6739548870888fdbb24216683955c`; checker fixes `2b2b6ec122e5b4e9c7f94c3599ffd0c4da60c526` and `ece28b515b546cd8539089975322cb79ecbe6395`. Inadyn #45826971290 and DNSMasq #45826971254 PASS.

## VSFTPD configuration durability

- `ftp_config_file_publish()` flushes and fsyncs staged data, renames it, then fsyncs the parent directory with EINTR retry. If parent sync fails after rename, the error reaches `run_ftp()`, which refuses daemon launch. Parent lookup follows configured `/etc` resolution.
- GNU89/Werror regression and source-order contract passed VSFTPD #45828730341 on `0eaada3`. Exact XY-C1 #45829100040 on `f0fbe839` remains the required product build check.

## Exact first action for the next GPT

1. Inspect exact-SHA DHCPv6 IA publication workflow for the config-writer token validation and adversarial GNU89 test; repair any regression/source-contract failure.
2. Inspect XY-C1 Build XY-C1, TRX verification and artifact upload independently. The last confirmed full L3 build was run #45831671503 on production source baseline `43ba9c6` (artifact ID `15123331261`, SHA-256 `19dfd06a2ad7c6427a55e26505552a8a3bfbc6f6e1bff2259304426261f2bc9c`); checker-only descendants do not change that production source baseline.
3. Continue the next independent stability/security issue across the remaining userland. Hardware/L4 remains NOT VERIFIED; PENDING Actions must not block independent work.

## Mandatory handoff checkpoint

Every session MUST perform an end-of-session accuracy checkpoint. Check the live `main` SHA, frozen backup SHA, exact-head Actions, completed/failed/pending tests, unresolved blockers, and the precise first action for the next session. Earlier-SHA PASS results do not validate a later SHA. **The checkpoint is mandatory; a document edit is conditional.** When all checkpoint facts and the first action are already accurate, conclude the check without a no-op file change or commit.

# Cross-session execution mandate

- Treat the repository as one continuous engineering effort across GPT conversations. Recover once, resume the exact unfinished task, and do not ask the user what to do next.
- Complete each logical source/test/workflow slice with one atomic main commit: create every required blob, one complete tree, and one commit; update `main` only through a non-forced expected-SHA compare-and-swap. Never create partial-file commits or alter the frozen backup.
- After every commit, inspect Actions for that exact SHA. PASS permits the next slice; FAIL requires diagnosis and repair; PENDING is not a stop condition while independent work remains.
- Before context exhaustion, finish and commit a safe bounded slice, verify its exact-SHA tests/build boundary, and update this handoff with the committed baseline and precise first action. Keep production-source and documentation-only SHAs distinct.
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

