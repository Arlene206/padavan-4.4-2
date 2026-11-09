## Current repository state

- Maintained fork: Arlene206/padavan-4.4-2; do not substitute upstream.
- Base main SHA for THIS IPv6 FULLCONENAT source-contract correction: 736fab062b66468a654f73f6c67ed42bd9babb69. Active branch is main; only main exists and no checkpoint branch is present.
- FULLCONENAT exact-head Actions [42677876443] passed per-net notifier/cleanup and GNU89 namespace/zone policy regression, then failed the namespace/zone semantic contract at its IPv6 target assertions. The contract incorrectly required a direct check_mapping6(mapping, &context) call in the target, although IPv6 inbound mapping validation is deliberately performed inside get_mapping6_by_ext_port() before it returns the candidate. The IPv6 target itself calls that lookup and validates outbound reuse with check_mapping6(src_mapping, &context).
- The same run passed the earlier struct, namespace/zone, five mapping lookup and check_mapping helper contracts. AI handoff docs [42677876166] completed SUCCESS on exact head_sha 736fab062b66468a654f73f6c67ed42bd9babb69.
- Earlier production correction 3f3368948e481d90db14e7e00868062dce7ef02b fixed the three stale IPv4 check_mapping() call sites and simplified five byte-identical version-conditional function signatures; the latter made the semantic function extractor work without counting duplicate preprocessor alternatives.
- THIS batch aligns the IPv6 contract with the actual inbound path by checking the getter's internal check_mapping6() guard and the target's call to that getter. The updated exact-head FULLCONENAT/docs rerun is pending.
- The earlier vsftpd correction remains closed: [42667988550] and [42667988522] both passed on exact SHA 895f2a9ebcb58e810694f76cbf13706ffea9ef75.
- Root dev_log.md remains under the 384 KiB target and within the 20–119 record window; no archive rotation is required.

## Completed FULLCONENAT namespace / zone work

- IPv4/IPv6 mapping entries and pending dying-tuple records retain the owning struct net pointer plus conntrack zone identity (id, flags and direction).
- Mapping lookup and tuple garbage collection are scoped by the full identity. Conntrack lookups reconstruct the saved zone; final per-net target teardown unregisters the notifier, waits for legacy RCU callback readers, purges only that namespace's mapping/queue entries, then drops the conntrack namespace reference.
- The GNU89 host policy regression covers same/different namespace, zone ID, flags, direction and null inputs. It has passed exact-head runs [42674430844], [42677259891] and [42677876443]. The notifier/cleanup contract also passed all three runs.
- The three IPv4 check_mapping call sites now pass &context. Five identical #if/#else signature wrappers were removed to keep the semantic source extractor aligned with the preprocessor-visible function body.
- The latest semantic run confirmed the struct layouts, zone context, all mapping lookup filters, check_mapping helpers and corrected IPv4 calls. It exposed one incorrect IPv6 test assumption: inbound mapping validation resides in get_mapping6_by_ext_port(), not as a second direct check in the target. THIS batch corrects that contract; new exact-head Actions pending.
- Kernel translation-unit compilation remains unverified; the standalone policy binary and source contracts do not replace a target kernel object build.

## Completed FTP work

- Existing RNFR/RNTO fix: absolute source-path pinning plus source-delete/destination-write ACL revalidation against canonical parents.
- Path-aware checks now cover STOR/APPE/STOU, RETR, MKD/RMD/DELE, LIST/NLST/STAT, SIZE, MDTM and SITE CHMOD. Upload/RETR/metadata operations authorize the effective existing symlink target; MKD/RMD/DELE authorize the entry parent; listings authorize the actual opened directory before transfer setup.
- Added fail-closed handling for dangling final upload symlinks and canonical resolution of symlinked parent directories.
- Sidecar disclosure finding: `trunk/user/libdisk/src/disk_share.c` creates `.__folder_list.txt` and `.__<account>_var.txt` under the USB mount root. Per-share READ ACLs do not govern MOUNT_LAYER, so direct FTP file commands could expose folder/account ACL metadata. Exact-name filtering now hides and rejects these reserved files before the chroot ACL bypass.
- Both existing-file and upload path resolvers also classify the canonical `realpath()` target. Thus a harmless-looking final symlink alias to an ACL sidecar is denied as well as the direct filename. Ordinary `.__notes.txt` user files do not match the classifier.
- GNU89 regression now covers positive/negative metadata filenames, direct sidecar denial, final symlink alias denial, cross-share symlinks, dangling links, new leaves and symlinked directories.
- The only failure on `30d64c73546db236c12679d24eda74c1ea9f2916` is the CI Python variable-order defect described above; host binaries themselves all passed. The workflow correction moves all helper-body extraction before assertions.

## Active task

1. Verify all Actions on THIS IPv6 source-contract correction's exact full SHA. The FULLCONENAT semantic check must assert that get_mapping6_by_ext_port() internally calls check_mapping6(p_current, context), and that fullconenat_tg6() uses that lookup plus check_mapping6(src_mapping, &context) for outbound reuse. Confirm the docs validator passes on the same SHA.
2. If green, resume the NFS custom exports trust-boundary investigation:
   - Trace UI, CGI, shell and RC writers/readers of /etc/storage/exports; identify ownership, mode, symlink creation and whether untrusted/local accounts can modify it.
   - Establish whether it is an intentional administrator-only authoritative override before treating absent in-code ownership/mode/content validation as an exploitable boundary.
   - Review /etc/storage provisioning, /etc tmpfs lifecycle, exportfs parser and error recovery, and generated export ACL/path checks.
   - Keep async,secure,no_root_squash,no_subtree_check defaults, AUTH_SYS root identity, durability and external-mount support as separate compatibility/security decisions.
3. Keep DHCP leasefail/accepted-NAK/release state semantics and FTP realpath-to-syscall TOCTOU in the broad audit queue.

## Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for namespace/zone identity propagation, mapping lookups, corrected IPv4 helper calls and the IPv6 inbound/outbound validation flow. |
| L2 host / Actions | Exact head 736fab062b66468a654f73f6c67ed42bd9babb69: FULLCONENAT [42677876443] passed notifier lifecycle, GNU89 policy binary and all preceding semantic checks; the last assertion failed because it expected a direct IPv6 check call instead of the getter's internal validation. AI docs [42677876166] SUCCESS on the same exact SHA. THIS workflow-contract correction awaits exact-head rerun. |
| L3 MIPS/uClibc kernel object, full firmware/TRX | NOT VERIFIED. |
| L4 target router namespace/conntrack-zone/NAT behavior | NOT VERIFIED. |

## Exact first action for the next GPT

1. Fetch live main, branch inventory and Actions; match every run head_sha to the full live SHA.
2. Inspect FULLCONENAT and docs job steps/logs. Require all notifier, GNU89, semantic source-contract and docs validator steps to pass.
3. If green, begin NFS custom exports provenance, writer permissions and content-policy tracing without making speculative option changes.

## Continuation rules

- `SESSION_HANDOFF.md` is the sole actionable continuation point; update only when resumable work/checkpoint/next action materially changes.
- Update `REFACTOR_ROADMAP.md` only when module status/evidence/priorities materially change.
- Root `dev_log.md` keeps 20–119 complete newest-first records; at 120, archive the oldest 100 verbatim and retain newest 20. Use 384 KiB as early-rotation target and 512 KiB as hard limit.
- Preserve archive snapshots/indexes byte-for-byte; do not add routine archive snapshots or docs-only commits.
- Run `scripts/check_ai_handoff_docs.py` only with a full local checkout; exact-head Actions are authoritative for the pushed SHA.
- Recheck live `main`, branches and exact-head Actions on every takeover. Never force-push. Do not claim target builds or device tests without evidence.
