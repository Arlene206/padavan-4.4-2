## Current repository state

- Maintained fork: Arlene206/padavan-4.4-2; do not substitute upstream.
- Base main SHA for THIS MAN DNS route conditional correction: `ca94b95812ba8eace9162279b268a54184e9f7bc`; active branch `main`; only `main` exists remotely and no checkpoint branch is present.
- The preceding WAN/MAN teardown status propagation implementation on `ca94b95812ba8eace9162279b268a54184e9f7bc` passed all five exact-head Actions: WAN route [42697380249], WAN interface teardown [42697380232], HTTPD [42697380274], WAN DHCP callback [42697380012], and AI docs [42697379978]. All jobs and steps completed SUCCESS.
- The preceding code aggregates MAN/WAN direct route and helper errors without short-circuiting, returns status through private helpers, preserves public void wrappers, and combines interface-clear / route-cleanup status in DHCP deconfig.
- L1 review of that source found a control-flow defect in the new MAN DNS status wrapper: the original DNS route condition had no braces, but the inserted status aggregation/logging statements did. As committed on `ca94b95812ba8eace9162279b268a54184e9f7bc`, those statements were outside the `if`, so a non-matching DNS entry could reuse a previous `route_status`. THIS corrective commit braces the full DNS route operation, status aggregation and error log as one conditional unit; the route-status workflow asserts that block structure.
- Earlier MAN DHCP route-leak fix remains: preserve `wanx_ipaddr` through off-subnet gateway host-route removal; invalidate afterward and before resolver/firewall refresh. Interface-clear failures are logged and combined with route cleanup status.
- BusyBox still discards `spawn_and_wait()` callback status; leasefail/NAK/SIGUSR2 semantics remain as previously traced. Per-operation and summary logs are the observable failure path.
- Next code audit after exact-head verification: route-add failures in `add_man_gateway_routes()`, `wan_up()`, and `man_up()`; then rotate governance coverage beyond WAN to the wider P2 queue.
- Root `dev_log.md` will contain 76 /119 complete newest-first records after THIS entry; no archive rotation is due.
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

1. Reconcile THIS MAN DNS control-flow correction and all prior WAN/MAN status changes on its exact resulting main SHA. Inspect WAN route status, WAN DHCP callback, WAN interface teardown, HTTPD security and AI handoff docs Actions.
2. If all are green, inspect ignored route-add results in `add_man_gateway_routes()`, `wan_up()` and `man_up()`; design bounded rollback/reporting that does not publish inconsistent route/lease state.
3. Then complete the bounded BusyBox leasefail/NAK/renew/SIGUSR2 callback review without changing ignored script-exit semantics absent a demonstrated defect.
4. Rotate broader repository coverage to Samba same-boot publication and other P2 items; keep NFS compatibility, FULLCONENAT target build and FTP pathname TOCTOU visible.
## Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for MAN off-subnet gateway route identity/order, accumulated MAN/WAN route-delete status, public API preservation, combined DHCP cleanup status, and THIS DNS condition brace correction. |
| L2 host / Actions | Parent SHA `ca94b95812ba8eace9162279b268a54184e9f7bc`: WAN route [42697380249], WAN teardown [42697380232], HTTPD [42697380274], WAN DHCP callback [42697380012], AI docs [42697379978] all SUCCESS. THIS corrective commit's route/DHCP/teardown/HTTPD/docs Actions are PENDING. |
| L3 production RC object, MIPS/uClibc kernel object, full firmware/TRX | NOT VERIFIED. |
| L4 target route reconciliation and DHCP failure/release tests | NOT VERIFIED. |
## Exact first action for the next GPT

1. Fetch live `main`, branch/checkpoint list and recent Actions; inspect the parent `ca94b95812ba8eace9162279b268a54184e9f7bc` run set and THIS correction's exact-head run set. Require the latter to pass all applicable jobs and inspect logs for the DNS condition contract.
2. If green, audit route-add error handling in MAN/WAN up paths with a focused host/source regression; do not undo the existing teardown fixes.
3. Then move on from WAN to the next highest-impact stable P2 governance issue elsewhere in the userland.
## Continuation rules

- `SESSION_HANDOFF.md` is the sole actionable continuation point; update only when resumable work/checkpoint/next action materially changes.
- Update `REFACTOR_ROADMAP.md` only when module status/evidence/priorities materially change.
- Root `dev_log.md` keeps 20–119 complete newest-first records; at 120, archive the oldest 100 verbatim and retain newest 20. Use 384 KiB as early-rotation target and 512 KiB as hard limit.
- Preserve archive snapshots/indexes byte-for-byte; do not add routine archive snapshots or docs-only commits.
- Run `scripts/check_ai_handoff_docs.py` only with a full local checkout; exact-head Actions are authoritative for the pushed SHA.
- Recheck live `main`, branches and exact-head Actions on every takeover. Never force-push. Do not claim target builds or device tests without evidence.
