# Session Handoff — 2026-11-26

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; develop on `main` only.
- Parent `main` at this checkpoint: `992bdf6da9ba45f217c88bdf42afea69932b45e7`. This parent is a one-line comment-only follow-up clarifying that retained DNSMasq rollback links are logged for manual recovery.
- Frozen `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify or delete. PR #1 remains closed/unmerged.
- Bootstrap/protocol recovery has already been completed; do not repeat. Read this handoff and continue from the exact first action below.
- Parent exact-head DNSMasq lifecycle #45814219448 PASS and AI handoff docs #45814219362 PASS on `3147c02f2bddba93a6034e529c5d8935b2768a05`. Parent `992bdf6` triggered a new exact-head batch; at handoff #45814745710 DNSMasq and #45814745724 XY-C1 were both IN PROGRESS.

## Active task

### DNSMasq failed-start rollback — source fixed; latest exact-head rerun pending

- Production source `f7b61ba09b9750b3e85c3884b6101a511a12c6e1`: start/restart no longer discard static-host file rollback snapshots before the detached dnsmasq daemon passes its RUNNING postcondition. Snapshot commit occurs only after `dnsmasq_confirm_started()` succeeds.
- On launch failure, the helper stops/rechecks the candidate daemon. Prior generated files and ARP are restored only after process inventory confirms ABSENT; RUNNING or UNKNOWN retains the current matched config/ARP generation and private rollback links. The original startup/postcondition error remains authoritative; rollback errors are logged separately.
- `992bdf6da9ba45f217c88bdf42afea69932b45e7` only clarifies the retained-link manual-recovery log comment; it does not change rollback behavior.
- Checker/workflow assertion follow-ups: `f6de10e6...` updated the failed-start helper call signature; `9166a3d4...` corrected compacted argument matching and stop-vs-launch ordering; `3147c02...` changed inline workflow assertions to require failed-start rollback before success commit.
- Exact DNSMasq run #45814219448 PASS on `3147c02...`. Current-head #45814745710 was IN PROGRESS at handoff; recheck exact SHA `992bdf6...` before concluding.

### QMI WDS client/session identity — source fixed, exact USB regression PASS

- Production change `4bfccc2e7c46df3495544ae0bb46f473775352f0`: stop reads the persisted WDS CID and PDH marker, fails closed on invalid/unreadable/inconsistent markers, and passes `--set-client-id wds,<saved-CID>` before `--stop-network`. It never lets uqmi silently allocate a fresh CID for stop.
- After confirmed stop, PDH marker removal is checked; only then is the same CID released; only after release is the CID marker removed. Failure preserves the relevant marker for retry.
- Follow-up checker/workflow repairs culminated in `bdc8992f6e9a7a6884422c9af3c88177abc700c5`; exact USB modem #45813115174 PASS and AI handoff #45813115104 PASS. These are host/CI L2 results, not live modem hardware verification.

### Other exact results for DNSMasq production source SHA `f7b61ba...`

- PASS: NVRAM transaction boundary #45813611887; WAN DHCP callback #45813611721; Process inventory #45813611693; miniupnpd atomic config #45813611534; WAN route #45813611530; Process lifecycle #45813611502; HTTPD #45813611375; Samba #45813611309; Inadyn #45813611245.
- XY-C1 run #45813611427 on `f7b61ba...` was still building at last poll. The current source head `992bdf6...` has its own XY-C1 run #45814745724, last observed in progress. Do not claim L3 unless Build XY-C1, Verify TRX output and artifact upload all PASS on one exact product source SHA.

## Verification boundary

- Main before this documentation refresh: `992bdf6da9ba45f217c88bdf42afea69932b45e7`; frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- L1/L2 host regressions and exact-head contracts are verified only where exact run IDs and SHAs are listed. No local full firmware build is claimed.
- Product L3 is NOT VERIFIED for the latest production source. Hardware L4, physical modem tests, live WAN and live DHCP/DNS behavior remain NOT VERIFIED.
- PENDING runs do not block independent source work. A result on an earlier SHA does not validate a later SHA, even when the later diff is comment-only.

## Exact first action for the next GPT

1. Read live `main` and this handoff only; do not repeat bootstrap or completed audits. Confirm the backup ref remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, then inspect exact-head Actions for the resulting documentation commit.
2. Recheck DNSMasq #45814745710 and XY-C1 #45814745724 on `992bdf6da9ba45f217c88bdf42afea69932b45e7`. At last poll both were in progress. If either remains pending, continue independent work rather than waiting. L3 requires same-SHA Build XY-C1, TRX verification and artifact upload all PASS.
3. Continue the roadmap's open LAN DHCP client/AP-mode lease slice, starting with `leasefail` and the full live-transition path. Inspect actual event-script side effects and the existing BOUND/RENEW, failed-deconfig retry, NAK, packet-send and deadline contracts before changing behavior. Add focused GNU89 regression/source assertions only after confirming the state-machine semantics.

## Mandatory handoff checkpoint

Every session MUST inspect the live repository state and exact-head Actions before ending. Verify source baseline, active task/progress, test/CI boundary, blockers and exact next action. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

# Cross-session execution mandate

Continue as the same maintainer across conversations. Every reviewable source/test milestone must be one atomic main commit: create all changed blobs, one complete Git tree and one commit, then update `main` with a non-forced compare-and-swap against the expected parent SHA. Never publish partial-file commits or force-push.

Before context exhaustion, complete a safe bounded slice, inspect exact-SHA Actions and update this handoff with the precise next task and validation boundary. Do not ask the user what to do next.

## Execution invariants

- Main only; full tree/atomic commit; expected-SHA non-forced update; never force-push.
- Preserve the frozen backup and do not reopen PR #1.
- PENDING never blocks independent safe work.
- Product L3 requires same-product-source-SHA Build XY-C1, TRX verification and artifact upload all PASS. Physical L4 requires actual device testing.
- Update `dev_log.md` for completed engineering milestones, handoff for material progress/validation changes, and roadmap only for material module/status changes. Routine snapshot archives are prohibited.

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

