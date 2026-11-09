## Current repository state

- Maintained fork: Arlene206/padavan-4.4-2; do not substitute upstream.
- Base main SHA for THIS MAN DHCP deconfig cleanup fix: `bf80204bf59e26f74636e9c75ce52c6d5d652bee`; active branch: `main`. The live branch inventory contains only `main`; no remote checkpoint branch exists.
- The previous first-lease rollback-failure regression and AI handoff documentation checks both completed SUCCESS on exact SHA `bf80204bf59e26f74636e9c75ce52c6d5d652bee`: WAN DHCP callback [42690696672] and AI handoff docs [42690696645]. The jobs checked out this exact full SHA.
- WAN DHCP run [42690696672] compiled and ran the GNU89 interface rollback regression, then passed the source-order, rollback and complete push/pull_request path-filter assertions. AI docs run [42690696645] passed `scripts/check_ai_handoff_docs.py` (root log 72/119; archive inventories valid).
- THIS batch fixes a MAN DHCP deconfig route leak: `man_up()` adds a host route for an off-subnet gateway based on `wanx_ipaddr` and `wanx_netmask`, but `udhcpc_deconfig()` previously replaced `wanx_ipaddr` with `0.0.0.0` before calling `man_down()`. Consequently, `man_down()` could no longer recognize/delete that gateway host route. The new internal teardown keeps the old address until the route-removal phase completes, then clears it before resolver/firewall refresh; the public `man_down()` wrapper preserves its existing behavior for other callers.
- Deconfig now captures the interface-clear status, logs failures, continues best-effort route/state cleanup, and returns that status. BusyBox's bundled `udhcp_run_script()` currently ignores `spawn_and_wait()`'s status, so this makes the failure observable but does not change BusyBox's internal state transition.
- Callback audit: `leasefail` currently does no state mutation; NAK logs the event and BusyBox invokes deconfig for an already-bound lease (but not initial REQUESTING); SIGUSR2 / release-on-quit use the deconfig callback, not a distinct release script. BusyBox's `spawn_and_wait()` result is discarded. Continue to treat those semantics deliberately rather than changing BusyBox callback behavior without a demonstrated requirement.
- Still open in WAN teardown: `man_down()` and `wan_down()` discard the results from gateway/DHCP/static route deletions even though the underlying helpers return status. Audit retry/reconciliation and state-publication ordering before changing that behavior.
- NFS custom-export provenance review found no untrusted/local-account writer. `/etc` is tmpfs; `/etc/storage` is root-managed persistent MTD content; web restore requires authenticated privileged access. The RC writer accepts regular files or symlinks to regular files, falls back only on ENOENT, and fails closed otherwise. `nfsd.sh` propagates `exportfs -r` failure. No NFS code change was justified; async durability, AUTH_SYS/no_root_squash and external mounts remain separate compatibility decisions.
- Root `dev_log.md` will contain 73 /119 complete newest-first records after THIS entry; no archive rotation is due.

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

1. Reconcile THIS MAN DHCP deconfig cleanup regression and AI handoff documentation workflow against the exact resulting main SHA. Inspect every step and require both checks to pass on the same SHA.
2. If green, audit `man_down()` / `wan_down()` route-delete and route-add result propagation. The underlying DHCP/static route helpers return status, but teardown currently discards it before clearing lease state; design best-effort cleanup and state reconciliation without leaving stale routes unreported.
3. Finish the bounded WAN callback review: confirm BusyBox's ignored script status, initial-discovery leasefail, NAK/deconfig, lease expiry, repeated renew and SIGUSR2 release semantics. Preserve the MAN off-subnet route fix and existing bound rollback behavior.
4. Keep NFS async durability/AUTH_SYS/root-squash/external mounts, Samba same-boot passdb/config recovery, FULLCONENAT target build, and FTP realpath-to-syscall TOCTOU visible in the wider whole-userland queue.

## Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for the MAN off-subnet gateway route lifecycle diagnosis and bounded deconfig ordering change: the prior lease address now remains available through the old gateway host-route deletion; failed interface clear is logged/returned while cleanup continues. Previous WAN bound transaction/rollback and NFS custom-export trust-boundary reviews remain PASS. |
| L2 host / Actions | Exact SHA `bf80204bf59e26f74636e9c75ce52c6d5d652bee`: WAN DHCP callback [42690696672] and AI handoff docs [42690696645] SUCCESS; job logs confirm matching full checkout SHA. THIS MAN deconfig source-contract extension and documentation update are PENDING exact resulting HEAD. |
| L3 production RC object, MIPS/uClibc kernel object, full firmware/TRX | NOT VERIFIED. |
| L4 target router DHCP release/NAK/lease-expiry, failed ioctl and route reconciliation behavior | NOT VERIFIED. |

## Exact first action for the next GPT

1. Fetch live `main`, branch/checkpoint inventory and recent Actions; verify the exact resulting SHA for THIS MAN DHCP deconfig change.
2. Inspect the WAN DHCP callback regression and AI handoff docs Actions for that exact SHA. Confirm GNU89 compile/run and every semantic/docs step; do not count the base SHA runs as validation of THIS patch.
3. Then trace the return values from `route_del()`, `del_dhcp_routes_by_prefix()` and `del_static_man_routes()` through `man_down()` / `wan_down()`; decide how to report accumulated errors while still attempting all required teardown. Preserve old lease inputs until all matching old routes have been attempted.

## Continuation rules

- `SESSION_HANDOFF.md` is the sole actionable continuation point; update only when resumable work/checkpoint/next action materially changes.
- Update `REFACTOR_ROADMAP.md` only when module status/evidence/priorities materially change.
- Root `dev_log.md` keeps 20–119 complete newest-first records; at 120, archive the oldest 100 verbatim and retain newest 20. Use 384 KiB as early-rotation target and 512 KiB as hard limit.
- Preserve archive snapshots/indexes byte-for-byte; do not add routine archive snapshots or docs-only commits.
- Run `scripts/check_ai_handoff_docs.py` only with a full local checkout; exact-head Actions are authoritative for the pushed SHA.
- Recheck live `main`, branches and exact-head Actions on every takeover. Never force-push. Do not claim target builds or device tests without evidence.
