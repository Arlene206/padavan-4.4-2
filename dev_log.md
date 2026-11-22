## 2026-11-22 — Fail closed on DHCPv6 DUID hardware-address NVRAM errors

- Parent main SHA: c338dc336ac798e6d319fb50fd0d3ec801fbc434. start_dhcp6c() used nvram_safe_get("wan_hwaddr") directly in ether_atoe(), so backend read failure was indistinguishable from a missing/invalid MAC and could be silently treated as normal while the DHCPv6 client started.
- Added a one-field checked rc_wan_nvram_copy_snapshot() before DUID file and callback-session side effects. A missing/invalid MAC still follows legacy behavior and skips DUID file generation; a real read or transaction-close error now aborts startup before acquiring resources.
- Added three fake-backend tests for missing-key compatibility, backend read failure, and transaction-close failure; both WAN source contracts require the checked snapshot and reject the legacy getter.
- Exact-head verification of the reset snapshot parent must be reviewed from live Actions; do not treat an ancestor PASS as proof of this combined source state. This commit also requires fresh exact-head WAN DHCP/WAN route/AI handoff runs.
- XY-C1 Build/TRX/artifact L3 and hardware L4 remain NOT VERIFIED.
- Next task: inspect remaining DHCPv6 lease/service startup inputs for error-collapsing NVRAM getters affecting route, DNS, DUID or client lifetime.

---
## 2026-11-22 — Snapshot IPv6 WAN reset settings before runtime mutation

- Parent main SHA: bbff528ccfe5f1cc45e9cbc87d699004867eb541. reset_wan6_vars_status() still consumed static address/prefix/gateway and 6rd relay/size with separate error-collapsing nvram_safe_get()/nvram_get_int() reads. Backend errors could masquerade as empty or zero configuration while reset logic proceeded.
- The active settings are now copied in one checked rc_wan_nvram_copy_snapshot() transaction: static mode reads address/prefix/gateway plus the 6rd pair; non-static mode reads only the 6rd pair. Prefix parsing uses rc_wan_nvram_parse_int_snapshot(), retaining empty-to-zero compatibility but rejecting malformed values. Transaction/read/close errors return before runtime NVRAM writes.
- Added three GNU89 fake-backend regressions covering coherent static tuple/prefix parse, failure injection at each of five fields with all outputs cleared, and the two-field dynamic mode. Updated both WAN source contracts to require the snapshot and reject legacy getters.
- Parent b7185685f1868b72ad48e87fadf434bdf7f7f0ee: WAN DHCP #45021657373 PASS, WAN route #45021657378 PASS, AI handoff #45021657238 PASS. These runs validate the previous DNS change, not this snapshot.
- Exact-head WAN DHCP, WAN route and AI handoff Actions are required for this new commit. XY-C1 L3 and hardware L4 remain NOT VERIFIED.
- Next task: inspect the `start_dhcp6c()` `wan_hwaddr` DUID/IAID read; preserve missing/invalid MAC behavior but stop startup on real backend read/close failure.

---
## 2026-11-22 — Align WAN DHCP source contract with compensated IPTV startup
- Parent main SHA: fe22b9fae01db2bb9fe57f2fffe67cee067ea364. WAN DHCP callback run #45022142584 passed every preceding host C regression, including the five new dynamic IPTV start/stop/rollback tests, then failed in its inline Python source contract at line 1242.
- The stale contract required literal assignments calling start_zcip_viptv(viptv_ifname) and start_udhcpc_viptv(viptv_ifname) inside launch_viptv_wan(). Those calls now sit behind wan_viptv_dynamic_start_callback() so failed launch can invoke stop-before-VLAN-rollback compensation.
- Updated the inline workflow contract to validate both real client start callbacks, the udhcpc/zcip stop callbacks and the compensated launcher call inside launch_viptv_wan(). Checker-only; no production behavior changed.
- Exact parent WAN route #45022142699, AI handoff #45022142702, WAN interface teardown #45022142730, HTTPD #45022142700 and IPv6 service-type #45022142701 PASS. Parent WAN DHCP failed only at the outdated assertion; NVRAM #45022142711 and XY-C1 #45022142628 remained in progress. New-SHA Actions must be checked independently.

---
## 2026-11-22 — Roll back IPTV VLAN after failed dynamic client start
- Parent main SHA: b7185685f1868b72ad48e87fadf434bdf7f7f0ee. launch_viptv_wan() published viptv_ifname as a recovery handle and tracked VLAN/bridge ownership, but DHCP/ZeroConf client launch errors returned directly. A failed detached udhcpc readiness check could leave the client/process uncertain while the newly-owned VLAN remained unreconciled.
- Added a dynamic IPTV start policy: start failure invokes the client-specific stop (udhcpc PID-file no-force stop, or the zcip process stop). Only a confirmed stop permits rollback_viptv_vlan_setup(); if stop is uncertain, the VLAN and recovery identity remain intact for later teardown. The primary launch status/errno remains authoritative; cleanup statuses are logged separately.
- Added five GNU89 ASan/UBSan policy regressions and expanded scripts/check_wan_vlan_setup.py to pin stop-before-rollback ordering, callback wiring, recovery identity retention and main() registration. The existing WAN workflow already watches source, policy and test and runs the Python checker.
- Parent exact-head AI handoff #45021657238, WAN DHCP callback #45021657373 and WAN route #45021657378 PASS. New-SHA results must be queried separately; XY-C1 will be triggered by net_wan.c.
- XY-C1 L3 remains unverified until same-SHA Build, TRX and artifact upload all pass; hardware L4 remains unverified.

---
## 2026-11-22 — Update WAN DNS source contracts for transactional static configuration reads

- Parent main SHA: 2d98a256c0703d02688de4ab88bb470bd1d5ee3d. Exact-head WAN DHCP callback run #45021524741 and WAN route status run #45021524732 exposed two stale Python source contracts after the DNS getter was correctly replaced. Both failures were in checkers, not C sanitizer tests.
- `check_wan_dns_read_status.py` still required `rc_wan_nvram_copy_text(nvram_safe_get("ip6_dns1"))` in build_dns6_value(); it now pins the checked `wan6_dns_copy_config_snapshot()` call after service-mode error handling.
- `check_wan6_dns_policy.py` expected a literal `destination[0]` clear in the DNS wrapper, but all-or-nothing clearing is correctly implemented by the shared `rc_wan_nvram_copy_snapshot()` helper as `fields[i].destination[0]`. The checker now verifies that shared implementation instead of a duplicate local clear.
- Parent exact-head HTTPD #45021524756 PASS, IPv6 service-type #45021524735 PASS, AI handoff #45021524758 PASS. WAN DHCP #45021524741 and WAN route #45021524732 failed on stale checker assumptions. NVRAM #45021524817 and XY-C1 #45021524718 were in progress at last inspection.
- The focused DNS policy C regressions and shared NVRAM sanitizer test were reached and their output showed successful behavior up to the Python source contract; re-run exact-head WAN workflows after this checker-only correction.
- Next source task remains static IPv6 WAN address/gateway/6rd reset inputs: inspect the remaining error-collapsing `nvram_safe_get()` calls in reset_wan6_vars_status() and move relevant values into one checked snapshot.

---
## 2026-11-22 — Read static WAN IPv6 DNS configuration as one checked NVRAM snapshot

- Parent main SHA: 1f1a8736e9e4edd0938002ee3a0321344eea3317. build_dns6_value() read ip6_dns1/ip6_dns2/ip6_dns3 through nvram_safe_get(). That API returns empty values when an NVRAM backend read fails, so a transient fault could be mistaken for intentionally empty static DNS and published as a DNS withdrawal.
- Replaced three error-collapsing reads with one rc_wan_nvram_copy_snapshot() transaction through the existing WAN NVRAM callbacks. All three cache-backed strings are copied before the next read; missing individual keys retain the legacy empty-string default; any backend or transaction-close error invalidates the complete tuple and prevents publication.
- Added five focused GNU89 host regressions for coherent cache-copying, missing-key compatibility, read failure, transaction close failure and begin failure. Updated check_wan6_dns_policy.py to reject the old nvram_safe_get path and require the checked transaction helper.
- Parent exact-head Actions on 1f1a8736e9e4edd0938002ee3a0321344eea3317: libdisk #45021103093 PASS, Process inventory #45021103117 PASS, AI handoff #45021103133 PASS and NVRAM transaction audit #45021103142 PASS. No XY-C1 Build/TRX/artifact gate was observed on this parent.
- Exact-head WAN DHCP callback Actions for THIS COMMIT must be checked after publication; no host test or CI result is claimed before that run completes. XY-C1 L3 and hardware L4 remain NOT VERIFIED.
- Next task: audit remaining DHCPv6 static WAN address/gateway and router advertisement settings for the same lossy NVRAM getter pattern, then add a checked snapshot if a caller can publish partial runtime identity.

---
## 2026-11-22 — Correct account-inventory test registration after exact-head failure

- Parent main SHA: a09d94d07031bf597c8b9ba77d65df6a40367dbb. Exact libdisk run #45020980995 failed at the first GNU89 test compile, before either sanitizer binary ran. The regression call list had matched the first generic return statement in the fake begin adapter instead of main(), producing implicit declarations and unused static test functions.
- Moved all ten checked-account-inventory test invocations into the real test main() and removed them from the fake NVRAM begin callback. The policy implementation, test bodies and source checker are unchanged. Re-run the exact-head libdisk workflow before treating this slice as validated.
- No other test result is inferred from the failed run; the exact-head AI handoff workflow on the source SHA passed.

---
## 2026-11-22 — Read libdisk account inventory through a checked NVRAM transaction

- Parent main SHA: 9ab6be48e197346138804d954433ffb956e507cf. get_account_list() relied on nvram_get_int("acc_num") and nvram_safe_get("acc_usernameN"); both collapse a backend/close failure into zero or empty text. During transient NVRAM faults, account management could mistake an unreadable inventory for an empty one.
- Added `disk_share_account_nvram_policy.h` using the existing checked count/copy helpers and a begin/get/end NVRAM transaction. Count parsing is strict and capacity-bounded; the router HTTP username and each account name are copied before the shared NVRAM cache is reused. The policy rejects malformed/sparse/duplicate/reserved inventories and discards partial results when any read or transaction close fails. Missing `acc_num` still maps to an empty inventory.
- `get_account_list()` now delegates to the policy through transaction adapters; the Samba configuration caller does not hold an outer NVRAM transaction, avoiding nested acquisition. The host workflow includes the shared policy dependency path and compiles fake-backend regressions for count/read/end/begin errors, malformed range, sparse and reserved names, duplicate values, and clean snapshot results.
- Parent SHA `3b52653213f3e9e3f0580944d0a59d8024430bec`: Process #45020631231 PASS, HTTPD #45020631208 PASS, AI handoff #45020631277 PASS; NVRAM audit #45020631251 and XY-C1 #45020631232 were in progress. Libdisk #45020631266 failed only in Python source contract because an obsolete duplicate mod_account assertion expected a direct errno expression; both sanitizer binaries passed. That stale block was removed in child checker repair #9ab6be48e197346138804d954433ffb956e507cf; exact-head results must be inspected.
- Exact-head Actions for THIS COMMIT are required before claiming the new fake-backend tests PASS. XY-C1 L3 remains unverified until Build, TRX and artifact upload pass on one exact product SHA; hardware L4 remains NOT VERIFIED.
- Next task: select a bounded WAN/service lifecycle defect outside libdisk and add a focused regression; do not reopen the account inventory slice unless its exact-head test finds a defect.

---
## 2026-11-22 — Remove stale duplicate mod-account checker contract

- Parent main SHA: 3b52653213f3e9e3f0580944d0a59d8024430bec. Exact-head libdisk run #45020631266: both GNU89 ASan/UBSan binaries PASS; Python source-contract step FAIL. The newly hardened mod_account() now checks old/new paths, source/target metadata types, checked disk inventory, and uses disk_share_account_rename_status() to normalize ENOENT/error outcomes. An older duplicate checker block still expected the pre-refactor direct errno!=ENOENT expression and rejected the valid policy-helper implementation.
- Removed only that stale duplicate assertion block. The subsequent comprehensive mod_account source contract remains: it requires read_disk_data_checked(), bounded paths, regular-file source, destination collision rejection, rename-result policy, cleanup, service reload, and error propagation. Existing GNU89 rename-policy regressions remain enabled.
- No production behavior or test assertion was weakened. The failure was a duplicate obsolete structural assumption; exact-head libdisk and handoff Actions must be rechecked on this checker-fix SHA.
- After the checker passes, continue with the previously identified get_account_list() defect: unsafe nvram_get_int()/nvram_safe_get() collapse backend read/close errors into zero/empty account state. Replace with one checked NVRAM transaction and fake-backend host regression.

---
## 2026-11-22 — Fail closed on incomplete mod-account inventory and metadata collisions

- Parent main SHA: `074bc11b3338917d1472e5146b31f9c0c59cca25`.
- `mod_account()` used legacy `read_disk_data()`, which returns NULL both when there are no disks and when the storage inventory cannot be read completely. After the account name was committed to NVRAM, this could silently skip all per-volume metadata migration and return success. It now uses `read_disk_data_checked()` and reports inventory failure after service reload.
- The per-volume rename used plain `rename(old,new)`, which can replace an existing destination metadata file. It now validates the old metadata is regular, treats only ENOENT as an absent source, checks that the new path does not already exist (EEXIST rather than clobbering stale permissions), and preserves the first path/stat/rename error while continuing across volumes.
- NVRAM-before-filesystem ordering is unchanged. A migration failure may leave NVRAM updated while some per-volume metadata remains under the old name; no rollback is claimed. The destination pre-check prevents ordinary stale-file clobbering but is not a kernel-atomic no-replace operation against an external race.
- Added GNU89 policy regressions for rename success, ENOENT idempotency, real errno preservation and EIO normalization, plus source contracts for checked inventory, bounded paths, target collision checks and cleanup/service ordering.
- Parent exact-head #45020129047 AI handoff, #45020129064 libdisk, #45020129138 Process and #45020129049 HTTPD were PASS. #45020129046 NVRAM audit was still scanning ioctl/API callers; #45020129134 XY-C1 Build was running, with TRX and artifact upload pending. These ancestor results do not validate this commit.
- L3 remains NOT VERIFIED until Build XY-C1, TRX verification and artifact upload all PASS on this exact product/source SHA. Hardware L4 remains NOT VERIFIED.

---

## 2026-11-22 20:23 CST (UTC+8) — Bound mod-account metadata renames and propagate failures

- Parent main SHA: 5f00e014e746562927ffe73b75db82333c170300. mod_account() changed the NVRAM username/password first, then formatted old/new per-volume metadata paths with unchecked snprintf() and ignored every rename() result. Long mount paths could silently target truncated paths; failed renames were reported as success.
- Both old and new metadata paths now use disk_share_format_account_var_path(). A missing old metadata file is treated as no volume-specific record; path construction and all other rename failures are aggregated while processing every mounted volume. Disk inventory is freed and ftpsamba is reloaded before the first error is returned.
- Password-only and same-name behavior are preserved, and the existing NVRAM commit ordering remains unchanged. No claim is made that a post-commit volume rename failure rolls NVRAM back.
- Added a source-contract regression for bounded old/new paths, ENOENT-only skip behavior, full-volume processing and cleanup/service/error ordering.
- Exact parent libdisk #45019979218, AI handoff #45019979195, HTTPD #45019979197, Process inventory #45019979224, NVRAM audit #45019979154 and XY-C1 #45019979256 were in progress at the last poll. No result is inferred from pending checks.
- Next bounded task: inspect the remaining libdisk account bootstrap/removal call sites and NVRAM error-return semantics, then select one independent source-level stability defect outside libdisk (WAN/service lifecycle) to broaden coverage.

---
## 2026-11-22 20:18 CST (UTC+8) — Fix del-account inventory leak and per-volume metadata cleanup

- Parent main SHA: 0a7581f2ef8ad2a04d00fa827626ae28fd85a339. del_account() decremented acc_num before passing it to free_2_dimension_list(), leaking the final owned account string. It also built per-volume metadata paths with unchecked snprintf() and ignored unlink() failures.
- Preserve the original account_count for full-list cleanup while using the decremented acc_num for NVRAM compaction. Even the zero-account/error path now releases the returned inventory.
- Per-volume metadata deletion now uses disk_share_format_account_var_path(), ignores only ENOENT, and records the first other path/unlink error while continuing through every mounted partition. The disk inventory is freed and ftpsamba is reloaded before the first cleanup error is returned.
- Added source-contract regressions for original-count freeing, checked path construction, ENOENT-only behavior, all-volume processing, cleanup ordering and first-error reporting. NVRAM mutation ordering is unchanged.
- Exact parent Actions on `0a7581f2ef8ad2a04d00fa827626ae28fd85a339`: libdisk #45019873074 PASS; AI handoff #45019873116 PASS. The earlier 9deb94c source/checker run failed due to multiline source lookup in the checker; that was corrected in 0a7581f and its exact-head libdisk run passed.
- New commit exact-head Actions must be inspected after publication. XY-C1 L3 remains NOT VERIFIED until Build, TRX and artifact upload all pass on a single product-source SHA; hardware L4 NOT VERIFIED.
- Next bounded task: audit mod_account() for unchecked metadata path construction and ignored rename failures while preserving the NVRAM transaction order.

---
## 2026-11-22 20:14 CST (UTC+8) — Normalize the remaining libdisk lifecycle contracts

- Parent main SHA: 9deb94ca8a41f17d782fb3626a41675fd91236e6. The new folder-cleanup contract normalized source calls but still compared whitespace-bearing first-error literals against the whitespace-free source. Corrected both first-error predicates to their compact forms.
- The newly added add_account() contract also needed to normalize its multi-line initializer call and cleanup sequence before checking; it now validates the exact checked call, per-volume error gate, free_disk_data(), Samba reload and final first-error return without assuming line wrapping.
- Exact parent Actions: Process inventory #45019778710, HTTPD #45019778715, XY-C1 #45019778714, libdisk #45019778723, handoff #45019778755 and NVRAM audit #45019778700 were in progress at the last poll. The previous parent checker run #45019713462 failed the `first_error` string formatting contract; both sanitizer binaries passed.
- Exact-head regression validation must be re-run on this checker-only commit. Next product task remains del_account() metadata unlink error propagation and account-list allocation cleanup.

---
## 2026-11-22 20:10 CST (UTC+8) — Propagate add-account volume metadata initialization errors

- Parent main SHA: a6908c5279bee5071759f8b45da3d1a75f27fd6d. add_account() committed the account to NVRAM, iterated mounted volumes, but ignored every initial_one_var_file_in_mount_path() return value and always reported success. A failed disk write could leave the new account without its permission metadata while the UI/API saw a successful add.
- It now records the first initialization errno (falling back to EIO), continues initializing every independent mounted volume, frees the disk inventory, reloads Samba, and then returns failure when any volume could not be initialized. The established NVRAM commit ordering and no-mounted-disk behavior are unchanged; this does not claim an NVRAM rollback.
- Added an exact-source regression contract requiring the initialization result to be checked, all volume attempts to finish, cleanup and Samba reload to occur, and the first error to be returned.
- Previous exact-head handoff Actions: #45019602967 PASS on a6908c5; libdisk #45019603097 failed because the expected call literal retained whitespace after the actual source was compacted. The follow-up checker correction is included in parent a6908c; its new exact-head libdisk run #45019713462 was QUEUED at last inspection.
- Exact-head Actions for THIS COMMIT must be fetched after publication. XY-C1 L3 remains unverified until Build, TRX and artifact upload all pass on one identical product-source SHA; hardware L4 remains NOT VERIFIED.
- Next bounded task: fix del_account()'s account-list length leak and checked per-volume metadata unlink/path handling without silently leaving cleanup failures unreported.

---
## 2026-11-22 20:06 CST (UTC+8) — Fix the remaining libdisk source-contract whitespace mismatch

- Parent main SHA: 00e3dd6c9bdfa3e5186a25b3c69524bafae4d7d0. On #45019603097, both GNU89 ASan/UBSan binaries passed, but the source contract still failed because the actual function call was whitespace-compacted while the expected-call literal was not. Normalize both sides before comparison; no C behavior or safety assertion is relaxed.
- The AI handoff contract #45019602967 passed on the same parent after restoring the required mandatory checkpoint and cross-session sections.
- The first attempt at removing source whitespace was incomplete: the expected call string itself retained spaces after commas. The checker now removes whitespace from both the actual and expected strings. Exact-head libdisk validation must pass on this new commit before moving the account lifecycle fix into main.
- Next source slice remains add_account(): propagate all mounted-volume permission-file initialization failures, continue independent volumes, and return the first failure only after cleanup/reloading Samba.

---
## 2026-11-22 20:02 CST (UTC+8) — Correct libdisk checker normalization and preserve mandatory handoff checkpoint

- Parent main SHA: 68b4958d3b6bc6ee3636e8ff1f479fbabdecc679. Exact-head libdisk run #45019373684 passed both GNU89 ASan/UBSan executables but failed the Python source checker because " ".join(split()) left a space after the updater function's opening parenthesis. The checker now removes all whitespace before comparing its compact call contract; this preserves the behavioral gate rather than relaxing it.
- Exact-head AI handoff run #45019373672 failed because SESSION_HANDOFF.md omitted the literal mandatory checkpoint marker and the cross-session execution sections required by the repository validator. Restored the mandatory handoff checkpoint, atomic-main-commit rule, verification boundary and exact first action while recording the real next account lifecycle task.
- On parent SHA 68b4958d3b6bc6ee3636e8ff1f479fbabdecc679, Process lifecycle inventory #45019373666 PASS and HTTPD Authentication/Security #45019373705 PASS. XY-C1 #45019373712 and NVRAM audit #45019373665 were in progress at the last inspected poll; re-fetch live status after this commit.
- The parent CI failures were checker/documentation issues, not GNU89 sanitizer failures. This new commit must obtain fresh exact-head libdisk and handoff Actions before the host verification state is called PASS.
- Next source slice after this check: fix add_account() ignored per-volume permission-file initialization status; then separately fix del_account() unchecked metadata path/unlink and account-list free-count leak. Preserve current NVRAM ordering unless evidence supports a safe transaction change.

---
## 2026-11-22 19:55 CST (UTC+8) — Continue libdisk permission cleanup after per-file failures

- Parent main SHA: fa49d01bc7884e68e5adcd3832d2b0480bfaa974. Folder deletion and rename already used per-file atomic replacement, but returned at the first account read/rewrite/write failure. One damaged metadata file could therefore prevent later account files and anonymous FTP from being reconciled.
- Added a shared bounded-path permission updater. It skips only missing files or missing records, preserves other read/rewrite/write failures, and publishes complete records atomically. del_folder() and mod_folder() now attempt every independent account plus anonymous FTP, retain the first error, and report it after cleanup.
- get_permission() now returns an error when its account metadata path cannot be constructed; it no longer treats a truncated pathname as a zero-permission grant.
- Updated the libdisk source contract for shared update ordering and first-error aggregation. The parent run #45019075810 exposed an undefined SHARE_TEST name after sanitizer binaries passed; this checker-only failure is fixed alongside the source change. Previous checker-only failures from stale rmdir/account-inventory assumptions were also resolved.
- Exact-head Actions for THIS COMMIT must be checked after publication. XY-C1 L3 still requires Build, TRX and artifact upload all PASS on one identical product-source SHA. Hardware L4 remains NOT VERIFIED.
- Next task: inspect add_account() and del_account() for ignored per-volume permission initialization/removal failures; propagate failures without leaving NVRAM account state inconsistent.

---
## 2026-11-22 — Propagate account permission bootstrap errors

- Latest product source SHA: `c08483beb90245f70fabe5edb74fa6e6658f2bb6`.
- `set_permission()` no longer ignores a failed `initial_one_var_file_in_mount_path()` or sleeps a fixed second before retrying. It returns immediately on initialization failure, then verifies the subsequent file read before editing permission data.
- The adjacent bounded-path helper ensures both permission reads and writes reject a truncated account-var pathname; read failures remain fail-closed and write/initialization failures are explicit.
- Exact-head Actions at this checkpoint:
- Firewall Startup Stability Regression #45018314341: IN_PROGRESS.
- Process inventory and lifecycle fail-closed regression #45018314337: IN_PROGRESS.
- HTTPD Authentication and Security Regression #45018314301: IN_PROGRESS.
- Process lifecycle inventory - c08483beb90245f70fabe5edb74fa6e6658f2bb6 #45018314273: IN_PROGRESS.
- XY-C1 Firmware Validation #45018314239: IN_PROGRESS.
- NVRAM transaction boundary audit #45018314219: PENDING.
- WAN route operation status regression #45018314198: IN_PROGRESS.
- HTTPD daemon lifecycle stability #45018314192: IN_PROGRESS.
- WAN DHCP callback state stability regression #45018314188: IN_PROGRESS.
- FULLCONENAT lifecycle regression #45018314180: IN_PROGRESS.
- IPv6 service-type checked-read regression #45018314178: IN_PROGRESS.
- Keep L3 unverified until same-SHA XY-C1 Build, TRX verification and artifact upload all pass; hardware L4 remains NOT VERIFIED.

---

## 2026-11-22 — Bound permission paths and make add-folder setup fail closed

- `fb430cb0adea84a06b14ef6f33579ea42e7d8106` hardened `read_whole_file()`: 1 MiB bound, EINVAL/EFBIG/EIO/ENOMEM handling, checked successful close and errno-preserving failure cleanup; added GNU89 ASan/UBSan regression for normal, empty, invalid, missing, oversized and directory-read cases.
- The new full-source `-Werror` test surfaced `sanity_name()` comparing signed `char` to 0x84. `29187fdd429a6475b5d7fec39259653ac19776b2` fixes the unsigned-byte comparison and tests it.
- `add_folder()` was rewritten to validate/allocate the mount path safely, fail closed on folder-list lookup errors, atomically publish the updated folder-list snapshot, then use `create_if_no_var_files()` for checked per-account metadata setup. It no longer ignores account initialization failures.
- Current path-bounds follow-up `9c9f120422bc5b3bd025a7a6325dacf12dba4a84` adds `disk_share_format_account_var_path()`, rejects snprintf truncation with ENAMETOOLONG, and uses it for both permission reads (deny on invalid path) and writes (return error). Regression covers normal formatting, small-buffer truncation and invalid arguments.
- Exact-head Actions at this checkpoint:
- NVRAM transaction boundary audit #45018180705: IN_PROGRESS
- HTTPD Authentication and Security Regression #45018180700: COMPLETED/SUCCESS
- libdisk share-list output safety #45018180692: COMPLETED/SUCCESS
- Process lifecycle inventory - 9c9f120422bc5b3bd025a7a6325dacf12dba4a84 #45018180618: COMPLETED/SUCCESS
- XY-C1 Firmware Validation #45018180615: IN_PROGRESS
- Same-SHA XY-C1 Build/TRX/artifact is still required for firmware L3; do not infer it from host sanitizer checks. Hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Harden libdisk file reads and add-folder initialization

- `fb430cb0adea84a06b14ef6f33579ea42e7d8106` added bounded, errno-preserving `read_whole_file()` handling: invalid arguments return EINVAL, files above 1 MiB return EFBIG, read errors preserve their cause, and successful reads must also close successfully. Added GNU89 ASan/UBSan coverage for text, empty, missing, oversized and directory-read cases, plus exact-source contracts and workflow triggers.
- The first compile exposed another existing warning in `sanity_name()`: plain signed `char` was compared with byte 0x84 and GCC correctly rejected the always-false comparison under `-Werror`. `29187fdd429a6475b5d7fec39259653ac19776b2` casts to `unsigned char` and adds a high-bit-byte regression.
- Current product follow-up `43a5bfb5bc3f0195abfc00db9e810e58ad3514a1` hardens `add_folder()`: it fails closed on folder-list lookup errors, checks path-length arithmetic and mkdir, publishes the new folder list before permission initialization, then delegates all account-file setup to the already checked `create_if_no_var_files()` flow. Metadata-stage failures are now returned rather than silently reporting success.
- Latest exact-head Actions observed before this documentation checkpoint:
- `Process lifecycle inventory - 43a5bfb5bc3f0195abfc00db9e810e58ad3514a1` #45018022701: IN_PROGRESS.
- `XY-C1 Firmware Validation` #45018022664: IN_PROGRESS.
- `NVRAM transaction boundary audit` #45018022657: IN_PROGRESS.
- `libdisk share-list output safety` #45018022649: IN_PROGRESS.
- `HTTPD Authentication and Security Regression` #45018022644: IN_PROGRESS.
- XY-C1 is still the required L3 gate for the product SHA `43a5bfb5bc3f0195abfc00db9e810e58ad3514a1`: Build XY-C1, TRX verification and artifact upload must all PASS on the same SHA. Hardware L4 is NOT VERIFIED.

---

## 2026-11-22 — Make volume share permission metadata fail closed

- Product commits: `f39bc0c3e2bdc149489b2f596cb3ae1859d8020f` (atomic initial account defaults), `49e6cbffc73373c8b292159c2984cdafa72697c4` (atomic new-folder permission updates), and `2c099f7a36ae84a2baf375e103ce4c7db8450238` (checked metadata cleanup and caller error propagation).
- `initial_one_var_file_in_mount_path()` now writes the complete permission list to a same-directory temporary file; buffered writes, fsync, close and rename are checked, preserving the last-good file if storage fills or a write fails. GNU89 ASan/UBSan libdisk run #45016346157 PASS on `f39bc0c`.
- `modify_if_exist_new_folder()` now treats a failed folder-list lookup as an error (rather than silently skipping it), preserves the old permission file, and atomically publishes the copied file plus new default records. Exact libdisk run #45016518545 PASS on `49e6cbf`.
- `clear_share_var_files()` now distinguishes directory EOF from readdir failure, checks managed-file unlink and closedir outcomes, and reports errors. `initial_all_var_file_in_mount_path()` and `create_if_no_var_files()` now propagate every stage's failure, validate path truncation, and free account lists on error. Exact libdisk run #45016804441 PASS on `2c099f7`; Process inventory #45016804504 PASS.
- XY-C1 run #45016804450 for `2c099f7` was IN_PROGRESS in Build XY-C1; TRX and artifact upload pending. HTTPD #45016804458 and NVRAM #45016804472 were IN_PROGRESS.
- Firmware build exposed an independent pre-existing syntax defect introduced in detect-WAN commit `00fb9c3`: `chk_udhcpc()` had an unmatched closing brace before its final return. Commit `07fb4ceef4be3efffd1cc32269eab762dde9e81b` removed the stray brace, normalized the block and added a source-contract final-return assertion. Exact detect-WAN #45016664463 PASS, Process inventory #45016664378 PASS, Process lifecycle #45016664391 PASS, AI docs #45016664392 PASS; same-SHA XY #45016664382 still building.
- XY-C1 #45015894601 on earlier source SHA `9228416` FAILED in cross-build at `detect_wan.c:169`; root cause is the unmatched brace above, not the libdisk metadata change. Do not claim firmware L3 until the corrected current source SHA's Build, TRX and artifact upload all PASS. Hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Write folder-list metadata as an atomic checked snapshot

- Parent main SHA: `75f461de7dfbfbd0f59867959c5b3d37042d0b0f`; product commit: `922841600a62aa8c4c98922da9002c6e4cdfb328`. `initial_folder_list_in_mount_path()` previously truncated the destination directly and ignored every `fprintf()` and `fclose()` failure. A full disk or interrupted write could leave an incomplete but existing metadata file, suppress future regeneration and keep Samba share discovery broken.
- Added a checked snapshot writer, same-directory `mkstemp()`, flush + `fsync()`, checked close and atomic `rename()`. Failure removes only the temporary file and preserves errno, leaving any previous complete list intact.
- GNU89 ASan/UBSan regression now covers complete output, invalid inputs and a failing `/dev/full` stream; the source contract verifies write/flush/sync/close/rename ordering and failure cleanup.
- Exact product SHA: libdisk share-list #45015894653 PASS. XY-C1 #45015894601 IN_PROGRESS in Build XY-C1; Process inventory #45015894636, HTTPD security #45015894612 and NVRAM audit #45015894614 were IN_PROGRESS at last poll. No L3/L4 claim.

---

## 2026-11-22 — Make libdisk readdir source contract whitespace-independent

- Product commit `c24dd1f0a513864249ae7817db7d3ddc6077f21b` compiled and passed the GNU89 Address/UndefinedBehaviorSanitizer policy regression. The exact libdisk workflow #45015397360 then failed only in the Python source checker: its locator expected a space before the call's closing parenthesis, while the production call had the normal `&read_error);` layout.
- Replaced the brittle literal locator with a whitespace-tolerant regular expression. The existing order assertions still require the adapter call, clean-EOF branch, error branch and errno propagation in that order; this is a checker-only correction, not a relaxation of product semantics.
- XY-C1 run #45015397223 for the same product SHA `c24dd1f` remained IN_PROGRESS in Build XY-C1 at last poll; TRX and artifact upload await that build. Process inventory #45015397306, Process lifecycle #45015397230, HTTPD security #45015397324 and AI handoff docs #45015397283 PASS. NVRAM audit #45015397192 remained IN_PROGRESS. Hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Fail closed on libdisk directory enumeration errors

- Source commit: `c24dd1f0a513864249ae7817db7d3ddc6077f21b` (parent `faed38733360380c55b605c0ec9a0f34538877b4`). `get_all_folder_in_mount_path()` previously treated every `readdir()` NULL as clean EOF, so a directory read error could publish and persist an incomplete share list. The new injectable status policy distinguishes entry, clean EOF and I/O failure; production code frees partial results, preserves errno across cleanup, and only publishes after successful enumeration/close.
- Added GNU89/ASan/UBSan policy cases for entry, EOF, EIO and invalid adapters. The source contract checks fail-closed ordering and protects the XY-C1 production-input path filter.
- Corrected the previous handoff's incorrect statement that libdisk sources were already watched by XY-C1. The main-push filter now includes the libdisk Makefile, `disk_share.c`, `disk_share.h` and installed `disk_share_list_policy.h`; the focused libdisk workflow continues to cover push and PR host tests.
- Exact source SHA Actions at last poll: libdisk share-list #45015397360 IN_PROGRESS; XY-C1 #45015397223 IN_PROGRESS with Build XY-C1 running; Process inventory #45015397306 IN_PROGRESS; Process lifecycle #45015397230 IN_PROGRESS; AI docs #45015397283 IN_PROGRESS; NVRAM #45015397192 QUEUED; HTTPD #45015397324 QUEUED. No firmware L3 or hardware L4 claim until same-SHA Build/TRX/artifact all pass.

---

## 2026-11-22 — Handoff and CI checkpoint refresh

- Parent main SHA: `1991166437f832772a8c9a0dc785f5a1f12d78c3`; frozen backup branch remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- The libdisk output-initialization implementation, Samba caller initialization, GNU89 sanitizer regression and focused workflow are committed. `<stddef.h>` was added after the first sanitizer job exposed the missing `NULL` definition.
- The XY-C1 path filter was extended to schedule firmware validation for `disk_share.c`, the new policy header and its regression test. Exact results are recorded in `SESSION_HANDOFF.md`; no firmware L3 or hardware L4 claim is made here.
- RC DDNS restart now fails closed when `ddns_enable_x` cannot be read. Inadyn, DNSMasq and WAN source checkers were corrected to normalize C whitespace and assert the actual `goto failed` provider-repair path.

---

## 2026-11-22 — Initialize libdisk share-list outputs before validation

- Parent main SHA: `3c08537210948fdc301203f854255320b55bbfba`. Both `get_folder_list_in_mount_path()` and `get_all_folder_in_mount_path()` returned on a NULL/empty mount path before resetting caller outputs. Samba's `write_smb_conf_unlocked()` declared `folder_list` uninitialized and then passed it to `free_2_dimension_list()` on the API error path.
- Added a shared GNU89-compatible output-preparation policy used by both APIs; it clears every supplied output before validating arguments. Also initialized Samba's local pointer defensively.
- Added ASan/UBSan regression coverage for NULL/empty mount paths, missing output parameters and valid-path initialization, plus a focused source-contract workflow. Device-level filesystem and router behavior remain unverified.

---

## 2026-11-22 — Initialize optional Internet LED transition setting

- Parent main SHA: `896500e1db96808f3a830386a2dcf41b2c821dc1`. Initialized the timer-local WAN LED mode to its legacy disabled default before the conditional checked read. The value is consumed only after a successful transition preflight; explicit initialization also keeps compiler data-flow analysis clear in non-optimized GNU89 builds.
- No behavior change; exact-head CI pending.

---

## 2026-11-22 — Check runtime Internet probe cadence and transition settings

- Parent main SHA: `060503f762d22fe0d847f05851981c005792c0ac`. After endpoint reads were made fail-closed, `di_on_timer()` still used unchecked NVRAM integer getters for connect timeout, success/failure cadence, WAN LED mode and lost-delay policy. A backend error could silently select default timing or partially commit a connectivity transition. Resume also read `di_notify_delay` without preserving backend errors.
- Replaced these remaining runtime reads with the checked integer helper. Timeout and cadence are validated before network work; LED/lost-delay settings are validated before changing `di_status` or publishing the transition; resume delay is checked before scheduling normal operation. Defaults/ranges remain unchanged and failed reads retry after one second without a partial transition.
- Added GNU89 ASan/UBSan backend-error regression and exact-source order assertions. CI for this new commit pending. Hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Snapshot all Inadyn configuration keys in one checked NVRAM transaction

- Parent main SHA: `31b13e1d0bf8f1dd055d5ebd7cea7eaa662b13bb`. `write_inadyn_conf()` mixed individually checked and unchecked getters: period/forced/check-IP could silently turn backend errors into defaults; provider, hostnames, credentials and custom endpoints used rotating-cache `nvram_safe_get()` pointers. Independent reads could mix generations, and a transient NVRAM failure could overwrite the last-known-good DDNS config with empty or partial values.
- The writer now copies all 22 DDNS config keys under one exclusive NVRAM transaction into bounded owned buffers before parsing or staging. Read, transaction-close, truncation and strict integer-validation failures abort before staging; the previous config remains intact. Existing period/forced out-of-range fallback behavior is retained.
- DDNS enable state now uses `nvram_get_checked()` and a tested status parser rather than collapsing read failures into “disabled”.
- Added GNU89/ASan/UBSan coverage for rotating-cache overwrite, read failure and transaction-close failure (each invalidates the whole snapshot), plus enable-key status cases. WAN route workflow watches config helper/test paths on push and pull_request and executes the new regression; Python source checks enforce key coverage, transaction adapters, no lossy getters and snapshot-before-stage ordering.
- Exact-head WAN route/Process/AI/XY-C1 Actions pending. No local checkout is available, so no local test execution is claimed. Hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Fail closed and retry when Internet probe settings cannot be read

- Parent main SHA: `a5c583ca01a807501388a246cb5e4c7b32071fab`. `di_load_settings()` used lossy `nvram_safe_get()` / `nvram_safe_get_int()` for its six probe endpoints and polling mode. A transient backend read/close error could be mistaken for a missing or invalid endpoint; if all six addresses looked empty, the monitor silently substituted Google DNS and could incorrectly infer WAN state from a probe the user had not configured.
- Added a checked endpoint policy that snapshots each address before reading its port, preserves legacy empty/malformed/out-of-range defaults, and propagates backend failures and address truncation. `di_load_settings()` now builds a local candidate and publishes it only after every address/port and polling-mode read succeeds. Startup, resume and timer refresh paths retry after one second and never probe with a failed/incomplete settings snapshot.
- Added GNU89 ASan/UBSan regression coverage for shared-cache reuse, missing/default values, malformed/out-of-range ports, backend errors and truncation; wired it into the existing detect-internet workflow with source-order contracts. Exact-head CI pending. Hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Fix detect-WAN compact-source assertion

- Parent main SHA: `00fb9c342a11e5be57b6662d9442adacbcd20fd4`. Exact detect-WAN run #45011007427 compiled and ran the checked-NVRAM sanitizer regression successfully, then failed at the source checker because it compared a whitespace-compacted function body against a string containing a space after `return`.
- Corrected that one checker literal to the compact form. No production or test code changed; exact-head detect-WAN rerun pending.

---

## 2026-11-22 — Snapshot detect-WAN gateway identity before process scan and ARP setup

- Parent main SHA: `1f7add7650acf76f7eb0c3d37cf9e7530612ded9`. `chk_udhcpc()` retained a shared-cache gateway pointer across `process_inventory_scan()`, and its AP path used a lossy unchecked read. `arpping_gateway()` also read WAN/AP gateway and hardware addresses through unchecked pointer-returning getters, so NVRAM backend failures or oversized values could be mistaken for normal no-response behavior.
- The DHCP monitor now checks WAN protocol selection and copies the gateway into a bounded owned buffer before scanning process state. ARP probing now checked-copies gateway and hardware address in both router-WAN and AP modes and returns a local probe error before socket setup on read/truncation failure.
- Added a shared-cache overwrite regression, wired the NVRAM sanitizer test into the detect-WAN workflow, and added source contracts for both production paths. Exact-head detect-WAN / WAN DHCP / XY-C1 validation pending. Hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Load LAN bridge teardown regression source in Process integration contract

- Parent main SHA: `c0ccc7d14448edccacda7b9a62719f96840bcc23`. Process lifecycle regression run #45010423695 on product source SHA `587427b3e8952a8bdefcd2bf8ec32c1c6ca2669c` completed all preceding host C/shell and integration checks, then failed at `scripts/check_process_lifecycle_integration.py:1009`: the new LAN teardown assertions referenced `bridge_init_regression` without loading `tests/bridge_init_policy.c`.
- Added an explicit read of the existing bridge-init policy regression source before those assertions. This restores the intended test-presence checks; it does not weaken source/behavior assertions or change production code.
- The paired stale WAN route contract was independently corrected on main in `c0ccc7d`; exact route rerun #45010773255 PASS. ZeroTier lifecycle + host race regression + Python source contract PASS on exact source/checker SHA `2db8f2afc34cfa133e8f0940d3f99f90d99a43ba` (run #45010582528); AI handoff #45010582559 PASS. XY-C1 run #45010582583 remained in Build XY-C1 at last poll, with TRX and artifact steps pending. Exact Process rerun for THIS checker change pending. Hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Align route contract with checked AP gateway preflight

- Parent main SHA: `2db8f2afc34cfa133e8f0940d3f99f90d99a43ba`. Exact WAN route run #45010582579 on this SHA failed in the route source checker because it still searched for `nvram_safe_get("lan_gateway_t")` in AP `stop_lan()`. The checked bounded snapshot is now the intentional preflight gate.
- Updated the checker to require the AP gateway read, fail-closed return before route operations, and the existing route-delete/rollback/bridge-down sequence. WAN DHCP callback #45010423606 and Process lifecycle inventory #45010423500 passed on the previous source SHA. Exact route rerun pending; XY-C1 fan-out pending.

---

## 2026-11-22 — Derive uploaded ZeroTier planet file and NVRAM from one snapshot

- Parent main SHA: `587427b3e8952a8bdefcd2bf8ec32c1c6ca2669c`. Review of the remaining uploaded `/etc/storage/planet` writer found a transaction-coherence race: it Base64-encoded the upload first, then copied the upload in a second read. If the source changed between those reads, the persistent `planet` file and `zerotier_planet` NVRAM value could represent different contents despite a successful commit.
- The import now copies the upload once into its private same-filesystem stage and derives Base64 from that staged snapshot before publishing either value. Rollback behavior and source deletion ordering are unchanged.
- Added a shell regression that replaces the upload immediately after Base64 reads the staged file; it proves the published file and persisted Base64 still match the original private snapshot. The lifecycle checker asserts copy → encode-stage → file publish → NVRAM publish ordering. Exact ZeroTier Actions pending; no L3/L4 claim.
 
---

## 2026-11-22 — Preserve LAN teardown cleanup identity read failures

- Parent main SHA: `f475d1f3d10d8e7eb8cd028520aa57704a8d6062`. `stop_lan()` still used lossy `nvram_safe_get()` for two teardown-critical identities. AP mode copied `lan_gateway_t` before pausing the WAN detector; a failed read looked like an empty gateway and could skip deleting the installed default route. Router mode used `lan_ipaddr_t` to scope conntrack flush; a failed read silently skipped cleanup and left `conntrack_status == 0`, allowing teardown to report success.
- AP mode now performs a checked bounded gateway snapshot before any pause/stop side effects and aborts with the exact read status. Router mode checked-copies the old address; on read error it records the error as conntrack cleanup status but continues static-route, IPv6 and bridge cleanup, preserving independent teardown progress.
- Added cache-overwrite and backend-failure regressions, plus Process source-order checks for AP preflight and router cleanup aggregation. Process/XY-C1 exact-head runs pending. Hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Align WAN callback inline contract with checked hostname reads

- Parent main SHA: `fc0b17fd3c1f7b72fada4929ec1160ebe0ce3844`. Exact WAN DHCP callback run [45010147937](https://github.com/Arlene206/padavan-4.4-2/actions/runs/45010147937) passed all twenty earlier regression/source steps, including LAN deconfig, DHCP dltime and MTU-withdrawal contracts, then failed in the final inline source contract at Python line 7. The only failing assertion still required the removed combined `!lan_hname || !is_valid_hostname(lan_hname)` expression in `net_lan.c`.
- Replaced that stale condition with explicit checks that `get_our_hostname()` is checked, NULL preserves its errno (fallback `EIO`) and returns, while non-NULL hostname values are still validated. Stronger statement ordering remains enforced in `scripts/check_lan_dhcp_deconfig.py`; no product source changed.
- Exact preceding tests on this SHA passed; this workflow contract was the sole observed failure. Exact-head rerun pending. XY-C1, NVRAM audit and Process lifecycle were previously in progress on product source SHA `452f666cd126e06284c0607fd6b6bbbcabc1bbe1`; L3/L4 not claimed.

---

## 2026-11-22 — Update DHCP MTU checker boundary after checked LAN snapshot

- Parent main SHA: `452f666cd126e06284c0607fd6b6bbbcabc1bbe1`. Exact WAN DHCP run #45010044573 passed the GNU89/ASan/UBSan NVRAM and lease tests plus LAN deconfig and dltime contracts, then `scripts/check_wan_dhcp_mtu_withdrawal.py:54` failed because it used the old lossy LAN `nvram_safe_get()` assignment as the end boundary of the IPTV MTU input block.
- The production code now performs checked LAN subnet snapshots before the overlap test. Updated the MTU checker to delimit its MTU-only source region at that first checked snapshot call; dedicated DHCP source-order assertions cover the new snapshot behavior. No production source change in this commit. Exact-head WAN DHCP rerun pending.

---

## 2026-11-22 — Check LAN subnet reads before WAN and IPTV DHCP address mutation

- Parent main SHA: `af02de689f5b2af8ee10011b342b68b199dc21f0`. Both `udhcpc_bound()` and `udhcpc_viptv_bound()` used lossy `nvram_safe_get()` for the router LAN address/netmask overlap check. A backend read/close error could appear as an absent value and let DHCP proceed without a trustworthy subnet-conflict check.
- Both callbacks now copy the LAN address/netmask into bounded `INET_ADDRSTRLEN` buffers with the checked NVRAM getter, returning the first error before subnet comparison, interface mutation, route transition or lease publication. Missing optional keys retain the prior empty-string behavior.
- Added shared-cache overwrite coverage and exact source-order assertions for both WAN and IPTV DHCP paths. Exact-head WAN DHCP/route/XY-C1 validation pending. Hardware L4 NOT VERIFIED.

---

## 2026-11-22 17:55 CST (UTC+8) — Align LAN DHCP deconfig checker with hostname error propagation

- Parent main SHA: `c15848a370ea022288217c9c1a9e298989233152`. Exact WAN DHCP callback push run [45009805732](https://github.com/Arlene206/padavan-4.4-2/actions/runs/45009805732) passed the USB identity, interface-name, DHCP lease and WAN NVRAM host-regression executables, then failed in `scripts/check_lan_dhcp_deconfig.py:413`. The checker still required the pre-fix combined condition `!lan_hname || !is_valid_hostname(lan_hname)`.
- The current `create_hosts_lan()` correctly treats a NULL hostname as a checked NVRAM read failure: it preserves `errno` (fallback `EIO`) and returns before IPv6 probing or stage-file creation; only present hostname text reaches `is_valid_hostname()`. Updated the Python contract to enforce that error-preserving order and forbid the stale combined condition. No production source behavior changed.
- Actions already passed the preceding GNU89/ASan/UBSan C tests on exact SHA `c15848a370ea022288217c9c1a9e298989233152`; the Python contract itself failed as above. Local checkout unavailable; exact-head rerun pending. XY-C1, Process lifecycle and NVRAM audit for this exact SHA were still in progress at last inspection; L3/L4 NOT VERIFIED.

---

## 2026-11-22 — Correct WAN DHCP fallback-netmask regression contract

- Parent main SHA: `1f47c1c34aab6888b3740e4ce6f5625bcaf8254c`. The new WAN DHCP test had a test-only overlapping source/destination buffer in its fake shared-cache reader; changed subsequent reads to a separate destination so the regression models cache overwrite without relying on overlapping `memcpy` behavior.
- Exact WAN DHCP run #45009574727 exposed a checker NameError: the new test assertion referred to `lease_policy_test`, which is the unrelated DHCP lease-policy test, not the NVRAM status test. The contract now reads the correct `tests/wan_nvram_status_policy.c` source explicitly.
- The product change remains the checked bounded fallback netmask copy from `0683a314...`; this commit repairs the regression harness. Exact-head WAN DHCP validation pending.

---

## 2026-11-22 — Preserve hostname NVRAM read errors through file publishers

- Parent main SHA: `0683a3146db2928f7edfe4ad53de757218eaabf9`. After `get_our_hostname()` was changed to use checked NVRAM copies and return NULL on backend failure, both LAN hostname publishers still folded that result into generic `EINVAL`. This lost the actual storage/read error, obscuring transient backend failures and preventing callers from distinguishing invalid input from failed configuration reads.
- `create_hosts_lan()` and `update_hosts_router()` now capture `errno` (falling back to `EIO`) and return it before IPv6 probes, domain reads, temporary file creation or publication. Invalid address/domain/hostname text still returns `EINVAL`.
- Updated the HTTPD cross-workflow contracts to enforce error preservation and side-effect ordering. Existing NVRAM-copy regressions cover getter failures; exact-head HTTPD/XY-C1 validation pending. Hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Copy fallback WAN DHCP netmask before previous-lease reads

- Parent main SHA: `afb7b19f7d91b61912abfbc89da4032dcb24e59b`. In `udhcpc_bound()`, when the DHCP environment omitted `subnet`, the callback retained `nvram_safe_get("<wan>_netmask")` storage across its subsequent transactional snapshot of the previous lease. That snapshot reads many NVRAM keys and can overwrite the shared cache before the fallback mask is passed to subnet comparison/interface configuration.
- Replaced the borrowed pointer with a bounded `INET_ADDRSTRLEN` buffer and checked `rc_wan_nvram_copy_value()`. Backend errors and truncation now return before lease validation or interface/route changes; a missing optional key still maps to empty text and is rejected by the existing netmask validator.
- Added a shared-cache overwrite regression and WAN DHCP source-order assertions. Exact-head WAN DHCP/route/XY-C1 validation pending. Hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Snapshot LAN gateway and router domain before resolver/IPv6 reads

- Parent main SHA: `bccd3fc915e963d4cb6ddeab223f02edf3fef57b`. Two remaining cache-lifetime defects were found in the LAN resolver/hostname path: `lan_up_manual()` kept `lan_gateway` borrowed while later reading both DNS server keys and then used it as fallback; `update_hosts_router()` kept `lan_domain` borrowed across IPv6 service-state probing before writing hostname files.
- Both now use `rc_wan_nvram_copy_value()` into bounded owned buffers and abort before resolver/hostname side effects on read or truncation errors. Missing NVRAM keys still become empty strings, preserving the existing fallback behavior.
- Added cache-overwrite regressions; updated Process and HTTPD source contracts. Exact-head Process/HTTPD and XY-C1 validation pending. Hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Align HTTPD security cross-check with AP host snapshot

- Parent main SHA: `2f6231349eaf3fafe0c1885a95afa887d82c11c5`. HTTPD Security run #45008874461 on `56137665cea32ba5d5befce235889f996e4c9fa3` passed the DHCP lease tests, then stopped at the stale AP hostname source assertion in `.github/workflows/httpd-security.yml` (embedded Python line 154). It still required the two direct `nvram_safe_get()` arguments removed in `update_hosts_ap()`.
- Updated the contract to require ordered checked copies into owned `lan_ipaddr` / `lan_dname` buffers before `create_hosts_lan`, and to forbid direct borrowed NVRAM pointers. Exact-head HTTPD rerun pending.

---

## 2026-11-22 — Update XUPnPD integration check for owned LAN address

- Parent main SHA: `3664d438d7b2ed36957b45b91b8e647f1a592deb`. The Process integration checker still required the old direct `xupnpd_config_valid_ipv4(nvram_safe_get(...))` expression after `start_xupnpd()` was correctly changed to a bounded checked copy. Exact Process run #45008874486 on `56137665cea32ba5d5befce235889f996e4c9fa3` completed the preceding regression and source checks, then failed specifically at `scripts/check_process_lifecycle_integration.py:406`.
- Updated that assertion to require the checked NVRAM copy, the owned `lan_addr` validation, and absence of the borrowed-pointer pattern. This is a checker update, not a production-code rollback. Exact rerun pending.

---

## 2026-11-22 — Copy udpxy argv options before the NVRAM cache can wrap

- Parent main SHA: `56137665cea32ba5d5befce235889f996e4c9fa3`. `start_udpxy()` passed three `nvram_safe_get()` pointers directly through the variadic `eval()` call. These pointers are backed by the shared NVRAM cache ring and can be invalidated by later reads before exec constructs argv.
- The enable port is now serialized from its already-parsed integer; client limit and renew interval are copied with checked bounded reads into owned buffers before launch. Read/truncation errors abort startup instead of passing stale or mixed arguments.
- Added cache-overwrite regression coverage and Process source-contract assertions; removed two accidental dead-negative source assertions from the preceding XUPnPD checker. Exact-head Process and XY-C1 validation pending. Hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Own hostname and XUPnPD address across NVRAM reads

- Parent main SHA: `c4bc1f86973e9f45fd53f74c7bd613edc48da6d7`. `get_our_hostname()` kept the shared-cache pointer for `computer_name` while falling back to `productid`, and returned a borrowed cache pointer to callers. `start_xupnpd()` retained `lan_ipaddr_t` across later port/enable NVRAM reads before serializing its URL.
- Replaced both with bounded checked copies into owned buffers. Hostname read errors now return NULL (the caller already rejects a NULL hostname); XUPnPD configuration returns a checked error before filesystem or daemon side effects. The existing URL formatter now consumes the stable preflight address.
- Added GNU89 cache-overwrite regressions and Process source-contract assertions. Exact-head Process / XY-C1 validation pending; Hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Normalize LAN NVRAM source-contract call whitespace

- Parent main SHA: `77fcbc9cc84a55906c1bed2456e87eab1c70bf86`. Exact Process run #45008185010 on `864e6c651675f3d22f1a23ebc9b0936f06d6833a` compiled and ran the bridge-init GNU89 ASan/UBSan regression successfully, then failed in the later embedded source checker at Python line 213. The checker collapsed line breaks but expected no space after the helper's opening parenthesis; the C call was intentionally split after `(`, so the normalized source contained one space.
- Corrected the LAN MAC, startup identity and AP-host snapshot matchers to account for the normalized space. This is checker-only; no production behavior changed. Exact rerun is pending. Hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Snapshot LAN address, mask and AP host identity before reuse

- Parent main SHA: `864e6c651675f3d22f1a23ebc9b0936f06d6833a`. `start_lan()` retained borrowed pointers for `lan_ipaddr` and `lan_netmask`, then fetched `lan_domain` before using all three; `update_hosts_ap()` also passed two cache-backed NVRAM pointers directly as function arguments, whose evaluation order is unspecified in C.
- Replaced these reads with checked copies into bounded owned buffers before interface activation or host-file publication. Backend errors/truncation now abort the operation before its corresponding side effects; absent keys preserve the prior empty-string behavior.
- Extended the bridge-init GNU89 cache-overwrite regressions and Process source-order contract. Exact-head Process and XY-C1 validation pending; Hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Copy LAN bridge MAC before subsequent NVRAM reads

- Parent main SHA: `0f4dcd5b0ed934726680577e7d830513e5ba4a5f`. `init_bridge()` retained the borrowed `nvram_safe_get("lan_hwaddr")` pointer across switch/VLAN setup, which performs additional NVRAM reads before applying the MAC to the physical and bridge interfaces. The NVRAM getter's backing storage is a shared cache and must not be retained across reads.
- Replaced the borrowed pointer with `rc_wan_nvram_copy_value()` into a bounded owned buffer. Backend errors and truncation now abort bridge initialization before switch/bridge side effects; a genuinely absent key still maps to the legacy empty value.
- Added GNU89 regression coverage for cache-slot overwrite and checked-read failure, plus Process source-order assertions. Process and XY-C1 exact-head validation pending; XY-C1 must run Build, TRX verification and artifact upload on this production-source SHA before L3 can be claimed. Hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Keep WAN route test assertion out of trigger-path count

- Parent main SHA: `4c0906ac9c8c79eb5e6f4119a002578366fd539d`. Exact WAN route workflow #45007745240 compiled and passed the shared WAN NVRAM policy regression after the missing-route expectation correction. It then failed at the DDNS status checker because the inline assertion added a third exact quoted occurrence of `trunk/user/rc/src/tests/wan_nvram_status_policy.c`; the checker intentionally requires exactly two occurrences for push and pull_request.
- The workflow already reads the test into `wan_nvram_test`. Changed the assertion to reuse that variable, preserving both trigger filters and reducing the exact path count back to two. No production behavior changed; exact-head WAN rerun pending.

---

## 2026-11-22 — Correct missing-route snapshot regression expectation

- Parent main SHA: `d1cd34322e3ce8058cf7fc363fbabaa8b4696247`. Pre-commit review found one incorrect assertion in the new DHCP route snapshot regression: the fixture marks only `wan0_routes_rfc` absent, so the first and third route strings must remain present.
- Corrected the missing-key case to retain `wan0_routes` and `wan0_routes_ms` while asserting only the middle `routes_rfc` destination is empty. No production source change; exact-head Actions for the route snapshot remain pending.

---

## 2026-11-22 — Snapshot DHCP route options transactionally

- Parent main SHA: `789ca3e5000e677f3e3c9e46609f64e9e771b469`. The DHCP route wrappers retained three pointers returned by `nvram_safe_get()` for `routes`, `routes_rfc`, and `routes_ms` while retrieving the remaining values. Those pointers refer to reusable NVRAM cache-ring storage and are not safe to retain across a wrap or concurrent NVRAM activity.
- Replaced the three unchecked reads with a single `rc_wan_nvram_copy_snapshot()` under the existing exclusive NVRAM transaction callbacks. Each value is copied to a bounded `NVRAM_MAX_VALUE_LEN + 1` buffer before the next read; key construction is length-checked, and failed reads/transaction close abort add/delete instead of silently applying an incomplete tuple.
- Added GNU89 regression for three route option values, reusable-cache overwrite, optional missing fields, every individual read failure and transaction-end failure; extended WAN route source-order contract. Exact-head route regression pending. Hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Align IPv6 workflow with pre-mount NFS ACL snapshot

- Parent main SHA: `01ccfa38a77c2d54327a7c96dfd9c8e4c10cfe26`. Exact IPv6 service-type run #45006965726 compiled and passed the checked service-type C policy tests, then failed only in `scripts/check_ipv6_service_type_status.py` line 202. That assertion expected NFS to read its IPv6 ACL after opening `/proc/mounts`.
- The NFS ACL fix `67514d707a91ac3e34bd513032a6a85a29e0f2d3` intentionally moves LAN override/fallback, IPv6 and VPN ACL copies and validation before opening /proc/mounts and before generated-file staging. Updated the checker to assert all seven checked ACL copies precede mount enumeration/staging, preserving the security and fail-closed ordering.
- WAN DHCP callback fix `01ccfa38a77c2d54327a7c96dfd9c8e4c10cfe26` exact run #45006965680 PASS. Its broad fan-out mostly PASS; exact XY-C1 #45006965654 and NVRAM audit #45006965617 remained IN_PROGRESS at last poll. Current L3 unverified; hardware L4 NOT VERIFIED. Exact IPv6 rerun pending.

---

## 2026-11-22 — Preserve WAN DHCP hostname and vendor class across NVRAM reads

- Parent main SHA: `67514d707a91ac3e34bd513032a6a85a29e0f2d3`. `start_udhcpc_wan()` used two `get_wan_unit_value()` cache-backed pointers for `hostname` and `vci`; reading the latter can overwrite the former before DHCP client launch.
- Replaced both pointers with checked, bounded copies into owned buffers. A read error or truncation now aborts DHCP startup before spawning udhcpc; absent/empty values remain optional as before.
- Added a fake shared-cache regression verifying the hostname survives the vendor-class read and extended the WAN DHCP source-order contract. Exact-head WAN validation pending. Hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Snapshot NFS export ACL NVRAM inputs

- Parent main SHA: `1c61749ba6887639a7430b1d7d7df0b6bc661189`. The active handoff identified shared-cache lifetime hazards in `write_nfsd_exports()`: successive `nvram_safe_get()` calls returned pointers reused before IPv4 LAN fallback, IPv6 LAN and VPN ACL formatting.
- Added a checked, bounded NFS ACL string-copy policy and switched LAN override/fallback, IPv6 address/prefix and VPN subnet reads to owned buffers. All required copies and ACL validation now complete before /proc/mounts is opened or an exports staging file is created; missing keys remain empty and backend/truncation failures abort publication.
- Added GNU89 test coverage for shared-cache overwrite, fallback ACL formatting, IPv6/VPN ACL formatting, missing settings, backend errors and truncation. Extended the NFS workflow source-order contract. Exact-head NFS run pending. Hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Avoid matching comments in the FTP firewall source contract

- Parent main SHA: 2333039ea9c33c9b02f941621928f64659b249fc. Exact storage hotplug #45005813291 passed the embedded whole-caller-graph check, then failed scripts/check_storage_safe_remove.py line 654 because its negative substring assertion matched the comment mentioning nvram_match() in control_ftp_fw.
- THIS correction narrows the check to the executable call token nvram_match(" while continuing to reject actual lossy NVRAM match calls. No product-source changes.
- Exact rerun pending; NFS export ACL input-pointer lifetime audit is next. Hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Update storage firewall-refresh presence contract

- Parent main SHA: 9553ea5c4331d15f85db2842765dd82cfd428e4d. Exact 595 Storage hotplug #45005582512 failed at the embedded whole-caller-graph checker: lines 473–474 still expected Transmission and Aria start-presence to equal RUNNING, although the new source intentionally requests firewall reconciliation for every state except confirmed ABSENT.
- THIS CI-only follow-up updates both assertions to the UNKNOWN-inclusive fail-closed condition. The actual 595 caller graph had passed preceding storage lock, mount inventory, eject and Optware assertions; the dedicated source checker for the new firewall-refresh gates had not yet been reached.
- HTTPD firewall snapshot checker fix at 9553 exact HTTPD Authentication #45005694009 and AI docs #45005694083 PASS. Corrected storage workflow rerun pending; no current product L3 claim.

---

## 2026-11-22 — Update HTTPD firewall snapshot source contract

- Parent main SHA: `595b2dcb7a927a369d3e29a0aef75a5431fc7e74`. Storage firewall-refresh source commit #595 was under exact-head validation; its storage hotplug, Process, AI docs, NVRAM, Firewall Startup and XY-C1 checks were IN_PROGRESS at last inspection.
- HTTPD Authentication and Security Regression #45005080682 on firewall source SHA `cea789c171d0698f32cbe1d188ce8dda79c43353` failed at the first inline checker because it still searched for the removed `nvram_safe_get("lan_ipaddr")` assignment. The unchanged HTTPD workflow on source parent 595 was also IN_PROGRESS as run #45005582489.
- THIS workflow-only change replaces the stale locator with status-bearing copy/error-order assertions for LAN IP, manual WAN IP, WAN-unit IP and LAN netmask. It ensures all four inputs are copied and errors returned before network side effects, while retaining address validation/fallback and netmask-before-subnet behavior. Exact-head HTTPD/AI reruns pending; product behavior unchanged; hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Always reconcile firewall after storage daemon presence changes

- Parent main SHA: 005d1e340cfc7a68b0686833c8ab96d30122fe73. Exact 005 Firewall Startup Stability #45005183052 PASS and AI docs #45005183110 PASS; exact cea NVRAM audit #45005080676 PASS. Exact cea XY-C1 #45005080673 remained in progress; no current product L3 claim.
- FTP, Transmission and Aria lifecycle paths used unchecked nvram_match("fw_enable_x","1") gates before firewall refresh. FTP additionally gated on ftpd_wopen, and its restart helper did not cover ordinary daemon disappearance. Read errors could silently skip the firewall rebuild and leave stale mappings.
- THIS FIX invokes restart_firewall() on each service-presence transition without lossy enable/exposure gates; the firewall itself consumes checked configuration and handles rule publication/cleanup. Storage start/stop app groups now include FTP regardless of its exposure toggle and treat UNKNOWN process presence conservatively.
- Added integration assertions to scripts/check_storage_safe_remove.py; its existing storage-hotplug workflow path filters cover both changed files. Exact-head validation pending; hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Repair stale firewall netmask source-contract locator

- Parent main SHA: `cea789c171d0698f32cbe1d188ce8dda79c43353`. Exact Firewall Startup Stability #45005080730 failed after the new checked-IP source-contract script and all firewall C ASan/UBSan tests passed. The final inline NAT/netmask contract still searched for `lan_netmask = nvram_safe_get(...)`, which this source fix intentionally removed; Python raised `ValueError: substring not found`.
- THIS correction anchors the existing contiguity-validation ordering assertion on the checked `"lan_netmask"` read instead. No product-source behavior changes in this CI-only follow-up.
- Exact cea HTTPD lifecycle #45005080894 and IPv6 service-type #45005080692 PASS. NVRAM, XY-C1, WAN route, FULLCONENAT, Process inventory/lifecycle, HTTPD auth and WAN DHCP were still running at last poll; active L3 remains unverified.

---

## 2026-11-22 — Use checked snapshots for firewall IP inputs

- Parent main SHA: `a5777a906c785d296e8612cf4954fc2a5dbb6749`. Exact a577 USB printer tests/checker #45004808849 PASS; Process umbrella #45004808803 PASS; Process inventory #45004808838 PASS; AI docs #45004808939 PASS. NVRAM audit #45004808813, USB storage #45004808805 and XY-C1 #45004808822 were pending at last poll.
- `start_firewall_ex_locked()` used `nvram_safe_get()` for LAN IP, management WAN IP and LAN netmask, plus the legacy WAN unit IP getter. Backend errors could be collapsed to empty strings; LAN IP then fell back to `0.0.0.0`, and firewall generation could continue with unknown network inputs.
- THIS FIX reads all four settings with checked APIs and copies shared-cache values immediately. Read/close failures and truncation abort before rp_filter writes and any firewall table generation; missing keys preserve empty-string behavior, including the established invalid LAN address fallback.
- Added strict checked-string policy cases and a dedicated source-order checker integrated into the firewall startup Actions. Exact-head workflow validation pending; hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Fail closed on unreadable USB printer service configuration

- Parent main SHA: `e29431f47e51c48bcbe0e8c4cf1adec51b55f36b`. Exact e294 IAPP #45004292701, Process umbrella #45004292730, Process inventory #45004292778 and AI docs #45004292741 PASS; XY-C1 #45004292784 was building, so no same-SHA L3 claim.
- `start_u2ec()`, `start_lpd()` and `start_p910nd()` used lossy `nvram_match()` reads. A backend failure could look like a missing/default setting and start a daemon despite unknown enable/raw mode.
- This fix reads each value with `nvram_get_checked()` and copies the shared-cache value immediately. Missing keys retain the empty legacy default; backend errors, invalid result states, NULL values and overlong data abort before PID-file unlink or daemon launch. The raw spooler reads `rawd_enable` once.
- Added GNU89/ASan/UBSan tests, source-order checks, dedicated USB workflow coverage and XY-C1 path filters. Exact-head verification pending; hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Correct IAPP firewall reconciliation command and verify ACCEPT target

- Parent main SHA: `117ae6e88894827d7340cd2f53d10b5b569c86b9`. Exact 117 AI docs #45004170577 and Process inventory #45004170607 PASS; IAPP lifecycle #45004170579 failed because the appended iptables command was split after a double-backslash, causing `-m` to execute as a command.
- THIS CORRECTION places the append on one line, fixes printf's newline format and requires the saved rule to include `-j ACCEPT`, not merely an owner tag/protocol/port. A new test rejects a tagged DROP rule without adding a duplicate. The pre-existing-daemon recovery and fail-reporting tests remain.
- Exact 117 XY-C1 #45004170597 and Process umbrella #45004170630 were running at last poll. Corrected-head IAPP/Process/XY checks pending; hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Reconcile IAPP firewall rules on duplicate start

- Parent main SHA: `46a32f2eb17d2bbca8b840ea5fbea30bc76a7731`. Exact-head Process lifecycle #45003857329, Process inventory #45003857277, NVRAM audit #45003857261 and AI docs #45003857267 PASS.
- `iappd.sh start` treated a live PID as sufficient proof of healthy startup and returned before checking the owned TCP/UDP 3517 firewall rules. If a partial rollback or external rule deletion left one rule absent, subsequent starts falsely reported success and never restored connectivity.
- THIS FIX reconciles the two tagged rules on an already-running daemon, appends only a missing rule, verifies the resulting filter inventory, and returns an error when rule inspection or repair fails. Added tests for missing-UDP recovery without daemon relaunch and failed repair without false success.
- The XY-C1 push path filter omitted `trunk/user/shared/src/router_account_nvram_policy.h`; adding it ensures future changes to this linked production header trigger firmware validation. IAPP's source path triggers the exact build on this commit. Current-head firmware validation pending; hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Parse oversized account counts without temporary-buffer truncation

- Parent main SHA: `7c4121b3c2d54e82931374015488d54401b4e784`. Exact 7c Process lifecycle #45003679784 failed at the oversized count case: `"999999999999999999"` exceeded the helper's 16-byte copy buffer, so it returned `ENAMETOOLONG` before the numeric parser could return the expected `ERANGE`.
- THIS FIX consumes the `nvram_get_checked()` shared-cache pointer immediately and parses decimal digits in place. The configured maximum is checked before each multiply/add, so arbitrarily long numeric input fails with `ERANGE` without integer overflow or truncation; malformed characters fail with `EINVAL`, missing keys remain zero.
- Exact 7c AI docs #45003679754, Process inventory #45003679750, HTTPD #45003679789, WAN DHCP #45003679775, DNS forwarder #45003679778 and GPIO #45003679755 PASS. NVRAM audit #45003679838 and XY-C1 #45003679777 were in progress; exact corrected-head Process regression and XY-C1 remain pending. Hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Correct account snapshot regression and restore handoff headings

- Parent main SHA: `1ba086afc059a4188884475532011bc27581c5bc`. Exact 1ba Process lifecycle #45003414996 failed in the new unit test because a 5-byte test buffer cannot hold the five-character string `admin` plus its NUL terminator; the helper correctly returned `ENAMETOOLONG`. This follow-up enlarges the test buffer to 6 bytes.
- Static source review also removed the old `sh_num` local left behind by replacing the `nvram_safe_get_int()` loop, and initializes the password snapshot buffer for the non-`force_create` path.
- Exact 1ba AI docs #45003415008 found that the handoff rewrite had removed required section headings. Restored `Current repository state`, `Active task`, `Verification boundary` and `Exact first action` sections; no project-state claims changed except recording the two exact-head failures above.
- Exact 1ba Process inventory #45003414964, HTTPD #45003415121, NVRAM audit #45003414955, WAN DHCP #45003415094, DNS forwarder #45003414947 and GPIO #45003414983 PASS; XY-C1 #45003414994 was still running. Exact correction-SHA CI pending.

---

## 2026-11-22 — Snapshot account NVRAM values before Unix database staging

- Parent main SHA: `229ee26729a5364b7c81ca3b7a4218e72ed000fe`. Exact 229 WAN route #45002519393 and AI handoff #45002519355 PASS. Earlier a5 WAN DHCP #45002087932, Process inventory #45002087966 and AI #45002088016 PASS; XY-C1 #45002087905 completed Build/TRX/artifact on a5, so L3 is VERIFIED only for that exact SHA.
- `nvram_linux.h` documents `nvram_get_checked()` values point into a shared cache and must be copied before another read. `recreate_passwd_unix()` retained root/account pointers across subsequent reads and used `nvram_safe_get*()`, which collapses backend errors into empty/default data and could silently publish an incomplete account list or use an empty password after read failure.
- THIS COMMIT adds an injected checked-copy policy and strict bounded-count parser. It snapshots root username/password and all account entries before opening staged files; missing keys preserve defaults, while backend, overlong and malformed-count failures abort before publication. Duplicate scans use owned snapshots. Added GNU89 sanitizer tests and Process workflow path-filter/source-contract coverage. Exact new-head CI pending; hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Initialize the DHCP policy header binding in the MTU checker

- Parent main SHA: `0a21e8adab28787b58fc795c58d4a75f0c46c3cf`. Exact 874 HTTPD #45002311801 and AI docs #45002311684 PASS; 874 WAN route #45002311669 exposed a checker header-binding issue.
- Commit 0a switched the helper extractor to `DHCP_POLICY` but omitted loading that variable. Exact 0a WAN route #45002417816 was queued at last poll; the first 874 route run had already shown the owning header is `dhcp_lease_policy.h`.
- THIS COMMIT adds the missing `DHCP_POLICY = (RC / "dhcp_lease_policy.h").read_text()` binding. No production source change; exact-head route/AI reruns pending. a5 XY-C1 #45002087905 remained in Build at last detailed poll; no L3/L4 claim.

---

## 2026-11-22 — Load the DHCP option parser contract from its owning header

- Parent main SHA: `874278785995f29628f39cad26d03e6d0054758c`. Exact 874 HTTPD #45002311801 and AI handoff #45002311684 PASS.
- Exact 874 WAN route #45002311669 failed after the prior source-contract update: `check_wan_initial_mtu_status.py` called its function extractor on `wan_mtu_setup_policy.h`, but `dhcp_lease_mtu_option_value()` is defined in `dhcp_lease_policy.h`. The production parser and callback source were unchanged; this was a test-checker file-binding error.
- THIS COMMIT loads the owning DHCP policy header and extracts the parser there. No product-source change. Exact WAN route rerun pending; a5 XY-C1 #45002087905 was still building at last poll. No L3/L4 claim.

---

## 2026-11-22 — Align DHCP MTU source contracts with the shared parser

- Parent main SHA: `a5d5bb332a7b18db1676f8e182a7ddaae92bd2ff`. Exact a5 WAN DHCP callback #45002087932, Process inventory #45002087966, AI docs #45002088016, detect-link #45002088042, router-mode NVRAM #45002087993, ZeroTier #45002087815, VPN server #45002087979, miniupnpd #45002087848, DNSMasq #45002087957, watchdog #45002087992, WAN teardown #45002087970, Wireless #45002087961, USB modem #45002087840, IPv6 service type #45002087857, WAN ARP #45002087845 and detect-internet #45002087952 PASS.
- Exact a5 HTTPD #45002087808 failed only at embedded checker line 214; WAN route #45002087933 failed in the initial-MTU checker. Both still expected the 576..1500 parser call inline in `udhcpc_bound()`, while a5 had deliberately centralized it in `dhcp_lease_mtu_option_value()`. The policy regression and the WAN option-withdrawal checker both passed.
- THIS COMMIT updates these source contracts to verify the shared parser and its 576..1500 bounds; no production C behavior changes. Exact-head HTTPD/WAN route reruns pending; XY-C1 #45002087905, Firewall #45002087962 and NVRAM audit #45002087864 were still running at last poll. No L3/L4 claim.

---

## 2026-11-22 — Restore default MTU when DHCP option 26 is withdrawn

- Parent main SHA: `12f62705798d1c639c4c0d6b542f82a33fc8ed64`. Exact e72 PID1 mdev, HTTPD, Process lifecycle/inventory, NVRAM, USB storage, DNS forwarder, PID1 notification, DNSMasq and AI runs PASS; XY-C1 #45001383211 was still building at last detailed poll.
- BusyBox `fill_envp()` exports a known DHCP option only when present. Physical WAN and IPTV callbacks previously applied and compared MTU only when `getenv("mtu")` was non-NULL, so withdrawing option 26 retained the previous reduced interface MTU and stale NVRAM lease state.
- THIS COMMIT maps an absent option to 1500, always includes physical/IPTV MTU in the accepted lease snapshot, and makes both renew fast paths re-enter bound when the effective MTU differs. Empty previous snapshots mean the interface's established default 1500; malformed present options remain unsupported. Added GNU89 policy coverage and a dedicated source-order workflow contract. Exact new-head CI pending; no L3/L4 claim.

---

## 2026-11-21 23:56 PST — Harden errno-less service waits and execute the existing regression in CI

- Parent main 0170c3948ac0178250ad8508485159b2adc7e9d8; PID1 inherited-signal-reset production source is in e72 `e72a377025dbd4e42cbbfe0186689b5e6e57720b`. The later 0170 commit updated only handoff/dev-log and did not change source.
- Exact e72 L2 PASS: Process lifecycle #45001383698, USB #45001383155, PID1 notification #45001383190, DNS forwarder #45001383158, Process inventory #45001383203, NVRAM #45001383196, DNSMasq #45001383159, HTTPD #45001383163, PID1 mdev #45001383227 and AI #45001383224.
- Exact e72 XY-C1 #45001383211: production-input detector PASS; Build XY-C1 IN_PROGRESS; TRX and artifact PENDING. Not yet L3.
- `service_wait_timespec_with()` used to return -1 with errno=0 when an injected nanosleep provider failed without setting errno; it could also treat an invalid positive return as an EINTR if errno matched. Unexpected status/errno-less failure now returns EIO while legitimate errno is preserved. Added fake-provider regression.
- `service_wait_policy.c` was present in Process lifecycle workflow path filters but had no compile/run step. Added explicit GNU89 `-Wall -Wextra -Werror` + ASan/UBSan compile/run to the exact-head process lifecycle job. L4 NOT VERIFIED.

---

## 2026-11-22 — Fail closed if the recovery shell cannot reset inherited PID 1 signals

- Parent main e72a377025dbd4e42cbbfe0186689b5e6e57720b. Exact e72 XY-C1 #45001383211, Process lifecycle #45001383698, USB #45001383155, PID1 notification #45001383190, DNS forwarder #45001383158, Process inventory #45001383203, NVRAM #45001383196, DNSMasq #45001383159, HTTPD #45001383163 and PID1 mdev #45001383227 were in progress at last poll; AI docs #45001383224 PASS.
- Parent 1a5 HTTPD #45001003234 had failed at an embedded-checker NameError because pid1_test was not loaded; parent 0b22 fixed the missing test-source binding.
- Exact 1a5 XY-C1 #45001003215 was host-only: Build, TRX and artifact steps were skipped because no production build inputs changed. Not L3.
- reset_signals() previously ignored disposition-reset sigaction and signal-unblock failures, and looped over invalid signal numbers. It now resets only PID1's fatal/shutdown/event groups via the checked sigaction helper, retains the SIGCHLD reaper, returns the first error, checks sigfillset and unblocks through the checked mask helper.
- Forked console child now logs and _exit(126)s before console initialization/exec if reset fails. Fatal emergency shutdown reports reset failure and continues best-effort. mdev and HTTPD source contracts verify the signal groups, mask unblocking and child fail-closed behavior. Hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Fail closed if the recovery shell cannot reset inherited PID 1 signals

- Parent main 0b22c9729e6abce02181e96c7a60560342db2b29. Exact 1a5 Process inventory #45001003203, PID1 mdev #45001003224, AI #45001003376 and NVRAM #45001003186 PASS. HTTPD #45001003234 failed only because its embedded source contract referenced pid1_test without loading the C test file; the binding fix is in parent 0b22. Parent 0b22 AI #45001261547 PASS; HTTPD #45001261606 was in progress.
- Exact XY-C1 #45001003215 was host-only: production input detector found no build input changes; Build, TRX and artifact were skipped. Not L3.
- reset_signals() previously ignored disposition-reset sigaction and signal-unblock failures, and looped over invalid signal numbers. It now resets only PID1's fatal/shutdown/event groups via the checked sigaction helper, retains the SIGCHLD reaper, returns the first error, checks sigfillset, and unblocks through the checked mask helper.
- Forked console child now logs and _exit(126)s before console initialization/exec if reset fails. Fatal emergency shutdown reports reset failure and continues best-effort. mdev and HTTPD source contracts verify all three groups, mask unblocking and child fail-closed behavior. Hardware L4 NOT VERIFIED.

---

## 2026-11-22 16:04 CST — Load PID 1 test source in HTTPD contract

- Parent main SHA: `1a5bb1367ba92b39f46a89b7172b9f816cc19f56`. Exact PID1 mdev #45001003224, Process lifecycle inventory #45001003203 and AI handoff #45001003376 PASS.
- Exact HTTPD #45001003234 passed its GNU89/Werror PID1 policy executable and all prior checks, then failed in the embedded Python source contract with `NameError: name 'pid1_test' is not defined`. The assertion referenced the test file but this HTTPD step loaded only `init.c` and `pid1_signal_policy.h`; the parallel mdev contract already loaded the test file explicitly.
- THIS COMMIT adds the missing `Path("trunk/user/rc/src/tests/pid1_signal_policy.c").read_text()` binding. No product C behavior changed. NVRAM #45001003186 and XY-C1 #45001003215 were IN_PROGRESS at last poll; exact new-head Actions required. No L3/L4 claim.

---

## 2026-11-21 23:51 PST — Correct errno matcher in PID 1 mask contracts

- Parent main `80ad0e36e6c2ebca9dd2347f2dee484ee50915f6`. Exact 80ad Process lifecycle #45000794677 and AI docs #45000794731 PASS; NVRAM #45000794652 was still running. PID1 mdev #45000794651 and HTTPD #45000794708 failed in their newly added source contracts, not in the checked mask logic: the raw helper line is `errno = 0;`, while our matcher expected `errno=0;`. HTTPD's GNU89 C policy regression compiled and executed before the Python contract failed.
- Corrected the whitespace-sensitive matcher in both workflows. Strengthened the test to assert the adapter forwards a SIGALRM-only set on the optional-old-mask SIG_UNBLOCK path, while clearing stale EINTR.
- No production runtime change in this correction. Exact reruns are triggered by the C test path change; no L3/L4 claim.

---

## 2026-11-21 23:45 PST — Cover optional old-mask output in PID 1 policy regression

- Parent main `c9c47f065a86645dabb8155d133b07e215d3b771`. Exact 34d Process inventory #45000638051, Process lifecycle #45000637980, DNSMasq #45000637989, DNS Forwarder #45000637987, AI #45000637929 and PID1 notification #45000638026 PASS. NVRAM #45000638001, XY-C1 #45000637923 and USB #45000637930 were still running.
- 34d HTTPD #45000637963 and PID1 mdev #45000637950 failed in their new signal-mask matcher: control_signal() calls init_signal_mask_update(), which delegates to the policy helper. c9 corrects both matchers to the actual call chain.
- Extended the C regression with a successful SIG_UNBLOCK call passing old_set == NULL, asserting the helper accepts the optional output, records the requested operation, and clears caller errno. Updating the watched C test path ensures corrected mdev/HTTPD contracts rerun on one exact SHA. No L3/L4 claim.

---

## 2026-11-21 23:41 PST — Correct PID 1 signal-mask workflow matcher

- Parent main `34d7b3cb77426c41e077e47c85da81b0441560a4`. The signal-mask change was committed with mdev and HTTPD inline contracts. Review found one test-only mismatch: control_signal() calls `init_signal_mask_update()`, while that wrapper calls `pid1_signal_update_mask()`; the new assertion incorrectly expected the lower-level call in control_signal().
- Corrected both inline workflows to verify the actual wrapper call chain. No production source changes.
- Exact deaca PID1 mdev #45000188630, HTTPD #45000188641, Process inventory #45000188627, AI #45000188637 and NVRAM #45000188665 PASS. deaca XY-C1 #45000188681 was host-only (production Build/TRX/artifact skipped), not L3. The 34d exact-head workflows were queued at first poll; this test correction now triggers fresh runs.

---

## 2026-11-21 23:37 PST — Check PID 1 signal masks and avoid invalid sigsuspend state

- Parent main `deaca535575f3032b02c53a52667f8eb74494fe2`. Exact deaca PID1 mdev #45000188630, HTTPD #45000188641, Process lifecycle inventory #45000188627 and AI handoff #45000188637 PASS. Exact deaca XY-C1 #45000188681 is HOST-ONLY success: production-input detector found no firmware build inputs changed, so Build, TRX and artifact steps were SKIPPED; not L3. Exact f6a NVRAM #44999987551 PASS; deaca NVRAM #45000188665 was running at last poll.
- PID 1 main loop ignored queued-work sigemptyset/sigaddset/sigprocmask failures, could pass an uninitialized original_mask to sigsuspend, and ignored console-shell mask restore/reblock failures. The two early control_signal mask changes were also unchecked.
- Added an injectable pid1_signal_update_mask() returning errno or EIO, checked control-signal masks and router-readiness gates, mask-construction status, original-mask validity, checked block/restore/reblock transitions and a bounded polling fallback that never suspends with invalid state. Failed SIGUSR1 unblock disables router notifications. Added GNU89 regression coverage for null args, stale errno clearing on success, old-mask publication, EBUSY and errno-less EIO; expanded mdev and HTTPD source contracts. Exact-head L2 reruns pending; hardware L4 NOT VERIFIED.

---

## 2026-11-22 18:31 CST — Cover successful signal installer errno reset

- Parent main `52e6692cd806abdb4c52ae2fd42271a3f085aed9`. The 8a66 HTTPD run #44999904303 passed all preceding tests and failed on the old direct-`sigaction` expectation. The checker update in 52e has not yet been exercised because its commit touched only the workflow and documentation.
- Extended `pid1_signal_policy.c` to seed `errno=EINTR` before a successful fake sigaction and assert that the checked wrapper clears it. Updated the mdev source contract to require the new regression. Updating the watched C test forces both mdev and HTTPD exact-head tests to rerun.
- Exact 8a66 PID1 mdev #44999904281, Process inventory #44999904395, Process lifecycle #44999904325 and AI handoff #44999904265 PASS. f6a NVRAM #44999987551 remained in progress. No L3/L4 claim.

---

## 2026-11-22 18:23 CST — Align HTTPD's PID 1 signal contract with checked installation

- Parent main `f6a3c1ca6bfa0ee3a8b4b76c37d1d4dc4eefa9c4`. Exact 8a66 HTTPD #44999904303 passed NVRAM rollback, wireless atomic publication/preflight, route access, Samba/Unix identity, and other earlier contracts; it failed only in the final PID1 signal section because it still expected direct `sigaction(signals_shutdown[i], ...)` instead of the checked installer introduced in c74.
- Replaced the legacy assertion with checks that fatal, shutdown, SIGCHLD and event handlers all flow through `init_signal_install_status()`, and that the checked wrapper calls `pid1_signal_install_action()`.
- Exact 8a66 PID1 mdev #44999904281, Process inventory #44999904395, Process lifecycle #44999904325 and AI docs #44999904265 PASS. NVRAM on 8a66 #44999904244 was still running; f6a exact NVRAM #44999987551 was queued at first poll. L3/L4 unverified.

---

## 2026-11-22 18:11 CST — Update NVRAM audit for checked PID 1 signal gate

- Parent main `8a66f03a5c0e1d7a73e1ee038c1d8f92be15ad79`. Exact c74 NVRAM audit #44999683896 passed ioctl/close, transaction lock, compaction, snapshot and NVRAM device-node regression steps, then failed only at the final PID 1 source assertion that still expected router startup/readiness without `signal_status`.
- Updated `scripts/check_nvram_transaction_boundary.py` to require both signal-aware router predicates, every checked fatal/shutdown/SIGCHLD/event installation, the first-error helper and suppression of deferred mdev retries on incomplete signal setup. Production sources and C tests unchanged.
- Exact 8a66 PID1 mdev #44999904281 and AI docs #44999904265 PASS. HTTPD #44999904303, Process inventory #44999904395, Process lifecycle #44999904325 and NVRAM #44999904244 were still running at first poll. No L3/L4 claim.

---

## 2026-11-22 18:02 CST — Correct PID 1 signal regression and shared boot checks

- Parent main: `c74ba42e7bab3eba8d89b4931873bce37a83cba5`. Exact PID1 mdev workflow #44999683905 failed because policy functions were used before `pid1_signal_policy.h` appeared in the test translation unit. Exact HTTPD #44999683880 failed its inline router-start/readiness contract because it did not include the new signal-status prerequisite. Exact Process inventory #44999683934 failed the same old conditions in `scripts/check_dev_init_boot.py`.
- Moved the policy header above all test references. Updated the shared Process boot checker and HTTPD's inline contract to require signal setup success before mdev deferred retry or router startup; both now pin all four signal handler groups, first-error propagation and the signal gate. The HTTPD workflow also watches the shared checker path explicitly.
- Exact c74 Process lifecycle #44999683951, USB storage #44999683921, DNSMasq #44999683891, DNS Forwarder #44999684058 and AI docs #44999683893 PASS. XY-C1 #44999684046 and NVRAM #44999683896 were still running at first poll; PID1, HTTPD and Process inventory require exact-head reruns. No L3/L4 claim.

---

## 2026-11-22 17:41 CST — Fail closed when PID 1 cannot install signal handlers

- Parent main SHA: `9a3b2a3f60ae9da088e5de17cda252172b418067`. Networkmap ARP receive source parent 2cd6cbb has three GNU89 sanitizer-backed C regressions PASS; exact checker correction on 9a3 is awaiting rerun #44999591254 with AI docs #44999591228. Exact 2cd Process inventory #44999477626, NVRAM #44999477587 and AI docs #44999477625 PASS; no same-SHA XY-C1 proof for 2cd.
- `init_signals()` previously ignored every `sigaction()` result for fatal, shutdown, SIGCHLD-reaping and event handlers. A failed installation could leave PID 1 running router services without required signal-driven shutdown, notification or child-reaping behavior.
- Added an injectable checked sigaction wrapper that returns the actual errno (or EIO if libc fails without errno), records first-error status while still attempting independent handlers, and logs each failed signal. PID 1 now suppresses deferred mdev alarms and skips router startup when handler installation is incomplete, while keeping the recovery event loop available.
- Added GNU89 policy coverage for successful installation, invalid inputs, EACCES, errno-less EIO and sticky first-error behavior; expanded the existing mdev/boot-device source contract. Exact-head CI and XY-C1 Build/TRX/artifact are pending; hardware L4 NOT VERIFIED.

---

## 2026-11-22 17:24 CST — Update networkmap main-loop contract for propagated status

- Parent main SHA: `2cd6cbb65590408bd14f2843f3187d3ed2d3010f`. Exact networkmap workflow #44999477629 compiled and passed the signal policy, daemon policy and new ARP receive error policy C executables; it failed only at `scripts/check_networkmap_signals.py:134` because an older source assertion still searched for the former standalone `nmap_iterate();` statement.
- Main now invokes `nmap_iterate() < 0` in its checked error gate so fatal receive status reaches cleanup and exit; corrected the source matcher to track that expression. Product source and C tests are unchanged.
- Exact parent Process lifecycle #44999477626 and AI handoff #44999477625 were still running at first poll; new WAN-related NVRAM audit #44999477587 was also running. Rerun the complete networkmap checker on this new SHA.

---

## 2026-11-22 17:12 CST — Stop networkmap ARP receive errors from spinning the daemon loop

- Parent main SHA: `ef3b22d28619427c3e5823b0888c2ea390405b76`. Exact networkmap signal lifecycle #44999104161 PASS and AI handoff #44999104151 PASS. Checked WAN WISP/manual-DHCP/transport flow passed exact WAN route #44998743083 on b809 and AI handoff #44998743195 PASS. XY-C1 #44998650971 was still IN_PROGRESS at last observed poll; it is not same-SHA L3 evidence.
- Found a networkmap runtime fault: `recvfrom()` failures were treated like short packets, so persistent errors such as `ENODEV`/`ENETDOWN` could make the outer loop call `recvfrom()` continuously. Malformed short frames also terminated the receive batch.
- Added `networkmap_arp_receive_error_is_retryable()` to distinguish expected `EINTR` and `SO_RCVTIMEO` `EAGAIN/EWOULDBLOCK` from fatal socket errors. The receive loop now preserves fatal errno and returns it after flushing already-collected client updates; short frames are discarded while draining, with every datagram counted against the 1024-frame storm cap.
- Propagated receive status through `nmap_iterate()`; main now logs and exits through `clear_resources()` on fatal receive errors rather than hot-looping. Added GNU89/Werror/ASan/UBSan unit coverage and source-order contracts in the existing networkmap workflow. Exact-head Actions pending; no target build claimed.

---

## 2026-11-22 16:55 CST — Correct networkmap signal setup source-order assertion

- Parent main SHA: 44a2a851ac6732dd59ee171a99328a0d5cc7299f. Exact Networkmap signal lifecycle #44999008557 FAILED at scripts/check_networkmap_signals.py:121; both the GNU89 signal-policy executable and networkmap daemon-policy regression passed. The new checker compared the installer-call offset (inside the if expression) as if it preceded the if keyword; the order was reversed.
- Production installer, five checked sigaction calls, and cleanup/return path compiled and passed the C tests. Corrected the source-order assertion to check if-condition → installer call → cleanup → return → daemonize order. No product source change.
- Exact 44a2 NVRAM audit #44999008558, process lifecycle #44999008622 and AI handoff #44999008560 were still in progress at last poll. Exact b809 WAN route #44998743083 and AI handoff #44998743195 PASS.

---

## 2026-11-22 16:43 CST — Make networkmap signal disposition setup fail closed

- Parent main SHA: `b809d95f35efebeb7e653feb2f96e1a0c9c8fb69`. Exact WAN route #44998743083 and AI handoff #44998743195 were in progress at last poll; previous exact WAN route #44998472638 and AI handoff #44998472600 PASS. The WISP and checked DHCP selector/transport changes await the combined WAN rerun; no L3 claim.
- Cross-userland stability review found networkmap's five `signal()` registrations (SIGPIPE/HUP/USR2 ignore and SIGUSR1/TERM event handlers) ignored all setup return values. If registration fails, the daemon can run without required shutdown/refresh/pipe protections; a later SIGTERM can terminate it without `clear_resources()`.
- Added `networkmap_signal_install_handlers()` using checked `sigaction()` for all dispositions. Main checks the installer before daemonization and on failure logs, releases socket/client resources and returns the errno status. Signal handlers remain async-signal-safe flag writers.
- Extended the GNU89 policy regression to install/query all five dispositions and reject null handlers, plus source contracts for installer error returns and main's cleanup-before-daemonize failure path. Exact workflow run pending; no local target or firmware build claimed.

---

## 2026-11-22 16:27 CST — Match compacted WAN callback diagnostics in source checker

- Parent main SHA: `fe1533a05beed4d2a9f3a40b7945a2cc25cc6d20`. Exact WAN route #44998650856 FAILED in `scripts/check_wan_dns_read_status.py:1039`. All C policy and earlier WAN source tests passed; the final assertion searched for spaced diagnostic strings after compacting the entire C function with `"".join(...split())`. The source literals were therefore correctly compacted too, so only the Python matcher was wrong.
- The checked `ifname_t` implementation itself is present and returns errors before USB/WISP classification or DHCP renew. Corrected both new diagnostic expectations to their compact literal forms; no product source change.
- Exact parent HTTPD #44998651042 PASS, firewall #44998650885 PASS, WAN DHCP callback #44998650941 PASS, AI handoff #44998650835 PASS. XY-C1 #44998650971 remained IN_PROGRESS at last poll; no same-SHA L3 claim. WAN route rerun pending.

---

## 2026-11-22 16:18 CST — Fail closed on unreadable WAN transport identity before DHCP renew

- Parent main SHA: `103281fb037a380dfa177edb6cf3b7c5acd7582f`. Exact parent WAN route #44998542768, AI handoff #44998542701 and XY-C1 #44998542772 were IN_PROGRESS at last poll; NVRAM audit #44998542802 and HTTPD #44998542654 were also running. Previous exact 59fa source/checker parent: WAN route #44998472638 PASS and AI handoff #44998472600 PASS, confirming WISP identity and matcher correction.
- Continued the link-restored DHCP state transition review and found `get_wan_unit_value(unit, "ifname_t")` was still a lossy safe_get read. A backend/close error could be treated as a non-USB transport and let the callback proceed without knowing the configured transport identity.
- Replaced it with bounded `get_wan_unit_value_copy_status()`; backend/close/size errors are returned before USB-net classification, WISP classification and DHCP signal. A genuinely missing/empty legacy key remains a non-USB interface, as before.
- Extended the WAN source contract to require checked transport read → error gate → USB classification → WISP read → DHCP renew, and forbid the old lossy getter. This is a source/checker slice only; exact-head Actions and firmware build remain pending.

---

## 2026-11-22 16:07 CST — Check the manual DHCP-client selector before link-restored renew

- Parent main SHA: `59fa3e95dc9cb7228837afda278023c3825aa0b4`. The prior WAN route checker variable-binding fix is now being validated by exact run #44998472638; its AI handoff run #44998472600 is also in progress. Exact b436 AI handoff #44998355100 and HTTPD Authentication/Security #44998355033 passed; b436 WAN route #44998355043 had failed only on the subsequently corrected undefined test variable.
- Found another real fail-open control read in `notify_on_wan_ether_link_restored()`: `nvram_match("x_DHCPClient", "0")` hides NVRAM backend/close errors. That allows an Ethernet link-restored event to reach DHCP renewal without knowing whether the manual DHCP-client mode is enabled.
- Replaced the boolean lookup with `get_nvram_match_checked_status()`, preserving missing-key false compatibility. Static-IP protocol still short-circuits first. A read error is logged and returned before USB/WISP checks or DHCP signalling; a confirmed disabled manual client still suppresses renew.
- Added a source-order regression ensuring the checked selector read/error gate precedes DHCP renewal and forbidding the old lossy `nvram_match()` predicate. The shared WAN NVRAM policy executable already covers checked string-match error, missing-key and output semantics. Exact-head route run pending; no target firmware build or L3 claim.

---

## 2026-11-22 15:55 CST — Fix WAN checker regression-test variable binding

- Parent main SHA: `b4366cb27be501905e3c5c7a3a73e8e2e5f9302d`. Exact b436 WAN route #44998355043 failed at `scripts/check_wan_dns_read_status.py:1030` with `NameError: nvram_policy_test is not defined`. The test source is already loaded as `test`; WISP policy compile and execution, existing WAN DNS checks and earlier route source contracts all passed. This is a checker-only variable-name defect, not a WISP product-source failure.
- Exact b436 HTTPD Authentication/Security #44998355033 PASS; AI handoff #44998355100 PASS. The earlier HTTPD stale firewall assertion is resolved.
- Rebind the new WISP unit-test presence/registration assertions to the existing `test` variable. The WISP C implementation is unchanged. Exact rerun must pass before the WISP slice is called host-verified; no same-SHA L3 claim.

---

## 2026-11-22 15:47 CST — Align WISP and HTTPD firewall source contracts

- Parent main SHA: `a41fdc4d1bf6aef69e5242aa26712c56fe2b56f7`. Exact WAN route #44998118160 FAILED in `scripts/check_wan_dns_read_status.py:988`: the new WISP source matcher collapsed runs of whitespace but retained a single separator after a wrapped function-call opening parenthesis. The WISP policy C executable compiled and ran successfully before the Python checker step.
- Exact HTTPD Authentication/Security #44998118167 FAILED in its route-access checker at embedded Python line 863 (`.github/workflows/httpd-security.yml`): it still required the removed `man_if_value`/`get_man_ifname()` firewall code after the checked MAN-interface fix landed on parent `5a86cca`. All earlier HTTPD steps in that job passed.
- Corrected both contracts without changing product behavior: WISP matching now strips all source whitespace before order assertions; HTTPD firewall assertions now require checked MAN read, error gate, safe-token validation and validation-before-raw-rules, and reject the lossy getter.
- The exact-head WISP source change itself is not yet host-verified because the route checker stopped on this assertion. Exact parent XY-C1 #44998117945 and AI docs #44998118765 were still in progress at last poll; no L3 claim.

---

## 2026-11-22 15:31 CST — Preserve WAN WISP identity read failures

- Parent main SHA: `5a86cca27cb9634a1a0c13375a58518c848b309e`. Its firewall checked-MAN-interface fix is validated by exact Firewall Startup run #44997818203 PASS; AI handoff #44997818219 and Process inventory #44997818261 PASS. WAN route #44997818209, NVRAM audit #44997818217, FULLCONENAT #44997818224, HTTPD security #44997818264 and XY-C1 #44997818248 were still in progress at the last check; no L3 claim.
- Found two consumers of `get_wan_wisp_active()` that could misclassify an unreadable `wan0_ifname` as non-WISP because the getter calls `get_man_ifname()`/safe_get: WAN link-restored DHCP renewal could run when the WISP interface is active, and WAN LED logic could fall back to cached Ethernet link state.
- Added `get_wan_wisp_active_status()`: it preserves checked NVRAM read/close status, validates a nonempty interface token before classification, distinguishes a genuinely empty legacy interface, and only queries AP-client link state after successful classification. The old boolean ABI remains as a conservative wrapper (unknown identity returns active).
- Updated the WAN LED path to turn the link indication off on unknown MAN identity without consulting cached Ethernet link state; updated link-restored handling to propagate the read error and suppress DHCP renewal. Added errno/validation precedence unit coverage and source-order contracts in the existing WAN route regression.
- Tests are wired into the WAN route workflow (GNU89/Werror/ASan/UBSan C policy test and the WAN DNS/source checker); no local target build is claimed. Exact-head Actions for THIS COMMIT are pending at creation time. Firmware/TRX and hardware verification remain NOT VERIFIED.

---

## 2026-11-22 — Fail closed when firewall cannot read its management interface

- Parent main SHA: `b9964bf12272c2f18d5fbc92f1fd9a155c6f4fe5`. Exact b996 AI handoff #44997591604 PASS after lossless `dev_log.md` rotation. Exact a3 WAN route #44997377580, DHCP #44997377452, teardown #44997377521, wireless #44997377474, Process lifecycle #44997377507, Process inventory #44997377633 and NVRAM transaction #44997377584 PASS; XY-C1 #44997377413 remains in Build XY-C1.
- Confirmed a P1 firewall fail-open input-read path in `start_firewall_ex_locked()`: `get_man_ifname()` uses `nvram_safe_get()`, which collapses NVRAM read/backend/close errors into an empty interface. The empty value was accepted, then used to build interface-dependent firewall/rp_filter policy.
- Replaced that getter with checked bounded `get_wan_unit_value_copy_status(unit, "ifname", man_if, sizeof(man_if))`. Real errors now set `errno` and return before firewall/rp_filter rules are applied; nonempty values keep the existing safe-token check; an actually empty legacy value remains compatible.
- Extended `check_wan_firewall_status.py` to pin checked read/error/validation before the first per-interface rp_filter write and forbid the lossy getter in `start_firewall_ex_locked()`. Exact-head firewall Actions are pending.

---
## 2026-11-22 — Rotate the rolling development log and preserve full-restart verification

- Root `dev_log.md` had 121 complete records. Archived the 101 oldest records byte-for-byte in chronological order in `docs/dev-log/2026/11/part-025.md`; retained the 20 newest original records at the root, then prepended this rotation checkpoint. Archive SHA: `daf3875420e99758082b0d43ab2e4b4b2cd57ab2`; pre-rotation root blob SHA: `418727893b781c626ffa7995bc378ac8b5cff66f`.
- Exact `a3b7e95b71776808fbb6e6bddf273db8cf0c98f4` WAN route #44997377580, WAN DHCP #44997377452, WAN interface teardown #44997377521, wireless radio #44997377474, Process lifecycle #44997377507 and Process inventory #44997377633 PASS. AI handoff #44997377383 failed only because the root log reached 121 entries; this lossless archive rotation restores the required rolling window.
- Exact a3 XY-C1 #44997377413 was still executing Build XY-C1 at last poll; TRX verification and artifact upload had not completed. NVRAM transaction audit #44997377584 and several independent suites were still running. No L3 claim.
- The same a3 source fix validates and preserves `wan0_ifname` before `full_restart_wan()` changes router mode or stops WAN, then reuses the checked local name for route flush. Hardware L4 remains unverified.

---
## 2026-11-22 — Validate full-restart route interface before WAN teardown

- Parent main SHA: `c69d200c788aafeb76c6905bb18a70d22f9d49d4`. In the prior 86d run, wireless radio #44997181767 and Process lifecycle #44997181794 failed only because the source checker called undefined `require()` at line 323; the wireless policy executables passed. Follow-up c69 changed that call to Python `assert`.
- Found another concrete state-transition read-loss path: `full_restart_wan()` fetched `route_ifnames[0]` via `get_man_ifname(0)` only after `stop_wan()` completed. A transient NVRAM read error could appear as an empty string after services had already been stopped, then route flush would operate on the wrong interface set.
- Moved checked `get_wan_unit_value_copy_status(0, "ifname", ...)` and `wan_ifname_copy()` validation before `update_router_mode()` and `stop_wan()`. The validated bounded local name is reused for route flush; real errors are logged, copied to `errno`, and returned before WAN side effects. Empty legacy configuration remains an empty string.
- Extended the WAN DNS/read checker to pin checked read/error/validation before mode update and service stop, then route use of the same validated buffer, with no `get_man_ifname(0)` left in full restart.
- Exact c69 wireless radio #44997296073, Process inventory #44997296036 and AI docs #44997296148 were in progress at last poll; no same-SHA XY-C1 build claim. Exact-head validation for this product change remains pending.

---
## 2026-11-22 — Fix AP-client radio source-checker assertion helper

- Parent main SHA: `86d18d524ca522dc4d85e5abf7c28376ac74a394`. Exact wireless-radio #44997181767 failed at line 323 with `NameError: name 'require' is not defined`; the newly added checker used a helper absent from this script, which consistently uses Python `assert`.
- Exact Process lifecycle #44997181794 also failed at the same imported guest-LAN checker assertion after its independent account bootstrap and bridge policy tests passed. The wireless-radio, bridge-membership and guest-isolation C policy executables passed before the Python checker.
- Replaced only the undefined `require(condition, message)` call with Python `assert (condition), (message)`, retaining all ordering, empty-configuration and errno assertions. No product source changed.
- Exact 86d AI handoff #44997181801, process inventory #44997181843, HTTPD Auth/Security #44997181802 and iNIC lifecycle #44997181809 PASS. XY-C1 #44997181783 and NVRAM transaction #44997181875 were still in progress at last poll.

---
## 2026-11-22 — Check AP-client radio recovery inputs before WAN restart decisions

- Parent main SHA: `0292b0a44ece35cac3e0331ab04aa15028a671a3`. Exact WAN route #44996976228 PASS, DHCP #44996976338 PASS, WAN teardown #44996976388 PASS and AI handoff #44996976327 PASS. Exact XY-C1 #44996976282 was still building.
- `check_apcli_wan()` previously used `get_ap_mode()` and `get_man_ifname()` (safe_get-backed) to decide whether an AP-client radio restart should trigger full WAN restart or reconnect. A transient NVRAM error could masquerade as AP mode false/empty manual interface and produce an incorrect recovery decision.
- Switched to `get_ap_mode_status()` and checked `get_wan_unit_value_copy_status()`; nonempty manual interface is validated by `wan_ifname_copy()`. Any read/token failure returns before restart/reconnect decision, with errno preserved. Empty legacy manual config continues to map to no APCLI MAN interface.
- Extended `check_guest_lan_isolation_status.py` to pin the checked mode/manual input order and `wan_ifname_policy.h` dependency. Exact-head wireless/WAN/XY Actions remain pending for this source change.

---
## 2026-11-22 — Guard AP-client WISP migration with checked prior WAN identity

- Parent main SHA: `2149789b0636dd794fcb7eeb5343e314045cdc9a`. Exact 214 WAN route #44996754354 PASS, WAN DHCP #44996754350 PASS, WAN interface teardown #44996754355 PASS and AI handoff #44996754406 PASS; XY-C1 #44996754358 remained in Build XY-C1.
- `config_apcli_wisp()` previously fetched old manual WAN interface via `get_man_ifname()` (safe_get-backed); a transient NVRAM error could look empty, skip clearing the prior WAN address/interface, then continue enabling/publishing the AP-client WISP interface.
- Added checked bounded `ifname` read and `wan_ifname_copy()` validation before old-interface cleanup, IPv6 enablement and NVRAM publication. Actual errors are returned with `errno`; empty legacy configuration remains an empty string.
- Extended the WAN source contract to assert read/error/validation ordering before ifconfig/IPv6/publication and forbid the lossy getter. Exact-head Actions for this additional source change remain pending.

---
## 2026-11-22 — Fail closed before WAN teardown when manual interface is unreadable

- Parent main SHA: `59cdab3123a8d6cce5dae6c5f5fe347d9d9bee67`. Exact WAN route #44996570973 PASS and AI handoff #44996570985 PASS; exact d018 WAN DHCP #44996488482 PASS.
- Reviewed `stop_wan_locked()`: it previously read `man_ifname = get_man_ifname(unit)` only after pausing Internet monitoring, invalidating PPP callback sessions and cancelling deferred reconnect. Since the getter is `nvram_safe_get()`-backed, a NVRAM read/backend error could be silently converted to an empty name while teardown proceeded.
- Moved a checked `get_wan_unit_value_copy_status(..., "ifname", ...)` and `wan_ifname_copy()` validation ahead of monitor pause and all teardown side effects. The real read/validation error is logged, copied to `errno`, and returned. An actually empty legacy/unconfigured value is preserved as an empty string rather than rejected; removed the later lossy getter.
- Added a source contract to pin checked read → read-error gate → nonempty token validation/error gate → empty-key preservation → monitor pause, and forbid the lossy getter in `stop_wan_locked()`.
- Exact-head route/DHCP and XY-C1 Actions are pending for this product source change. No MIPS build/TRX/artifact or hardware test is claimed.

---
## 2026-11-22 — Keep modem identity error contract errno-aware

- Parent main SHA: `d018080e3ef848417c1969ba17adfd0b8a775624`. Exact WAN DHCP #44996488482 PASS. Exact WAN route #44996488531 FAILED at `scripts/check_wan_dns_read_status.py:515`: after the `get_wan_ip4()` checked manual fallback change, the old modem identity assertion still required `if(status!=0)returnINADDR_ANY`.
- The actual code correctly sets `errno=status` before returning `INADDR_ANY` for modem identity failures. The stale assertion had not been updated for this improved error preservation; the checked manual-fallback source contract immediately above it passed.
- Replaced the errno-less pattern with an ordered contract requiring modem status read → `errno=status` failure gate → transport selection. This is test-only; no product behavior change in this follow-up.
- Exact d018 XY-C1 #44996488349 was still building/validating at last poll. L3 remains unverified until Build XY-C1, TRX and artifact all pass on one identical product-source SHA.

---
## 2026-11-22 — Check manual interface only when get_wan_ip4 selects it

- Parent main SHA: `616d5a710acdf727a8f110052289c1d63d966a9d`. Exact WAN DHCP #44996346231 PASS and AI handoff #44996346193 PASS; prior WAN route #44996213814 PASS.
- Found the remaining `nvram_safe_get()` loss path in `get_wan_ip4()`: it read `get_man_ifname()` before modem/PPP overrides. Backend/read/close errors could be mistaken for an empty interface; moreover manual `ifname` was fetched even when a modem/PPP interface won.
- Changed interface selection to check `ifname_t`, modem identity and protocol first, retaining modem/PPP overrides. Only when no override is selected does it now perform checked `get_wan_unit_value_copy_status(..., "ifname", ...)`, validate via `wan_ifname_copy()`, and preserve read/validation failure in `errno` before returning the existing `INADDR_ANY` sentinel.
- Updated `scripts/check_wan_dns_read_status.py` to require manual fallback read/validation after modem/protocol selection and before address lookup, while preserving the cache-overwrite regression.
- Exact-head WAN route/DHCP for this product-source SHA are pending. No MIPS build, TRX, artifact or hardware validation is claimed.

---
## 2026-11-22 — Normalize WAN manual-fallback checker ordering

- Parent main SHA: `ecfdafb0e348bcec466d4372400f5a2c87f02006`. Exact ecfd WAN route #44996213814 PASS; AI handoff docs #44996213622 PASS. WAN DHCP callback #44996213685 failed after its GNU89/ASan/UBSan `wan_ifname_policy.c` executable and basic bounds regression both passed.
- The failing assertion at `scripts/check_wan_ifname_bounds.py:51` compared `manual_ifname_gate_at` from the raw source string with later positions from the whitespace-compacted source. Those offsets are not comparable, so a valid checked manual fallback failed the checker.
- All ordering positions are now measured in the same compacted function. The contract also verifies modem and protocol selection happen before the fallback, and that read error and token-validation error branches both precede the final output copy.
- The exact dc6 product-source XY-C1 run #44996098229 remained in progress at last poll with Build XY-C1 running; TRX and artifact upload were pending. The checker-only ecfd commit did not alter product firmware inputs. L3 remains unverified.

---

## 2026-11-22 — Align WAN DNS checker with checked manual-interface getter

- Parent main SHA: `dc6f6b4efb8b0ac088e82e18f9bd42168efdd9d0`. Exact WAN DHCP callback #44996098158 PASS, including the expanded `check_wan_ifname_bounds.py`; HTTPD Auth/Security #44996098097 and AI handoff #44996098288 PASS.
- Exact WAN route #44996098107 FAILED at `scripts/check_wan_dns_read_status.py:493`. Its contract still required `get_man_ifname()` followed by `rc_wan_nvram_copy_text()` in `get_wan_ifname()`, while the current implementation now selects modem/PPP overrides first and performs checked `get_wan_unit_value_copy_status()` plus `wan_ifname_copy()` only when using the manual interface. The C path itself was not the failing step.
- Replaced that stale assertion with the checked fallback read/validation contract and strengthened `check_wan_ifname_bounds.py` to verify the exact manual `ifname` read occurs inside the fallback branch before its copy and the final public output copy; this avoids accidentally matching the separate earlier `ifname_t` checked read.
- Exact dc6 XY-C1 #44996098229 was still IN_PROGRESS before Build/TRX/artifact at last poll. This follow-up's exact-head checks are still pending; no L3 claim.

---
## 2026-11-22 — Preserve checked interface identity in get_wan_ifname

- Parent main SHA: `2b2c2e6c1ef1ca9fc41dd39ebb88858e33cf1bcb`. Exact 2b WAN DHCP callback #44995928887 PASS, WAN route status #44995928825 PASS and AI handoff #44995928838 PASS. Exact XY-C1 #44995928841 remained IN_PROGRESS at last poll; no L3 claim.
- Continued the same concrete NVRAM failure path beyond `start_wan()`: `get_wan_ifname()` also used `get_man_ifname()` backed by `nvram_safe_get()`, then copied the lossy value through `rc_wan_nvram_copy_text()`. A backend/read/close error could be rewritten as an empty-interface `EINVAL` instead of preserving the actual NVRAM error.
- Removed the lossy getter from `get_wan_ifname()`. The function now performs checked bounded read and token validation only if its selected interface still uses the configured manual name; modem/PPP overrides do not gain an unrelated read dependency. Checked status is returned with `errno` preserved.
- Extended the existing WAN interface source contract to pin that conditional checked read, validation, error propagation and lack of the lossy getter. Exact-head WAN DHCP/route/XY verification is pending for this commit.

---
## 2026-11-22 — Align WAN startup source contracts with checked interface identity

- Parent main SHA: `e34444ae55651085a157996f4c6b5699b20066c9`. Exact 6ec WAN DHCP #44995722211 first failed because the first version of the new checker searched for literal `\\n`; the e344 follow-up corrected this, but WAN DHCP #44995792863 then exposed two additional whitespace-sensitive expectations that omitted the space after `(`. The underlying WAN interface-name GNU89/ASan/UBSan executable passed on both source SHAs before the Python checker stopped the job.
- Exact 6ec WAN route #44995722139 and exact 6ec XY-C1 #44995722192, repeated on e344 as #44995792838 and #44995792759, failed at `scripts/check_wan_firewall_status.py:1170`: it still required `wan_ifname = get_man_ifname(unit)` and was stale after the checked source fix.
- Updated the interface-name checker to match normalized actual C call formatting. Updated the WAN firewall source contract to require checked `ifname` read → explicit read-error status handling → empty legacy no-op → validated interface token → explicit validation-error status handling → first interface side effect, and to forbid the removed lossy getter.
- Exact e344 AI handoff #44995792742 and Process inventory #44995792750 PASS; no XY-C1 L3 build is claimed. Recheck all exact-head Actions on the current follow-up SHA.

---
## 2026-11-22 — Correct WAN interface diagnostic and checker newline anchors

- Parent main SHA: `6ec3e3d6bf3576864dee986aaf1e7075a4fbc1fa`. Immediate source review caught two defects before treating CI as validated: the invalid-interface log format accepted only one of its two integer arguments, and the newly added Python checker delimited C functions using a double-escaped newline (literal backslash-n).
- Supplied both unit and ifname_status to the diagnostic and corrected the source-checker delimiters to resolve actual newline boundaries. Added an exact source assertion for the diagnostic argument order.
- The exact 6ec push triggered WAN DHCP callback/route, Process, NVRAM and XY-C1 plus the broader source-filtered suites; their first poll showed pending/in-progress only. Verify the follow-up SHA rather than inferring results from the intermediate head.

---
## 2026-11-22 — Fail closed when WAN startup cannot read its configured interface

- Parent main SHA: `7bd0c10e867a429dfbc7dc2573e211c1963aa580`. Exact 7bd storage hotplug #44995310447 and AI handoff #44995310444 passed. Exact XY-C1 #44995310496 completed host-only validation successfully, but Build XY-C1, TRX verification and artifact upload were skipped because 7bd changed only tests/checkers/handoff; no L3 claim.
- Confirmed a P1 startup-status loss in `start_wan()`: it called `get_man_ifname()`, which uses `nvram_safe_get()`. A transient NVRAM backend/read/close error was therefore indistinguishable from an empty configured interface and silently skipped WAN startup.
- Replaced the pointer lookup with `get_wan_unit_value_copy_status()` into a bounded local buffer, retained the established empty/unconfigured no-op, validated non-empty names through `wan_ifname_copy()`, and merged both read/validation failures into the first WAN startup status before any interface setup.
- Extended `scripts/check_wan_ifname_bounds.py` to pin checked read → empty legacy no-op → validated interface token → network side effects, plus status propagation and removal of the lossy getter from `start_wan()`. Existing WAN interface-name GNU89/ASan/UBSan policy cases and checked NVRAM-copy regression are reused by the existing WAN DHCP workflow.
- Exact-head WAN DHCP/route and XY-C1 Actions are pending for the new product-source SHA. No local MIPS build or hardware test is claimed. L3/L4 remain unverified.

---

## 2026-11-22 — Align printer hotplug checker with start-before-ack semantics

- Parent main SHA: `886f9cf54a6abafad251fb444de199e0930446f8`. Exact 1160 storage run #44995018412 passed the storage policy regressions and whole caller graph, then failed in `scripts/check_storage_safe_remove.py:1512`. Its assertion required the printer failure branch to precede `storage_ack_event_marker_status("usb_hotplug_lp")`.
- The product callback correctly calls `try_start_usb_printer_spoolers()`, acknowledges `usb_hotplug_lp` only when start status is zero, and only then enters the failure branch for start/ack failures. The branch logs that the event remains queued and calls `alarm(5)`. Updated the checker to assert this exact order and retry contract.
- Exact 886 notification queue Actions #44995118883 and AI handoff docs #44995119002 passed. Exact 886 XY-C1 validation #44995118906 completed SUCCESS, but Build XY-C1, TRX verification and artifact upload were SKIPPED because 886 changed only a test expectation and docs. This is not an L3 build result.
- Next: validate the checker on a fresh storage workflow, then push a real firmware-relevant product change so XY-C1 Build/TRX/artifact execute on one SHA.

---

## 2026-11-22 — Include storage-unplug bit in retry-retention regression

- Parent main SHA: `1160bf68ae35438c2ae5acbef8252b7faaea2c77`. Exact PID-1 notification workflow #44995018361 failed in its first GNU89 policy test. After adding the storage-unplug retry bit, the test correctly retained that bit when a different event was recorded with marker status 0, but one intermediate expected mask still omitted the new bit.
- Updated that expectation to include `RC_NOTIFY_HOTPLUG_RETRY_STORAGE_UNPLUG`. The PID-1 retry implementation itself was not the failure; the assertion was stale after expanding the mask.
- Local policy test will be rerun with GNU89, Werror, ASan and UBSan; exact-head notification/storage/XY-C1 Actions must verify the corrected test and product code.

---

## 2026-11-22 — Retain storage-unplug retry when the NVRAM marker write fails

- Parent main SHA: `7db3ff63d8c65c99163b59027fc00feb80e0171a`. The preceding storage run #44994403806 passed the storage inventory/marker/lifecycle regressions and complete caller graph, then failed in the Python checker because it searched for literal backslash-n instead of a real newline at the end of `storage_ack_event_marker_status()`. The parser correction is in parent `7db3ff63d8c65c99163b59027fc00feb80e0171a`.
- `storage_schedule_ejected_cleanup_retry()` previously ignored `nvram_set_int_temp("usb_unplug_ms", 1)`. If ejected cleanup and the NVRAM marker write both failed, the five-second alarm had no durable event to process, so cleanup could be abandoned.
- The scheduler now checks the marker result and calls `rc_notify_schedule_deferred_hotplug_retry("on_unplug_mass_storage")`. PID 1 records a timed retry bit and requeues the event on its next alarm; short-lived applets enqueue it through the existing secure notification queue. Original `errno` is restored.
- Added a storage-unplug retry bit and event mapping, extended GNU89 queue-policy regression for mapping/record/requeue/failure retention, and strengthened storage and PID-1 notification source contracts.
- Exact 7db3 storage #44994641468, XY-C1 #44994641391 and AI handoff docs #44994641406 were in progress at last poll. No local MIPS firmware build or hardware test is claimed.

---

## 2026-11-22 — Fix storage checker function-boundary escape

- Parent main SHA: `8f54faaaf0e0cbd2461ebd35f595612032521987`. Exact-head USB storage run #44994403806 passed the whole storage caller graph and all preceding C/shell regressions, then failed in `scripts/check_storage_safe_remove.py:1446` with `ValueError: substring not found`. The checker searched for literal backslash-`n` sequences (`"\\nstatic int\\nstorage_read_nvram_string"`) while the C source contains real newline characters.
- Corrected the Python string to use actual newline escapes so the checked marker-writer function ends at `storage_read_nvram_string`; product code and the asserted error-return contract are unchanged.
- Exact-head AI handoff documentation contract #44994403647 passed. XY-C1 validation #44994403623 completed SUCCESS on `8f54faa`, but its Build XY-C1, TRX and artifact-upload steps were SKIPPED due to the workflow's path/condition logic; this is not an L3 firmware-build result.
- Next: check exact-head storage workflow after this checker fix and establish why XY-C1's build steps were skipped; do not claim L3 until all three firmware gates actually execute and pass.

---

## 2026-11-22 — Update safe-remove retry checker for PID 1 queue handoff

- Parent main SHA: `9110090e4886d5a183dfe21dfbd26fde08698aea`. The corrected whole-storage caller graph passed on exact SHA `9110090e4886d5a183dfe21dfbd26fde08698aea`; the workflow then reached `scripts/check_storage_safe_remove.py` and failed at its stale line 478, which still demanded `nvram_set_int_temp("usb_hotplug_ms", 1)` and `alarm(5)` inside `safe_remove_stor_device()`.
- The product fix at `89a6e740e24a63142a56e3406136e86a514ce1a2` intentionally replaced that short-lived applet's local alarm with `notify_rc_status("on_hotplug_mass_storage")` after releasing `storage_lock`. PID 1 then owns the deferred event/alarm lifecycle; queue failure is logged and propagated rather than falsely reported as scheduled.
- Updated the source contract to require the PID 1 event name, verify no direct NVRAM marker/alarm remains in the short-lived retry block, require both success/failure logging and preservation of the notification errno. No product source changed in this checker-only slice.
- Exact 9110090e storage run #44994229595: the embedded whole-storage caller graph passed; the workflow failed in this stale safe-remove checker. The AI handoff contract #44994229636 passed. XY-C1 #44994003623 and Process lifecycle #44994003659 on product source ancestor `89a6e740` remained in progress; Process fail-closed #44994003779 and NVRAM boundary #44994003791 passed.
- Next: rerun storage checker and downstream firmware persistence gates on the corrected SHA; inspect XY-C1 Build/TRX/artifact on the same product-source content. Hardware L4 remains NOT VERIFIED.

---

## 2026-11-22 — Match USB printer acknowledgement checker to the retry-safe source order

- Parent main SHA: `89a6e740e24a63142a56e3406136e86a514ce1a2`. Exact-head USB storage run #44994003668 passed the BusyBox ash Optware lifecycle checks, MTD save/restore/erase/backup transaction checks, persistence gate, eject inventory regression, deferred storage cleanup/ack regression, checked event-marker and Optware child-result tests, FTP daemon and storage-stop regressions, RC service-launch test, filesystem module-load lock checks and automount ownership checks.
- The final embedded whole-storage caller-graph checker failed at line 239 because its old assertion expected `printer_start_pos < printer_failure_pos < printer_ack_pos`. The actual retry-safe implementation acknowledges the marker only inside the successful-start branch, then checks the combined start/ack status and schedules `alarm(5)` on either failure; the correct order is start → conditional acknowledgement → failure/retry branch.
- Updated that source contract to require the actual order and to scope the retry assertions through the next modem section. No product source changed in this checker-only slice.
- Exact-head XY-C1 #44994003623, Process inventory #44994003659, NVRAM #44994003791 and Process fail-closed #44994003779 were IN_PROGRESS at the last poll. Firmware Build/TRX/artifact and those source validations must be checked again after this correction; no L3 claim.
- Next: rerun the storage source contract on the new SHA; fix only the first remaining stale assertion or real regression. Frozen backup remains read-only; hardware L4 NOT VERIFIED.

---

## 2026-11-22 — Align the storage caller-graph regression with durable marker acknowledgement

- Parent main SHA: `d414a788abc66e0bf5d8711b4d0584de505964c9`. The exact-head USB storage run #44993693581 failed in the embedded “whole storage caller graph” source contract at the old assertion that required `usb_unplug_ms` to be cleared before `umount_ejected()`. This is now deliberately forbidden by the product fix in the parent.
- Updated the embedded workflow contract to require checked marker read → action-completion gate → eject cleanup → marker acknowledgement → ack-latch handling, and to reject any premature marker clear. This retains the existing caller-graph/lock-order assertions; no product source changed in this checker-only slice.
- The standalone deferred cleanup/ack policy regression had already passed locally with GNU89, Werror, ASan and UBSan. The failed workflow had reached the embedded source contract; the full storage checker and remaining gates were skipped afterward.
- Next: push this workflow-only correction atomically and inspect its exact-SHA USB storage and XY-C1 runs. No firmware/L3 or hardware/L4 claim.

---

