## 2026-11-15 — Repair DNSMasq source-contract variable in cleanup workflow

- The new rollback-snapshot cleanup source and its ASan/UBSan test compiled and passed all preceding steps on exact commit `d2784f00b3ffd5e1988a5dbf780f12061e4b93a4`.
- DNSMasq Action #43722144765 then failed at the final source-contract stage with Python `NameError: name 'wan' is not defined`. The workflow loads the file as `net_wan`; the new assertion referenced the wrong variable. Corrected the assertion to use `net_wan`.
- AI docs #43722144758 and WAN route #43722144902 passed; XY-C1 #43722144832 and NVRAM #43722144898 were still in progress at last poll. Rerun DNSMasq source contract and retain exact-SHA evidence.
- This is a workflow assertion repair, not a C-source/test failure. No current-source MIPS/TRX or physical-router result is claimed.

---

## 2026-11-15 — Make DNSMasq rollback-snapshot cleanup observable

- Found a related cleanup hole: `dnsmasq_file_backup_discard()` returned void. It preserved a path on unlink failure but exposed no status for callers, so leftover hard-link recovery snapshots could remain unnoticed.
- It now returns the retryable discard result and shares the injectable unlink implementation. Tests cover backup snapshot EACCES retention and successful retry.
- Three-output DHCP publication, router hostname/hosts publication and LAN hostname/hosts publication now retry residual backup cleanup while path buffers are alive, logging any persistent leftovers. Failure paths retain and log snapshot names for recovery rather than deleting them blindly.
- Parent main: `a3291427a15d8c7d55a38ebe20404e3a5af541ce`. Staging-discard cleanup exact DNSMasq #43721283505 and AI docs #43721283501 PASS; WAN route #43721283467, process inventory #43721283468 and NVRAM #43721283671 PASS. XY-C1 #43721283605 remained in progress; fresh exact-head validation required.
- No current-source target MIPS/TRX build or physical-router result is claimed; L4 remains NOT VERIFIED.
- Changed paths: `dnsmasq_file.h`, `tests/dnsmasq_file.c`, `services_ex.c`, `net_wan.c`, `net_lan.c`, DNSMasq workflow, SESSION_HANDOFF, REFACTOR_ROADMAP and dev_log.

---

## 2026-11-15 — Preserve DNSMasq staging paths when cleanup fails

- Found a resource-management defect: `dnsmasq_file_discard()` unconditionally zeroed its caller's pathname even when `unlink()` failed, leaving a private staging file orphaned with no retry handle and no cleanup error reported.
- Replaced it with a status-returning helper: successful unlink and ENOENT clear the path; other errors preserve the exact path and errno. Added an injectable unlink callback for deterministic regression coverage.
- Updated resolver, hostname/LAN, DNSMasq service-config and static-host cleanup paths to check and log leftover staging paths. The DNSMasq workflow now contracts the helper/test and path list; XY-C1 watches the DNSMasq file regression too.
- Parent main: `39a67af8d061f6f3eceeb2efbea7bd9a4c7cfe10`. Resolver fix exact-head DNSMasq #43719693642, WAN route #43719693710, AI docs #43719693546, process inventory #43719693606 and NVRAM #43719693596 passed; XY-C1 #43719693623 was IN_PROGRESS. Fresh exact-head Actions are required for this cleanup commit.
- No current-source target MIPS/TRX build or physical-router result is claimed; L4 remains NOT VERIFIED.
- Changed paths: `dnsmasq_file.h`, `tests/dnsmasq_file.c`, `services_ex.c`, `net_wan.c`, `net_lan.c`, DNSMasq/XY-C1 workflows, SESSION_HANDOFF, REFACTOR_ROADMAP and dev_log.

---

## 2026-11-15 21:35 CST — Propagate blocked DNSMasq upstream refresh errors

- Found a partial-success defect in `update_resolvconf()`: when `fill_dnsmasq_servers()` failed and the previous `DNS_SERVERS_FILE` was missing or unsafe, the code correctly suppressed DNSMasq cache notification but still returned success after publishing `/etc/resolv.conf`. WAN/PPP callers and PID 1 could acknowledge an incomplete refresh.
- Preserve the validation errno and propagate it after resolver publication and resolver-lock release, before any DNSMasq cache notification. A safe last-known-good upstream snapshot remains a valid fallback.
- Extended the GNU89 resolver policy regression for safe/unsafe/missing-errno states; added a production source-order contract and symmetric WAN push/PR filters; added the policy header to XY-C1 build triggers.
- Parent main: `fedd187503974f72651ea960b4d87f5a558c7c2f`. DNSMasq #43719236286, WAN route #43719236311, AI docs #43719236448 and process inventory #43719236422 passed on preceding product source `3fa8d08b968de653a7998175bdab897e51b91379`; XY-C1 #43719236268 remained in progress. Exact-head Actions for this resolver fix are required. No current-source MIPS build or physical-router result is claimed.
- Isolated GNU89 policy regression passed with ASan/UBSan; this is host-only evidence, not target MIPS validation.
- Changed paths: `net_wan.c`, `wan_resolver_refresh_policy.h`, resolver test, WAN and XY-C1 workflows, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.

---

## 2026-11-15 — Correct static-ARP transaction test literal after CI failure

- DNSMasq lifecycle #43718997744 on `d5660e3cdfa9804a23335656676c5f598c19a37e` initially failed when compiling the expanded static-host transaction regression: its mock compared `mac[0]` against a multi-character constant produced by over-escaping the backslash.
- Corrected the test-only literal to C `'\\0'` in `23cc4e9e509f13a4ea7636c067d6cfd869fde81b`. Exact-head DNSMasq #43719124911 then passed, including the GNU89 static-host regression; AI handoff #43719124917 and process inventory #43719124976 also passed.
- XY-C1 #43718997699 for the unchanged product source `d5660e3` was still in progress at checkpoint; L3 is unverified until build, TRX and artifact upload all pass. No local firmware or physical-router result is claimed.

---

## 2026-11-15 — Roll back prior static ARP additions after setup failure

- `fill_static_ethers()` previously stopped on a later failed `arp -s` without removing earlier static ARP entries installed during the same setup pass, leaving a partial kernel ARP table while DNSMasq refused to start.
- Added an injectable transaction policy that prevalidates all IP/MAC pairs, applies entries in order, and on failure deletes every previously successful add in reverse order. Rollback continues after an individual delete error and returns the rollback error (otherwise the original add error).
- The failed target is not deleted: in-tree BusyBox `arp_set()` returns success only after `xioctl(SIOCSARP)` succeeds; deleting a failed target could destroy a pre-existing neighbour mapping. The DNSMasq workflow asserts this applet contract.
- GNU89 regressions cover successful installation, reverse rollback, continued rollback after delete failure and rejection of an invalid set before side effects. The workflow checks that the production callback runs after all generated files have published.
- Parent main: `fef0bc61085a5e9364e972c00c97b5b0e3a8c59d`; exact-head DNSMasq/XY-C1 validation for this source change is pending. No local firmware build or physical-router L4 result is claimed.

---

## 2026-11-15 11:12 UTC — Reject multicast and all-zero static DHCP MACs

- Tightened dnsmasq_static_mac_is_valid(): 12 hex characters are necessary but insufficient. A DHCP hardware address must also be nonzero and unicast (I/G bit clear); multicast and 00:00:00:00:00:00 are now rejected.
- Updated GNU89 regression: accepts ordinary and locally administered unicast MACs; rejects zero, multicast, wrong-length and injection-character forms. DNSMasq source contract now requires the new negative test case.
- Baseline exact-head b39d7749: DNSMasq #43706419314, HTTPD #43706419305, AI docs #43706419277, Process inventory #43706419288 and NVRAM #43706419285 PASS. Earlier hostname-pair rollback test failed on bc91ec1 due to a stale test path; b39d7749 corrected the fixture and all these Actions passed.
- Parent main b39d77496885c65b1725dfc904788ec41f69f8fb. Changed: dnsmasq_static_host_policy.h, tests/dnsmasq_static_host_policy.c, DNSMasq source contract, SESSION_HANDOFF.md, REFACTOR_ROADMAP.md and dev_log.md. No local compile/hardware result claimed.

---

## 2026-11-15 11:02 UTC — Record IPv6 stop failures and repair DNSMasq build triggers

- full_restart_ipv6() now captures and logs nonzero stop_dns_dhcpd() status before the existing fresh tri-state process inventory and locked start preflight.
- Added dnsmasq_file.h and tests/dnsmasq_file.c to DNSMasq push/PR path filters and explicit workflow path contract. Added dnsmasq_file.h and dnsmasq_domain_policy.h to XY-C1 push triggers for header-only product changes.
- Parent exact-head evidence 34ec88e1: DNSMasq #43705925858, HTTPD #43705926049, AI handoff #43705925896 and process inventory #43705926093 PASS. XY-C1 #43705925871 and NVRAM #43705925906 remain in progress. This commit's Actions remain pending.
- Parent main: 34ec88e12393d8c8645ad25674ccf44aacbe1256. Changed paths: net6.c; DNSMasq and XY-C1 workflows; SESSION_HANDOFF.md; REFACTOR_ROADMAP.md; dev_log.md. Frozen backup unchanged. No local or hardware test claimed.

---

## 2026-11-15 — Stage HTTPD restore uploads before replacing retained retries

- Fixed the restore upload transaction: `do_restore_st_post()` no longer unlinks or writes directly to `STORAGE_FIFO_FILENAME`. It authenticates the request, validates the multipart boundary and creates a unique mode-0600 stage in the same directory.
- The multipart parser now accepts a pre-opened stream and publish target. For storage restores, the old archive is replaced only after the complete payload/boundary validate, followed by checked fflush, fsync and fclose, then atomic rename. Parse/write/flush/fsync/close/rename failures remove only the new stage.
- Added a one-request readiness latch, reset at POST entry and consumed by the CGI output callback. A failed/malformed new upload cannot cause the callback to apply an older retained archive. A failed MTD restore keeps the new archive; a successful restore removes it.
- Extended `httpd_upload_file.c` with staging-open, flush, fsync, close and rename fault injection; asserts old archive bytes remain unchanged and the stage is removed. Successful replacement checks mode 0600 and confirms rename replaces a symlink entry rather than following its victim.
- Local helper test: `cc -std=gnu89 -Wall -Wextra -Werror -I <httpd-src> tests/httpd_upload_file.c` and execute — PASS in scratch workspace. Embedded HTTPD source contracts now check trusted auth, stage/publish ordering, stable retry retention, callback readiness and HTTPD input/output request order.
- Parent main SHA: `de14e2ec6c3365fd1e9be4cce5c13cb37ace60bc`. Parent HTTPD #43697297903 and current-baseline XY-C1 #43697297850 passed on source `8c95ef5ad8a2d4a2ac7755fbe02b7ac2211214b8`; both predate this HTTPD product change. Exact-head Actions for THIS COMMIT pending. No L4 hardware result claimed.
- Changed paths: `upload.c`, `httpd_upload_file.h`, `tests/httpd_upload_file.c`, `web_ex.c`, `httpd.h`, `.github/workflows/httpd-security.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.

---
## 2026-11-15 — Keep the MTD save lock until blank-partition hash cleanup succeeds

- Auditing both zero-byte branches in `func_load()` found a persistence hazard: stale MD5 removal failures were ignored and the one-shot save lock could still be removed. A later identical save could match the stale checkpoint and skip writing an erased partition.
- Both branches now check `rm -f "$hsh"` before attempting `rm -f "$slk"`. If hash deletion fails, the old settings tree, checkpoint and lock remain intact, an error is logged, and load returns failure.
- Added failure injection for hash-unlink errors after (1) nonzero decompressor status with zero output and (2) nominally successful empty output. The fixture asserts the previous configuration/hash/lock survive; existing storage Actions run it under host shell and BusyBox 1.24.2 ash. The source contract requires checked hash-before-lock ordering and both injection cases.
- Parent USB storage #43696174488 on `bb2b87054ef9631fc97c0473dd47d7e8b4fccd87` passed the complete storage workflow after earlier checker fixes. Parent AI docs #43696174518 passed. XY-C1 #43694958186 succeeded on source SHA `99eeb6fa237b529ec9b9615976961c7a60a02575`, but predates this `mtd_storage.sh` modification; a fresh build/TRX/upload is required.
- Changed paths: `mtd_storage.sh`, `mtd_storage_load_transaction.sh`, `check_storage_safe_remove.py`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`. Parent main: `bb2b87054ef9631fc97c0473dd47d7e8b4fccd87`; result: THIS COMMIT. Exact-head USB storage, AI docs and XY-C1 runs required. No local test/build/hardware result is claimed; L4 remains NOT VERIFIED.

---

## 2026-11-15 17:05 CST — Parse MTD shell function contracts without the C brace parser

- Exact-head USB storage #43695808627 on `c2ee97b727e1fa16c86a93969129bf3c1c54f49f` passed syntax, MTD save retry, restore, host + BusyBox ash load/backup/erase-reset fixtures, GNU89 policy tests and the full storage caller graph. The final checker failed at `body(mtd_storage, "func_erase()")`: this helper parses C comments/strings and interpreted the apostrophe in a shell comment as a character quote, so it never recognized the closing function brace.
- Updated only the source checker to isolate shell `func_erase()` and `func_reset()` by their neighboring function declarations. C function checks continue to use the C-aware parser. No production behavior or regression fixture changed.
- Exact-head AI docs #43695808690, NVRAM #43695808608, infosvr #43695808631, process lifecycle #43695808654, AdGuardHome #43695808676, AliyunDrive #43695808670, AliDDNS #43695808625 succeeded on the same SHA. USB storage #43695808627 is the only listed failure; current-source XY-C1 result remains to be checked.
- Changed paths: `scripts/check_storage_safe_remove.py`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`. Parent: `c2ee97b727e1fa16c86a93969129bf3c1c54f49f`; resulting SHA: THIS COMMIT. Exact-head storage/docs rerun required. L3/L4 remain NOT VERIFIED.
- No local firmware build or router test is claimed.

---

## 2026-11-15 16:22 CST — Align empty-MTD hash assertion with combined cleanup

- Exact-head USB storage run #43694958246 on `99eeb6fa237b529ec9b9615976961c7a60a02575` passed production shell syntax, MTD save retry, transactional restore, host + BusyBox ash load/backup/erase-reset fixtures, GNU89 policy tests and the full caller graph. The final checker failed because it required a standalone `rm -f "$hsh"` in the successful empty-decompression branch, while source correctly issues `rm -f "$tmp" "$hsh"` and then checks one-shot lock removal.
- Changed the assertion to require the hash path token plus checked lock cleanup, matching the actual combined cleanup without weakening the invariant. No production source changed.
- AI handoff docs #43694958340, Process inventory/lifecycle #43694958225, infosvr #43694958242, AdGuardHome #43694958168, AliDDNS #43694958275, AliyunDrive #43694958193, process inventory #43694958234, NVRAM #43694958278 and HTTPD security #43694958209 succeeded on the same exact parent. XY-C1 #43694958186 was still in progress.
- Changed paths: `scripts/check_storage_safe_remove.py`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`. Parent: `99eeb6fa237b529ec9b9615976961c7a60a02575`; result: THIS COMMIT. Fresh exact-head storage/docs checks required; current-source L3/L4 are not yet verified.

---

## 2026-11-15 16:05 CST — Make MTD erase and reset fail closed

- `func_erase()` now establishes the persistent save lock before the irreversible MTD erase, checks the flash command and every hash/directory operation, and keeps the lock on failed or uncertain outcomes. A successful physical erase is not claimed to be reversible.
- `func_reset()` now locks persistence before deleting the stale hash and old settings directory, checks removal/recreation, and keeps the lock on partial failure. The dispatcher fills defaults before releasing the marker and does not start services after an incomplete preparation.
- Added deterministic regression coverage for flash failure, lock creation, hash removal, storage removal/recreation, default-fill and lock-release failures. USB storage Actions now run it under the host shell and BusyBox 1.24.2 ash. Corrected the exact-head MTD-load source contract from two to three checked lock-removal paths; the previous exact-head run failed only at that assertion after all preceding steps passed.
- Changed paths: `mtd_storage.sh`, `mtd_storage_erase_reset.sh`, `check_storage_safe_remove.py`, storage-hotplug workflow, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`. Parent SHA: `1eb57714cd9cbaaa71a4074615eaf472f053f359`; result: THIS COMMIT. Exact-head storage, AI docs and XY-C1 Actions required.
- No local checkout/test, current-source MIPS firmware build or hardware result is claimed. L4 remains NOT VERIFIED.


---

## 2026-11-15 15:44 CST — Repair MTD load source contract and restore full log window

- Exact-head USB storage #43691365991 on 361bf44c724e64f3826b876d43a876bb65fc6039 passed MTD syntax, save-retry, restore, host + BusyBox ash load/backup fixtures, GNU89 persistence/inventory/lock tests and the full caller graph. It failed only in scripts/check_storage_safe_remove.py line 502: the order assertion used index() and selected the first save-lock removal token in the blank-flash error path, rather than the final successful-load cleanup. The checker now explicitly requires both checked lock-removal paths and orders the final one with rindex().
- Exact-head AI handoff documentation run #43691366036 failed because the preceding commit unintentionally replaced dev_log.md with one entry, dropping 110 previously retained records. Restored all 110 records verbatim from parent 2feac289de28bca92469c304a0af13d90c1fb5e9 and retained the new 361bf44 entry; the root now contains 111 complete records, below the 120-record rotation threshold. No archive shard or index was changed.
- Changed paths: scripts/check_storage_safe_remove.py, dev_log.md, SESSION_HANDOFF.md and REFACTOR_ROADMAP.md. Parent SHA: 361bf44c724e64f3826b876d43a876bb65fc6039; resulting SHA: THIS COMMIT.
- New exact-head storage and AI docs Actions are pending. No firmware source changed; no local test, target build or hardware result is claimed.

---

## 2026-11-15 — Correct MTD load regression newline and exit-status handling

- Exact parent `2feac289de28bca92469c304a0af13d90c1fb5e9` USB storage #43690684317 passed shell syntax, MTD save retry and restore transaction, then exited nonzero in the combined MTD load host/BusyBox ash test. HTTPD security #43690684263, process lifecycle #43690684217, process fail-closed #43690684307, infosvr #43690684231, AI docs #43690684277, AdGuardHome #43690684211, AliyunDrive #43690684251 and AliDDNS #43690684338 passed; NVRAM #43690684273 and XY-C1 #43690684216 were still in progress.
- Two previous-checkpoint writes used a doubled backslash printf format, producing literal backslash-n rather than a newline. The fixture now writes a proper newline.
- The old cleanup trap could mask a nonzero assertion result because its final `rm -rf` succeeded. The EXIT trap now preserves the original status, and HUP/INT/TERM exit nonzero.
- Test/handoff/roadmap only; no product source behavior changed. Exact-head storage rerun required. No local test is claimed. Parent SHA: `2feac289de28bca92469c304a0af13d90c1fb5e9`.

---

## 2026-11-15 — Release the post-erase MTD save lock only after confirming blank flash

- Exact-head parent `546ca8e66a00aee08c53922a55368e61ceb507e2`: HTTPD security #43690054071 SUCCESS after the prior commit retained uploaded storage restore files on failure and fixed the escaped-newline source-boundary checks. USB storage #43690054009 FAILED only at the final trigger contract because `mtd_storage_load_transaction.sh` and `mtd_storage_backup_failure.sh` were not present in pull_request filters.
- `func_erase()` deliberately touches `$slk` after successful flash erase to prevent runtime from immediately repopulating the partition. On the next boot, blank flash causes bzcat to fail with a zero-byte output. The transactional loader previously returned early and left `$slk`, permanently disabling future saves. It now clears MD5 and the lock only when file size is exactly zero; nonempty partial/corrupt output preserves the previous checkpoint and lock.
- Added host/BusyBox ash cases for zero-byte failure, zero-byte success and nonempty decompressor failure; retained existing extraction/publication rollback cases. Added both MTD regression paths to storage-hotplug pull_request filters and removed a duplicated load path in the checker.
- Parent XY-C1 #43690054061 was IN_PROGRESS at last inspection. NVRAM #43690054024, HTTPD security #43690054071, FRP #43690054108, AdGuardHome #43690054047, AI docs #43690054062, Process inventory #43690054006, Process lifecycle #43690054021, infosvr #43690054113, AliDDNS #43690054011 and AliyunDrive #43690054109 SUCCESS. Fresh exact-head storage/XY-C1 results required.
- No local shell/build or physical hardware result claimed. Changed paths: `mtd_storage.sh`, `mtd_storage_load_transaction.sh`, `check_storage_safe_remove.py`, storage-hotplug workflow, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`. Parent SHA: `546ca8e66a00aee08c53922a55368e61ceb507e2`.

---

## 2026-11-15 — Preserve failed restore upload for retry; repair source contracts

- Parent exact-head USB storage #43689545333 and HTTPD security #43689545379 failed because the newly added Python boundary lookups searched for literal escaped backslash-n strings rather than actual source newlines. This commit uses `chr(10)` in the storage checker and HTTPD workflow contract.
- `do_restore_st_cgi()` unconditionally deleted `STORAGE_FIFO_FILENAME` even when `mtd_storage.sh restore` failed, destroying the exact uploaded archive needed for retry. The handler now unlinks it only when restore returns success; failure displays `UploadError.asp` and keeps the uploaded archive.
- Added HTTPD regression assertions requiring one unlink under `ret == 0` and none under the error branch. Exact-head Actions for this commit pending. No local compile/test, firmware build or router result claimed. Parent SHA `0baeb3d38480464bcfc7c3127847d234caca226a`.

---

## 2026-11-15 — Reject invalid MTD backup archives before HTTPD download

- Parent exact-head USB storage #43688959814 passed MTD syntax, save retry, restore, host/BusyBox ash load fixtures, persistence gates and embedded caller graph, then failed at the final path-list assertion because the load regression path was not listed in the checker’s push/pull filter contract. This commit adds it together with the new backup regression path.
- Old `func_backup()` could leave partial archive output on compressor failure, had no actual partition-capacity check and did not validate bzip2 or tar integrity. It also did not load the target MTD size. Backup now obtains the real size, checks compression return, size range, `bzcat` and `tar tf`, and cleans partial output on every failure.
- HTTPD `do_storage_file()` now streams only when backup returns success and its output exists; otherwise it sends HTTP 500 and removes the stale path.
- Added host and BusyBox ash coverage for partial compressor failure, oversized output, corrupt compressed stream, malformed tar and a valid archive; integrated source contracts. Parent SHA `c6ae3785862d7a0272f5ecc0c7e59199f3ee3593`; exact-head Actions for this commit pending. No local test/build/hardware result is claimed.

---

## 2026-11-15 — Correct the MTD-load regression source contract

- Exact-head storage-hotplug #43688365428 on `dbfcdaba996473dd135c93c574c88f02f6f3bcf0` passed script syntax, MTD save retry, restore staging/rollback, both POSIX-shell and BusyBox 1.24.2 ash load transaction fixtures, policy tests and the storage caller graph. The final standalone checker failed because it expected a literal `BUSYBOX_BIN=` assignment inside the fixture, although the fixture correctly reads `${BUSYBOX_BIN:-}` and the workflow supplies the environment variable.
- Corrected the source contract to verify the fixture's environment lookup and ash dispatch plus the workflow's explicit `BUSYBOX_BIN="$busybox_bin"` invocation. The regression cases and workflow execution are unchanged; this is not a test bypass.
- Changed paths: `scripts/check_storage_safe_remove.py`, `SESSION_HANDOFF.md`, `dev_log.md`. Parent: `dbfcdaba996473dd135c93c574c88f02f6f3bcf0`; resulting SHA: THIS COMMIT. Fresh exact-head Actions pending. No local checkout/test, MIPS build or hardware result is claimed.
- Next: harden `func_backup()` and HTTPD `do_storage_file()` so failed/partial archives cannot be streamed as valid configuration downloads.

---
## 2026-11-15 — Make boot-time MTD load transactional

- Traced early boot: `dev_init.sh` creates `/etc/storage` and runs MTD load before defaults and the user start script. Old `func_load()` ignored decompression/extraction status, accepted partial output, advanced MD5 before extraction and unpacked over live data.
- New load validates decompression and nonzero numeric output, extracts into a private sibling, checks tar/chmod/hash staging, then moves the old directory to rollback and publishes the verified stage. A failed publish restores old settings; the old hash and lock marker remain on all pre-publication failures.
- Added host and BusyBox ash fault injection for decompression failure after output, partial extraction, failed publish/rollback and successful replacement. Integrated push/PR filters and production source contracts.
- Parent `b799ac5bb7588e015913a5bbc70e68efb5adec77`: USB storage #43687393291, HTTPD security #43687393326, Process inventory/lifecycle #43687393295, NVRAM #43687393286, AI docs #43687393272, infosvr #43687393356, AdGuardHome #43687393288, AliDDNS #43687393306 and AliyunDrive #43687393297 SUCCESS; XY-C1 #43687393279 IN_PROGRESS. New commit Actions pending. No local shell, MIPS build or hardware result is claimed.

---

## 2026-11-15 — Reconcile transactional restore contracts across storage and HTTPD Actions

- Exact-head USB storage #43687076965 on `0ed88bde4fe182f057291429e7a94af5149da772` passed shell syntax, MTD save retry, the new restore copy-failure/install-rename rollback regression, storage persistence gate, disk inventory, daemon stop and storage caller graph. Its final checker failed with `NameError: pr_filters is not defined`; it had already split the workflow into `push_filters` and `pull_filters`. The path contract now consistently checks `pull_filters`.
- HTTPD security #43687076956 passed 33 preceding policy checks, including NVRAM and storage safe-eject, then failed at its inline source contract because it still expected the old destructive `cp -rf "$tmp_storage"/. "$dir_storage"/` copy. Replaced that expectation with staging, same-filesystem rename, rollback and recovery checks matching the production restore implementation.
- Process lifecycle #43687076988, NVRAM transaction #43687076931, Process lifecycle inventory #43687076969 and AI handoff docs #43687076907 all passed exact-head. XY-C1 #43687076955 was still in progress at last poll.
- No production source semantics were changed here. Changed paths: `.github/workflows/httpd-security.yml`, `scripts/check_storage_safe_remove.py`, `SESSION_HANDOFF.md`, `dev_log.md`. Parent SHA: `0ed88bde4fe182f057291429e7a94af5149da772`.

---

## 2026-11-15 — Make MTD storage restore replacement transactional; repair exact-head contracts

- Exact-head USB storage #43686251138 on `a7fe3e4648563f6941dcb0816b59c9a903b5f8bc` passed the MTD script syntax check and the injected flash-failure/identical-retry/unchanged-skip shell test, then passed storage persistence gates, inventory, daemon-stop and full caller-graph checks. The final standalone source checker crashed at line 470 because it searched for the two literal characters `\\n` before `func_backup()` rather than an actual newline. The checker now uses the real function separator.
- Process inventory/lifecycle #43686251333 failed at shutdown ordering line 286 because it expected the obsolete `write_storage_to_mtd();` token after the caller became a status-returning expression. The check now matches the call expression while keeping the storage-removal gate ordering. HTTPD security #43686251157 and Process lifecycle inventory #43686251172 succeeded on the same SHA.
- Auditing `func_restore()` found a data-loss branch: the old implementation removed the live storage directory before unchecked `mkdir`/copy. A failure could erase the old settings and still report success. Restore now prepares a complete tree in a same-filesystem stage, renames the existing tree to a rollback path, atomically installs the staged tree, and restores the original directory when installation fails.
- Added copy-failure and install-rename-failure injection, with checks for old-config preservation, uploaded archive retention, successful replacement and service restart; wired the test into storage-hotplug Actions and expanded source contracts.
- Changed paths: `mtd_storage.sh`, `mtd_storage_restore_transaction.sh`, `check_process_lifecycle_integration.py`, `check_storage_safe_remove.py`, storage-hotplug workflow, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`. Parent SHA: `a7fe3e4648563f6941dcb0816b59c9a903b5f8bc`. New exact-head checks and current-source XY-C1 build remain required; no local full checkout or hardware test is claimed.

---

## 2026-11-15 — Fix GNU md5sum compatibility and checked-storage call contracts

- Exact-head USB storage run #43685359346 on `5338c5adfe54a7eedcfc460b284f4783c03a09cf` passed MTD script syntax and the baseline/write-failure/retry cases, then failed because the unchanged-save check used BusyBox's `md5sum -s` option. The GitHub-hosted GNU coreutils `md5sum` rejects `-s`, so every unchanged save fell through and rewrote MTD. Replaced it with portable `md5sum -c` and explicit output redirection; a local shell reproduction with GNU md5sum confirms the retry succeeds and the next identical snapshot is skipped.
- Exact-head Process inventory/lifecycle #43685359507 and HTTPD security #43685359427 exposed stale source-contract literals expecting `write_storage_to_mtd();`; production now invokes the status-returning function as an expression in a ternary. Updated both assertions to recognize the call while preserving their surrounding ordering checks.
- No lifecycle or storage semantics were relaxed. Fresh exact-head storage, process-lifecycle, HTTPD and XY-C1 Actions are required; the prior XY-C1 run #43685359467 was still IN_PROGRESS at last observation.
- Changed paths: `trunk/user/scripts/files/sbin/mtd_storage.sh`, `scripts/check_process_lifecycle_integration.py`, `.github/workflows/httpd-security.yml`, `SESSION_HANDOFF.md`, `dev_log.md`. Parent SHA: `5338c5adfe54a7eedcfc460b284f4783c03a09cf`. Full repository regression suite and target build are not claimed locally.

---

## 2026-11-15 — Preserve retryability and propagate MTD storage save failures

- mtd_storage.sh previously published the changed archive MD5 to storage_md5 before invoking mtd_write. A failed flash then caused the next identical save to skip retrying and leave the old MTD snapshot active. The hash is now staged and published only after a successful flash; staging, compression, flash and checkpoint errors return non-zero.
- write_storage_to_mtd() propagates doSystem() status. Startup logs failed persistence and continues crond startup; shutdown reports failure but continues power-down; firmware update refuses to write the image unless timestamp and MTD persistence succeed and attempts watchdog recovery on abort.
- storage_save_time() now validates time/localtime and overflow, stages a same-directory temp file, checks write/flush/fsync/close, and atomically renames it. Failures clean up without truncating the prior timestamp.
- Added a shell regression for successful baseline → injected MTD failure → identical retry → unchanged skip, a GNU89 -Wall -Wextra -Werror persistence-policy test, and source/order contracts. Storage-hotplug and XY-C1 path filters include the source and tests.
- Changed paths: rc.c, rc.h, mtd_storage.sh, MTD retry test, persistence policy/header test, storage-hotplug and XY-C1 workflows, checker, handoff, roadmap and dev_log. Parent SHA: 31ae7b6f253734cc0abf5f67b5c34e861229155b. Exact-head Actions/local test/build/hardware are not claimed.

---

## 2026-11-15 — Stop Transmission and Aria daemons by verified process identity

- Transmission and Aria stop paths use process-inventory preflight, bounded kill_services(), and verified daemon absence. Public stop APIs hold the shared storage operation lock and re-check postconditions; RUNNING or UNKNOWN prevents eject success.
- Source commit 2aedad115793e7a6de4f2114272c845559105783. The initial source commit was not separately recorded in the top-level chronological dev log before this catch-up entry.

---

