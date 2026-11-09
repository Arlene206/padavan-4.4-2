## Current repository state

- Maintained fork: Arlene206/padavan-4.4-2; do not substitute upstream.
- Base main SHA for THIS WAN DHCP callback transaction fix: `bc99b6801c44f55e13dc0214336408f712e0abf8`; active branch main; only main exists; no checkpoint branch.
- FULLCONENAT lifecycle run [42678875089] and AI handoff docs run [42678875391] both completed SUCCESS on exact `bc99b6801c44f55e13dc0214336408f712e0abf8`. The previous FULLCONENAT callback source-contract task is complete at L2.
- THIS batch fixes WAN DHCP bound callback state ordering. The old code deleted routes and exposed candidate IP/netmask/gateway NVRAM before checking interface configuration; failed `ifconfig()` calls were ignored. The new `dhcp_wan_interface_apply()` policy checks clear/set results, attempts to restore the old address (or clear when no valid previous lease exists), returns the original error and reports rollback failure. `udhcpc_bound()` now configures the candidate first; only then removes the old route inventory and publishes the candidate IP/netmask/gateway. When address activation fails, the previous route inventory and primary NVRAM lease remain available for the next callback.
- A pre-edit NFS custom-export trust audit found no untrusted/local-account writer. `/etc` is tmpfs; `/etc/storage` is root-managed persistent MTD content; UI restore reaches it only through authenticated privileged backup restore. The RC writer accepts regular files or symlinks to regular files, falls back to generated exports only on ENOENT, and fails closed otherwise. `nfsd.sh` propagates `exportfs -r` failures. No NFS source change was justified. Keep async durability, AUTH_SYS/no_root_squash and external-mount policy separate.
- Root `dev_log.md` has 71 /119 complete newest-first records after THIS commit entry; no rotation due.

## Completed FULLCONENAT namespace / zone work

- IPv4/IPv6 mapping entries and pending dying-tuple records retain the owning struct net pointer plus conntrack zone identity (id, flags and direction).
- Mapping lookup and tuple garbage collection are scoped by the full identity. Conntrack lookups reconstruct the saved zone; final per-net target teardown unregisters the notifier, waits for legacy RCU callback readers, purges only that namespace's mapping/queue entries, then drops the conntrack namespace reference.
- The GNU89 host policy regression covers same/different namespace, zone ID, flags, direction and null inputs. It passed exact-head runs [42674430844], [42677259891], [42677876443] and [42678293858]. The notifier/cleanup lifecycle contract also passed all four runs.
- Three IPv4 helper call sites now pass &context; five identical version-conditional declaration wrappers were removed so the semantic function-body extractor can parse the refactored functions.
- The exact-head semantic contract progressed through mapping structures, namespace/zone lookup filters, IPv4/IPv6 lookup paths and conditional notifier callback signatures. FULLCONENAT lifecycle regression [42678875089] and AI handoff docs [42678875391] both passed on the exact base SHA `bc99b6801c44f55e13dc0214336408f712e0abf8`.
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

1. Verify the WAN DHCP callback regression and AI handoff documentation Action runs on THIS commit's exact full SHA. Require the GNU89 helper compile/run and every Python source-order/path-filter assertion to pass; inspect logs, not only conclusions.
2. If green, continue the WAN DHCP event-state audit at `udhcpc_deconfig()`, leasefail/NAK/release semantics and BusyBox callback exit-status handling. Inspect cleanup and route result propagation; do not repeat the interface-apply transaction.
3. Keep separate compatibility questions open: NFS async durability, AUTH_SYS/root-squash and external mounts. Retain Samba same-boot file publication and FTP realpath-to-syscall TOCTOU in the wider queue.

## Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for WAN DHCP input validation, candidate interface-before-state ordering, rollback path and NFS custom-export trust boundary. Prior FULLCONENAT namespace/zone source review remains PASS. |
| L2 host / Actions | FULLCONENAT [42678875089] and AI handoff docs [42678875391] SUCCESS on exact base SHA `bc99b6801c44f55e13dc0214336408f712e0abf8`. THIS WAN DHCP callback helper/source regression is PENDING the resulting exact HEAD. No local host binary was run in this session. |
| L3 production RC object, MIPS/uClibc kernel object, full firmware/TRX | NOT VERIFIED. |
| L4 target router DHCP failure injection, route reconciliation, storage/NFS behavior | NOT VERIFIED. |

## Exact first action for the next GPT

1. Fetch live `main`, branches/checkpoints and recent Actions; confirm the exact full SHA for THIS commit.
2. Inspect the WAN DHCP callback workflow [pending at commit creation] and AI handoff docs run. Require both to pass on the same SHA; inspect every step.
3. Then audit `udhcpc_deconfig()`, leasefail/NAK/release callback handling and the remaining WAN route-delete/add status propagation. Preserve this commit's ordering/rollback behavior.

## Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for namespace/zone identity propagation, mapping lookup/GC scoping, teardown ordering and corrected IPv4/IPv6 target behavior. |
| L2 host / Actions | Exact head 32ade08379b4bad18b8137801a57bb109815cf2a: FULLCONENAT [42678293858] passed notifier, GNU89 policy binary and every semantic check through IPv6 path assertions; last step failed only because the generic scanner counted both conditional ct_event_cb signature braces. AI docs [42678293955] SUCCESS on same exact SHA. THIS callback contract correction awaits exact-head rerun. |
| L3 MIPS/uClibc kernel object, full firmware/TRX | NOT VERIFIED. |
| L4 target router namespace/conntrack-zone/NAT behavior | NOT VERIFIED. |

## Exact first action for the next GPT

1. Fetch live main, branches and new Actions; verify exact head_sha.
2. Inspect the FULLCONENAT log through the final semantic contract and the AI handoff validator. All steps must pass at the same full SHA.
3. Then start the NFS custom exports writer provenance/permissions/content-policy audit; do not alter defaults without a confirmed compatibility/security finding.

## Continuation rules

- `SESSION_HANDOFF.md` is the sole actionable continuation point; update only when resumable work/checkpoint/next action materially changes.
- Update `REFACTOR_ROADMAP.md` only when module status/evidence/priorities materially change.
- Root `dev_log.md` keeps 20–119 complete newest-first records; at 120, archive the oldest 100 verbatim and retain newest 20. Use 384 KiB as early-rotation target and 512 KiB as hard limit.
- Preserve archive snapshots/indexes byte-for-byte; do not add routine archive snapshots or docs-only commits.
- Run `scripts/check_ai_handoff_docs.py` only with a full local checkout; exact-head Actions are authoritative for the pushed SHA.
- Recheck live `main`, branches and exact-head Actions on every takeover. Never force-push. Do not claim target builds or device tests without evidence.
