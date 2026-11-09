## 2026-11-10 00:12 CST (UTC+8) — Align Samba regression contract with extracted daemon policy

- **Base main SHA:** `ef9217b4af66b81e76b53c21bbb0ac559756fa39`; resulting commit: THIS COMMIT.
- **Exact-head parent Actions:** USB storage [42651732010], WAN route [42651731613], HTTPD/security [42651731589], vsftpd [42651731452], AI handoff docs [42651731340] and NFS [42651731312] completed SUCCESS with matching full `head_sha=ef9217b4af66b81e76b53c21bbb0ac559756fa39`. Samba [42651731824] compiled and ran its GNU89 config, passdb and new daemon-presence regressions successfully, but the semantic source-contract step failed at the old assertion requiring the wait loop to remain inline in `services_stor.c`.
- **Finding:** daemon wait logic is now deliberately extracted into `samba_daemon_policy.h`; the previous regression still required `usleep(500000)`, `pids(daemon)` and `ETIMEDOUT` directly inside the wrapper. This was a stale test contract, not a production-code or host-test failure.
- **Fix:** assert the wrapper delegates to the extracted policy and that its runtime callbacks call `pids()` and `usleep()`; timeout interval and `ETIMEDOUT` behavior remain covered by the host policy test. Align `stop_stor_apps()` local variable indentation.
- **Changed paths:** `.github/workflows/samba-share-stability.yml`, `trunk/user/rc/src/services_stor.c`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation boundary:** exact parent Samba log confirms compile/run success for all three GNU89 regressions; the source-contract false negative is fixed here. New exact-head Actions are pending. No production RC object/MIPS build, firmware/TRX or router test is claimed.
- **Next:** verify all Actions for THIS exact SHA. If green, resume the whole-repository queue at vsftpd authentication/credential, ACL/path-confinement and stop/restart lifecycle.

---

## 2026-11-09 23:35 CST (UTC+8) — Gate Samba stop and WINS lifecycle on verified outcomes

- **Base main SHA:** `af3a388fd889238789f9eb0d19efe6b89c2b24e9`; resulting commit: THIS COMMIT.
- **Recovered exact-head parent Actions:** HTTPD/security [42647880398] and AI handoff docs [42647880152] both completed SUCCESS with exact matching full `head_sha=af3a388fd889238789f9eb0d19efe6b89c2b24e9`; inspected all job steps.
- **Finding — storage stop:** `stop_samba()` discarded `kill_services()` status, and `stop_stor_apps()` ignored Samba's stop result. Its safe-removal/eject callers already abort unmount when `stop_stor_apps()` fails, but the failure was only collected for NFS; a still-running Samba daemon could therefore be hidden while storage removal continued.
- **Finding — restart inventory:** `storage_has_mounted_devices()` is tri-state (negative on incomplete `/proc/mounts` inventory, zero for known-empty, positive for mounted storage). `restart_smbd()` treated any nonzero result as true, so an inventory error could incorrectly enter the service-start path instead of failing closed.
- **Finding — WINS-only lifecycle:** `start_wins()` ignored the nmbd `eval()` result and did not verify that nmbd remained present. The nmbd stop/restart and smbd reload paths also discarded stop/start/reload status.
- **Audit correction:** `dev_init.sh` mounts `/etc` as tmpfs before creating `/etc/samba`. Samba config/passdb and any journal under `/etc` are volatile and disappear on reboot. A persistent power-loss journal in this location cannot provide durability; same-boot abrupt process termination is a separate, narrower concern.
- **Fix:** make Samba/nmbd stop and start/reload helpers return status; propagate `stop_samba()` through `restart_smbd()`, RC stop commands and `stop_stor_apps()`; abort storage-dependent Samba startup on unknown mount inventory; enforce bounded process-presence postconditions for WINS-only starts/reloads. Preserve best-effort service stop ordering but return failure so the existing safe-removal gate blocks unmount.
- **Regression:** add a GNU89 callback-driven daemon-presence policy test for delayed startup, exit during settling, timeout and invalid arguments; extend Samba workflow path filters, compile/run coverage and source contracts for stop propagation, storage gating and WINS result handling.
- **Changed paths:** `trunk/user/rc/src/services_stor.c`, `trunk/user/rc/src/services_ex.c`, `trunk/user/rc/src/rc.h`, `trunk/user/rc/src/rc.c`, `trunk/user/rc/src/samba_daemon_policy.h`, `trunk/user/rc/src/tests/samba_daemon_policy.c`, `.github/workflows/samba-share-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation boundary:** L1 caller/source review PASS. Exact-head GNU89/Actions validation for THIS commit is pending. No local full repository build, MIPS/uClibc production object, firmware/TRX or router test is claimed.
- **Next:** verify exact-head Samba, HTTPD, USB/storage, NFS, vsftpd and AI docs Actions; then rotate to vsftpd authentication, ACL/path confinement and stop lifecycle. Keep same-boot process-abort recovery as a separate lower-priority item; do not add an /etc tmpfs “durable journal”.

---

## 2026-11-09 22:55 CST (UTC+8) — Align HTTPD and handoff CI contracts with daemon status API

- **Base main SHA:** `30432e4636c5b00c96cb8918caaebe37b133a28c`; resulting commit: THIS COMMIT.
- **Exact-head Actions:** Samba [42647280352], NFS [42647280434], USB storage [42647280185], vsftpd [42647280155] and WAN route [42647280379] passed. HTTPD [42647280187] failed because its source contract still searched for `void run_samba(void)` after the function became status-returning. AI handoff docs [42647282663] failed because its validator requires `## Active task` and `## Exact first action for the next GPT`.
- **Fix:** update the HTTPD regression signature lookup to `int run_samba(void)`; restore the two required handoff headings and record the exact next action. No production C change in this corrective patch.
- **Verification:** failure causes confirmed from the GitHub Actions logs; corrective exact-head Actions pending. No production RC object/firmware/device build claimed.
- **Next:** verify every current exact-head workflow; if green, continue durable PREPARED/COMMITTED passdb/config recovery and then the WINS-only daemon status path.

---

## 2026-11-09 22:45 CST (UTC+8) — Check Samba daemon reload/start status and postconditions

- **Base main SHA:** `cf75016ebf1f1150cd4db39308ead1e09bb0e66e`; resulting commit: THIS COMMIT.
- **Recovered exact-head parent CI:** all six workflows completed SUCCESS with exact matching `head_sha=cf75016ebf1f1150cd4db39308ead1e09bb0e66e`: Samba [42638624538], NFS [42638624367], HTTPD/security [42638624464], USB storage [42638624595], vsftpd [42638624417], and AI handoff docs [42638624607].
- **Finding:** `run_samba()` ignored command status for `killall -SIGHUP` and daemon `eval()`; a failed reload/start could be hidden from RC callers and the notification queue.
- **Fix:** check each required nmbd/smbd command result and a bounded stable-PID postcondition; treat a nonzero command result or missing/stale process as failure. Check optional WSDD2 too, but keep its failure as a warning rather than masking SMB availability. Make `run_samba()`/`restart_smbd()` return int; propagate through RC notification and direct command entry points. Storage startup logs failures.
- **Regression contract:** Samba workflow checks command return handling, bounded PID postconditions, function return types and RC propagation; add `rc.c` to workflow path filters.
- **Verification:** source/caller review PASS. Parent exact-head Actions above PASS. This commit's Samba/HTTPD Actions pending exact-head. No local full checkout, production RC object, MIPS/uClibc build, firmware/TRX or physical router test is claimed.
- **Next:** implement durable PREPARED/COMMITTED recovery for the passdb/config pair; then fix WINS-only nmbd start/stop status propagation before rotating to vsftpd lifecycle.

---

## 2026-11-09 22:20 CST (UTC+8) — Defer Samba share config until passdb provisioning succeeds

- **Base main SHA:** `90e6c14d835216f06163f1347c0d5d8a3a465484`; resulting commit: THIS COMMIT.
- **Recovered exact-head CI:** the preceding NFS custom override fail-closed commit passed all six workflows on matching exact `head_sha=90e6c14d835216f06163f1347c0d5d8a3a465484`: Samba config [42635610400](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42635610400), vsftpd [42635610401](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42635610401), USB storage [42635610419](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42635610419), AI handoff docs [42635610461](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42635610461), NFS [42635610314](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42635610314), and HTTPD/security [42635610731](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42635610731).
- **Finding:** `run_samba()` atomically published the new `/etc/smb.conf` before validating/provisioning all NVRAM credentials. On Samba 3.6.25 the passdb is separately staged and the previous database is preserved when a later credential fails, so the new share ACL/config could be left on disk with the old passdb while daemon reload is skipped.
- **Fix:** finish all account writes to the private staged passdb first. Only then snapshot the previous `/etc/samba/smbpasswd` with a same-directory hard link, atomically replace it, and publish the newly generated smb.conf through the existing staged writer. If config generation/publication fails, restore the old passdb (or remove a new first-run DB); preserve and log the recovery snapshot if rollback itself fails. A passdb rename failure leaves old smb.conf untouched.
- **Regression:** extend `samba_passdb_policy.h` and its GNU89 test for hard-link snapshot/restore, first-run cleanup, and non-regular/symlink target rejection. Extend `samba-share-stability.yml` to compile/run those cases and assert source order: credential provisioning -> passdb snapshot/replace -> config publication -> daemon reload.
- **Limit:** this is rollback on normal error returns, not a power-loss-atomic two-file transaction. A reset between the passdb replacement and smb.conf publication may leave the hidden rollback snapshot; interrupted-transaction startup recovery remains a follow-up. Live daemon reload/signal command status propagation also remains open.
- **Changed paths:** `trunk/user/rc/src/services_stor.c`, `trunk/user/rc/src/samba_passdb_policy.h`, `trunk/user/rc/src/tests/samba_passdb_policy.c`, `.github/workflows/samba-share-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation:** L1 caller-order and rollback-flow review PASS. GNU89 host regression and exact-head Samba/HTTPD Actions are pending for THIS commit. No full local repository compile, MIPS/uClibc production object, firmware/TRX or router test is claimed.
- **Next:** reconcile exact-head Samba/HTTPD workflows; then consider startup recovery for interrupted passdb/config updates and check daemon SIGHUP/start status/postconditions before rotating to vsftpd.

---

## 2026-11-09 22:18 CST (UTC+8) — Fail closed on invalid custom NFS exports paths

- **Base main SHA:** `a753c86b1f6329e4f9ac8e73ccfcb97f0a14865d`; resulting commit: THIS COMMIT.
- **Recovered exact-head CI:** the preceding generated NFS reserved-source-port commit was green on exact `head_sha=a753c86b1f6329e4f9ac8e73ccfcb97f0a14865d`: storage [42631443686](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42631443686), NFS [42631443757](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42631443757), vsftpd [42631443784](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42631443784), Samba [42631443514](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42631443514), AI docs [42631443719](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42631443719) and HTTPD [42631443747](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42631443747); all six were completed SUCCESS and their full `head_sha` values matched.
- **Finding:** `write_nfsd_exports()` used `check_if_file_exist()`, which follows symlinks with `stat()` and returns false for directories/non-regular targets and unresolved symlinks. A dangling `/etc/storage/exports` symlink or an existing non-regular/inaccessible path was therefore conflated with an absent override; the writer could publish generated default media exports instead of preserving the operator's intended custom-export boundary.
- **Fix:** use `lstat()` to distinguish an existing entry, follow only symlinks with `stat()`, and permit generated fallback only for an `ENOENT` lstat result. Accept a direct regular file or a symlink to a regular file; fail closed on other lstat errors, dangling/looping links, non-regular targets, and direct directories/FIFOs/devices. Reject before reading `/proc/mounts` or staging `/etc/exports`, so invalid override state cannot replace the existing exports file. Preserve existing atomic symlink publication for valid overrides.
- **Regression:** add GNU89 state-matrix assertions for true absence, EACCES/ENOTDIR, direct regular file, valid symlink, dangling/error symlink, non-regular symlink target and direct non-regular entry. Extend NFS workflow's source contract to require the lstat/stat classifier before generated exports and forbid the old `check_if_file_exist(exports_link)` branch.
- **Changed paths:** `trunk/user/rc/src/services_stor.c`, `trunk/user/rc/src/storage_nfs_export_policy.h`, `trunk/user/rc/src/tests/storage_nfs_export_policy.c`, `.github/workflows/nfsd-start-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation:** L1 source and error-path review PASS. Focused standalone classification test compiled with `cc -std=gnu89 -Wall -Wextra -Werror` and passed; this scratch test exercises the policy logic, not the complete production source. Exact-head NFS/HTTPD/storage/docs Actions are pending on THIS commit. No full local checkout, MIPS/uClibc production object, firmware/TRX or router test is claimed.
- **Compatibility/open risks:** `no_root_squash` remains for root-client compatibility; FAT/exFAT/NTFS/HFS automount uses broad `umask=0`, while ext filesystems retain POSIX ownership/mode behavior, so root-squash needs a separate compatibility path. `async` durability versus throughput is separate. A valid custom export still bypasses generated ACL/path restrictions; malformed regular-file content and `exportfs` recovery remain open; external/network mounts require an explicit trusted override.
- **Next:** reconcile all exact-head workflows. If green, rotate to Samba passdb/account mutation consistency, credential failure/recovery and live reload. Keep NFS `async`, root-squash, custom override authority/content handling and external-mount behavior as explicit audit items.

---

## 2026-11-09 22:00 CST (UTC+8) — Require reserved source ports for generated NFS exports

- **Base main SHA:** `010d5ebec0926cb52686b5c24f145d33a06514d7`; resulting commit: THIS COMMIT.
- **Recovered exact-head CI:** the preceding Samba write-only ACL correction was green on exact `head_sha=010d5ebec0926cb52686b5c24f145d33a06514d7`: Samba [42628532465](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42628532465), HTTPD [42628532589](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42628532589), NFS [42628532555](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42628532555), storage [42628532417](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42628532417), vsftpd [42628532559](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42628532559), and AI docs [42628532588](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42628532588); all six full `head_sha` values exactly matched.
- **Finding:** the generated `/media/*` NFSv2/v3 AUTH_SYS export policy used `async,insecure,no_root_squash,no_subtree_check`. In Linux 4.4 `fs/nfsd/auth.c::nfsd_setuser()`, the request UID/GID is applied directly to filesystem credentials and UID/GID 0 is mapped only when `NFSEXP_ROOTSQUASH` is enabled. In `fs/nfsd/nfsfh.c::nfsd_setuser_and_check_port()`, a non-reserved source port is rejected unless `NFSEXP_INSECURE_PORT` is set. Together, `insecure,no_root_squash` let an unprivileged process on an ACL-authorized LAN/VPN network claim UID 0 from a high source port and operate as root on writable exported media.
- **Compatibility decision:** keep `no_root_squash` and `async` unchanged in this bounded fix. Root-squash changes access to existing root-owned files and needs a separate root-client/migration plan. `async` durability versus USB throughput also needs a separate decision. This commit removes only the unnecessary `insecure` exception by requiring `secure` source ports; standard clients that use reserved ports retain root UID mapping. Root clients can still act as server root, so this is a targeted mitigation, not full AUTH_SYS authentication or root-squash protection.
- **Fix:** define the shared default `async,secure,no_root_squash,no_subtree_check` and make `write_nfsd_exports()` consume it. Preserve the operator `/etc/storage/exports` override, which remains a complete authoritative configuration that bypasses generated LAN/VPN ACL and local-media path restrictions; an administrator may explicitly opt into `insecure` there, but must treat the file as trusted root configuration.
- **Regression:** extend the GNU89 policy test to assert the exact option tuple, absence of `insecure` and intentional retention of `no_root_squash`. Extend the NFS workflow source contract to require use of the shared option constant, forbid the former default, and preserve custom override precedence/staged symlink publication.
- **Changed paths:** `trunk/user/rc/src/storage_nfs_export_policy.h`, `trunk/user/rc/src/services_stor.c`, `trunk/user/rc/src/tests/storage_nfs_export_policy.c`, `.github/workflows/nfsd-start-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation:** L1 kernel/source trust-boundary review PASS. The GNU89 regression and exact-head NFS/HTTPD/docs Actions are pending on THIS commit. No local complete checkout, host compiler execution or documentation validator invocation is claimed. L3 MIPS/uClibc/production RC/firmware and L4 client behavior/physical router remain NOT VERIFIED.
- **Next:** reconcile exact-head Actions. If green, decide a compatibility-backed root-squash migration, then audit `async` durability, custom exports absent/dangling/unreadable/malformed-path behavior, export failure recovery and external-mount support before resuming Samba passdb/account/credential audits.

---

## 2026-11-09 21:31 CST (UTC+8) — Preserve Samba write-only permissions in generated share ACLs

- **Base main SHA:** `6f9226283de4a2b2dce2b14a21efd7efb4a0fc2b`; resulting commit: THIS COMMIT. Active branch `main`; no other checkpoint branch exists.
- **Recovered exact-head CI:** after the NFS IPv4 policy-header guard fix, AI handoff docs [42625376685](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42625376685), NFS lifecycle [42625376591](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42625376591), and HTTPD security [42625376585](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42625376585) completed SUCCESS; each full `head_sha` exactly matched `6f9226283de4a2b2dce2b14a21efd7efb4a0fc2b`.
- **Finding:** `libdisk/src/disk_share.c::get_permission()` returns CIFS rights 0=deny, 1=read, 2=write, 3=read/write, and returns -1 for malformed/error state. Samba mode-4's `invalid users` builder used “cannot read” as “deny”. That incorrectly put right=2 users into the deny list despite adding them to `write list`; Samba denies users listed in `invalid users`, including users also allowed by `valid users`. The same read-only guest gate omitted write-only guest sessions from `guest ok` and `valid users`.
- **Fix:** add `samba_acl_right_is_denied()`, which denies only when neither read nor write is granted (including -1 and out-of-range values). Use it for the guest login/valid-users gates and the account deny list. Keep read-list and write-list output tied to read and write rights respectively. This restores write-only behavior without treating lookup errors as grants.
- **Exact-head Actions:** Samba [42628532465](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42628532465), HTTPD [42628532589](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42628532589), NFS [42628532555](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42628532555), storage [42628532417](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42628532417), vsftpd [42628532559](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42628532559) and AI docs [42628532588](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42628532588) all SUCCESS on exact `head_sha=010d5ebec0926cb52686b5c24f145d33a06514d7`.
- **Regression:** `tests/samba_share_policy.c` covers -1/0/1/2/3/4; HTTPD production-source contract requires the helper in both guest gates and the actual `invalid users` block.
- **NFS audit evidence:** nfs-utils 2.6.1 disables NFSv4, GSS and Kerberos and installs no GSS auth daemons; generated exports retain `async,insecure,no_root_squash,no_subtree_check`. Kernel nfsd maps request UID/GID directly unless root-squash is enabled. With AUTH_SYS and `no_root_squash`, a client in an admitted LAN/VPN network can assert UID/GID 0 against exported removable-media data. This is a P2 data trust-boundary risk, not evidence of direct router-root compromise. `async` durability, `insecure` source-port behavior, custom `/etc/storage/exports` authority and external mounts remain compatibility items; no historic NFS option was changed.
- **Changed paths:** `trunk/user/rc/src/services_stor.c`, `trunk/user/rc/src/samba_share_policy.h`, `trunk/user/rc/src/tests/samba_share_policy.c`, `.github/workflows/httpd-security.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation boundary:** L1 source and call-site review completed. L2 exact-head HTTPD/Samba/NFS/storage/docs workflows PENDING; no local complete checkout or docs validator invocation is claimed. L3 production RC/MIPS/full firmware and L4 physical router remain NOT VERIFIED.
- **Next:** inspect all Actions for THIS resulting SHA. If green, make an explicit, compatibility-backed NFS AUTH_SYS/root-squash policy decision before altering export defaults; then continue Samba passdb/account mutation and vsftpd authentication/path lifecycle audits.

---

## 2026-11-09 21:27 CST (UTC+8) — Correct NFS policy header conditional guard after exact-head compile failure

- **Base main SHA:** `ef1aec7b10389eaf240c20b5e999d80ef48996d1`; resulting commit: THIS COMMIT.
- **Exact-head Actions:** storage [42624781463](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42624781463), Samba [42624781449](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42624781449), vsftpd [42624781407](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42624781407) and AI docs [42624781292](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42624781292) all SUCCESS on exact SHA. NFS [42624781415](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42624781415) and HTTPD [42624781560](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42624781560) failed during compile of `storage_nfs_export_policy.h`; each run's full `head_sha` matched `ef1aec7b10389eaf240c20b5e999d80ef48996d1`.
- **Finding:** the new unconditional IPv4 includes and old IPv6-only block left two consecutive `#if defined (USE_IPV6)` openings but only one corresponding close, so the outer include guard was unterminated. NFS shell syntax and lifecycle fixture passed; the HTTPD NFS-policy step was reached after all preceding checks succeeded.
- **Fix:** remove the duplicate opening conditional only; retain one IPv6 helper guard and the final header include guard. No production ACL algorithm, caller behavior or historic exports options changed.
- **Changed paths:** `trunk/user/rc/src/storage_nfs_export_policy.h`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation:** preprocessor directive balance checked from the fetched exact source (outer include guard and one IPv6 block balanced); NFS/HTTPD Actions compile reruns pending on THIS commit's exact head. No local complete checkout or target build claimed.
- **Validation boundary:** L3 MIPS/uClibc/production RC object, complete firmware/TRX and L4 physical-router NFS behavior remain NOT VERIFIED.
- **Next:** reconcile all Actions for THIS exact SHA; only then continue the NFS AUTH_SYS identity, `no_root_squash`, `async`, `insecure`, custom-exports and external-mount audit.

---

## 2026-11-09 21:20 CST (UTC+8) — Validate NFS LAN and VPN IPv4 export networks

- **Base main SHA:** `3c879e06efa256ffbf376848ad69359a9098330e`; resulting commit: THIS COMMIT.
- **Recovered exact-head CI:** the preceding storage regression correction passed USB storage [42623404967](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42623404967) and AI handoff docs [42623405148](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42623405148), both SUCCESS on exact `head_sha=3c879e06efa256ffbf376848ad69359a9098330e`. The exact NFS lifecycle workflow [42622521213](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42622521213) passed on the unchanged NFS source SHA `eb21523750bc3218121f5adf3457c587d8e9a6e0`.
- **Finding:** `write_nfsd_exports()` built IPv4 LAN and VPN ACLs with legacy `ip2class()`. That helper feeds unchecked `inet_addr()` results into the network calculation and counts every set netmask bit instead of verifying a contiguous prefix. The LAN path's `is_valid_ipv4()` check validates only address syntax, not mask contiguity; the enabled VPN path did not validate `vpns_vnet` at all. A non-contiguous mask could be represented as a different CIDR, resulting in an unintended export client range.
- **Fix:** add a strict IPv4 ACL formatter that validates dotted-quad address/netmask syntax, rejects non-contiguous masks, unspecified/all-ones addresses and unrestricted /0, computes the canonical network and checks output bounds. Use it for LAN and enabled VPN ACLs; on invalid state discard the staged exports candidate and prevent NFS start/reload. Preserve the legacy `async,insecure,no_root_squash,no_subtree_check` options pending their separate compatibility review.
- **Regression:** add GNU89 host cases for /24, /32 and /1, non-contiguous masks, /0, invalid/broadcast addresses, injection-like tokens and undersized output buffers. Extend the NFS workflow source contract to require the formatter in both LAN/VPN paths and forbid the unchecked legacy call in the export writer.
- **Changed paths:** `trunk/user/rc/src/storage_nfs_export_policy.h`, `trunk/user/rc/src/tests/storage_nfs_export_policy.c`, `trunk/user/rc/src/services_stor.c`, `.github/workflows/nfsd-start-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation:** L1 source review; the affected GNU89 test and exact-head Actions are pending. No local full checkout/target build or hardware validation is claimed.
- **Validation boundary:** L3 MIPS/uClibc and production RC object, full firmware/TRX, live NFS AUTH_SYS/permission behavior and L4 hardware remain NOT VERIFIED.
- **Next:** reconcile all affected Actions against the resulting full SHA; then continue the NFS root UID/GID, export-option and custom-exports compatibility audit.

---

## 2026-11-09 21:13 CST (UTC+8) — Align storage caller-graph contract with NFS mount-state semantics

- **Base main SHA:** `eb21523750bc3218121f5adf3457c587d8e9a6e0`; resulting commit: THIS COMMIT.
- **Exact-head Actions:** NFS [42622521213](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42622521213) and AI handoff docs [42622521280](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42622521280) both SUCCESS on exact `head_sha=eb21523750bc3218121f5adf3457c587d8e9a6e0`. USB storage [42622521225](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42622521225) FAILED on the same exact SHA at its final whole-caller-graph assertion; all preceding shell syntax, Optware lifecycle and GNU89 host regression steps passed.
- **Finding:** the storage workflow still expected `func_unmount_if_mounted()` to use both numeric `$mount_status -eq 0/1` tests. The NFS mount-identity fix replaced that logic with an explicit `case` that preserves the three states (expected mount / absent / conflicting or unreadable) and refuses unsafe teardown. This is a stale cross-workflow contract, not a production NFS test failure.
- **Correction:** require the `case`-based pre-unmount conflict guard, exact tuple inventory invocation, refusal log, checked `umount`, guard-before-unmount ordering and post-unmount absent-state check. Production NFS/storage code is unchanged.
- **Changed paths:** `.github/workflows/storage-hotplug-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation:** exact job-log/source review identified the obsolete assertion in the final caller-graph step. No local full checkout or local execution of the workflow Python contract/docs validator is available; the updated storage and docs Actions are pending on THIS COMMIT's exact head.
- **Validation boundary:** L1 contract/source review PASS. NFS runtime workflow and docs/archive workflow PASS on the base exact SHA; the full storage workflow is not green yet. L3 MIPS/uClibc, complete firmware/TRX and L4 hardware remain NOT VERIFIED.
- **Next:** reconcile storage/docs Actions on the resulting exact SHA; then continue NFS export UID/GID/permission and option compatibility review before rotating to Samba passdb/account/ACL/credential security.

---

## 2026-11-09 21:01 CST (UTC+8) — Verify exact NFS pseudo-filesystem mounts before startup and teardown

- **Base main SHA:** `363f2e4101bd8087e25710e875a4719023af6637`; resulting commit: THIS COMMIT.
- **Recovered exact-head CI:** the preceding IPv6 export ACL commit passed NFS [42620760496](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42620760496), USB storage [42620760576](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42620760576), HTTPD [42620760658](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42620760658), vsftpd [42620760715](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42620760715), AI docs [42620760726](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42620760726), and Samba [42620760733](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42620760733); all six exact `head_sha` fields matched the full SHA.
- **Finding:** `func_mount_if_needed()` and `func_unmount_if_mounted()` in `nfsd.sh` searched only for the mountpoint text. They could accept an unrelated source/filesystem as the nfsd or rpc_pipefs pseudo-filesystem and could unmount that unrelated filesystem during rollback or stop. Multiple stacked mount entries were also treated as a simple present mount.
- **Fix:** add an exact `/proc/mounts` tuple check for source, mountpoint and filesystem type; distinguish absent from exactly one expected row and reject mismatched/duplicate/error states. Refuse to mount over a conflict, verify the expected entry after mount, only unmount an exact single expected tuple, and require the mountpoint to be empty afterwards.
- **Regression:** update the NFS lifecycle fixture's mount mock to record the real `mount -t filesystem source mountpoint` tuple. Add wrong-source, wrong-filesystem and duplicate/stacked mount cases, assert conflicts are preserved and never unmounted, and extend the NFS workflow source contract to cover exact field equality and guarded teardown.
- **Changed paths:** `trunk/user/nfs-utils/nfsd.sh`, `trunk/user/nfs-utils/tests/nfsd_start_lifecycle.sh`, `.github/workflows/nfsd-start-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation:** local shell regressions PASS for absent, expected, wrong-source, wrong-filesystem, prefix-neighbor and duplicate mount entries; guarded mount/unmount simulation PASS. The full repository NFS workflow and AI docs/archive validator are PENDING on THIS exact commit.
- **Validation boundary:** L1 focused source review PASS. The test harness does not execute on the actual router mount namespace; MIPS/uClibc production RC build, complete firmware/TRX, real mount-stack behavior and L4 hardware validation are NOT VERIFIED.
- **Remaining risks:** check-to-mount/unmount races are not made atomic by this shell-level check. Only the reserved fixed NFS pseudo-filesystem mountpoints are covered. Historic NFS export options and root identity semantics remain unchanged and need a separate compatibility/security audit.
- **Next:** verify NFS lifecycle and AI documentation Actions on the exact resulting SHA. If green, continue the NFS root UID/GID/export permission review (`no_root_squash`, AUTH_SYS, `async`, `insecure`), custom exports override and external-mount behavior.

---

## 2026-11-09 20:51 CST (UTC+8) — Validate IPv6 LAN ACL before publishing NFS exports

- **Base main SHA:** `5faded68510e91fb526dea6ab3ebd34b5d9dc252`; resulting commit: THIS COMMIT.
- **Recovered exact-head state:** preceding MTD retry/shutdown checkpoint passed USB storage [42618640557](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42618640557), AI docs [42618640385](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42618640385), Samba [42618640482](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42618640482), WAN [42618640381](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42618640381), and HTTPD [42618640379](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42618640379); all run SHAs exactly matched `5faded68510e91fb526dea6ab3ebd34b5d9dc252`.
- **Finding:** `write_nfsd_exports()` emitted raw `ip6_lan_addr` and `ip6_lan_size` NVRAM values into `/etc/exports`. Unlike the separate IPv6 network configuration path, this writer did not validate either field before interpreting them as NFS export syntax.
- **Fix:** validate the IPv6 address with `inet_pton(AF_INET6)`, accept only the existing optional CIDR lexical form, canonicalize with `inet_ntop(AF_INET6)`, and accept only decimal LAN prefix lengths 48–64. The separately stored prefix size remains authoritative, matching `reset_lan6_vars()`. Emit only the canonical combined value. Invalid non-empty configuration aborts and removes the staged file before `/etc/exports` is renamed, preserving the previous file.
- **Regression:** extend `storage_nfs_export_policy.c` with canonicalization, optional CIDR, empty/unconfigured, /48–/64 bounds, invalid addresses and NFS syntax injection cases. The NFS workflow now compiles/runs the policy in GNU89 with `USE_IPV6`, watches helper/test paths on push and pull requests, and asserts the production writer cannot emit the raw address/prefix interpolation.
- **Changed paths:** `trunk/user/rc/src/services_stor.c`, `trunk/user/rc/src/storage_nfs_export_policy.h`, `trunk/user/rc/src/tests/storage_nfs_export_policy.c`, `.github/workflows/nfsd-start-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation boundary:** L1 source review PASS. L2 GNU89 IPv6 ACL policy compile/run, NFS source/start/reload regression, storage/HTTPD caller contracts and docs/archive validator pending for THIS COMMIT's exact head. L3 MIPS/uClibc production object and firmware/TRX NOT VERIFIED. L4 live NFS/IPv6 client/permissions, storage-eject and hardware NOT VERIFIED.
- **Open security compatibility item:** generated exports still use the historic `async,insecure,no_root_squash,no_subtree_check` options. Root squash, AUTH_SYS identity mapping and durability/client-port policy require a separate backward-compatibility and permissions review; this commit does not silently change access semantics.
- **Next:** verify exact-head NFS/storage/HTTPD/docs Actions, then continue NFS root UID/GID policy and external-mount lifecycle/mountpoint matching audit.

---

## 2026-11-09 20:30 CST (UTC+8) — Bound MTD Optware retries and cancel its worker during shutdown

- **Base main SHA:** `652581beb0b71787096a1a74a90a3f6c32ce7cd6`; resulting commit: THIS COMMIT.
- **Recovered exact-head state:** the previous MTD caller-contract checkpoint passed USB storage [42615347517](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42615347517) and AI handoff docs [42615347722](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42615347722), both with exact `head_sha=652581beb0b71787096a1a74a90a3f6c32ce7cd6`. The remote branch list contained only `main`.
- **Finding:** boot-time MTD Optware retries were relaunched every five seconds indefinitely after persistent hook failures. The worker inherited PID1's blocked signal mask and signal dispositions: most importantly, PID1's SIGCHLD reaper could reap the shell child used by `system()`, while inherited shutdown handlers did not terminate the worker. No explicit worker-group stop ran before ordinary services were stopped, allowing late startup hooks to race shutdown.
- **Fix:** retain five-second nonblocking polling but exponentially back off actual relaunch attempts from 5 seconds to a 300-second ceiling using monotonic `uptime()`; reset after successful startup. Give the worker a private process group, reset inherited signal dispositions and unblock its signal mask before `system()`, track the worker PID, and terminate its process group with TERM/KILL from `shutdown_router()` before ordinary service-stop work. Keep external hooks asynchronous and outside `storage.lock`; preserve fail-closed RWFS/UBI teardown.
- **Regression:** extend the existing GNU89 retry-policy test with the complete backoff sequence and saturation checks. Extend the storage workflow's whole-caller-graph contract to assert worker group/signal isolation, monotonic backoff/reset, PID tracking, SIGCHLD-safe cancellation and shutdown ordering.
- **Changed paths:** `trunk/user/rc/src/common_ex.c`, `trunk/user/rc/src/rc.c`, `trunk/user/rc/src/rc.h`, `trunk/user/rc/src/storage_optware_start_policy.h`, `trunk/user/rc/src/tests/storage_optware_start_policy.c`, `.github/workflows/storage-hotplug-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation boundary:** L1 source/caller review PASS. L2 GNU89 policy compile/run, whole storage caller-graph contract and docs/archive validator are pending exact-head Actions for THIS COMMIT. No local full checkout or production RC build. L3 MIPS/uClibc/firmware and L4 actual UBI hooks, shutdown race injection and router hardware are NOT VERIFIED.
- **Risk note:** a third-party daemon that deliberately changes process group/session can escape group cancellation; the existing RWFS teardown lock/result gates remain fail-closed if Optware teardown is still busy.
- **Next:** verify exact-head storage/docs Actions. If green, audit NFS export ACL/permission mapping and external mount lifecycle across `nfsd.sh`, `services_stor.c` and RC safe-removal callers.

---

## 2026-11-09 20:15 CST (UTC+8) — Remove obsolete MTD fire-and-forget workflow assertion

- **Base main SHA:** `93406e8618deae765a7c9cf6033ce978e58e230a`; resulting commit: THIS COMMIT.
- **Exact-head Actions:** USB storage [42615049407](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42615049407) passed the Optware/NFS shell checks, lifecycle lock/teardown/startup regressions, GNU89 inventory test, deferred child-result test, and all new MTD worker/result/retry/PID1 source checks. It then failed at the final legacy assertion that still required `doSystem("/usr/bin/opt-mount.sh ... &")`. Samba [42615049566](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42615049566), WAN route [42615049422](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42615049422), HTTPD [42615049699](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42615049699), and docs [42615049390](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42615049390) all SUCCESS on the same full SHA.
- **Correction:** remove the superseded fire-and-forget expectation and retain explicit presence checks for the new tracked `start_rwfs_optware()` and `retry_rwfs_optware()` entrypoints. Production C is unchanged.
- **Changed paths:** `.github/workflows/storage-hotplug-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation boundary:** base workflow's earlier shell/host/GNU89 tests and new MTD assertions passed; only stale final assertion failed. Exact-head storage/docs reruns pending. No full local checkout, production RC object, MIPS/uClibc, firmware/TRX, or hardware test claimed.
- **Next:** verify exact-head storage/docs Actions, then assess persistent-failure retry/backoff and shutdown races.


---

## 2026-11-09 20:08 CST (UTC+8) — Track and retry internal MTD/UBI Optware boot startup

- **Base main SHA:** `19c1627490ec550b74b2deb3e19e8965d4374406`; resulting commit: THIS COMMIT.
- **Recovered exact-head verification:** prior removable USB deferred startup fix is complete: storage [42613569550](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42613569550) and AI handoff docs [42613569474](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42613569474) both SUCCESS on full SHA `19c1627490ec550b74b2deb3e19e8965d4374406`; all 11 storage steps and the handoff/archive validator passed.
- **Finding:** boot-time `start_rwfs_optware()` still used `doSystem("... opt-mount.sh ... &")`, which only reports background launch and discards the actual MTD Optware startup exit status. There was no retry mechanism if the script's later mount/lock/hook path failed.
- **Fix:** retain asynchronous startup but track a worker using a nonblocking pipe; the worker waits for the real `opt-mount.sh` status, writes a one-byte result and signals PID1. Add a retry/poll function and call it from PID1's deferred SIGALRM event, rearming five-second polling while running or after failure. Share the existing Optware child-result decoder, use close-on-exec on the writer, and provide a no-NAND stub. PID1 never waits synchronously for external hooks.
- **Regression:** extend the storage workflow's symmetric push/PR filters and whole-caller-graph source contract to check MTD result reader, worker descriptor flags, fork/exit decoding, result publication, pending-state transitions, retry, and PID1 event-loop caller.
- **Changed paths:** `trunk/user/rc/src/common_ex.c`, `trunk/user/rc/src/init.c`, `trunk/user/rc/src/rc.h`, `.github/workflows/storage-hotplug-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation boundary:** L1 source review PASS. Prior USB storage+docs Actions are green on the exact base. THIS commit's exact-head storage/docs Actions pending. No full local checkout, production RC object, MIPS/uClibc, firmware/TRX, real NAND/UBI or physical-router test claimed.
- **Next:** reconcile exact-head storage/docs workflows; then scrutinize persistent-failure retry cadence and shutdown while the lifecycle lock is held.


---

## 2026-11-09 19:52 CST (UTC+8) — Correct deferred Optware caller-graph assertion anchors

- **Base main SHA:** `a154f30da5d0eac2d3923d221c39eeb8295965f2`; resulting commit: THIS COMMIT.
- **Exact-head evidence:** USB storage run [42612494953](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42612494953) compiled and passed the new GNU89 child-result decoder test, Optware shell regressions, and storage inventory/lock tests, then failed the final source contract at stdin line 129 (the deferred inventory/startup ordering assertion). AI docs [42612494818](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42612494818), Samba [42612494997](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42612494997), vsftpd [42612494935](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42612494935), NFS [42612494823](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42612494823), and HTTPD [42612494815](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42612494815) SUCCESS on the same exact SHA.
- **Correction:** explicitly anchor the inventory assignment, scope the mounted-branch `usb_hotplug_ms` clear search after `start_stor_apps();` (the complete-empty branch legitimately clears it earlier), and find `storage_optware_start_async()` without requiring a standalone semicolon. This is a workflow contract correction only; production code and test coverage are unchanged.
- **Changed paths:** `.github/workflows/storage-hotplug-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation boundary:** base GNU89 child-result test and all earlier shell/storage tests PASS; final source contract failed at the identified assertion. Exact-head storage/docs reruns pending. No full local checkout, full production object, MIPS/uClibc, firmware/TRX, or target-router validation claimed.
- **Next:** reconcile exact-head storage/docs Actions; then audit the separate async MTD boot-time Optware caller result/retry path.


---

## 2026-11-09 19:45 CST (UTC+8) — Preserve deferred USB Optware startup result and retry state

- **Base main SHA:** `6ae43a6a71e05bf5aeba47516b1c4b193c75075e`; resulting commit: THIS COMMIT.
- **Recovered exact-head CI:** the previous MTD/UBI and RWFS source-contract task is complete: storage [42609876845](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42609876845) and AI handoff docs [42609876722](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42609876722) both SUCCESS on the exact full SHA `6ae43a6a71e05bf5aeba47516b1c4b193c75075e`. All storage workflow steps and the documentation/archive validator passed.
- **Finding:** `storage_has_mounted_devices()` conflated incomplete `/proc/mounts` reads with a complete empty inventory. The deferred handler also treated successful background launch as successful Optware startup, cleared both pending NVRAM flags without observing hook exit status, and repeated integrated service startup on retries.
- **Fix:** preserve the inventory's three-state result; keep both pending flags on incomplete inventory; clear the one-shot `usb_hotplug_ms` flag only after integrated services have been started against a complete mounted inventory; run `opt-start.sh` in a single asynchronous worker only after releasing `storage.lock`; deliver its actual exit result through a nonblocking pipe and SIGALRM wakeup; clear `usb_opt_start` only after child success, retain it and retry after child failure/EOF, and suppress duplicate worker launches while the result channel is open.
- **Regression:** add a GNU89 `-Wall -Wextra -Werror` unit test for child-result decoding and source contracts for inventory tri-state, lock release before fork, close-on-exec pipe writer, real wait-status decoding, child result publication, retry flag ordering, and symmetric push/PR path filters. The policy unit test compiled and passed locally in a scratch directory.
- **Changed paths:** `trunk/user/rc/src/services_stor.c`, new `trunk/user/rc/src/storage_optware_start_policy.h`, new `trunk/user/rc/src/tests/storage_optware_start_policy.c`, `.github/workflows/storage-hotplug-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Verification boundary:** L1 source/lock-order review PASS; local GNU89 child-result policy test PASS. Exact-head storage and AI-doc Actions for THIS commit are pending at creation. No full local checkout/full-source build, production RC object, MIPS/uClibc object, firmware/TRX, external hook failure/hang injection, removable-media stress or physical-router test is claimed.
- **Next:** reconcile the new exact-head storage/docs workflows. If green, audit the separate asynchronous boot-time `start_rwfs_optware()` result/retry path in `common_ex.c`.


---

## 2026-11-09 19:03 CST — Correct RWFS failure-gate source-contract search offset

- **Base main SHA:** `d225e9e1e09e56b79c6b00b46e61d2e0a08b6166`; resulting workflow-validation checkpoint: THIS COMMIT.
- **Exact-head Actions:** storage [42609330908](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42609330908) passed NFS/Optware shell syntax, Optware lock/teardown/startup host regressions and GNU89 storage tests, then failed only in the final caller-graph Python contract at line 309. The lookup searched for `if (doSystem(` starting after the matched `doSystem` call, even though the `if` token precedes it. Docs [42609330808](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42609330808) SUCCESS on the same SHA.
- **Correction:** start the `if (doSystem(` lookup at the RWFS function body. The checks still require its failure return to precede RWFS unmount and UBI detach. No production source or tested behavior changed.
- **Changed paths:** `.github/workflows/storage-hotplug-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Verification boundary:** base shell/runtime/GNU89 checks passed; final source contract failed only at this offset. Exact-head Actions pending for THIS commit. No local test/build or target runtime claimed.
- **Next:** reconcile exact-head storage/docs Actions. When green, resume the documented USB deferred Optware child result/retry audit; inspect alarm callback/signal delivery and child ownership before changing NVRAM flag semantics.

---

## 2026-11-09 19:00 CST — Correct MTD branch anchor in storage source contract

- **Base main SHA:** `b13ca6b46e0bef9474ee6744cf4f7f1734aadc97`; resulting validation checkpoint: THIS COMMIT.
- **Exact-head Actions:** storage regression [42608853044](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42608853044) passed `opt-mount.sh` syntax, Optware lock, teardown-status, startup/rollback and GNU89 storage tests, then failed only in the final caller-graph Python contract. The traceback was `ValueError: substring not found` at line 294: the assertion looked for an explicit `if [ -n "$mtd_device" ] ; then`, while the production source represents its MTD branch as the `else` of the removable-device check. AI docs [42608852967](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42608852967) SUCCESS on the same full SHA.
- **Correction:** anchor the MTD-specific source contract to the existing explanatory branch comment. Preserve all checks that the shared lock is sourced/acquired before /opt mount and bind, common supervised hooks run, and the lock is released. No production assertion or lifecycle behavior is weakened.
- **Changed paths:** `.github/workflows/storage-hotplug-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Verification boundary:** this is a test-contract-only correction. Base Actions establish that shell syntax, existing Optware regressions and storage GNU89 tests pass; the new static contract did not reach its new RWFS/MTD assertions. Exact-head storage/docs Actions are pending for THIS commit. No local build/test, production MIPS object, firmware/TRX, UBI runtime or device test is claimed.
- **Next:** reconcile exact-head storage/docs results. If green, continue the deferred USB `opt-start.sh` result/retry redesign in `services_stor.c`; audit async result/retry for the boot-time `start_rwfs_optware()` caller separately.

---

## 2026-11-09 18:57 CST — Serialize internal MTD Optware startup and fail closed on RWFS teardown

- **Base main SHA:** `0ae74382cdeb593770ef609f7685f36d28ee46f8`; resulting implementation/test/docs checkpoint: THIS COMMIT.
- **Exact-head recovery:** before editing, USB storage hotplug regression [42602101323](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42602101323) and AI handoff documentation contract [42602101300](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42602101300) were both SUCCESS on the exact full base SHA. The remote branch list contained only `main`.
- **Module / priority:** USB storage / internal NAND RWFS Optware lifecycle, P2 stability.
- **Caller trace:** `init_router()` calls `start_rwfs_optware()` in `common_ex.c`, which launches `opt-mount.sh /dev/ubiN_0 /media/mtd_rwfs` asynchronously and does not hold `storage.lock`. The normal removable-storage caller reaches `opt-mount.sh` from `automount.sh` while mdev owns `storage.lock`, but that non-MTD branch only records `usb_opt_start` and returns. The internal MTD/UBI branch formerly bound `/opt` and ran hooks through an unquoted `ls` command-substitution loop, bypassing the shared lifecycle lock and rollback helper. On shutdown, `umount_rwfs_partition()` ignored `opt-umount.sh` failure and continued with RWFS unmount/UBI detach even though the teardown script deliberately returns failure when startup owns the lifecycle lock.
- **Fix:** MTD/UBI `opt-mount.sh` now acquires the shared lifecycle lock before the `/opt` mount check and holds it across bind setup and supervised service startup. It calls `optware_lifecycle_start_services()`, propagates hook/rollback failure, and releases the lock on every explicit early/success/failure exit. The removable-storage branch retains its deferred-start behavior and does not take this lock while the caller owns `storage.lock`. `umount_rwfs_partition()` now aborts before unmount/detach when the Optware stop hook fails; a real RWFS unmount error also keeps UBI attached, and detach failures are logged.
- **Changed paths:** `trunk/user/optware/opt-mount.sh`, `trunk/user/rc/src/common_ex.c`, `.github/workflows/storage-hotplug-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Regression:** extend the storage workflow to run `sh -n opt-mount.sh` and assert the MTD caller takes the lifecycle lock before binding, uses the common rollback supervisor instead of the direct loop, and that shutdown gates both RWFS unmount and UBI detach on teardown/unmount status. Exact-head Actions are pending at commit creation; no local test/build is claimed.
- **Verification boundary:** L1 source/caller and lock-order review PASS. L2 storage/AI-doc Actions PENDING on the resulting exact SHA. No MIPS/uClibc production object, firmware/TRX, live UBI filesystem, external init-hook failure injection or physical-router test. The C boot caller still launches MTD startup asynchronously and does not consume the script exit status; the deferred USB child result/retry contract is also open.
- **Next:** reconcile exact-head storage/docs Actions. Then fix `try_start_stor_apps()` / `on_deferred_hotplug_dev()` so background `opt-start.sh` failure does not lose `usb_opt_start`/hotplug retry state, without executing external hooks under `storage.lock` or starting duplicate children while the lifecycle lock is busy. Preserve pending work when mount inventory is incomplete, and do not repeat integrated service startup on every retry. Separately assess retry/error reporting for asynchronous `start_rwfs_optware()`.

---

## 2026-11-09 18:36 CST (UTC+8) — Align storage source contract with shared Optware startup helper

- **Base main SHA:** `57aee4fd406d9e4c06f3ee113742ba5b6999fc0d`; resulting commit: THIS COMMIT.
- **Exact-head evidence:** startup regression, teardown propagation, lifecycle-lock regression, NFS shell syntax and GNU89 storage inventory/operation-lock tests all SUCCESS on `57aee4fd406d9e4c06f3ee113742ba5b6999fc0d`. AI handoff docs [42601392329](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42601392329) SUCCESS on that exact SHA. Storage [42601392256](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42601392256) failed only in its final whole-caller-graph Python contract.
- **Finding:** the source contract still searched for the old inline `start 9>&-` command inside `opt-start.sh`. The production loop had intentionally moved into `optware_lifecycle_start_services()` in the shared helper, so the assertion raised `ValueError: substring not found` before the remaining caller graph assertions ran.
- **Correction:** assert lock acquisition → /opt mount recheck → shared startup supervisor → lifecycle lock release in `opt-start.sh`; separately require hook invocation with fd 9 closed inside the shared helper. No production code behavior changed.
- **Changed paths:** `.github/workflows/storage-hotplug-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation:** exact-head job logs inspected. Fresh storage and docs Actions are pending on THIS COMMIT; no claim of green current-head workflow yet.
- **Validation boundary:** L1 production logic unchanged from prior source review; L2 prior host regressions pass but the storage caller-graph contract has just been corrected and awaits exact-head rerun. No MIPS/uClibc production object, firmware/TRX or hardware validation.
- **Next:** reconcile the corrected storage/docs Actions on the resulting exact SHA. Continue analysis of the Optware retry contract: `try_start_stor_apps()` launches `opt-start.sh` in the background and clears `usb_opt_start` on successful launch without observing the child's exit status. Determine a non-blocking, lock-order-safe way to surface startup completion/failure and schedule retries before taking the separate `opt-mount.sh` MTD/UBI startup bypass.

---

## 2026-11-09 18:30 CST (UTC+8) — Correct Optware startup regression and handoff contract

- **Base main SHA:** `7005e6c48835b928d596f4a52f79ca9168a52994`; resulting commit: THIS COMMIT.
- **Recovered exact-head failures:** storage-hotplug [42600941876](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42600941876) passed the existing lock and teardown regressions, then failed the new startup fixture. Source review found the fixture's printf formats were double-escaped, leaving literal backslash-n text and a trailing event marker; normalize them to actual newline escapes. AI handoff docs [42600941917](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42600941917) failed because the live handoff used `## Active checkpoint` instead of the validator-required `## Active task`.
- **Correction:** correct all startup fixture printf escapes and retain the exact mandatory handoff heading. No production implementation change; the startup supervisor remains the behavior introduced by base commit `7005e6c`.
- **Validation:** corrected startup fixture and shared lifecycle helper `sh -n` and focused host run PASS in the scratch directory. The prior storage job's first three shell regressions were green; its final whole-caller-graph step was skipped after the startup fixture failed. The docs failure occurred at the heading contract before archive checks. New exact-head storage/docs Actions pending.
- **Changed paths:** `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`, `trunk/user/scripts/tests/optware_lifecycle_startup.sh`.
- **Validation boundary:** L1 source review unchanged; L2 focused host regression PASS, corrected exact-head Actions pending. No production MIPS/uClibc object, firmware/TRX or router validation.
- **Next:** reconcile corrected storage/docs Actions on the resulting exact SHA. If green, investigate the separate `opt-mount.sh` internal MTD/UBI direct service-start loop and its caller/lock context; preserve NFS ACL/permissions, Samba and vsftpd audits, DHCP callback semantics and FULLCONENAT namespace ownership as open work.

---

## 2026-11-09 18:26 CST (UTC+8) — Supervise Optware startup hooks and roll back partial starts

- **Base main SHA:** `8e818c475d3aa26fb4d92e255e25405c11f2a08f`; resulting commit: THIS COMMIT.
- **Recovered exact-head CI:** NFS daemon start/reload [42598774070](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42598774070), USB storage hotplug [42598773937](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42598773937), and AI handoff docs [42598774002](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42598774002) all completed SUCCESS on exact `8e818c475d3aa26fb4d92e255e25405c11f2a08f`. The NFS follow-up is therefore complete at source/CI level and is not repeated.
- **Finding:** `opt-start.sh` enumerated third-party `S??*` hooks through `ls` plus command substitution, then executed unquoted path words. Paths containing whitespace were split, hook exit statuses were discarded, startup proceeded after failure, and the final lock close could leave the script reporting success. A partially-started service set had no rollback.
- **Fix:** add `optware_lifecycle_start_services()` to the shared Optware lifecycle helper. It uses quoted pathname expansion, skips non-regular/non-executable entries, starts hooks in order, and on the first failure calls `stop` on that possibly partially-started hook and all prior successful hooks in reverse order. Rollback continues after individual stop failures and the helper remains failed. Each child closes fd 9 so external hooks cannot inherit the lifecycle lock. `opt-start.sh` now aborts before package-manager bootstrap when service startup fails, releases the lock and exits nonzero; normal completion explicitly exits zero after release.
- **Regression:** new `optware_lifecycle_startup.sh` tests ordered startup, filenames with spaces, non-executable filtering, closed lock-fd inheritance, empty/missing init directories, failure of a start hook, rollback order, and continued cleanup after a rollback stop failure. The storage-hotplug workflow runs this test and checks the source/caller contract on push and pull_request.
- **Changed paths:** `.github/workflows/storage-hotplug-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`, `trunk/user/optware/opt-start.sh`, `trunk/user/scripts/files/sbin/optware_lifecycle_lock.sh`, new `trunk/user/scripts/tests/optware_lifecycle_startup.sh`.
- **Local validation:** `sh -n` passed for the changed helper, startup script and regression; the focused shell regression passed in a host scratch directory. The regression is not a target-filesystem or real third-party init-script integration test.
- **Exact-head Actions:** storage-hotplug and AI handoff documentation runs for THIS COMMIT are pending at commit creation; fetch their run IDs, conclusions and complete `head_sha` before treating L2 CI as green.
- **Validation boundary:** L1 source/caller review PASS; L2 focused host shell regression PASS, exact-head workflows pending. No MIPS/uClibc production object, firmware/TRX, live Optware package/service integration, storage contention stress or router hardware test.
- **Next:** reconcile storage/docs Actions against the new full main SHA. If green, audit the duplicate `opt-mount.sh` internal MTD/UBI startup loop (which still bypasses this supervisor), including lock ordering and failure propagation, without introducing a storage-lock/lifecycle-lock inversion. Keep NFS export ACL/permission mapping, Samba account/ACL/credentials, vsftpd authentication/path confinement, DHCP release/NAK semantics and FULLCONENAT namespace ownership open.

---

## 2026-11-09 18:06 CST (UTC+8) — Correct NFS start contract and roll back pre-daemon failures

- **Base main SHA:** 'a816159737241f398baa7f827d753f7b99e8131b'; resulting commit: THIS COMMIT.
- **Exact-head CI:** storage [42597871309](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42597871309), HTTPD [42597871334](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42597871334), Samba [42597871360](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42597871360), vsftpd [42597871344](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42597871344) and AI docs [42597871472](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42597871472) all SUCCESS on a816159. NFS [42597871349](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42597871349) passed shell syntax and every runtime mock; only its Python source contract failed because it expected numeric comparisons while the helper uses a case statement.
- **Correction:** align source contract with the explicit 0 / 1 / unknown case labels; do not change the production implementation to satisfy a stale assertion.
- **Implementation:** failed mounting either NFS control pseudo-filesystem and failed export publication now invoke checked func_stop cleanup. Tests verify successful rollback removes previously mounted control filesystems and etab; if export withdrawal fails, etab and pseudo-mounts remain as recovery evidence.
- **Changed paths:** .github/workflows/nfsd-start-stability.yml, SESSION_HANDOFF.md, REFACTOR_ROADMAP.md, dev_log.md, trunk/user/nfs-utils/nfsd.sh, trunk/user/nfs-utils/tests/nfsd_start_lifecycle.sh.
- **Validation:** the previous exact-head shell syntax and runtime mocks PASS; this follow-up's source-contract correction and added rollback assertions require fresh exact-head Actions. No local full checkout, docs validator, production MIPS/uClibc object, firmware/TRX, live NFS or router testing is claimed.
- **Validation boundary:** L1 source review PASS; L2 this follow-up pending exact-head NFS/HTTPD/storage/docs Actions. L3/L4 NOT VERIFIED.
- **Next:** reconcile exact-head Actions. If green, rotate to Optware opt-start.sh startup-hook enumeration, status aggregation, partial-start rollback, lock/fd ownership and retry semantics; keep NFS ACL/permissions and external mount lifecycle open.

---

## 2026-11-09 17:56 CST (UTC+8) — Fail closed on NFS start and reload errors

- **Base main SHA:** `d143581bf5f351cb2cf0d388f2542c3710802c7b`; resulting commit: THIS COMMIT.
- **Recovered CI:** HTTPD [42596369094](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42596369094) SUCCESS on exact base SHA d143581. The predecessor HTTPD run [42596117899](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42596117899) had failed on exact SHA `9dd2f4a6eed24103617023fc93ea058f51466a0a` because its NFS restart assertion still expected the old void signature; d143581 corrected the workflow contract to the int status/unknown-inventory guard. Storage [42596118456](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42596118456), Samba [42596117948](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42596117948), vsftpd [42596117585](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42596117585), WAN route [42596117615](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42596117615) and AI docs [42596117462](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42596117462) all SUCCESS on exact source SHA 9dd2f4a6. AI docs [42596369073](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42596369073) remained queued on d143581 at base inspection.
- **Finding:** `func_start()` ignored rpcbind, state-file/directory, module/mount and `exportfs -r` failures. The final `$?` check tested only `rpc.mountd`, so a failing `rpc.nfsd` could be masked by a later successful mountd command. A mountd failure could leave a partially started kernel server. The already-running and `func_reload()` branches could also report success when reload prerequisites were absent or export reload failed. C `run_nfsd()` and `reload_nfsd()` discarded `eval()` results.
- **Implementation:** check rpcbind start and process presence; refuse to reset export recovery files if only one NFS daemon is alive; check state creation, `/proc/filesystems`, NFS control and rpc_pipefs mounts, exports publication and each RPC start. A tri-state mount helper treats an unreadable `/proc/mounts` as an error instead of an absent mount. Verify both daemons exist after successful commands; roll back a partial kernel-server start with the existing checked `func_stop()` path and retain export evidence if cleanup fails. Keep max-block-size tuning best-effort. Reload now requires rpcbind, both daemons and `/etc/exports`, and returns the real `exportfs` failure. C start/reload wrappers log nonzero command results.
- **Regression:** add a mock-based shell test for rpcbind/module/mount/exportfs/RPC failures, process false-success, partial daemon state preservation, startup rollback, start ordering and reload prerequisites/results. Add a path-filtered NFS start/reload workflow covering both push and pull_request, shell syntax, runtime mocks and C caller source contracts.
- **Changed paths:** `.github/workflows/nfsd-start-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`, new `trunk/user/nfs-utils/tests/nfsd_start_lifecycle.sh`, `trunk/user/nfs-utils/nfsd.sh`, `trunk/user/rc/src/services_stor.c`.
- **Validation:** source/caller order and failure gates reviewed against the exact base blobs. No full local repository checkout is available; no local docs-validator, full production object or MIPS build is claimed. Exact-head NFS, HTTPD, storage and docs Actions are pending for THIS COMMIT.
- **Validation boundary:** L1 focused source/caller review PASS; L2 exact-head regression and Actions pending. No production RC object/MIPS-uClibc, firmware/TRX, live NFS export/reload/rollback or physical-router test.
- **Next:** reconcile all exact-head NFS/HTTPD/storage/docs Actions. If green, rotate to Optware `opt-start.sh` service-hook enumeration and start-result aggregation; preserve NFS export ACL/permissions and external mount lifecycle as open audits.

---

## 2026-11-09 — Propagate NFS stop failures before storage detach

- **Base main SHA:** `656e9819f1de186767738a6d49aedbd171f26ff0`; resulting commit: THIS COMMIT.
- **Recovered baseline:** remote branches contained only `main`. HTTPD/security [42594767916](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42594767916) and AI handoff docs [42594767915](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42594767915) passed on exact SHA `656e9819f1de186767738a6d49aedbd171f26ff0`. Prior NFS mount-inventory/restart-preflight work is complete and not repeated.
- **Finding:** NFS stop previously ignored daemon termination, `exportfs -ua` and pseudo-filesystem unmount results. `etab` removal could erase recovery evidence after an incomplete stop. C `stop_nfsd()` dropped the shell result and storage eject continued to unmount the device.
- **Implementation:** wait for both `rpc.mountd` and `nfsd`, escalate only after a bounded graceful window, verify exit, check export withdrawal, and use tri-state `/proc/mounts` handling (absent / present / unreadable) for `rpc_pipefs` and `nfsd`. Retain `etab` on failure. Gate shell restart on successful stop; propagate return status through C stop/restart APIs and RC applets/notifications; abort per-device and eject-all removal before unmount while freeing staged data and releasing the storage lock.
- **Regression coverage:** storage-hotplug workflow now watches `nfsd.sh` and `rc.h`, runs `sh -n`, and checks shell/C/RC propagation plus both safe-eject abort paths and lock cleanup.
- **Changed paths:** `trunk/user/nfs-utils/nfsd.sh`, `trunk/user/rc/src/services_stor.c`, `trunk/user/rc/src/rc.h`, `trunk/user/rc/src/rc.c`, `.github/workflows/storage-hotplug-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation:** source transformations and regression-contract invariants checked against the exact base blobs before creating Git objects. No local checkout is available; no local shell syntax, compiler, Python workflow test or docs-validator run is claimed. Exact-head storage/docs Actions are pending.
- **Validation boundary:** L1 bounded source review PASS; L2 for this commit PENDING exact-head Actions. No production MIPS/uClibc object, firmware/TRX, live NFS export/stop, USB detach or physical router test.
- **Next:** reconcile exact-head storage/docs Actions. If green, audit NFS start/reload partial failure and cleanup, then rotate to arbitrary Optware/third-party service hooks and init ordering. Keep Samba account/ACL, vsftpd confinement, DHCP callback state and the separate FULLCONENAT kernel namespace audit open.

---
## 2026-11-09 17:35 CST (UTC+8) — Preserve NFS service when mount inventory is incomplete

- **Base main SHA:** `d5cd264e5fda1568d8c1872bac09c0a4ec0da233`; resulting commit: THIS COMMIT.
- **Module / priority:** NFS storage-service restart lifecycle, P2 stability. NFS and third-party add-ons remain PARTIAL / AUDITING.
- **Finding:** `restart_nfsd()` stopped the NFS daemon before calling `count_stor_mountpoint()`. If `/proc/mounts` open/read/close failed, the function returned on the unknown (`-1`) inventory, leaving the previously running NFS service stopped.
- **Fix:** add `storage_mount_inventory_is_complete()` to the tri-state mount-inventory policy and use it in the shared storage-service preflight. Move `stop_nfsd()` after the NFS restart's completeness guard. A failed inventory now preserves current daemon/module state; complete inventories continue through the existing restart/unload decision.
- **Regression:** extend the GNU89 policy test for incomplete, empty and non-empty inventory states. Strengthen the HTTPD workflow source contract to assert inventory read → incomplete guard/return → daemon stop ordering. Existing path filters already cover the source, helper and regression paths.
- **Changed paths:** `trunk/user/rc/src/services_stor.c`, `trunk/user/rc/src/storage_mount_inventory_policy.h`, `trunk/user/rc/src/tests/storage_mount_inventory_policy.c`, `.github/workflows/httpd-security.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Baseline exact-head evidence:** HTTPD [42591187385](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42591187385) and AI handoff docs [42591187335](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42591187335) SUCCESS on `d5cd264e5fda1568d8c1872bac09c0a4ec0da233`. This commit's host policy test, source contract and docs validation await exact-head Actions.
- **Local validation:** no local checkout is available; no local GNU89 test, workflow Python contract or `scripts/check_ai_handoff_docs.py` run is claimed.
- **Validation boundary:** L1 focused source/caller review PASS; L2 pending exact-head Actions. No production RC object, MIPS/uClibc, firmware/TRX or live NFS mount/restart/router validation. The separate `nfsd.sh` command-level start/stop/reload status audit and Optware startup-hook aggregation remain open.
- **Next:** reconcile HTTPD/docs Actions against the resulting full SHA. Then audit `trunk/user/nfs-utils/nfsd.sh` start/stop/reload error propagation as one bounded lifecycle task, followed by third-party `opt-start.sh` start-hook result handling.

---

## 2026-11-09 17:09 CST (UTC+8) — Bound Samba folder contract to the complete writer body

- **Base main SHA:** `301de504e474f51f4f04c08ef7f36975f19df9fa`; resulting commit: THIS COMMIT.
- **Exact-head results:** HTTPD [42590546450](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42590546450) FAILED at step 26, line 62 of its Samba trust-boundary Python contract, with `ValueError: substring not found`. Investigation showed the slice stopped at the first free_2_dimension_list(&sh_num, &folder_list); inside an early close-error branch, before the share output. The old check therefore looked for output outside its own truncated slice. Docs [42590546420](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42590546420) PASS. The C test remains unchanged and passed on exact SHA `088a87e61ca0da3ab040fc230f2d9e875b3920a1`.
- **Correction:** find the complete write_smb_conf() range, then bound the folder-loop contract from its marker through the final folder-list cleanup in that writer. This keeps the share-name/path guard-before-output assertions over the whole relevant block.
- **Changed paths:** `.github/workflows/httpd-security.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation:** exact source and job-log inspection; the corrected HTTPD/docs workflows are pending on THIS COMMIT. No local checkout, local Python workflow run or docs-validator run is available/claimed.
- **Validation boundary:** no production C implementation changed. Samba GNU89 host test PASS on SHA 088a87e; HTTPD trust-boundary contract not green yet. No MIPS/uClibc, firmware/TRX, live SMB share/auth/ACL or hardware test.
- **Next:** reconcile HTTPD and docs Actions against the resulting full main SHA. If green, rotate to NFS mount/permission lifecycle and third-party add-on service supervision; keep Samba account/ACL and FTP authentication/path-confinement audits open.

---

## 2026-11-09 17:04 CST (UTC+8) — Remove brittle Samba contract slice from HTTPD regression

- **Base main SHA:** `088a87e61ca0da3ab040fc230f2d9e875b3920a1`; resulting commit: THIS COMMIT.
- **Exact-head results:** Samba share stability [42589984483](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42589984483) and AI handoff docs [42589984462](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42589984462) both SUCCESS on the exact base SHA. HTTPD [42589984431](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42589984431) still FAILED at the Samba trust-boundary step: its `config_header` slice raised `ValueError: substring not found` after the function signature migration. Earlier HTTPD policy/regression steps passed.
- **Correction:** remove the unnecessary end-marker slice. Anchor at `write_smb_conf_header(FILE *fp)`, then compare the workgroup and hostname sanitization guards directly with their respective `fprintf()` calls. Preserve both checks and their ordering requirement.
- **Changed paths:** `.github/workflows/httpd-security.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation:** root cause confirmed from exact-head job 129069003316 logs and current `services_ex.c`. No local repository checkout; no local execution of the Python contract or docs validator. Updated HTTPD and docs Actions are pending on THIS COMMIT.
- **Validation boundary:** no production C implementation changes. Samba GNU89 host regression PASS on exact SHA `088a87e61ca0da3ab040fc230f2d9e875b3920a1`; exact-head HTTPD contract is not yet green. No MIPS/uClibc, firmware/TRX, live SMB share or hardware validation.
- **Next:** reconcile HTTPD and AI handoff documentation Actions against the resulting full main SHA. If green, rotate to NFS mount/permission lifecycle and third-party add-on service supervision; keep Samba account/ACL and FTP authentication/path-confinement audits open.

---

## 2026-11-09 16:53 CST (UTC+8) — Repair Samba exact-head regression and dependent HTTPD contract

- **Base main SHA:** `724b4c9982cba13c3c7274ba4744f88fb5799835`; resulting commit: THIS COMMIT.
- **Recovered failures:** exact-head Samba workflow [42587965407](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42587965407) failed at GNU89 test compilation because `tests/samba_config_file.c:95` contained a double-escaped character constant (`'\\0'`). The exact-head HTTPD workflow [42587965159](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42587965159) failed in its Samba trust-boundary source contract: it still required the old `write_smb_conf_header(void)` signature and direct live-file append/close/return patterns after the implementation moved to a stream-based header and atomic staging.
- **Correction:** normalize the discard test's newline/NUL literals to valid single C escapes. Update HTTPD's Samba contract to require `write_smb_conf_header(FILE *fp)`, same-directory staging, staged-file appends/duplicate checks, checked close, discard on generation failure and publication only after complete generation; explicitly reject direct append to `SAMBA_CONF`. Preserve the existing permission/account/security assertions.
- **Other exact-head evidence:** vsftpd [42587965266](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42587965266), storage [42587965191](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42587965191), WAN route [42587965174](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42587965174) and AI handoff docs [42587965152](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42587965152) all passed on the exact base SHA.
- **Changed paths:** `.github/workflows/httpd-security.yml`, `trunk/user/rc/src/tests/samba_config_file.c`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation:** root-cause review is based on the exact Actions job logs and current source blobs. No local repository checkout is available; local GNU89 test, workflow Python contracts and docs validator were NOT RUN in this round. Corrected Samba/HTTPD exact-head Actions are pending after this commit.
- **Validation boundary:** no new production C implementation was changed. The atomic Samba publisher remains L1-reviewed but its corrected regression still requires L2 confirmation. No production RC/Samba object, MIPS/uClibc build, firmware/TRX, live SMB share/auth/ACL or hardware test.
- **Next:** fetch live main/all branches and verify the corrected Samba and HTTPD Actions against the resulting exact SHA. If both pass, rotate to NFS mount/permission lifecycle and third-party add-on service supervision; keep Samba account/ACL and FTP authentication/path-confinement audits open.

---

## 2026-11-09 — Stage and atomically publish generated Samba configuration

- **Base main SHA:** `c51c0f73fa2dccdd191515347acf80fd2a0132d2`; resulting source/test checkpoint: THIS COMMIT.
- **Module / priority:** Samba share configuration and service lifecycle, P2 stability with security implications. Samba remains PARTIAL / AUDITING.
- **Findings:** `write_smb_conf_header()` unlinked `SAMBA_CONF` before opening it and wrote the global section directly to the live file. `write_smb_conf()` subsequently wrote share sections to the same live file, closed/reopened it for duplicate-share checks, and could leave a partial/truncated config on later write or read errors. The folder-list reader's failure was treated as `continue`, silently omitting shares. `start_wins()` started nmbd even when header write/close failed; `restart_nmbd()` ignored `write_smb_conf()` failure and stopped nmbd before checking whether a replacement could be generated.
- **Fix:** add `samba_config_file.h` to stage a same-directory candidate with `mkstemp()`, mode 0644, checked stream/flush/close and atomic `rename()`. The active `smb.conf` is left intact on all pre-publication failures. Refactor `write_smb_conf_header(FILE *)` into a stream-only emitter; write all share definitions and duplicate checks against the staged file; match exact section names; treat folder inventory/read/append failures as fatal and clean up the account/folder lists; publish only after complete generation. `start_wins()` now publishes before nmbd launch. `restart_nmbd()` generates the new config before stopping nmbd, and skips both nmbd restart and smbd HUP if generation fails.
- **Regression:** add `tests/samba_config_file.c` for old-file visibility before rename, successful replacement/mode 0644, buffered `/dev/full` ENOSPC handling, rename failure cleanup, explicit discard, missing-parent failure and short-path-buffer failure. New `.github/workflows/samba-share-stability.yml` compiles with `-std=gnu89 -Wall -Wextra -Werror` and checks the generation, share scan, WINS and restart failure gates.
- **Changed paths:** `.github/workflows/samba-share-stability.yml`, `trunk/user/rc/src/services_stor.c`, `trunk/user/rc/src/services_ex.c`, `trunk/user/rc/src/rc.h`, new `trunk/user/rc/src/samba_config_file.h`, new `trunk/user/rc/src/tests/samba_config_file.c`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Base exact-head Actions:** vsftpd [42586171489](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42586171489), storage [42586171713](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42586171713), HTTPD [42586171537](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42586171537) and AI handoff docs [42586171552](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42586171552) all completed SUCCESS with exact `head_sha=c51c0f73fa2dccdd191515347acf80fd2a0132d2`.
- **Local validation:** the new Samba config helper and host test were compiled using `cc -std=gnu89 -Wall -Wextra -Werror` and executed in a scratch directory: **PASS**. This exercised successful publication, prior-file preservation before rename, ENOSPC, rename failure, explicit discard and invalid-path cases. Source-contract assertions were evaluated against the staged source blobs. No complete local repository checkout was available, so the GitHub Python source contract and `scripts/check_ai_handoff_docs.py` were not run locally.
- **Exact-head Actions:** the new Samba share workflow and FTP/storage/HTTPD/docs workflow runs for THIS COMMIT are PENDING at commit creation; reconcile every run against the resulting full `head_sha` and inspect failed steps.
- **Validation boundary:** L1 targeted source/caller review PASS; L2 standalone GNU89 host helper test PASS, exact-head Actions pending. No production RC/Samba object build, MIPS/uClibc, firmware/TRX, real SMB mount/auth/ACL test, external add-on integration or physical-router validation.
- **Next:** verify all exact-head workflows, then rotate to NFS mount/permission lifecycle and third-party add-on service supervision. Keep Samba passdb/account and ACL audits plus broader FTP authentication/path-confinement work open; preserve DHCP callback semantics and FULLCONENAT namespace work as separate tasks.

---
## 2026-11-09 16:27 CST (UTC+8) — Correct vsftpd configuration error return

- **Base main SHA:** `2e27361f396adfe79017b1d815962524af47d097`; resulting commit: THIS COMMIT.
- **Review correction:** after the atomic-publication commit passed its exact-head vsftpd, storage, HTTPD and documentation workflows, a final source reread found that the new int-valued write_vsftpd_conf() used a bare return after a failed ftp_config_file_publish(). That is an invalid return contract and may trigger a compiler diagnostic.
- **Fix:** return -1 on publication failure; preserve return 0 only for a completed publication. Extend the Python source contract to require the publication failure return to precede the success return.
- **Prior exact-head evidence:** vsftpd 42585929021, storage 42585929025, HTTPD 42585929042 and AI handoff docs 42585929052 all completed SUCCESS on 2e27361f396adfe79017b1d815962524af47d097. THIS COMMIT's vsftpd and docs workflows are PENDING.
- **Validation boundary:** the previous commit's GNU89 test passed; this follow-up is a small RC source-return correction and CI contract addition. No full production object/MIPS/uClibc build or hardware testing.
- **Next:** reconcile THIS COMMIT's exact-head checks, then continue the Samba/accounts/NFS/add-on reliability rotation. The wider FTP credential/ACL/path-confinement audit remains open.

## 2026-11-09 16:25 CST (UTC+8) — Atomically publish generated vsftpd configuration

- **Base main SHA:** `85591fa3ea404728bc47a24c4d018148fa974295`; resulting commit: THIS COMMIT.
- **Module / priority:** vsftpd generated configuration and service startup, P2 stability with security implications; broader FTP audit remains AUDITING.
- **Finding:** write_vsftpd_conf() opened /etc/vsftpd.conf directly with "w", truncating the last working configuration before generation completed. It ignored all fprintf()/flush/close failures and returned no status; run_ftp() launched vsftpd regardless. A full/read-only filesystem or late write/close failure could leave the daemon reading a partial or empty configuration.
- **Fix:** add ftp_config_file.h to create a same-directory mkstemp() staging file, set its mode, and publish only after stream-error, flush and close checks succeed. A single rename() atomically replaces the destination; failure discards the staged file and retains the previous config. write_vsftpd_conf() now returns status and run_ftp() logs and skips daemon startup when publication fails.
- **Changed paths:** .github/workflows/vsftpd-stability.yml, SESSION_HANDOFF.md, REFACTOR_ROADMAP.md, dev_log.md, trunk/user/rc/src/services_stor.c, new trunk/user/rc/src/ftp_config_file.h, and new trunk/user/rc/src/tests/ftp_config_file.c.
- **Regression:** add GNU89 -Wall -Wextra -Werror host coverage for successful atomic replacement/file mode, buffered write failure injected with /dev/full, failed staging open, pathname truncation and preservation of last-known-good config. Workflow source contracts check publication ordering, required output/close checks, daemon-start gating and complete push/PR path filters. No local checkout was available, so the C test and Python source contract were not run locally; exact-head Actions are PENDING.
- **Validation boundary:** L1 selected caller/config control-flow review PASS. No production RC/vsftpd object build, MIPS/uClibc, firmware/TRX, real FTP login/transfer or hardware validation.
- **Next:** reconcile the vsftpd stability and AI handoff Actions on THIS COMMIT's exact full head_sha; then rotate to Samba/accounts/NFS/add-on service reliability/security. Keep the broader FTP credential/ACL/path audit open.

## 2026-11-09 — Extend USB storage hotplug caller-graph regression

- **Base main SHA:** `2b3a2d19d76399199d8d813fe96260b0a0910368`; resulting source/test checkpoint: THIS COMMIT.
- **Scope:** P2 storage stability audit follow-through. This is a CI/source-contract change; no production C or shell code was changed.
- **Reviewed graph:** SD/MMC mdev dispatch and per-device/shared operation locks; synchronous automount; automount mountpoint ownership lock and fd-8 child-close discipline; filesystem swap activation; Optware bind and swap setup; exact swap state and failure recheck; complete `/proc/swaps` + `/proc/mounts` snapshots before global cleanup side effects; swapoff-before-unmount; safe-removal and eject-all error gates; disk spindown only after successful unmount and a second mounted-source guard; deferred startup retry; RC mdev/notification/eject/firmware/shutdown entry points.
- **Regression:** expanded `.github/workflows/storage-hotplug-stability.yml` source contracts, added production/helper path filters for push and pull_request, and renamed the step to reflect whole caller-graph coverage. Existing Optware lock/teardown and storage C GNU89 host tests remain in the same workflow.
- **Review result:** source/caller and lock-order review completed. The new inline Python assertion block could not be run locally because this session has no repository checkout; its execution is delegated to the exact-head Actions workflow. Existing shell and C regressions had passed on the immediately preceding checkpoint `2b3a2d19d76399199d8d813fe96260b0a0910368`.
- **Actions rule:** query the storage-hotplug and AI handoff-documentation runs generated by THIS COMMIT and match their complete `head_sha`; never reuse results from the preceding checkpoint. Do not add a docs-only follow-up just to restate a green result.
- **Validation boundary:** no production RC/libdisk object build, MIPS/uClibc build, firmware/TRX, real removable-media stress, external Optware script integration, filesystem failure injection or physical-router testing.
- **Next:** rotate to `vsftpd` service/config/auth/path-confinement lifecycle audit and choose one bounded reliability/security defect. Keep DHCP leasefail/accepted-NAK/release semantics and FULLCONENAT namespace work separate.

---

## 2026-11-09 15:18 CST (UTC+8) — Propagate Optware stop-hook and /opt unmount failures

- **Base main SHA:** `70415484275e9c542be3d45b8bad807fb6a09591`; resulting source/test checkpoint: THIS COMMIT.
- **Module / priority:** USB storage / Optware safe teardown, P2 stability. Storage remains PARTIAL / AUDITING.
- **Finding:** `opt-umount.sh` ignored every executable `S??*` stop-hook result, ignored the second `umount /opt` result, and then released fd 9, potentially returning success despite incomplete teardown. The storage C callers already checked `eval()` status, but the shared macro's `_eval()` status contract was not enforced in the storage workflow.
- **Fix:** add shared helper functions that attempt all executable init scripts in reverse order and aggregate failures; empty init.d is valid, while failed script enumeration fails closed. Any stop failure releases the lifecycle lock and exits nonzero before swapoff/unmount. The `/opt` helper retries once and returns nonzero if both attempts fail; `opt-umount.sh` explicitly propagates failure after lock release. The nonblocking teardown try-lock and blocking startup lock remain unchanged.
- **Caller proof:** `eval()` in `trunk/user/shared/src/shutils.h` returns `_eval()`'s command status/errno. Both `services_stor.c` call sites check `hook_status` and skip the matching mount unmount on failure; no unnecessary C production edit was made.
- **Changed paths:** `.github/workflows/storage-hotplug-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`, `trunk/user/optware/opt-umount.sh`, `trunk/user/scripts/files/sbin/optware_lifecycle_lock.sh`, and new `trunk/user/scripts/tests/optware_lifecycle_teardown.sh`.
- **Regression:** mock-based shell test covers reverse-order full traversal after a stop failure, all-success, missing and empty service directories, fail-closed inventory enumeration, immediate unmount success, retry success and two-attempt failure. Local `sh` regression: **PASS**. `sh -n` on the changed shell scripts: **PASS**. Storage workflow source contracts now verify both C callers gate their unmount on `eval()` status and the new production helper wiring.
- **Actions:** exact-head USB storage regression and AI handoff documentation contract are **PENDING** at commit creation; reconcile both run IDs, conclusions and full `head_sha` against the resulting `main` commit.
- **Validation boundary:** L1 selected source/caller review PASS; L2 focused host shell regression PASS locally, Actions pending. No production RC object, MIPS/uClibc, firmware/TRX, real Optware scripts, filesystem failure injection or physical-router validation.
- **Next:** reconcile exact-head storage/docs Actions, then finish the drafted whole in-tree storage caller-graph regression. After that bounded audit, rotate to another userland service area. Keep DHCP leasefail/NAK/release semantics and FULLCONENAT namespace work separate.

---
## 2026-11-09 15:04 CST (UTC+8) — Make AP-mode LAN DHCP bound updates transactional

- **Base main SHA:** `2f4942be4a1068e72982475166b178add129981c`; resulting commit: THIS COMMIT.
- **Module / priority:** LAN DHCP client and resolver lifecycle, P1/P2 stability; module remains PARTIAL / AUDITING.
- **Finding:** the bound/renew callback published temporary NVRAM lease identity before checking interface address changes, ignored default-route addition errors, and rewrote `/etc/resolv.conf` in place without the resolver lock or checked atomic publication. BusyBox's `udhcp_run_script()` synchronously waits for the callback but discards its exit status, so an error return alone cannot keep RC state and the kernel on the previous lease.
- **Fix:** add a host-testable DHCP bound transaction policy. It deletes a replaced old route, applies the address and new route, then atomically publishes the resolver before committing temporary lease fields. Any failure attempts all compensating operations, restores the previous address/route where they were actually changed, leaves temporary lease state untouched, and records the first rollback failure separately. Route-add `EEXIST` is treated as already-present without claiming ownership for rollback. Resolver generation now uses the shared `resolv` lock and `dnsmasq_file_open/publish` staging path.
- **Changed paths:** `trunk/user/rc/src/net_lan.c`, `trunk/user/rc/src/dhcp_lease_policy.h`, `trunk/user/rc/src/tests/dhcp_lease_policy.c`, `.github/workflows/httpd-security.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Regression:** expanded GNU89 policy tests for successful transition, old-route deletion failure, interface clear/set failures, route-add failure, resolver publication failure, `EEXIST` ownership behavior, withdrawn gateway, and incomplete rollback. The HTTPD workflow compiles/runs the host regression and verifies the transaction/source wiring. Exact-head Actions are PENDING at commit creation.
- **Validation boundary:** this commit's host regression and source contracts require exact-head Actions confirmation. L1 source/caller review is complete for the bounded bound callback; L3 production RC object/MIPS-uClibc/firmware and L4 real DHCP renew/failure injection/router validation remain NOT VERIFIED. The BusyBox callback exit-status limitation remains intrinsic; this change protects RC/kernel state with rollback instead of relying on the callback status.
- **Next:** reconcile HTTPD/DHCP and AI handoff Actions against the exact implementation SHA; then resume Optware `opt-umount.sh` service-stop/second-`umount /opt` error propagation. Keep the storage caller-graph contract queued, then rotate to an unaudited user-space module.

---

## 2026-11-09 14:37 CST (UTC+8) — Simplify cross-session records and batch log rotation

- **Base main SHA:** `ae0ddfe000ca0ed92154f3b14b125d667a76832a`.
- **Governance correction:** replace per-entry 20-record rotation with a 120/100/20 batch policy. The root `dev_log.md` accumulates newest-first records from 20 through 119; when a new entry would create record 120, archive the oldest 100 complete entries verbatim in one batch and retain the newest 20. A 384 KiB target is an earlier size-based rotation trigger; 512 KiB is the hard limit.
- **Live-state files:** `SESSION_HANDOFF.md` remains one actionable checkpoint and `REFACTOR_ROADMAP.md` remains one current dashboard. Update each in place only when its own facts materially change. Git history is the version archive; routine Handoff/Roadmap snapshots are no longer created.
- **No mechanical triple-update:** update the log for a coherent milestone, Handoff only when the resumable checkpoint changes or before stopping, and Roadmap only when module status, evidence, inventory or priorities change. Do not create a docs-only follow-up commit just to write a completed CI result back into the files.
- **Historical preservation:** all existing dev-log, handoff and roadmap archive files/indexes remain unchanged and immutable. The validator continues checking their indexed paths, byte counts, heading counts and Git blob SHAs.
- **Changed paths:** `AI_BOOTSTRAP_PROMPT.md`, `AI_DEVELOPMENT_PROTOCOL.md`, `scripts/check_ai_handoff_docs.py`, `.github/workflows/ai-handoff-docs.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- **Validation:** reviewed the new count/size policy and the checker/workflow wiring. The local checker was not run because this session edited repository files through the GitHub API without a local checkout. Exact-head `AI handoff documentation contract` is triggered by this commit; inspect its run against the resulting SHA.
- **Resulting commit:** THIS COMMIT.
- **Next:** resume the existing DHCP bound-path transaction in `udhcpc_lan_bound()` / `lan_up_auto()` as recorded in the current handoff. Reconcile this documentation workflow from live Actions; do not make another docs-only commit solely to record its conclusion.

---

## 2026-11-09 14:27 CST — Atomically publish LAN DHCP deconfig resolver cleanup

- Repository: `Arlene206/padavan-4.4-2`. Base source/test head: `011b6fcf27129efcde3b948384c5b4d1907d0599`.
- Implementation: `d900e566cb990891551f4dfad8dffd2fd897548e` changes `lan_down_auto()` to stage an empty resolver file with `dnsmasq_file_open("/etc/resolv.conf.XXXXXX")` and publish it with `dnsmasq_file_publish()` while holding the existing `resolv` lock. This replaces truncate-in-place `fopen(DNS_RESOLV_CONF, "w+")`: failed staging, flush/write, close or rename does not expose a partial empty resolver file, and the DHCP last-known-good temporary lease identity is still reset only after route deletion, static interface configuration and resolver publication all succeed.
- GNU89 follow-up: `c42ce06cfc784e64bb71e44088423abc144e2cd6` moves the temporary pathname declaration to the start of the function's declaration block; no declaration-after-statement dependency is introduced.
- Regression: the existing DHCP GNU89 host policy test and HTTPD workflow source contract now verify the deconfig stage/publish order and reject direct resolver truncation. Exact-head HTTPD Authentication and Security Regression [42572938754](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42572938754) SUCCESS and AI handoff documentation contract [42572938846](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42572938846) SUCCESS, both on `c42ce06cfc784e64bb71e44088423abc144e2cd6`.
- Verification boundary: L1 source and contract review PASS; L2 the existing DHCP pure-policy regression compiled/run under GNU89 with `-Wall -Wextra -Werror` and exact-head Actions PASS. The workflow does not compile the complete production `net_lan.c` translation unit. No production RC object, MIPS/uClibc, firmware/TRX, live DHCP failure injection or router hardware test.
- Next: finish the still-open bound transition in `udhcpc_lan_bound()` / `lan_up_auto()`: check address reset/configuration, default-route add and resolver publication before state/side-effect publication, and design retry-safe rollback while BusyBox still ignores DHCP callback exit status. Then audit Optware stop-hook/second-umount status propagation and add the drafted storage caller-graph regression.

---

## 2026-11-09 14:00 CST — Map BusyBox LAN DHCP callbacks and avoid Optware eject lock inversion

- Repository: Arlene206/padavan-4.4-2. Source/test HEAD: c2706ac972970bd325b3b9ebb9bd5311f49125f8. Current source commits include AP-mode DHCP lease validation (b96fa1e), fail-closed changed-gateway replacement (2332158), Optware lifecycle safe-eject integration (2c8b5ff), storage test correction (2a5911b), and the lock-order correction below.
- Optware lock-order correction: c2706ac972970bd325b3b9ebb9bd5311f49125f8 adds optware_lifecycle_lock_try_acquire() using flock -n. opt-umount.sh uses this nonblocking acquire because it runs below storage.lock; if opt-start.sh currently owns the lifecycle lock, teardown returns failure promptly and leaves storage attached for retry rather than waiting in reverse lock order. opt-start.sh retains the blocking lifecycle acquisition. Child service commands close fd 9 so they cannot inherit the lock.
- Regression: storage Actions now verify that a competing teardown immediately fails to acquire while startup owns the lifecycle lock, while a normal blocking acquisition waits until release. Exact-head USB storage run 42570160275 SUCCESS and AI handoff documentation contract 42570160219 SUCCESS, both on c2706ac972970bd325b3b9ebb9bd5311f49125f8.
- Remaining Optware issue: opt-umount.sh currently does not check each S??* service stop status and does not check the status of the second umount /opt attempt; a final lock-release command can mask those earlier failures. Inspect and correct hook status propagation before considering safe eject complete. External init scripts and their behavior remain user-supplied and outside repository review.
- BusyBox DHCP callback contract: trunk/user/busybox/busybox-1.24.x/networking/udhcp/dhcpc.c udhcp_run_script() is void; it calls spawn_and_wait(argv) synchronously but discards child exit status. Therefore callback return values do not steer BusyBox's lease state machine. leasefail runs after discovery/request retry exhaustion; accepted NAK invokes nak, then deconfig only if previous state != REQUESTING; deconfig also follows lease expiry, ARP-conflicting offer (outside REQUESTING), repeated SIGUSR1 during RENEW_REQUESTED, and SIGUSR2/release or SIGTERM with release-on-quit.
- DHCP failure state: route_del() in rc/src/net.c returns 0 or positive errno; ifconfig() in shared/src/netutils.c returns 0, positive errno on ioctl error, and -1 on socket/name-copy failure. udhcpc_lan_deconfig() ignores static-address ifconfig and lan_down_auto results. lan_down_auto() ignores route_del and resolver fopen/fclose, then calls update_lan_status(0), which can erase the only temp gateway identity after a failed route delete. Bound path publishes new temp lease fields before ignoring ifconfig / route_add / resolver write-close failures; treat bound/deconfig as one transaction review.
- No new DHCP production code change in this audit. Existing 23321588 change only protects changed-gateway deletion before bound/renew publication; L3/L4 remain unverified.
- Next: design DHCP bound/deconfig internal state handling around BusyBox ignoring script exit status. Preserve old route identity across failed deletion; define interface fallback semantics and atomic/checked resolver publication; add GNU89 helper regression and exact workflow assertions. Then fix opt-umount service-stop/second-umount error propagation, finish the drafted storage caller-graph CI contract, and rotate to another userland module.

---

## 2026-11-09 13:42 CST — Serialize Optware startup and teardown with safe storage removal

- Repository: Arlene206/padavan-4.4-2. DHCP work: b96fa1ee123d45bd15d2e90ff6e4a183ecda0fd0 hardened AP-mode bound/renew lease validation; 23321588f38c3ea9a28355ce7ccc50feb06ef1d8 moved changed-gateway deletion ahead of lease publication, accepts an already-absent ESRCH route and fails closed on other route-delete errors.
- DHCP exact-head regressions: HTTPD Authentication and Security Regression [42568560276](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42568560276) SUCCESS and AI handoff docs [42568560354](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42568560354) SUCCESS on 23321588f38c3ea9a28355ce7ccc50feb06ef1d8. The former b96fa1e HTTPD failure was a stale validator locator after extraction into dnsmasq_domain_policy.h; fixed in 23321588.
- Optware/safe-eject implementation: 2c8b5ff9797cbb6a0abf99c5fa18210d09df6671 adds /sbin/optware_lifecycle_lock.sh and takes its process-shared flock in opt-start.sh and opt-umount.sh. Storage unmount/ejected cleanup now treats a failing Optware stop hook as a reason not to detach the affected mount; eject-all avoids its generic fallback after a failed per-device pass. Deferred storage-app startup acquires storage.lock and preserves pending hotplug flags / schedules retry if the operation lock cannot be acquired.
- Regression correction: first exact-head storage Actions [42568701858](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42568701858) failed because the new shell test called the lock function before sourcing its helper. 2a5911b24548ac2a1ef06442e0b61771a2714733 adds the missing source statement; storage hotplug Actions [42568851009](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42568851009) then completed SUCCESS on exact HEAD. HTTPD/security [42568701873](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42568701873) passed on 2c8b5ff; AI docs [42568850779](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42568850779) passed on 2a5911b before this documentation refresh.
- Storage caller review: traced mdev SD/MMC rules, per-device/shared locks, synchronous _eval/automount, swap/mount/unmount, safe-eject aliases, USB wrapper, PID 1 cleanup and libdisk range/eject-all flow. No direct bypass found in the inspected in-tree call graph. The separate mountpoint flock is not the operation lock. A drafted whole-callgraph Python contract remains uncommitted; current storage Actions cover the Optware lifecycle lock and existing storage regressions, not that uncommitted draft.
- Limits / residuals: Optware init scripts are user-supplied and absent from this repository. Review lock-order interactions if a custom init script invokes storage eject/mount operations while holding the Optware lifecycle lock. Check whether opt-umount stop/unmount failures are surfaced sufficiently; the caller's physical umount still serves as the final busy-device gate. No package-wide external-hook audit or runtime claim.
- Validation boundary: exact-head storage Actions and current pre-refresh docs Actions PASS as described. No MIPS/uClibc production object build, firmware/TRX, live DHCP lease churn, removable-disk churn or physical hardware test was run.
- Next: continue LAN DHCP deconfig / leasefail / NAK semantics, trace BusyBox callback producers and route/interface/resolver failure propagation while preserving last-known-good lease state. Then review Optware hook status/lock-order edge cases and complete the drafted storage caller-graph CI contract; after these bounded items rotate to a different userland module.

---

## 2026-11-09 13:04 CST — Serialize storage hotplug and safe-removal operations

- **Base main SHA:** `b0eecff6e22d056ea144595fa69ceeac1bf1aa83`.
- **Implementation commit:** `5d5b61cfc9d5dd3bca237ff4be31eb0118da1dfb` — introduce a process-shared `storage` fcntl lock around SD/MMC hotplug add/remove, manual safe removal, eject-all and PID 1 ejected-device cleanup. Preserve per-device locks and acquire the shared lock after them; release in reverse order.
- **Regression correction commit:** `cb70977f05b3d7d64dcabf5c02247bda05969f35` — correct the storage policy include terminator and refine the source contract to distinguish the early per-device-lock error release from the normal reverse-order cleanup.
- **Changed paths:** `trunk/user/rc/src/hotplug_stor.c`, `trunk/user/rc/src/services_stor.c`, new `trunk/user/rc/src/storage_operation_lock.h` and `trunk/user/rc/src/tests/storage_operation_lock.c`, `.github/workflows/storage-hotplug-stability.yml`, `.github/workflows/httpd-security.yml`.
- **Finding:** `mdev_sd_main()` / `mdev_mmc_main()` used different `/var/lock/<device>.lock` identities for a disk and its partitions (for example `sda` versus `sda1`); `safe_remove_stor_device()`, eject-all and PID 1 ejected cleanup had no common serialization. Concurrent add/remove and eject flows could inspect and mutate storage topology concurrently. The shared lock uses the existing `file_lock()` ABI. Its same-process re-entry guard makes a nested public `umount_ejected()` unsafe, so the operation body is now `umount_ejected_locked()` behind a locking public wrapper; safe removal invokes the locked helper while already holding the lock.
- **Validation:** exact-head USB storage Actions [42564939868](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42564939868), HTTPD/security Actions [42564939905](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42564939905), and AI handoff documentation contract [42564939843](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42564939843) all **SUCCESS** on `cb70977`. The storage job compiled and ran the new GNU89 `-Wall -Wextra -Werror` fork-based lock regression against the repository's `bin_sem_asus.c`; the competing process was blocked for 2,000 ms. HTTPD/security completed all 39 validation steps. The first implementation attempt [42564591289](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42564591289) and [42564591242](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42564591242) failed on a source-contract ordering assumption and a malformed include; both were corrected before the successful exact-head reruns. The documentation-only snapshot `c2020331b891ed07dd01f729136a5b728fff99f2` was also checked by AI handoff documentation contract [42565709754](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42565709754), which passed on that exact SHA.
- **Validation boundary / residuals:** no full RC production object, MIPS/uClibc build, firmware/TRX build, live filesystem test or physical router test. The existing `automount.sh` mountpoint flock remains a separate label/mountpoint-ownership guard; direct shell-side mount/unmount/swap entrypoints and all callers still need auditing to prove they cannot bypass the operation lock.
- **Next:** inventory every direct caller of `/sbin/automount.sh`, mount/unmount and swapon/swapoff paths; verify which callers already execute beneath the new `storage` lock and identify any bypass without creating nested-lock deadlocks. Then continue disk/partition notification ordering and per-device lock scope. Keep FULLCONENAT mapping-table namespace isolation as a separate P2 security/stability task.

---

## 2026-11-09 12:31 CST — Record green storage eject and repository security regressions

- **Source change:** `7fee5c1fc589dc98c3333dd4e052d28881b8756a` — stage a complete partition inventory before ejected-device unmounts and propagate cleanup errors into PID 1 notification completion.
- **Contract synchronization:** `82096fbab795598bc6eb4384003e8f66998d1997` — update both legacy HTTPD safe-eject assertion blocks to the single-snapshot design and correct archive-index byte counts for the handoff/roadmap snapshots.
- **Exact-head Actions:** USB storage hotplug regression [42561240425](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42561240425) **SUCCESS** on source head `7fee5c1`; HTTPD Authentication and Security Regression [42562165008](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42562165008) **SUCCESS** on `82096fb`; AI handoff documentation contract [42562165077](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42562165077) **SUCCESS** on `82096fb`. The HTTPD job completed all 39 validation steps successfully. The earlier failed attempts [42561802353](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42561802353) and [42561802333](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42561802333) were corrected and superseded by these green exact-head runs.
- **Validation boundary:** L1 source/control-flow review and L2 focused GNU89 helper regression plus whole-repository HTTPD/source/documentation contracts PASS. This does not constitute a production MIPS/uClibc build, firmware/TRX build, filesystem integration test or physical router test.
- **Next:** verify the documentation contract triggered by the final handoff snapshot commit, then continue the storage caller/concurrency audit (disk add/remove ordering, safe-remove callers and lock identity). USB/storage remains PARTIAL / AUDITING.

---

## 2026-11-09 12:27 CST — Correct staged eject CI contracts and archive index byte counts

- **Base main SHA:** `fdd00a4893d40225105bb7521691dbd2621351fd`.
- **Workflow correction:** THIS COMMIT — update the second legacy safe-eject contract block to check the operation-wide partition snapshot, instead of the removed per-mount `procpt2` scan.
- **Documentation correction:** the prior archive-index update recorded the superseded handoff and roadmap lengths as 15 bytes because the index generator measured the wrapper object rather than its `content`. The original Git blob references and archived bytes were preserved; the indexed sizes are corrected here to 6,377 and 8,180 bytes.
- **Exact-head CI at the base:** HTTPD regression [42561802353](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42561802353) failed at the second, still-stale `ejected_body` assertion block; AI handoff contract [42561802333](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42561802333) failed on the two incorrect archive byte counts. The focused USB hotplug workflow [42561240425](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42561240425) passed on code head `7fee5c1fc589dc98c3333dd4e052d28881b8756a`.
- **Validation boundary:** implementation regression remains L2 PASS on its exact source SHA; this follow-up corrects CI/index metadata and awaits exact-head reruns. No MIPS/uClibc firmware or physical USB validation is claimed.
- **Next:** verify HTTPD security and AI handoff contracts on the exact resulting main SHA. Once green, resume the storage add/remove concurrency and per-device lock audit.

---

## 2026-11-09 12:20 CST — Align the storage CI contract with staged eject inventory

- **Base main SHA:** `7fee5c1fc589dc98c3333dd4e052d28881b8756a`.
- **Workflow correction:** THIS COMMIT — migrate the legacy `httpd-security.yml` safe-eject contract from the removed per-mount `procpt2` scan to the operation-wide staged partition snapshot.
- **Exact-head CI:** storage hotplug regression [42561240425](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42561240425) and AI handoff documentation contract [42561240402](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42561240402) both passed on `7fee5c1fc589dc98c3333dd4e052d28881b8756a`. HTTPD security regression [42561240397](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42561240397) failed only at its legacy source assertion requiring `procpt2`/per-mount `/proc/partitions` reads; preceding compile/regression and unrelated source checks passed. The assertion is updated here to require one staged inventory before unmount and no per-mount rescan.
- **Validation boundary:** implementation and focused GNU89 policy test are L2 PASS on the prior exact code head; the updated HTTPD workflow and refreshed documentation require exact-head Actions verification. L3 MIPS/uClibc and L4 physical-device validation remain NOT VERIFIED.
- **Next:** check all Actions for the exact resulting `main` SHA; then continue storage hotplug concurrency audit from add/remove event ordering and per-device lock scope. Do not treat the contract correction as module completion.

---

## 2026-11-09 12:10 CST — Stage storage eject inventories and propagate cleanup failures

- **Base main SHA:** `692b7b2372dd4ccabab879a53ddcd78aad69e751`.
- **Implementation:** THIS COMMIT — stage one complete `/proc/partitions` snapshot before any ejected-device unmount; propagate `umount_ejected()` failure into PID 1 notification completion.
- **Finding:** `umount_ejected()` already staged all of `/proc/mounts`, but reopened and reparsed `/proc/partitions` for each mount. A later partition-inventory read/parse/close failure could therefore be discovered only after earlier mountpoints had already been unmounted. The `on_unplug_mass_storage` handler also discarded the cleanup return value, causing the queue to mark a failed operation as completed.
- **Change:** parse and validate every partition row once, including the procfs heading, exact field count, numeric major/minor/block fields, newline termination, read/close status and allocation outcomes. On any preflight failure, leave all staged mountpoints untouched. Compare the staged names during cleanup instead of reopening procfs. PID 1 now retains the incomplete notification marker when safe-eject cleanup fails; the failure log is generic for all notification types.
- **Regression:** extend `storage_ejected_inventory_policy.c` for valid/header/blank/malformed/truncated/extra-field partition records; add `storage-hotplug-stability.yml` to compile/run the GNU89 host test and assert preflight ordering, single partition scan and notification error propagation. Exact-head execution is pending.
- **Validation boundary:** L1 source/caller review PASS. L2 host regression / Actions PENDING on THIS COMMIT. No production MIPS/uClibc RC build, full firmware image or physical USB/filesystem test is claimed.
- **Next:** verify the storage-hotplug workflow and AI documentation contract on the exact resulting `main` SHA, then continue the caller/concurrency audit across `mdev_sd_main()`, `mdev_mmc_main()`, `safe_remove_usb_device()`, eject commands and per-device lock identity. USB/storage remains PARTIAL / AUDITING.

---

## 2026-11-09 12:05 CST (UTC+8) — Reconcile withdrawn DHCP router options during renewal

- **Base main SHA:** `d96f8df72fe36afcd7875d30ca70228151085a83`; **implementation commit:** `4e9614b5b40ff602423da3488d1aa6dfe859d4d4`.
- **Repository:** maintained fork `Arlene206/padavan-4.4-2`.
- **Finding:** BusyBox udhcpc exports only DHCP options present in the ACK. A server that withdraws option 3 (Router) on renewal leaves `getenv("router") == NULL`. The old WAN and IPTV renew callbacks guarded router comparison with `if (value)`, so neither reconciled a withdrawn gateway and the old default route could survive.
- **Fix:** add `dhcp_lease_router_is_changed()` to `dhcp_lease_policy.h`, normalizing missing/empty router options to an empty gateway; invoke it unconditionally from WAN and IPTV renew callbacks in `net_wan.c`. A changed/withdrawn router enters the existing bound path, which removes old lease routes and clears the gateway. Add host cases for absent, empty, unchanged, changed and invalid options; extend workflow source contracts for both callers.
- **Exact-head verification:** [AI handoff contract 42559613261](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42559613261), [WAN teardown 42559613131](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42559613131), [HTTPD/DHCP regression 42559613119](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42559613119) and [WAN route status 42559612983](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42559612983) all SUCCESS on exact implementation SHA `4e9614b5b40ff602423da3488d1aa6dfe859d4d4`. The HTTPD job's DHCP policy test and normalization/renew source-contract step both reported OK.
- **Verification boundary:** L1/L2 PASS for this scoped path. MIPS/uClibc RC/full firmware (L3) and router DHCP withdrawal/route behavior (L4) NOT VERIFIED.
- **Next:** verify the AI handoff contract on the new governance/archive commit's exact SHA, then rotate to USB/storage hotplug caller/concurrency audit. Keep DHCP LAN and deconfig/leasefail/NAK/release coverage open.

---

## 2026-11-09 12:04 CST (UTC+8) — Restore canonical roadmap archive shard and repair archive inventory

- **Repository:** `Arlene206/padavan-4.4-2`.
- **Recovery:** the prior handoff referenced `docs/roadmap-history/2026/11/part-008.md`, but that canonical path was absent. Recovered the exact Git blob `622163bfb030d1d7fbdf7e4bc844a14ace7df1d0` from its misplaced nested copy and restored the standard indexed path in commit `8bcde43b62bd8c2116a2023ee48c3cdc04ee61a3`.
- **Correction:** the misplaced nested duplicate was not indexed and caused the follow-up archive inventory check to fail. Removed only `docs/roadmap-history/archive/2026/11/part-008.md` in commit `d96f8df72fe36afcd7875d30ca70228151085a83`; the canonical standard-path archive remained intact and byte-identical.
- **Verification:** [AI handoff contract 42559250321](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42559250321) SUCCESS on exact correction SHA `d96f8df72fe36afcd7875d30ca70228151085a83`; log confirms `root log=20 entries; archive shards: dev-log=5, handoff=9, roadmap=8`.
- **Next:** preserve the archive/index invariant during this checkpoint rotation; always compare archive inventory with index rows, not only blob contents.

---

## 2026-11-09 11:40 CST (UTC+8) — Restore required handoff verification heading after contract failure

- **Base main SHA:** `0810b15526a7c6c7b7ba5980679c62fd522f481a`; resulting documentation-only commit: THIS COMMIT.
- **Wireless code regression:** [42557455460](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42557455460) SUCCESS on exact code SHA `0810b15526a7c6c7b7ba5980679c62fd522f481a`; the workflow compiled and ran the GNU89 policy test and passed the wireless source contracts for iNIC recovery, interface-down aggregation, MII teardown and LED publication. Dependent HTTPD regression [42557455207](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42557455207) also SUCCESS on that exact SHA.
- **Documentation contract finding:** [42557455144](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42557455144) FAILED on `0810b15526a7c6c7b7ba5980679c62fd522f481a` because `SESSION_HANDOFF.md` used `## Validation boundary`; `scripts/check_ai_handoff_docs.py` requires the literal heading `## Verification boundary`. No archive/hash/count error was reached in that run.
- **Correction:** restore the required heading without changing the substantive checkpoint; preserve the superseded handoff and roadmap byte-for-byte, update indexes, and rotate the oldest complete root log record to the active shard.
- **Validation:** wireless L2 Actions PASS on exact code SHA; docs L2 correction is PENDING until a fresh exact-head contract run. No target RC/MIPS firmware or hardware check.
- **Remaining:** current docs run must pass on the resulting main SHA. Wireless driver/switch consumers and board-specific runtime remain open; FULLCONENAT mapping namespace scope remains a distinct P2 audit.
- **Next:** verify the fresh AI handoff documentation contract on exact live main. Then resume DNS/DHCP client lifecycle error handling.
- **Result:** THIS COMMIT.

---

## 2026-11-09 11:21 CST (UTC+8) — Fix wireless interface-down result handling and LED state publication

- **Base main SHA:** `91b3bb41efe0f6ad11d18f8a19d9df3592d7c7b6`; resulting source/documentation commit: THIS COMMIT.
- **Module / priority:** wireless management and RT3352 iNIC lifecycle, P2 stability. Whole-module audit remains open.
- **Findings:** `check_inic_mii_rebooted()` discarded the return from the periodic iNIC guest-interface-up command, so a failed recovery was indistinguishable from success. `inicd.c` invokes the check from its SIGALRM timer handler, making this an opportunity for an explicit retry on the next health-check tick. `stop_wifi_all_wl()` and `stop_wifi_all_rt()` ignored every interface-down result and switched the radio LED off even if a netdevice remained up. The RT3352 MII startup path brings up `IFNAME_INIC_MAIN` and optionally `IFNAME_INIC_GUEST`, but the old stop path attempted the non-MII 2.4GHz APCLI/WDS/AP names instead of tearing down those iNIC interfaces.
- **Fix:** route the health-check guest-up through `wif_control_success()` and explicitly defer recovery to the next periodic check on failure. Add a sticky shutdown-result policy; every supported interface-down is attempted and checked, a later success cannot erase an earlier failure, and the LED is switched off only if all required shutdown operations succeeded. The MII path now stops the actual iNIC guest/main interfaces, removes guest VLAN bridge membership, and reports isolate/RGMII-disable failures without short-circuiting cleanup.
- **Changed paths:** `trunk/user/rc/src/net_wifi.c`; `trunk/user/rc/src/wireless_radio_policy.h`; `trunk/user/rc/src/tests/wireless_radio_policy.c`; `.github/workflows/wireless-radio-state.yml`; compact root handoff/roadmap/log plus lossless archives/indexes.
- **Regression:** extended GNU89 host policy test with sticky shutdown-failure aggregation; added source contracts for periodic iNIC recovery status, every stop attempt, MII teardown and LED-off ordering. Exact-head wireless and documentation Actions are PENDING at commit creation.
- **Validation boundary:** L1 source/caller review PASS for the scoped health-check and shutdown paths. L2 host compile/test and Actions PENDING until the new exact-head run completes. No MIPS/uClibc RC build, complete firmware/TRX, live radio restart or hardware test claimed.
- **Remaining scope:** driver/switch consumers, other board-conditional lifecycle paths and concurrent restart/stop behavior remain open. Firewall/NAT remains AUDITING; FULLCONENAT mapping hashes and the dying-tuple queue are still module-global and not namespace-keyed.
- **Next:** reconcile wireless and AI handoff documentation workflows against exact live main SHA. Once green, rotate to DNS/DHCP client lifecycle error handling; retain FULLCONENAT mapping namespace isolation as a separate P2 audit.
- **Result:** THIS COMMIT.

---

## 2026-11-09 11:17 CST (UTC+8) — Confirm FULLCONENAT per-net notifier regression and handoff archive CI

- **Base main SHA:** `91cae3d19bb72a9e0cebcad509db256aded52467`; this documentation-only follow-up records the exact-head results for the code commit.
- **FULLCONENAT lifecycle regression:** [run 42555493213](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42555493213), SUCCESS on exact source commit `91cae3d19bb72a9e0cebcad509db256aded52467`. Its updated step, “Verify FULLCONENAT per-net notifier ownership and cleanup”, passed the per-net registry/refcount, both notifier API layouts, registration/unwind, owning-net unregister, RCU grace period, IPv4/IPv6 rule destructor/module reference order and two-namespace lifecycle assertions.
- **AI handoff documentation contract:** [run 42555491302](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42555491302), SUCCESS on exact source commit `91cae3d19bb72a9e0cebcad509db256aded52467`. The executed `scripts/check_ai_handoff_docs.py` reported: `PASS (root log=20 entries; archive shards: dev-log=4, handoff=6, roadmap=5)`.
- **Code commit:** `91cae3d19bb72a9e0cebcad509db256aded52467` — `fix(netfilter): scope FULLCONENAT notifiers per netns`.
- **Validation boundary:** L1 source review PASS; L2 focused source-contract and documentation-archive Actions PASS on the exact code commit. These workflows do not compile the kernel module or execute conntrack/NAT in a live namespace. No local kernel object, MIPS/uClibc, full firmware/TRX or hardware test is claimed.
- **Open risks:** Firewall/NAT stays AUDITING. FULLCONENAT IPv4/IPv6 mapping tables and its dying-tuple queue remain module-global and are not netns-keyed; IPv4/IPv6 writer parity, rollback/runtime semantics and complete module closure remain open.
- **Actions on this follow-up:** the documentation-only commit will trigger a fresh AI handoff contract run. The next handoff must reconcile that run against the resulting live `main` SHA; the FULLCONENAT code regression above remains tied to the unchanged source commit `91cae3d19bb72a9e0cebcad509db256aded52467`.
- **Next:** after the new documentation contract passes on exact live main, rotate to Wireless: inspect `check_inic_mii_rebooted()` guest-interface-up handling, then `stop_wifi_all_wl/rt()` interface-down and LED publication gates. Keep FULLCONENAT mapping namespace scope as an explicit later audit.

---

## 2026-11-09 11:11 CST (UTC+8) — Scope FULLCONENAT conntrack notifiers to their owning network namespace

- **Base main SHA:** `93277fe13c6c53e49fd94d3badfd4a1a4def0241`.
- **Module / priority:** Linux Netfilter `xt_FULLCONENAT`; P2 conntrack event lifecycle and namespace-cleanup reliability. Firewall/NAT remains **AUDITING**.
- **Recovered task:** resumed the exact handoff's remaining audit of the module-global notifier/reference count against Linux 4.4's per-`struct net` conntrack notifier API. Previous FULLCONENAT GC workqueue/registration-error handling was not repeated.
- **Confirmed defect:** `include/net/netns/conntrack.h` stores `nf_conntrack_event_cb` / `nf_conntrack_chain` inside each namespace. A single global `tg_refer_count` registered the callback only in the first namespace; a last-rule teardown in a second namespace then unregistered against the wrong `par->net`. In the legacy event API this can trigger `BUG_ON(notify != new)`; in chain-event builds it targets the wrong notifier chain. Destroy events for other namespaces were also not delivered to FULLCONENAT's GC.
- **Fix:** `xt_FULLCONENAT.c` now maintains one notifier object and reference count per namespace. Each namespace registers independently; same-net rules share the node; the last destructor unregisters from the exact stored owner. Allocation, registration and overflow failures release the `nf_ct_netns_get()` reference; legacy callback storage is freed only after `synchronize_rcu()`. Added explicit slab/RCU headers and kept declarations GNU89-compatible.
- **Teardown review:** IPv4/IPv6 iptables cleanup passes its owning `struct net` into the target destructor and performs `module_put()` only after the destructor returns. The regression contract was extended to cover this ordering, both notifier API variants, registration/unwind, per-net refcounts and independent namespace lifecycles.
- **Changed paths:** `trunk/linux-4.4.x/net/netfilter/xt_FULLCONENAT.c`; `.github/workflows/fullconenat-lifecycle.yml`; the workflow also reads `include/net/netns/conntrack.h`, IPv4 `ip_tables.c`, IPv6 `ip6_tables.c`, conntrack event provider and existing IPv6 restore rollback sources.
- **Validation:** L1 source review PASS for notifier API ownership, error unwind, target teardown and RCU lifetime. The expanded workflow is a source-contract/model regression only; exact-head Actions are **PENDING**. No local kernel object/module build or standalone host executable was run.
- **Actions:** FULLCONENAT lifecycle regression and AI handoff documentation contract are PENDING until their run IDs, conclusions and `head_sha` are reconciled against the resulting main commit.
- **Remaining risks:** FULLCONENAT mapping hashes and the dying-tuple queue remain module-global and are not keyed by namespace; this round fixes notifier ownership only. IPv4/IPv6 writer parity, complete NAT rollback/runtime semantics, MIPS/uClibc, full firmware/TRX and hardware are not verified.
- **Resulting commit:** THIS COMMIT.
- **Next:** verify both Actions workflows on exact live main. If green, rotate to Wireless residuals at `check_inic_mii_rebooted()` guest-interface-up handling and `stop_wifi_all_wl/rt()` interface-down/LED publication; retain FULLCONENAT mapping namespace scope as an explicit open audit.

---

## 2026-11-09 11:08 CST (UTC+8) — Confirm exact-head route, teardown, HTTPD and handoff CI

- **Base main SHA:** `3e1be8adf8ac531581fa6e505da36c0e5e1e5966`.
- **Route code CI:** WAN route operation status regression [42552840579](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42552840579) SUCCESS on exact implementation SHA `6f4b55e88a553e8bf6ae6f3b41c59963335eb787`; sanitizer helper regression and route source/caller contract passed.
- **WAN teardown CI:** [42552840447](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42552840447) SUCCESS on that same implementation SHA.
- **Documentation at route implementation SHA:** [42552840438](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42552840438) SUCCESS.
- **Cross-workflow stale assertion:** HTTPD run 42552840406 first failed because an existing DHCP lease source contract still expected a void route-delete wrapper and an `if (is_add)` branch. Its DHCP lease C regression passed. The contract was updated in `.github/workflows/httpd-security.yml`; corrected HTTPD run [42553153266](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42553153266) and documentation run [42553153260](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42553153260) both SUCCESS on exact SHA `3e1be8adf8ac531581fa6e505da36c0e5e1e5966`; all HTTPD workflow steps passed.
- **Handoff:** root status records both the route implementation and the cross-workflow contract correction; the prior handoff/dashboard are archived byte-identically. New root log is still the newest 20 records.
- **Validation boundary:** L1 source review PASS; L2 helper sanitizer, route workflow, WAN teardown, corrected HTTPD and pre-final docs workflow PASS. The final documentation-only commit's handoff-contract run must be reconciled against its exact HEAD. Production RC object, MIPS/uClibc, complete firmware/TRX, live route failure injection and hardware remain NOT VERIFIED.
- **Resulting commit:** THIS COMMIT.
- **Next:** verify the final handoff-contract Actions run against exact main, then start the bounded FULLCONENAT namespace/notifier ownership audit. Do not repeat the route status fix.

---

## 2026-11-09 10:47 CST (UTC+8) — Reconcile WAN route Actions and correct dependent HTTPD contract

- **Base main SHA:** `6f4b55e88a553e8bf6ae6f3b41c59963335eb787`.
- **Route implementation CI:** [WAN route operation status regression 42552840579](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42552840579) completed SUCCESS on exact `6f4b55e88a553e8bf6ae6f3b41c59963335eb787`; sanitizer regression and source/caller contract both passed.
- **Additional exact-head checks:** WAN interface teardown run 42552840447 and AI handoff documentation run 42552840438 both completed SUCCESS on exact `6f4b55e88a553e8bf6ae6f3b41c59963335eb787`.
- **Recovered cross-workflow failure:** HTTPD run 42552840406 failed in its existing DHCP lease source-contract Python step after the route API changed from `void` to `int` and from a literal `if (is_add)` branch to the equivalent checked ternary. The DHCP lease C policy regression itself passed. This was a stale workflow expectation, not a production compile or runtime finding.
- **Correction:** `.github/workflows/httpd-security.yml` now checks the int-returning DHCP delete wrapper/prototype and ternary add/delete selection. The corrected workflow is included in this follow-up commit; rerun on its exact HEAD is PENDING.
- **Documentation:** handoff/roadmap now record the exact run outcomes and next Firewall/NAT task. Previous root checkpoint/dashboard are preserved byte-identically in indexed archives; root log remains a 20-entry window.
- **Validation boundary:** route host sanitizer regression and exact-head route Actions PASS; exact-head WAN teardown and documentation-contract Actions PASS. Corrected HTTPD Actions PENDING. No production RC object, MIPS/uClibc, firmware/TRX or hardware test.
- **Resulting commit:** THIS COMMIT.
- **Next:** verify corrected HTTPD and AI handoff workflows against exact main HEAD, then begin FULLCONENAT per-net notifier/reference-count lifecycle audit.

---

## 2026-11-09 10:34 CST (UTC+8) — Propagate static and DHCP route operation failures

- **Base main SHA:** \`347ab8b0392a3c74d85a230e199c2faae27f646c\`.
- **Finding:** \`control_static_routes()\` discarded every \`route_add()\` / \`route_del()\` status and returned success; DHCP route setup/removal used a void helper and discarded legacy and RFC3442/Microsoft route failures. DHCP-list allocation failures were also hidden.
- **Fix:** static and DHCP helpers now return the first nonzero operation status while continuing to attempt every remaining valid route. Both DHCP option lists are handled independently, so one allocation failure does not suppress processing of the other. Public DHCP wrappers now return status; every failed route and allocation is logged. Route deletion remains available when dynamic-route addition is disabled, and WAN/MAN cleanup ordering is preserved.
- **Changed paths:** \`trunk/user/rc/src/net.c\`, \`net_wan.c\`, \`rc.h\`; new \`route_status_policy.h\`, \`tests/route_status_policy.c\`, and \`.github/workflows/wan-route-status.yml\`.
- **Validation:** local GNU89 \`-Wall -Wextra -Werror\` regression with ASan/UBSan and leak detection: **PASS**. The regression verifies first-error preservation and that later route operations are still attempted. Workflow source contracts cover static/DHCP wiring, allocation handling, public return types and WAN/MAN delete ordering.
- **Actions:** exact-head WAN route workflow is **PENDING** at commit creation; reconcile its run ID, conclusion and head SHA during the next recovery. The existing documentation-contract run [42550725545](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42550725545) passed on the pre-change main SHA \`347ab8b0392a3c74d85a230e199c2faae27f646c\`.
- **Validation boundary:** L1 selected helper/caller review PASS; L2 focused host policy regression PASS. Production RC translation-unit/object, MIPS/uClibc target build, complete firmware/TRX, live route failure injection and physical-router validation are **NOT VERIFIED**.
- **Resulting commit:** THIS COMMIT.
- **Next:** reconcile exact-head Actions, then rotate away from WAN to Firewall/NAT lifecycle ownership; do not mark WAN/PPP audited or target-verified.

---

## 2026-11-09 10:23 CST — Establish durable cross-conversation handoff and bounded project records

- **Base main SHA:** `71de75dd39b2ebc87fa56b1c74ff798cc0e57fdf`.
- **First priority:** make GitHub sufficient for a fresh GPT session to recover the exact active task, whole-repository progress and verification boundary without relying on chat memory.
- **Governance:** bootstrap and protocol now enforce one recovery sequence, compact current-state root files, size limits, lossless archives and resumption of the open task before selecting new work.
- **History preservation:** the old `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md` and all 663 original `dev_log.md` entries are retained verbatim in indexed archives. This root log contains this entry plus the 19 newest prior entries.
- **Automation:** add `scripts/check_ai_handoff_docs.py` and the `AI handoff documentation contract` workflow. Its exact-HEAD result is PENDING until retrieved.
- **Scope:** documentation/governance only; no firmware source, MIPS build, firmware image or hardware test is changed or claimed.
- **Resulting commit:** THIS COMMIT; fetch live `main` during the next recovery to obtain the exact SHA.


---

## 2026-11-09 09:58 CST — Include the active USB NDIS interface in WAN teardown verification

- **Base main SHA:** `d324ce8e26f6f730aa36d311c8d51c701879edcc`.
- **Follow-up finding:** a final caller-path review found that `stop_wan_usbnet()` can operate on a distinct NDIS interface stored in `wan0_ifname_t`. The first teardown-policy version checked only the management interface and `IFNAME_WAN`; because the USB helper itself returns void, a distinct `wwan0` / `weth0` down failure could still escape the checked result.
- **Fix:** code commit `8a02953fb60cced248254df86feca85e8bc42d10` passes the runtime `ifname_t` as an optional third interface only when a USB modem is active and `modem_type == 3`. The shared policy now attempts all non-empty distinct management, physical-WAN and USB-NDIS names, deduplicates equal names, and aggregates failures without skipping later cleanup. The adapter's existing `ENODEV` handling still treats a disappeared interface as unavailable, while other errors keep the final state-clear gate closed.
- **Regression expansion:** `tests/wan_teardown_policy.c` now covers optional NDIS shutdown, first/second/third-interface failures, continued attempts after a failure, duplicates across the three names, wired-only operation and invalid required inputs. The workflow source contract verifies the active NDIS selection and that `ifname_t` reaches the common checked teardown.
- **Workflow compatibility:** exact-head WAN regression [run 42548284186](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42548284186) completed **SUCCESS** on `8a02953fb60cced248254df86feca85e8bc42d10`. The broader HTTPD/service lifecycle regression [run 42548284184](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42548284184) also completed **SUCCESS** on the same head. This followed two corrected source-contract mismatches: the initial WAN workflow's global return search was narrowed in `395af4db6d5f10e9356b9709d9072aa4673be0f8`, and the existing HTTPD workflow's expected `stop_wan()` failure-return count was updated from three to four, with an explicit interface-state gate, in `d63ddae44e104d3fd781a2d19b30a86d2bd7d083`. Both workflows subsequently passed on the final code head.
- **Additional diagnostic precision:** `d324ce8e26f6f730aa36d311c8d51c701879edcc` changes the failure log to say that the *final* WAN state clear is skipped; this avoids implying that protocol-specific `wan_down()` bookkeeping performed earlier has been rolled back.
- **Validation:** L1 control-flow review PASS. Local GNU89 host regression compiled with `-Wall -Wextra -Werror -fsanitize=address,undefined` and ran with ASan/UBSan plus leak detection: **PASS**. Exact-head Actions above passed both the focused policy regression and the existing service-stop/WAN contracts.
- **Validation boundary:** no production RC translation-unit/object build, MIPS/uClibc, firmware/TRX, runtime NDIS hot-unplug/failure injection or physical-router testing. The static/DHCP route result-propagation finding remains separate and open; WAN/PPP, Wireless and Firewall/NAT are not closed.

---

## 2026-11-09 09:49 CST — Preserve WAN runtime state when interface shutdown fails

- **Base main SHA:** `54689c55e57094f95fda7566afd2933ecf40fbf9`.
- **Implementation commit:** `26c3c67d27e1b18fba4da14bd607f80fa70afe3b` — add a small host-testable interface teardown policy and make `stop_wan()` check the final management and physical-WAN `ifconfig(..., down, 0.0.0.0)` operations after USB-modem shutdown. Both distinct interface names are attempted even if the first fails; duplicate names are attempted once. `ENODEV` is treated as an interface already removed/unavailable; other failures are logged and cause `stop_wan()` to return `-1` after callback-link cleanup but before `clear_wan_state()` and final WAN LED-down publication.
- **Finding:** `ifconfig()` returns `-1` for socket/name-copy failures and positive errno values for ioctl failures. The previous `stop_wan()` discarded the return from both interface-down calls and still cleared WAN runtime state and returned success. A failed interface-down could therefore be reported as a completed disconnect/reconnect precondition while an interface remained active.
- **SIGUSR2 investigation:** the suspected ignored `killall -SIGUSR2 udhcpc` status was traced rather than patched speculatively. It is a best-effort DHCP RELEASE signal; the subsequent `kill_services(svcs_wan, 3, 1)` uses the process inventory, TERM/KILL escalation and final survivor check, and `stop_wan()` returns before interface teardown if a WAN worker remains. No unsafe teardown ordering was demonstrated for the ignored RELEASE result or fixed 300ms grace delay. Keep its status/retry policy as an explicit review item only if new evidence shows a defect.
- **Route audit evidence:** `route_add()` / `route_del()` return `route_manip()` errors, but `net.c::control_static_routes()` discards per-route results and returns success, while `net_wan.c::control_dhcp_routes()` is void and discards dynamic-route add/delete failures. This is a separate unimplemented follow-up, not included in this bounded interface teardown change.
- **Changed paths:** `trunk/user/rc/src/net_wan.c`; new `trunk/user/rc/src/wan_teardown_policy.h`; new `trunk/user/rc/src/tests/wan_teardown_policy.c`; new `.github/workflows/wan-teardown-stability.yml`.
- **Regression / workflow correction:** the initial workflow commit's global `return -1` ordering assertion matched an earlier PPP/service failure return and failed on run [42547441349](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42547441349). Workflow-only correction commit `395af4db6d5f10e9356b9709d9072aa4673be0f8` scopes the assertion to the interface-failure block; no production-code change was needed for this test correction.
- **Validation:** L1 source review confirmed the `ifconfig()` errno contract, the preceding PPP/WAN service-exit fences, the dual-interface shutdown ordering and the state-clear gate. L2 local host regression compiled with `cc -std=gnu89 -Wall -Wextra -Werror -fsanitize=address,undefined` and ran with ASan/UBSan plus leak detection: **PASS**. Exact workflow-head Actions [run 42547527340](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42547527340) completed **SUCCESS** on `395af4db6d5f10e9356b9709d9072aa4673be0f8`; compile/run and Python source-contract steps passed.
- **Validation boundary / remaining risks:** no production RC translation-unit/object build, MIPS/uClibc build, complete firmware/TRX, live DHCP client/interface failure injection, USB modem runtime or physical-router validation. Static and DHCP route add/delete result propagation remains open; Wireless still has the separate iNIC reboot/stop residuals; Firewall/NAT and the wider WAN/PPP audit remain open.

---

