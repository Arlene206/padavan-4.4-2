# Cross-session execution mandate (read before resuming)

This is a continuing autonomous engineering assignment. The next GPT must recover live state once, then immediately resume the exact P1 task below and implement/test/commit—not repeat this session's documentation work, re-audit already completed evidence, or ask the user to choose a task. Do not stop after a plan or status report when safe source work is available. Keep working through bounded engineering slices while context/time permits.

**Mandatory handoff checkpoint:** Every session must verify before ending that this file accurately records the meaningful source baseline, active task/progress, relevant exact-head Actions, validation boundary, blockers and exact first action. If any next-session-critical fact is missing, stale or changed, update and commit one consolidated correction (prefer batching with source work). If everything is accurate, leave this file untouched and make no no-op documentation commit. The checkpoint is mandatory; a document edit is conditional.

For every multi-file slice, publish source + regression tests + required workflow/build integration as **one atomic main commit**: create blobs, one tree, one commit parented to the freshly fetched main SHA, then non-forced update of main with that same SHA as expected_sha. Never create partial per-file commits for one logical change. If main moved, rebase/reapply and retest; never force-push. Verify exact-head push Actions. Before context exhaustion, finish a safe slice and leave this handoff materially updated with committed progress, validation and the next exact action. Avoid progress narration; repository progress and a truthful continuation point are the priority.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; live main at takeover for this slice: `e875f25d55a1e3523a1912368c5dd407b593946a`. The active source baseline now includes DNSMasq/VPN lock-order fixes, IPv6 DNSMasq lifecycle serialization (`8a909474`), optional daemon start-status propagation (`55947ba`), and TTYD boot failure logging (`30c5b48`). Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only). PR #1 is CLOSED / UNMERGED; comparison shows no PR-only commits ahead of main.
- DNSMasq static-ARP rollback transaction source landed in `d5660e3cdfa9804a23335656676c5f598c19a37e`. Its initial exact-head #43718997744 exposed a test-only multi-character NUL literal; `23cc4e9e509f13a4ea7636c067d6cfd869fde81b` corrected the literal. Exact-head DNSMasq #43719124911 now PASS; AI docs #43719124917 and process inventory #43719124976 PASS.
- XY-C1 run #43718997699 for product source `d5660e3` was still IN_PROGRESS at checkpoint; the source code is unchanged by `23cc4e9e`, but L3 must not be claimed until Build XY-C1, TRX verification and artifact upload all pass.
- BusyBox udhcpc ARP-conflict deconfig retry is on main; exact checker #43718099815 PASS on `fef0bc61085a5e9364e972c00c97b5b0e3a8c59d`.
- P1 HTTPD restore-upload protection retains prior L2/L3 evidence on `ee18adc7fe9062d2fe5284a3e64802242b89fad0`; physical router L4 remains NOT VERIFIED.
- No local build or physical-router result is claimed.


## Active task

### P2 — VPN/DNSMasq lock ordering; align VPN lifecycle regression contract

- Production lock fixes are committed in `18c5f948efe89d9c8bcbff9caa14708686b2340d`: WAN stop callbacks now call `stop_vpn_client_under_lifecycle_lock()` rather than reacquiring the process-associated `F_SETLK` lock; LAN restart performs VPN preflight before taking the DNSMasq lock, restoring VPN if DNS lock acquisition fails.
- DNSMasq exact-head action #43724605898 PASS, including the staged-file sanitizer tests and source-order contracts; WAN route #43724605834 and AI handoff #43724605883 PASS. XY-C1 #43724605847 was still building when checked.
- VPN server lifecycle #43724605659 FAILED only because its legacy source checker still required the public `stop_vpn_client()` call inside the VPN-lock-owning WAN callbacks. The correct new under-lock helper is now reflected in `scripts/check_vpn_server_lifecycle.py`, including assertions that it does not acquire/release the outer lock.
- Exact-head VPN lifecycle/AI/XY-C1 reruns are required after this checker update. Continue checking for any additional DNSMasq->VPN or same-process nested lock paths.


## Explicitly uncompleted P1 work

- No outstanding P1 source change identified. HTTPD retained restore-upload protection has prior L2/L3 evidence on `ee18adc7fe9062d2fe5284a3e64802242b89fad0`; physical restore/retry/reboot L4 remains NOT VERIFIED.
- P2 DNSMasq: static-ARP transaction, unsafe-upstream refresh error propagation, retryable stage/backup cleanup, manual LAN resolver atomic publication, VPN/DNS lock-order repair and IPv6 restart serialization are implemented. DNSMasq regression #43726863022 passed on `55947ba869421a8108d24898abb761ba58bb81a7`; the later RC start-status source changes do not modify DNSMasq code. Resolver/config concurrency and the remaining complete caller graph remain open.
- Linux 4.4 PID/start-time revalidate-to-signal TOCTOU remains; continue direct `pids()`, `kill_pidfile_s()` and name-wide signal audit without changing legacy semantics globally.
- FTP failed-upload unlink TOCTOU remains blocked by lack of inode-conditional unlink-by-fd; preserve STOR/APPE/REST/STOU semantics. NFS durability, AUTH_SYS/root-squash and external-mount compatibility require real client/storage evidence.

## Verification boundary

- At source SHA `30c5b48ba8a8e996d13548280a54a9b7f5dbda7a`, AI handoff #43727223494, VPN server #43727223521, RC queue #43727223568 and ZeroTier #43727223519 PASS; storage's stale ZeroTier checker assertion was fixed by the follow-up checker-only commit. Exact checker SHA `e875f25d55a1e3523a1912368c5dd407b593946a`: storage #43728578619 and AI handoff #43728578578 PASS. XY-C1 #43727223434 completed PASS with Build XY-C1, TRX verification and artifact upload; artifact #14166909490 (`XY-C1-firmware-30c5b48ba8a8e996d13548280a54a9b7f5dbda7a`, 12,602,685 bytes). This L3 evidence applies to product source `30c5b48`, not to the new `services.c` change being committed now.
- The checker repair changes only the source contract and handoff records. Fresh VPN server lifecycle and AI handoff Actions are required; the already running XY-C1 run still covers the production-source SHA `18c5f948efe89d9c8bcbff9caa14708686b2340d`.
- No current-source MIPS/TRX completion or physical-router result is claimed; L4 remains NOT VERIFIED.


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

1. Confirm this commit's exact-head VPN server lifecycle and AI docs Actions; the new source contract asserts `start_services_once()` logs a failed `start_vpn_server()` result. Confirm XY-C1 is triggered for the changed `services.c` and require Build, TRX verification and artifact upload before claiming the new source SHA at L3.
2. DNSMasq direct caller/lock-order review is complete: only `services_ex.c` public wrappers, `net_lan.c`/`net6.c` caller-held-lock paths, boot `rc.c`, watchdog recovery and shutdown `services.c` call the lifecycle APIs; `net_wan.c` releases the resolver publication lock before invoking `restart_dns()`. No additional same-process nested DNSMasq F_SETLK release or reverse VPN/DNS lock edge was found. Do not repeat this audit. Remaining known limitation: Linux 4.4 PID/start_time validation-to-kill TOCTOU; do not claim it is solved by an extra recheck.
3. After exact-head checks, rotate to the P1 process-lifecycle ledger: investigate the remaining direct `pids()` / `kill_pidfile_s()` users and direct name-wide signal sites, without re-auditing already migrated services. Prefer a concrete high-impact caller and add a focused host regression/source contract.
4. Keep frozen backup `backup/pre-plugin-overhaul-2026-11-14` unchanged and physical-router L4 NOT VERIFIED.


Source/validation notes:
- Parent main for this slice: d2784f00b3ffd5e1988a5dbf780f12061e4b93a4; frozen backup: 72f4f157a084f8184b9597ca5bf334b8080e5807.
- Preserve docs/dev-log archived shards and docs/dev-log/INDEX.md byte-for-byte.
