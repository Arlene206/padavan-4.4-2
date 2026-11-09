## Current repository state

- Maintained fork: Arlene206/padavan-4.4-2; do not substitute upstream.
- Base main SHA for THIS WAN/MAN route teardown status-propagation fix: `19b3ce5dc958a3a1594cf18baa9aaba96e55b806`; active branch `main`; only `main` exists remotely and no checkpoint branch is present.
- The previous source-contract repair is green on exact SHA `19b3ce5dc958a3a1594cf18baa9aaba96e55b806`: WAN route status [42695351469], HTTPD Authentication/Security [42695351402], and AI handoff docs [42695351591] all completed SUCCESS. Route-status sanitizer host compilation/execution and every semantic assertion passed; the HTTPD suite completed all 39 steps; docs validator reports root log 74/119 and valid archive inventory.
- The preceding production MAN deconfig change passed WAN DHCP callback [42694649038] and WAN interface teardown [42694649527] on exact parent SHA `5718821e4b566fea1856945c322899b5145c995c`. This commit changes `net_wan.c` and callback contracts, so those workflows must rerun on the new exact SHA.
- The MAN DHCP route-leak correction remains: retain the old `wanx_ipaddr` through off-subnet gateway host-route deletion; clear it only after MAN route cleanup and before resolver/firewall refresh. Interface-clear errors are logged and combined with cleanup status.
- THIS batch makes MAN/WAN teardown report route failures: private `man_down_internal()` and `wan_down_internal()` aggregate each direct gateway/DNS route deletion and DHCP/static route helper result through `route_status_accumulate()`, continue attempting later cleanup, and return the first failure. Existing public void wrappers preserve their APIs and log incomplete teardown; `udhcpc_deconfig()` consumes the internal result and combines it with interface-clear status.
- BusyBox's `udhcp_run_script()` still discards `spawn_and_wait()` status; the new callback return value is therefore not used to stop BusyBox's internal transition. Per-operation and summary log messages make cleanup failures observable.
- Route-add status in `add_man_gateway_routes()` / `wan_up()`, callback semantics for leasefail/NAK/release and route retry/reconciliation remain open. Do not conflate this teardown fix with target routing tests.
- NFS custom-export provenance review found no untrusted/local-account writer. `/etc` is tmpfs; `/etc/storage` is root-managed persistent MTD content; web restore requires authenticated privileged access. The RC writer accepts regular files or symlinks to regular files, falls back only on ENOENT, and fails closed otherwise. `nfsd.sh` propagates `exportfs -r` failure. No NFS code change was justified; async durability, AUTH_SYS/no_root_squash and external mounts remain separate compatibility decisions.
- Root `dev_log.md` will contain 75 /119 complete newest-first records after THIS entry; no archive rotation is due.

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

1. Reconcile THIS WAN/MAN teardown status-propagation change on its exact resulting main SHA. Inspect WAN DHCP, route status, WAN interface teardown, HTTPD security and AI handoff documentation Actions; require all applicable workflows to pass on the same SHA.
2. If green, audit route additions in `add_man_gateway_routes()`, `wan_up()` and `man_up()` for ignored failures and incomplete rollback/reconciliation; extend the existing GNU89 route-status regression and source contracts without repeating deletion-status work.
3. Complete the bounded BusyBox callback state review (initial leasefail, NAK/deconfig, lease expiry, repeated renew and SIGUSR2 release) without changing ignored script-exit semantics absent a demonstrated defect.
4. Rotate broader governance coverage after the WAN batch: revisit Samba same-boot file publication and other P2 entries; keep NFS compatibility decisions, FULLCONENAT target build, and FTP realpath-to-syscall TOCTOU visible.

## Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for MAN off-subnet gateway route identity/order and accumulated route teardown status across direct host/default/DNS plus DHCP/static route deletion; WAN teardown aggregates DHCP/default/static route and static-address-clear results. Public void wrappers log failures; deconfig combines interface and route cleanup status while continuing teardown. |
| L2 host / Actions | Exact SHA `19b3ce5dc958a3a1594cf18baa9aaba96e55b806`: WAN route status [42695351469], HTTPD security [42695351402] and AI docs [42695351591] SUCCESS. Previous parent SHA `5718821e4b566fea1856945c322899b5145c995c` WAN DHCP [42694649038] and WAN teardown [42694649527] SUCCESS. THIS code change and callback-dependent workflow updates are PENDING exact resulting HEAD. |
| L3 production RC object, MIPS/uClibc kernel object, full firmware/TRX | NOT VERIFIED. |
| L4 target router ioctl failures, stale-route cleanup, DHCP NAK/expiry/release and multi-WAN behavior | NOT VERIFIED. |

## Exact first action for the next GPT

1. Fetch live `main`, branches/checkpoints and recent Actions; confirm the exact SHA for THIS teardown status fix.
2. Inspect the exact-head WAN DHCP callback, WAN route-status, WAN interface teardown, HTTPD and AI handoff docs jobs. Confirm both host tests and semantic contracts pass; do not treat parent SHA results as validation of THIS code.
3. Then trace ignored route-add results through `add_man_gateway_routes()`, `wan_up()` and `man_up()`; design bounded rollback/reporting that preserves route and NVRAM consistency. Do not undo the first-lease interface transaction or MAN deconfig fix.

## Continuation rules

- `SESSION_HANDOFF.md` is the sole actionable continuation point; update only when resumable work/checkpoint/next action materially changes.
- Update `REFACTOR_ROADMAP.md` only when module status/evidence/priorities materially change.
- Root `dev_log.md` keeps 20–119 complete newest-first records; at 120, archive the oldest 100 verbatim and retain newest 20. Use 384 KiB as early-rotation target and 512 KiB as hard limit.
- Preserve archive snapshots/indexes byte-for-byte; do not add routine archive snapshots or docs-only commits.
- Run `scripts/check_ai_handoff_docs.py` only with a full local checkout; exact-head Actions are authoritative for the pushed SHA.
- Recheck live `main`, branches and exact-head Actions on every takeover. Never force-push. Do not claim target builds or device tests without evidence.
