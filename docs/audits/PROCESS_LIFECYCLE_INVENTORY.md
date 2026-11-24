# P1 process-inventory and service-lifecycle source census

Date: 2026-11-14 (UTC+8)  
Repository: `Arlene206/padavan-4.4-2`  
Source/audit commit: `d0404d5e33d4239e19c92db9b9ab4fb7529cf26c`  
`trunk/user` tree: `daafdae4e97b401a428a0ac65d83a94ebeab0ff5`

## Result and reproducibility

The first repository-wide lexical census ran on GitHub Actions, not on a developer workstation.

- Workflow: [Process lifecycle source inventory](../.github/workflows/process-lifecycle-inventory.yml)
- Scanner: [`scripts/audit_process_lifecycle.py`](../../scripts/audit_process_lifecycle.py)
- Exact-head run: [43581078509 — SUCCESS](https://github.com/Arlene206/padavan-4.4-2/actions/runs/43581078509)
- Exact event/ref/SHA: `push` / `main` / `d0404d5e33d4239e19c92db9b9ab4fb7529cf26c`
- Artifact: [complete Markdown + JSON inventory](https://github.com/Arlene206/padavan-4.4-2/actions/runs/43581078509/artifacts/14108820679), artifact ID `14108820679`, 213,402 bytes, retained through 2026-12-14 09:33 UTC.

The workflow uses a depth-one partial clone with `--filter=blob:none`, enumerates the immutable `trunk/user` Git tree, then performs a no-cone sparse checkout of exact candidate paths. It does not check out the firmware toolchain, kernel source trees, bundled source archives or the complete 1+ GiB repository. The workflow pins and verifies the exact event SHA before scanning.

### Coverage

| Measure | Result |
|---|---:|
| Complete tracked Git blobs under `trunk/user` | 9,439 |
| Candidate source/script/build/test/config/executable files | 6,581 |
| Exact sparse paths (including the scanner itself) | 6,582 |
| Readable text candidates scanned | 6,193 |
| NUL-containing binary candidates skipped | 388 |
| Missing candidate files | 0 |
| Candidate read errors | 0 |
| Raw process API symbol occurrences | 193 |
| Broad lifecycle/process-management lexical candidates | 5,664 |

The candidate set includes C/C++ and headers, shell and embedded scripts, Makefiles/build inputs, tests, selected configuration/template formats, and tracked executable files. The 388 NUL-containing candidates were detected and skipped as binary; the remaining 2,858 tracked blobs were outside the candidate suffix/executable policy and were not read as source text. This is a complete path census plus a broad, bounded source-text census, not a claim that every blob in the 222 MB userland subtree was interpreted as text.

The scanner is lexical, not a compiler AST or interprocedural call graph. It can count references in comments or documentation and its role classifier is heuristic. Counts below are an index, not a substitute for checking each occurrence.

## API occurrence index

| Symbol | Raw hits | Definition candidates | Prototype/statement candidates | Call/macro candidates |
|---|---:|---:|---:|---:|
| `pids` | 79 | 1 | 1 | 77 |
| `pids_main` | 3 | 1 | 1 | 1 |
| `find_pid_by_name` | 12 | 2 | 0 | 10 |
| `procps_scan` | 16 | 2 | 0 | 14 |
| `alloc_procps_scan` | 5 | 2 | 0 | 3 |
| `free_procps_scan` | 6 | 2 | 0 | 4 |
| `kill_services` | 48 | 1 | 1 | 46 |
| `kill_pidfile` | 7 | 1 | 1 | 5 |
| `kill_pidfile_s` | 17 | 1 | 1 | 15 |

### Important implementation boundary: shared Padavan scanner vs BusyBox scanner

Do not merge same-named functions into one API inventory:

- Padavan shared implementation: `trunk/user/shared/src/pids.c`. Public Boolean wrappers `pids()` and `pids_main()` are declared in `trunk/user/shared/src/shutils.h`. Shared `find_pid_by_name()` and `procps_scan()` also live in this `pids.c`.
- Vendored BusyBox 1.24.x has its own `libbb/procps.c`, `libbb/find_pid_by_name.c`, declarations in `include/libbb.h`, and internal applet callers in `procps/kill.c`, `procps/pidof.c`, `procps/ps.c`, `procps/top.c`, `procps/pgrep.c`, `procps/lsof.c`, `procps/pstree.c`, `init/bootchartd.c`, and `init/halt.c`. These are a distinct BusyBox implementation/build surface. Audit their own safety and linkage separately; do not mechanically migrate them with the Padavan shared API.

## Padavan shared API consumers found

### `pids()`

The 77 lexical call candidates are concentrated in:

- HTTPD: `trunk/user/httpd/src/aidisk.c` (2) and `trunk/user/httpd/src/web_ex.c` (23). These include WebUI status hooks and service-state presentation.
- RC: `trunk/user/rc/src/common_ex.c`, `detect_internet.c`, `detect_link.c`, `detect_wan.c`, `gpio_btn.c`, `net.c`, `net_ppp.c`, `net_wan.c`, `services.c`, `services_ex.c`, `services_stor.c`, `usb_modem.c`, `vpn_client.c`, `vpn_server.c`, and `watchdog.c`.
- Public interface: `shutils.h:73`; implementation: `pids.c:483`.

High-impact examples:

- `rc/src/common_ex.c:1127`: service-lifecycle `is_running` adapter returns `pids()` directly.
- `rc/src/services_ex.c:333`: dnsmasq-running status; lines 832, 967, 1010, 1023, 1062 and 1621: Samba/WINS, miniupnpd and inadyn lifecycle checks.
- `rc/src/services.c:104,159,187,203,235,240,275,300,389,415,689,710`: infosvr/crond/networkmap/SSH and daemon start/restart postconditions.
- `rc/src/services_stor.c:114,230,889,948,1046,1047,1170,1239,1736,1927,2114,2218`: FTP/Samba/WSDD/media/download service state and restart handling.
- `httpd/src/web_ex.c:2514-2708`: proxy, DNS, SmartDNS, ZeroTier, DDNS, WebDAV and other UI status functions.
- `rc/src/watchdog.c:146,156,477,624,633`: wget/httpd/watchdog presence checks.

These callers do not all have the same correct UNKNOWN policy. A WebUI display may show UNKNOWN/degraded state; a service start gate must refuse to start a duplicate when state is unknown; a stop path must not signal or report success; a restart path must not start a replacement until the previous instance is authoritatively absent. Never blindly cast negative UNKNOWN to Boolean.

### `pids_main()`

One caller candidate: `trunk/user/rc/src/rc.c:2005`, the `pids` applet. Declaration is `shutils.h:74`; definition is `pids.c:462`. Preserve applet exit/status behavior deliberately when introducing a tri-state internal API.

### `kill_services()`

There are 46 call candidates, one definition and one declaration. Definition: `rc/src/common_ex.c:1152`; declaration: `rc/src/rc.h:194`. Callers span:

- `auth.c`, `net.c`, `net_lan.c`, `net_ppp.c`, `net_wan.c`, `net_wan6.c`, `net_wifi.c`, `rc.c`
- `services.c`, `services_ex.c`, `services_stor.c`, `services_usb.c`
- `usb_modem.c`, `vpn_client.c`, `vpn_server.c`, `watchdog.c`

Every caller must be classified for stop failure propagation. Some callers check the return value and refuse subsequent work; others ignore it. The latter require deliberate decisions around network/bridge teardown, storage release, and USB/PPP transitions.

### PID-file signal API

- `kill_pidfile()`: five call candidates in `rc/src/common_ex.c` (two), `net_lan.c` (one), and `net_wan.c` (two); declaration/definition in `shutils.h:70` and `shutils.c:595`.
- `kill_pidfile_s()`: 15 call candidates across HTTPD, `detect_internet.c`, `detect_link.c`, `net_wan.c`, `rstats.c`, `vpn_server.c`, and `watchdog.c`; declaration/definition in `shutils.h:71` and `shutils.c:569`.

The helper reads a numeric PID from a PID file and calls `kill(pid, sig)`; it does not bind the PID to an expected executable, process start time, owner, or identity token. A stale PID file plus PID reuse can therefore signal an unrelated process. The parser also uses `strtoul()` without checking `errno` or the end pointer. Treat PID identity, malformed/truncated files, permissions, `ESRCH`, and signal error propagation as first-class stop-safety work.

## Source-confirmed failure paths to address

1. `alloc_procps_scan()` returns NULL for both scan-context allocation failure and `opendir("/proc")` failure.
2. `procps_scan()` treats any NULL `readdir()` as normal EOF without checking `errno`; iteration errors are indistinguishable from complete scans.
3. On per-PID `/proc/PID/stat` read/parse failure, the current loop can break out of the whole scan, not merely skip a vanished PID. This risks truncating inventory.
4. argv0 allocation (`malloc` / `strdup`) is unchecked; missing argv0 can cause false negatives for truncated `comm` matches.
5. PID-list allocation/growth failure returns NULL after discarding partial results. `find_pid_by_name()`, `pids()` and `pids_main()` cannot distinguish UNKNOWN from ABSENT.
6. `service_lifecycle_is_running()` returns the Boolean `pids()` result.
7. `service_lifecycle_stop()` sends SIGTERM to every configured service before obtaining authoritative inventory. Its wait predicate treats any nonzero `is_running` result as running; its force-kill pass also treats any nonzero result as permission to send SIGKILL. A future negative UNKNOWN must explicitly block both signal paths.
8. `kill_process_pidfile()` and `kill_pidfile_s()` are separate stop routes and must not be left outside the new safety contract.

## Required next review before implementation

1. Use the full artifact hit list to trace every `pids()` and `kill_services()` caller to callers/callees and its enclosing start/stop/restart/readiness transaction. Record whether it may display UNKNOWN, abort start, abort stop, or defer cleanup.
2. Separate the Padavan shared API from BusyBox's independent internal process scanner, verify package/build linkage, and decide whether BusyBox needs a separate change or remains out of scope.
3. Finish the full signal/PID-file inventory, including direct `kill()`, `killall`, shell `kill`, `pidof`, `pgrep`, `pkill`, `waitpid`, `/proc` enumeration and dynamic command construction. The lifecycle regex summary is broad and includes vendor source, documentation, comments and tests.
4. Design explicit tri-state values (for example RUNNING=1, ABSENT=0, UNKNOWN=-1) and a distinct error channel. UNKNOWN must never authorize signals or be treated as ABSENT for stop-success/restart/cleanup.
5. Preflight all services before the first SIGTERM; send no signal if any required inventory is UNKNOWN. Recheck before every signal/escalation. SIGKILL, success, restart and destructive cleanup all require authoritative state.
6. Define per-PID disappearance versus permission/I/O/read errors, process identity and PID reuse semantics. Add injectable GNU89/`-Werror` regressions for allocation, `/proc` open, `readdir` EOF/error, per-PID disappearance/access errors, argv allocation, PID-list growth, empty/matched lists, all-target preflight, UNKNOWN blocking TERM/KILL, PID-file parsing/identity and caller failure propagation.

## Validation boundary

- Inventory workflow and scanner self-test: PASS.
- Source census: exact `main` source commit above; 6,193 candidate text files scanned; no missing candidate or read error.
- This is L1 lexical inventory evidence only. It is not a completed L1 semantic call graph, not L2 production regression coverage, not an L3 MIPS/uClibc firmware build, and not L4 router validation.
- No production C source, firmware template, runtime configuration or WebUI was changed by this inventory milestone.
