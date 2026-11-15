# Cross-session execution mandate (read before resuming)

This is a continuing autonomous engineering assignment. The next GPT must recover live state once, then immediately resume the exact P1 task below and implement/test/commit—not repeat this session's documentation work, re-audit already completed evidence, or ask the user to choose a task. Do not stop after a plan or status report when safe source work is available. Keep working through bounded engineering slices while context/time permits.

**Mandatory handoff checkpoint:** Every session must verify before ending that this file accurately records the meaningful source baseline, active task/progress, relevant exact-head Actions, validation boundary, blockers and exact first action. If any next-session-critical fact is missing, stale or changed, update and commit one consolidated correction (prefer batching with source work). If everything is accurate, leave this file untouched and make no no-op documentation commit. The checkpoint is mandatory; a document edit is conditional.

For every multi-file slice, publish source + regression tests + required workflow/build integration as **one atomic main commit**: create blobs, one tree, one commit parented to the freshly fetched main SHA, then non-forced update of main with that same SHA as expected_sha. Never create partial per-file commits for one logical change. If main moved, rebase/reapply and retest; never force-push. Verify exact-head push Actions. Before context exhaustion, finish a safe slice and leave this handoff materially updated with committed progress, validation and the next exact action. Avoid progress narration; repository progress and a truthful continuation point are the priority.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; source parent for this staging-cleanup slice: `39a67af8d061f6f3eceeb2efbea7bd9a4c7cfe10`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only); PR #1 is CLOSED / UNMERGED.
- DNSMasq static-ARP rollback transaction source landed in `d5660e3cdfa9804a23335656676c5f598c19a37e`. Its initial exact-head #43718997744 exposed a test-only multi-character NUL literal; `23cc4e9e509f13a4ea7636c067d6cfd869fde81b` corrected the literal. Exact-head DNSMasq #43719124911 now PASS; AI docs #43719124917 and process inventory #43719124976 PASS.
- XY-C1 run #43718997699 for product source `d5660e3` was still IN_PROGRESS at checkpoint; the source code is unchanged by `23cc4e9e`, but L3 must not be claimed until Build XY-C1, TRX verification and artifact upload all pass.
- BusyBox udhcpc ARP-conflict deconfig retry is on main; exact checker #43718099815 PASS on `fef0bc61085a5e9364e972c00c97b5b0e3a8c59d`.
- P1 HTTPD restore-upload protection retains prior L2/L3 evidence on `ee18adc7fe9062d2fe5284a3e64802242b89fad0`; physical router L4 remains NOT VERIFIED.
- No local build or physical-router result is claimed.


## Active task

### P2 — DNSMasq staging cleanup and remaining lifecycle

- DHCP hosts, `/etc/ethers` and `/tmp/hosts.static` publish as a rollback set; static MAC/domain/pool validation is strict; static-ARP additions are prevalidated and unwound in reverse after a later add failure.
- Unsafe-upstream resolver partial-success is fixed: when server snapshot generation fails and no safe last-known-good file exists, `update_resolvconf()` propagates the validation errno after publishing resolver configuration and releasing its lock, before any DNSMasq cache notification.
- New cleanup defect: `dnsmasq_file_discard()` previously cleared its staging pathname even when `unlink()` failed. It now returns status, preserves the path and errno on failure, treats ENOENT as already-clean, and allows caller cleanup to retry/report orphan paths.
- Added an injected-unlink regression for EACCES retention, retry success, ENOENT and invalid callback. DNSMasq workflow path/source contract and XY-C1 test path trigger updated. Exact-head DNSMasq, AI handoff and XY-C1 checks are required for this commit.
- Remaining DNSMasq work: backup snapshot/discard error reporting, resolver/config publication concurrency and the public/locked start/stop/restart caller graph. Then rotate to broader whole-tree reliability/security work.


## Explicitly uncompleted P1 work

- No outstanding P1 source change identified. HTTPD retained restore-upload protection has prior L2/L3 evidence on `ee18adc7fe9062d2fe5284a3e64802242b89fad0`; physical restore/retry/reboot L4 remains NOT VERIFIED.
- P2 DNSMasq: static-ARP transaction L2 is verified. This slice adds unsafe-upstream refresh error propagation; exact-head regression and XY-C1 product build/TRX/artifact are required. Resolver concurrency, temp-file/FD cleanup and remaining caller-graph review remain open.
- Linux 4.4 PID/start-time revalidate-to-signal TOCTOU remains; continue direct `pids()`, `kill_pidfile_s()` and name-wide signal audit without changing legacy semantics globally.
- FTP failed-upload unlink TOCTOU remains blocked by lack of inode-conditional unlink-by-fd; preserve STOR/APPE/REST/STOU semantics. NFS durability, AUTH_SYS/root-squash and external-mount compatibility require real client/storage evidence.

## Verification boundary

- Resolver-status source commit `39a67af8d061f6f3eceeb2efbea7bd9a4c7cfe10`: DNSMasq #43719693642 PASS; WAN route #43719693710 PASS; AI handoff #43719693546 PASS; process inventory #43719693606 PASS; HTTPD #43719693685 PASS; NVRAM #43719693596 PASS. XY-C1 #43719693623 remained IN_PROGRESS at last poll.
- The isolated GNU89 resolver policy regression passed with ASan/UBSan; this is host-only evidence, not target validation.
- Previous product source `3fa8d08b968de653a7998175bdab897e51b91379`: DNSMasq #43719236286, WAN route #43719236311, AI docs #43719236448 and process inventory #43719236422 PASS; XY-C1 #43719236268 remained IN_PROGRESS at last poll.
- Exact-head DNSMasq, AI handoff and XY-C1 checks for this staging-cleanup commit are required. No current-source MIPS/TRX or physical-router result is claimed; L4 remains NOT VERIFIED.


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

1. Check exact-head DNSMasq lifecycle and AI handoff for the staging-discard change; diagnose any failure from full logs and require the injected unlink-failure regression to pass.
2. Check exact-head XY-C1; require Build XY-C1, TRX verification and artifact upload before any L3 claim.
3. Continue DNSMasq backup snapshot/discard error reporting, resolver/config concurrency and public/locked caller graph; do not redo static-ARP or unsafe-upstream status work.
4. After DNSMasq, rotate to an unblocked high-impact reliability/security area elsewhere in user space. FTP inode-conditional unlink remains blocked by Linux 4.4 semantics; NFS durability/auth compatibility needs real client evidence.
5. Keep frozen backup unchanged and physical router L4 marked NOT VERIFIED.


Source/validation notes:
- Parent main for this slice: 39a67af8d061f6f3eceeb2efbea7bd9a4c7cfe10; frozen backup: 72f4f157a084f8184b9597ca5bf334b8080e5807.
- Preserve docs/dev-log archived shards and docs/dev-log/INDEX.md byte-for-byte.
