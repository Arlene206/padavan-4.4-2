## Current repository state

- Maintained fork: Arlene206/padavan-4.4-2; work only on this fork.
- Live `main` SHA: `38a86273a44bc481666674606979321701703f71`; parent: `639ebbdedca75d1938e7d280108d146d05e2e479`. Remote branch list contains only `main`; no checkpoint branch exists.
- The route-add propagation commit `38a86273a44bc481666674606979321701703f71` is published. It adds private status-returning MAN/WAN up helpers, propagates direct and static/DHCP helper route-add errors without short-circuiting, preserves the public void APIs, and makes `udhcpc_bound()` log/return the route-setup status. It intentionally avoids blind route rollback because route ownership cannot be inferred from an add failure.
- Exact-head Actions on `38a86273a44bc481666674606979321701703f71`: WAN route operation status [42703179674], AI handoff docs [42703179667], WAN interface teardown [42703179436], and HTTPD security [42703179416] completed SUCCESS.
- WAN DHCP callback [42703179710] is FAILURE, but its GNU89 interface transaction compile/run step passed. The subsequent source-contract step failed because it still searched for `wan_up(wan_ifname, unit, 0)`; this change correctly calls `wan_up_internal()` / `man_up_internal()` to capture the route status. The failure is a stale test assertion, not evidence of a production compile failure. THIS commit updates that assertion and adds checks for status capture/return; exact-head rerun is pending.
- BusyBox `udhcp_run_script()` continues to discard callback status; logs remain the operational failure channel. The callback-state review is still open.
- L3 production RC/MIPS/uClibc/firmware and L4 physical router validation are not established.
- Root `dev_log.md` currently has 77 complete records; THIS contract-correction entry makes 78 /119. No archive rotation is due.

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

1. Reconcile THIS DHCP callback contract correction on the resulting exact `main` SHA. Require the DHCP interface transaction compile/run and full source-order assertions to pass, along with WAN route, WAN teardown, HTTPD and AI handoff workflows; compare each run's `head_sha`.
2. Once green, finish the bounded BusyBox DHCP callback audit: initial `leasefail`, accepted NAK followed by deconfig, lease expiry, renew/repeated renew, and SIGUSR2/release-on-quit. Trace how `udhcp_run_script()` discards child status; do not change that legacy behavior absent a demonstrated lifecycle/state defect.
3. Rotate out of WAN after that bounded review. Start with Samba same-boot passdb/config publication, then the next most consequential stability/security item from the whole-userland governance ledger. Keep FTP realpath-to-syscall TOCTOU, FULLCONENAT target build, and NFS compatibility questions visible.

## Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for MAN/WAN direct and helper route-add aggregation, first-error retention without short-circuiting, logging, public void API preservation, and DHCP bound status capture/return. |
| L2 host / Actions | Exact SHA `38a86273a44bc481666674606979321701703f71`: WAN route [42703179674], AI docs [42703179667], WAN teardown [42703179436], and HTTPD [42703179416] SUCCESS. DHCP [42703179710] host transaction compile/run PASS; semantic source-order check failed only on its obsolete `wan_up()` call assertion. THIS correction is pending exact-head rerun. |
| L3 production RC object, MIPS/uClibc objects, complete firmware/TRX | NOT VERIFIED. |
| L4 router route reconciliation, DHCP failure/release behavior, and hardware | NOT VERIFIED. |

## Exact first action for the next GPT

1. Fetch live `main`, branch list and Actions. Confirm the new DHCP workflow commit's exact `head_sha`; require its updated callback-ordering assertion and every triggered workflow to complete SUCCESS.
2. Then inspect BusyBox's actual `udhcp_run_script()` dispatch/callers for `leasefail`, NAK/deconfig, expiry, renew, and SIGUSR2/release-on-quit. Determine whether discarded callback status causes a reproducible transition defect before changing behavior.
3. When callback review is complete, move outside WAN to Samba same-boot passdb/config publication, then continue the whole-userland priority queue. Keep FTP path TOCTOU, FULLCONENAT target build, and NFS compatibility visible.

## Continuation rules

- `SESSION_HANDOFF.md` is the sole actionable continuation point; update only when resumable work/checkpoint/next action materially changes.
- Update `REFACTOR_ROADMAP.md` only when module status/evidence/priorities materially change.
- Root `dev_log.md` keeps 20–119 complete newest-first records; at 120, archive the oldest 100 verbatim and retain newest 20. Use 384 KiB as early-rotation target and 512 KiB as hard limit.
- Preserve archive snapshots/indexes byte-for-byte; do not add routine archive snapshots or docs-only commits.
- Run `scripts/check_ai_handoff_docs.py` only with a full local checkout; exact-head Actions are authoritative for the pushed SHA.
- Recheck live `main`, branches and exact-head Actions on every takeover. Never force-push. Do not claim target builds or device tests without evidence.
