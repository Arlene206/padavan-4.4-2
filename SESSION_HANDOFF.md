## Current repository state

- Maintained fork: Arlene206/padavan-4.4-2; do not substitute upstream.
- Base main SHA for THIS conntrack-callback source-contract correction: 32ade08379b4bad18b8137801a57bb109815cf2a; active branch main; only main exists, no checkpoint branch.
- FULLCONENAT exact-head run [42678293858] passed notifier/cleanup, GNU89 namespace/zone policy regression, struct and mapping-lookup filters, check_mapping helpers, corrected IPv4 calls, and the IPv6 inbound/outbound lookup assertions. It then failed at ct_event_cb extraction: the workflow's generic brace scanner counted both alternative signatures under CONFIG_NF_CONNTRACK_CHAIN_EVENTS and its #else branch. This is valid conditional C source with differing signatures, not a runtime-code failure.
- AI handoff docs [42678293955] completed SUCCESS on exact head_sha 32ade08379b4bad18b8137801a57bb109815cf2a.
- The preceding workflow contract was aligned to the IPv6 behavior: get_mapping6_by_ext_port() validates its candidates internally; the target uses that lookup for inbound DNAT and check_mapping6(src_mapping, &context) for outbound reuse.
- THIS batch scopes the callback source-contract extraction from its stable marker through the next function declaration, instead of feeding a preprocessor-alternative signature into a naive brace counter. No production C behavior is changed.
- The last production-code batch corrected three IPv4 check_mapping() calls and removed five now-identical #if/#else declaration wrappers to make ordinary function extraction reliable.
- Prior FTP and docs validation remains green at exact SHA 895f2a9ebcb58e810694f76cbf13706ffea9ef75 (runs [42667988550], [42667988522]).

## Completed FULLCONENAT namespace / zone work

- IPv4/IPv6 mapping entries and pending dying-tuple records retain the owning struct net pointer plus conntrack zone identity (id, flags and direction).
- Mapping lookup and tuple garbage collection are scoped by the full identity. Conntrack lookups reconstruct the saved zone; final per-net target teardown unregisters the notifier, waits for legacy RCU callback readers, purges only that namespace's mapping/queue entries, then drops the conntrack namespace reference.
- The GNU89 host policy regression covers same/different namespace, zone ID, flags, direction and null inputs. It passed exact-head runs [42674430844], [42677259891], [42677876443] and [42678293858]. The notifier/cleanup lifecycle contract also passed all four runs.
- Three IPv4 helper call sites now pass &context; five identical version-conditional declaration wrappers were removed so the semantic function-body extractor can parse the refactored functions.
- The exact-head semantic contract has progressed through mapping structures, namespace/zone lookup filters, IPv4 and IPv6 lookup paths, then exposed the notifier callback's intentionally different preprocessor signatures. THIS workflow-only patch scopes callback extraction between stable source markers. Exact-head rerun pending.
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

1. Verify every Action on THIS callback-contract correction's exact full SHA. In FULLCONENAT, require notifier lifecycle, GNU89 policy compile/run and the entire namespace/zone semantic contract to pass; confirm AI handoff docs succeeds on the same SHA.
2. If green, continue NFS custom exports provenance and trust-boundary review:
   - Trace UI/CGI/shell/RC writers and readers of /etc/storage/exports and identify ownership, permissions, symlinks and whether untrusted/local accounts can write it.
   - Establish administrator-only override semantics before classifying missing in-code mode/owner/content validation as a vulnerability.
   - Inspect /etc/storage provisioning, /etc tmpfs lifecycle, exportfs parse/failure recovery and the NFS writer's generated fallback rules.
   - Treat async durability, AUTH_SYS/no_root_squash identity compatibility and external mounts as separate decisions.
3. Retain DHCP callback-state semantics and FTP realpath-to-syscall TOCTOU in the broader queue.

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
