## Current repository state

- Maintained fork: Arlene206/padavan-4.4-2; do not substitute upstream.
- Base main SHA for THIS edge-case regression and handoff checkpoint correction: `b6c1cdea285b39925c1e9254c7fb325739129491`; active branch main; only main exists; no checkpoint branch.
- The WAN DHCP callback fix commit `b6c1cdea285b39925c1e9254c7fb325739129491` and every dependent Action below completed SUCCESS on that exact SHA:
  - WAN DHCP callback policy compile/run and source-order contract [42690381038].
  - AI handoff documentation validator [42690381155].
  - WAN route status [42690381079], WAN interface teardown [42690380777], and HTTPD security/DHCP source contract [42690380735].
- The code fix adds `dhcp_wan_interface_apply()`: candidate WAN addresses are applied before old gateway/DHCP route deletion or primary lease NVRAM publication; failed interface ioctls trigger best-effort restoration of the old address (or a clear when no prior lease was valid), and the callback returns the original error. All five exact-head Actions above passed on `b6c1cdea285b39925c1e9254c7fb325739129491`.
- THIS follow-up extends the host regression with the first-lease edge case where the initial clear fails and the best-effort rollback clear also fails; it also removes duplicate continuation headings introduced in the prior handoff edit. The new exact-head regression and documentation check are pending.
- NFS custom-export provenance review found no untrusted/local-account writer. `/etc` is tmpfs; `/etc/storage` is root-managed persistent MTD content; web restore requires authenticated privileged access. The RC writer accepts regular files or symlinks to regular files, falls back only on ENOENT, and fails closed otherwise. `nfsd.sh` propagates `exportfs -r` failure. No NFS code change was justified; async durability, AUTH_SYS/no_root_squash and external mounts remain separate compatibility decisions.
- Root `dev_log.md` has 72 /119 complete newest-first records after THIS entry; no archive rotation is due.

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

1. Verify THIS follow-up WAN DHCP edge-case regression and AI handoff documentation checks against the exact resulting main SHA. Inspect every step.
2. If green, continue WAN DHCP event-state review at `udhcpc_deconfig()`, leasefail/NAK/release handling and BusyBox's ignored callback exit status. Separately inspect remaining route-delete/add result handling; do not repeat the interface-apply transaction.
3. Keep NFS async durability, AUTH_SYS/root-squash and external mounts as separate compatibility decisions. Retain Samba same-boot file publication and FTP realpath-to-syscall TOCTOU in the wider queue.

## Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for WAN DHCP bound callback input checks, candidate interface-before-state ordering, rollback path and NFS custom-export trust boundary. Prior FULLCONENAT namespace/zone review remains PASS. |
| L2 host / Actions | Exact SHA `b6c1cdea285b39925c1e9254c7fb325739129491`: WAN DHCP [42690381038], AI docs [42690381155], WAN route [42690381079], WAN teardown [42690380777] and HTTPD [42690380735] all SUCCESS. THIS added first-lease rollback-failure test and documentation correction are PENDING on the resulting exact HEAD. |
| L3 production RC object, MIPS/uClibc kernel object, full firmware/TRX | NOT VERIFIED. |
| L4 target router DHCP failure injection, route reconciliation, storage/NFS behavior | NOT VERIFIED. |

## Exact first action for the next GPT

1. Fetch live `main`, branches/checkpoints and recent Actions; confirm the exact full SHA for THIS follow-up commit.
2. Inspect the WAN DHCP callback regression and AI handoff docs run. Require both to pass on the same SHA, and inspect logs.
3. Then audit `udhcpc_deconfig()`, leasefail/NAK/release callback handling and remaining WAN route-delete/add status propagation. Preserve the existing bound transaction/rollback ordering.

## Continuation rules

- `SESSION_HANDOFF.md` is the sole actionable continuation point; update only when resumable work/checkpoint/next action materially changes.
- Update `REFACTOR_ROADMAP.md` only when module status/evidence/priorities materially change.
- Root `dev_log.md` keeps 20–119 complete newest-first records; at 120, archive the oldest 100 verbatim and retain newest 20. Use 384 KiB as early-rotation target and 512 KiB as hard limit.
- Preserve archive snapshots/indexes byte-for-byte; do not add routine archive snapshots or docs-only commits.
- Run `scripts/check_ai_handoff_docs.py` only with a full local checkout; exact-head Actions are authoritative for the pushed SHA.
- Recheck live `main`, branches and exact-head Actions on every takeover. Never force-push. Do not claim target builds or device tests without evidence.
