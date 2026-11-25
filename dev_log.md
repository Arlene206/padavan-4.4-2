## 2026-11-25 20:40 CST — Include the string API used by SOAPAction tests

- Exact miniupnpd HTTP request framing run #45606537043 on `1f5e34e7` failed before executing tests: the new quoted-value assertion used `memcmp()` without including `<string.h>`, and the workflow treats implicit declarations as errors under GNU89 `-Werror`.
- Added the standard declaration include to the test translation unit. The parser change itself has not yet completed CI validation; exact-head build/test rerun required.

---

## 2026-11-25 20:35 CST — Extract MTD fill source without parsing heredoc bodies as C

- Exact USB-storage run #45605993018 on `1c62feb6` passed both host-shell and BusyBox ash MTD save/restore/load/erase/backup/fill regressions, including all eight injected fill failures and the normal success case. The final static checker failed when the C-style `body()` scanner tried to parse the shell `func_fill()` function: heredoc payloads contain shell braces and quoting that are not C source, so the scanner could not find a balanced closing brace.
- Replaced only the `func_fill()` extraction with its stable shell boundary: the function starts at `func_fill()` and ends at the top-level `case "$1" in` command dispatcher. The smaller helper functions continue using the shared brace parser. Runtime MTD source and tests are unchanged.
- Exact storage rerun required; `1c62feb6` is FAIL solely at the static checker after the runtime regression steps passed. No local or hardware result is claimed.

---

## 2026-11-25 20:30 CST — Reject malformed quoted MiniUPnP SOAPAction values

- `ParseHttpHeaders()` stripped surrounding quotes by testing `p[n-1]` after finding a value length. A one-byte value consisting only of a quote satisfied both ends of the check and reduced the signed SOAPAction length from 1 to -1; downstream `%.*s` logging then treated the negative precision as unbounded. The parser also retained trailing whitespace and scanned the line without an explicit header-bound check.
- Added a length-safe SOAPAction normalizer that trims optional edge whitespace, rejects unbalanced/interior quotes and embedded control bytes, and returns an offset/length pair without modifying outputs on failure. The production parser now bounds the value scan by `req_contentoff`; Content-Length scanning is bounded the same way. Added GNU89-compatible boundary tests for one-byte quotes, mismatched quotes, empty quoted value, whitespace and control characters.
- Exact MiniUPnP framing Actions required. No local build or hardware result is claimed.

---

## 2026-11-25 20:35 CST — Publish MTD default files atomically

- The latest MTD fill guard now detected failed heredoc writes and chmod errors, but wrote directly to live config paths before checking status. A partial file remained at its final name after ENOSPC/I/O failure; subsequent fills would skip it because the pathname already existed.
- Added same-directory temporary staging for generated scripts and configuration, verified the staged file's type/mode, then published with no-clobber rename. Failures remove the stage and propagate to reset/fill callers. DNSMasq base content and all optional overlays publish as one file; existing user config is not replaced.
- Extended host-shell and BusyBox ash regressions to inject content-write, chmod and rename failures and verify no partial target/stage remains. Exact USB-storage and XY-C1 Build/TRX/artifact workflows required; no local hardware result is claimed.

---

## 2026-11-25 20:25 CST — Teach the storage checker to parse shell parameter expansions

- Exact USB-storage run #45605321376 on `6dd7052c` passed the runtime restore/load/erase/backup tests and the pre-existing full-caller-graph contract, then the static checker failed while extracting `func_fill_check_file()`. Its C-style brace scanner interpreted the shell parameter expansion `${target%/*}` as the start of a C block comment and reached EOF without a closing delimiter.
- Extended the shared function-body extractor to skip nested `${...}` shell parameter expansions, and added a focused parser regression containing the slash-star pattern. This is a checker-only correction; MTD production/test behavior is unchanged.
- The exact `6dd7052c` storage run is FAIL due to checker parser incompatibility; exact-head rerun required. No local test or firmware result is claimed.

---

## 2026-11-25 20:20 CST — Fail MTD fill on default-file write or mode errors

- The 23 default-file heredoc writes and their `chmod` operations were unchecked. A full filesystem, read-only transition or write error could leave an empty/partial configuration file or a non-executable hook while `func_fill()` continued and reset started services.
- Added immediate status checks after every default-file `cat >`/`cat >>`, plus a common file-type/symlink/mode helper that propagates each chmod error. Added host/BusyBox ash injection for heredoc write and mode-setting failures; static checker pins all 23 heredoc status gates and representative mode operations.
- Exact USB storage and XY-C1 validation required. No local shell/firmware/hardware result is claimed.

---

## 2026-11-25 20:15 CST — Isolate clean fill from the corrupt archive fixture

- Exact USB-storage run #45604771628 on `13f9f431` passed restore/load/erase/backup, the four path-symlink denials, the injected mkdir failure, the reset_ss failure case, and the chnroute partial-extraction failure. The final positive-path assertion failed because the negative chnroute test deliberately left a plain-text invalid archive at the fixture path; the subsequent clean-fill case reused it, so real `tar jxf` correctly rejected it. The production source and mkdir wrapper were not at fault.
- The successful-fill fixture now removes the deliberately corrupt archive before running against a clean storage tree. Production source is unchanged.
- Exact storage run is FAIL due to test-state leakage between scenarios; new exact-head validation required. No local test or hardware result is claimed.

---

## 2026-11-25 20:10 CST — Point the chnroute failure fixture at its test archive

- Exact USB-storage run #45604235661 on `5dd0dcf0` passed restore/load/erase/backup and the preceding fill symlink and mkdir-failure cases, but the injected chnroute failure case unexpectedly succeeded because the test's sed pattern started at column zero while the production `chnroute_file` assignment is indented inside `func_fill()`. The test therefore left the production path `/etc_ro/chnroute.bz2` unchanged and skipped extraction.
- Replaced the fixture with an exact assignment-value substitution that preserves the indentation, and pinned the redirection in the source contract. Production code is unchanged.
- Exact storage and XY-C1 validation required; the 5dd0dcf storage run is FAIL due to fixture path injection, not a successful archive-failure test. No local test result is claimed.

---

## 2026-11-25 20:05 CST — Verify the safe default-fill success path

- The new fill regression already exercised four fail-closed cases; it lacked a clean-tree success case, so an overly strict path preflight or a shell portability issue could have made every fill fail while all negative assertions passed. The reset_ss fixture now publishes a valid start script on success, and the test verifies the core generated scripts and dnsmasq/inadyn/WLAN defaults under both host shell and BusyBox ash.
- Test-only expansion; the storage workflow and source contract are unchanged apart from requiring the positive-path assertion. Exact storage run required; no local test result is claimed.

---

## 2026-11-25 20:00 CST — Require successful reset_ss publication during fill

- Default fill previously invoked `reset_ss.sh -a` without checking its result or confirming that the expected `start_script.sh` was published. A write/rename failure could be hidden while fill returned success and reset proceeded to service startup.
- Fill now stops on a failing generator and independently verifies that the resulting start script is a regular non-symlink file. Added a host/BusyBox ash failure-injection case and static source-contract assertions.
- Exact storage and XY-C1 validation required. No local shell/firmware/hardware result is claimed.

---

## 2026-11-25 19:55 CST — Propagate required MTD fill directory-creation failures

- MTD default fill ignored several `mkdir -p` failures for cron, HTTPS, dnsmasq, inadyn, WLAN, OpenVPN and strongSwan state. The function could continue with an incomplete configuration tree and let reset proceed to application startup. Added a checked mkdir helper that revalidates the path, preserves existing-directory modes, logs failures, and aborts fill; migrated all required default directories to it.
- Extended the host/BusyBox ash fill regression with injected failure at the first required cron directory, asserting nonzero fill status and that reset-script generation never begins. Static source contract covers the directory call sites.
- Exact storage and XY-C1 validation required. No local shell/firmware/hardware result is claimed.

---

## 2026-11-25 19:50 CST — Propagate MTD fill failures to the command exit status

- The new path guard correctly returns failure from `func_fill()`, but the command dispatcher previously invoked it and then exited the script with the unchanged global `result=0`. Direct `mtd_storage.sh fill` callers therefore saw success even when a protected symlink or archive extraction failure aborted fill. The reset path already propagates the function result through its conditional.
- The `fill)` dispatcher now records a nonzero result when `func_fill` fails; the static contract pins this status propagation. The existing production-linked host/BusyBox fill regression checks that unsafe paths and partial chnroute extraction return nonzero.
- Exact storage and XY-C1 validation required. No local shell/firmware/hardware result is claimed.

---

## 2026-11-25 19:45 CST — Preserve shell parameter expansion in fill path guards

- Exact USB-storage run #45603292894 on `dfee708a` passed the earlier MTD restore/load/erase/backup regressions, then failed when the new fill-path test tried to create its sandbox: literal backslashes had been retained before `${TMPDIR}` and `${BUSYBOX_BIN}` in the generated shell fixture. The source helper's `${target}`/parent/component parameter expansions had the same quoting defect and would not walk path components correctly.
- Removed the unintended backslashes from the three production shell expansions and both test expansions; strengthened the static source contract to pin the parameter expansions themselves. Exact storage and XY-C1 validation required.
- The prior exact-head storage run is FAIL due to fixture/bootstrap expansion, not a passing result for the new fill guard. No local test or firmware result is claimed.

---

## 2026-11-25 19:40 CST — Make bundled chnroute extraction fail closed

- The MTD default-fill path used `mkdir -p ... && tar jxf ...` inside a conditional but discarded the compound command's failure. A corrupt bundled chnroute archive could leave a partial `/etc/storage/chinadns` directory; subsequent boots would see the directory and never retry extraction.
- Fill now checks directory creation, checks archive extraction, removes partial output on extraction failure (logs a critical cleanup failure if necessary), and returns failure so reset retains its save barrier and does not restart apps. Extended the host/BusyBox ash fill regression with injected partial extraction and cleanup assertions.
- Exact USB storage, static source contract and XY-C1 build verification required. No local BusyBox/firmware/hardware result is claimed.

---

## 2026-11-25 19:35 CST — Block nested symlink traversal during MTD default fill

- `func_mdir()` guarded only the `/etc/storage` root. The legacy `func_fill()` then treated symlinked nested directories as valid because `[ -d ]` follows them and used heredoc redirections for default scripts/configs. A crafted `dnsmasq`, `cron`, `inadyn`, `wlan`, OpenVPN or strongSwan directory link could therefore redirect privileged writes outside the storage tree; symlinked leaf config files were also silently accepted by `[ -f ]`.
- Added a preflight that checks each path component and every generated/migrated file leaf before any fill-side mkdir, move or heredoc write. Added production-linked regressions for a nested dnsmasq directory symlink, an ancestor cron symlink, a dnsmasq config leaf symlink, and a boot-script leaf symlink; scheduled under both host shell and firmware BusyBox ash.
- Exact USB storage, static source contract and XY-C1 firmware validation required. No local BusyBox/firmware/hardware result is claimed.

---

## 2026-11-25 19:30 CST — Repair zombie fixture setup after failed daemon launch

- Exact USB-storage run #45602345339 confirmed the new parser accepted the complete live proc records and passed all preceding restore checks, then the final zombie scenario failed before the restore command: the preceding no-daemon launcher case had removed the synthetic PID directory, and the zombie fixture wrote directly into the now-missing `stat` path. Restored explicit directory setup and corrected the fixture's printf format. Production parser is unchanged.
- Exact storage and XY-C1 checks required. No local BusyBox/firmware/hardware result is claimed.

---

## 2026-11-25 19:25 CST — Complete the proc stat host fixtures

- Exact storage run #45601959884 exposed a test-fixture defect in the new full-record check: the synthetic `rstats` records stopped at field 21 and omitted `starttime` (field 22), so the production parser correctly returned UNKNOWN before the restore scenarios began.
- Extended all five live-process fixture records and the zombie fixture through field 22. Production parser behavior is unchanged; rerun exact storage/Process and XY-C1 checks. No local BusyBox/firmware/hardware result is claimed.

---

## 2026-11-25 19:20 CST — Reject truncated proc stat records and zombie rstats

- The MTD restore/erase postcondition treated any numeric `/proc/<pid>/stat` line ending in `<comm>) ` as a complete live process. A truncated record such as `998 (rstats) ` passed the comm check and could authorize a destructive restore; a zombie `rstats` also counted as running and could trigger an unnecessary stop timeout. The scanner now requires the state and numeric PID/group/session/starttime fields through proc stat field 22, rejects malformed state records as UNKNOWN, and counts only non-zombie/non-dead rstats tasks as present.
- Extended the production-linked restore regression for truncated matching records and zombie rstats, updated the source contract, and expanded valid proc fixtures. Exact storage/Process/XY-C1 Actions required. No local BusyBox/firmware/hardware result is claimed.

---

## 2026-11-25 19:15 CST — Remove the obsolete second MTD backup temp file

- After the recursive-tar repair, `func_tarb()` still allocated a `.sort.*` temp file that is no longer read or written. That dead allocation could cause a backup to fail on a second `mktemp` error despite the only required NUL inventory file being available. Removed the unused path and cleanup references; the source contract now requires exactly one temporary inventory allocation and no stale `sort_list` state.
- Exact USB-storage regression and XY-C1 Build/TRX/artifact checks required for this production shell change. No local BusyBox/firmware/hardware result is claimed.

---

## 2026-11-25 19:10 CST — Match backup fixture contract to its streaming AWK assertions

- Exact USB-storage run #45600452827 on `b0a6b00d` passed the recursive archive builder contract and the 1,200-file host/BusyBox-ash regression, then failed another legacy fixture assertion: the checker still required single-quoted grep patterns, while the corrected test now validates archive entries within one AWK stream to avoid early pipe closure and counts only files, not the `./bulk/` directory.
- Updated the fixture contract to pin the AWK hidden-file/empty-directory checks, exclude the bulk directory itself from the payload count, and retain the exact 1,200-file postcondition. Checker-only correction; exact-head storage Actions required. No local BusyBox/firmware/hardware result is claimed.

---

## 2026-11-25 19:05 CST — Align the detailed MTD backup contract with recursive tar

- Exact USB-storage run #45600031271 on `276b29f5` passed the corrected 1,200-file backup test and all earlier MTD restore/load/erase regressions, then failed a second, older source-contract assertion at line 1266. The checker still required `sort -z | xargs -0 tar`, even though the production fix now normalizes timestamps from the checked NUL inventory and performs one recursive `tar -cf "$tmp" .` to avoid BusyBox xargs command-size truncation.
- Updated the detailed contract to pin the actual inventory/timestamp/archive sequence and explicitly reject the repeated xargs/tar form. Checker-only correction; exact-head USB-storage rerun required. No local BusyBox/firmware/hardware result is claimed.

---

## 2026-11-25 19:00 CST — Count MTD backup payload files without counting the parent directory

- Exact USB-storage run #45599645522 on `ae45657d` passed the restore rollback, load/fill, and erase/reset regressions, then failed the large-backup test with 1,201 entries instead of 1,200. The archive correctly contains the `./bulk/` directory plus its 1,200 files; the assertion had counted the directory as a payload file. The three separate `grep -q` pipelines also let their producer receive early pipe closure. Replaced them with one full-stream AWK assertion that checks required entries and counts only children under `./bulk/`.
- Test-only correction; production tar traversal remains unchanged. Exact-head storage-hotplug Actions required; no local BusyBox/firmware/hardware result is claimed.

---

## 2026-11-25 18:55 CST — Match MTD restore retry rollback to the active tree

- Exact USB-storage run #45598586819 on `32d56070` passed the preceding storage regressions, then `mtd_storage_restore_transaction.sh` failed because its second startup-failure scenario reused the tree installed by the first failed-start scenario but still asserted that the original source file existed. The first scenario correctly installs the restored tree and retains the former tree in rollback; after the test removes that backup, the next scenario must expect the currently active `new.conf` in its rollback snapshot. Corrected the fixture assertion; production restore logic is unchanged.
- Exact-head USB-storage Actions required. No local host test, MTD, firmware or hardware result is claimed.

---

## 2026-11-25 — Prevent large MTD backups from being truncated by BusyBox xargs batching

- Review of BusyBox 1.24.2 `findutils/xargs.c` confirmed it caps each command line at 32 KiB. `func_tarb()` passed every path to `xargs -0 tar -cf "$tmp"`; on a larger `/etc/storage`, xargs launches tar multiple times and every `-c` truncates the same archive. The command can return success even though only its final batch remains, so MTD backup silently drops most files.
- Keep the checked NUL-delimited `find` snapshot and safe timestamp normalization, but create the archive with one recursive `tar -cf "$tmp" .` invocation. BusyBox tar's recursive walk handles the whole tree without an argv-size boundary and propagates file traversal errors. Regression creates 1,200 long-path entries (>32 KiB of argv) and requires all 1,200 to appear in the archive, under host shell and BusyBox ash. Static source contract rejects the old xargs/tar pattern.
- Exact-head storage-hotplug validation is required; no local BusyBox binary or firmware run is claimed.

---

## 2026-11-25 — Correct escaped newline delimiters in storage checker

- Source-contract review found the two new MTD function-boundary lookups in `check_storage_safe_remove.py` searched for the two literal characters `\\n` instead of an actual newline. This would raise `ValueError` when the storage workflow reaches the checker, despite the C/shell tests being independent.
- Corrected both Python string literals to use a newline escape and left the production source/test changes unchanged. The storage workflow on the current exact SHA will validate this checker correction.

---

## 2026-11-25 — Do not mask partial MTD backup inventory failures

- `func_tarb()` piped `find` and `sort` directly into `xargs`. Padavan's BusyBox ash does not enable `pipefail`, so a partial `find`/sort result followed by successful `xargs` could be treated as a complete snapshot and written to flash. This risks persisting an incomplete /etc/storage tree after an inventory I/O error.
- The inventory is now written to a private NUL-delimited temporary file, and find, touch, sort and tar/xargs each have independently checked exit status. The backup regression injects a find command that emits a valid prefix and exits 1; backup must fail and leave no archive. Added a source-contract assertion. Exact-head storage Actions required; no local BusyBox or firmware execution claimed.

---

## 2026-11-25 — Correct the MTD fill test's escaped newline

- Exact storage-hotplug run #45597825821 on `429aa42a` failed at the final non-directory storage-root assertion. Inspection of the fixture showed its `printf` format contained two backslashes (`\\n`), so it wrote the literal characters `\n` and the equality check failed even if the storage path was unchanged. Corrected the fixture to emit a real newline; the CI message did not establish a production storage-path mutation.
- Kept the explicit regular-file rejection in `func_mdir()` as defense in depth alongside its symlink and generic non-directory checks, and pinned it in the source contract. This updated test needs a fresh exact-head host/BusyBox ash run.
- No local shell, MTD or hardware result is claimed.

---
## 2026-11-25 — Verify rstats readiness after a zero-exit launcher

- Follow-up to the restore failure fix: `func_start_apps()` previously treated `/sbin/rstats` exit status 0 as proof the daemon was running. A daemon that forks then exits immediately (or whose startup wrapper returns success without leaving a process) could still cause restore/reset to be reported as successful.
- Added a bounded five-poll procfs postcondition matching the existing fail-closed stop logic. UNKNOWN inventory and a daemon that never appears both fail startup. Extended restore regression with a launcher that exits zero but does not publish an rstats process; the archive, save marker and rollback tree must remain. Exact-head storage Actions required; no router test claimed.

---

## 2026-11-25 — Retain the MTD reset save barrier when services fail to restart

- Review of the reset dispatch found the same recovery-state bug recently fixed in restore: reset removed `.storage_locked` before calling `func_start_apps()` and only set a failure status if startup failed. This allowed subsequent MTD saves to run even though the default storage service set had not restarted.
- Reset now requires `func_start_apps()` to succeed while the save barrier is still present, and only then removes the marker. A startup failure returns non-zero and retains the marker. Extended the host/BusyBox ash erase-reset regression so the mock verifies the marker exists at every start call and injects a failed restart; source-contract assertions now enforce the ordering.
- Exact-head storage-hotplug CI is required. No device test is claimed.

---

## 2026-11-25 — Remove the unnecessary unbound mkdir helper from the MTD load fixture

- USB-storage exact-head run #45597345823 passed the MTD save-retry and restore transaction tests (including BusyBox ash), then failed in the load transaction fixture at its symlinked-storage check with `real_mkdir: parameter not set`.
- The fixture had no wrapped mkdir command, so the saved `real_mkdir` indirection was unnecessary. Removed the variable and use the system `mkdir` directly for the outside-directory fixture. Exact-head storage Actions must rerun the full load/erase/backup sequence; no local result is claimed.

---

## 2026-11-25 — Report and preserve MTD restore state when rstats restart fails

- The restore transaction previously called func_start_apps() after deleting its save-lock marker, archive and rollback tree, then returned success without checking the start result. A failed rstats launch could therefore be reported as a successful restore with no service.
- Restore now starts rstats before clearing the barrier or deleting recovery inputs. A start failure returns non-zero and retains the lock marker, archive and previous storage snapshot. Added an injected start-failure regression that verifies the failure is surfaced and all recovery state remains available. Exact-head storage Actions are required; no device test is claimed.

---

## 2026-11-25 — Define the real mkdir command in the MTD fill safety test

- The storage-hotplug workflow on `5733132a` passed RWFS, reset_ss, MTD save/retry and the MTD restore rollback regression after the procfs fixture redirect. The next `mtd_storage_load_transaction.sh` step failed under `set -u` because the new symlink/non-directory fill test called `$real_mkdir` without capturing it. Added the same real-command capture pattern already used for `rm`; removed the duplicate `/proc/mtd` rewrite introduced while the fixture redirect was being developed.
- This is a regression test harness correction; production `func_mdir()` remains unchanged. Re-run storage-hotplug on the exact new SHA to verify the fill path guard and later storage checks.

---

## 2026-11-25 — Reject a symlinked storage root in SCUT cron registration

- The cron helper hardened `cron/` and `cron/crontabs/` but ran `cd /etc/storage` before checking the storage root itself. A symlinked `/etc/storage` would redirect privileged lock and crontab writes into another directory. Require the storage root to be a real directory and not a symlink before changing directory.
- Added a fixture regression that swaps the test storage root for a symlink to an outside directory and verifies the helper returns failure without creating `cron/` in the target. The process-lifecycle source checker now requires this guard and regression.
- Pending exact-head crond/process-lifecycle Actions. No local router result is claimed.

---

## 2026-11-25 — Update remaining MTD fixtures for direct procfs parsing

- The save-retry regression now passes on both host sh and firmware BusyBox ash after replacing the actual /proc/mtd pathname. The next restore-transaction step still used an older cat-wrapper fixture, but production func_get_mtd() now invokes awk directly; that wrapper cannot redirect the read. The backup, load and erase/reset fixtures retained the same stale assumption.
- Updated all four remaining harnesses to rewrite /proc/mtd itself, preventing the host tests from reading the runner's absent procfs node and allowing the restore/backup/load/erase transaction assertions to execute against their intended synthetic MTD inventory. Exact-head storage Actions are required; no local router/firmware run is claimed.

---

## 2026-11-25 — Fail closed on FTP LIST entry-stat errors

- Review of the no-chroot LIST fast path found a visibility bypass: it treated an entry as visible when fstatat(..., AT_SYMLINK_NOFOLLOW) failed, because symlink resolution ran only after a successful stat. A dangling/replaced entry or transient lookup error could therefore skip the internal-metadata symlink gate.
- The path now requires a valid listing directory descriptor and a successful no-follow stat; symlinks additionally require successful descriptor-anchored target resolution. Added direct policy regression cases for stat failure and symlink-resolution failure, plus an exact-source workflow contract. This requires exact-head VSFTPD Actions; no local firmware build is claimed.

---

## 2026-11-25 — Point MTD save-retry fixture at the strict procfs parser

- USB-storage exact-head run #45596308813 on `62254bda` passed the preceding storage checks, RWFS conversion under host sh/BusyBox ash, and reset_ss publication regression. It then failed before testing save retry because the fixture still rewrote `cat /proc/mtd`, while production `func_get_mtd()` had been hardened to parse `/proc/mtd` directly with awk. The test therefore read the host runner's absent `/proc/mtd` instead of its fixture.
- Updated the fixture rewrite to substitute the procfs pathname itself. This is a regression-harness correction only; exact-head storage Actions must rerun the save failure→retry flow and all later storage gates. No local firmware/MTD execution is claimed.

---

## 2026-11-25 — Reject symlinked MTD storage directories before fill/save/load

- Review found `func_mdir()` treated a symlink to an existing directory as valid (`[ -d symlink ]` succeeds) and did not propagate `mkdir` failure to callers. The `fill` action could therefore write privileged defaults through a symlinked `/etc/storage`; `save`/`backup` could archive the symlink target, and `load` did unnecessary device reads before its later check. The helper now rejects symlinks and non-directory objects, quotes its path, reports creation failures, and every caller aborts before continuing.
- Extended the transactional MTD-load regression to invoke `fill` against a symlinked directory and a regular-file path, proving neither target is modified and failure is returned. The same test runs under host `sh` and the firmware BusyBox ash in CI.
- Exact RWFS and MTD workflow verification is still pending after the reset_ss fixture corrections; no local hardware/firmware result is claimed.

---

## 2026-11-25 — Correct the reset_ss runtime-argument assertion

- Exact storage-hotplug run #45595654382 on `c3b9d616` passed the RWFS flash/NVRAM regression and the reset_ss runtime-interface and variable-preservation checks, then failed because the new assertion expected four leading spaces before `case $1 in`; the template intentionally has that case statement at column zero. Corrected the expected line. The same test commit also normalizes the prior directory-destination checkpoint entry to real Markdown line breaks.
- No source change was required for this CI failure. The directory-target production guard and regressions remain in place and require a fresh exact-head storage run.

---

## 2026-11-25 — Preserve empty-line and comment matching after crontab trimming

- Follow-up syntax review of the new leading-whitespace normalization found the case pattern did not retain the empty alternative for blank lines and normal comments. Corrected the pattern to skip both empty trimmed lines and lines whose first non-whitespace character is `#`, and pinned that exact case arm in the checker.
- This is a corrective follow-up to the preceding crontab parser change; exact-head process-lifecycle regression is required before treating it as validated.

---

## 2026-11-25 — Ignore indented comments when matching crontab commands

- Review of the exact-sixth-field duplicate check found it skipped only lines whose first byte was `#`. A valid crontab comment with leading spaces could be split as fields and, if its sixth token matched the target executable, incorrectly suppress the job. Normalize leading whitespace before comment detection and field parsing.
- Expanded the shell regression with an indented comment whose sixth token is the target script; updated the source-contract checker to pin the whitespace normalization and test fixture. Exact-head process-lifecycle Actions validation is required; no local router test or firmware build is claimed.

---

## 2026-11-25 — Align SCUT crontab source checker with exact-token lookup

- Exact process-lifecycle run #45595370787 on `6554727e` passed the shell regression for exact command identity, concurrent writers, lock permissions and symlink rejection, then failed in the static checker because it still required the removed grep-substring implementation and obsolete injected grep failure fixture. Updated the contract to assert the production lock and parsed sixth-field identity check, and to require the newer exact-match/concurrency/symlink regression cases.
- This is a static-test alignment only; no production code change is claimed. Fresh exact-head process-lifecycle Actions validation is required; no local runner result is claimed.

---

## 2026-11-25 — Reject directory destinations in reset_ss atomic publication

- Review of the new `reset_ss.sh` staging flow found `mv temp start_script.sh` treats an existing directory destination (and a symlink to a directory) as a container, moving the staged script inside it while returning success. That violates the promised atomic replacement and can leave the actual startup-script path unchanged. Added a preflight that rejects real directories and directory symlinks while continuing to replace regular files and symlinks to non-directories.
- Added regression cases proving directory destinations and symlinks to directories are rejected without modifying their contents or moving the staged script into them. The reset_ss test's prior indentation assertion correction is included. Exact-head host and BusyBox ash verification required; no local result is claimed.

---

## 2026-11-25 — Align reset_ss template regression with preserved indentation

- Exact USB-storage run #45594844271 on `9e2f6dc4` passed the RWFS-to-UBI regression under host `sh` and BusyBox ash after removing shell `command -v` preflight gates. The next reset_ss test step failed because the generator correctly preserved the template's four/eight leading spaces, while the test expected unindented exact lines. Updated only the expected template lines; runtime variable and command-substitution checks remain intact.
- No production source change was required for this failure. Fresh exact-head storage-hotplug validation is required; no local firmware build is claimed.

---

## 2026-11-25 — Keep internal ACL symlinks hidden with share ACL enforcement disabled

- Follow-up review of the LIST symlink ACL fix found the legacy fast path `if (!tunable_chroot_local_user) return 1` ran before symlink target inspection. That setting disables share ACL checks, but must not expose internal `.__*` ACL metadata through an alias. The no-chroot listing path now resolves symlinks relative to the pinned directory and hides unresolved/metadata targets before preserving ordinary visibility.
- Extended the vsftpd workflow source contract to assert this early-return ordering. Existing ACL policy regression covers a symlink alias to an internal metadata file. Exact-head vsftpd CI is required; no local build is claimed.

---

## 2026-11-25 — Enforce target-share ACLs for FTP LIST symlinks

- The FTP listing filter classified an entry by its lexical path and used `fstatat(..., 0)`, which follows a directory symlink. A user allowed to read share A could therefore see symlink names targeting unreadable share B: the canonical target's share ACL was not checked until a later CWD or file-open command. Added a descriptor-anchored symlink resolver (`/proc/self/fd/<dirfd>/<leaf>`), rechecks the symlink inode after resolution, rejects dangling/internal-metadata targets, and applies the normal read ACL to the canonical target before listing the entry.
- Added GNU89 regression coverage for a cross-share directory symlink, dangling link, internal metadata alias, and regular-file rejection, plus exact source-contract checks in `vsftpd-stability.yml`. This change still requires exact-head vsftpd CI; no local compile/runtime result is claimed.

---

## 2026-11-25 — Fix MTD restore test wrapper parameter expansion

- Source review of `0bfcc6eb`'s new MTD-index-100 regression found the `bzcat` fixture was inside a single-quoted heredoc but escaped its shell variables as `\\${...}`. Those backslashes suppress expansion, so the fixture ignores `BZCAT_EMPTY` and exits with a literal status token; the successful extraction case cannot actually pass. Removed the unnecessary escapes so the existing empty-output, nonzero-status, and valid-archive cases exercise the intended paths.
- This is a test-fixture correction only. The current storage-hotplug workflow is still running on the RWFS fixture fix; this commit requires a fresh exact-head storage run to reach and verify the MTD transaction tests.

---

## 2026-11-25 — Make the BusyBox ash RWFS fixture resolve applets deterministically

- Exact storage-hotplug #45592026746 on `6eac6fbe` passed all host-sh tests, including the newline-separated multi-mount guard and blank-inventory rejection, but its second run under BusyBox ash failed before the first conversion: the source preflight could not resolve `awk` from the fixture PATH. The test now exposes the firmware BusyBox binary through `awk`/`cat` applet-name symlinks and resets PATH per fixture instead of accumulating prior sandbox paths.
- This is test-harness-only; RWFS production parser changes are unchanged. Fresh exact-head storage-hotplug and XY-C1 checks are required. No local firmware BusyBox run is claimed.

---

## 2026-11-25 — Reject blank mount output before RWFS erase

- Exact storage-hotplug #45591520927 on `5b9938c5` passed the multi-line `ubi50` decoy / later `ubi5` mounted-volume regression but failed the empty-inventory case. The mocked `mount` emits a single newline for empty output; AWK increments `NR` for that blank line even though it is skipped. The parser now counts structurally valid mount records and rejects zero valid records. The existing empty-inventory fixture covers this case.
- This fixes a second fail-closed gap in the same RWFS conversion preflight. The next exact-head storage-hotplug run must validate the blank inventory and all erase/format/attach failure paths. No local shell or firmware build is claimed.

---

## 2026-11-25 — Repair RWFS mount-inventory parsing and regression fixture

- Exact storage-hotplug run #45590500560 on `09e56c54` exposed a destructive-path blind spot: an over-escaped `printf` format emitted literal `\\n` separators, so the AWK mount parser could inspect only the first mount line and miss a later RWFS mount. Restored real newline separation and added a two-entry regression with an unrelated `ubi50` decoy before the owned `ubi5` mount.
- The empty-inventory fail-closed guard also revealed that later injected erase/format/attach cases inherited an empty test mount table. The fixture now defaults to a valid unrelated rootfs mount and explicitly overrides it for empty/malformed/error cases.
- The failed run's preceding storage regressions and BusyBox ash build passed; its RWFS regression stopped at the erase-failure fixture before reaching the erase command. The correction needs new exact-head storage-hotplug and XY-C1 checks. No local test, firmware build, or hardware result is claimed.

---

## 2026-11-25 — BusyBox procfs and U2EC lifecycle follow-up

- BusyBox procfs scanner hardening: `d018a324` rejects empty/malformed stat records, `f097a5b5` retries bounded interrupted reads, `63d18699` validates every numeric stat field used by FAST_TOP/SMP, and `7b65f0d9` removes one-byte-past-length sentinel reads from the reusable validation helpers. Exact host ASan/UBSan/source-contract regressions passed (#45575979483, #45577415801, final exact helper-boundary run #45578684818 on `7b65f0d9`).
- U2EC FIFO event path: `a9231a89` handles FIFO EOF/EAGAIN safely and retains an RDWR nonblocking descriptor to avoid read-side HUP wakeups. The regression now models a writer connecting, writing and closing before asserting EOF readiness; the previous initial test incorrectly assumed an unopened FIFO writer would already make `select()` readable. Final test/source-contract run #45580342749 passed on `66d40c75`.
- U2EC resource cleanup: `6112d036` closes discovery datagram/stream sockets on setsockopt/bind/listen failures, and `b6fc588b` closes both listener sockets when the discovery select loop exits on a non-EINTR error. Exact U2EC run #45580771843 passed on `b6fc588b`.
- Concurrent U2EC lock cleanup follow-ups: `0d20e25f` makes printer lock release idempotent; `640a9580` also releases the lock on cancelled/rate-limited FIFO events. Current exact-head U2EC run #45580921217 and process lifecycle inventory #45580921120 both PASS on `640a9580`.
- Intermediate U2EC CI failures were isolated to test fixture declarations, the FIFO lifecycle test setup/sentinel, and checker text matching; each was corrected atomically. The final current-head host regression and source contract are green.
- During a concurrent update, `95ea99d`/`85dfaea` temporarily replaced the repository root tree with a subtree-only tree. The complete root was restored without force by `9294af65`; subsequent main history includes the full restored tree. Backup branch remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- XY-C1 #45580921143 on `640a9580` and NVRAM audit #45580921210 were still IN_PROGRESS at the last poll. XY Build/TRX/artifact steps had not yet completed; no L3 claim. AI handoff contract #45579577795 passed on `9294af65` but must be rerun for the new docs commit. No L4 hardware test is claimed.
- Unresolved candidate for the next source slice: U2EC `multi_client()` SIGALRM handler reads connection-list/last-busy state that may be concurrently changed; investigate a safe signal/list ownership strategy before modifying it. PPP process-wide umask changes remain deliberately untouched without tracing their actual credential/file writers.

---

## 2026-11-25 — FTP ACL, DHCP ACK, HTTPD upload, U2EC build and PPP lock hardening

- VSFTPD ACL metadata reader now revalidates that the pinned directory entry still names the inode whose bytes were read; rename+replacement regression and exact-source check passed (#45571067464 on `a4959513`).
- BusyBox udhcpc now rejects DHCPACK server-ID mismatches in selected-server states while retaining any-identified-server semantics during REBINDING; exact regression passed (#45572615762 on `88eaeb8b`).
- HTTPD firmware CRC opens only no-follow, nonblocking, single-link regular files; symlink/FIFO/hardlink cases are covered. A Python workflow delimiter assertion was corrected in a separate atomic test commit; exact HTTPD run #45573299714 passed on `b59901af`.
- U2EC's package Makefile now chooses `all` as the default target; this fixes package.mk's bare-make invocation silently compiling only the first object and not linking the daemon. U2EC #45573521621 and Process inventory #45573521659 passed on `147a76bc`; XY-C1 #45573521580 on the same SHA passed Build, TRX verification and artifact upload.
- PPP callback sidecar lock opens now use `O_CLOEXEC` for both creation and reopen. Two successive workflow-checker mistakes were corrected atomically; final WAN callback #45574283546 and AI handoff contract #45574283526 both passed on `c78487d2`. On parent source SHA `97871e6c`, NVRAM #45574145573, Process inventory #45574145516, HTTPD #45574145500, AI docs #45574145491 and process inventory #45574145427 passed.
- Exact-source XY-C1 run #45574145443 for SHA `97871e6c42862b38cf152f31171ea0b631f14118` remained IN_PROGRESS at Build XY-C1; TRX/artifact pending. Do not infer this from the earlier `147a76b` pass. Current source HEAD before this documentation checkpoint: `c78487d2b564b43f7a7f249618a8d5b4d340744b`.
- A review noticed four legacy `umask(0000)` sites in PPP callbacks. No change was made: existing PPP options regression intentionally reproduces PID-1's zero umask and asserts credentials are tightened to 0600. Next review must trace concrete callback file writers before considering any umask change; avoid a speculative process-wide permission regression.

---

## 2026-11-25 — Exact-head status follow-up

- On e487703a3163f9bb7bf10fe09d4e389602ce34d3, Process lifecycle fail-closed #45569803612 and NVRAM transaction boundary audit #45569803507 both completed PASS. XY-C1 #45569803520 remains IN_PROGRESS in Build XY-C1; TRX and artifact upload are not yet confirmed.
- Documentation checkpoint dbac68aff56555f92369f9d224f1e894fbe40322 triggered AI handoff documentation contract #45570435066, which completed PASS. The code-bearing source SHA remains e487703a3163f9bb7bf10fe09d4e389602ce34d3.

---
## 2026-11-25 — Harden WAN gateway parsing, MiniUPnP framing, SmartDNS refresh and printer TCP I/O

- WAN ARP now rejects malformed non-empty gateway NVRAM instead of collapsing it to INADDR_ANY; empty/0.0.0.0 retains no-gateway semantics. Exact gateway regression #45565621317 PASS on 583576a.
- MiniUPnP strictly parses one bounded Content-Length, rejects unsupported Transfer-Encoding and duplicate lengths, caps headers/body and realloc growth, and sends a valid HTTP/1.0 431 before request-version parsing. Exact GNU89 sanitizer/source-contract #45566912855 PASS on 019f005.
- SmartDNS Start_AD preserves last-known-good adblock output on failed download, empty/invalid config and refuses symlink targets; test covers successful replacement and fresh-install fallback. DNSMasq lifecycle #45568556249 PASS on 1817dd6.
- U2EC uses full-read/full-send SIGPIPE-safe framing; validates outer frame, IRP BufferSize against actually received payload, URB transfer lengths and reply buffer bounds. Exact GNU89 ASan/UBSan/source-contract #45569202180 PASS on e5d039b.
- p910nd bidirectional copy validates both descriptors against FD_SETSIZE before FD_SET; Makefile rebuild dependencies include both policy headers. Exact GNU89 ASan/UBSan socketpair/source-contract #45569803535 PASS on e487703.
- Current main at this checkpoint: e487703a3163f9bb7bf10fe09d4e389602ce34d3. XY-C1 #45569803520 (Build active; TRX/artifact not yet confirmed), Process fail-closed #45569803612 and NVRAM audit #45569803507 were still in progress. Process inventory #45569803552 PASS. No product L3 or physical L4 claim.

---
## 2026-11-25 — Repair WAN ARP workflow PR path filter

- Exact-head WAN gateway ARP #45565097044 on `b35e958`: all five GNU89 regression binaries PASS (socket setup, probe error policy, gateway address parser, checked NVRAM copy, signal deferral). The production source/RC contract reached its final path-filter audit, where `trunk/user/rc/src/tests/detect_wan_gateway_address_policy.c` was present in push paths but omitted from pull_request paths. Added it once to PR paths; no source behavior changed. Exact-head rerun pending.
- AI handoff #45565097043 PASS. On source baseline `570fc3e`, VSFTPD #45564343508, NVRAM #45564343650, AI docs #45564343731 and Process inventory #45564343830 PASS; XY-C1 #45564343590 was still building.

---

## 2026-11-25 — Recheck ACL metadata link count after reading

- Follow-up race closure for the hard-link ACL metadata fix: `read_file_to_buffer_at()` already re-fstat'd after consuming the bounded file to catch inode/size changes, but did not recheck link count. A hard link created after initial open could therefore evade the new open-time `st_nlink == 1` check. The post-read identity gate now also requires `final_stat.st_nlink == 1`, returning transient `EAGAIN` if an alias appeared during the read; workflow source contract pins the check.
- Parent VSFTPD run #45564032851 on `0abdaee` PASS, including host ACL policy regression and configuration/source contract. This follow-up's exact-head run is pending.

---

## 2026-11-25 — Correct XY-C1 path allowlist duplicate

- XY-C1 run #45563653212 on `7492a52` failed before firmware-input detection: `Verify Samba shared-header staging contract` reported two entries for the XY-C1 workflow in its push path list. The workflow already contained its self-trigger lower in the long path allowlist; the previous commit accidentally added a duplicate at the top while adding the AliyunDrive ROMFS pattern. Removed only the duplicate, retained the existing self-trigger once and kept `trunk/user/aliyundrive-webdav/**` exactly once. Re-run on this correction SHA should proceed to production input detection and Build XY-C1.
- XY-C1 run #45564032922 on `0abdaee` was also triggered while the duplicate was present; inspect its result as historical failure, then verify this correction SHA.
- VSFTPD ACL metadata single-link source/test change remains in `0abdaee`; its exact-head regression is pending.

---

## 2026-11-25 — Reject multiply-linked FTP ACL metadata

- Targeted VSFTPD review found that ordinary share-file opens reject hard-linked inodes (`st_nlink != 1`) to prevent cross-share aliases, while the bounded ACL metadata reader accepted any regular file regardless of link count. Since the ACL metadata file is an authorization source, a hard link can alias content writable through another path or local service. `ftp_acl_path_open_bounded_regular_file_at()` now rejects link counts other than one with `EACCES`; host regression covers both pathname and descriptor-relative readers, and the exact-head workflow source contract pins the rule.
- Exact-head host regression pending. Normal single-link ACL config behavior is unchanged.

---

## 2026-11-25 — Add missing XY-C1 trigger for AliyunDrive ROMFS package

- AliyunDrive exact-head lifecycle #45563131218 and AI handoff #45563131174 on `fdba8a4` PASS, including shell syntax, QEMU MIPSel CLI, host stop regression, early private-lock provisioning and final source/RC integration contracts.
- Firmware verification gap: XY-C1 workflow used a push path allowlist which omitted `trunk/user/aliyundrive-webdav/**`; edits to this ROMFS package therefore did not trigger a build. Added both the workflow file itself and the complete add-on package to its push paths. This push should trigger an exact-head XY-C1 build of the exact-PID launcher. No build result is claimed until its Build XY-C1, TRX and artifact steps complete.
- Older XY-C1 run #45562193492 on unrelated parent `63da84a` finished success (Build XY-C1, TRX, artifact upload), but predates the AliyunDrive production change and is not a substitute for this build.

---

## 2026-11-25 — Pin sleep-fixture counter reset without escaped-newline ambiguity

- Exact-head AliyunDrive lifecycle #45562928673 on `5544b95`: shell syntax, QEMU MIPSel CLI, stop lifecycle regression and early private-lock provisioning all PASS; final Python source contract alone failed because its fixture assertion searched for literal backslash-n separators instead of actual line breaks. The test fixture itself did reset both signal counters correctly. THIS commit scopes the checker to the `TEST_SLEEP_FAIL=1` fixture and checks the two reset tokens without a newline-encoding dependency. AI docs #45562928669 PASS; Process inventory #45562928835 was still running at last poll.

---

## 2026-11-25 — Reset exact-PID signal counter in sleep-failure fixture

- Exact-head AliyunDrive lifecycle #45562716442 on `3a08741`: shell syntax and QEMU CLI checks PASS; lifecycle regression reached the stop cases and failed because the newly inserted foreign-UID-after-TERM case left `TEST_TARGETED_SIGNAL_COUNT=1`, while the following sleep-failure fixture did not reset it before expecting exactly one TERM signal. This was a test-fixture isolation defect, not extra production signalling. Reset both target/unexpected counters before sleep failure and pin that reset in the source checker.
- Exact-head AI handoff #45562716449 and Process inventory #45562716611 on `3a08741` PASS. No AliyunDrive runtime/lifecycle PASS is claimed until the corrected exact-head rerun.

---

## 2026-11-25 — Fix AliyunDrive PID-list shell pattern

- Exact-head AliyunDrive lifecycle #45562524959 on `a2715db` failed at the first `sh -n`: the PID whitespace character class had two literal backslashes, causing the space to split the shell case pattern. Corrected it to one escaped-space character class. This was a syntax-only failure before QEMU or lifecycle assertions ran; rerun all lifecycle steps on this correction SHA.
- Exact-head AI handoff #45562525000 and Process inventory #45562525082 on `a2715db` PASS. Parent USB hotplug #45561746653 PASS. Parent XY-C1 validation #45561746460 PASS as a host-only job, but the production-input detector skipped Build XY-C1, TRX verification and artifact upload; no firmware build is claimed.

---

## 2026-11-24 状态增量：AliyunDrive stop 改为逐 PID 身份复核后发信号

- `ald_kill_unlocked()` 原先先验证 `pidof` 候选的 UID 和可执行文件 inode，却随后调用 `killall`；后者会重新按名称枚举，可能把验证后新出现的同名异用户进程也 TERM/KILL，令 caller-UID 检查失效。现在仅在完整 inventory 校验成功后发布 PID 集合；TERM/KILL 只对快照中的精确 PID 调用 `kill`，并在每次信号前重新扫描 UID/executable identity。UNKNOWN 阻止信号和 KILL 升级；新增 foreign-UID-after-TERM 回归。Linux 4.4 无 pidfd，最终 procfs 校验与 `kill(2)` 之间仍有极窄 PID 重用窗口；不再使用 name-wide `killall`。Exact-head Shell/QEMU/lifecycle/source-contract Actions 待验证。

---

## 2026-11-25 — Initialize the udhcpc select-wait timestamp

- The DHCP client event loop declared `timestamp_before_wait` by initializing it from itself (`unsigned timestamp_before_wait = timestamp_before_wait;`). That reads an indeterminate automatic object and invokes undefined behavior, despite the comment claiming it only suppresses an uninitialized warning. The variable is refreshed from `monotonic_sec()` immediately before every blocking `select()`; waits skipped because the timeout is already exhausted do not consume the value. Initialize it to zero, retain the pre-`select()` refresh, and add an exact-source regression assertion against reintroducing self-initialization. The udhcpc workflow compiles policy tests under GNU89/ASan/UBSan; the firmware-input detector must also select XY-C1 for this production DHCP source change. No local MIPS/uClibc or hardware result is claimed.

---

## 2026-11-25 — Align USB storage source contract with FTP reconciliation policy

- USB storage hotplug run #45561164857 failed only at the final `scripts/check_storage_safe_remove.py` source contract. All preceding GNU89/ASan regressions, BusyBox ash Optware tests, disk inventory, eject inventory, operation locks and teardown contracts passed. The stale assertion required `storage_service_process_presence_changed()` to appear directly inside `control_ftp_fw()`; the FTP fix intentionally centralizes presence-transition plus confirmed-absence logic in `ftp_firewall_reconcile_needed()`. Update the checker to verify the production helper call, both policy predicates and the absent→absent regression marker. No production behavior changes in this follow-up; exact-head storage workflow rerun required.

---

## 2026-11-24 状态增量：修正 Inadyn restart contract 的换行字面量

- Exact-head Inadyn workflow `#45561380431` 的 GNU89 + ASan/UBSan 三个既有回归和新接入的 DDNS restart selector 回归全部 PASS；最终源码合同因 Python 字符串把 C 函数声明换行误写成字面量反斜杠+n 而失败。THIS COMMIT 将声明检查拆成稳定 token 检查，不放宽 0/1 状态边界和回归要求；Inadyn exact-head rerun pending。同期 AI docs `#45561380495`、Process inventory `#45561380463`、DNSMasq `#45561380421` PASS；Process umbrella、NVRAM audit、XY-C1 当时仍在运行。

---

## 2026-11-24 状态增量：Inadyn DDNS restart selector 边界测试接入

- `rc_ddns_restart_run()` 对外接收 AP-mode 与 DDNS enabled 两个整数状态，却把任意非零值都当作 true；未知/损坏的 selector 会被错误地解释为 stop 或 start 请求。现在仅接受 0/1，非法输入返回 `EINVAL` 且不调用服务回调。补齐 -1/2 双字段回归，并将原先未执行的 `ddns_restart_policy.c` 纳入 Inadyn exact-head GNU89 + ASan/UBSan workflow 与源码合同，push/PR 路径过滤均覆盖策略头和测试文件。

---

## 2026-11-25 — Reconcile FTP firewall rules after an absent-to-absent stop

- `stop_ftp()` and `restart_ftpd()` only called `restart_firewall()` when daemon presence changed. If vsftpd had crashed and was already absent before an explicit stop/restart, `ftp_stop_should_reconcile_firewall()` allowed the reconciliation path, but `control_ftp_fw()` returned early for absent→absent. Stale FTP NAT/UPnP rules could therefore survive a confirmed stopped state. Add a shared policy that refreshes on presence changes and whenever the final inventory positively proves absence. Apply it in `control_ftp_fw()`; test absent→absent, running→absent, absent→running, unknown transitions and stable running. The existing storage-stop stale-rule path is unchanged. Exact-head VSFTPD lifecycle/source contracts required; no target hardware result claimed.

---

## 2026-11-25 — Bound MT7628 switch CLI page selector to its three-bit field

- `mii_mgr -L/-G` accepted page values up to `UINT_MAX >> 12`, although register 31 encodes the page in bits 12–14 and reserves bit 15 for the local/global selector. Values 8–15 therefore overlapped the selector bit; larger values could exceed the 16-bit Clause 22 payload and be rejected by the kernel after the CLI had accepted them. Limit parsing to 0–7. Extend the host regression to assert valid maximum local/global encodings (`0xf000` and `0x7000`) and reject page 8 for both modes. Add an MT7628-specific compile and ASan/UBSan execution so this previously unbuilt conditional parser path is exercised alongside MT7621. Exact-head switch CLI workflow required; no board hardware result claimed.

---

## 2026-11-25 — Restore explicit end-of-session checkpoint instruction

- Exact-head AI documentation check #45559337773 validated the roadmap's byte cap after the previous correction, then failed because the current handoff's cross-session mandate omitted the required literal `Before context exhaustion`. Restore the explicit instruction to finish the safe slice and checkpoint the next concrete action before context exhaustion. No source code or test behavior changes.

---

## 2026-11-25 — Keep the validated roadmap below the AI handoff byte cap

- The AliyunDrive source and lifecycle checks passed on `ed40cf1` (#45559141117), but the AI documentation contract rejected `REFACTOR_ROADMAP.md` at 131,145 UTF-8 bytes, 73 bytes above its 131,072-byte hard limit. The prior preflight counted JavaScript characters rather than encoded bytes. Compress the AliyunDrive status row without dropping its process-owner boundary, regression coverage or remaining validation limits. Revalidate using UTF-8 byte length and exact-head documentation Actions.

---

## 2026-11-25 — Align AliyunDrive UID fixture contract with the mode loop

- Exact-head AliyunDrive workflow #45558898981 passed shell syntax, bundled MIPSel QEMU CLI compatibility, the lifecycle host regression (including foreign/unreadable/malformed UID cases), and early private-lock provisioning. Its final Python contract failed because it looked for three literal variable assignments, while the test correctly sets the mode through `for TEST_PROCESS_UID_MODE in foreign inaccessible malformed; do`. Replace the stale marker with the actual loop contract. Production launcher and regression behavior are unchanged; rerun the exact-head lifecycle workflow.

---

## 2026-11-25 — Bind AliyunDrive process identity to the lifecycle caller

- The AliyunDrive launcher matched each `pidof` candidate to the managed binary's executable inode, but did not verify the PID owner. Since the binary is executable by other accounts, a same-binary process launched by another user could be mistaken for the managed service; name-wide stop could then signal it. Record the lifecycle caller UID and require every candidate's `/proc/PID` owner to match. Foreign, inaccessible or malformed UID data returns UNKNOWN. The host regression retains the matching executable inode while varying only PID ownership data, and asserts no signal or duplicate launch. Exact-head AliyunDrive lifecycle, Process and XY-C1 checks required; no local runtime result claimed.

---

## 2026-11-25 — Correct ZeroTier route mock newline format

- A post-commit fixture review found the new route-output override used two backslashes before `n` in its `printf` format, which would append the literal characters `\n` rather than a record newline. Correct the test double to emit the same line framing as `ip route show`; production route matching is unchanged.

---

## 2026-11-25 — Keep ZeroTier shutdown fail-closed for conflicting and multipath routes

- Found a route-cleanup ambiguity in `zero_route_entry_matches()`: it returned the same status for “destination absent” and “destination exists but its gateway/interface differs.” `zero_route()` treats status 1 as an idempotent absence, so a route still installed for a configured destination could be left behind while `stop_zero()` removed ZeroTier persistent identity and retry state. The parser also retained only the last `via/dev` pair on a multipath line, potentially missing the configured ZeroTier nexthop when another nexthop followed it.
- Give the matcher a three-state contract: exact nexthop found (0), destination truly absent (1), conflicting/unrecognized destination route (2); inspect each `via ... dev ...` pair in multipath output. Existing caller logic already treats any status other than 1 as an error, so conflict now preserves recovery state. Extend end-to-end stop tests for a changed gateway and for the configured first multipath hop, including verified deletion. Exact-head ZeroTier Actions required; no local shell result claimed.

---

## 2026-11-25 — Complete the rstats procfs inventory before authorizing storage mutation

- Confirmed a fail-open ordering defect in `func_rstats_presence()`: it returned “present” immediately when it encountered an `rstats` PID, without validating the remainder of `/proc`. A later unreadable or malformed numeric process entry was therefore never observed; restore/erase could proceed to signal the daemon even though the process inventory was UNKNOWN. The existing regression put the malformed PID before the daemon and missed this ordering.
- Record an `rstats` match but continue the entire numeric procfs scan; return PRESENT only after every live entry has a valid stat record, and preserve UNKNOWN on any later read/parse failure. Move the malformed fixture to PID 999 so it sorts after the valid rstats PID 321, assert no kill/rename is performed, and pin the no-early-return scan ordering in the source contract. Exact-head Storage hotplug and XY-C1 validation required; no local shell or firmware result claimed.

---

## 2026-11-25 — Reject reserved RFC1035 extended label types

- `dname_dec()` recognized only the `11xxxxxx` compression-pointer class, then treated both reserved `01xxxxxx` and `10xxxxxx` label encodings as ordinary length octets. A buffer with 64 or 128 following bytes could therefore be accepted as a domain label even though those extended-label classes are unsupported. Reject nonzero top-bit classes other than compression pointers before label processing. Add sanitizer cases for both reserved encodings and retain acceptance of a maximum 63-byte ordinary label. Exact-head DHCP decoder, XY-C1 and AI checks are triggered by the atomic source/test/log/handoff commit; no local compile or target/hardware result is claimed.

---

## 2026-11-25 — Include DHCP domain codec in XY-C1 firmware validation paths

- The new decoder sanitizer workflow passed on `66efb19`, but review of the XY-C1 workflow exposed a production-validation blind spot: `networking/udhcp/domain_codec.c` was missing from the push path filter, so edits to this shipped BusyBox source did not trigger the MIPS firmware build/TRX/artifact job. Add the source to the path filter and pin the one-entry filter plus production classification in `test_xy_c1_firmware_inputs.py`. The workflow's own path is already included, so this commit re-triggers XY-C1 against the complete current tree.

---

## 2026-11-25 — Initialize domain decoder return pointer deterministically

- Remove the legacy self-initialization `char *ret = ret`, which reads an indeterminate automatic pointer before the decoder assigns its successful result. Initialize it to `NULL` and add a null-buffer rejection case alongside zero/negative length checks.

---

## 2026-11-25 — Make domain-codec sanitizer harness leak-clean

- The domain-codec sanitizer progressed past compilation after the validated signed/unsigned input-length change, then LeakSanitizer exposed allocations intentionally printed but never freed by the legacy demonstration `main()`. Replace print-only demos with checked decode/encode round-trip helpers that release every buffer; retain valid no-compression, suffix-compression and whole-name-compression coverage.

---

## 2026-11-25 — Cover zero and negative domain input lengths

- Extend the domain-decoder regression to exercise the new API guard with zero and negative lengths as well as malformed wire payloads. The input pointer remains valid in these cases, isolating length validation.

---

## 2026-11-25 — Validate decoder length and use unsigned-safe bounds

- The first domain-codec workflow exposed three pre-existing signed/unsigned comparison warnings in the decoder's unsigned cursor versus signed length bounds checks (the regression workflow treats warnings as errors). Harden the API boundary: reject null inputs and non-positive lengths, then compare cursor offsets against the validated unsigned length. This removes the warnings without weakening compiler diagnostics and makes negative caller lengths fail closed.

---

## 2026-11-25 — Reset domain completion for every new label

- Review of the first sanitizer slice found the per-name completion flag also must be cleared whenever a new label begins after a previously terminated name. Without that reset, a valid first search-domain entry could mask a truncated final entry in the same option. Reset the state on every label and add a two-entry regression (valid first name, unterminated second name); normalize diagnostic newlines in the embedded test harness.

---

## 2026-11-25 — Reject truncated and forward RFC1035 domain compression

- `dname_dec()` previously accepted a domain that reached the end of its option buffer without an encoded root terminator. In particular, a label followed by a compression pointer whose target was beyond the option length could be returned as a valid but truncated name. The decoder also followed forward/self-referential compression targets even though RFC1035 pointers are backward references; the existing jump-depth cap prevented unbounded loops but did not enforce the wire-format rule.
- Track explicit completion of each expanded name and reject any path that reaches the buffer end without a root terminator. Require each compression target to precede the pointer. Added sanitizer-backed cases for valid backward compression, truncated labels, out-of-range targets and forward targets. Exact SHA workflow run pending; no local compiler result claimed.

---

## 2026-11-25 — Anchor DHCPv6 interface contract at the function definition

- On exact SHA `14e4051`, the GNU89 ASan/UBSan policy binary passed, but the DHCPv6 source checker failed before its new assertion: its generic function parser matched the `if6init()` forward declaration, then encountered the intervening enum brace instead of the function body.
- Anchor the check at the second occurrence (the definition) before using the existing body parser. This is a checker-only correction; the production diagnostic fix remains unchanged. Exact-head rerun required.

---

## 2026-11-25 — Use initialized interface name in DHCPv6 startup failure

- The DHCPv6 client already terminates when `if_nametoindex()` returns zero, but the fatal diagnostic formatted `ifp->ifname` before that pointer was initialized (the assignment occurs after the guard). This makes the error path depend on an uninitialized pointer and can emit a bogus interface name.
- Keep the existing fail-fast behavior and report the original `ifname` argument instead. Extend the DHCPv6 source contract to pin the read → zero-index guard → fatal diagnostic → pointer assignment order. Exact-head DHCPv6 workflow required; no local test is claimed.

---

## 2026-11-25 — Bound optional SmartDNS adblock downloads

- `Start_AD` already limited connection establishment to 10 seconds but left transfer time unbounded; a user-configured or stalled adblock endpoint could therefore hold SmartDNS startup indefinitely. Add a 30-second per-attempt maximum and reduce retries from three to one, bounding worst-case transfer wait while keeping one transient retry.
- Add an exact-source contract for connection timeout, transfer timeout and retry count. SmartDNS lifecycle and source contract rerun required; this is independent of the in-progress XY-C1 validation on the previous product-source SHA.

---

## 2026-11-25 — Align Redsocks checker with split function braces

- Exact Redsocks run on `dd12814` passed the full shell regression matrix, including invalid-address preservation, duplicate draining, check/delete error propagation and partial rollback. The source-contract step then crashed with `ValueError` because it searched for literal `func_clean(){`, while production uses `func_clean()` followed by a newline and `{`.
- Replace the brittle substring lookup with a line-anchored regex accepting whitespace before the opening brace. This preserves the start-function boundary and all semantic assertions; exact-head rerun required.

---

## 2026-11-25 — Isolate Redsocks cleanup-failure trace assertion

- Exact Redsocks run #45555165127 on `437fd9e` passed invalid IPv4 rejection, duplicate-draining/idempotent reapply, and iptables check-error handling. The delete-error test then falsely detected `-N REDSOCKS` from the earlier successful setup because it reused the same diagnostic log. Clear only the command trace before injecting the delete failure; keep the existing rules and chain state intact so the assertion proves no new chain is created after cleanup failure.
- Exact-head regression rerun required. Production source unchanged; no local test result is claimed.

## 2026-11-25 — Correct Redsocks mock rule keys after chain parsing

- Exact `7cfd647` regression still failed because the stateful mock stored `table|chain rule` using an explicit `$chain` prefix while `$rule="$*"` already contains the chain token. This left an invalid doubled key and the test observed zero PREROUTING jumps. Updated check/delete/append and append-fault matching to use the complete original argv once (`table|$rule`).
- This correction is isolated to `scripts/tests/redsocks_lifecycle.sh`; production policy remains unchanged. Exact-head Redsocks CI must now validate the full test matrix.

## 2026-11-25 — Restrict downloaded native SmartDNS filters to address rules

- The native SmartDNS path previously copied the entire downloaded file whenever it found one `address=` line. A mixed or compromised list could therefore inject unrelated daemon settings such as `server`, `bind` or nested `conf-file` directives into the active SmartDNS config.
- Whitelist native filter content to address directives, comments and blank lines; reject any other directive and preserve the previous output on failure. Add mixed-content regression and exact-source contract. Exact SmartDNS lifecycle, DNSMasq regression, AI docs and any source-triggered firmware checks are pending; no local test or firmware result is claimed.

---

## 2026-11-25 — Correct Redsocks mock newline escaping

- Exact review of the preceding fixture correction found its generated `printf` format strings contained two literal backslashes before `n`, rather than the shell's single escape. That would store the rule key with a literal backslash and letters `n`, not terminate the record, so the mock still could not validate its own rule table.
- Normalize both command trace and rule storage to a single `\n` format escape. Keep complete `table|chain rule` identity; production source unchanged.

---

## 2026-11-25 — Preserve complete chain identity in Redsocks firewall mock

- The next fixture review found a second inconsistency after clearing the escaped CHECK/DELETE controls: mocked `-C/-D` already keyed rules as `table|chain rule`, but `-A` still omitted the chain. This made valid additions invisible to the mock's own lookup/count/flush logic. Updated `-A` storage and command traces to retain chain identity consistently.
- This is regression-fixture-only; no production code change. Confirm exact Redsocks run after this correction.

---

## 2026-11-25 — Fix Redsocks mock's escaped failure controls and chain keys

- The diagnostic exact-head Redsocks run confirmed the initial failure was status 4 from the mock's very first `iptables -C PREROUTING`: a backslash left the CHECK failure-injection expression literal, so the mock always returned an error. The adjacent DELETE failure control had the same escaping defect.
- Removed the unintended escapes from both controls and made mocked `-A` / trace output retain the chain as well as the table and rule. The full rule key now agrees with the existing `-C/-D/-F/-X` model and the test's duplicate-count assertions. Production source remains unchanged; rerun required.

---

## 2026-11-25 — Correct Redsocks firewall regression mock command handling

- Exact Redsocks Actions #45554742701 on `e51171f` exposed a test-fixture defect: the firewall mock escaped its failure-injection variable, so every `iptables -C` returned synthetic status 4; it also omitted the chain name from stored rule keys, conflicting with flush/count/rollback expectations.
- Unescaped the mock's test-only failure controls and made `-C/-D/-A` use the complete `table|chain rule` identity. This changes only regression fixture behavior; production firewall policy is unchanged. AI handoff docs #45554742775 PASS; exact Redsocks rerun required.

---

## 2026-11-25 — Diagnose Redsocks initial NAT apply regression

- Exact Redsocks Actions run #45554360655 on `87a3eea` passed shell syntax and rejected the octal REMOTE_IP correctly, then failed the first valid firewall apply. Added the mock's command trace and exact return status to the regression failure path so the failed iptables operation can be identified without weakening assertions.
- The 100-record log rotation is committed in `614c30609b212a0fa9ad6db23a9f8bf1573020d3`; root log now has 20 records and archive part-034 preserves 100 oldest records byte-for-byte.

## 2026-11-24 — Redsocks NAT policy idempotence and failure propagation

- func_clean previously parsed human-readable iptables -L --line-numbers output by stripping every digit and deleting a guessed line number; cleanup statuses were ignored. func_iptables also ignored -N/-A failures and appended duplicate chain rules/jumps on reapply.
- Replaced line-number parsing with exact iptables -C/-D draining (64-delete bound), checked -S/-F/-X, and checked append helpers with rollback for partial policy publication. REMOTE_IP is validated as canonical decimal IPv4 before the old firewall is touched; invalid/octal aliases fail closed. ipset population and zone downloads now propagate errors rather than publishing the completion sentinel after failed population.
- Extended the existing Redsocks lifecycle regression and source contract with duplicate-jump cleanup, idempotent reapply, invalid IPv4 preservation, check/delete error propagation, and partial-append rollback. Exact-head Redsocks Actions pending; no local shell or target build is claimed.

## 2026-11-24 — Update SmartDNS startup rollback count contract

- DNSMasq run #45553387354 passed the shell regressions and revised stop-order assertions, then found one stale source-contract count: checked firewall setup adds a new failure branch to the existing seven SmartDNS startup rollback sites. Updated the exact expected count to eight.
- AI docs #45553386628 passed for the parent SHA. DNSMasq and XY-C1 verification remain pending on the corrected source-contract SHA.

## 2026-11-24 — Align the DNSMasq source contract with safe SmartDNS stop ordering

- Exact-head DNSMasq run #45553242639 confirmed the existing SmartDNS config-preservation fixture now extracts and runs with both firewall helpers, but its source checker still mandated the old unsafe order (stop daemon, then delete NAT redirects). Updated the checker to require redirect cleanup before daemon stop and Adbyby retargeting.
- The independent SmartDNS lifecycle checker and shell cases remain in place; exact-head rerun required. No local test or firmware result is claimed.

## 2026-11-24 — Update the existing DNSMasq preservation fixture for checked firewall helpers

- Exact-head DNSMasq run #45553050279 confirmed all seven new firewall-policy shell cases passed, then the older `dnsmasq_config_preservation.sh` fixture failed because it extracted `Change_iptable()` without the newly required helper functions. Updated its extraction block to source add, delete and dispatch helpers together.
- Extended the source contract to reject unchecked direct `iptables`/`ip6tables` invocations inside the dispatcher. Prior exact-head AI docs and process lifecycle checks passed; DNSMasq rerun required. No local shell or firmware result is claimed.

## 2026-11-24 — Keep SmartDNS alive until redirect cleanup succeeds

- Exact-head DNSMasq job #45552869102 ran all seven new firewall shell cases successfully, then the Python source contract failed on an over-broad assertion that confused helper-wrapped `iptables -A` arguments with raw unchecked calls. Removed that invalid assertion and the unrelated check against the older process test fixture.
- Tightened stop safety: NAT redirect cleanup now runs before the SmartDNS daemon is stopped. A check/delete error therefore returns while the daemon and saved resolver state are still available; successful cleanup permits daemon shutdown and later state-file removal. The source contract now pins this ordering.
- AI docs #45552869211 passed for parent source SHA `b42e026`; exact-head test reruns required on this correction. No local shell or XY build is claimed.

## 2026-11-24 — SmartDNS NAT redirect status and retry-state integrity

- `Change_iptable()` previously ignored all IPv4/IPv6 redirect add/delete results. Startup could continue without required DNS redirects; stop could report success and delete saved `smartdns_conf.ini` state while stale NAT redirects remained.
- Added checked add and idempotent delete helpers. Deletion uses bundled iptables 1.8.7 `-C` to distinguish absent rules from check errors; present rules are deleted, absent rules are harmless, other errors propagate. Stop attempts all owned rules and preserves the first failure. Startup sends add failures through existing rollback; stop preserves saved state for retry until firewall cleanup succeeds.
- Added shell regression for absent-rule idempotence, best-effort IPv4/IPv6 deletion, delete/check errors, partial and successful add, reset failure and unknown actions. DNSMasq lifecycle workflow runs the test and source contract. Exact-head Actions pending; no local shell run or firmware build is claimed.

## 2026-11-24 — Expand the PPPoE-MWAN LAN mask fixture variable

- Exact WAN run #45551840669 on `f8d0b7a` ran the repaired /16 tests but stopped earlier in the NVRAM failure matrix: the br0 mock emitted `Mask:${LAN_MASK...}` literally because the echo string remained single-quoted. Changed the mock to double quotes so its configurable mask is expanded; the production parser and firewall rules are unchanged.
- Exact-head WAN route / AI docs / XY-C1 checks must be observed on the corrected test SHA. No local shell run or firmware build is claimed.

## 2026-11-24 — Correct the PPPoE-MWAN LAN-prefix fixture splice

- The preceding repair attempt remained malformed: the remote test file still had duplicated fragments of the normal-run section, so the exact-head WAN rerun could not parse it. Rebuilt the shell regression directly from the intact `0b85e2e` baseline and inserted the two prefix cases by line index, not substring replacement.
- The /16 case now verifies both mangle and NAT command arguments; the non-contiguous mask case verifies rejection before firewall or route changes. The AI docs roadmap-size correction remains in effect.
- Exact-head WAN route / AI docs / XY-C1 reruns are required; no local shell or firmware result is claimed.

## 2026-11-24 — Repair PPPoE-MWAN prefix regression fixture and roadmap budget

- The first exact-head WAN rerun found a fixture-edit error: the /16 and non-contiguous mask assertions were inserted into the middle of the existing shell test, leaving invalid Bash. Restored the intact pre-slice test and inserted both cases at the unique normal-run boundary; the br0 mock now supplies a configurable `LAN_MASK`.
- AI docs validation also reported `REFACTOR_ROADMAP.md` at 131,322 UTF-8 bytes, 250 bytes above its 128-KiB hard limit. Condensed a superseded 2026-11-21 historical checkpoint while preserving the live module dashboard, priority queue and pointers to the canonical dev-log archive.
- Exact-head WAN route and AI docs reruns are required. No local shell test is claimed.

## 2026-11-24 — Derive PPPoE-MWAN source scope from the active LAN netmask

- The web LAN settings expose configurable `lan_netmask`, but `detect.sh` hard-coded `$lanip/24` in both the mangle source match and NAT masquerade rule. On /16 or /25 LANs the policy therefore omitted valid clients or matched an unintended range.
- The detector now extracts the live bridge mask from `ifconfig`, validates that its IPv4 bits are a contiguous prefix, and uses the resulting CIDR in both rules. Regression coverage checks that a /16 setting reaches both mangle and NAT argv and that a non-contiguous mask fails before existing rules or routes are changed.
- Exact-head WAN-route, XY-C1 Build/TRX/artifact and AI-docs checks are pending; no local shell or firmware result is claimed.

## 2026-11-24 — Reject octal-ambiguous PPPoE-MWAN IPv4 inputs

- `detect.sh` accepted multi-digit IPv4 octets beginning with zero, but the bundled iptables 1.8.7 `xtables_numeric_to_ipaddr()` parser uses `xtables_strtoui(..., base=0, ...)`; an octet such as `010` is therefore interpreted as octal rather than decimal. A static source rule could target a different address than the NVRAM value represents.
- `mwan_ipv4_valid()` now accepts only canonical decimal octets and rejects leading-zero aliases before deleting existing firewall rules. The shell regression injects `010.0.0.7` as the static source and asserts fail-closed status, unchanged firewall/routes, and lock cleanup. The existing WAN-route workflow runs the shell regression and source contract; XY-C1 path filters already include the changed source and tests.
- Exact-head WAN-route, XY-C1 Build/TRX/artifact and AI-docs checks are pending. No local shell or firmware result is claimed.

## 2026-11-24 — Fix WAN source-contract helper ordering

- Exact WAN workflow #45550662855 failed with `NameError: body is not defined`: the new LAN udhcpc hostname source assertion called the function-body parser before its definition. Moved only that assertion below the helper definition; the production hostname guard and GNU89 regression remain unchanged. Exact-head WAN rerun required.

## 2026-11-24 — Fail closed before starting LAN udhcpc without a valid hostname

- `start_udhcpc_lan()` passed `get_our_hostname()` directly into the `-H` argv slot. When the checked NVRAM lookup failed, the helper returned NULL; that NULL terminated argv immediately after `-H`, so udhcpc was still spawned with a missing argument and the later readiness timeout obscured the original NVRAM error.
- Added a pre-spawn policy that preserves lookup errno (or EIO when absent), rejects empty/invalid hostname values, and returns before calling the launcher. GNU89 ASan/UBSan boundary tests and a WAN source-order contract cover lookup errors, missing/invalid values and valid names. Exact-head WAN/Process/NVRAM/XY-C1 checks pending; no local compile or firmware build is claimed.

## 2026-11-24 — Repair SmartDNS DNSMasq source-checker initialization order

- Exact DNSMasq run #45549996147 on `5d713067` passed all SmartDNS shell regressions, then the Python integration contract failed with `NameError: stop is not defined`: it inspected the `Stop_smartdns` body before assigning it. Moved the body extraction before its first use; no production source changed. Exact-head rerun is required.

## 2026-11-24 — Bind AP static-host fallback to its file-safety branch

- `create_hosts_lan()` omitted braces around the safe `/tmp/hosts.static` branch, so the `else` bound to the nested load-error check rather than the file-safety test. A successful copy with non-`ENOENT` stale `errno` could emit a false “skipping” warning. Added explicit braces and a DNSMasq workflow source-contract regression for the complete branch.
- The source-contract job is the verification path; no local firmware or target build is claimed.

## 2026-11-24 — Preflight PPPoE-MWAN settings before deleting working firewall rules

- The detector previously read 443 and static policy selectors only after cleaning prior rules; transient NVRAM errors could leave a partially rebuilt firewall policy. All selectors are now read and validated first, with static entries snapshotted in Bash arrays before cleanup.
- Added failure injection for each of the five optional selector classes and checks that no firewall deletion/addition or route mutation occurs and the lifecycle lock is released.
- The exact-owner cleanup regression had failed on #45548806225/#45549253398 because a malformed test rewrite left incomplete shell assertions. The subsequent test-repair commits restored the shell fixture; WAN workflow #45549413305 PASS on `a74d142c`, XY-C1 build still pending at that checkpoint. No local shell test or firmware build is claimed.

## 2026-11-24 — Scope PPPoE-MWAN firewall cleanup to explicitly owned rules

- `detect.sh` previously deleted any mangle/NAT rule whose serialized text contained the broad tokens `ctstate`, `br0` or `ppp`. This could remove unrelated QoS/firewall policies and the primary PPP WAN masquerade rule during ordinary multi-WAN reconciliation.
- Added the exact comment owner tag `Padavan-PPPoE-MWAN` to all seven rules generated by the detector. Cleanup parses the saved rule into argv and deletes it only when a `--comment` option exactly equals that tag; malformed snapshots still fail closed, and unrelated rules with matching substrings are left intact. Added fixtures for owned mangle/NAT rows and unrelated conntrack, bridge, quoted-comment and PPP NAT rows.
- Exact WAN-route/XY-C1 CI is pending; no local shell test or firmware build is claimed.

## 2026-11-24 — Reconcile stale FTP firewall rules during storage-stop

- `stop_stor_apps()` claimed it refreshes FTP firewall state even when external FTP is disabled, but only set its refresh flag when the pre-stop daemon inventory was non-ABSENT. If FTP was already absent (including stale rules left by a previous crash), and no other storage service was running, the refresh path was skipped.
- The stop now captures pre-stop presence and the final state from `stop_ftp_locked()`. It preserves the prior refresh behavior for pre-existing RUNNING/UNKNOWN daemons and additionally refreshes whenever final inventory confirms ABSENT; an initially absent daemon that becomes UNKNOWN/RUNNING does not authorize cleanup. The original stop error still aborts storage removal. Added GNU89 policy matrix coverage and source-order assertions. Exact-head VSFTPD/AI/XY-C1 checks pending; no local compile or target build is claimed.

## 2026-11-24 — Throttle zero-delay udhcpc leasefail and DECLINE retries

- BusyBox `getopt32` documents the `A+` argument as nonnegative (0..INT_MAX), so `-A 0` is valid. Exhausted discovery (`leasefail`) and successful DHCPDECLINE recovery previously assigned the value directly; zero could spin immediate DISCOVER/ARP/DECLINE cycles. The client now applies the existing one-second retry floor to leasefail and both successful DECLINE restart paths.
- Added GNU89 ASan/UBSan policy boundaries for zero/negative/positive retry values and strengthened the source contract to pin all three call sites. Exact-head UDHCP and XY-C1 checks are pending; no local compile is claimed.

## 2026-11-24 — Record IAPPD lifecycle test assertion diagnostics

- Main commit `5b7df92aaaad83eb9f903582982070da95c92aad` adds an ordinal to the IAPPD lifecycle status assertion so a failing fixture identifies its assertion index. Exact IAPPD run #45547735320 PASS; Process lifecycle inventory #45547735377 PASS; Process inventory umbrella #45547735444 and XY-C1 #45547735319 were still pending at checkpoint. This commit changes test diagnostics only.

