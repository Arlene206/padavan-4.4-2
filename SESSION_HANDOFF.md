## Current repository state

- Repository: Arlene206/padavan-4.4-2. Development and commits remain restricted to main; refetch the live head before every write. Latest exact source/test checkpoint is 6e384bf903ad6b856ed0054fe0f07fc41869798e; this follow-up synchronizes the source checker with the current credential/log-filter failure marker. Only main is present; legacy PR #1 remains CLOSED/unmerged and GPT Session Rotator remains untouched.
- Remote branch inventory at this checkpoint contains only `main`. Legacy PR #1 remains CLOSED and unmerged; its useful AliDDNS source/helper/package/host tests were already integrated on main. Do not cherry-pick or reopen it. GPT Session Rotator remains immutable.
- Recent AliDDNS commit chain: `1931488bc4767b7abcd4710bee3c6fcce00eb65d` implemented exact keeper PID matching and bounded TERM/KILL; `a8e30e38cf2c35387e5c7e409b314eb03f8878dd` is a tree-identical empty commit (no source change); `f19e8e43369d3e3ae9dba507573626960a7e0478` corrected the self-PID test fixture; `c30e9b29abc093963a4276ca22eeb2c7e18f1001` fixed the command-substitution argv-boundary handling and an obsolete HTTPD workflow assertion; `8eec765e9a91c617eb2eb6fab582321a8e04966f` tightened the keeper action to the final argv token and added an extra-argument decoy; `70db3735b4c129622914cee610ac02e8b99fb017` corrected the semantic checker to assert that exact contract.
- The previous AliyunDrive WebDAV lifecycle task remains L2-closed on exact SHA `18234ff0a41eb5343aaf58207b3cf641bbf4a244`: lifecycle run [43287688178] / job [131425308994] passed bounded stop, duplicate-start/argv integrity, serialized transitions and asynchronous process-presence readiness. This does not prove WebDAV HTTP readiness.
- Exact latest source/test checkpoint Actions on 6e384bf903ad6b856ed0054fe0f07fc41869798e:
  - AliyunDrive WebDAV lifecycle [43327886559] / job [131564888160] — FAILURE only in the final source/RC checker. Shell syntax, QEMU version/help capability and the complete host regression passed: private workdir/token 0700/0600, owner/symlink/hardlink rejection, umask 077, RUST_LOG, exact credential env/no-argv, bounded stop/start, duplicate-start and lifecycle lock/readiness. The semantic checker required the obsolete failure string "FAIL: credentials were not passed as three exact environment values"; the fixture now reports "FAIL: credentials and log filter were not passed as exact environment values".
  - AI handoff docs [43327886525] / job [131564888272] — SUCCESS; root log=85/119; archive inventory dev-log=7, handoff=22, roadmap=18.
  - AliDDNS [43327886574] / job [131564887809], AdGuardHome [43327886676] / job [131564888329], and infosvr [43327886567] / job [131564888347] — all SUCCESS on the same exact SHA.
  - This follow-up changes only the checker marker to match the host regression's actual failure contract.

- Earlier checkpoint workflow history (not current-head evidence):
  - AliyunDrive WebDAV lifecycle/source contract: run [43312330424] / job [131511324570] — SUCCESS on `0460f705b6ea3bbe1bcc9cfa61d66b677e5612da`. Shell syntax, bounded stop, duplicate-start/argv integrity, lifecycle lock, asynchronous process-presence startup and RC/source contracts passed. The exact-binary checker warned that the bundled daemon lacks its required credential environment-variable interface; this is not a credential-redaction pass.
  - AI handoff docs: run [43312330419] / job [131511324768] — FAILURE on the same SHA because `SESSION_HANDOFF.md` lacked the required `## Active task

AliyunDrive credential argv redaction is L2-verified on exact SHA dbf15f4bd8472da954a9f5ce68e4b4b57f09f2bb. The bundled v1.2.1 daemon's environment bindings were QEMU-verified; all five path-triggered workflows passed at that checkpoint.

The token-at-rest follow-up reviews upstream v1.2.1 persistence (create_dir_all + tokio::fs::write, no explicit file mode) and an INFO event that contains the rotated refresh token. Commit 2cce075b10faf5b044b5a3b75bb06ae7335dd0af hardened the workdir/token with owner/symlink/hardlink checks and modes 0700/0600, runs the daemon with umask 077 and RUST_LOG=aliyundrive_webdav=warn, and closes FD 9 inside the background subshell before daemon exec.

The host regression was corrected to use its isolated ALD_WORK_DIR in a67610618da02275f4bad8433e5f6755a7f3d846. The source checker was corrected for the dynamic workdir and FD-9 background subshell in 2ebbd0eba0bfbdb09ae1361f2194bac600c29b34. On exact SHA 6e384bf903ad6b856ed0054fe0f07fc41869798e, shell syntax, QEMU and all host tests passed, including private workdir/token modes, symlink/hardlink, umask, log filter, credential environment/argv privacy and lifecycle. The last checker run failed only because its expected failure-marker inventory still named the old credential-env error text; the test now emits the more specific "credentials and log filter" marker. This follow-up updates that one checker marker; no launcher or host-test behavior changes.

Exact-head Actions for this checker change are PENDING. No token-at-rest L2 closure until the full semantic checker passes along with the host/QEMU tests on one exact SHA. Privileged root can still read /proc/PID/environ; process presence is not WebDAV HTTP readiness. After green, continue with FTP cleanup TOCTOU, NFS client evidence and FRP/Adbyby whole-tree audit. Current-main L3 firmware/TRX and L4 hardware remain NOT VERIFIED.

## Completed task: AliDDNS lifecycle process identity and bounded stop

Traced the HTTPD `variables.c` → PID 1 `rc.c` → `services.c` → `/usr/bin/aliddns.sh` control route. The previous `kill_ps()` selected processes with broad `ps | grep | awk | eval` matching and sent TERM/KILL without confirming exit.

Current implementation:
1. Removes generic `kill_ps()`; enumerate `/proc/[0-9]*/cmdline` and match only the exact launcher path followed by the final `keep` argument. Convert NUL separators to newlines and restore the trailing boundary that command substitution strips. Reject nonnumeric/reserved PIDs and the current shell.
2. Revalidate the exact argv immediately before signalling. Send TERM, allow at most five one-second intervals, send KILL only to still-matching keeper PIDs, then allow at most two more seconds and return failure if the keeper remains.
3. Make stop failure a hard gate: a failed close prevents status mutation or replacement keeper startup. Restart/start/stop/check still run through the stable-inode lifecycle lock and the RC restart remains one atomic locked action.
4. The host regression covers same-path/final-`keep` matching, current-shell exclusion, similar-path and wrong-action decoys, extra argv rejection, graceful TERM, bounded KILL escalation and stubborn-process failure preventing restart. The HTTPD umbrella contract now validates the exact keeper-stop helpers rather than requiring the removed grep-based `kill_ps`.

Intermediate CI failures were investigated, not hidden: the first regression failed because the test fixture used a literal dollar path instead of the test shell PID; the next exposed command substitution removing the trailing newline; the next semantic check itself was corrected to assert the final-argument pattern and literal `$$` current-shell exclusion. Current code/test/checker exact-head run [43306180515] is green. The HTTPD workflow's obsolete `kill_ps "$ALIDDNS_HELPER"` assertion was replaced with checks for `aliddns_stop_keeper`, `aliddns_keeper_pid_matches` and absence of `kill_ps`; HTTPD run [43305744811] is green on the same production source tree.

## Verification boundary

- L1: required bootstrap/protocol and recovery sequence completed; main/head, handoff, roadmap, dev log and archive index reviewed. QEMU verifies the exact packaged v1.2.1 MIPSel executable and all three credential env bindings. Upstream source confirms token persistence uses default umask/mode and INFO includes the rotated token. Launcher implements private workdir/token checks, child umask 077, INFO filtering and early FD-9 closure.
- L2 credential argv redaction: exact SHA dbf15f4bd8472da954a9f5ce68e4b4b57f09f2bb; AliyunDrive [43321989293], docs [43321989151], AliDDNS [43321989235], AdGuardHome [43321989308] and infosvr [43321989090] all SUCCESS.
- L2 token-at-rest attempt 6e384bf903ad6b856ed0054fe0f07fc41869798e: shell syntax, QEMU and full host security/lifecycle tests PASS; checker failed only on an outdated negative-marker string for the credential/log-filter comparison. Docs [43327886525], AliDDNS [43327886574], AdGuardHome [43327886676], and infosvr [43327886567] SUCCESS on same SHA. This commit corrects the checker inventory; exact-head reruns pending.
- No local full checkout or local full-project shell/Python test run is claimed.
- L3 current-main firmware/TRX and L4 hardware: NOT VERIFIED.
- Residual: privileged /proc/PID/environ, target BusyBox ash/procfs timing, HTTP readiness and Linux 4.4 process identity races.

## Exact first action for the next GPT

Fetch live main and inspect the exact-head AliyunDrive source/RC semantic checker. Confirm its negative-marker list matches the current "credentials and log filter" test failure and that no other source/test marker mismatch remains. Require QEMU, every private workdir/mode/symlink/hardlink/umask/RUST_LOG/argv/lifecycle case, the complete source checker and all path-filtered workflows to pass on the same SHA. If green, continue FTP cleanup TOCTOU, then NFS and FRP/Adbyby. Current-main firmware/TRX L3 and L4 hardware remain NOT VERIFIED.
