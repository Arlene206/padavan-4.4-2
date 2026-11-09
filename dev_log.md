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

