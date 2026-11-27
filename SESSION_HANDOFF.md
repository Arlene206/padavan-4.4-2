## 2026-11-27 — ZeroTier static-route duplicate preflight checker correction

- Current parent `main`: `c8c3256f339b70bf3e66a1526275e0e94ccfe508`; frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (unchanged).
- Duplicate static-route destination source/tests were committed in `c8c3256f339b70bf3e66a1526275e0e94ccfe508`. Exact-SHA ZeroTier run #46054412381: shell lifecycle regression PASS; Python contract checker FAILED before route assertions because its C-comment scanner parsed the POSIX case pattern `*/*)` as a block-comment opener. WAN-up integration was skipped. AI handoff #46054412361, Process inventory #46054412408 and WAN route #46054412388 passed on that SHA; XY-C1 firmware run #46054412480 was still in progress at last poll.
- Corrective slice changes the slash case pattern to `*'/'*)` (same shell matching semantics, no accidental `/*` token) and adds a checker assertion to lock this parser-compatible form. Duplicate destination logic is unchanged; shell tests had already passed on parent, but the full workflow must pass on the corrective SHA.
- Local shell execution and target build are unavailable here. No XY-C1 build/TRX/artifact or physical L4 result is claimed; physical L4 remains NOT VERIFIED.

## Exact first action

1. Fetch Actions for the corrective commit by exact `head_sha`. Verify ZeroTier lifecycle shell + Python source contracts + WAN-up integration all pass. Fix any remaining issue with a full-tree atomic `main` commit.
2. Check XY-C1 Build/TRX/artifact on the latest exact source SHA and the Process/NVRAM workflows; pending jobs are not passes. Continue with an independent P1 stability issue without repeating bootstrap/handoff audits.

---

## 2026-11-26 — Autonomous development checkpoint

- Current `main`: `3cc85ff726be701162e820a6fed0f282f5eba355`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (not modified).
- This session's DNS Forwarder work:
- `fix(dns-forwarder): report CLI startup failures`: `93f2b137ea212a5fbc857fae09faec384bcb519b` — XY-C1 Firmware Validation #45848922656: in_progress/—; Process lifecycle inventory - 93f2b137ea212a5fbc857fae09faec384bcb519b #45848922607: completed/success; NVRAM transaction boundary audit #45848922583: completed/cancelled; DNS Forwarder launch path stability regression #45848922574: completed/failure; AI handoff documentation contract #45848922565: completed/success
- `fix(dns-forwarder): unwind listener startup failures`: `9a6d645f10ea2032819ac63467c0cdb2a10ca29f` — NVRAM transaction boundary audit #45848220502: completed/cancelled; Process lifecycle inventory - 9a6d645f10ea2032819ac63467c0cdb2a10ca29f #45848220500: completed/success; AI handoff documentation contract #45848220397: completed/success; XY-C1 Firmware Validation #45848220389: in_progress/—; DNS Forwarder launch path stability regression #45848220378: completed/failure
- `fix(dns-forwarder): validate upstream address and port before setup`: `a039b68d37cb3eb5f2acdd4ae7d448c6851d5223` — NVRAM transaction boundary audit #45847194072: completed/cancelled; AI handoff documentation contract #45847194027: completed/success; XY-C1 Firmware Validation #45847193944: completed/success; DNS Forwarder launch path stability regression #45847193941: completed/success; Process lifecycle inventory - a039b68d37cb3eb5f2acdd4ae7d448c6851d5223 #45847193923: completed/success
- `test(dns-forwarder): correct event-loop compile inputs`: `050365be1040bd88b2c06d0942f0d43f3f92dab6` — AI handoff documentation contract #45846263346: completed/success; DNS Forwarder launch path stability regression #45846263342: completed/success
- `fix(dns-forwarder): roll back failed event source registration`: `3ccc3c94fe83bc0acdcf5c4a86aa70e302040bb6` — AI handoff documentation contract #45846000394: completed/success; Process lifecycle inventory - 3ccc3c94fe83bc0acdcf5c4a86aa70e302040bb6 #45846000350: completed/success; NVRAM transaction boundary audit #45846000085: completed/success; DNS Forwarder launch path stability regression #45846000054: completed/failure; XY-C1 Firmware Validation #45846000051: completed/success
- Implemented: strict ASCII-decimal upstream port parsing (1–65535) and pre-resource upstream validation; transactional listener initialization that checks `FIONBIO`, `SO_REUSEADDR`, bind, source allocation/registration and unwinds all partial resources; CLI startup now propagates failures and correctly tracks ownership of the DNS-port string.
- Host regressions: parser boundary/error behavior and event-loop registration rollback/re-attach tests are wired into `.github/workflows/dns-forwarder-launch-path.yml`. Hardware/router runtime testing has not been performed here.
- First next task: add deterministic failure-injection coverage for DNS Forwarder constructor setup (`getaddrinfo`, `socket`, `ioctl(FIONBIO)`, `setsockopt`, bind and event-source registration), then fix any remaining event-loop/daemon shutdown defects. Continue checking the Actions attached to each exact commit SHA; do not repeat completed startup/handoff audits.

# Session Handoff — 2026-11-26

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`; current base for the WSD HTTP-contract checker correction: `c4a627632235f2e240797a41294e5dc062848f06`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (unchanged; do not modify). PR #1 remains closed and unmerged.
- Bootstrap/protocol recovery was completed earlier in this continuous project; do not repeat it. Work on `main` only with full-tree atomic commits and non-forced expected-SHA updates.
- Exact-SHA evidence: MiniUPnP event-timeout subscriber regression #45845370609 and AI handoff #45845370434 PASS on `a271fcc0f818b834b21fe9681717a18dd07f3c50`; U2EC bounded TCP framing #45846984410 PASS on `4d146475b0a34f4971a364dd5ece2775dc433f8a`. DNS Forwarder source checks on `a039b68d` and its XY-C1 run were in progress at last poll.
- Latest confirmed full L3 evidence is production source SHA `b81ac7815d7fbe3b4573f742b00cbdba19c2e0d7`, XY-C1 run #45844543988 with Build XY-C1, TRX verification and artifact upload all PASS. Later production descendants require their own source-triggered build. No physical-router/L4 validation is claimed.
- Recent source work includes MiniUPnP event timeout parsing/deadline hardening, networkmap ARP socket lifecycle fixes, DNS Forwarder event-source rollback/upstream input validation (`3ccc3c94` and `a039b68d`), and U2EC TCP receive timeout hardening (`2b6d5db9`).

## Active task

- WSD SOAP Action/MessageID parsing is hardened at `c4a627632235f2e240797a41294e5dc062848f06`: raw `strstr()` tag hunting has been replaced with a bounded XML token scanner, and the GNU99 ASan/UBSan parser/reflection regression passed (#45847368757).
- The same source commit exposed a stale independent HTTP-framing checker: it still expected the old internal `strchr()` nested-tag implementation, rather than the new delegating bounded scanner. That exact-SHA workflow failed only at this obsolete source assertion (#45847368988). Updated the checker to assert the bounded wrapper and helper contract; this regression rerun is pending on the next commit.
- XY-C1 build for `c4a6276` had passed production-input detection and host checks and was compiling Build XY-C1 at last poll (#45847369057); TRX/artifact remain unverified until the run completes. No physical L4 validation.

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

## Completed production work — DHCPv6 config-writer token validation

- `start_dhcp6c()` now validates the interface token with `wan_ifname_copy()` before generating `dhcp6c.conf`. DHCPv6 IA #45835968592 PASSed with the GNU89 ASan/UBSan adversarial-token test; dependent WAN DHCP contract correction #45836670352 PASSed.

## Verification boundary

- Latest main parent before this source slice: `f2d7d6907939b93745eb67fdb51f89d75ba2ffc0`. Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; unchanged.
- DHCPv6 config-token validation source `3cb9d6b909da6ef09656567d244aee26d4a3c4ff`: dedicated DHCPv6 IA/source-contract #45835968592 PASS; dependent WAN DHCP contract correction #45836670352 PASS.
- AdGuardHome resolver-failure source `bbcfd8532dec133ff8c60b690f8aba9017af7acc`; test/checker descendant `b4758abe4d81752f604013d6819ce4ba9d29e9c6`: exact config-security #45837257425 PASS.
- XY-C1 #45837257027 on `b4758abe` PASSed host checks but skipped Build XY-C1, TRX verification and artifact upload because that commit changed only tests/checkers. Last confirmed full L3 build remains #45831671503 on production source baseline `43ba9c6`, artifact ID `15123331261`, SHA-256 `19dfd06a2ad7c6427a55e26505552a8a3bfbc6f6e1bff2259304426261f2bc9c`; it does not validate subsequent DHCPv6, AdGuardHome, networkmap, SmartDNS or LAN DHCP source changes.
- Physical-router/hardware L4 remains NOT VERIFIED. Never treat a green run that skipped Build/TRX/artifact steps as a firmware build.

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

1. Inspect the exact-SHA WSD HTTP framing contract rerun; verify the updated checker no longer requires the removed `strchr()` implementation and that parser and XML token scanner invariants remain asserted.
2. Check the XY-C1 run for `c4a6276`; require Build XY-C1, TRX verification and artifact upload to PASS before marking this source SHA L3-verified.
3. Continue the next concrete P1 stability/security repair in another userland component. Do not repeat the WSD scanner review; frozen backup remains untouched and hardware/L4 remains NOT VERIFIED.

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

