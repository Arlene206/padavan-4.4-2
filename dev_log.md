## 2026-11-23 08:35 UTC+8 — Cancel PID1 recovery-shell retry during shutdown

- Parent main SHA: `aa163f6cc5993029684fd55a489528f3133fbf26`. After the previous fast-exit throttle, an ordinary shutdown signal could arrive during `sleep()`; the retry loop kept waiting and then continued through the console relaunch path before the outer loop handled shutdown.
- Added a shutdown-aware retry-wait predicate, immediate event-loop exit on pending shutdown, and guards that prevent console relaunch or another suspend while shutdown is already published. Added GNU89 policy coverage for normal waiting, shutdown cancellation and elapsed-delay completion, plus exact source-contract assertions.
- Updated `SESSION_HANDOFF.md` because the previous first action (verify the fixed compiler step) is complete and the resumable task has materially changed. Roadmap priority/module state is unchanged; no roadmap rewrite.
- Local compile/test: NOT RUN (no local repository checkout/network access). Exact-head PID1 mdev and XY-C1 push Actions: PENDING at commit creation; match each run's `head_sha` to THIS COMMIT. L3/L4 NOT VERIFIED.
- Resulting commit: THIS COMMIT.

---

## 2026-11-23 08:15 UTC+8 — Fix PID1 console retry regression compiler command

- Exact-head PID1 mdev run `45087584585` on `36bd2b1e01c0472b7d8fdc0a31e4a56e3b6762de` failed at the newly added console retry test compile, after earlier GNU89/ASan/UBSan policy tests (including `pid1_signal_policy`) passed. Log showed literal double-backslash arguments to ld (`cannot find \\\\: No such file or directory`), caused by doubled YAML shell continuations.
- Corrected the new workflow step to use one shell continuation per line. No production source or test semantics changed.
- Updated handoff with exact-SHA PASS/FAIL/PENDING results and first action; roadmap now tracks the console test rerun rather than marking PID1 as closed.
- Local checkout/build unavailable in this connector-only session. Exact-head mdev rerun required; XY-C1 `45087584681` was still in progress at last poll.
- Base SHA: `36bd2b1e01c0472b7d8fdc0a31e4a56e3b6762de`; resulting SHA: THIS COMMIT.

---

## 2026-11-22 — Restore required cross-session handoff contract sections

- AI handoff documentation run `45086487184` on exact SHA `fff2b43aa36e71dbe2b3bbda4c74079799419e59` failed because the rewritten session handoff omitted required section headings and exact cross-session/mandatory-checkpoint wording. Restored `## Active task`, `## Verification boundary`, `## Exact first action for the next GPT`, `## Mandatory handoff checkpoint`, and `# Cross-session execution mandate`, including the exact mandatory-vs-conditional checkpoint statement and atomic main-commit requirement.
- The HTTPD security regression on the same exact SHA passed (run `45086487150`). The new handoff docs result is pending on the resulting commit; do not transfer the old docs failure to its descendant.
- Actual firmware PID 1 remains RC init; board BusyBox `CONFIG_INIT` is disabled. Current implementation/test scope is recorded in `SESSION_HANDOFF.md`.

---

## 2026-11-22 — Repair HTTPD workflow block indentation

- The first follow-up commit `00cb20522874510b126cc647efc7afcb1d2952f4` inserted the corrected PID 1 assertions without preserving the embedded Python block's ten-space YAML indentation. GitHub reported the HTTPD workflow run `45086402010` as failed before creating a job (`total_count: 0`); this was a workflow formatting error, not a source/test regression.
- Restored the block indentation for every new assertion while preserving the updated helper contract. The next exact-head HTTPD and AI handoff Actions determine verification; no local YAML parser or checkout was run.
- The actual RC PID1 host test result remains PASS on source SHA `f0ee5131e495a7b9f4a903293d53d304a3ec830b`; NVRAM audit `45086111754` subsequently completed PASS. XY-C1 firmware validation `45086111757` had not completed at the last poll.

---

## 2026-11-22 — Align HTTPD PID1 contract with errno-preserving reaper

- Exact-head HTTPD run `45086111706` on `f0ee5131e495a7b9f4a903293d53d304a3ec830b` failed only in its embedded PID 1 source contract: it still required the `waitpid()` loop and EINTR handling directly inside `catch_sig_reap()`. The production implementation intentionally moved the loop into `pid1_signal_reap_available_children()` so it can preserve the interrupted caller's errno and be exercised through an injected waitpid callback.
- Updated the HTTPD contract to require handler delegation, the helper's waitpid/WNOHANG and EINTR loop, saved/restored errno, exact-child bookkeeping, and the injected regression. This is a test-contract correction; production behavior is unchanged.
- Other exact-head results for `f0ee5131...`: PID 1 mdev/security `45086111693` PASS; notification queue `45086111684` PASS; USB storage hotplug `45086111681` PASS; DNS forwarder `45086111683` PASS; DNSMasq `45086111735` PASS; Process lifecycle `45086111753` PASS; Process umbrella `45086111764` PASS; AI handoff documentation `45086111726` PASS. HTTPD `45086111706` FAIL due to the stale source assertion. NVRAM audit `45086111754` and XY-C1 validation `45086111757` were IN_PROGRESS at last poll. The injected PID1 host regression passed in the PID1 workflow; no local compile was run.
- Runtime scope: board BusyBox config has `CONFIG_INIT` disabled; Padavan's actual firmware PID 1 is `trunk/user/rc/src/init.c`. The generic BusyBox init deferred-reap work is not the running router PID1 and must not be presented as such.

---

## 2026-11-22 — Preserve interrupted errno across PID1 child reaping

- The shipped Padavan PID1 is the RC init binary (`trunk/user/rc/src/init.c`); the board BusyBox configuration explicitly has `CONFIG_INIT` disabled. Therefore the live runtime defect is in the RC SIGCHLD handler, not the optional BusyBox init applet.
- `catch_sig_reap()` called `waitpid(-1, ..., WNOHANG)` in a loop and left errno at the terminal `ECHILD` (or another wait error). Since SIGCHLD can interrupt PID1 while it is performing an unrelated libc/system call, returning with clobbered errno can change the interrupted operation's error handling.
- Extracted the async-signal-safe reap loop into `pid1_signal_reap_available_children()`, preserving errno around the entire waitpid/EINTR/reap sequence while retaining exact-child console-shell state clearing. Production handler now delegates to that helper.
- Added an injected host regression covering another child reap, EINTR retry, tracked console-shell reap, terminal ECHILD, and restoration of the caller's original errno. Updated the existing PID1 source contract to enforce this data path.
- Local test not run in this connector-only session. Exact-head `init-mdev-security.yml` host compile/runtime Action will provide verification; no firmware or device result is claimed.

---

## 2026-11-22 — Refresh DNSMasq split-DNS unlock-path contract

- Exact-head DNSMasq run 45085012107 on 5866501d8fd54f35c30d28b3b442924ed6e8e650 failed at the split-DNS server writer assertion. Production now unlocks on four distinct paths: NVRAM snapshot failure, temporary-file open failure, publish failure, and success. The checker still expected the previous three-path count.
- Updated the source contract to require four unlocks. Existing assertions continue to pin unlock on the snapshot failure and after publish, and no unlock on failed lock acquisition. Production behavior is unchanged.

---

## 2026-11-22 — Align HTTPD security workflow with MTD restore symlink rejection

- Exact-head HTTPD security run 45084820446 on 3adf4cc9d94bd930b6504b0dd11cc1fa109852fd failed at the archive-restore source contract. The production guard had already been hardened to reject a symlinked `start_script.sh`, but the workflow still required the old `[ ! -f ... ]`-only condition.
- Updated the workflow source assertion to require both the symlink check and the regular-file check. This is a checker-only synchronization; no production source changed in this slice.

---

## 2026-11-23 04:55 UTC+8 — Align DNSMasq lock contract with fail-closed split-DNS snapshot

- The first DNSMasq run on source SHA 4fad5ae2ac97a35d79031c3a18e828702d010109 failed in the generated-file/RC status contract, not in compilation or the split-DNS unit test. The old assertion treated every line between failed lock acquisition and staged-file open as the lock-acquisition failure branch; the new snapshot-error path correctly calls file_unlock() because that branch has already acquired the lock.
- Narrowed the no-unlock assertion to the actual lock-acquisition failure branch, and added explicit assertions that snapshot reads happen after lock acquisition and before stage creation, and that snapshot failure releases the held lock, preserves the existing server file, restores errno and returns failure.
- Exact source SHA 4fad5ae2ac97a35d79031c3a18e828702d010109: HTTPD, Process inventory/lifecycle, Inadyn, miniupnpd, WAN route, WAN DHCP and Samba workflows PASS; DNSMasq run 45084767538 FAIL only on the stale assertion above; NVRAM audit 45084767487 and XY-C1 firmware validation 45084767430 were still in progress at last check. No local compile/test was run.
- This commit updates only the workflow contract and handoff/log state; its exact-head Actions remain pending until the push creates them.

---

## 2026-11-22 — Reject symlinked MTD restore startup scripts

- The restore guard used `[ -f start_script.sh ]`, which follows symlinks. An archive containing `start_script.sh -> /bin/sh` therefore passed the required-file check and was copied into the live storage tree; the firmware later executes that path as root during boot.
- Require the extracted `start_script.sh` to be a non-symlink regular file before staging or stopping rstats. Added an end-to-end restore regression with an archive symlink to an existing executable and assertions that the previous storage tree and running rstats remain untouched, no rename occurs, and the archive is retained. The test rebuilds a normal archive before its existing successful restore case.

---

## 2026-11-23 04:30 UTC+8 — Fail closed when split-DNS routing NVRAM is unreadable

- fill_dnsmasq_servers() used error-collapsing reads for vpnc_pdns, VPN domain/server values and MAN domain/server values. A transient NVRAM fault could silently omit private-domain routing rules from the newly published DNSMasq server file, allowing VPN/MAN-specific queries to fall back to the default resolver path.
- Added one bounded, transactionally copied snapshot for all five inputs. The VPN private-DNS flag preserves the original integer-greater-than-zero semantics; missing keys retain empty/disabled defaults. Read, transaction-close, parse and truncation failures now abort before staging/publication, leaving the previous DNS server snapshot intact.
- Added injected host regressions for every read position, begin/end failure, absent and negative flags, malformed integers, and oversized domain/server lists. The DNSMasq source checker now enforces snapshot-before-stage ordering and rejects the old lossy reads.
- Source commit: `d73ba6d2b9df959b73f0c0cf283d9694362b85c3`. Exact-head push Actions were QUEUED at first observation: DNSMasq 45084574655; NVRAM transaction audit 45084574653; Process umbrella 45084574525; Process inventory 45084574515; XY-C1 45084574492; WAN DHCP 45084574658; miniupnpd 45084574657; Inadyn 45084574546; AI docs 45084574530; WAN route 45084574493; HTTPD 45084574469; Samba 45084574447. Recheck live conclusion and exact `head_sha`. Local compile/test NOT RUN; no L3/TRX/artifact or L4 hardware claim.

---

## 2026-11-22 — Protect Optware startup and teardown from shared /var/lock replacement

- Optware's default lock lived at `/var/lock/optware-lifecycle.lock`, while this firmware creates `/var/lock` world-writable without the sticky bit. A local user could unlink or replace the pathname while startup held the old inode, allowing teardown/startup to lock a different inode and bypass serialization.
- Moved the default to `/tmp/optware-lifecycle.lock`, provisioned atomically as owner-only mode 0600 by `dev_init` after sticky `/tmp` setup and before mdev coldplug. The helper now refuses missing, relative, symlinked, hard-linked, non-private, wrongly-owned or unsafe-directory lock paths; it no longer creates the lock itself. Blocking startup/mount and nonblocking teardown retain the existing lock-order behavior.
- Added negative-path lock regressions (symlink, hardlink, permissive mode, missing file, relative path and non-sticky writable directory), updated startup/teardown fixtures, pinned dev_init ordering, and wired the three Optware test scripts plus source contracts into Process lifecycle CI. Exact-head Actions pending; no device runtime claim.

---

## 2026-11-22 — Load process-inventory sources before asserting their parser contract

- Exact-head Process inventory run 45083076267 on 5e15ed4dec1dd59ea3953d2873dbea8cc73bfb9f failed with `NameError: process_inventory is not defined`. The new source-contract assertions referenced `process_inventory` and `process_inventory_test`, but the checker had not loaded either source file.
- Added explicit reads for the production parser and its host regression test alongside the existing shared-process source loads. The source assertions now inspect the intended files rather than failing before evaluation.

---

## 2026-11-22 — Correct AliyunDrive lock-directory parameter expansion

- Exact-head AliyunDrive lifecycle run 45083004459 on c1a2f7f03c9223acb5807cd4bb03918575e89d6b failed immediately when the new lock-policy regression tried to acquire its pre-provisioned fixture. Review of the shell log showed the parent-directory parameter expansion had been committed with a literal leading backslash, so the path validator tested a non-existent literal directory.
- Corrected the expression to evaluate the parent of `ALD_LOCK_FILE` and strengthened the source checker to pin the literal shell expansion and unescaped default path. This restores the intended test/production path checks; exact-head Actions on the corrective commit remain required.

---

## 2026-11-22 — Align process checker with no-follow PID-file open

- Process lifecycle run `45082617344` on `489d1a1a4f90816adcf24b069a3505b31a8d038d` failed because the integration checker still required a direct `open(pidfile, O_RDONLY | O_NONBLOCK)` call. Production now routes this through `shutils_pidfile_open()`, adding `O_NOFOLLOW` while preserving `O_NONBLOCK`; the legacy PID-file workflow separately tests regular-file compatibility and symlink rejection.
- Updated the Process contract to require the helper and both flags. This is a checker-only correction; process inventory unit tests passed on the prior SHA.

---

## 2026-11-22 — Protect AliyunDrive lifecycle serialization from shared /var/lock replacement

- The target boot setup creates `/var/lock` with mode 0777 and no sticky bit, while the AliyunDrive launcher previously created/opened a predictable lock there. A local user could unlink or replace that path while a transition held the old inode, allowing another start/stop operation to acquire a different inode and bypass serialization; a symlink/hardlink could also redirect the lock to an unrelated inode.
- Provisioned `/tmp/aliyundrive-webdav.lock` exclusively with the existing `required_lock_file` helper after `/tmp` is made root-owned sticky and before `mdev -s`. The launcher now refuses missing, symlinked, multiply-linked, non-0600, wrongly-owned or replaceable-directory lock paths, and never creates the lock itself. This keeps the flock inode stable for the entire boot.
- Added host regressions for symlink, hardlink, permissive mode, missing path and non-sticky writable-directory rejection; extended dev_init provisioning/order contracts and wired those tests into AliyunDrive lifecycle CI. Exact-head Actions are pending; no device runtime claim.

---

## 2026-11-22 — Require decimal-only procfs process start times

- Hardened the procfs stat parser against signed start-time tokens. `strtoull()` accepts a leading sign (including `-1` as an unsigned value), but the kernel's `starttime` field is an unsigned decimal counter; accepting a signed token could turn malformed procfs data into a valid process identity.
- The parser now validates every start-time character as an ASCII decimal digit before conversion. Added injected regressions for both positive-sign and negative-sign tokens; each must return `PROCESS_PRESENCE_UNKNOWN/EPROTO` and discard the partial inventory.

---

## 2026-11-22 — Validate procfs task-state codes before process matching

- The shared C process scanner accepted any one-character `/proc/<pid>/stat` state and treated every state except uppercase `Z`/`X` as live. A malformed state could therefore be promoted to a process match and authorize lifecycle actions; Linux's lowercase `x` dead state was also incorrectly reported as present.
- The parser now accepts only documented kernel state codes (`RSDZTtXxKWPI`), treating unknown/malformed values as `PROCESS_PRESENCE_UNKNOWN`. Both `Z`, `X`, and `x` are treated as exited/absent.
- Added an injected procfs regression covering every supported live state, all dead states, and an invalid state, and pinned the contract in the Process checker. Exact-SHA Process and XY-C1 validation pending.

---

## 2026-11-22 — Reject symlink PID files before privileged signalling

- `kill_pidfile_s()` opened its PID-file path with `O_RDONLY|O_NONBLOCK` and then checked only the opened descriptor's file type. Because `open()` followed symlinks, a writable PID-file directory or replaced path could redirect this privileged helper to an attacker-controlled regular file containing an unrelated PID.
- Added a shared no-follow open helper and routed `kill_pidfile_s()` through `O_NOFOLLOW`; the existing nonblocking FIFO protection and subsequent regular-file/size/strict-record checks remain intact. Symlink paths now fail with `ELOOP` instead of supplying a PID to `kill()`.
- Added a GNU89 ASan/UBSan regression covering regular-file compatibility and symlink rejection, plus source contracts that pin the production call path. Exact-head legacy PID-file and XY-C1 Actions pending.

---

## 2026-11-22 — Normalize resolver errno-preservation source contract

- Exact-head Process run 45082141507 on 631aa825873d7f69087ec735a40046eefc4d49f5 progressed past the WAN MAC check, then failed on a DNS resolver source contract that searched for double-escaped newline/tab text. The production early-return path unlocks the resolver lock, restores the saved NVRAM error to errno, and returns that status; the checker compared literal backslash sequences instead of C source whitespace.
- Replaced the formatting-sensitive assertion with a whitespace-normalized contract for the full unlock/errno/return sequence. Production resolver code is unchanged; exact-head Process rerun pending.

---

## 2026-11-22 — Make WAN MAC contract insensitive to escaped whitespace

- Exact-head Process run 45081952718 on b4f3e3dbd7563b7ea5d339b14f5df390595d256e reached the WAN VLAN source contracts, then failed because the WAN MAC assertion contained double-escaped newline/tab sequences and searched for literal backslashes rather than the production call's actual whitespace.
- Normalize the config_vinet_wan() function body and match the complete checked wan_hwaddr accessor call without formatting assumptions. Production VLAN/MAC code is unchanged; exact-head Process rerun pending.

---

## 2026-11-22 — Update PID 1 recovery call-site source contract

- Exact-head Process run 45081733777 on 96d0037e76b41214c865b9cff45b7579677d92e9 failed in scripts/check_dev_init_boot.py. The integration checker had been updated to require init_console(int create_session) and both new caller modes, but a second legacy assertion still required the old zero-argument recovery call.
- Updated that remaining assertion to require init_console(1). The production caller and explicit PID 1 boot call init_console(0) are unchanged by this test-only follow-up. Process exact-head rerun pending.

---

## 2026-11-22 — Fix Aria2 default-config staging collision during concurrent start

- Exact-head Storage run 45081558026 on ebbff58e321030829cc517fd0f1da21ed5923660 failed in the existing two-process Aria start regression. The default config path used the literal suffix .tmp.$ instead of shell PID expansion .tmp.$$, so both launchers collided on one noclobber staging file; one aborted and the other timed out at the test barrier.
- Changed the staging path to include the invoking shell PID. The existing concurrency regression requires two distinct staged config files and two successful launches; added a source contract to pin PID uniqueness. No firmware/runtime router claim.
- Storage and Process exact-head Actions on this fix are required; the previous failure was a reproducible production staging-name defect, not an infrastructure result.

---

## 2026-11-22 — Correct whitespace-free MAN teardown return assertion

- The first checker normalization removed all whitespace from the C function, but its expected literal still retained spaces after commas. This was a second test-only false negative; the expected nested `route_status_accumulate()` string now uses the same whitespace-free form as the normalized source.

---

## 2026-11-22 — Keep PID 1 boot console setup out of the recovery-child session path

- Review of the first console-attachment commit found an important caller-context distinction: init_console() is called both by PID 1 during normal boot and by a forked recovery-shell child. PID 1 is already a session/process-group leader, so setsid() fails with EPERM there; making that failure fatal would incorrectly mark the normal boot console unavailable.
- Added an explicit create_session argument. PID 1's boot-time console setup skips setsid() (the legacy outer startup call remains unchanged); only the recovery child creates and verifies its own session before opening/attaching the console. The integration checker pins both call sites.
- This is a corrective follow-up to ebbff58e321030829cc517fd0f1da21ed5923660; exact-head Process/XY-C1 reruns are required. No target build or hardware validation claimed.

---

## 2026-11-22 — Check PID 1 recovery console attachment and post-fork exec failure

- init_console() previously ignored failures from setsid(), all three dup2() operations, TIOCSCTTY and tcsetpgrp(). A recovery child could continue to termios setup and exec an interactive shell without valid standard descriptors or a controlling foreground terminal. The new checked attachment policy fails at the first failed operation, avoids terminal ioctls on the intentional /dev/null fallback, and preserves the first error.
- The execve() failure path also used exit() after fork(), potentially running PID 1's inherited exit handlers in the child. It now preserves the exec errno and terminates with _exit().
- Added GNU89/ASan/UBSan policy coverage for descriptor duplication, controlling-terminal acquisition, foreground-group setup, fallback behavior and failure gates; extended the PID 1 integration checker. Exact-head Process and XY-C1 verification required; no router/hardware validation claimed.

---

## 2026-11-22 — Make Aria2 RPC-secret fixture emit a valid deterministic token

- Exact-SHA Storage Actions failed the legacy RPC migration test before any production daemon launch. The test's `hexdump` stub ran `printf '%032x' "$"`; a bare dollar sign is not a numeric shell argument, so POSIX `printf` returned status 1 and Aria2 correctly aborted secret creation.
- Replaced the malformed numeric conversion with a fixed 32-character lowercase hexadecimal fixture. The secret remains deterministic for tests; production still reads random bytes from `/dev/urandom`.

---

## 2026-11-22 — Make MAN cleanup checker whitespace invariant

- Exact-SHA Process Actions exposed a checker false negative: `" ".join(source.split())` inserted spaces after multiline opening parentheses, while the expected nested `route_status_accumulate()` expression had none. The checker now removes all whitespace before matching the return expression; production code is unchanged.

---

## 2026-11-22 — Do not stop MiniUPnP on an unreadable WAN identity

- `check_upnp_wanif_changed_locked()` compared the callback interface with a lossy `get_wan_unit_value()` read. A transient NVRAM backend error appeared as an empty interface name, so the comparison looked like a WAN transition and stopped a healthy MiniUPnP daemon.
- The callback and configured interface are now bounded and validated, and the active WAN name is obtained with `get_wan_unit_value_copy_status()`. A backend error, missing key, invalid name or overlong value returns a status without stopping the daemon; only a verified difference can request stop.
- Added a GNU89/ASan/UBSan unit regression for backend errors, missing/invalid identities, identical names and a verified transition, plus exact production-wiring assertions. Exact-SHA MiniUPnP, Process and XY-C1 Actions pending.

---

## 2026-11-22 — Replace Aria2 router-password RPC auth with a dedicated persistent token

- The prior RPC hardening reused the router web password as `rpc-secret`. Aria2 runs as `nobody`, so that placed the administrator credential in a config readable by the daemon's shared UID. Replaced it with a dedicated 128-bit token read from `/dev/urandom` through target BusyBox `hexdump`; it is stored as `aria2.rpc-secret` mode 0600 and copied into `aria2.conf`. Set AriaNg's RPC Secret Token from that file. Router login credentials are no longer read or written.
- Concurrent token creation uses PID-unique noclobber staging and atomic hard-link publication, so concurrent starts converge on one persistent token. Existing unauthenticated `rpc-listen-all=true` configs are migrated to that token; existing authenticated configs are preserved and chmodded 0600. Secret files are validated for symlink/type, exact byte and line count, and lowercase-hex format.
- Removed runtime `mktemp` usage because `CONFIG_MKTEMP` is disabled in target BusyBox; config staging uses shell noclobber creation and mode 0600. Target BusyBox enables `HEXDUMP` and `LN`. Expanded host storage regression for random token generation, persistent/config consistency, concurrent creation, legacy migration and generator failure. Exact-head Actions pending.

## 2026-11-22 — Check LAN/WAN MAC overrides before VLAN topology changes

- `config_soft_bridge_lan()` and `config_vinet_wan()` used lossy `nvram_safe_get()` reads for the configured LAN/WAN MAC overrides immediately before creating VLAN interfaces or changing VLAN hardware state. A transient NVRAM error could be mistaken for an empty MAC and continue through topology mutation.
- Both values are now copied through the checked, bounded NVRAM accessor before the corresponding topology/hardware changes. An absent optional override retains its previous empty-string semantics; backend errors and overlong values abort before mutation.
- Extended the Process lifecycle source contract to pin both checked reads and ordering. Exact-SHA Process and XY-C1 Actions pending.

---

## 2026-11-22 — Preserve resolver errno across lock release

- The checked WAN DNS read failure path now restores `errno` after releasing the resolver publication lock, matching the neighbouring VPN-DNS and WAN-DNS-mode error paths.

---

## 2026-11-22 — Align MAN cleanup integration assertion with nested status aggregation

- Process inventory on exact SHA `f99bf7c1711669961755cb06189db388bf54f099` failed only because the integration checker required the obsolete spelling `route_status_accumulate(status, conntrack_status)`. Production `udhcpc_deconfig()` already nests accumulation of protocol, interface, conntrack and route cleanup errors, preserving the status correctly.
- Updated the static contract to normalize whitespace and assert the actual nested aggregation expression. This is test-only; exact-head Process rerun pending.

## 2026-11-22 — Fail closed on unreadable previous WAN VLAN cleanup identity

- `config_vinet_wan()` used the lossy `get_wan_unit_value(0, "viflast")` getter immediately before removing an old VLAN interface. A temporary NVRAM read failure looked like an empty identity, silently skipped cleanup, and then proceeded to replace/publish the new VLAN identity.
- The old interface name is now copied with the checked bounded getter and validated before any VLAN hardware/interface mutation. Missing configuration remains an empty value; read, length, or name-validation failures abort setup so the previous VLAN is not orphaned.
- Added an integration contract that pins the checked read/validation before hardware VLAN changes. Exact-SHA WAN/Process and XY-C1 Actions pending.

---

## 2026-11-22 — Keep resolver input errors separate from DNSMasq snapshot health

- The new resolver DNS-read gate now uses its own status accumulator. The existing `dns_servers_error` belongs to the independent DNSMasq upstream-file health gate; sharing it could erase a previous DNSMasq validation error. The two error domains remain independent.

---

## 2026-11-22 — Keep the last-good resolver on WAN DNS NVRAM read failures

- `update_resolvconf()` carefully checked the VPN DNS snapshot and static/dynamic DNS mode, but subsequently used `nvram_safe_get()` / `get_wan_unit_value()` for WAN DNS values. A transient NVRAM backend/close failure was converted to an empty list, then the function published the public `8.8.8.8` fallback over the previously valid resolver configuration.
- Static DNS entries now use one bounded transactional snapshot; dynamic WAN DNS, MAN fallback and (when enabled) IPv6 DNS use checked bounded reads. All inputs are validated before a staging file is opened. Any read error is surfaced and leaves the existing resolver file untouched; genuinely absent keys keep the prior empty/default behavior.
- Added integration contracts for all checked source keys and the no-lossy-get requirement. Existing NVRAM copy/snapshot unit cases cover absent values, backend errors, truncation, and transaction rollback. Exact-SHA Process and XY-C1 Actions pending.

---

## 2026-11-22 — Preserve WAN MAN conntrack cleanup errors on NVRAM read failure

- `udhcpc_deconfig()` already checked the MAN protocol, but still used `nvram_safe_get("wanx_ipaddr")` for the old address used by conntrack cleanup. A transient backend error was therefore indistinguishable from an absent key and silently skipped flushing stale conntrack entries; the existing return status also stayed successful.
- Replaced that read with the checked, bounded NVRAM copy helper. Missing keys retain the legacy empty-address behavior; backend errors and truncation are logged and accumulated into the deconfig result without changing the remaining route/address teardown sequence.
- Added an exact-key regression for present, absent and backend-error reads, plus a production wiring contract in the Process lifecycle checker. Exact-SHA Process and XY-C1 Actions pending.

---

## 2026-11-22 — Forward Aria2 RPC password overrides through the startup test runner

- The exact USB regression on `aae8c0af3577bf7b06aa5c15dccaa02ebbf5bcfd` failed at the empty-password case: `run_aria_config_failure()` set `STORAGE_TEST_HTTP_PASSWD` in its own environment, but `run_launcher()` invokes the child with an explicit environment whitelist, so the override was dropped and the test stub silently used `test-pass`.
- Added a dedicated 22nd `run_launcher` argument and explicitly forwards it in both BusyBox ash and host sh invocations; the empty and newline-injection cases now pass the requested credential into the NVRAM stub. Production source is unchanged in this correction. Exact-head USB, Process and XY-C1 Actions pending.

## 2026-11-22 — Fix Aria2 legacy-config regression test variable isolation

- USB storage on exact SHA `c9014ea94517be6d713a8029efe3bff9fb4bd16c` reached the new Aria2 migration test after the launcher passed `sh -n` and earlier startup cases passed. The test failed because `run_launcher()` overwrites the generic `config_dir` variable, so its post-run assertion accidentally checked `/aria2.conf` rather than the migration fixture path.
- Renamed the migration fixture variable to `migration_config_dir` throughout the test so it remains isolated from runner state. Production code is unchanged in this correction; exact-head USB/XY rerun pending.

## 2026-11-22 — Fail closed on PID 1 console termios errors

- `setup_term()` previously ignored `tcgetattr()` and then rewrote terminal fields even when the read failed, passing an uninitialized `struct termios` to `tcsetattr()`. This was reachable when the recovery-shell child fell back to `/dev/null`; `/dev/null` is not a terminal.
- Added an injectable termios policy that stops before any write on a failed read, reports both getter and setter failures, and returns a deterministic error when libc supplies no errno. The console initializer now skips termios setup on the deliberate `/dev/null` fallback, propagates setup errors, and the recovery-shell child aborts rather than execing an interactive shell without a usable console.
- Added GNU89/ASan/UBSan coverage for missing callbacks, getter/setter errors (including zero errno), no setter call after a failed getter, and the exact terminal flag/control-character configuration. Local host compile and execution passed; exact-head Process and XY-C1 verification pending.

---

## 2026-11-22 — Cover existing Aria2 RPC config migration

- Hardened the RPC config classifier so an explicit secret/basic-auth setting is forced to mode 0600 even when the listener is loopback-only; absent listener and missing authentication now remain separate states, while read/grep errors fail closed.
- Added an executable legacy-config migration test: an existing `rpc-listen-all=true` file with no auth is migrated without losing the listener setting, gains the router-web-password token, is atomically replaced at mode 0600, starts only after migration, and leaves no staging file.
- Exact-head Actions are still the source of truth for target/host verification.

## 2026-11-22 — Repair Aria2 RPC hardening shell integration and build triggers

- The first RPC hardening commit exposed a malformed newline at the end of the default-config branch (`umask ...fi`). Fixed the branch delimiter and restored the saved umask on the config-write error path before proceeding.
- Added the Aria2 launcher and its startup/security regression test to the XY-C1 firmware workflow push path filter; without those paths the firmware validation did not run for Aria2-only changes.
- Exact-head USB/Process/XY-C1 Actions will validate this correction. The generated token/mode tests and migration code remain subject to those checks.

## 2026-11-22 — Authenticate Aria2 all-interface RPC configs

- Default Aria2 config exposed an unauthenticated RPC control/download API by enabling `rpc-listen-all=true` and wildcard CORS with all authentication directives commented out.
- New configs now require the router web password as `rpc-secret`, reject empty/control/line-break passwords, and use exclusive `mktemp` staging under umask 077 with explicit mode 0600 before atomic publication. Existing custom configs with unauthenticated `rpc-listen-all=true` are atomically migrated; existing explicit secret or complete basic-auth configs are preserved but hardened to mode 0600. Local-only configs are unchanged. Set AriaNg's RPC Secret Token to the router web password.
- Host regression checks generated token/mode and rejects empty/newline passwords. Exact-head USB/XY-C1 Actions pending.

## 2026-11-22 — Correct Transmission refreshed-PID pattern for dash

- The USB regression on \`a7316d47ecbfd70b356667b4acf949685c480355\` stopped at \`sh -n\`: the new post-graceful PID pattern had two literal backslashes before the space, which dash does not accept in a case pattern. The existing initial-snapshot pattern was already correct.
- Reduced the new pattern to one escaped space; verified the source contains five single-escaped PID character classes and no doubled form. Exact-head USB rerun pending.

## 2026-11-22 — Emit real line records in refreshed PID regression fixtures

- A source review of the new refreshed-PID test found that its two \`printf\` format strings contained doubled backslashes, so the fixture would emit literal \`\\n\` characters rather than line terminators. That would reject the initial fixture for the wrong reason and fail to isolate the post-graceful leading-zero PID case.
- Corrected both fixture writes to emit actual newline-terminated records; the pidof stub then normalizes the state file to the expected space-separated PID output. Production source unchanged in this follow-up; exact-head USB regression pending.

## 2026-11-22 — Revalidate Transmission and Aria2 PID snapshots before escalation

- Review found that Transmission's post-graceful-stop snapshot still used the weaker \`''|0|1\` token check, so a refreshed leading-zero PID such as \`0707\` could reach \`kill -9\` even though the initial snapshot had been validated strictly.
- Tightened the refreshed Transmission snapshot to validate the complete character set, positive PID tokens and nonempty count before per-PID escalation. Added a post-killall pidof override to the shared storage regression, proving both Transmission and Aria2 reject a leading-zero refreshed PID without issuing SIGKILL. Exact-head USB/Process CI pending.

## 2026-11-22 — Correct Aria2 stop PID pattern for dash

- The follow-up USB run on \`0fd18d97a70df0cccc487492922cfbe578bb33a0\` did not yet reach the new leading-zero test: \`sh -n\` rejected the Aria2 stop controller because both new character-class patterns contained two literal backslashes before the space. This is the same dash portability issue previously corrected in the reload path.
- Corrected both stop-path classes to a single escaped space and extended the optional-app source checker to require that exact form, reject the doubled escape and verify the nonempty token-count guard. Exact-head USB/Process validation pending.

## 2026-11-22 — Validate procfs stat PID and state fields before classifying absence

- Review of the new pidof-absence fallback found that it treated any nonempty token after \`") "\` as a valid task state. A malformed record such as \`323 (test-daemon) BROKEN ...\` could therefore be misclassified as RUNNING instead of UNKNOWN; it also did not verify that the stat record's PID matched the enumerated procfs directory.
- Added explicit directory-PID matching and a whitelist of Linux task-state codes (including the kernel-4.4 states and modern idle state). Extended the shared process regression with invalid-state and PID-mismatch records, while retaining zombie exclusion. Exact-head Process CI pending.

## 2026-11-22 — Reject malformed Aria2 stop PID snapshots before signaling

- The exact USB storage regression on `9686ad35850b6e183c65b8e8408fce1e62088410` exposed that Aria2's stop controller still accepted PID tokens with leading zeroes (`0123`). Its `''|0|1` pattern rejected only the single token `0`, and whitespace-only successful output was also able to pass an empty `for` loop and reach the name-wide graceful `killall`.
- Added complete character-set, positive-token and nonempty-token-count validation before graceful stop, then repeated the same validation on the refreshed snapshot before per-PID SIGKILL. The existing shared Transmission/Aria stop regression now covers leading-zero and whitespace-only PID lists and asserts no signal is sent. Exact-head USB/Process Actions pending.

## 2026-11-22 — Shared process lifecycle confirms ABSENT through procfs

- Source baseline: `9686ad35850b6e183c65b8e8408fce1e62088410`. The shared shell helper `pl_process_state()` classified `pidof` status 1 with empty output as confirmed ABSENT. BusyBox pidof can also return this status when procfs enumeration is incomplete, so the helper could authorize duplicate ttyd startup or report an already-stopped daemon when it was still present but omitted from the scan.
- Kept the validated PID fast path for status 0, but status 1/empty now triggers a full procfs inventory. A complete scan with no matching live task is ABSENT; unreadable or malformed per-PID stat records, an unavailable proc root, or other pidof status/output inconsistencies remain UNKNOWN. Zombies/dead tasks are excluded from RUNNING.
- Extended the shared shell regression with fallback discovery, unreadable/malformed stat, zombie, and missing-proc-root cases. The process-lifecycle umbrella workflow already invokes this test. Exact-head CI pending; no router/firmware test claimed.

## 2026-11-22 — Use portable octal newline in the Transmission pidof stub

- The PID-output fixture normalizer used a doubled backslash in the `tr` set, which translates literal backslash/letter bytes instead of line-feed bytes. Corrected it to the POSIX octal newline escape `\012`, preserving the production parser's strict space-separated pidof contract.
- This is a test-fixture-only correction; exact-head USB hotplug regression rerun pending.

## 2026-11-22 — Match Transmission stop PID fixture to pidof output framing

- The first exact USB storage run after the Transmission stop preflight failed at the existing graceful-stop case. The new validator correctly rejected the test stub's newline-separated PID file, but the stub had returned raw file lines rather than pidof's whitespace-separated output. Updated only the fixture to convert the state records to a space-separated PID snapshot; the production parser continues to accept spaces and reject embedded newlines.
- Re-run required to validate both normal multi-PID stop and the new no-signal malformed-snapshot cases.

## 2026-11-22 — Reject malformed Transmission PID snapshots before stop signals

- Review of `func_stop()` found that it checked pidof's exit status and empty/nonempty consistency, but called `killall transmission-daemon` before validating the successful PID output. A status-0 response containing malformed, unsafe or whitespace-only data could therefore authorize a broad signal despite an UNKNOWN snapshot.
- Added strict validation and a nonempty token-count check before the graceful killall; expanded the storage-app stop regression for malformed, zero, PID 1, leading-zero and whitespace-only snapshots, asserting that neither killall nor per-PID SIGKILL is attempted. Exact-head USB storage regression pending.

## 2026-11-22 — Aria2 build downloads must verify TLS certificates

- Source baseline: `cb2cf663a60f5e06dded99b4b55b248a9ce95b38`. The Aria2 build Makefile passed `--no-check-certificate` to wget for both the upstream aria2 source tarball and the AriaNg web UI archive, disabling server certificate authentication on two supply-chain inputs fetched over HTTPS.
- Removed the bypass from both download rules so wget verifies the GitHub TLS certificate chain using the build host trust store. Kept the URLs HTTPS. Added an exact source-contract assertion to the process-lifecycle checker to prevent the bypass from returning.
- This change intentionally fails the build instead of silently accepting an untrusted/mismatched TLS endpoint; exact-head CI pending. No local firmware build claimed.

## 2026-11-22 — Correct Aria2 checker function-boundary extraction

- Exact SHA `ef3ee77898620902bbc5bb98e4bc0ed75241a38e` reached `check_aria2_reload()` after the SCUTClient 59-assertion parser test passed, but the checker raised `ValueError: substring not found`. Its source slice searched for the two-character literal `\\n` before `case`, while `aria.sh` contains an actual line break.
- Changed the static checker to locate the function start and the following newline-delimited dispatch case separately. Aria2 production source was not implicated in this run; exact-head Process regression rerun pending.

## 2026-11-22 — Make Dropbear readiness regression reach its intended postcondition

- The process-lifecycle rerun on exact SHA `3d3494c7cc022444c17f165741546ae507375351` passed the VLMCSd/DNS Forwarder/IAPP checks and reached `check_dropbear()`, but its "successful launcher without a visible process" case returned the earlier injected launcher error (53), not the expected readiness timeout (1).
- The fixture had left `MOCK_DROPBEAR_STATUS=53` from the preceding launcher-failure assertion. Reset it to zero before the no-PID case so the test isolates a successful launcher followed by a missing process. Product behavior remains unchanged; exact-head regression pending.

## 2026-11-22 — Aria2 reload now validates and signals every daemon PID

- Source baseline: `bcfca6426dad985aee97b93a6146ff537940a4d2`. `func_reload()` treated any nonempty `pidof aria2c` output as valid, passed the entire whitespace-separated list as a single argument to `kill -1`, ignored the signal status, and printed `[ OK ]` even when no process was running or the reload failed. A partially failed procfs scan could therefore report a false successful reload.
- Reload now requires a consistent pidof status/output pair, validates every positive PID before signaling any process, sends SIGHUP individually to all listed aria2c instances, and propagates the first signal failure instead of printing success. Review caught two shell-specific defects in the first implementation before CI: the new PID character class carried a doubled escape, and `if ! kill ...; then status=$?` captured the inverted status (zero), not the failed signal's status. Corrected the class to one escaped space and rewrote the branch as `if kill; then :; else status=$?; ...`.
- Added an exact executable-source regression for empty-success, pidof error, inconsistent output, malformed/unsafe PID lists, whitespace-only snapshots, multiple running PIDs, and signal failure. The process lifecycle regression workflow executes `check_optional_app_start_status.py`; exact-head CI pending.

## 2026-11-22 — Add direct security regression for SCUTClient private credential parser

- The one-shot private credential-file implementation was already present in the package overlay, but the host lifecycle workflow only exercised the wrapper and fake launcher; it did not compile or directly exercise the C parser.
- Added a GNU89/ASan/UBSan production-linked test covering valid literal credentials and legacy empty values, malformed/reordered/duplicate fields, embedded NUL and CR/LF framing, symlinked file and parent rejection, exact private ownership/modes, hard-link rejection, bounded size, unlink-before-read behavior, read I/O failure, EINTR retry and short reads. Local compile/run passed: 59 assertions.
- Registered the test and SCUTClient package inputs in both push and pull-request process-regression filters. Exact-head CI pending; no XY-C1 build is claimed by this host-test-only slice.

## 2026-11-22 — Dropbear start/stop now verify process identity state

- Source baseline: `ccb9c0ac835291901b46734b9c84a912a18c2524`. `func_start()` launched Dropbear without checking whether the listener was already running or verifying that a successful fork produced a daemon; `func_stop()` returned success solely from `killall` status and treated an already-absent daemon as failure. Both operations ignored UNKNOWN procfs state.
- Added a strict RUNNING/ABSENT/UNKNOWN `pidof dropbear` parser. Start now refuses UNKNOWN before key/config mutation or launch, returns idempotently when already running, and verifies bounded startup readiness. Stop is an idempotent no-op only for confirmed absence, refuses UNKNOWN before signaling, and confirms the daemon has disappeared after a successful signal; it preserves signal failures if the daemon remains.
- Expanded the existing `check_dropbear()` executable integration with a fake procfs/daemon state, duplicate-start suppression, UNKNOWN and inconsistent pidof outputs, failed/successful stop postconditions, absent-stop idempotence, and launcher-without-daemon readiness failure. The first implementation commit's PID bracket class carried two backslashes before its space, which dash rejects; the dedicated static contract checks the single escaped-space expression before the existing `/bin/sh -n` check. The first rerun then caught a test-authoring mistake: while inserting the Dropbear sleep stub I replaced the first `sleep` fixture in the file (the unrelated VLMCSd fixture), leaving its following shell lines outside a Python string. Restored the VLMCSd fixture verbatim and corrected the Dropbear stub in its own function. This was a Python checker syntax failure; Dropbear behavior had not yet run. Exact-head CI pending.

## 2026-11-22 — DNS Forwarder treats pidof scan failures as UNKNOWN

- Source baseline: `17402ad9b359b388b292c960964ab4224b91def7`. `dns_forwarder_is_running()` reduced `pidof` to a Boolean. On BusyBox, the same status 1 is used for no matching process and for some procfs enumeration failures, so an UNKNOWN process table could authorize duplicate startup or a false successful stop.
- Added strict pidof status/output and positive-PID validation. Start and its readiness polling now accept only confirmed ABSENT before launch and RUNNING after launch; stop and its postcondition polling distinguish confirmed ABSENT from UNKNOWN and refuse to signal or report success on an incomplete inventory.
- Extended the dedicated DNS Forwarder lifecycle test for pidof error and inconsistent status/output cases, asserting neither start-stop-daemon nor killall is invoked while the daemon marker remains. Exact SHA `83706df2...` ran the source checker successfully but the lifecycle integration returned status 2 on its first start; inspection of the dash failure shows the same syntax error as iperf3: an unescaped space in a `case` bracket expression. Escaped the class space and added a `dash -n`-compatible `/bin/sh -n` contract check. The corrected exact-head run is pending.

## 2026-11-22 — iperf3 process-state inventory must distinguish UNKNOWN

- Source baseline: `358aca9453aaf9b5dad4814799d8b560689b4f6f`. `is_running()` treated every nonzero `pidof` result as ABSENT. Since BusyBox `pidof` also returns 1 when its procfs scan fails, start could launch a duplicate server, stop could return success without stopping a live server, and startup/stop postcondition loops could falsely interpret UNKNOWN as success.
- Added strict PID status/output validation (including numeric nonzero PID parsing) and explicit RUNNING/ABSENT/UNKNOWN handling. Start now refuses UNKNOWN before launch and during readiness checks; stop refuses UNKNOWN before signaling, after a failed signal, and in its postcondition polling. Only pidof status 1 with empty output is accepted as ABSENT.
- Extended the existing executable-source integration in `scripts/check_optional_app_start_status.py` with pidof error, status/output mismatch, and no-duplicate/no-signal assertions. Exact SHA `17402ad9...` failed before behavioral tests because the original expression contained two backslashes before the space; exact SHA `1035c916...` then showed that an unescaped space is also rejected by dash's case parser. Both scripts now escape the bracket-class space with one literal backslash (`[!0123456789\\ ]`), and the checker runs `/bin/sh -n` before exercising the controller. The next exact-head run will verify both parse and behavior.
- No local router or firmware build is claimed.

## 2026-11-22 — SmartDNS Adbyby process inventory must be tri-state

- Source baseline: `8ecb9cdbdc34dc62f8b774a227b217afdc47f251`. `Change_adbyby()` piped `pidof adbyby` through `awk`, discarding pidof's status; a failed or malformed process scan therefore became an empty string and was treated as confirmed absence. SmartDNS could then report a successful DNS-owner transition without updating Adbyby's DNSMasq/SmartDNS ownership.
- Generalized the existing strict SmartDNS process-state helper to accept an explicit process name, removed the unused early pidof/awk probe, and made `Change_adbyby()` distinguish RUNNING, confirmed ABSENT, and UNKNOWN. Unknown/inconsistent output now aborts before Adbyby NVRAM reads or DNS ownership changes.
- Extended the exact SmartDNS lifecycle regression to exercise the extracted `Change_adbyby()` function with absent, pidof-error, empty-success, inconsistent, malformed, and running Adbyby states. Exact SHA `8b354207...` passed the process lifecycle inventory and SmartDNS process lifecycle workflows, but the independent DNSMasq integration suite first exposed a missing `smartdns_process_state()` helper in its extracted `Change_adbyby()` fixture. Commit `e805e5e4...` added the helper to both extracted bundles; its rerun then showed every shell integration case passing and exposed only a misplaced `adbyby_change` assignment in the static checker. This was a validation-harness ordering issue, not a product-code failure; the checker is corrected in this follow-up.
- No router or firmware build is claimed.

## 2026-11-22 — Preserve UNKNOWN process state in Ralink IAPP lifecycle

- Source baseline: `55be802fbc344d9809c9f1a9eb283debb6f64ebf`. The existing IAPP controller delegated process discovery to BusyBox `pidof`, whose exit status 1 is also used for an empty result; it therefore could not distinguish confirmed absence from a failed `/proc` scan. Both start and stop trusted that status as ABSENT, so a procfs inventory failure could launch a duplicate daemon or remove its firewall rules while the daemon remained alive.
- Replaced the ambiguous `pidof` probe with a bounded procfs inventory: verify the root can be enumerated, inspect every numeric PID's `stat` record, skip processes that disappear during the scan and zombies/dead tasks, and return UNKNOWN when a still-present process cannot be read or parsed. The service only treats a complete scan with no live `ralinkiappd` as ABSENT.
- Reworked lifecycle fixtures around an injectable procfs root and added regressions for unavailable procfs on start/stop, unreadable per-PID state, and zombie exclusion. Corrected the three new fixture writes to emit actual newline-terminated proc stat records (not literal backslash-n bytes). The process-lifecycle umbrella workflow then exposed its old static checker still requiring the removed `UNKNOWN inventory` text; its contract was updated to assert the new unavailable-procfs, unreadable-stat and zombie cases instead. The dedicated IAPP lifecycle and XY-C1 firmware workflows passed on exact SHA `8ecb9cdb...`; the umbrella rerun is pending.
- Exact-head validation pending.

## 2026-11-22 — QMI credential cleanup inode binding

- Source baseline: `fa03ddb1a15b32552a4ee20dddeee635a468d6f9`. QMI private-credential staging and parser were already integrated; exact USB modem workflow #45073574933, Process lifecycle #45073574919 and NVRAM transaction audit #45073574859 passed on this baseline. XY-C1 run #45073574883 was still in progress at the last poll.
- Found a mismatch between the QMI staging cleanup contract comment and implementation: cleanup checked that the path still named a mode-0600 owner-owned regular file, but did not prove it was the inode created by the staging call. A replaced file at the same private path could therefore be unlinked.
- The staging descriptor now records the created file's device/inode identity. Cleanup refuses to unlink a path whose identity differs, while preserving the expected ENOENT path after uqmi has consumed the original file. Added a regression that holds the original inode open, removes the name, installs a replacement, and verifies cleanup returns EPERM without deleting it. Extended the QMI source contract checker; the existing USB modem workflow already covers all touched paths.
- Exact-head validation for this implementation commit is pending; no local C or firmware build is claimed.

## 2026-11-22 — SmartDNS, AliDDNS and SCUTClient fail-closed follow-up

- Source/test baseline before this handoff refresh: `33a91907446b7854c9736d3246152382dbb5f218`; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remained at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- SmartDNS: `4cd7a0c` checks Adbyby enable/add reads before DNS owner transitions; `8a68095` checks primary and secondary SmartDNS enable/port/redirect reads before lifecycle changes. Regression and source checker extended.
- AliDDNS: `56f1a91` makes startup enable, interval, credentials, domain/record and retry reads status-bearing; `a838773` checks fingerprint state reads/writes; `00f9143` checks retry-cancel reads; `b884b84` checks retry and relaunch NVRAM writes; `a6cad20` checks retry lock writes/removals and rejects unsafe symlink paths. Test/checker follow-ups `dec612d`, `b8c42c7`, `6749633`, `58eb9d5`, `296dd89` repaired mock newline escapes, marker placement and lock fixtures.
- SCUTClient: `33a9190` reads the checked WAN interface before stopping the local client, leaving it running on read failure; it propagates the remote logout command result. Added read-failure, success and logout-failure regression cases to `scripts/tests/legacy_wrapper_pidfile_safety.sh`.
- CI: exact SHA `33a91907446b7854c9736d3246152382dbb5f218`: Legacy wrapper stale-pidfile safety #45066964140 PASS; Process inventory and lifecycle fail-closed #45066964156 PASS; Process lifecycle inventory #45066964164 PASS. XY-C1 Firmware Validation #45066964181 still IN PROGRESS at this checkpoint (build dependency installation); Build/TRX/artifact not yet proven. Previous exact SHA `296dd893b20d14c7487bda4565cd15562f619c86`: AliDDNS #45066583234 PASS and Process inventory #45066583292 PASS. SmartDNS/DNSMasq #45064508066 on `8a68095` PASS.
- Intermediate AliDDNS failures were investigated: 00f914/b884/58eb test issues were fixture newline or test path setup; subsequent fixture fixes passed #45066583234 on 296dd. No unresolved production regression from those CI failures.
- No local test execution or physical-router validation. Next: inspect the bundled SCUTClient upstream archive for an interface that avoids putting authentication credentials in process argv; verify support before changing the wrapper. Recheck exact XY-C1 run #45066964181 and record Build/TRX/artifact independently.

- AI handoff documentation contract #45067172284 initially failed because the refreshed handoff lacked the validator-required `Before context exhaustion` phrase and canonical `Current repository state` / `Active task` / `Verification boundary` headings. The failure was diagnosed from the job log; this corrective checkpoint supplies those required fields.

---


## 2026-11-22 — PID 1 and SmartDNS integration checkpoint

- PID 1 signal-mask initialization now checks `sigemptyset` / `sigaddset` and fails closed before installing handlers or forking the recovery shell; deterministic callback failure tests and source contracts were added.
- WAN teardown workflow policy bindings were corrected after an inline-checker NameError.
- SmartDNS no longer invokes `/usr/bin/adbyby.sh switch`, which is not an implemented Adbyby action; NVRAM persistence and in-memory owner updates remain status-checked.
- Adbyby cleanup preserves the first DNS/firewall cleanup error while still attempting independent cleanup operations; storage swap/spindown handling remains fail-closed.
- Current main: `447d3683e2ae30c7e82b699d89dc978f75b32136`. Exact-SHA status: main USB storage hotplug stability regression=in_progress; XY-C1 Firmware Validation=in_progress; Process lifecycle inventory - 447d3683e2ae30c7e82b699d89dc978f75b32136=completed/success; DNSMasq lifecycle stability regression=completed/success; PID1 `ad1837f1...` AI handoff documentation contract=completed/success; PID 1 mdev hotplug configuration security regression=completed/success; HTTPD Authentication and Security Regression=completed/success; Process inventory and lifecycle fail-closed regression=completed/success; XY-C1 Firmware Validation=completed/success; WAN `8728b518...` WAN interface teardown stability regression=completed/success; AI handoff documentation contract=completed/success.
- Next: test actual SmartDNS/Adbyby runtime configuration ownership in both directions. Hardware/XY-C1 validation is not inferred from host-side CI.

## 2026-11-23 04:22 CST (UTC+8) — Fix Adbyby snapshot cleanup test newline

- Parent main SHA: `a633cf309bd672b2c8111a9c09ba891223f8f1ba`. Adbyby #45061097586 and HTTPD #45061097680 both failed at the shared `firewall_snapshot.sh` test after every preceding regression passed. The final assertion expected an exact marker line, but the fixture used two backslashes before `n` and wrote literal backslash-n bytes.
- Corrected only the fixture to use `printf '%s\\n' 'snapshot-remove-attempted'`. The production `del_rule()` change in `a633cf3` remains unchanged: it attempts all firewall/ipset cleanup but returns the original snapshot-removal failure instead of falsely reporting success.
- Exact parent Process inventory #45061097595 and Process fail-closed #45061097583 PASS. XY-C1 #45061097611 was still running; no L3 claim. Local checkout unavailable; fresh exact-head Adbyby/HTTPD verification required.
- Next independent task after rerun: make `init_signals()` check `sigemptyset()`/`sigaddset()` while constructing handler masks, with injectable error tests.

## 2026-11-23 04:05 CST (UTC+8) — Preserve original /proc/swaps record terminators

- Parent main SHA: 5c929dbe5b40cdc7414bfa9997cf3edce834714e. Both automount_swap_status() and spindown_swap_status() captured procfs with command substitution, which strips trailing LF bytes, then appended a replacement LF before awk. That could normalize a truncated snapshot and falsely prove a disk inactive/unused.
- Both shell readers now append a sentinel inside command substitution, remove exactly that sentinel, require the captured source to retain its final LF, and pass original bytes to awk. Missing final terminators return UNKNOWN.
- Added automount and spindown regressions for a header-only snapshot without LF and a full-looking final swap row without LF. Existing workflow push/PR filters already include both scripts and tests.
- Parent USB storage #45060433291 on 80bfd667 PASS includes the physical-eject parser changes; AI docs #45060286887, Process #45060286908 and HTTPD #45060286954 PASS on 64d3b46. XY-C1 #45060433345 still building; DNSMasq checker-only failure #45060433306 was fixed by 5c929db, rerun #45060566217 in progress. Local execution NOT RUN. Fresh exact-head Storage and XY-C1 required; L3/L4 NOT VERIFIED.

## 2026-11-23 03:30 CST (UTC+8) — Validate physical-eject swap inventory semantics

- Parent main SHA: `4c6d1b9c4c0258641da90f7a1511b775a5fb70ea`. After hardening automount and global `umount_all_storage()` swap inventories, a separate physical-eject parser in `storage_ejected_inventory_policy.h` still accepted any five tokens before the ejected-device `swapoff()` stage.
- Hardened `storage_ejected_parse_swap_line()` to require Type `partition|file`, unsigned decimal Size/Used and signed decimal Priority, rejecting extra fields. Existing octal-decoding token reader remains the path parser.
- Extended the GNU89 ejected-inventory regression with invalid Type/Size/Used/Priority and a valid octal-escaped swap-file path with `+2` priority. The storage source-contract checker now pins these validations; roadmap and handoff updated.
- Exact parent USB storage #45059907996 and AI docs #45059908027 PASS. XY-C1 #45059757452 on ancestor product-source `d37cf8a` was still building at last check; this new production header needs a fresh exact-head Build/TRX/artifact run. Local execution NOT RUN; L3/L4 NOT VERIFIED.

## 2026-11-23 03:14 CST (UTC+8) — Validate semantic fields in global swap cleanup inventory

- Parent main SHA: `9c75e1bf1e13da1dc673a120267827ee1e25c4a5`. Review of `umount_all_storage()` found that its global cleanup parser accepted any five tokens, unlike the strict per-device swap parser. Invalid swap Type or malformed Size/Used/Priority could therefore be treated as a complete snapshot and passed to the `swapoff()` loop.
- Hardened `storage_inventory_parse_swap_line()`: require Type `partition|file`, decimal Size/Used and signed-decimal Priority; reject extra fields while retaining procfs octal path decoding.
- Extended the GNU89 policy regression with each invalid column, extra fields and an escaped swap-file path. Extended `check_storage_safe_remove.py` to enforce the semantic checks; updated the storage roadmap/handoff.
- Parent exact-head Process inventory #45059493600 and HTTPD security #45059493548 PASS. USB Storage #45059493487 and XY-C1 #45059493535 were still in progress; no local execution. This new code needs exact-head Storage and XY-C1 verification; L3/L4 NOT VERIFIED.
- Separate automount awk extraction repair is in `9c75e1b`; check the same exact-head Storage run for it.

## 2026-11-23 03:07 CST (UTC+8) — Fail closed on automount swap inventory errors

- Parent main SHA: `b794d511f0a70d77cc046090b39c416f89c64006`. `automount.sh` previously treated any nonzero `awk` result while reading `/proc/swaps` as proof that the hotplug partition was inactive, so a failed read or malformed inventory could still issue `swapon`.
- Added a tri-state inventory reader that validates the exact five-column header and every complete swap row, including type and numeric fields. Only a complete inventory proving the target inactive reaches `swapon`; UNKNOWN refuses activation, and command failure is now logged.
- Extended `storage_mountpoint_inventory.sh` with active/inactive/exact-device, malformed-header/row, invalid-number, empty, missing-file and non-file inventory cases, plus a caller-order contract. The existing storage workflow already watches both changed paths for push and pull_request; no trigger was weakened or broadened.
- Validation before publication: local execution NOT RUN (no checkout). Exact-head storage, AI-handoff and XY-C1 Actions are pending. XY-C1 L3 and device L4 are NOT VERIFIED.
- Next: resolve exact-head Action failures first; after passing checks, inspect disk-removal swapoff lifecycle in `disk_initial.c` for a separate failure-propagation gap.

## 2026-11-22 — Restore the required Active task handoff section

- Exact-SHA AI handoff run #45052462155 on `4a39de2f31404d94ed342a1468de57b88997151a` accepted the mandatory checkpoint clause and rolling-log rotation, then failed because `SESSION_HANDOFF.md` used `## Active engineering task` instead of the required `## Active task` heading.
- Restored the exact section heading and advanced the handoff parent SHA to `4a39de2f31404d94ed342a1468de57b88997151a`. This is a documentation contract correction; the production account rollback source changes remain on SHA `0502c7ab3b3ad32c0682a0c372b79789174bfe7e`.
- On `0502c7a`, Process regression #45052382237 passed including the account publication GNU89/ASan/UBSan tests and source contract. XY-C1 #45052382239 was still building, with TRX/artifact pending. NVRAM transaction audit #45052382219 was still running.
- No local test or hardware validation was run for this documentation-only correction.

---

## 2026-11-22 — Match the handoff checkpoint sentence to the enforced contract

- Exact source SHA `d1d26ed9305792ba5810634e660420a75ad9c837` AI handoff run #45052323259 passed the 20-entry rolling-log check but failed the handoff contract: `SESSION_HANDOFF.md` lacked the literal sentence `The checkpoint is mandatory; a document edit is conditional.`.
- Added the exact mandatory checkpoint clause while retaining the conditional-document-edit rule. This is a checker-contract correction only; no firmware or production code changed in this slice.
- At last inspection on source SHA `d1d26ed...`, Process regression #45052323143 was cancelled, NVRAM transaction #45052323058 and HTTPD security #45052323146 were running, and XY-C1 #45052323200 was still running. These results do not transfer to the new documentation SHA.
- No local test or hardware validation was run. Check the AI handoff workflow on the new exact SHA; source regressions and XY-C1 production verification remain pending/NOT VERIFIED.

---

## 2026-11-22 09:44 PST (UTC-8) — Rotate bounded development log and identify account rollback defect

- Exact parent main SHA: `ff25eacd70dccd78c6416c76ad199ad69aa8f61d`. The exact-SHA AI handoff workflow #45051742269 failed in `scripts/check_ai_handoff_docs.py` because the root `dev_log.md` contained 120 entries; the previous handoff-only commit did not perform the required rolling-window rotation.
- This atomic documentation slice archives the 101 oldest complete root records, byte-for-byte and oldest-first, in `docs/dev-log/2026/11/part-027.md`; the root retains the newest 20 records. The archive index records its exact entry count, byte size and blob SHA.
- A source review found a recovery gap in `recreate_passwd_unix()`: after publishing replacement passwd/group/shadow files, it deletes the old hard-link backups before checking whether `change_passwd_unix()` successfully installed the configured root password. A chpasswd failure can therefore leave the new locked shadow entry active without a rollback path.
- First engineering task: make passwd/group/shadow publication two-phase, retaining recovery links until password installation succeeds; restore the prior files on password failure and test both successful and failed rollback paths.
- No local tests or hardware validation were run in this documentation-only slice. Exact-SHA CI will be checked after publication.

---

## 2026-11-22 — Update process-lifecycle L2TP source contract

- Exact parent main SHA: `6418e87add0ffce79fd918077bb1e7dfdded2fc0`. The updated modem/WAN-route checkers now pass their import/setup stages, but Process/lifecycle #45050810754 failed at its own stale L2TP assertion: it still expected unchecked `nvram_match()` and a direct `return xl2tp_status`. The production helper had already been hardened to use checked NVRAM status, return the merged WAN/L2TP status, and fail closed on selector read errors.
- Updated the process checker to inspect the checked selector, merged result helper and route-status regression cases. This is checker-only; no production logic changed.
- At the last poll, WAN route #45050810781, USB modem #45050810849 and Firewall #45050810827 had progressed past the previously missing-import issue; Process was the remaining contract failure. XY-C1 #45050810796 still needed final Build, TRX and artifact confirmation.
- Re-run Process and XY validation on a new exact SHA. Do not count L3 until production build, TRX verification and artifact upload all pass; L4 hardware remains NOT VERIFIED.

---

## 2026-11-22 — Repair WAN source-checker runtime imports

- Exact parent main SHA: `9056ed1235425ca1c30657b59ff36c268758a937`. The first exact-SHA validation on 9056 exposed two pure-checker runtime defects: WAN route #45050500613, USB modem #45050500601, Firewall #45050500619 and Process #45050500590 failed because `check_wan_vlan_setup.py` called `re.search` without importing `re`, and `check_modem_wan_start_status.py` referenced `ROUTE_STATUS_TEST` without loading `tests/route_status_policy.c`. These failures occurred before the related source-contract assertions could execute; they do not establish a C implementation failure.
- Added the missing standard-library import and loaded the route regression source used by the L2TP fail-closed assertion. No production C source changed in this slice.
- Re-run WAN route, USB modem, Firewall and Process at the new exact SHA; continue beyond the source-contract stage. XY-C1 #45050500629 was PENDING at last inspection and must complete Build, TRX and artifact upload on an exact SHA before L3 can be marked verified.
- Firmware L3 and hardware L4 remain NOT VERIFIED.

---

## 2026-11-22 — Make WAN selector source contracts whitespace-tolerant

- Parent main SHA: `55e39b6a3756e7cb5fae92cfcc59bced6a61a056`. The previous L2TP checker correction still omitted the single space inserted after a line-broken opening parenthesis by its `compact()` helper. The new HW-NAT VLAN checker had the same issue: its source compactor also joins tokens with one space, so the exact `get_nvram_match_checked_status(` substring was absent despite correct production code. WAN route #45050298583 failed at that checker; Firewall/USB modem checks on the prior HEAD were still blocked at the L2TP assertion.
- Replaced fragile exact-call strings in both `check_modem_wan_start_status.py` and `check_wan_vlan_setup.py` with whitespace-tolerant regular expressions. Kept ordering checks proving selector/VID errors return before physical PVID fallback. No C logic changed.
- Exact new SHA must rerun WAN route, firewall, USB modem and process checks; no result is claimed until those checks pass. XY-C1 run #45050298446 on `b5fbceb` was still building at last inspection.
- The L2TP fail-closed source fix and checked HW-NAT VLAN ID source fix remain production changes awaiting exact-SHA validation.

---

## 2026-11-22 — Match L2TP checker expectation to the actual whitespace-normalizer

- Parent main SHA: `b5fbcebd47fa735f70d06064a8a8cb682f4b0229`. The previous checker-only attempt removed all call whitespace, but the checker helper normalizes it to one space. The first correction therefore still failed: Firewall #45050163151 and USB modem #45050163172 logged AssertionError at line 612. Updated the expected call to the actual normalized C text: `get_nvram_match_checked_status("l2tp_srv_t", "1", &l2tp_enabled)`.
- No production behavior changed. Re-run the modem/firewall/process and XY-C1 checks on the next source-triggered SHA; the checker must progress past line 612 before their full contracts count as PASS.
- Separate concurrent WAN VLAN fix on this parent is in commit `b5fbceb`; exact WAN-route/XY actions still need verification.

---

## 2026-11-22 — Fail closed on WAN VLAN-ID lookup errors for HW NAT

- Parent main SHA: `19af0ec53cb268ab85a70c6f8ec56796b56890d6`. `get_vlan_vid_wan()` used lossy `nvram_match("vlan_filter", "1")` and `nvram_get_int("vlan_vid_cpu")`; transient NVRAM read failures were indistinguishable from a disabled selector/invalid VID and could fall back to the physical switch WAN PVID. The caller then passed that substitute VID into HW-NAT setup, risking an incorrect hardware-offload WAN VLAN.
- Replaced both selectors with checked reads and return a negative errno immediately on either failure, before consulting physical PVID. Configured invalid/missing VID and disabled filtering retain the established physical-PVID fallback. The HW-NAT warning now accurately covers checked VLAN/PVID lookup failures.
- Added a negative-errno policy regression and an exact-source ordering check proving both checked reads and failure exits precede PVID fallback. The WAN route workflow now compiles the regression under GNU89 warnings-as-errors plus ASan/UBSan.
- No local MIPS build or target-device test is claimed. Exact-SHA WAN route and XY-C1 build/TRX/artifact actions must verify this slice.
- Next: follow the WAN VLAN lookup exact-SHA actions and the previously failed WAN-modem/L2TP checker successors.

---

## 2026-11-22 — Keep checked L2TP selector assertion consistent with compacted source

- Parent main SHA: `6c360d4acebe53d916d587328a96d00306a38de1`. The L2TP selector hardening on `0c3d6dc` uses `get_nvram_match_checked_status()` and fail-closes on NVRAM read errors. The WAN startup checker compacts whitespace before asserting the call, but its expected string retained spaces after commas; Firewall, USB modem and Process/XY validation therefore failed at the checker before validating the unchanged C logic.
- Corrected the source-contract literal to match the whitespace-normalized C call. This is a test-only correction; no production source behavior changed.
- Exact-head runs on `6c360d4` failed for this checker assertion: Firewall #45050009351, USB modem #45050009362, Process/lifecycle #45050009357. XY-C1 #45050009356 was running but its LAN restart regression step also stopped at the same checker. Re-run on the new commit; no firmware verification claim from the interrupted run.
- The separate LAN snapshot, NFS ACL snapshot and DHCP callback changes remain as recorded in preceding entries. L3 still requires exact-SHA Build, TRX and artifact upload; L4 hardware not verified.

---

## 2026-11-22 — Align WAN startup checker with checked L2TP selector

- Parent main SHA: `0c3d6dc4f092398bcd1ccb8db2026c4f8d51b703`. Exact-SHA XY-C1, firewall-startup and process-lifecycle runs reached their host regression stage but stopped at `scripts/check_modem_wan_start_status.py:612`: its source contract still required the removed lossy `nvram_match("l2tp_srv_t", "1")` and old early-return implementation.
- Updated the source contract to require the checked NVRAM read, fail-closed start gate, status-merging helper, error log and registered policy regression instead. This is checker-only; production source and its policy tests remain in the preceding commit.
- Exact failing-run details: XY-C1 `45049790490` (failed before firmware Build/TRX/artifact), Firewall `45049790328` and Process inventory `45049790320` (same stale checker assertion). Their exact-head successors must be checked; no firmware result is claimed from these failures.
- Next: inspect the fixed checker's exact-SHA consumers and the WAN route regression, then proceed with the separate WAN VLAN-ID checked-read defect.

---

## 2026-11-22 — Fail closed on unreadable L2TP server enable state

- Parent main SHA: `fbee4e58b90004f7904c6e6f4195c73d7030c0c3`. WAN post-start called `nvram_match("l2tp_srv_t", "1")`; a transient NVRAM read error was indistinguishable from the configured-disabled value, silently skipping the optional L2TP server after WAN activation.
- Replaced that lossy selector with the existing checked NVRAM match reader. An unreadable selector now prevents an uncertain daemon start and is merged into the return status without masking an earlier WAN error. A successfully enabled selector still starts the server even when an earlier WAN step has failed, preserving the previous start-attempt behavior; xl2tpd's `1` already-running result remains successful.
- Added policy regressions for enabled/disabled/read-error decisions, first-error preservation, negative launch failures and the already-running sentinel. The exact-source workflow pins checked-read → start gate → daemon invocation ordering.
- No local firmware build or target hardware test is claimed; exact-SHA WAN route and XY-C1 Actions must establish CI/build coverage.
- Next: inspect exact-SHA WAN route and XY-C1 outcomes, then continue an independent service/configuration boundary.

---

## 2026-11-22 — Repair WAN DHCP callback snapshot regression contract

- Parent main SHA: `1da00c08b8e6b5c51675cbe6138df6c2af9369c9`. The earlier exact-head WAN DHCP run on `f5b93cf324f5d0f46d2218cc6b1e2705c01759ba` failed because its source contract still expected separate LAN address/netmask getters after the callback had moved to one NVRAM snapshot. That stale ordering check was replaced before the current checkpoint.
- The subsequent exact-head run `45049080840` on `1da00c08b8e6b5c51675cbe6138df6c2af9369c9` passed all C policy tests and the added LAN reset/start snapshot checks, then failed on a Python `NameError`: the IPTV snapshot assertion referenced undefined `wan_nvram_test`. The same test file was already read directly with `Path(...).read_text()` for the WAN callback assertion.
- Fixed the remaining assertion to read the test file explicitly. The production change remains transactional: both WAN and IPTV DHCP callbacks snapshot LAN address/netmask together before the subnet-overlap decision. No production behavior was changed in this checker repair.
- Exact-head Actions for the repair commit are pending; no local full workflow, MIPS build, TRX, artifact or hardware verification is claimed here.
- Next: inspect the WAN DHCP callback workflow on the repair commit, then check the NFS ACL source/workflow and XY-C1 exact-SHA Build/TRX/artifact stages that remain relevant to the active handoff.

---

## 2026-11-22 — Generate NFS export ACLs from one NVRAM configuration generation

- Parent main SHA: `cd92a56405e96a08ab9c64e45cc345828ba5a37d`. `write_nfsd_exports()` independently fetched the LAN address/netmask, fallback LAN pair, IPv6 address/prefix, router mode and VPN selectors/address. A concurrent configuration commit could pair an old LAN address with a newer, broader netmask and publish an NFS client CIDR that authorizes hosts outside the intended LAN; IPv6 and VPN ACL inputs could similarly be mixed across generations.
- Added a transaction-backed ACL snapshot for all 12 related NVRAM keys. `nvram_transaction_begin/get/end` keep the tuple coherent, `rc_wan_nvram_copy_snapshot()` copies shared-cache values immediately, and every field is discarded if a read or transaction close fails. The export writer now consumes this snapshot and parses VPN selectors with the checked integer parser before formatting ACLs.
- Added GNU89 regression coverage for one-transaction/cache-slot reuse, missing-key empty defaults, mid-snapshot read failure and transaction-close failure; updated the NFS workflow contracts to require the atomic snapshot and forbid independent VPN/IPv6 getters.
- Parent exact-SHA checks: WAN route #45047446525 PASS; AI handoff #45047446469 PASS. The previous XY-C1 run #45047262320 on `690dc9` had reached Build with TRX/artifact pending. The new `services_stor.c` source change should trigger NFS and XY-C1 validation on the new exact SHA; inspect those runs before making any L3 claim.
- No local tests/build or hardware validation were performed. Firmware L3 and hardware L4 remain NOT VERIFIED.

---

## 2026-11-22 — Keep VPN-only DNS fail-closed during NVRAM faults

- Parent main SHA: `e49224f71f90339247917b66d8a9add54228e8a1`. In `update_resolvconf()`, `nvram_get_int("vpnc_pdns")` and `nvram_safe_get("vpnc_dns_t")` hid GET/read/close failures. With `vpnc_pdns=2` (prefer VPN DNS), a transient backend failure looked like mode 0 or an empty VPN server list and caused fallback to WAN/public DNS, leaking queries outside the VPN.
- Replaced the split unchecked reads with `rc_wan_nvram_copy_int_text_snapshot("vpnc_pdns", "vpnc_dns_t", ...)` using the existing NVRAM transaction callbacks. A missing policy/server key still preserves the legacy zero/empty behavior; a backend, parsing, truncation or transaction-close error logs and returns before staging `/etc/resolv.conf`, so the last-known-good file remains in place.
- Added GNU89 policy regressions for same-generation policy/server capture, failure at either member with both outputs cleared, and absent server value retaining the legacy empty fallback signal. Added source contract requiring the snapshot and early abort before `dnsmasq_file_open`, and rejecting the lossy getters.
- Exact parent WAN route #45047446525 PASS and AI handoff #45047446469 PASS. The XY-C1 production run #45047262320 on source `690dc9` was still in Build, with TRX/artifact pending at last inspection. No local test/build or hardware validation was performed.

---
## 2026-11-22 — Align WAN deferred-watchdog source contract with the existing priority log

- Parent main SHA: `690dc90846b959d05a5ee51e8f5d9cfa5c960408`. Exact WAN route #45047262283 executed the new `di_lost_action` checked-read source contract successfully, then failed in `check_wan_deferred_watchdog_status.py:71` while searching for the modem-priority error log. That checker expected `cannot publish modem priority before automatic WAN reconnect`, but `net_wan.c`'s actual log is `cannot publish modem priority before automatic reconnect`; this mismatch predates the current change and does not indicate a production regression.
- Updated only the stale expected phrase in the deferred-watchdog checker. Rerun exact-head WAN route to confirm this and the newly added Internet-loss action tests pass together.
- On `690dc9`, WAN DHCP callback #45047262363, ZeroTier #45047262287, Samba #45047262278, signal lifecycle #45047262383/#45047262340, Wireless #45047262438, IPv6 service type #45047262463 and AI handoff #45047262377 were PASS. XY-C1 #45047262320 was still starting its build path; TRX/artifact not yet verified.
- No local test/build or hardware validation was performed; L3/L4 remain NOT VERIFIED.

---
## 2026-11-22 — Preserve Internet-loss action NVRAM read failures

- Parent main SHA: `88f33bc3d23fd4b1c4cd909abfcd55d6991f7ab2`. `notify_on_internet_state_changed()` used `nvram_safe_get_int("di_lost_action", 0, 0, 3)`, which internally collapses NVRAM GET ioctl/read/close errors into integer 0. A transient backend fault therefore silently skipped configured automatic reboot/reconnect.
- Replaced the unchecked read with `get_nvram_int_checked_status()` and a policy preserving the old 0..3 bounds and default-zero behavior for absent/out-of-range values. Backend/parse/close failures now suppress only the automatic action and emit a diagnostic rather than masquerading as disabled policy.
- Added GNU89 policy coverage for backend errors, valid actions 2/3, missing/default zero, lower/upper out-of-range fallback and invalid output; extended the WAN DNS source checker to pin checked-read → status guard/log → switch ordering and forbid the old helper.
- Exact parent WAN route #45046759582 PASS and AI handoff #45046759590 PASS. The XY-C1 production run #45046472404 on `b4c5967` remained at Build with TRX/artifact pending at last inspection. No local test/build or hardware check was performed.

---
## 2026-11-22 — Fix SIT regression checker initialization order

- Parent main SHA: `034ddac2a662f27510e6ce4735efdbd9b6a07e80`. WAN route run #45046635467 passed all policy binaries and earlier DNS/PPP/WAN contracts, then failed at the new SIT 6rd test-marker assertion because `sit_policy_test` was assigned below the assertion block. The first b4c5967 route run #45046472456 had already exposed and led to correction of the stale split-getter contract.
- Move the `wan_nvram_status_policy.c` read above the new test-marker assertions. No production source or test behavior changes in this checker-only repair.
- On exact production SHA `b4c59675fa4d34edfe4bb5a28a590aff5b7f2313`, WAN DHCP callback #45046472587 passed including the new snapshot tests; XY-C1 #45046472404 was in Build with TRX/artifact pending at the time of this handoff.
- No local checks or hardware validation were performed. Exact-head WAN route rerun and XY Build/TRX/artifact status remain to be checked.

---
## 2026-11-22 — Update WAN DNS source contract for atomic SIT 6rd tuple

- Parent main SHA: `b4c59675fa4d34edfe4bb5a28a590aff5b7f2313`. Exact WAN DHCP callback #45046472587 PASS, including the GNU89/ASan/UBSan compile and execution of the new two-field 6rd snapshot regression tests.
- WAN route #45046472456 failed only at `scripts/check_wan_dns_read_status.py:1191`, whose older SIT source contract still required the removed `"6rd_size"` and `"6rd_relay"` independent reads in `start_sit_tunnel()`. All preceding policy tests and other WAN checked-read source assertions passed. Updated this contract to require `wan6_read_sit_6rd_parameters()`, the `wan%d_6rd_size`/`wan%d_6rd_relay` pair under `rc_wan_nvram_copy_int_text_snapshot()`, no individual getter calls in the 6rd branch, and snapshot-before-tunnel-mutation ordering.
- On `b4c5967`, the new helper tests passed inside WAN DHCP; the exact WAN route source contract is being rerun on this checker-only commit. XY-C1 #45046472404 was still running through build dependency installation; Build/TRX/artifact status was not yet known at this log entry.
- No local tests/build or hardware check was performed. Firmware L3 and hardware L4 remain NOT VERIFIED.

---
## 2026-11-22 — Read SIT 6rd size and relay from one NVRAM generation

- Parent main SHA: `cee3941a374c410a208f64aef8fa3f214d5d2be1`. The DHCP lease path publishes the 6rd tuple (`addr6`, `6rd_size`, `6rd_relay`) under one NVRAM transaction with rollback, but `start_sit_tunnel()` previously read `wan0_6rd_size` and `wan0_6rd_relay` with separate getters. A lease renewal could commit between those reads, resulting in a mixed-generation prefix length and relay during tunnel activation/replacement.
- Added `rc_wan_nvram_copy_int_text_snapshot()` to copy an integer selector and associated text under one lock, then parse only after a successful transaction close. SIT now uses it for the 6rd size/relay pair before deleting an existing tunnel. Missing size preserves the legacy zero default; malformed integer, backend read, truncation or transaction-close errors discard both outputs and abort before mutation.
- Added GNU89 fake-backend regressions for same-transaction reads, failure at either field, transaction-close failure, missing-size compatibility and malformed-size rejection. Updated the SIT source contract to require the transactional pair and ensure independent getters are absent from the 6rd branch.
- Exact parent `cee3941a374c410a208f64aef8fa3f214d5d2be1` Actions: WAN route #45045448318 PASS; HTTPD security #45045448442 PASS; firewall #45045448338 PASS; USB modem identity #45045448331 PASS; Process lifecycle #45045448370 PASS; AI handoff #45045448321 PASS. XY-C1 #45045448352 passed host tests but skipped Build/TRX/artifact because no production source changed on that parent SHA. The present `net_wan6.c` source change should trigger full XY-C1 validation; inspect exact SHA.
- No local tests or firmware build were run. L3 and hardware L4 remain NOT VERIFIED pending exact-source Build/TRX/artifact and physical-router checks.

---
## 2026-11-22 — Align WAN route contracts with optional-device selection

- Parent main SHA: `45a78362cb1c4a6082514049d309508f80e9e28a`; production change is `b56e0b5c14a7b15dfe39a973dfa38eea7b2dda70`.
- The b56e0b5 route policy test printed PASS under WAN route #45045047588, HTTPD security #45045047671, firewall #45045047557, modem identity #45045047641, process lifecycle #45045047662 and XY-C1 #45045047511. The jobs failed only after the C test, in stale Python/source checks still pinned to the removed two-element assumption: `check_wan_dns_read_status.py:514` required `route_ifnames[0]=man_ifname`; `check_modem_wan_start_status.py:641` required `route_ifnames,2,...`; `httpd-security.yml:1389` required that same fixed count. Firewall, process and XY-C1 failures also reached the modem checker. No production source regression was exposed by those failures.
- Updated WAN DNS-read, modem WAN-start and HTTPD inline source contracts to require the route selector to run before `stop_wan()`, include the bridge unconditionally, include the manual interface only when nonempty, pass the selected count into the unchanged strict flush helper, and retain regression coverage for unset/configured interface cases. The XY path filter includes `scripts/check_modem_wan_start_status.py`.
- Exact b56e0b5 C route selector tests did pass in all five affected host workflows. All source-check reruns must be assessed on a single exact current SHA; the older XY-C1 job failed in host validation before Build/TRX/artifact, so L3 remains NOT VERIFIED. No local tests or firmware build were run; hardware L4 remains NOT VERIFIED.

---

## 2026-11-22 — Keep full WAN restart safe when manual interface is unset

- Parent main SHA: `11576ae4e50f8bb063682bbb94929a7bae394a47`. Current code explicitly accepts an empty `wan0_ifname` and sets `man_ifname[0] = '\\0'`, but then passed that empty string as one of two required devices to `rc_wan_route_flush_and_cache_apply()`. The policy correctly returns `EINVAL` for empty device names; this happened only after `stop_wan()`, so a restart with no manual WAN device could stop services, flush the global cache, then abort before the remaining restart sequence.
- Added a route-selection policy that always includes the required LAN bridge and includes the manual WAN device only when nonempty. The selection/validation now occurs before `update_router_mode()` and `stop_wan()`; the existing strict flush helper still rejects empty names. Added host regressions for empty optional device, configured two-device order, missing bridge and insufficient capacity, plus exact source-contract assertions.
- Exact SHA `11576ae4e50f8bb063682bbb94929a7bae394a47` checks PASS: HTTPD #45044509138, AliyunDrive #45044509155, AI handoff #45044509168, Process inventory #45044509233, NVRAM boundary #45044509202. NVRAM regression included the newly-added checked-get CLI test. XY-C1 push workflow did not trigger on that SHA because path filters omitted the AliyunDrive and NVRAM source locations; older #45043730811 passed on product source SHA `3556d297e887ad81b5457e16a33b32eafadafa9a` and does not cover `11576ae`. The present `net_wan.c` change is included in the XY-C1 path filter.
- No local C test, firmware build, TRX/artifact or hardware validation was performed during this session. L3/L4 remain NOT VERIFIED until same-source build/TRX/artifact and physical-router checks respectively.

---

## 2026-11-22 — Distinguish NVRAM backend errors in AliyunDrive startup

- Parent main SHA: `26d91277af7c0983a4a031f280bdef72ed4010ba`. The legacy CLI `nvram get` uses `nvram_get()`, which collapses missing keys and `/dev/nvram` open/ioctl/close failures to NULL with status 0. A transient error reading `aliyundrive_enable` could therefore be treated as disabled and stop a healthy daemon; failures for other settings or credentials could silently become empty values.
- Added an opt-in `nvram get_checked KEY` command without changing legacy `get`. It calls `nvram_get_checked()`, preserves errno, returns success/no output for absent keys, and returns nonzero for backend errors, invalid API statuses or a present/null inconsistency. Added GNU89 status-mapping regression tests and source-contract checks to the NVRAM boundary workflow.
- Added `ald_nvram_get()` and routed all 13 AliyunDrive startup settings through it. Failed enable reads now abort before the stop path; all other failed reads abort before workdir permission changes and daemon launch. Lifecycle tests inject an enable-read error while a daemon is running and an error for every startup setting, asserting no signal, launch, readiness wait or workdir mode change.
- Exact parent checks on `26d91277af7c0983a4a031f280bdef72ed4010ba`: AliDDNS signed API #45044076046 PASS and AI handoff #45044076040 PASS. The older AliyunDrive source-check failure after `72ffcb3` was caused by the checker selecting the duplicate-start `ald_wait_started` occurrence instead of the daemon-start readiness call; that checker logic was updated in later main history. No local C/shell test or firmware build was run.
- Exact-head NVRAM CLI, NVRAM boundary, AliyunDrive lifecycle and XY-C1 Build/TRX/artifact workflows are required. L3 and hardware L4 remain NOT VERIFIED.

---
