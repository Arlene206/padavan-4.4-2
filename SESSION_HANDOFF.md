## Current repository state

- Maintained fork: Arlene206/padavan-4.4-2; work only on this fork.
- Live baseline for THIS change: `639ebbdedca75d1938e7d280108d146d05e2e479`; active branch `main`. Remote branch enumeration contains only `main`; no checkpoint branch exists.
- Exact-head recovery for the completed MAN DNS conditional correction on `639ebbdedca75d1938e7d280108d146d05e2e479`: WAN interface teardown [42698031149], AI handoff docs [42698030922], WAN route status [42698030798], HTTPD security [42698030765], and WAN DHCP callback [42698030655] all completed SUCCESS with matching full `head_sha`. Relevant compile, sanitizer, semantic, and documentation steps all passed.
- The DNS teardown predicate correction is complete: route deletion, error accumulation and logging are enclosed by the full eligibility condition; the route workflow asserts this structure.
- Active code task for THIS commit: propagate route-add failures through `add_man_gateway_routes()`, `man_up()`, and `wan_up()`. The direct MAN/WAN gateway host/default/DNS additions and the existing static/DHCP helper results were previously discarded by their callers.
- Implementation shape: private int-returning `man_up_internal()` / `wan_up_internal()` and `add_man_gateway_routes()`; retain the existing public void APIs as logging wrappers. `udhcpc_bound()` calls the private helpers, logs accepted leases with incomplete route setup, and returns the aggregate route status. All route attempts and subsequent service startup continue after the first route failure.
- Do not blindly roll back successful additions after a later failure: the kernel route operation cannot identify whether an identical route pre-existed or was created by this pass. Preserve the accepted DHCP lease and report the first failure; existing scoped teardown remains responsible for route cleanup.
- BusyBox `udhcp_run_script()` still discards callback status; operational logs remain essential. L3 production RC/MIPS/uClibc/firmware and L4 device validation are not established.
- Root `dev_log.md` contains 76 complete records before THIS commit; the new record makes 77 /119. No archive rotation is due.

## Completed FULLCONENAT namespace / zone work

- IPv4/IPv6 mapping entries and pending dying-tuple records retain the owning struct net pointer plus conntrack zone identity (id, flags and direction).
- Mapping lookup and tuple garbage collection are scoped by the full identity. Conntrack lookups reconstruct the saved zone; final per-net target teardown unregisters the notifier, waits for legacy RCU callback readers, purges only that namespace's mapping/queue entries, then drops the conntrack namespace reference.
- The GNU89 host policy regression covers same/different namespace, zone ID, flags, direction and null inputs. It passed exact-head runs [42674430844], [42677259891], [42677876443] and [42678293858]. The notifier/cleanup lifecycle contract also passed all four runs.
- Three IPv4 helper call sites now pass &context; five identical version-conditional declaration wrappers were removed so the semantic function-body extractor can parse the refactored functions.
- The exact-head semantic contract progressed through mapping structures, namespace/zone lookup filters, IPv4/IPv6 lookup paths and conditional notifier callback signatures. FULLCONENAT lifecycle regression [42678875089] and AI handoff docs [42678875391] passed on exact SHA `bc99b6801c44f55e13dc0214336408f712e0abf8`; subsequent WAN DHCP work did not modify the kernel code.
- Kernel translation-unit compilation remains unverified; the host policy test and semantic source contract do not replace a target kernel object build.

## Completed FTP work

- Existing RNFR/RNTO fix: absolute source-path pinning plus source-delete/destination-write ACL revalidation against canonical parents.
- Path-aware checks now cover STOR/APPE/STOU, RETR, MKD/RMD/DELE, LIST/NLST/STAT, SIZE, MDTM and SITE CHMOD. Upload/RETR/metadata operations authorize the effective existing symlink target; MKD/RMD/DELE authorize the entry parent; listings authorize the actual opened directory before transfer setup.
- Added fail-closed handling for dangling final upload symlinks and canonical resolution of symlinked parent directories.
- Sidecar disclosure finding: `trunk/user/libdisk/src/disk_share.c` creates `.__folder_list.txt` and `.__<account>_var.txt` under the USB mount root. Per-share READ ACLs do not govern MOUNT_LAYER, so direct FTP file commands could expose folder/account ACL metadata. Exact-name filtering now hides and rejects these reserved files before the chroot ACL bypass.
- Both existing-file and upload path resolvers also classify the canonical `realpath()` target. Thus a harmless-looking final symlink alias to an ACL sidecar is denied as well as the direct filename. Ordinary `.__notes.txt` user files do not match the classifier.
- GNU89 regression now covers positive/negative metadata filenames, direct sidecar denial, final symlink alias denial, cross-share symlinks, dangling links, new leaves and symlinked directories.
- The only failure on `30d64c73546db236c12679d24eda74c1ea9f2916` is the CI Python variable-order defect described above; host binaries themselves all passed. The workflow correction moves all helper-body extraction before assertions.

## Active task

1. Reconcile the new route-add status propagation commit on its exact live `main` SHA. Inspect WAN route status, WAN DHCP callback, HTTPD security, and AI handoff Actions; verify every run's `head_sha` matches exactly.
2. Then complete the bounded BusyBox DHCP callback state audit: initial `leasefail`, accepted NAK followed by deconfig, lease expiry, renew/repeated renew, and SIGUSR2/release-on-quit. Trace how `udhcp_run_script()` discards child status and do not change that legacy behavior without a demonstrated state defect.
3. Move the next code round outside WAN into the highest-value remaining userland stability item, starting with Samba same-boot passdb/config publication and continuing the repository-wide priority queue. Keep FTP realpath-to-syscall TOCTOU, FULLCONENAT target build, and NFS compatibility questions visible.
4. Preserve the L3 full target build and L4 router tests as later consolidated project milestones; do not claim them from host/Actions results.

## Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for MAN gateway host/default/DNS route-add status accumulation, WAN gateway host/default and static/DHCP route helper propagation, public void API preservation, continued cleanup/service setup, and DHCP callback route-status reporting. |
| L2 host / Actions | Base SHA `639ebbdedca75d1938e7d280108d146d05e2e479`: WAN teardown [42698031149], AI docs [42698030922], WAN route status [42698030798], HTTPD [42698030765], and WAN DHCP callback [42698030655] all SUCCESS. THIS route-add change's focused GNU89 sanitizer and semantic source-contract Action is PENDING on its resulting SHA. |
| L3 production RC object, MIPS/uClibc objects, full firmware/TRX | NOT VERIFIED. |
| L4 router route reconciliation, DHCP failure/release behavior, and hardware | NOT VERIFIED. |

## Exact first action for the next GPT

1. Fetch live `main`, enumerate branches, and inspect Actions. Require the WAN route operation status workflow and all other triggered workflows to match the exact resulting SHA and pass; inspect failed job logs if any.
2. After exact-head green, inspect BusyBox DHCP callback transitions at the concrete `udhcp_run_script()` call sites for `leasefail`, NAK/deconfig, expiry, renew, and SIGUSR2/release-on-quit. Determine whether callback status discard creates a reproducible state/lifecycle defect before proposing a behavior change.
3. When that review is closed, rotate outside WAN: begin Samba same-boot passdb/config publication review, then the next highest-risk item in the whole-userland ledger. Keep FTP path TOCTOU, FULLCONENAT target compile, and NFS compatibility decisions explicit.

## Continuation rules

- `SESSION_HANDOFF.md` is the sole actionable continuation point; update only when resumable work/checkpoint/next action materially changes.
- Update `REFACTOR_ROADMAP.md` only when module status/evidence/priorities materially change.
- Root `dev_log.md` keeps 20–119 complete newest-first records; at 120, archive the oldest 100 verbatim and retain newest 20. Use 384 KiB as early-rotation target and 512 KiB as hard limit.
- Preserve archive snapshots/indexes byte-for-byte; do not add routine archive snapshots or docs-only commits.
- Run `scripts/check_ai_handoff_docs.py` only with a full local checkout; exact-head Actions are authoritative for the pushed SHA.
- Recheck live `main`, branches and exact-head Actions on every takeover. Never force-push. Do not claim target builds or device tests without evidence.
