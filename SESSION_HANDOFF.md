# Cross-session execution mandate (read before resuming)

This is a continuing autonomous engineering assignment. The next GPT must recover live state once, then immediately resume the exact P1 task below and implement/test/commit—not repeat this session's documentation work, re-audit already completed evidence, or ask the user to choose a task. Do not stop after a plan or status report when safe source work is available. Keep working through bounded engineering slices while context/time permits.

**Mandatory handoff checkpoint:** Every session must verify before ending that this file accurately records the meaningful source baseline, active task/progress, relevant exact-head Actions, validation boundary, blockers and exact first action. If any next-session-critical fact is missing, stale or changed, update and commit one consolidated correction (prefer batching with source work). If everything is accurate, leave this file untouched and make no no-op documentation commit. The checkpoint is mandatory; a document edit is conditional.

For every multi-file slice, publish source + regression tests + required workflow/build integration as **one atomic main commit**: create blobs, one tree, one commit parented to the freshly fetched main SHA, then non-forced update of main with that same SHA as expected_sha. Never create partial per-file commits for one logical change. If main moved, rebase/reapply and retest; never force-push. Verify exact-head push Actions. Before context exhaustion, finish a safe slice and leave this handoff materially updated with committed progress, validation and the next exact action. Avoid progress narration; repository progress and a truthful continuation point are the priority.

## Current repository state

- Repository: Arlene206/padavan-4.4-2. Main parent for this slice: b39d77496885c65b1725dfc904788ec41f69f8fb. Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807 (read-only); PR #1 is CLOSED / UNMERGED.
- P1 HTTPD retained restore-upload protection is L2/L3 verified on product SHA ee18adc7fe9062d2fe5284a3e64802242b89fad0: HTTPD #43701369173, AI docs #43701369189 and XY-C1 #43701369143 PASS through MIPS build, TRX verification and artifact upload. Physical router L4 remains unverified.
- P2 DNSMasq mainline now includes SIGHUP ESRCH re-inventory; staged DHCP hosts/ethers/hosts.static; fail-closed preparation; input/domain validation; hostname publication with rollback; IPv6 tri-state restart; and boot/watchdog failure reporting. Key commits include bc2b1193, 393bb762, 64e5714d, 9e9bac08, 514572c3, 55ff022f, f188bacf, d74f61b1, 37350a66, c3227528, e749890c, 53cf5fbe, 34ec88e1, 7b452f49, bc91ec1f and b39d7749.
- Exact-head b39d7749 Actions: DNSMasq #43706419314 PASS; HTTPD #43706419305 PASS; AI handoff #43706419277 PASS; Process inventory #43706419288 PASS; NVRAM audit #43706419285 PASS. Earlier XY-C1 #43706197275 on 7b452f49 was still IN_PROGRESS at last poll (Build XY-C1/TRX/artifact not concluded).
- This slice tightens static DHCP MAC validation from "12 hex chars" to a nonzero unicast Ethernet address and adds regression coverage. No physical-router result is claimed.


## Active task

### P2 — DNSMasq lifecycle, generated inputs and validation

- restart_dns() re-inventories after SIGHUP ESRCH; only a complete post-scan proving ABSENT treats the race as a no-op. UNKNOWN and a replacement daemon remain errors.
- DHCP hosts, /etc/ethers and /tmp/hosts.static are privately staged then published; startup fails closed on stage/publish failure. Router/AP hostname+hosts pairs now use backup-and-rollback publication to restore the first destination if the second rename fails. Rollback failure preserves backup evidence for recovery.
- Static DHCP MAC validation formerly accepted all-zero and multicast addresses despite the regression name "unicast"; it now rejects all-zero and multicast (I/G bit set) while retaining local/unicast MACs. The policy test and DNSMasq source contract include this case.
- LAN domain validation blocks config-directive injection; WAN/AP lease IP/domain are validated, AP DHCP restart is gated on host publication, IPv6 restart uses tri-state process presence and boot/watchdog start failures are logged.
- Exact-head b39d7749: DNSMasq #43706419314, HTTPD #43706419305, AI docs #43706419277, process inventory #43706419288 and NVRAM #43706419285 PASS.
- Remaining high-risk source work: add best-effort backup/rollback for the three DHCP/ethers/hosts.static destinations as one prepared generation; inspect ARP command return status and cleanup.


## Explicitly uncompleted P1 work

- No outstanding P1 source change identified. HTTPD retained restore-upload protection is L2/L3 verified on ee18adc7fe9062d2fe5284a3e64802242b89fad0; physical restore/retry/reboot L4 remains NOT VERIFIED.
- P2: three DHCP/ethers/hosts.static renames are individually atomic; add rollback/recovery if a later publish fails after an earlier rename. Inspect ARP command return status and temporary-file/FD cleanup.
- P2: review hostname/hosts rollback failure recovery, resolver concurrency and daemon configuration rollback.
- Linux 4.4 PID/start-time revalidate-to-signal TOCTOU remains; continue kill_pidfile_s() and name-wide signal audits.
- FTP failed-upload unlink TOCTOU remains blocked by lack of inode-conditional unlink-by-fd; preserve STOR/APPE/REST/STOU semantics.
- NFS durability/auth/root-squash/external mount compatibility require real client/storage validation.


## Verification boundary

- HTTPD restore-upload L2/L3: HTTPD #43701369173, AI docs #43701369189 and XY-C1 #43701369143 PASS on product SHA ee18adc7fe9062d2fe5284a3e64802242b89fad0, including build/TRX/artifact. Physical router L4 remains NOT VERIFIED.
- Exact-head b39d7749: DNSMasq #43706419314 PASS; HTTPD #43706419305 PASS; AI handoff #43706419277 PASS; Process inventory #43706419288 PASS; NVRAM #43706419285 PASS. The pair rollback fault-injection test had initially failed on bc91ec1 because pair_first was not removed before creating a symlink; b39d7749 fixes this test setup and all listed workflows are green.
- The prior XY-C1 #43706197275 on 7b452f49 was still IN_PROGRESS at last poll. No L3 status for later sources unless exact Build, TRX verification and artifact upload all succeed.
- No local checkout/build or hardware device was available. No local compile or physical-device result is claimed.


## HTTPD daemon lifecycle — completed slice

- `start_httpd()` now uses `process_inventory_scan("httpd", ...)` with tri-state `ABSENT/RUNNING/UNKNOWN` semantics. Only an authoritative ABSENT result permits NVRAM configuration normalization, stale-state clearing, launcher execution, or firewall publication.
- A confirmed RUNNING daemon returns `EBUSY` and may publish `httpd_started=1`; UNKNOWN returns `EIO` (or scan errno) without mutating configuration or claiming the daemon is running.
- Detached-child readiness now requires RUNNING. ABSENT retries then returns `ESRCH`; UNKNOWN terminates the probe with an error and never publishes started state. GNU89 regression covers RUNNING, ABSENT, UNKNOWN, invalid states and preserved launcher errors.
- The first exact-head lifecycle Action exposed a stale source-contract assertion expecting the old `status == 0` form. The checker was corrected to assert the new fail-before-publish branch. Current exact run state is listed in the repository checkpoint above.

## Storage eject propagation — completed slice

- `read_disk_data_checked()` now strictly validates `/proc/partitions`: canonical four-column header, blank separator, complete newline-terminated records, exactly four fields, unsigned decimal major/minor/block counts, and overflow-safe conversion. Malformed, signed, truncated, overlong, or overflowed records fail closed; the legacy `read_disk_data()` pointer wrapper is preserved.
- `safe_remove_all_stor_devices()` distinguishes unknown inventory from valid empty inventory, checks the per-device path and generic fallback, and requires a complete final mount snapshot to prove zero storage mounts. Firmware update does not persist storage MTD or execute `mtd_write` when eject is incomplete; normal reboot/halt proceeds but skips storage MTD persistence on failure.
- Parser regression is compiled as GNU89 with `-Wall -Wextra -Werror`. The exact c5 parser Action initially failed earlier in the job on BusyBox `writev` missing its declaration; `b022942` added `<sys/uio.h>`, after which the storage-hotplug exact-head run is represented in the latest status block above. Do not treat prior SHA results as evidence for later source.
- Backup branch remains immutable; physical-router L4 remains unverified.

## Exact first action for the next GPT

1. Verify whether XY-C1 #43706197275 on 7b452f49 ever completed; require Build XY-C1, TRX verification and artifact upload before L3 claim.
2. Check exact-head Actions for this unicast-MAC validation commit; repair any FAIL.
3. Implement recovery for the three generated DHCP-host, /etc/ethers and /tmp/hosts.static publications if rename 2/3 fails after an earlier publish. Reuse the tested hostname-pair backup primitive, preserve backups if rollback fails, and add injected third-rename failure coverage.
4. Inspect ignored ARP command status, temporary-file/FD cleanup, and continue remaining resolver/concurrency/signal audit.
5. Re-fetch main before writes; produce complete tree/commit and non-forced expected-SHA update. Frozen backup remains immutable; hardware L4 remains unverified.

Source/validation notes:
- Parent main for this slice: b39d77496885c65b1725dfc904788ec41f69f8fb; frozen backup: 72f4f157a084f8184b9597ca5bf334b8080e5807.
- Preserve docs/dev-log archived shards and docs/dev-log/INDEX.md byte-for-byte.
