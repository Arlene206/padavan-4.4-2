## Current repository state

- Repository: Arlene206/padavan-4.4-2. Development and commits remain restricted to main; refetch the live head before every write. Latest exact source/test checkpoint is a67610618da02275f4bad8433e5f6755a7f3d846; this follow-up aligns the source checker with the dynamic workdir argument. Only main is present; legacy PR #1 remains CLOSED/unmerged and GPT Session Rotator remains untouched.
- Remote branch inventory at this checkpoint contains only `main`. Legacy PR #1 remains CLOSED and unmerged; its useful AliDDNS source/helper/package/host tests were already integrated on main. Do not cherry-pick or reopen it. GPT Session Rotator remains immutable.
- Recent AliDDNS commit chain: `1931488bc4767b7abcd4710bee3c6fcce00eb65d` implemented exact keeper PID matching and bounded TERM/KILL; `a8e30e38cf2c35387e5c7e409b314eb03f8878dd` is a tree-identical empty commit (no source change); `f19e8e43369d3e3ae9dba507573626960a7e0478` corrected the self-PID test fixture; `c30e9b29abc093963a4276ca22eeb2c7e18f1001` fixed the command-substitution argv-boundary handling and an obsolete HTTPD workflow assertion; `8eec765e9a91c617eb2eb6fab582321a8e04966f` tightened the keeper action to the final argv token and added an extra-argument decoy; `70db3735b4c129622914cee610ac02e8b99fb017` corrected the semantic checker to assert that exact contract.
- The previous AliyunDrive WebDAV lifecycle task remains L2-closed on exact SHA `18234ff0a41eb5343aaf58207b3cf641bbf4a244`: lifecycle run [43287688178] / job [131425308994] passed bounded stop, duplicate-start/argv integrity, serialized transitions and asynchronous process-presence readiness. This does not prove WebDAV HTTP readiness.
- Exact latest source/test checkpoint Actions on a67610618da02275f4bad8433e5f6755a7f3d846:
  - AliyunDrive WebDAV lifecycle [43326575598] / job [131560419812] — FAILURE only in the final source/RC checker. Launcher shell syntax, QEMU execution of the packaged v1.2.1 MIPSel binary and SHA-bound credential-env capability report all passed. The expanded host regression passed private workdir creation and 0700/0600 mode repair, symlink/hardlink rejection, inherited umask 077, exact RUST_LOG and credentials, bounded stop/start, argv privacy, duplicate-start and lock/readiness cases. The checker still expected the old literal --workdir "/tmp/$NAME" even though the launcher now correctly uses --workdir "$ALD_WORK_DIR".
  - AI handoff docs [43326575671] / job [131560419539] — SUCCESS; root log=83/119; archive inventory dev-log=7, handoff=22, roadmap=18.
  - AliDDNS [43326575555] / job [131560419469], AdGuardHome [43326575649] / job [131560419668], and infosvr [43326575524] / job [131560419403] — all SUCCESS on the same exact SHA.
  - This follow-up updates only the stale source-checker expectation; production launcher and host regression are unchanged.

- Earlier checkpoint workflow history (not current-head evidence):
  - AliyunDrive WebDAV lifecycle/source contract: run [43312330424] / job [131511324570] — SUCCESS on `0460f705b6ea3bbe1bcc9cfa61d66b677e5612da`. Shell syntax, bounded stop, duplicate-start/argv integrity, lifecycle lock, asynchronous process-presence startup and RC/source contracts passed. The exact-binary checker warned that the bundled daemon lacks its required credential environment-variable interface; this is not a credential-redaction pass.
  - AI handoff docs: run [43312330419] / job [131511324768] — FAILURE on the same SHA because `SESSION_HANDOFF.md` lacked the required `## Active task

AliyunDrive credential argv redaction is L2-verified on dbf15f4bd8472da954a9f5ce68e4b4b57f09f2bb: QEMU confirmed the bundled v1.2.1 daemon supports REFRESH_TOKEN, WEBDAV_AUTH_USER and WEBDAV_AUTH_PASSWORD environment variables; the launcher no longer includes credentials in argv. All five exact-head workflows passed at that checkpoint.

The follow-up token-at-rest review found upstream v1.2.1 persists the rotated refresh_token with create_dir_all() and tokio::fs::write(), allowing permissions to depend on umask/default mode, and emits the rotated token in an INFO event. Commit 2cce075b10faf5b044b5a3b75bb06ae7335dd0af added fail-closed workdir/token hardening (directory 0700, token 0600; owner, regular-file, symlink and single-hardlink checks), child umask 077, RUST_LOG=aliyundrive_webdav=warn, and early closure of inherited FD 9 to avoid retaining the lifecycle lock.

The first exact-head host run exposed a stale expected-workdir path; commit a67610618da02275f4bad8433e5f6755a7f3d846 corrected that fixture. On a676, shell syntax and QEMU passed, and the host regression then passed all new workdir/mode/symlink/hardlink/umask/log-filter cases plus bounded stop/start, argv privacy, duplicate-start and lock/readiness. The final source/RC checker failed because it still required the old literal --workdir "/tmp/$NAME" rather than the new --workdir "$ALD_WORK_DIR". This is a stale source-checker assertion; no production or host-regression step failed. This follow-up updates that one required source string; production and test behavior are unchanged.

Exact-head Actions for this checker correction are PENDING. No L2 closure is claimed for token-at-rest hardening until QEMU, all host cases and the complete source/RC checker pass on one SHA. Privileged root can still read environment values via /proc/PID/environ; process presence does not prove WebDAV HTTP readiness.

Next after green: continue the whole-repository stability queue with FTP failed-upload cleanup TOCTOU design (remain OPEN/BLOCKED until STOR/APPE/REST/STOU and concurrent external-writer semantics are race-safe), followed by NFS client evidence and FRP/Adbyby review. Current-main L3 firmware/TRX and L4 hardware remain NOT VERIFIED.

## Completed task: AliDDNS lifecycle process identity and bounded stop

Traced the HTTPD `variables.c` → PID 1 `rc.c` → `services.c` → `/usr/bin/aliddns.sh` control route. The previous `kill_ps()` selected processes with broad `ps | grep | awk | eval` matching and sent TERM/KILL without confirming exit.

Current implementation:
1. Removes generic `kill_ps()`; enumerate `/proc/[0-9]*/cmdline` and match only the exact launcher path followed by the final `keep` argument. Convert NUL separators to newlines and restore the trailing boundary that command substitution strips. Reject nonnumeric/reserved PIDs and the current shell.
2. Revalidate the exact argv immediately before signalling. Send TERM, allow at most five one-second intervals, send KILL only to still-matching keeper PIDs, then allow at most two more seconds and return failure if the keeper remains.
3. Make stop failure a hard gate: a failed close prevents status mutation or replacement keeper startup. Restart/start/stop/check still run through the stable-inode lifecycle lock and the RC restart remains one atomic locked action.
4. The host regression covers same-path/final-`keep` matching, current-shell exclusion, similar-path and wrong-action decoys, extra argv rejection, graceful TERM, bounded KILL escalation and stubborn-process failure preventing restart. The HTTPD umbrella contract now validates the exact keeper-stop helpers rather than requiring the removed grep-based `kill_ps`.

Intermediate CI failures were investigated, not hidden: the first regression failed because the test fixture used a literal dollar path instead of the test shell PID; the next exposed command substitution removing the trailing newline; the next semantic check itself was corrected to assert the final-argument pattern and literal `$$` current-shell exclusion. Current code/test/checker exact-head run [43306180515] is green. The HTTPD workflow's obsolete `kill_ps "$ALIDDNS_HELPER"` assertion was replaced with checks for `aliddns_stop_keeper`, `aliddns_keeper_pid_matches` and absence of `kill_ps`; HTTPD run [43305744811] is green on the same production source tree.

## Verification boundary

- L1: bootstrap/protocol and prescribed recovery sequence completed; current head, handoff, roadmap, root log and archive index reviewed. QEMU confirms the exact packaged v1.2.1 MIPSel binary and all three credential env bindings. Upstream source review confirmed the token-file umask/default mode and token-bearing INFO event. Workdir/token owner/mode/symlink/hardlink checks, umask 077, log filter and FD 9 closure are in launcher source.
- L2 credential argv redaction: exact SHA dbf15f4bd8472da954a9f5ce68e4b4b57f09f2bb; AliyunDrive [43321989293], AI docs [43321989151], AliDDNS [43321989235], AdGuardHome [43321989308] and infosvr [43321989090] all SUCCESS.
- L2 token-at-rest attempt a67610618da02275f4bad8433e5f6755a7f3d846: shell syntax, QEMU, and complete host lifecycle/privacy/workdir permission/symlink/hardlink/umask/env tests PASS. Source/RC checker failed only on the stale literal workdir argument expectation; AI docs [43326575671], AliDDNS [43326575555], AdGuardHome [43326575649], and infosvr [43326575524] SUCCESS. This commit corrects that checker assertion; exact-head Actions pending.
- No complete local repository checkout was available; no local full-project shell/Python test run is claimed.
- L3 current-main firmware/TRX: NOT VERIFIED. The old ancestor XY-C1 build is not current-main evidence.
- L4 router flashing, boot and hardware runtime: NOT VERIFIED.
- Residual: privileged /proc/PID/environ, target BusyBox ash/procfs timing, WebDAV HTTP readiness, Linux 4.4 PID-reuse concerns, L3/L4.

## Exact first action for the next GPT

Fetch live main and inspect the AliyunDrive workflow on the exact SHA. Confirm the source/RC checker now expects the configurable ALD_WORK_DIR while keeping the tests for credential env/no-argv, 0700/0600 storage, owner/symlink/hardlink rejection, umask 077, RUST_LOG filtering, lifecycle stop/start and FD 9 release. Verify all other path-triggered workflows on the same SHA. If green, proceed to the queued FTP cleanup TOCTOU design and then NFS/FRP/Adbyby. Current-main XY-C1 firmware/TRX and L4 hardware remain NOT VERIFIED.
