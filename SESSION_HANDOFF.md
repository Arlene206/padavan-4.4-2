# Cross-session execution mandate (read before resuming)

This is a continuing autonomous engineering assignment. The next GPT must recover live state once, then immediately resume the exact P1 task below and implement/test/commit—not repeat this session's documentation work, re-audit already completed evidence, or ask the user to choose a task. Do not stop after a plan or status report when safe source work is available. Keep working through bounded engineering slices while context/time permits.

**Mandatory handoff checkpoint:** Every session must verify before ending that this file accurately records the meaningful source baseline, active task/progress, relevant exact-head Actions, validation boundary, blockers and exact first action. If any next-session-critical fact is missing, stale or changed, update and commit one consolidated correction (prefer batching with source work). If everything is accurate, leave this file untouched and make no no-op documentation commit. The checkpoint is mandatory; a document edit is conditional.

For every multi-file slice, publish source + regression tests + required workflow/build integration as **one atomic main commit**: create blobs, one tree, one commit parented to the freshly fetched main SHA, then non-forced update of main with that same SHA as expected_sha. Never create partial per-file commits for one logical change. If main moved, rebase/reapply and retest; never force-push. Verify exact-head push Actions. Before context exhaustion, finish a safe slice and leave this handoff materially updated with committed progress, validation and the next exact action. Avoid progress narration; repository progress and a truthful continuation point are the priority.

## Current repository state

- Repository: Arlene206/padavan-4.4-2. Main parent for this slice: 34ec88e12393d8c8645ad25674ccf44aacbe1256. Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807 (read-only); PR #1 remains CLOSED / UNMERGED.
- P1 HTTPD retained restore-upload protection is L2/L3 verified on product SHA ee18adc7fe9062d2fe5284a3e64802242b89fad0: HTTPD #43701369173, AI docs #43701369189 and XY-C1 #43701369143 PASS through firmware build, TRX verification and artifact upload. Physical router L4 remains unverified.
- P2 DNSMasq on main includes SIGHUP ESRCH re-inventory, staged DHCP hosts/ethers/hosts.static, fail-closed preparation, static MAC validation, LAN domain validation, WAN/AP hostname publication, AP restart gating, IPv6 tri-state restart, and boot/watchdog start failure reporting. Latest functional commits: bc2b1193, 393bb762, 64e5714d, 9e9bac08, 514572c3, 55ff022f, f188bacf, d74f61b1, 37350a66, c3227528, e749890c, 53cf5fbe, 34ec88e1.
- Exact-head Actions on 34ec88e1: DNSMasq #43705925858 PASS; HTTPD #43705926049 PASS; AI handoff #43705925896 PASS; Process inventory #43705926093 PASS; Samba #43705925862 and Inadyn #43705925881 PASS. XY-C1 #43705925871 and NVRAM audit #43705925906 remain IN_PROGRESS at last poll.
- This slice records IPv6 stop failure and closes DNSMasq helper/test and XY firmware helper-header trigger gaps. Frozen backup remains unchanged. No physical-router result is claimed.


## Active task

### P2 — DNSMasq lifecycle and generated-host consistency

- restart_dns() re-inventories after targeted SIGHUP ESRCH; only a fresh complete ABSENT scan turns the race into a no-op. UNKNOWN and a replacement daemon remain errors.
- fill_static_ethers() stages DHCP hosts, /etc/ethers and /tmp/hosts.static before individual atomic renames; preparation refuses startup on failures, ARP changes follow publication. The three renames are not a cross-file transaction.
- Static DHCP MAC fields now require exactly 12 hexadecimal characters. LAN domain is validated before any generated host/ARP data changes and before raw config emission. Regression checks cover directive injection and invalid DNS labels.
- WAN/AP hostname+hosts are staged, lease IP/domain validated, and AP DHCP restart waits for successful publication; status propagates to PID 1. Pair rollback remains an audit item.
- IPv6 full_restart_ipv6() uses tri-state dnsmasq_process_status(): only ABSENT starts, RUNNING skips, UNKNOWN/invalid refuses duplicate start. This slice captures stop_dns_dhcpd() status and logs failure before the fresh process-state decision.
- On 34ec88e1 exact head, DNSMasq #43705925858, HTTPD #43705926049 and AI docs #43705925896 PASS; XY-C1 #43705925871 and NVRAM #43705925906 are IN_PROGRESS.
- Workflow gap: DNSMasq runs tests/dnsmasq_file.c and consumes dnsmasq_file.h but the path filters omitted them; this commit adds both to push/PR filters and path contract. XY-C1 now tracks dnsmasq_file.h and dnsmasq_domain_policy.h to trigger MIPS rebuilds for header-only dependency edits.


## Explicitly uncompleted P1 work

- No outstanding P1 source fix is currently identified. HTTPD retained restore-upload preservation is L2/L3 verified on ee18adc7fe9062d2fe5284a3e64802242b89fad0; physical restore/retry/reboot L4 remains NOT VERIFIED.
- P2: three generated DHCP/ethers/hosts.static renames are individually atomic only. Audit rollback/recovery if rename 2 or 3 fails after an earlier publish; inspect ARP command return values and temp/FD cleanup.
- P2: WAN/AP hostname+hosts two-file publication likewise needs recovery if the second rename fails.
- P2: continue resolver concurrency and daemon-config rollback review; Linux 4.4 PID/start-time revalidate-to-signal TOCTOU remains.
- Continue remaining kill_pidfile_s() and name-wide signal audits. FTP failed-upload cleanup TOCTOU remains blocked by lack of inode-conditional unlink-by-fd; preserve STOR/APPE/REST/STOU.
- NFS async durability, AUTH_SYS/root-squash and external-mount compatibility need real client/storage evidence.


## Verification boundary

- HTTPD restore-upload L2/L3: HTTPD #43701369173, AI docs #43701369189 and XY-C1 #43701369143 PASS on product SHA ee18adc7fe9062d2fe5284a3e64802242b89fad0, including build/TRX/artifact. Physical router L4 remains NOT VERIFIED.
- Exact-head 34ec88e1: DNSMasq #43705925858 PASS; HTTPD #43705926049 PASS; AI docs #43705925896 PASS; Process inventory #43705926093 PASS. XY-C1 #43705925871 and NVRAM #43705925906 remain IN_PROGRESS at last poll.
- No local checkout/build or hardware device was available. No local compilation or physical-device test is claimed.


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

1. Recheck exact-head XY-C1 #43705925871 and NVRAM #43705925906 on 34ec88e1. Require Build XY-C1, TRX verification and artifact upload before marking current L3.
2. Verify the exact-head DNSMasq/HTTPD/AI runs for this commit; fix source or workflow regressions, don't just annotate them.
3. Continue the highest-risk DNSMasq generated-input issue: stage and recover DHCP-host, /etc/ethers and hosts.static as a coherent generation if a later rename fails. Check ARP command status and all temp/FD cleanup.
4. Audit two-file hostname/hosts publication rollback and remaining caller/signal/concurrency issues.
5. Re-fetch main before each write; use one complete Git tree and commit, then non-forced update with the same expected SHA. Keep backup immutable and physical L4 unverified.

Source/validation notes:
- Parent main for this slice: 34ec88e12393d8c8645ad25674ccf44aacbe1256; frozen backup: 72f4f157a084f8184b9597ca5bf334b8080e5807.
- Preserve archived docs/dev-log shards and docs/dev-log/INDEX.md byte-for-byte.
