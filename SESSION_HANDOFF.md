## Current repository state

- Maintained fork: Arlene206/padavan-4.4-2; do not substitute upstream.
- Base main SHA for THIS FULLCONENAT correction batch: e0dcc7dbc3a85d5f6b1115e63e8f7b75a0e8e08d. Active branch is main. Remote branch inventory contains only main; no checkpoint branch exists.
- Commit e0dcc7dbc3a85d5f6b1115e63e8f7b75a0e8e08d introduced per-network-namespace and conntrack-zone identity into global FULLCONENAT mapping and dying-tuple state. Exact-head FULLCONENAT Actions [42674430844](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42674430844) passed the prior notifier lifecycle contract and GNU89 context-policy regression, then failed the new semantic contract because its struct regex was over-escaped and could not match struct nat_mapping. AI handoff docs run [42674431213](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42674431213) also reports failure; the Actions API returned no job/check-run detail, so the cause is unclassified.
- Source review of e0dcc7dbc3a85d5f6b1115e63e8f7b75a0e8e08d also found three IPv4 target call sites retaining the old check_mapping() API: two passed context by value and one used obsolete (net, zone) arguments. THIS batch corrects all three to pass &context and adds IPv4/IPv6 target source contracts. Exact-head rerun is pending.
- The previous vsftpd correction is closed: runs [42667988550](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42667988550) and [42667988522](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42667988522) both completed SUCCESS on exact head_sha 895f2a9ebcb58e810694f76cbf13706ffea9ef75.
- Root dev_log.md remains below the 384 KiB rotation target and within the 20–119 record window after THIS entry. No archive rotation is required.

## Completed FULLCONENAT namespace / zone work

- IPv4/IPv6 mapping entries and pending dying-tuple records retain the owning struct net pointer plus conntrack zone identity (id, flags and direction).
- Mapping lookup and tuple garbage collection are scoped by the full identity. Conntrack lookups reconstruct the saved zone; final per-net target teardown unregisters the notifier, waits for legacy RCU callback readers, purges only that namespace's mapping/queue entries, then drops the conntrack namespace reference.
- The new GNU89 host policy regression covers same/different namespace, zone ID, flags, direction and null inputs. It passed in both exact-head runs [42674430844] (before final semantic contract) and [42677259891] (before final semantic contract).
- The three IPv4 check_mapping call sites now pass &context; the exact-head semantic contract confirmed those target call sites. In run [42677259891], the same contract advanced to get_mapping6_by_ext_port() and then failed because a source scanner counted both identical #if/#else signature opening braces. THIS batch removes five redundant version-conditional wrappers whose signatures are now byte-for-byte identical after the context API refactor. These are preprocessor-equivalent declaration cleanups, not runtime behavior changes.
- Kernel translation-unit compilation remains unverified; the standalone policy binary and source-contract workflow do not replace a target kernel object build.

## Completed FTP work

- Existing RNFR/RNTO fix: absolute source-path pinning plus source-delete/destination-write ACL revalidation against canonical parents.
- Path-aware checks now cover STOR/APPE/STOU, RETR, MKD/RMD/DELE, LIST/NLST/STAT, SIZE, MDTM and SITE CHMOD. Upload/RETR/metadata operations authorize the effective existing symlink target; MKD/RMD/DELE authorize the entry parent; listings authorize the actual opened directory before transfer setup.
- Added fail-closed handling for dangling final upload symlinks and canonical resolution of symlinked parent directories.
- Sidecar disclosure finding: `trunk/user/libdisk/src/disk_share.c` creates `.__folder_list.txt` and `.__<account>_var.txt` under the USB mount root. Per-share READ ACLs do not govern MOUNT_LAYER, so direct FTP file commands could expose folder/account ACL metadata. Exact-name filtering now hides and rejects these reserved files before the chroot ACL bypass.
- Both existing-file and upload path resolvers also classify the canonical `realpath()` target. Thus a harmless-looking final symlink alias to an ACL sidecar is denied as well as the direct filename. Ordinary `.__notes.txt` user files do not match the classifier.
- GNU89 regression now covers positive/negative metadata filenames, direct sidecar denial, final symlink alias denial, cross-share symlinks, dangling links, new leaves and symlinked directories.
- The only failure on `30d64c73546db236c12679d24eda74c1ea9f2916` is the CI Python variable-order defect described above; host binaries themselves all passed. The workflow correction moves all helper-body extraction before assertions.

## Active task

1. Verify the exact full SHA of THIS redundant-signature cleanup commit on every triggered workflow. Inspect FULLCONENAT job steps/logs; the notifier contracts, GNU89 policy regression and namespace/zone semantic contracts must all pass. Confirm the AI handoff documentation validator on the same full SHA.
2. If exact-head checks pass, continue the NFS custom exports trust-boundary audit:
   - Trace UI, CGI, shell and RC writers/readers of /etc/storage/exports; identify ownership, modes, symlink creation and whether untrusted/local accounts can modify it.
   - Decide whether this is an administrator-only authoritative override before classifying missing in-code ownership/mode/content validation as a vulnerability.
   - Review /etc/storage provisioning, /etc tmpfs lifecycle, exportfs parsing/failure recovery and NFS mount/ACL constraints.
   - Keep generated async,secure,no_root_squash,no_subtree_check defaults, root UID/GID compatibility and external-mount policy as separate decisions; do not alter options speculatively.
3. Keep DHCP leasefail/accepted-NAK/release callback-state semantics and FTP realpath-to-syscall TOCTOU visible in the whole-repository queue.

## Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for namespace/zone identity propagation and the corrected IPv4 helper call sites. Five identical preprocessor signature wrappers have now been collapsed so the semantic contract scanner can reliably extract the affected functions. |
| L2 host / Actions | Base [42677259891] exact-head: notifier/lifecycle contract PASS; GNU89 namespace/zone policy binary compile/run PASS; semantic step advanced past struct and IPv4 call-site checks, then failed because function_body() counted both identical #if/#else function-signature opening braces. AI handoff docs [42677259833] SUCCESS on the exact base SHA. THIS cleanup's exact-head FULLCONENAT/docs runs pending. |
| L3 MIPS/uClibc kernel object, full firmware/TRX | NOT VERIFIED. |
| L4 target router namespace/conntrack-zone/NAT behavior | NOT VERIFIED. |

## Exact first action for the next GPT

1. Fetch live main, remote branches and Actions for the new full SHA; verify exact head_sha equality.
2. Inspect FULLCONENAT job logs and confirm all source-contract checks complete successfully, and confirm the AI handoff docs workflow is green on the same full SHA.
3. If green, proceed to trace NFS custom exports provenance, writer permissions and content policy before changing behavior.

## Continuation rules

- `SESSION_HANDOFF.md` is the sole actionable continuation point; update only when resumable work/checkpoint/next action materially changes.
- Update `REFACTOR_ROADMAP.md` only when module status/evidence/priorities materially change.
- Root `dev_log.md` keeps 20–119 complete newest-first records; at 120, archive the oldest 100 verbatim and retain newest 20. Use 384 KiB as early-rotation target and 512 KiB as hard limit.
- Preserve archive snapshots/indexes byte-for-byte; do not add routine archive snapshots or docs-only commits.
- Run `scripts/check_ai_handoff_docs.py` only with a full local checkout; exact-head Actions are authoritative for the pushed SHA.
- Recheck live `main`, branches and exact-head Actions on every takeover. Never force-push. Do not claim target builds or device tests without evidence.
