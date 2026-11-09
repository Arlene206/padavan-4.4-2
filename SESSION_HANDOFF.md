## Current repository state

- Maintained fork: Arlene206/padavan-4.4-2; do not substitute upstream.
- Base main SHA for THIS downstream source-contract repair: `5718821e4b566fea1856945c322899b5145c995c`; active branch `main`. The live remote branch inventory contains only `main`; no checkpoint branch exists.
- Exact-head Actions on `5718821e4b566fea1856945c322899b5145c995c`: WAN DHCP callback [42694649038] SUCCESS; WAN interface teardown [42694649527] SUCCESS; AI handoff docs [42694649648] SUCCESS. The WAN DHCP GNU89 rollback compile/run and extended MAN deconfig ordering contract passed; the documentation inventory validator passed.
- Two dependent source-contract workflows exposed follow-up test-maintenance gaps on the same SHA: WAN route status [42694649350] compiled and ran its sanitizer host regression, then failed because its Python check still extracted the public `man_down()` wrapper after the implementation moved to `man_down_internal()`; HTTPD security [42694649091] passed its earlier policy and source checks, then failed its final DHCP lease contract because it likewise expected resolver/firewall calls in the wrapper. These are stale test extraction ranges, not observed compiler or host-policy failures; the exact failing assertions were inspected in the Action logs.
- THIS batch updates both workflows to inspect the actual internal teardown body, verifies gateway host-route deletion precedes MAN address invalidation, and separately checks that the public wrapper still delegates with address reset disabled. No production code is changed in this correction.
- The underlying MAN deconfig fix remains on main: the old lease address is available through off-subnet gateway route cleanup, then cleared before resolver/firewall refresh; interface-clear failure is logged and returned after best-effort cleanup.
- BusyBox callback audit remains: `udhcp_run_script()` discards `spawn_and_wait()` status; `leasefail` occurs on initial discovery retry exhaustion; a NAK is followed by deconfig for a previously bound lease but not initial REQUESTING; SIGUSR2/release-on-quit invoke deconfig rather than a distinct release callback. Do not change these semantics without a demonstrated requirement.
- Remaining WAN teardown concern: `man_down()` / `wan_down()` ignore route deletion return values despite the underlying DHCP/static helpers returning status; trace failure accumulation and state publication before modifying.
- NFS custom-export provenance review found no untrusted/local-account writer. `/etc` is tmpfs; `/etc/storage` is root-managed persistent MTD content; web restore requires authenticated privileged access. The RC writer accepts regular files or symlinks to regular files, falls back only on ENOENT, and fails closed otherwise. `nfsd.sh` propagates `exportfs -r` failure. No NFS code change was justified; async durability, AUTH_SYS/no_root_squash and external mounts remain separate compatibility decisions.
- Root `dev_log.md` will contain 74 /119 complete newest-first records after THIS entry; no archive rotation is due.

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

1. Reconcile THIS workflow contract repair on its exact resulting main SHA. Require WAN DHCP, WAN interface teardown, WAN route-status, HTTPD and AI handoff documentation workflows to complete successfully on that same SHA, and inspect the failed/repaired steps' logs.
2. If green, continue the WAN/MAN route teardown audit: propagate accumulated statuses from gateway, DHCP and static route deletes through `man_down()` and `wan_down()` while still attempting every cleanup; retain source lease fields until all associated route removals have been attempted.
3. Finish the DHCP callback semantics review without changing BusyBox's ignored script-exit-status contract absent a demonstrated issue. Preserve the MAN off-subnet route fix and the bound callback rollback ordering.
4. Keep NFS async durability/AUTH_SYS/root-squash/external mounts, Samba same-boot passdb/config recovery, FULLCONENAT target build, and FTP realpath-to-syscall TOCTOU visible in the wider whole-userland queue.

## Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for the MAN off-subnet gateway host-route dependency and deconfig ordering fix, and for the downstream workflow assertion updates; the public `man_down()` wrapper preserves its original non-reset behavior. Previous WAN bound transaction/rollback and NFS trust-boundary reviews remain PASS. |
| L2 host / Actions | Exact SHA `5718821e4b566fea1856945c322899b5145c995c`: WAN DHCP callback [42694649038], WAN interface teardown [42694649527] and AI docs [42694649648] SUCCESS. WAN route status [42694649350] failed only in source-body extraction after implementation refactor; HTTPD security [42694649091] failed its final MAN source-contract assertion for the same wrapper/body assumption. THIS workflow correction and exact-head reruns are PENDING. |
| L3 production RC object, MIPS/uClibc kernel object, full firmware/TRX | NOT VERIFIED. |
| L4 target router DHCP release/NAK/lease-expiry, failed ioctl and route reconciliation behavior | NOT VERIFIED. |

## Exact first action for the next GPT

1. Fetch live `main`, branch/checkpoint inventory and recent Actions; verify the exact resulting SHA for THIS source-contract repair.
2. Confirm WAN DHCP, WAN interface teardown, WAN route status, HTTPD security and AI handoff docs all complete on that exact SHA. Inspect the repaired route/HTTPD step logs and ensure the host tests remain green.
3. Then trace return values from `route_del()`, `del_dhcp_routes_by_prefix()` and `del_static_man_routes()` through `man_down()` / `wan_down()`; decide how to report accumulated failures while still attempting all cleanup. Preserve old lease inputs until route operations have been attempted.

## Continuation rules

- `SESSION_HANDOFF.md` is the sole actionable continuation point; update only when resumable work/checkpoint/next action materially changes.
- Update `REFACTOR_ROADMAP.md` only when module status/evidence/priorities materially change.
- Root `dev_log.md` keeps 20–119 complete newest-first records; at 120, archive the oldest 100 verbatim and retain newest 20. Use 384 KiB as early-rotation target and 512 KiB as hard limit.
- Preserve archive snapshots/indexes byte-for-byte; do not add routine archive snapshots or docs-only commits.
- Run `scripts/check_ai_handoff_docs.py` only with a full local checkout; exact-head Actions are authoritative for the pushed SHA.
- Recheck live `main`, branches and exact-head Actions on every takeover. Never force-push. Do not claim target builds or device tests without evidence.
