## Current repository state

- Maintained fork: `Arlene206/padavan-4.4-2`; never substitute the upstream repository.
- Base `main` SHA for THIS FTP path-ACL batch: `d784677ab88890bf1551c0a03fec85219285933f`. Only `main` exists; there is no separate checkpoint branch.
- Root `dev_log.md` has 63 complete newest-first records before THIS entry; it remains below the 384 KiB early-rotation target.
- The previous checkpoint's exact-head Actions both passed on `d784677ab88890bf1551c0a03fec85219285933f`: vsftpd [42662555873] and AI handoff docs [42662555510]. Both full `head_sha` values matched `d784677ab88890bf1551c0a03fec85219285933f`; the vsftpd job compiled and ran the three GNU89 host regression binaries and passed its semantic source contracts; the docs job passed the archive/root-log validator.

## Work completed in THIS patch

- The bundled server is vsftpd 3.0.3; PAM is disabled, and Padavan NVRAM FTP accounts are mapped to UID/GID 99. Per-share metadata ACLs are therefore the account-separation boundary.
- The prior RNFR/RNTO correction is already committed: RNFR pins an absolute source and checks canonical source-parent delete permission; RNTO rechecks source delete and destination write permission.
- The follow-up source audit found several file and directory commands checking `PERM_READ/WRITE/DELETE` against the session CWD even when the FTP argument named a different share. This left cross-share argument paths insufficiently guarded.
- STOR/APPE/STOU now check write rights against the canonical parent of the effective destination. Existing final symlinks are resolved to the actual target before ACL evaluation; dangling/unresolvable existing entries fail closed. A missing leaf is checked against its canonical parent, including symlinked parent directories.
- RETR now checks read rights against the canonical parent of the resolved existing target, so final symlinks into another share are evaluated against that share.
- MKD/RMD/DELE now check the canonical parent of the requested directory entry, matching their filesystem semantics (these operations do not follow a final symlink to operate on its target).
- LIST/NLST/STAT now check read rights against the actual canonical directory opened, not the session CWD. The permission check occurs before sending a data-channel opening reply or STAT output.
- Extended the GNU89 host path-policy regression to cover existing cross-share symlink targets, dangling final symlinks, new upload leaves, symlinked parents and directory symlinks. The vsftpd workflow now asserts that each command family uses its target-path ACL helper and that listing checks precede data-channel setup.
- Path checks remain pathname-based and therefore do not eliminate a concurrent symlink-swap TOCTOU window between canonicalization and the later filesystem operation. A dirfd/openat-style operation binding remains a separate hardening opportunity.

## Active task

1. Verify the exact full SHA on every GitHub Actions run triggered by THIS commit. Inspect job steps/logs; specifically confirm the GNU89 helper/test binaries and the new semantic source contracts pass.
2. Continue the FTP filename-command audit: SIZE, MDTM, SITE CHMOD, direct access to reserved `.__*` share metadata (including per-account ACL files and folder inventory), and any remaining path-taking commands. Confirm whether metadata files are readable under the deployed mount modes before deciding the protection boundary.
3. Keep NFS custom override content/ACL and `async` durability, DHCP leasefail/accepted-NAK/release semantics, and FULLCONENAT mapping-hash/dying-tuple namespace ownership on the whole-repository queue.

## Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for the CWD-versus-target ACL gap in STOR/APPE/STOU, RETR, MKD/RMD/DELE and LIST/NLST/STAT; operation-specific final-symlink handling and fail-closed resolution are implemented. |
| L2 host / Actions | PENDING for THIS SHA. Parent checkpoint `d784677ab88890bf1551c0a03fec85219285933f` had exact-head vsftpd [42662555873] SUCCESS and AI docs [42662555510] SUCCESS. This patch adds GNU89 path-policy regression cases and semantic source contracts; verify the exact new head before claiming them passed. |
| L3 MIPS/uClibc / production vsftpd object / firmware/TRX | NOT VERIFIED. |
| L4 live FTP cross-share ACL, authentication/anonymous policy, storage hot-unplug and router validation | NOT VERIFIED. |

## Exact first action for the next GPT

1. Fetch live `main`, branch list and Actions. Verify every triggered workflow's full `head_sha` equals the new main SHA; inspect each job's steps and failure logs.
2. If exact-head CI is green, continue SIZE/MDTM/SITE CHMOD and direct share-metadata-file access review. Do not infer that these commands are covered by the helpers in THIS patch.
3. Keep the NFS, DHCP and FULLCONENAT queue items visible; do not narrow the project to vsftpd.

## Continuation rules

- `SESSION_HANDOFF.md` is the sole actionable continuation point; update it only when the resumable task/checkpoint/next action materially changes.
- Update `REFACTOR_ROADMAP.md` only when module status/evidence/priorities materially change.
- Root `dev_log.md` keeps 20–119 complete newest-first records; at 120, archive the oldest 100 verbatim and retain the newest 20. Use 384 KiB as the early batch-rotation target and 512 KiB as the hard limit.
- Preserve existing archive snapshots/indexes byte-for-byte; do not add routine archive snapshots or docs-only follow-up commits.
- Run `scripts/check_ai_handoff_docs.py` only with a full local checkout; exact-head Actions are authoritative for the pushed SHA.
- Recheck live `main`, branches and exact-head Actions on every takeover. Never force-push. Do not claim target builds or device tests without evidence.
