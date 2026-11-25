## 2026-11-25 — Trigger MTD fill regressions for pull requests

- Exact USB-storage run #45609688415 on `eed1e197` passed the host-shell and BusyBox ash fill regressions and surrounding source contracts, but the workflow checker correctly found that `mtd_storage_fill_safe_paths.sh` was listed under `push.paths` and omitted from `pull_request.paths`.
- Added the regression script to the pull-request path filter beside the other MTD lifecycle tests. PRs changing the test or MTD fill contract will now trigger the same storage stability workflow.
- Exact-head storage and AI handoff workflows must be green before this slice is complete.

---
## 2026-11-25 — Fix storage checker filter scope and rotate log window

- Exact USB-storage run #45608745837 on `fa7d05e5` passed shell syntax, host-shell and BusyBox ash MTD fill regressions, then the source checker raised `NameError` because its early MTD contract referenced `push_filters`/`pull_filters` before the workflow-filter variables were defined. Moved this path assertion into the existing later push/pull filter inventory.
- The AI handoff validator also reported 121 root log entries. Archived the 101 oldest complete records byte-for-byte, chronological oldest-to-newest, and retained the newest 20 before this correction entry. The pre-rotation root blob SHA and archive shard are indexed.
- Exact-head storage and AI handoff reruns required. Product source was unchanged in this checker/governance correction.

---
## 2026-11-25 — Align U2EC framing source contract with bounded reads

- Exact U2EC run #45608574671 on `0c505e0b` compiled the GNU89 policy test with ASan/UBSan and passed both complete-field and partial-field timeout regressions. The final source checker failed only because three pre-existing assertions still required the old unbounded `u2ec_recv_all()` call sites in `usb_connection()`.
- Updated the production-source contract and read-order assertion to require `u2ec_recv_all_timeout()` for both the fixed header and IRP body. No runtime source changed in this follow-up.
- Exact-head U2EC, storage, and XY-C1 checks remain required.

---
## 2026-11-25 — Fix literal backslash count in chnroute source contract

- Exact USB-storage run #45608329431 on `14ec3f27` passed runtime MTD fill regressions but the checker-only chnroute rewrite assertion failed: the test shell source contains one backslash before each escaped quote, while the Python raw-string expectation contained two.
- Corrected the raw-string expectation to one literal backslash. This calibrates only the source contract; MTD runtime source and fixtures are unchanged.
- Exact-head storage rerun required.

---
## 2026-11-25 — Bound U2EC client TCP field reads

- `usb_connection()` runs synchronously in the sole U2EC event loop. Its blocking `u2ec_recv_all()` calls could wait indefinitely after a peer sent only part of the fixed TCP header or advertised IRP body, preventing other printer clients, FIFO events and timeout processing from running.
- Added a 10-second absolute deadline per incoming TCP field. Readiness waits use the remaining deadline and nonblocking recv, so partial progress does not reset the total budget; timeout returns `ETIMEDOUT` and the existing close path removes the stalled connection. Header and IRP-body reads use the bounded helper; discovery worker reads retain their separate one-second socket timeout.
- Added successful complete-field and stalled-after-partial-field socketpair regressions plus source-contract checks for the main event-loop call sites. Exact U2EC host ASan/UBSan/source-contract and XY-C1 Build/TRX/artifact verification pending; no device result claimed.

---
## 2026-11-25 — Match quoted chnroute fixture rewrite literally

- Exact storage run #45607584405 on `0cc1703c` passed shell syntax, host-shell and BusyBox ash MTD fill regressions, checked caller graph and preceding assertions. The final static checker failed because the sed substitution in the shell fixture escapes its inner double quotes, while the Python source contract expected the unescaped form.
- Updated the contract to match the literal escaped shell text. No production source or runtime test behavior changed.
- Exact-head storage workflow rerun required.

---

## 2026-11-25 — Align chnroute source contract with checked directory helper

- Exact USB-storage run #45607146427 on `72f9d8fa` passed shell syntax validation and both host-shell and BusyBox ash MTD fill regressions, including injected write/chmod/rename failures. The static checker then failed on a stale assertion that still expected raw `mkdir -p`; production code now correctly uses `func_fill_mkdir "$dir_chnroute" 755` to validate the path and propagate directory-creation errors.
- Updated the source contract to require the checked helper while retaining assertions for archive extraction failure cleanup. No production source or test behavior changed.
- Exact-head storage workflow rerun required.

---

## 2026-11-25 — Correct MTD fill function source-contract delimiter

- Exact USB-storage run #45606836538 on `da508394` passed the MTD shell syntax and host/BusyBox runtime regressions, then the static checker failed because its Python string searched for the literal characters `\\ncase` rather than the actual function boundary `\n}\n\ncase "$1" in`.
- Corrected the delimiter so the checker slices the complete `func_fill()` body through its closing brace. Production MTD behavior and test fixtures are unchanged.
- Exact-head USB storage rerun is required; the earlier checker failure is a test-contract error, not a firmware/runtime failure.

---

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

