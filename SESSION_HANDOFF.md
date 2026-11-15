# Cross-session execution mandate (read before resuming)

This is a continuing autonomous engineering assignment. The next GPT must recover live state once, then immediately resume the exact P1 task below and implement/test/commit—not repeat this session's documentation work, re-audit already completed evidence, or ask the user to choose a task. Do not stop after a plan or status report when safe source work is available. Keep working through bounded engineering slices while context/time permits.

**Mandatory handoff checkpoint:** Every session must verify before ending that this file accurately records the meaningful source baseline, active task/progress, relevant exact-head Actions, validation boundary, blockers and exact first action. If any next-session-critical fact is missing, stale or changed, update and commit one consolidated correction (prefer batching with source work). If everything is accurate, leave this file untouched and make no no-op documentation commit. The checkpoint is mandatory; a document edit is conditional.

For every multi-file slice, publish source + regression tests + required workflow/build integration as **one atomic main commit**: create blobs, one tree, one commit parented to the freshly fetched main SHA, then non-forced update of main with that same SHA as expected_sha. Never create partial per-file commits for one logical change. If main moved, rebase/reapply and retest; never force-push. Verify exact-head push Actions. Before context exhaustion, finish a safe slice and leave this handoff materially updated with committed progress, validation and the next exact action. Avoid progress narration; repository progress and a truthful continuation point are the priority.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; source parent for this slice: `fef0bc61085a5e9364e972c00c97b5b0e3a8c59d`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only); PR #1 is CLOSED / UNMERGED.
- P1 HTTPD retained restore-upload protection retains prior L2/L3 evidence on `ee18adc7fe9062d2fe5284a3e64802242b89fad0`; physical router L4 remains NOT VERIFIED.
- DNSMasq three-file generated-input rollback and static-ARP command wait-status propagation were implemented and DNSMasq #43717087846 passed on `1f9105e975abb1fc5c66a2c07f16f858b304f80a`. This slice adds reverse rollback for prior successful static ARP additions after a later add fails.
- BusyBox udhcpc ARP-conflict deconfig retry is on main. The earlier #43717810267 checker failure was fixed by using the real `if (!arpping(packet.yiaddr,` branch anchor; exact run #43718099815 passed on `fef0bc61085a5e9364e972c00c97b5b0e3a8c59d`.
- WAN DHCP callback #43717201135 passed on `928f80c64683e9340b28ec769fd5266378a780c5`; other independent Actions continue to validate new source.
- No local target build or physical-router result is claimed. L4 remains NOT VERIFIED.


## Active task

### P2 — DNSMasq static-ARP transaction and remaining lifecycle

- DHCP hosts, `/etc/ethers` and `/tmp/hosts.static` are privately staged and published as one rollback set; static MACs are nonzero unicast; ARP add/delete wait statuses are checked and stop cleanup is fail-closed.
- Static-ARP installation now applies a fully prevalidated set. If a later `arp -s` fails, each earlier successful addition is removed in reverse order; rollback continues after an individual delete failure and returns the rollback error. The failed target is not deleted because in-tree BusyBox `arp_set()` returns success only after `SIOCSARP` succeeds.
- GNU89 tests cover success, reverse rollback, continued cleanup after delete failure, and invalid-set rejection before side effects. DNSMasq workflow contracts the production callback and BusyBox applet behavior.
- Remaining DNSMasq work: resolver/config publication concurrency, temp-file/FD cleanup, and remaining start/stop/restart caller graph. Then rotate to a broader independent user-space stability/security area.


## Explicitly uncompleted P1 work

- No outstanding P1 source change identified. HTTPD retained restore-upload protection has prior L2/L3 evidence on `ee18adc7fe9062d2fe5284a3e64802242b89fad0`; physical restore/retry/reboot L4 remains NOT VERIFIED.
- P2 DNSMasq: exact-head lifecycle and XY-C1 validation for the new ARP rollback transaction are pending; resolver concurrency, temp-file/FD cleanup and caller graph remain open.
- Linux 4.4 PID/start-time revalidate-to-signal TOCTOU remains; continue direct `pids()`, `kill_pidfile_s()` and name-wide signal audit without changing legacy semantics globally.
- FTP failed-upload unlink TOCTOU remains blocked by lack of inode-conditional unlink-by-fd; preserve STOR/APPE/REST/STOU semantics. NFS durability, AUTH_SYS/root-squash and external-mount compatibility require real client/storage evidence.

## Verification boundary

- DNSMasq previous implementation: #43717087846 PASS on `1f9105e975abb1fc5c66a2c07f16f858b304f80a`; not evidence for this rollback change.
- BusyBox udhcpc policy binary passed ASan/UBSan and corrected checker rerun #43718099815 passed on `fef0bc61085a5e9364e972c00c97b5b0e3a8c59d`.
- No local checkout/build or hardware device was available. Physical-router L4 remains NOT VERIFIED.


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

1. Inspect this commit's exact-head DNSMasq lifecycle Action; require all GNU89 tests and source assertions to pass, and diagnose any FAIL from full job logs.
2. Inspect this commit's XY-C1 run; require Build XY-C1, TRX verification and artifact upload before L3 claim.
3. Continue DNSMasq resolver/config concurrency, temp-file/FD cleanup and remaining caller-graph review; do not repeat the ARP transaction slice.
4. Rotate to an independent high-impact stability/security candidate outside DNSMasq and process inventory.
5. Re-fetch main before each write; use one complete Git tree/commit and a non-forced expected-SHA update. Frozen backup remains unchanged; physical L4 remains unverified.


Source/validation notes:
- Parent main for this slice: b39d77496885c65b1725dfc904788ec41f69f8fb; frozen backup: 72f4f157a084f8184b9597ca5bf334b8080e5807.
- Preserve docs/dev-log archived shards and docs/dev-log/INDEX.md byte-for-byte.
