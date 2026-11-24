## 2026-11-25 — Align DHCP retry scheduler checker with tested plan

- Exact main SHA `fd59df3710ddcadfd9ca96f9e3b486f5cce7866e`, udhcpc run #45480041494: GNU89 -Werror ASan/UBSan policy regression PASS; Python source checker failed at line 238 because it still expected `udhcpc_retry_delay(retry_timeout)` to appear directly in the scheduler after the call moved into `udhcpc_deconfig_retry_plan_init()`.
- THIS COMMIT updates the stale source assertion to match the new scheduler/plan integration. The independent source-order checks still pin plan initialization, LISTEN_NONE, state publication and timer/counter publication; no DHCP product C code or test semantics changed.
- Parent AI handoff #45480041430 PASS. Exact-head udhcpc rerun pending; prior-SHA target-build and integration runs remain separate. L3 baseline `80f1bfccd255536a0a2682c7d046f0216038a922`, hardware L4 unverified.

---

## 2026-11-25 — Make DHCP deconfig retry state and timer planning testable

- Extends the active WAN DHCP transition slice by extracting the pending-cleanup resume tuple into a pure GNU89 policy plan: resume state, release intent, retry timeout (including the one-second floor), packet count and elapsed-wait counter. The event loop prepares the tuple before closing its listener, then publishes the saved transition and timer only after switching to LISTEN_NONE.
- Added regression cases for normal retry, zero/negative retry intervals and release-intent normalization. Expanded source contracts for the pending transition publication order, INIT_SELECTING versus non-discovery resume socket/timer setup, lease-expiry gating, SIGUSR2 release-on-pending and -R exit cleanup. Wrong-server DHCPNAK filtering remains required.
- Parent `98168ad16689cf47e1efeb2ee3999374790cbc41`; THIS COMMIT changes only DHCP production/header/test/checker and the active handoff/log. No local checkout is available, so no local compiler run is claimed. Exact-head GNU89 ASan/UBSan, Python contracts and XY-C1 Build/TRX/artifact remain pending; L3 baseline remains `80f1bfccd255536a0a2682c7d046f0216038a922`, L4 unverified.

---

## 2026-11-25 — Normalize the udhcpc retry-helper return contract

- Exact main SHA `0db869b438d344406375dde71ee8b1764b047272`, udhcpc run #45476513525: the GNU89 -Werror ASan/UBSan policy binary compiled and passed; the Python source contract failed at its retry-helper return assertion. The checker compares `policy_compact`, which removes all whitespace, but the expected token still contained a space after `return`.
- THIS COMMIT changes that one checker expectation to the compact spelling `returnudhcpc_retry_delay(retry_timeout);`. It does not change DHCP production behavior or relax the state/order assertions.
- Parent AI handoff run #45476513417 PASS. Exact-head udhcpc and any triggered source/firmware checks for THIS COMMIT are pending; no local test was run because this session has no repository checkout. L3 remains limited to the earlier verified production SHA `80f1bfccd255536a0a2682c7d046f0216038a922`; hardware L4 is not verified.

---

## 2026-11-25 — Correct shared retry-delay checker whitespace normalization

- Exact 36a10c0e5844daf31b3cc483c20d9e25a798ef6b udhcpc workflow #45476128312 compiled and ran the GNU89 ASan/UBSan C regression successfully, then failed its Python source checker on a typo in the new compact-source assertion: whitespace normalization removes the space after the C return keyword.
- Corrected the assertion to match normalized C source. This was a checker-only failure; no C test or product code failure was reported.
- Parent da2 exact workflow state at correction time: udhcpc #45475643463 COMPLETED (SUCCESS); AI docs #45475643556 COMPLETED (SUCCESS). Exact retry on this checker fix is required.

---

## 2026-11-25 — Unify udhcpc cleanup retry delay and lock full state transitions

- Exact run #45475288109 on a218e0ce5cfeba84cc0dd82c82afc319041d834b compiled/ran C tests PASS but its source checker failed on whitespace formatting of the multi-line DHCPNAK predicate call. Checker-only correction is da2bdbcf1ec3720d547c2665c638900bb628d289; exact Actions #45475643463 and AI handoff #45475643556 were queued at the last check.
- Consolidated duplicate retry-delay floors into udhcpc_retry_delay(), now shared by DECONFIG_PENDING scheduling, pending-state retries, exit cleanup and DECLINE_PENDING retries. The GNU89 test covers positive, one, zero and negative intervals and the decline wrapper.
- Expanded Python source contracts to enforce scheduler state capture, listener closure, pending state, bounded timeout and counter resets; failed-vs-successful deconfig transitions; lease-expiry cleanup before INIT_SELECTING; SIGUSR1 preservation of an active-lease release obligation; and SIGUSR2 release behavior while cleanup is pending.
- This preserves existing fail-safe semantics and avoids divergent timeout handling across cleanup paths. Exact-SHA Actions on THIS commit are required.

---

## 2026-11-25 — Fix formatting-sensitive NAK source-contract assertions

- Exact Actions run #45475288109 on a218e0ce5cfeba84cc0dd82c82afc319041d834b compiled and ran the GNU89 ASan/UBSan regression binary successfully, but its Python checker still failed: it compared a call expression as one whitespace-exact string, while the production call spans multiple indented lines. The earlier policy-body check also searched for escaped newline text rather than relying on normalized code.
- Replaced those brittle checks with whitespace-normalized source/policy/test assertions, preserving the same semantic ordering and all server-ID presence/mismatch checks. No production code changed.
- Exact-SHA udhcpc checker rerun on THIS commit is required; the a218 docs validator passed (#45475287900).

---

## 2026-11-25 — Align DHCP NAK source contract with extracted predicate

- Exact run #45475021198 on fd8697d50c16035051a63cb6cbe932e99ac1a533 passed the GNU89 -Werror ASan/UBSan test binary and failed the Python source checker because it still looked for the pre-refactor inline server_addr conditional and svid comparison.
- Rewrote the checker to assert the equivalent extracted helper contract: option extraction and helper rejection occur before the advisory nak script, missing/mismatched server ID is rejected when an expected server is known, and the no-known-server behavior remains unchanged. This is a checker expectation mismatch, not a compile/runtime test failure.
- Exact-SHA rerun on THIS checker correction is mandatory. XY-C1 and other independent workflows for fd8697 are still being inspected separately.

---

## 2026-11-25 — Cover DHCPNAK server identity filtering with executable policy tests

- Preserved BusyBox's explicit multi-server safeguard and extracted it into udhcpc_nak_server_id_rejected(): when an expected server is known, missing/mismatched server IDs are rejected; when none is known, the old permissive startup behavior remains unchanged.
- Added GNU89 test cases for no-known-server, exact server match, missing server identifier and mismatched server identifier. The checker now verifies that the production NAK path calls this predicate before the nak script and lease cleanup.
- The previous exact-SHA udhcpc run #45474436146 failed compiling parent 8151d689580ff7e991f539d466c4b8a1ce9ebfb7 due the malformed test-main insertion; b61d41a56a409053559909a26c8849f3b9d4d7af fixes it, but did not receive its own workflow run in the initial query. This new source/test slice creates a fresh exact-SHA gate. No PASS is claimed until Actions complete.

---

## 2026-11-25 — Correct DHCP policy test registration before execution

- Immediately after 8151d689580ff7e991f539d466c4b8a1ce9ebfb7, reviewed the changed C fixture and caught a malformed test-main insertion: the second if ( was duplicated, making the GNU89 test binary fail to compile.
- Corrected the registration to one condition chain. This was caught from source inspection before the exact-SHA udhcpc workflow reached its compile step; no CI result is claimed for the faulty parent.
- Re-run the exact-SHA udhcpc workflow on THIS correction and inspect XY-C1 Build/TRX/artifact separately.

---

## 2026-11-25 — Make DHCP NAK lease gating an explicit tested policy

- Refined the WAN DHCP task after checking BusyBox's established wrong-server NAK filter and RFC renewal/rebinding guidance. Kept that compatibility/security behavior unchanged: packets with a nonmatching server identifier remain ignored; this is not treated as a confirmed defect.
- Extracted the existing NAK cleanup condition into udhcpc_nak_requires_deconfig(): REQUESTING has no active lease to tear down, while any post-acquisition state must successfully deconfigure before restarting address selection. Added a GNU89 regression for both sides.
- Extended the source contract to pin server-ID filtering before the nak callback, require active-lease deconfig and retry scheduling before transition, and assert that leasefail callback status remains advisory while normal retry/background/abort behavior proceeds.
- Existing udhcpc-script-status workflow already watches the changed DHCP source/header/test/checker paths; exact-SHA Actions are the next verification gate. No router/hardware test is claimed.

---

## 2026-11-25 — Confirm descriptor-pinned libdisk L3 and refresh cross-userland handoff

- Re-polled exact Actions after the docs-only checkpoint correction. On production source SHA 80f1bfccd255536a0a2682c7d046f0216038a922, libdisk share-list #45471885325 PASS, Process inventory #45471885205 PASS, HTTPD auth #45471889467 PASS, NVRAM audit #45471885464 PASS, and XY-C1 #45471885211 PASS with Build, TRX verification and artifact upload all successful. L3 is VERIFIED for that exact production source tree; no hardware/router L4 claim.
- The only failure on 80f1 was AI handoff #45471885677, caused by missing mandatory checkpoint wording. Docs-only correction 5ec726d5123bbfde442f15b1afad114e64747511 passed AI handoff #45472190508. That commit changed only SESSION_HANDOFF.md and dev_log.md, so product source inputs remain identical to the L3-verified 80f1 tree. Its docs-only path filter did not rerun product workflows.
- Reconfirmed main 5ec726d5123bbfde442f15b1afad114e64747511 and frozen backup 72f4f157a084f8184b9597ca5bf334b8080e5807; backup untouched.
- Next concrete cross-userland task: deepen the WAN DHCP client state-transition tests around NAK/leasefail, active-lease deconfig retry, expiry and release while cleanup is pending. The current callbacks are advisory/no-op for NAK and leasefail, so preserve established runtime semantics unless a test demonstrates a real gap; use the WAN DHCP callback workflow.

---

## 2026-11-25 — Restore mandatory handoff checkpoint contract after exact-SHA CI failure

- Exact main 80f1bfccd255536a0a2682c7d046f0216038a922: libdisk share-list #45471885325 PASS (both GNU89 sanitizer regressions and public API/source-contract verification). This confirms the new FD-pinned del_folder policy and host regression harness pass in that workflow.
- AI handoff documentation contract #45471885677 FAILED at its validator: SESSION_HANDOFF.md no longer contained the mandatory end-of-session handoff checkpoint heading and its required statement that the checkpoint is mandatory while document edits are conditional. Restored the cross-session execution mandate and exact checkpoint rule in this documentation-only corrective commit.
- XY-C1 #45471885211 remained in progress at the Build step, with TRX verification and artifact upload pending. Parent be4e55398b9e4038f97f9cbc937686e7c74da96e XY-C1 #45469960684 also remained in progress at Build. Process, HTTPD and NVRAM on 80f1bf were still in progress at the last observation. No L3/L4 claim.
- Re-run the AI handoff validator on THIS COMMIT and inspect all same-SHA exact results. No source files changed in this corrective commit.

---

## 2026-11-25 — Pin folder deletion, ACL publication and recovery to one volume descriptor

- del_folder() previously checked the share, staged it, read/published per-account ACLs, refreshed the folder snapshot and attempted rollback through independently resolved pathnames. A mountpoint replacement during hotplug could therefore delete on one volume while changing ACLs or compensating on another.
- Reworked deletion to pin the mount root once, use no-follow fstatat plus descriptor-relative folder enumeration, prebuild ACL rewrite images with read_regular_file_nofollow_at, reserve/open a hidden staging directory with mkdirat/openat, and move/remove the share using renameat/unlinkat anchored to the two pinned descriptors. ACL write and rollback, folder snapshot refresh and staging recovery use those same descriptors. Failed directory compensation preserves the staged data instead of pruning its recovery parent.
- Added GNU89 policy regressions for descriptor-relative stage-name collision/error/truncation handling, mountpoint-to-symlink replacement, ACL write rollback restoring both ACLs and directory placement, and incomplete directory compensation retaining the staged folder. The source checker now rejects path-based del_folder() operations and asserts the FD transaction ordering.
- Parent exact main SHA be4e55398b9e4038f97f9cbc937686e7c74da96e repaired the XY-C1 implicit-declaration/static-definition build failure from #45468585947; at the last exact-head query its Actions run list had not appeared yet. This deletion slice has not yet been tested by Actions. Require exact-SHA libdisk and XY-C1 Build/TRX/artifact results before claiming L3; no hardware/L4 claim.

---

## 2026-11-25 — Restore C89 prototype for descriptor-relative folder lookup

- Exact main SHA `53ef9feca9ef9a031fae2782f0f83144be832d18`: libdisk host regression #45468585750 PASS; AI handoff #45468585944, Process inventory #45468585831, HTTPD auth #45468585722 and NVRAM audit #45468585988 PASS. XY-C1 push run #45468585947 FAILED during the MIPS build: `set_permission()` called `disk_share_folder_exists_in_directory_fd()` before its later `static` definition, producing an implicit declaration followed by “static declaration follows non-static declaration”. TRX verification and artifact upload were skipped.
- Added the file-scope prototype before `set_permission()` and a source-contract assertion that prevents the declaration-order regression. No runtime behavior changed. This is a buildability correction to the descriptor-pinned libdisk change.
- Exact-head libdisk and XY-C1 Actions must rerun on THIS COMMIT. L3 remains NOT VERIFIED until Build, TRX verification and artifact upload all pass on the same SHA; L4 remains NOT VERIFIED.

---

## 2026-11-25 — Pin folder rename and ACL rollback to one removable-volume descriptor

- Review found `mod_folder()` independently re-resolved mount_path for source/destination checks, ACL preparation, directory rename, snapshot refresh and rollback. A mountpoint replacement/hotplug race could move the folder on one volume while updating or compensating another volume's ACL files.
- Reworked `mod_folder()` to pin one directory descriptor and use no-follow `fstatat`, directory-FD existence scanning/cache refresh, bounded ACL leaf names, no-follow ACL reads, atomic descriptor-relative writes, `renameat`, snapshot publication and reverse compensation through that same FD. Added descriptor-relative batch policy helpers and GNU89 host regressions covering a mountpoint-to-symlink swap and ACL-write failure rollback.
- Exact a6cbec6ca88598ce1a85daabad9b7febd32e860c libdisk #45468099683 PASS: both GNU89 ASan/UBSan binaries and all Python source contracts; AI handoff #45468099898 PASS. Earlier dc094 libdisk #45467759017 passed both binaries but its obsolete reader-count assertion was fixed in a6cb. This production slice needs fresh exact-head validation. No firmware or hardware claim.

---

## 2026-11-25 — Update libdisk no-follow reader inventory contract for pinned-FD migration

- Exact dc09472450a390828a9814b0690585a088c2f11e libdisk run #45467759017 passed both GNU89 ASan/UBSan binaries but the Python source contract failed its global reader-count assertion. The set_permission() descriptor migration intentionally converted several former path reads to `read_regular_file_nofollow_at()`, reducing legacy path-wrapper occurrences from six to four while adding six FD-relative calls. Updated the guard to require both the remaining wrappers and descriptor-relative readers separately; no production source or test semantics changed.
- On dc094, AI handoff #45467759273 and Process inventory #45467759132 PASS. XY-C1 #45467758980, HTTPD auth #45467759477 and NVRAM #45467759168 were IN_PROGRESS at last poll. The libdisk fix needs fresh exact-head rerun.

---

## 2026-11-25 — Pin set_permission ACL read/modify/write to one volume

- Identified a mountpoint replacement race: set_permission() independently resolved mount_path for folder stat/cache, ACL read, missing ACL creation and atomic replacement. A hotplug replacement could read one device and write another.
- set_permission() now pins the mount root once and uses fstatat(AT_SYMLINK_NOFOLLOW), directory-FD folder enumeration and snapshot refresh, bounded ACL leaf formatting, read_regular_file_nofollow_at(), FD-relative default ACL creation and disk_share_replace_file_atomic_at() across the operation. Cleanup releases the pinned descriptor on success, unchanged and failure paths while preserving NVRAM transaction errors.
- Added a host path-swap regression: rename the original mount directory after opening its FD, replace the old mount path with a symlink to a different volume, then verify FD-relative ACL replacement changes only the original volume. Added source-order contract checks.
- Parent 1a62da7d321c4860c0531a289993a54192d67131 libdisk #45466418305 and AI handoff #45466418446 PASS; those validate only the preceding checker contract, not this source change. Fresh exact-head GNU89 ASan/UBSan and Python source checks are required.

---

## 2026-11-25 03:20 CST (UTC+8) — Align libdisk ACL updater contract with pinned descriptor API

- Exact b0d3012e1144f7edd3c6f16658da4d047c5ba8ae libdisk run #45465855322 passed both GNU89 ASan/UBSan host binaries, then failed the Python source contract at the ACL updater assertion. Production `disk_share_update_folder_permission_file()` now accepts the caller's pinned `directory_fd`, builds a bounded account leaf, reads it with `read_regular_file_nofollow_at()` and publishes with `disk_share_replace_file_atomic_at()`; the checker still expected the removed mount-path formatter/reader/writer API. THIS COMMIT aligns the contract to that descriptor-relative implementation and retains ENOENT-only skip and other-error propagation assertions. No product C or regression binary changed.
- Exact b0d3012 AI handoff run #45465855318 PASS. On f148, udhcpc #45465557325, USB storage #45465557087, NVRAM #45465557210, Process inventory #45465557323, Process regression #45465557058 and AI handoff #45465557053 PASS; XY-C1 #45465557176 remained IN_PROGRESS at the last check. No L3 or L4 claim.

---

## 2026-11-24 — Check staging mkdir/remove syscalls on their own policy helpers

- Exact fe2e86b3a6f3d36537bc3c6f9c6b64f04e37c86d libdisk run #45465306855 passed both GNU89 ASan/UBSan host binaries, then failed the Python source contract. The staging contract incorrectly searched rename_directory_entry_nofollow() for mkdirat()/unlinkat(); those calls correctly live in the separate create/remove directory-entry policy helpers. Updated the checker to extract and assert each helper independently rather than weakening the descriptor-relative/no-follow requirements.
- Exact 57db udhcpc run #45465005678 PASS. USB run #45465005107 BusyBox compile PASS but failed line 1334 because an old integration assertion expected one udhcpc retry-policy call; current f148 source has two intended callers and updated the assertion to two. Fresh f148 USB run #45465557087 queued at last poll.
- f148 XY-C1 #45465557176 and related actions were queued; 57db XY-C1 #45465005302 still IN_PROGRESS at last poll.

---

## 2026-11-24 — Align storage BusyBox caller contract with the second DHCP retry gate

- Exact 57db1f707f65c8f753d964ebf69d9cacfb26ec28 USB storage run #45465005107 compiled the host BusyBox after the DHCP brace fix, then failed its embedded Python caller-graph check because it expected exactly one call to udhcpc_script_failure_prepare_retry(). The new fail-closed ARP-probe path intentionally adds a second distinct caller alongside the existing event-script failure path; updated the expected count to two.
- Also moved resume_state to the top of the ARP-conflict deconfig block so this branch remains C89 declaration-before-statement compatible.
- Exact 57db udhcpc run #45465005678 PASS; its USB hotplug run failed only the stale call-count assertion. Its XY-C1 run #45465005302 was still IN_PROGRESS at last poll; inspect Build, TRX and upload steps.

---

## 2026-11-24 — Correct the escaped staging-helper signature in the libdisk checker

- Exact eb75f63ecd7ac2779fdbf53b989c94531ff3cd13 libdisk run #45464697321 passed both GNU89 ASan/UBSan host binaries, then the declaration-aware source checker failed at line 812 because the staging-cleanup signature was stored with literal backslash+n instead of matching the C function definition. Changed the contract to search by function name and let the extractor skip its earlier prototype.
- Exact eb75 udhcpc run #45464697251 PASS (GNU89 sanitizer policy test and all Python source assertions).
- Exact 57db1f707f65c8f753d964ebf69d9cacfb26ec28 udhcpc, USB hotplug, XY-C1, Process and NVRAM runs were IN_PROGRESS at last poll; the unmatched DHCP source block was corrected in this SHA. New exact-head verification required.

---

## 2026-11-24 — Make source-contract extractors declaration-aware

- Exact be691b89477fefaa2e0fc42606b9bf8ba4650528 udhcpc run #45464184832: GNU89 ASan/UBSan policy binary PASS; Python checker failed because it expected a standalone test invocation ending with semicolon, while the test is correctly called in main's nonzero-return condition. Assertion aligned to the real call form.
- Exact 8e3c3ba6702c2fe7a1a28ca3016a22611aad0b7a libdisk run #45463559390: both GNU89 ASan/UBSan host binaries PASS; Python checker matched a forward declaration for disk_share_account_metadata_status_at() and extracted the next unrelated body. The generic extractor now skips prototypes/call sites and has a synthetic prototype/unrelated-function regression.
- Exact be691 XY-C1 #45464184736 and 8e3c3 XY-C1 #45463559354 were still IN_PROGRESS at the last poll; continue independent work while they run. be691 AI handoff #45464184981 PASS. Fresh exact-head checker runs are required.

---

## 2026-11-24 — Fail closed when the udhcpc ARP preflight cannot run

- Audited the DHCP client address-conflict path. arpping() documented a tri-state result but collapsed setsockopt, sendto, poll and read failures into rv=1 (“no conflict”). dhcpc.c treated every nonzero result as a clear address and could commit a DHCPACK even when the probe failed.
- Updated arpping() to return -1 on socket-option/send/short-send/poll/read failures, retained 1 only for a completed probe without a conflict, and kept 0 for a detected conflict. DHCPACK processing now rejects negative results, preserves the current lease/state, charges rounded-up probe runtime against the existing deadline, and retries after a bounded interval.
- Added GNU89 policy regression coverage and source checks for the tri-state/error gate, and included arpping.c in the udhcpc stability workflow path filters. Exact-head host sanitizer/source Actions are pending; no firmware or hardware validation is claimed.

---

## 2026-11-24 — Restore C89 declarations for pinned-mount helpers

- Exact XY-C1 run 45461770986 on product SHA 572ec294eeae01befe36684ed7ab3102b9430e21 failed during MIPS musl compilation in disk_share.c: initial_all_var_file_in_mount_path() called disk_share_account_metadata_status_at() before its static definition; disk_share_create_folder_staging_paths() similarly called disk_share_remove_folder_path() before its static definition. The compiler emitted implicit declarations followed by conflicting static definitions. TRX and artifact steps were skipped.
- Added file-scope prototypes and static source-contract checks that each declaration precedes its first use and definition. Also corrected the add_folder checker to match the actual failure path (close the mount fd before releasing the NVRAM transaction; the success path releases the transaction first). Product behavior outside buildability is unchanged.
- Parent c7b44c5facfd78736909a869b79f07d6d7f8c5f3 libdisk run 45462879523 passed both GNU89 ASan/UBSan host executables but failed the incorrect add_folder ordering assertion; AI handoff run 45462879233 PASS. New exact-head reruns pending.

---

## 2026-11-24 — Align add_folder source contract with the pinned-directory implementation

- Exact-head libdisk run 45462263753 on 3629400054732dc685d102cb2aee0e78b9604e94 passed both GNU89 ASan/UBSan host executables, then failed the Python source checker at line 1338 because its add_folder contract still required pathname-based folder lookup, mkdir, ACL initialization and rollback. Production now pins one mount directory descriptor for lookup, creation, folder-list publication, ACL setup, deletion compensation and ACL compensation.
- Replaced the stale contract with ordered assertions for descriptor-relative operations, NVRAM account-snapshot lease, bounded target formatting, first-error cleanup and mount-path replacement regression. Product source unchanged.
- Exact-head rerun pending. AI handoff run 45462263766 PASS. XY-C1 run 45461770986 was still IN_PROGRESS on product source parent 572ec294; this checker-only follow-up does not change firmware inputs. L3 only if same production SHA Build/TRX/artifact all PASS.

---

## 2026-11-24 — Remove obsolete duplicate metadata-rebuild checker

- Exact-head libdisk run `45461771046` on `572ec294eeae01befe36684ed7ab3102b9430e21` passed both GNU89 ASan/UBSan executables, then a second, older static-check block failed by requiring pathname-based ACL writes. The same checker already contains the newer descriptor-pinned contract immediately above it; the duplicate stale block was removed so the current single-FD implementation is checked once.
- Product source unchanged. Re-run libdisk against this exact new SHA; XY-C1 firmware build for the prior SHA was still running when last polled.

---

## 2026-11-24 — Place rollback ACL updater before its caller

- The exact-head libdisk workflow `45461428507` passed both GNU89 ASan/UBSan binaries, then the source checker selected a forward declaration of `disk_share_update_folder_permission_file()` and parsed the following function instead of its implementation. Moved the descriptor-relative updater body ahead of `add_folder()` and removed the prototype/duplicate trailing body, preserving the same code while making the checker bind to the real definition.
- No behavior change; fresh exact-SHA libdisk and XY-C1 verification is required.

---

## 2026-11-24 — Pin full metadata rebuild and restore the shared list destructor

- The XY-C1 compiler exposed a source truncation: `free_2_dimension_list()` had lost its body and opening brace, so the compiler parsed later function definitions as old-style parameters. Restored the established element/free/reset implementation from the frozen baseline and added a source contract check.
- Completed whole-volume metadata consistency: `initial_all_var_file_in_mount_path()` now opens the removable-volume directory once, preflights ACL targets on that descriptor, revalidates the account inventory under the NVRAM lease, writes all ACL files, publishes the folder snapshot and prunes stale ACL files using the same FD. The stale pruner now borrows rather than closes the caller's descriptor.
- Updated the contract checker to require this exact single-FD ordering. No local build is claimed; exact-SHA sanitizer and firmware checks remain outstanding.

---

## 2026-11-24 — Check the new caller-pinned ACL initializer API

- Exact-head libdisk run `45460328727` on `552699f8003dbd1e48c2c109e526a77b4b15da8d` passed both GNU89 ASan/UBSan host test binaries, then the source checker failed because its ACL-initializer contract still extracted the old pathname-taking worker and expected it to open its own directory. The implementation now separates `create_if_no_var_files_with_accounts_at(directory_fd,...)` from a compatibility pathname wrapper; the checker now validates all-account preflight/write ordering on the caller-pinned FD and separately checks wrapper delegation/close semantics.
- Checker-only correction; product source and regression remain unchanged. Fresh exact-head CI is required.

---

## 2026-11-24 — Restore add_folder and pin creation/rollback to one volume

- The latest main commit had replaced the public `add_folder()` implementation with its rollback ACL helper, leaving the header-declared API without a definition. Restored the operation and closed its mountpoint-swap race: folder existence scan, `mkdirat`, snapshot publication, account ACL initialization, rollback removal and ACL compensation now all use one opened mount directory descriptor.
- Added a GNU89 policy regression that renames the volume and redirects the former mount path before directory create/rollback; both operations remain on the original descriptor and leave the redirected volume untouched. The rollback ACL writer now accepts the pinned FD instead of reopening the mount pathname.
- Restored the static contract to the current metadata rebuild's per-operation pinned wrappers. CI boundary: prior host ASan/UBSan binaries passed, but this restored API/source slice requires fresh exact-SHA validation. No local build claimed.

---

## 2026-11-24 — Anchor staging-removal checker to its definition

- Exact-head libdisk run `45458513380` on `c2e9b4213b3cd039061539a52141d04e17b9e6bb` passed the GNU89 ASan/UBSan regressions but failed the static staging-helper contract. The checker searched for `disk_share_remove_folder_path(` and captured its earlier cleanup call inside the staging-path builder instead of the helper definition. It now anchors extraction to the full `static int` definition, so the pinned-parent/remove-leaf/cleanup assertions exercise the actual helper body.
- No production source changed; exact-head test verification remains open.

---

## 2026-11-24 — Correct staging-helper source contract formatting

- Exact-head libdisk run `45458316166` on `2d8f56d3fc5fea4ab29722e781591646e5278de2` passed both GNU89 ASan/UBSan host test executables and then failed the static staging check. The checker searched for a whitespace-free call string in the raw C source, while the real call is intentionally line-wrapped; the dedicated extracted-function/whitespace-normalized check already validates the exact descriptor-relative helper call.
- Replaced the raw formatting-sensitive assertion with a symbol-presence check, retaining the function-level semantic assertion. No production C source changed; exact-head verification is required.

---

## 2026-11-24 — Validate the no-follow reader split after descriptor pinning

- Exact-head libdisk run `45457952519` on `88c485d15624e6decffc8b09f34cf5c39318e586` passed both GNU89 ASan/UBSan binaries, then the static checker failed because it searched for the old inline pathname-reader body in `disk_io_tools.c`. The implementation now separates `read_regular_file_nofollow_at(parent_fd, leaf)` from a small path wrapper; the checker now validates the descriptor-relative `openat`/regular-file/stream path and the wrapper's pinned-parent delegation/cleanup. It also explicitly requires the mount-replacement regression for the pinned helper.
- No product C source changed in this checker repair. Run a new exact-SHA check; prior failures remain bound to their historical commits.

---

## 2026-11-24 — Align libdisk metadata-rebuild contract with current implementation

- The libdisk workflow's GNU89 ASan/UBSan host regressions completed, then the Python source checker failed because it still extracted the deleted public `create_if_no_var_files()` wrapper and depended on its removed worker contract. The current whole-volume rebuild is `initial_all_var_file_in_mount_path()`; its checker now enforces checked inventory/path preflight, NVRAM writer lease and inventory revalidation, anonymous/account ACL publication, folder snapshot publication before stale-file pruning, and transaction release.
- Checker-only correction; production source is unchanged. Previous workflow failures remain tied to their exact SHAs. THIS COMMIT must pass the libdisk contract workflow before this slice is called validated.

---

## 2026-11-24 — Fix relocated VSFTPD ACL open assertions

- Exact source SHA `9fe57d025796fbc0d6574dc68b9f71c5dc80903b`: Process lifecycle #45448598258 PASS and AI handoff #45448598098 PASS. XY-C1 #45448598232 remained IN_PROGRESS; NVRAM #45448598003 was CANCELLED.
- VSFTPD #45448598152 FAILED at inline Python line 450 before running host regression binaries. The new bounded-file type/size checks correctly live in `ftp_acl_path_open_bounded_regular_file_at()`, but two legacy duplicate assertions still looked for them in the compatibility wrapper. Removed only those stale assertions; equivalent assertions on the `_at` helper remain.
- The latest independent libdisk parent commit is `0f812eef2ce02057c8d95227dcad7d1478f758ba`; its Process #45448794129, XY-C1 #45448794152, libdisk share-list #45448794198 and NVRAM #45448794248 were pending at last poll. Exact-head VSFTPD and AI reruns for THIS contract correction are pending.

---

## 2026-11-24 — Revalidate FTP ACL mount identity across metadata reads

- Parent-pinning commit `ecb381796f856495b41d0d76e87718c82944ea6d` changed the ACL path helper to pin the complete parent directory and open the metadata leaf with `openat()`; ancestor symlinks, final symlinks and FIFO/device nodes are rejected. Exact-head VSFTPD #45447276290, Process #45447276310 and AI handoff #45447276525 PASS; XY-C1 #45447276297 and NVRAM #45447276423 were IN_PROGRESS at last poll.
- A second race remained: the helper closed the pinned parent immediately after opening the leaf, so the reader could not detect a mount pathname replacement during the read. This slice retains the directory descriptor through the full ACL read, verifies parent identity before open, after open and after reading, and compares the opened file device, inode and size before accepting its contents. A changed mount path or file object now fails closed.
- Added GNU89 host regressions for descriptor-relative regular metadata open, symlink and FIFO denial, and a parent-path replacement where the pinned descriptor still accesses the original metadata inode while path identity returns `ESTALE`. The VSFTPD source contract now checks the production revalidation sequence.
- Metadata writer audit: `libdisk/disk_share.c` stages ACL metadata with `mkstemp`, `fsync` and atomic `rename`; shared account validation excludes path separators. Writer parent-dirfd conversion remains separate work.
- Source SHA `6d4b0537316390ca00a388a015edb828456cc213` has exact VSFTPD #45444917834, Process #45444917951, NVRAM #45444916236 and XY-C1 #45444916327 PASS; Build, TRX verification and artifact upload all passed on that exact SHA (L3 verified for it only). Documentation-only descendant `f6ed1313a31c9a4a89cc9a44d3994869719e5c08` passed AI handoff #45445344806. This follow-up's exact-head Actions are pending.

---

## 2026-11-25 — Restore required AI handoff section contract

- Exact parent SHA `6d4b0537316390ca00a388a015edb828456cc213`: VSFTPD #45444917834 and Process inventory #45444917951 PASS; NVRAM #45444916236 and XY-C1 #45444916327 IN_PROGRESS. AI docs #45444916200 failed because the handoff had renamed the required `## Active task` section to `## Active source tasks`.
- THIS commit restores the required heading while retaining both active FTP work items and updates exact-head validation/next-action notes. No product source changed.
- XY-C1 Build/TRX/artifact remain pending on the parent source SHA; no L3 claim.

---

## 2026-11-25 — Reject FTP account path separators in ACL metadata lookup

- The post-login ACL reader `get_permission_ftp()` built `mount_path/.__<username>_var.txt` from the authenticated FTP username without rejecting `/`; this username originates in the FTP `USER` command and Padavan's custom NVRAM account lookup does not itself validate path characters. The old 256-byte `snprintf` result was also unchecked, so a long mount/user string could silently select a truncated filename.
- Added `ftp_acl_path_build_account_permission_path()`: mount paths must be absolute, usernames cannot add path components, and any output truncation is rejected with `ENAMETOOLONG`. Invalid metadata paths fail closed (zero ACL rights). The valid filename convention is unchanged.
- Added GNU89 host regressions for a normal account, slash-based traversal input, relative mount path and short output buffer; extended the VSFTPD source contract to pin the helper and forbid the old unchecked formatter.
- Parent `0fa6f300b6bfa8353dd5a756a41c099b6afa9d73`: VSFTPD #45443260653 PASS; USB Storage Hotplug #45443260461 PASS; NVRAM #45443260389 PASS; Process Inventory #45443259419 PASS; HTTPD #45443259557 PASS; AI handoff #45443260460 PASS; Firewall Startup #45443260423 PASS; XY-C1 #45443260501 remained IN_PROGRESS, so current firmware L3 is not yet established. This source slice's exact-head Actions are pending.

---

## 2026-11-25 — Reconcile firewall state after a direct FTP stop

- Found a second path bypassing the restart fix: `rc.c` dispatches the public `/sbin/rc stop_ftp` action to `stop_ftp()`, but that function stopped the daemon under the storage lock and unlocked immediately without invoking `control_ftp_fw()`. A confirmed RUNNING-to-ABSENT transition could therefore retain stale FTP firewall/UPnP mappings when using the direct stop command.
- THIS COMMIT captures prior process presence under the same storage-operation lock, calls the shared confirmed-stop predicate, reconciles firewall exactly once only on successful stop, and merges firewall refresh status without masking the stop error. The lock remains held through reconciliation to prevent hotplug/restart races.
- Renamed the pure status predicate to `ftp_stop_should_reconcile_firewall()` and reused it from restart/public-stop paths. Extended GNU89 policy test/source contracts for stop outcomes, ordering and the `rc.c` action route.
- Parent `377f5a6670caefe0da2e46b8fd8b97977ca8543c`: AI #45442773576 PASS; HTTPD #45442773609 was IN_PROGRESS following the firewall checker correction. Parent `5e955b3` had XY Build IN_PROGRESS, TRX/artifact pending. This commit's exact-head regression results are pending.

---

## 2026-11-25 00:54 CST (UTC+8) — Validate all firewall startup status arguments

- Exact main `5e955b3bb2015e8db61a7e4c23eb44d29f873f8e` reran HTTPD Security #45442258062 and AI handoff #45442257900. AI handoff PASS; HTTPD still failed at inline Python line 204 because the first correction updated the boolean-helper/gate assertions but left the old function-argument suffix assertion unchanged.
- Replaced that stale substring with an eleven-argument order check for `firewall_startup_policy_wan_failure_status()`, normalized only for whitespace. The status-return branch, first-error boot-status merge, IPv4/IPv6 forwarding-disable checks and all existing HTTPD security regressions remain required. No production source changed.
- FTP restart source SHA `7ff02de99fc676e0262b7c5770356b1e31bda758`: VSFTPD #45441167767, USB hotplug #45441189222, storage enable #45441167587, Process #45441167968 and NVRAM #45441167946 PASS. XY-C1 #45441167736 is `in_progress` / `pending`; Build/TRX/artifact still require exact-step confirmation. No L3/L4 overclaim.

---

## 2026-11-25 00:49 CST (UTC+8) — Align HTTPD Security firewall contract with status-return gate

- HTTPD Authentication/Security run #45441167757 on source SHA `7ff02de99fc676e0262b7c5770356b1e31bda758` failed at `Check firewall command status reporting`. The firewall C policy/restore tests passed before its static script asserted the obsolete `firewall_startup_policy_allows_wan_checked()` call. This was a stale cross-workflow contract after RC now returns and merges `firewall_startup_status`.
- Replaced only that assertion with checks for the status helper, nonzero rejection branch, exact `boot_services_status, firewall_startup_status` merge operands and ordering before `start_wan()`. Existing forwarding-disable and actual HTTPD auth/disclosure assertions remain intact.
- The roadmap size correction commit `5248d91` is validated by AI handoff #45441892663 PASS. FTP source commit `7ff02de` XY-C1 #45441167736 still IN_PROGRESS at last poll.

---

## 2026-11-25 00:46 CST (UTC+8) — Fit active roadmap within the handoff validator limit

- AI handoff #45441278100 on exact main `a8284ea16840ffbd10f3c0444e68a829bfc7d9d5` failed only in live handoff/roadmap validation: the roadmap was 131,452 UTF-8 bytes, 380 bytes above the 131,072-byte cap. Switch CLI #45441278142 PASS.
- Compressed one stale, over-detailed historical USB checked-read bullet, preserving the checked selectors, absent-key compatibility, error aggregation, regression coverage and DHCP/DDNS persistence outcomes. The roadmap is now 130288 UTF-8 bytes.
- Parent `7ff02de` FTP firewall reconciliation source change passed FTP, USB storage, Process, Firewall, Samba, NFS, MiniDLNA, Firefly and storage-enable checks; XY-C1 #45441167736 remained in progress. HTTPD Authentication/Security #45441167757 failed; inspect its job log before deciding whether source or checker needs correction.

---

## 2026-11-24 — Reconcile FTP firewall state after every confirmed stop

- Found a concrete VSFTPD lifecycle gap in `restart_ftpd()`: after `stop_ftp_locked()` succeeded, `control_ftp_fw()` ran only inside the mounted-storage + successful-relaunch branch. If storage was absent, mount inventory failed, or `run_ftp()` failed, a transition from RUNNING to ABSENT bypassed firewall/UPnP reconciliation and could leave stale FTP rules.
- The production flow now records an inventory failure (errno or EIO), still skips an unsafe FTP start, and always calls the firewall transition reconciler after a confirmed stop. The first mount/start error is preserved ahead of firewall refresh failure. Stop failures still abort before reconciliation because daemon absence was not proven.
- Added `ftp_restart_should_reconcile_firewall()` and GNU89 regression cases; extended the VSFTPD integration source contract to enforce one reconciliation call after all mount/start branches, without a post-stop early exit. The Storage Hotplug workflow also watches these production/test paths.
- Parent SHA `79c278b3f12cd95db5880052c95e912d482e96c5` repairs two exact-head contract failures from `a0643ff`: Switch CLI #45440335408 had passed the Clause 22 host binary but its source assertion did not recognize the combined register/data condition; AI handoff #45440335554 required the literal `Before context exhaustion`. Exact reruns for 79c have not yet been observed.
- Parent source SHA `18d90355fb56b33100c05439df30ecc3e61ef85f`: AI #45439390998, Switch CLI #45439391302 and NVRAM #45439391348 PASS; XY-C1 #45439391086 was in progress (Build running, TRX/artifact pending).
- XY-C1 #45437415030 on `a650ed8c8fdd8b0100bc26bb91458cdf4bde9277` and #45438125350 on `bc17758dcb25dc349c9304a6d3f671600fc7f5c7` passed same-SHA Build, TRX and artifact upload. XY-C1 #45438348129 on `7ae410c1ff14d974e659b8c29bbcd83d18269f0c` was still in progress (Build running; TRX/artifact pending) at last poll. No local repository checkout/toolchain was available; no local compile is claimed. New exact-head VSFTPD/Storage/AI/XY validation is pending.

---

## 2026-11-24 — Repair Clause 22 path-aware source-contract anchors

- Exact main SHA `a0643ffbe0b7e715c265de12d5b6a3de48b5303f`: Switch CLI #45440335408 passed the Clause 22 GNU89/ASan/UBSan boundary binary, then failed its source contract because the new combined direct C22 guard no longer matched the checker’s old exact substring ending immediately after the register predicate. Updated the checker to anchor the predicate within the combined register/data condition; boundary policy and production source are unchanged by this correction.
- AI handoff #45440335554 failed because the handoff omitted the required cross-session phrase `Before context exhaustion`; this checkpoint restores that explicit rule.
- XY-C1 #45440335185 and NVRAM audit #45440335316 on `a0643ffbe0b7e715c265de12d5b6a3de48b5303f` were IN_PROGRESS at last poll. XY Build was running; TRX/artifact pending. Exact new-head checks remain required.
- The separate FTP restart firewall cleanup finding is being implemented as an independent source/test slice; no FTP files are changed in this checker/handoff correction.

---

## 2026-11-25 00:38 CST (UTC+8) — Preserve indirect Clause 22 logical register range

- Reviewed `18d9035` after its C22 width guard landed and found a compatibility regression: the five-bit register / 16-bit payload checks ran before the architecture-specific indirect access selector. The existing indirect policy tests use logical register `0x1234` and a u32 value `0x89abcdef`, which are intentionally split across page/low/high word MDIO transactions.
- Adjusted `mii_mgr_read/write()` so the indirect register-31 path uses a bounded 16-bit logical register and accepts the full u32 payload; direct C22 remains limited to a five-bit register and 16-bit payload. PHY address remains five-bit on both paths.
- Added tests for 16-bit indirect register boundaries and updated source contracts to prove each path validates before its matching transaction.

---

## 2026-11-25 00:36 CST (UTC+8) — Correct Clause 45 regression call count and restore the required handoff rule

- Exact source SHA 7ae410c1ff14d974e659b8c29bbcd83d18269f0c: its Clause 45 GNU89/ASan/UBSan policy binary passed; Switch CLI #45438347998 then failed because the newly added source checker used undefined policy/test variables. The parallel checker-load fix landed as 52e34a54f58930247d5ec5c5df0f1a99b6de52fb, but exact Switch CLI #45438676877 exposed a second checker-only mismatch: the regression contains exactly four calls to raeth_mdio_cl45_write_data_status(), not five. This commit asserts four calls, verifies the named boundary test and directly pins the policy header include at the kernel call site.
- AI handoff #45438348166 (7ae) and #45438677094 (52e) both failed the same required-handoff-token check: SESSION_HANDOFF.md must contain the exact phrase "one atomic main commit". THIS commit restores that governance statement.
- Exact XY-C1 #45438348129 and NVRAM audit #45438348160 on source SHA 7ae were still IN_PROGRESS at the last poll; do not transfer their outcomes to later commits. XY Build was still running, TRX/artifact pending. L3 requires Build, TRX and artifact PASS on one exact production SHA.

---

## 2026-11-25 00:26 CST (UTC+8) — Bound Clause 45 MDIO write payload to 16 bits

- Confirmed `mii_mgr_write_cl45()` ORed a full-width `u32 write_data` directly into the MDIO command register although Clause 45 payload data is only 16 bits. Added a fail-closed policy check before lock acquisition and any MMIO/MDIO access.
- Added GNU89 policy regression for 0, 0xffff, 0x10000 and 0xffffffff; extended Switch CLI source contract to require rejection before `spin_lock_irqsave` and before command writes.
- At parent `bc17758dcb25dc349c9304a6d3f671600fc7f5c7`, Switch CLI #45437706525 PASS on checker correction SHA `472064b`; AI docs #45437414952 PASS on `a650ed8`; NVRAM #45437414867 and XY-C1 #45437415030 on `a650ed8` remained running. XY-C1 #45436853853 on `65c6051` completed SUCCESS with Build/TRX/artifact SKIPPED (checker-only).
- Earlier a08 XY-C1 fail due missing MDIO address policy header was fixed in a650. Exact-head validation of the current source is pending.

---

## 2026-11-25 00:22 CST (UTC+8) — Fix Clause 45 policy declaration in the production MDIO path

- XY-C1 #45436447371 on `a08b86e` failed during kernel compile: `mii_mgr.c:256` called `raeth_mdio_cl45_address_status()` without including its declaring header, and `-Werror=implicit-function-declaration` stopped the build. TRX/artifact were skipped.
- Added `#include "raeth_mdio_cl45_address_policy.h"` to `mii_mgr.c`. The existing Clause 45 GNU89 policy regression is unchanged; exact-head Build/TRX/artifact is the verification gate.
- The earlier XY-C1 run #45434352284 for the firewall source fix `7ca46fc` has now passed Build, TRX verification and artifact upload (L3). The later check-only commit does not invalidate that source-SHA evidence.
- XY-C1 #45436853853 on `65c6051` predates this include fix and is expected to expose the same compiler failure; inspect its final conclusion separately.

---

## 2026-11-25 00:21 CST (UTC+8) — Retarget IPv4 forwarding integration check to firewall rejection status

- Exact `1c4cdd6` Firewall Startup job #45436267565 passed its firewall policy source assertions and host regressions, then failed the existing forwarding-control checker because it searched for the removed boolean gate. `scripts/check_forwarding_control_status.py` now extracts the current `if (firewall_startup_status != 0)` branch; the checks still require IPv4/IPv6 forwarding-disable errors to be captured and WAN startup to remain absent from that rejection block.
- AI handoff #45436267737 on `1c4cdd6` PASS. RAETH shared Clause 45 exact SHA `a08b86e`: AI handoff #45436446803 PASS, Switch CLI #45436447892 and NVRAM #45436447356 in progress, XY-C1 #45436447371 in progress.
- Firewall production source SHA `7ca46fc` XY-C1 #45434352284 still pending Build/TRX/artifact at last poll; no L3 claim.

---

## 2026-11-25 00:18 CST (UTC+8) — Anchor firewall boot-status assertion after the gate

- Exact SHA `77dc29cb83a5c9ce45a3f9262d85b8429dee4b43`: Firewall Startup #45435774161 advanced through the earlier checks but failed because the source checker selected an earlier boot-service status merge, not the firewall-specific merge. The checker now begins its merge lookup after the firewall gate, validates the exact first-error argument, and requires that merge before `start_wan()`.
- Exact SHA `77dc29c`: AI handoff #45435774251 PASS. On follow-up RAETH MDIO checker commit `2f95d1c`, Switch CLI #45435916372 PASS and AI handoff #45435916408 PASS.
- XY-C1 #45434352284 (`7ca46fc`) and #45434537034 (`a76f771`) remained in progress at last poll; Build/TRX/artifact must be confirmed per exact SHA.

---

## 2026-11-25 00:15 CST (UTC+8) — Normalize firewall merge checker and restore AI handoff checkpoint

- Exact SHA `03ca0dbf9b201bb8752c53afbf0b44632c1818e8`: Firewall Startup #45435162086 failed because the C boot-status merge call wraps arguments after the opening parenthesis; the static contract now checks the merge call, exact `boot_services_status, firewall_startup_status` operands and ordering independently.
- AI handoff #45435161894 and #45435366901 failed because the handoff omitted the exact sentence `The checkpoint is mandatory; a document edit is conditional.` and expected top-level section names. Handoff rewritten to meet the current checker contract and retain precise SHA/CI status.
- Newer RAETH MDIO Clause 22 address guard's policy test passed, but Switch CLI #45435366858 failed later in its source-order checker; next step is to inspect the precise function-body mismatch. XY-C1 #45434352284 and #45434537034 builds remained pending at last poll.
 
---

## 2026-11-25 00:15 CST (UTC+8) — Correct firewall startup workflow source contract

- Exact SHA `7ca46fc7049f3380755f6b2fe5073d86727c6382`: firewall policy and checked-NVRAM host regressions passed; the static startup contract failed because it expected `firewall_startup_status = firewall_startup_policy_wan_failure_status(` as one contiguous line, while the valid C assignment is line-wrapped. Changed the contract to assert the status assignment and helper invocation separately.
- Exact-head Process inventory #45434352443 PASS. AI handoff #45434352428 was cancelled after a newer concurrent push; rerun on this commit. XY-C1 #45434352284 still had Build XY-C1 IN_PROGRESS, TRX and artifact pending. No L3 claim.
- Concurrent RAETH Clause 45 ioctl bounds commit `a76f771cd56c1ad623769d32400562f853330e34` landed after `7ca46fc`; its Switch CLI and XY-C1 actions require exact-head inspection.

---

## 2026-11-25 00:09 CST (UTC+8) — Bound RAETH QDMA selectors and switch rate ioctl inputs

- Main commits `c0ba5c6`, `56f4a10`, `4ab45d3`, `4920912` and `a937cc4` bound QDMA queue selectors/register offsets and ESW ingress/egress rate ioctl port/bandwidth inputs. The `4920912` Switch CLI regression exposed a checker false-positive: the egress slice still began at the ingress case, so its order assertion mixed the two paths. `a937cc4` now extracts each switch case by explicit boundaries and adds QDMA access-path coverage.
- Parent `a937cc4` Switch CLI #45432913975 PASS, NVRAM #45432913762 PASS, AI docs #45432913776 PASS; XY-C1 #45432913772 was in progress at last poll. The failed #45432229601 belongs to the superseded `4920912` checker. Hardware L4 remains unverified.

---

## 2026-11-25 00:09 CST (UTC+8) — Preserve firewall startup failure in PID 1 readiness status

- Found that the firewall/NAT prerequisite gate safely blocked `start_wan()` on required NAT restore or firewall-filter errors but did not merge those applicable statuses into `boot_services_status`; PID 1 could report READY with WAN deliberately withheld.
- Added `firewall_startup_policy_wan_failure_status()` to retain the first applicable selector/VLAN/NAT/IPv6/filter error. `init_router()` now merges it before the no-WAN branch; failed baselines for explicitly disabled features remain non-blocking.
- Added GNU89 tests for exact error propagation, first-error precedence, disabled-feature compatibility and READY→DEGRADED mapping. Workflow source checks require the merge before WAN launch and both host-test header paths are now in push/PR filters.
- Local GNU89 `-Wall -Wextra -Werror` + UBSan compile/run PASS. ASan build succeeded, but this container could not reserve ASan shadow memory at runtime; exact-head GitHub regression must complete the ASan run. L3 depends on same-SHA XY-C1 Build/TRX/artifact PASS; L4 is NOT VERIFIED.

---

## 2026-11-24 — Fix SmartDNS checker extraction and cover failed KILL postcondition

- Exact parent `9416e328`: Process inventory #45427406147 failed because the static checker supplied a literal backslash-n in the function-boundary string for `smartdns_signal_process_ids()` instead of matching actual newlines. AI handoff #45427406412 also failed because `SESSION_HANDOFF.md` did not include the exact required phrase `one atomic main commit`.
- Correct the checker delimiters; add executable test markers for unknown daemon state, verified absence after graceful TERM, unknown post-signal no-KILL, and TERM-before-KILL escalation. Add a stuck-daemon test proving the stop function reports still-running after both bounded signals, while continuing to assert zero name-wide killall calls.
- Parent `9416e328` Process lifecycle inventory #45427405894 and DNSMasq #45427405995 passed; USB storage #45427406211 and XY-C1 #45427405793 were pending at last poll. This correction needs fresh exact-head Process/AI/firmware validation.

---
## 2026-11-24 — Pin PID-scoped SmartDNS stop in runtime and workflow triggers

- The SmartDNS source checker was aligned with the live helper contract in `af57e892ed4f1671cd1262feb83c0a98eaea9272`; this follow-up adds executable assertions for per-PID executable/start_time revalidation and the exact `kill -TERM/-KILL <pid>` path.
- The shell lifecycle fixture keeps a killall mock as a tripwire and now requires `KILLALL_CALLS=0` after graceful, forced and UNKNOWN stop scenarios. Both Process workflow push and pull-request filters now include `trunk/user/smartdns/**`, so changes to the shell helper and fixture trigger the same regression suite.
- Exact-head Process run #45427108566 for the checker correction was IN_PROGRESS at last inspection. Parent `822d8a8` XY-C1 run #45425821347 remained IN_PROGRESS with Build active and TRX/artifact pending. This commit's Process and AI Actions must be checked on its own SHA.

---
## 2026-11-24 — Align SmartDNS lifecycle source contract with current tri-state helpers

- Exact Process inventory run #45425821194 on `822d8a8c40808290825d838d746d7fec034bb2fc` ran the SmartDNS shell regression successfully, including the new procfs-default case, PID identity checks, stop revalidation, Adbyby ownership and adblock conversion. It then failed in `scripts/check_smartdns_lifecycle.py` because the static checker still expected a former generic `_smartdns_process_name` helper and `killall smartdns` behavior.
- Current source intentionally separates `smartdns_named_process_state()` for Adbyby from `smartdns_process_state()` for verified SmartDNS executable/argv/start_time identity, and uses PID-scoped TERM/KILL with fresh post-signal state checks. Update the checker to assert the actual tri-state helper branches and PID-scoped stop path rather than obsolete name-wide killall behavior.
- AI handoff #45425821211 on parent `822d8a8c40808290825d838d746d7fec034bb2fc` PASS. Its XY-C1 run #45425821347 remained in progress at last poll; the Process failure was a stale checker assertion, not a shell runtime regression.

---
## 2026-11-24 — Include DNS Forwarder production sources in XY-C1 validation triggers

- Exact SHA `0761c14cd929958b6398fa862f93e74b635d82ce`: DNS Forwarder launch-path #45424750772, AI handoff #45424750770 and Process inventory #45424750807 all PASS. No XY-C1 run was created because `xy-c1-firmware-validation.yml` did not include `trunk/user/dns-forwarder/**` in its push path filter.
- Add the production package path to the XY-C1 workflow and a unit regression that proves the path is selected and classified as a firmware input. This commit also ensures the workflow file itself is in its own trigger set so later filter edits cannot silently skip validation.
- The DNS Forwarder source fix itself is host/contract L2 verified; firmware Build/TRX/artifact L3 and physical BusyBox/MIPS DNS behavior remain unverified until exact-source evidence exists.

---
## 2026-11-24 — Confirm ambiguous DNS Forwarder pidof absence with procfs

- `dns_forwarder_process_state()` documented that BusyBox `pidof` status 1 can mean an incomplete procfs scan, but treated empty output as confirmed ABSENT. That could launch a duplicate DNS Forwarder instance when process discovery was unreliable.
- The service now cross-checks ambiguous pidof absence with the installed shared `pl_process_state()` inventory. A procfs-visible matching task or an incomplete procfs scan maps to UNKNOWN and blocks start/stop actions. Regression cases inject a hidden `/proc/4242` daemon and a missing procfs root; both must leave launch and signal counts unchanged. The source checker pins helper installation and workflow path filters.
- Parent `fd81fc8c6863f2ca59d3610dfead53db076173ee` contains the latest SmartDNS source/test follow-up. This change requires fresh exact-head DNS Forwarder, Process, AI and XY-C1 validation; no firmware L3 or hardware L4 claim.

---
## 2026-11-24 — Align generated include redirection with source contract

- The exact-head DNSMasq run confirmed both new runtime write-failure regressions pass. Its source checker then caught a formatting mismatch in the two checked whitelist/blacklist truncation statements (the shell accepted `: >/tmp/...`, while the contract expected `: > /tmp/...`). Normalize the spacing; no behavior change.

---

## 2026-11-24 — Load SmartDNS generators before their write-failure tests

- Review of the newly extended fixture caught that the real generator extraction block was still located below the inserted base-generator tests. Move the extraction/source step to the test preamble so both base and secondary regressions execute against the production function definitions.

---

## 2026-11-24 — Fail closed on every SmartDNS base-config write

- `Get_sdns_conf()` checked the secondary resolver helper after the previous slice, but its own dozens of staged-config appends remained unchecked. If the staging filesystem filled or an individual append failed, a later successful echo or conditional could mask the error and allow an incomplete resolver config to be published.
- Every base-config append now returns its write status immediately. The primary stage truncation and generated whitelist/blacklist files are checked as well, so a missing include cannot be silently referenced.
- Extended the existing generator regression to inject a mid-file echo failure (asserting the exact status and that no later directive is written) and a `/dev/full` failure. The source checker requires every staged-config append and generated include write to be guarded.

---

## 2026-11-24 — Propagate SmartDNS secondary resolver bind write failures

- `Get_sdnse_conf()` ignored both generated bind-line write statuses. With TCP binding disabled, a failed UDP bind append was followed by a false conditional whose success masked the write error; `Get_sdns_conf()` also ignored the helper return, allowing publication of a config missing the secondary listener.
- Each UDP/TCP bind write now returns its original failure status, and the caller aborts generation before publishing an incomplete staged config.
- Extended the existing SmartDNS atomic-config regression with /dev/full first-write failure and an injected second-write failure. The source checker pins both error paths. Exact-head DNSMasq/XY-C1 results are pending for this change.

---

## 2026-11-24 — Balance the RAETH SET_LAN_IP lock on rejected input

- Review found an unbalanced lock path introduced by bounded `RAETH_SET_LAN_IP` validation: the validation error `break` ran after `page_lock` had already been released, then reached an unconditional unlock in the common epilogue.
- Validate the copied fixed-width buffer while the lock is released, then reacquire `page_lock` unconditionally before branching on the validation result. The Switch CLI source-order contract now pins unlock → copy → validate → reacquire → reject-or-copy. Existing GNU89 policy coverage includes valid, empty, unterminated, NULL and zero-length inputs.
- Parent `9804865fa5c34b5e7e00b57ce3949dca89ab5893`: DNSMasq #45421011367 and Process inventory #45421011328 PASS; USB #45421011314 PASS; XY-C1 #45421011312 was still in progress. AI handoff #45421011316 failed only because the handoff omitted a required literal checkpoint sentence; the same commit repairs that wording. New exact-head Switch/AI/Process/NVRAM/DNSMasq/XY-C1 checks are required. No L3/L4 claim.

---
## 2026-11-24 — Make SmartDNS procfs-root fallback safe under nounset

- The SmartDNS process-state helper defaulted `_smartdns_proc_root` to `/proc` but then expanded an unset `SMARTDNS_PROC_ROOT` directly. In strict-shell callers (`set -u`), a confirmed-absent `pidof` result could therefore abort before checking `/proc/1/stat`; the earlier identity helper had the same unsafe override expansion.
- Both `smartdns_process_state()` and `smartdns_process_identity()` now use `${SMARTDNS_PROC_ROOT:-/proc}`. The regression explicitly unsets the override and confirms the absent-process result before building a fake proc tree; the source checker pins both default expressions and the regression case.
- Parent `9e602b00973a2c41c109fc47dc50b57f43ecf7ba`: AI documentation #45420686264 PASS; DNSMasq lifecycle #45420685909 was queued at last observation. Prior runtime tests passed through process lifecycle, Adbyby ownership, DNSMasq preservation and atomic config; the then-failing static assertion was updated in the parent. New exact-head Actions are required for this change.

---
## 2026-11-24 — Align SmartDNS process checker with deterministic PID reuse test

- DNSMasq's runtime regressions now pass through the deterministic snapshot/replacement test, but the source checker still looked for the old race-test phrase. It now asserts the observable invariant: a replacement process must never receive a signal.

---

## 2026-11-24 — Correct SmartDNS fixture extraction checker

- The runtime DNSMasq fixture now passes with its renamed Adbyby-only helper, but the source checker retained one additional assertion for the former function name. Updated the expected two extraction sites so the contract reflects the verified runtime setup.

---

## 2026-11-24 — Update SmartDNS Adbyby ownership source contract

- The DNSMasq contract checker still required the old shared SmartDNS process-state helper name after the production code split it into a dedicated executable-identity inventory and an Adbyby-only read-only name probe. Updated the stale source assertion to match the new call site; the runtime regression itself passed on the prior exact SHA.

---

## 2026-11-24 — Make SmartDNS PID-reuse regression deterministic

- The first fake-proc race test attempted to swap the executable during the helper's internal readlink call; command-substitution boundaries made the expected read count brittle. Replace it with an explicit snapshot, process replacement, then PID-scoped signal attempt. This directly verifies that revalidation refuses to signal a reused PID without relying on timing.

---

## 2026-11-24 — Keep DNSMasq fixture aligned with SmartDNS process probe split

- The SmartDNS DNS ownership fixture extracts only the read-only Adbyby process helper. After separating that helper from SmartDNS's executable-bound process inventory, the two fixture extraction sites were updated to source the renamed function explicitly; the checker now guards both sites.

---

## 2026-11-24 — Fix SmartDNS proc-root state initialization

- Exact-head DNSMasq Actions exposed a nounset failure after the first valid fake-proc identity: process-state confirmation reused a proc-root variable assigned only inside command substitution. Initialize the proc root in the parent function so absence checks are deterministic in the test and service shell.

---

## 2026-11-24 — Bind SmartDNS lifecycle actions to the exact daemon

- The previous helper trusted `pidof smartdns` and used `killall`, so a process with the same comm name could block startup or be terminated by a stop request. Its process regression was not executed or triggered by DNSMasq CI.
- The helper now confirms `/proc/PID/exe`, the complete SmartDNS argv/config tuple and kernel `start_time`, then revalidates each PID before TERM/KILL. Adbyby retains a separate read-only name-state check. UNKNOWN procfs state blocks signalling and force escalation; Linux 4.4's final validate-to-signal race remains documented.
- Added fake-proc regressions for impostors, wrong argv, incomplete metadata, bounded escalation, unknown post-TERM state and executable replacement. Wired the helper/test paths into push/PR filters and DNSMasq CI. Exact-head Actions pending.

---

## 2026-11-24 — Propagate AliDDNS success-state persistence failures

- Found that arDdnsUpdate() ignored both NVRAM writes after AliDNS had accepted an add/update. A failed record-ID or last-action write could therefore return success and log a complete local update while persistent state remained stale.
- The function now uses the checked NVRAM writer for aliddns_record_id and aliddns_last_act, stops at the first failed write, preserves the original errno/status, and logs that the remote DNS change succeeded but local state could not be saved.
- Extended the API transport fixture with injectable NVRAM write failures for existing-record updates and new-record creation; the source checker pins both checked writes before the final success log. Exact-head Actions pending; XY-C1 L3 and physical-router L4 are not claimed.

---

## 2026-11-24 — Require CAP_NET_ADMIN for MTK ESW switch ioctls

- Found a local privilege-boundary defect: `/dev/mtk_esw` was created world-writable and the kernel ioctl handler accepted switch-state changes without checking caller capabilities.
- Atomic source commit `c28bc0011a34f04e45b27f25891b37824634e15e` changes the node mode to `0600` and requires `CAP_NET_ADMIN` before every ioctl enters the switch lock. It adds a host-testable denial policy and regression; the Switch workflow also checks the kernel call ordering, node mode, and both CI trigger lists.
- Local GNU89 `-Wall -Wextra -Werror` ASan/UBSan policy test PASS. Exact Switch CLI #45410754770, Process lifecycle #45410754891, Process inventory #45410754960, HTTPD #45410754925 and NVRAM #45410754900 all PASS on this SHA.
- AI documentation #45410754763 failed only because the roadmap exceeded its 131,072-byte hard limit (131,220 bytes). This checkpoint shortens the new P1 entry and updates the handoff; exact documentation rerun pending.
- XY-C1 #45410754918 was still building at last poll. Build/TRX/artifact L3 and physical hardware L4 are not yet claimed.

---

## 2026-11-24 — Verify VSFTPD malformed ACL record rejection

- Exact VSFTPD workflow #45408307661 passed on `82962240fea531c282def4176c5a0833ceeae676`, including GNU89 ACL policy regressions and source-order assertions for malformed duplicate and unterminated records.
- AI handoff documentation #45408307604 and Process inventory #45408307674 also passed on the same SHA. NVRAM audit #45408307596 and XY-C1 firmware #45408308038 were still in progress at last poll; the firmware Build was running, with TRX verification and artifact upload pending.
- No L3 claim for the current VSFTPD SHA. The separately verified XUPnP source SHA `eb0fbb1` retains its own Build/TRX/artifact PASS evidence.

---

## 2026-11-24 — Reject unterminated duplicate VSFTPD ACL records

- Follow-up review found the same fail-open pattern at EOF: after a valid permission line, the parser stopped at a final unterminated line before checking whether it repeated the requested `*folder=` key. A truncated duplicate could therefore be silently ignored.
- The parser now evaluates the final byte range even when it lacks a newline, but will never accept a matching permission record without its required terminator. Matching malformed/truncated or duplicate final records now invalidate the result.
- Added regressions for a valid record followed by an overlong unterminated record and by an unterminated exact duplicate. The workflow source contract now pins EOF processing and the final-newline requirement. Exact-head validation is pending.

---

## 2026-11-24 — Reject malformed duplicate VSFTPD share ACL records

- Review of the new exact-record parser found an edge case: it matched only lines of the expected total length. An overlong malformed line with the exact requested `*folder=` key could therefore be ignored, allowing a later valid duplicate record to determine the permission. This was fail-open on malformed ACL input.
- Changed parsing to recognize the exact key first, then reject unexpected record length, invalid permission digits or any duplicate key. Added regressions for malformed-only, malformed-before-valid and valid-before-malformed records; tightened the workflow source contract.
- Baseline `6725aaf`: VSFTPD #45406961857 PASS, AI docs #45406961491 PASS, Process inventory #45406961396 PASS, NVRAM #45406961526 PASS; XY-C1 #45406961610 was PENDING. New exact-head checks are required for this follow-up.

---

## 2026-11-24 — Parse VSFTPD share ACL permissions by exact record

- `get_permission_ftp()` used an unanchored `strstr()` for `*<folder>=`; filesystem share names can legally contain newlines, so a crafted name such as `public=000\n*secret` could span two ACL records and resolve the next share's write/delete rights. This could bypass the share ACL when `st_ftp_mode` is enabled.
- Added a bounded line-record parser that rejects CR/LF in folder names, requires an exact `*name=XYZ\n` record, validates all three permission digits, rejects duplicate records and ignores unterminated trailing text. The existing read-failure and missing-file behavior is unchanged.
- Added GNU89 regression coverage for normal permissions, missing names, LF/CRLF boundary injection, malformed/unterminated records and duplicate entries. Extended the existing VSFTPD workflow source contract to require production use of the exact-record parser and reject the old unanchored search. Exact-SHA Actions pending; this is a source-level ACL fix, not a firmware or router L4 claim.

---

## 2026-11-24 — Align MiniUPnP secure-delete checker with the extended policy-test banner

- Exact MiniUPnP run #45406283079 on `5f0360d` passed the GNU89 ASan/UBSan SOAP policy binary, including IPv4 identity cases, but the dedicated source checker still expected the pre-range-delete test banner. The actual test and integrated workflow both use the expanded `MiniUPnP SOAP unsigned input and range-delete result policy: PASS` banner.
- Updated only the source-check assertion and handoff/log. AI docs #45406283195 and Process inventory #45406283064 PASS on `5f0360d`; NVRAM and XY-C1 were pending at last poll.

---

## 2026-11-24 — Fail closed on MiniUPnP single-entry secure-mode ownership

- Auditing the companion `DeletePortMapping()` path found another authorization bypass: when secure mode was enabled, a failed `upnp_get_redirection_infos()` lookup or an invalid returned IPv4 string silently skipped the ownership comparison and still reached `upnp_delete_redirection()`.
- The single-entry action now fails closed with SOAP 501 when owner identity cannot be read or parsed, returns SOAP 606 for a foreign owner, and uses the same IPv4 peer matcher as the range action. The source contract checks the owner lookup/parse/compare/error gate precedes the actual delete call.
- Restored the expanded SOAP policy PASS banner required by the integrated workflow after #45405754223 on `13ae1bc` exposed the temporary banner mismatch. Added single-action source-order coverage; host ASan/UBSan and exact MiniUPnP/AI checks are pending for this commit.
- XY-C1 #45405466882 for `852eb72` remained in Build (TRX/artifact pending); Process #45405467000 and NVRAM #45405466901 PASS. No L3 claim for the current source SHA.

---

## 2026-11-24 — Repair MiniUPnP secure-mode source-checker quoting

- Exact MiniUPnP run #45405466908 on `852eb72` passed the GNU89 ASan/UBSan SOAP numeric/range policy binary, including the new IPv4 peer-identity cases, but failed before subsequent steps because the Python checker embedded a double-quoted C string using invalid backslash escaping.
- Replaced that assertion with a single-quoted Python literal; no production source or test behavior changed. Exact-head MiniUPnP and AI-handoff workflows must rerun.
- AI docs #45405466936 passed on `852eb72`. Process inventory #45405467000, NVRAM #45405466901 and XY-C1 #45405466882 were pending at last poll; no L3 claim for this MiniUPnP source SHA.

---

## 2026-11-24 — Preflight MiniUPnP range deletes against secure-mode mapping owners

- Found a second authorization defect in `DeletePortMappingRange()`: unlike the single-entry `DeletePortMapping()` action, the range action did not enforce `SECUREMODEMASK` ownership. A LAN peer could request deletion of another internal host's mapping when secure mode was enabled.
- Added a complete read-only preflight of every mapping's internal IPv4 owner before the first delete. Foreign mappings return SOAP 606; lookup or invalid-address failures return 501. No range member is deleted until every owner is verified, avoiding partial deletion when a later entry is unauthorized.
- Added IPv4 identity policy coverage (same, different, null) and extended the source-order checker to require the owner preflight and cleanup gates before the first mutation. Restored the legacy SOAP policy success banner expected by the integrated workflow contract.
- Base `0f7f90f`: MiniUPnP exact run #45405070252 PASS (false-success policy/source checks), AI docs #45405070245 PASS. Exact target firmware Build/TRX/artifact is not yet verified for the current product source; previous XUPnP-only L3 evidence remains isolated to `eb0fbb1`.

---

## 2026-11-24 — Align MiniUPnP SOAP policy workflow marker with the expanded regression

- Exact MiniUPnP run #45404676427 on `e1fb2a714b8bbe225294d8f58d8996c7750658c2` passed the GNU89 ASan/UBSan SOAP input regression and the new DeletePortMappingRange source-order contract. The enclosing inline workflow contract still expected the old banner `MiniUPnP SOAP unsigned input policy: PASS`; update it to the test's expanded `...unsigned input and range-delete result policy...` banner so integrated CI checks the added behavior instead of rejecting a valid regression result.
- Exact AI handoff contract #45404676277 on the same SHA passed after the rolling-log restoration. No product C source changed in this workflow-contract correction.

---

## 2026-11-24 — Correct MiniUPnP range-delete ordering contract and rotate the dev log

- Exact MiniUPnP run #45403990490 on `f12d4d58c6cff4370deb62f3e12f4695bf07a976` compiled and passed the GNU89 ASan/UBSan SOAP policy regression, then failed the Python source contract: its first `free(port_list)` match was the empty-range early-return cleanup, not the release after the deletion loop. The checker now searches for the post-loop free starting after the per-result recording call.
- Exact AI handoff run #45403990489 on the same SHA found 120 root log entries. This commit archives the 100 oldest complete records verbatim in `part-032.md`, retains the newest 20 plus this new record (21 total), and updates the archive index. No product source changed.

---

## 2026-11-24 — Repair MiniUPnP range-delete contract and restore batched history

- MiniUPnP SOAP range-delete policy C test compiled and passed under GNU89 + ASan/UBSan on c2dc6d3. The first exact run #45403376487 stopped at the new Python source checker because a raw-string escape produced an invalid Python quote sequence; this follow-up fixes the checker literal.
- AI handoff run #45403376294 exposed that a previous dev-log update had fetched only a short line range and replaced the 119-entry archive with four entries. Restored the complete log from the preceding full-history commit and added this current validation record; the documentation contract requires at least 20 complete newest entries.
- Process lifecycle checker correction commit 364383be7889a9339887c1b4f04ce9ff9a273a10 passed exact Process lifecycle run #45402861054. The follow-on Process inventory run #45403376647 on c2dc6d3 also passed.
- XUPnP pair product-source commit eb0fbb1f3d858876e754efc2815c3880b74f764a has exact XY-C1 workflow #45401700209 PASS for firmware build, TRX verification and artifact upload. L3 is complete for that source slice only; physical-router L4 remains unverified.

---

## 2026-11-24 — Align lifecycle integration checks with staged XUPnP pair

- Exact Process lifecycle run #45401700271 on `eb0fbb1f3d858876e754efc2815c3880b74f764a` passed the XUPnPD storage-directory and file-safety C regression plus multiple other daemon regressions, then failed only because `check_process_lifecycle_integration.py` still required two calls using `tmp2/tmp1` and sequential `rename(tmp1,tmp2)`. The product code now stages `common_path/common_stage` and `runtime_path/runtime_stage`, then calls `dnsmasq_file_publish_pair()`.
- Updated the lifecycle checker to require the two distinct staged paths, both source inputs and the rollback-capable publisher, and to reject the old sequential-rename contract. This is a validator-only fix.
- The separate DNSMasq lifecycle checker was also fixed in `2fd4c74` after #45401700240 exposed a raw-versus-compacted offset comparison bug. Both exact-head workflows must rerun; no new product source change in these checker corrections.

---

## 2026-11-24 — Fix XUPnP source-checker offset comparison

- Exact DNSMasq lifecycle run #45401700240 on source SHA `eb0fbb1f3d858876e754efc2815c3880b74f764a` passed the GNU89 ASan/UBSan DNSMasq file regression and failed the XUPnP source-order checker. The checker compared character offsets from un-compacted `body` text to offsets from whitespace-compacted `compact` text; it therefore rejected correctly ordered fsync/NVRAM operations.
- Corrected all four relative-order comparisons to use the same compact source string. No production C or test code changed. Exact-head DNSMasq and documentation checks must rerun on this correction SHA.

---

## 2026-11-24 — Publish XUPnP Lua configuration as a rollback-capable pair

- `start_xupnpd()` previously renamed staged `common.lua` live before preparing `xupnpd.lua`. A later source/NVRAM/write/fsync failure could leave a mixed config generation while startup returned an error.
- Both files are now fully read, rendered, flushed and fsynced before either destination changes. `dnsmasq_file_publish_pair()` snapshots both old paths and restores the first if the second rename fails. A preserve-mode sentinel retains each tempfile's preselected destination mode.
- Added GNU89 ASan/UBSan regression for distinct mode retention and forced second-rename rollback, plus a Python source-order contract. DNSMasq lifecycle now watches `net.c` and runs the new contract after the pair regression.
- Parent `aae46a6`: udhcpc regression/source contract, AI handoff and Process lifecycle PASS; USB storage and NVRAM audit pending at last poll. XY-C1 for production `4015869` still needed Build/TRX/artifact confirmation.

---

## 2026-11-24 — Fix stale DHCPDECLINE regression invocation

- Exact SHA `4015869` udhcpc workflow #45400399177 failed GNU89 compilation: the regression was renamed to `test_failed_packet_send_blocks_dhcp_reselection_until_retry()`, but `main()` still invoked the removed `test_failed_dhcpdecline_blocks_reselection_until_retry()` name. This also triggered an unused-function error for the new definition.
- Updated the test runner to invoke the existing renamed function. Production DHCP code and source-contract assertions were already aligned; this is a regression-harness compile fix.
- On parent `4015869`, AI handoff #45400399146 and Process lifecycle #45400399189 PASS. USB storage, NVRAM audit, XY-C1 and Process inventory were pending at the last poll.

---

## 2026-11-24 — Honor sendto byte-count semantics in DHCP retries

- Review of BusyBox packet.c confirmed udhcp_send_raw_packet() and udhcp_send_kernel_packet() return sendto()/safe_write() byte counts on success and -1 on failure. The new DHCPDECLINE retry predicate initially treated every nonzero result as an error, which would have retried successful transmissions forever. The recent perform_release() diagnostic similarly mislabeled successful DHCPRELEASE sends as failures.
- Replaced the predicate with udhcpc_packet_send_failed(status < 0), using it for both DECLINE retries and RELEASE logging. Regression now covers positive byte-count success, zero success and negative failure; source checker pins the release diagnostic to the same convention. This is a correctness repair to the DHCPDECLINE slice, not merely a wording change.
- Exact SHA 4ca919b's first udhcpc run passed GNU89/ASan/UBSan but failed the Python contract on its outdated literal INIT_SELECTING assertion. AI docs also flagged the handoff checkpoint wording; both validator fixes were committed in 02ddda7 and have exact-head reruns queued at last poll.

---
## 2026-11-24 — Repair exact-head DHCPDECLINE and handoff validator failures

- Exact push SHA 4ca919b compiled and passed the GNU89 ASan/UBSan udhcpc policy binary, but its Python source contract retained one old assertion requiring a literal INIT_SELECTING argument after the ARP-conflict retry path became a status-dependent resume_state. Updated the contract to check the new resume_state call.
- Exact AI docs workflow also caught that SESSION_HANDOFF.md used equivalent but non-canonical checkpoint wording and omitted the required exact phrase “one atomic main commit.” Restored those mandatory handoff contract strings. Both failures were validator-only; DHCPDECLINE product code remains unchanged in this correction.

---
## 2026-11-24 — Retry failed DHCPDECLINE before requesting another address

- ARP conflict handling ignored the return value from send_decline(). It immediately restarted DHCP selection even when DHCPDECLINE could not be sent, so the server could offer the same conflicting address again; the source violated the required decline-before-restart sequence.
- Added an ARPING-only DECLINE_PENDING state retaining the original server and address. A failed send blocks discovery and retries after a bounded delay. If old-lease deconfiguration also fails, DECONFIG_PENDING resumes into DECLINE_PENDING only after cleanup succeeds. Successful send then returns to INIT_SELECTING. GNU89 policy regression covers send status and retry-delay floor; the Python source contract pins saved tuple, cleanup ordering and retry-before-rediscovery.
- Parent 94960ce: exact BusyBox udhcpc regression/source contract #45398493359 PASS; AI docs #45398493347 was pending at last check. Product-source XY-C1 #45398034078 is building on SHA 694732a; TRX/artifact pending. QMI firmware L3 is confirmed on b05eae9: #45396493118 Build, TRX verification and artifact upload all SUCCESS.

---
## 2026-11-24 — Correct udhcpc pending-release source contract

- Exact SHA 694732a passed the GNU89 ASan/UBSan state-policy binary, but the Python checker failed one stale assertion: it still expected the SIGUSR2 branch to assign deconfig_resume_state = RELEASED directly. The production code now correctly routes that path through perform_release() and udhcpc_schedule_deconfig_retry(RELEASED, 0, ...), so the source contract was updated to pin the shared routine and retry-before-continue order rather than the obsolete assignment.
- This is a checker-only correction; no product source or test semantics changed. Exact-head rerun pending. Parent XY-C1 #45398034078 is still building with TRX/artifact pending; Process inventory #45398034058, umbrella Process #45398034079, USB storage #45398034361 and NVRAM #45398034033 were in progress at last poll; AI docs #45398034008 PASS.

---
## 2026-11-24 — Unify pending DHCP release and -R exit handling

- Follow-up state-path review found that the initial SIGUSR2 pending-state repair bypassed `perform_release()`; a daemon exiting through `-R` while the same deconfig was pending could still skip DHCPRELEASE. The release routine now recognizes the saved “lease still active” bit while `DECONFIG_PENDING`, and the signal handler reuses that routine instead of duplicating packet logic.
- Normal BOUND/RENEWING/REBINDING/RENEW_REQUESTED release behavior is preserved. Pending NAK/expiry/ARP cleanup and an already-sent release do not emit duplicates; pending release send errors are now logged. Source contracts pin the shared SIGUSR2/`-R` path.
- Parent `afba6d0005263387d20539f6719c1befbe8b223b`: udhcpc GNU89 ASan/UBSan and Python source contract #45397570960 PASS; AI docs #45397570943 PASS. XY-C1 #45397571031 still pending at cache/build preparation; Process inventory #45397571017 PASS; USB storage #45397570936 and NVRAM #45397571009 in progress. L3 remains pending.

---

## 2026-11-24 — Honor DHCPRELEASE during pending manual-renew cleanup

- Found a state-machine gap in BusyBox udhcpc: a repeated SIGUSR1 while RENEW_REQUESTED can fail its deconfig callback and enter DECONFIG_PENDING with the prior lease still active. A subsequent SIGUSR2 only changed the resume state to RELEASED; it skipped the DHCPRELEASE packet that the normal perform_release() path sends.
- Carry a narrowly scoped “prior lease still active” bit only from that manual-renew failure path. SIGUSR2 sends DHCPRELEASE before resuming pending cleanup; NAK, lease expiry, ARP conflict and already-sent release paths do not send a duplicate. Clear the bit on resume. Added GNU89 policy tests and source-order/call-site checks.
- QMI teardown fix on exact SHA `b05eae9`: USB modem #45396493190, Process inventory #45396493113/#45396492751, AI docs #45396493061, NVRAM #45396492703, WAN DHCP #45396492710, WAN teardown #45396492829, detect-link #45396493228 and HTTPD #45396492941 PASS. XY-C1 #45396493118 remains in Build; TRX/artifact pending. No L3 claim; physical-router L4 unverified.

---

## 2026-11-24 — Preserve QMI WDS ownership when PDH cleanup fails

- After `--stop-network` succeeded, failure to unlink `QMI_HANDLE_PDH` only set an error flag; teardown still released the WDS client ID and removed its marker. The stale PDH then made future starts fail with `EBUSY`, while the released CID was no longer reusable.
- Gate CID release on successful or already-absent PDH cleanup. Any other unlink failure now returns immediately, preserving the WDS ID and marker for recovery. Added GNU89 policy regression and a source-order contract; the existing USB modem workflow already includes the source/test/checker paths.
- Parent `0dbf3d2f49133f6cbd471d7b6ba4efe633e5bb96` adds the checked Ralink regulatory-SKU read. Its AI docs, Process inventory and wireless-radio-state workflows PASS; NVRAM audit and XY-C1 remain in progress (XY build/TRX/artifact not yet completed). New QMI exact-head Actions are pending; no L3 claim. Physical modem L4 remains unverified.

---

## 2026-11-24 — Preserve Ralink regulatory SKU selection errors

- `symlink_sku_file()` used `nvram_safe_get("regspec_code")`, masking a missing key and an NVRAM backend failure as the same empty value. A transient read error could silently select a different regulatory SKU.
- Switched to `nvram_get_checked()` and explicit status validation: missing optional key still falls through, present values remain table-validated, and backend/API errors abort config generation. Added GNU89 regression coverage for present/missing/error/invalid API states and a workflow source-order contract.
- Parent `5a7b2c4`: WAN route regression, AI docs, Process inventory and NVRAM audit PASS; XY-C1 Build remains in progress with TRX/artifact pending.

---

## 2026-11-24 — Reject whitespace and signs in the SYN-cookie selector

- Found that `strtol()` accepts leading whitespace and optional signs, so malformed `fw_syn_cook` values such as `" 0"` and `"-0"` could be mistaken for an explicit disable.
- Added a first-byte decimal-digit guard before `strtol()`; complete-token, boolean-domain and overflow checks remain intact. Regression adds space/tab/newline and +/- cases; the source checker verifies validation occurs before parsing.
- Parent `aa3d55b`: VSFTPD #45393626354, AI #45393626335, Process #45393626312 and NVRAM #45393626295 PASS. XY-C1 #45393626376 Build in progress, TRX/artifact pending. No L3 claim for recent VSFTPD source fixes yet.

---

## 2026-11-24 — Close stale FTP data endpoints on EPSV ALL

- `handle_pasv()` accepted `EPSV ALL` by setting `epsv_all` and returning before active/passive cleanup. Existing `PORT`/`EPRT` destination or a `PASV` listener survived; later transfer code could consume that endpoint although new non-EPSV setup commands were blocked.
- The branch now calls `pasv_cleanup()` and `port_cleanup()` before setting EPSV-only state or sending success. Passive cleanup is idempotent in the privilege operation; active cleanup clears the stored sockaddr.
- Extended the VSFTPD stability source contract to require each cleanup once and before the state flag, 229 reply and return. Both VSFTPD and XY-C1 track `postlogin.c`; unknown production paths trigger full firmware validation.
- Parent `5a7500e`: VSFTPD #45392889702, AI #45392889910 and Process #45392889923 PASS; NVRAM #45392889664 pending and XY-C1 #45392889756 building. Exact-head verification required; physical L4 unverified.

---

## 2026-11-24 — Fail closed on truncated VSFTPD hidden-share marker paths

- Found `test_if_hidden_share()` building `<mount>/.hidden` in a 256-byte static array without checking truncation; a valid long mount component could make `stat()` miss the marker and expose a hidden share.
- Reused `ftp_acl_path_build_child_path()`, expanded the marker buffer to `PATH_MAX`, and fail closed on construction errors. Added normal, 255-byte component, undersized-output, full-path and invalid-leaf regression cases; the VSFTPD workflow checks production ordering.
- Concurrent checkpoint on parent `18af4fb`: detect-link, AI docs, Process, USB modem, HTTPD auth and NVRAM PASS; XY-C1 Build is still running with TRX/artifact pending. No firmware L3 claim.

---

## 2026-11-24 — Preserve UNKNOWN WAN link state until a valid PHY sample

- `detect_link.c` initialized `dl_status_wan=0` and `dl_status_wan_nvram_old=-1`. A failed first switch PHY query correctly left the cached value unchanged and scheduled retry, but the same timer tick still called `dl_handle_link_wan()`, which published the synthetic zero to `link_wan` before any successful hardware sample.
- Added a tri-state known flag updated only by nonnegative PHY results. The WAN transition/NVRAM handler is deferred until that first valid sample; after it becomes known, subsequent query errors continue retaining the last valid link state while retrying. Independent WISP/LAN/USB handling remains active.
- Added GNU89/`-Werror`/ASan/UBSan policy coverage for unknown retention on negative results, valid link-down/up initialization and preserving known state through later read errors. The detect-link workflow adds an exact source-order/handler-gating contract.
- Prior WAN source PHY selector production SHA `d3733ec` is L3 verified: XY-C1 #45390205170 completed Build/TRX/artifact PASS; host and documentation corrections on test/docs-only descendant `c954e19` passed detect-link #45390666226, AI #45390666278, Process #45390666391 and NVRAM #45390666354. This new product-source change needs fresh exact-head firmware validation; physical L4 remains unverified.

---

## 2026-11-24 — Correct WAN selector test contract and handoff size

- Exact SHA `d3733ecbec2e0cf652c00eadf1db4645ae5ff13e`: detect-link run #45390205127 compiled and passed the new GNU89 ASan/UBSan selector regression, but the final Python contract expected direct test calls. This repository's test main wraps each case in `CHECK(test() == 0)`; THIS FOLLOW-UP matches that convention and fixes the test diagnostic newline.
- AI docs run #45390205125 caught `REFACTOR_ROADMAP.md` at 131111 bytes, 39 bytes above the 131072-byte hard limit. THIS FOLLOW-UP compresses the new priority entries and verifies the encoded byte count below the ceiling.
- XY-C1 #45390205170 was still building at last poll (TRX/artifact pending). Exact-head reruns are required; no L3 claim is made for the WAN selector yet.

---

## 2026-11-24 — Reject unsupported persisted WAN source PHY selectors

- `get_wan_ether_link_direct()` used a checked NVRAM read for `wan_src_phy` but passed all nonnegative integers to the switch UAPI. In the MTK kernel, `get_port_from_uapi()` returns a sentinel for unsupported IDs and `esw_status_link_port_uapi()` translates that to success with link=0. A corrupt selector could therefore be mistaken for an actual Ethernet link-down, allowing detect_link to publish a false WAN disconnect and run teardown logic.
- Added a range policy for the generic external switch UAPI: WAN=0 and LAN1..LAN5=1..5 are accepted; legacy negative values still fall back to WAN=0; unknown/CPU port IDs above 5 fail with `ERANGE`. The userland caller rejects before issuing the ioctl and returns `-1` with errno, preserving the last known link state and retry path.
- Added GNU89/`-Werror`/ASan/UBSan tests for 0..5, negative compatibility including INT_MIN, and invalid/CPU IDs 6/13/14/15/INT_MAX with no output mutation. The detect-link workflow checks policy ordering and path coverage; XY-C1 path filters include the production caller, helper and test.
- Parent exact SHA `659b91b`: HTTPD lifecycle #45388114470, AI docs #45388114468, Process lifecycle #45388114507, and XY-C1 #45388114515 all PASS, including Build/TRX/artifact. New exact-head CI results must be checked on this commit. Physical-router L4 remains unverified.

---

## 2026-11-24 — Correct HTTPD selector contract and ensure XY-C1 build coverage

- On exact SHA `86d3078c1ead2dfd5578830e069b684e12a4b072`, HTTPD run #45387713984 compiled and passed the GNU89 ASan/UBSan policy regression, then failed its Python source contract. The new assertion used escaped backslash-n/tab text rather than the C source's actual whitespace; production code was not implicated.
- The same SHA had no XY-C1 push run because `.github/workflows/xy-c1-firmware-validation.yml` did not watch `trunk/user/rc/src/httpd_daemon_policy.h`, despite that header being included by the firmware's `services.c`.
- THIS FOLLOW-UP normalizes whitespace before the source assertion and adds the header to both XY-C1 push and pull-request path filters. Exact-head HTTPD, XY-C1 Build/TRX/artifact and AI/Process/NVRAM results must be checked on THIS COMMIT.

---

