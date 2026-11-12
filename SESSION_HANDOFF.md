## Current repository state

- Repository: Arlene206/padavan-4.4-2. Development and commits remain restricted to main; refetch the live head before every write. Latest exact source/test checkpoint is 2cce075b10faf5b044b5a3b75bb06ae7335dd0af; the current AliyunDrive workdir-test fixture correction is based directly on that SHA. Only main is present; legacy PR #1 remains CLOSED/unmerged and GPT Session Rotator remains untouched.
- Remote branch inventory at this checkpoint contains only `main`. Legacy PR #1 remains CLOSED and unmerged; its useful AliDDNS source/helper/package/host tests were already integrated on main. Do not cherry-pick or reopen it. GPT Session Rotator remains immutable.
- Recent AliDDNS commit chain: `1931488bc4767b7abcd4710bee3c6fcce00eb65d` implemented exact keeper PID matching and bounded TERM/KILL; `a8e30e38cf2c35387e5c7e409b314eb03f8878dd` is a tree-identical empty commit (no source change); `f19e8e43369d3e3ae9dba507573626960a7e0478` corrected the self-PID test fixture; `c30e9b29abc093963a4276ca22eeb2c7e18f1001` fixed the command-substitution argv-boundary handling and an obsolete HTTPD workflow assertion; `8eec765e9a91c617eb2eb6fab582321a8e04966f` tightened the keeper action to the final argv token and added an extra-argument decoy; `70db3735b4c129622914cee610ac02e8b99fb017` corrected the semantic checker to assert that exact contract.
- The previous AliyunDrive WebDAV lifecycle task remains L2-closed on exact SHA `18234ff0a41eb5343aaf58207b3cf641bbf4a244`: lifecycle run [43287688178] / job [131425308994] passed bounded stop, duplicate-start/argv integrity, serialized transitions and asynchronous process-presence readiness. This does not prove WebDAV HTTP readiness.
- Exact latest source/test checkpoint Actions on 2cce075b10faf5b044b5a3b75bb06ae7335dd0af:
  - AliyunDrive WebDAV lifecycle [43325869188] / job [131558034165] — FAILURE at the host regression's exact argv comparison. Shell syntax and QEMU SHA-bound daemon env capability both passed. The test reached the new workdir path assertion but still expected the former fixed path /tmp/aliyundrive-webdav; the launcher correctly supplied the test-overridden ALD_WORK_DIR. The source/RC checker was skipped after this test failure, so token mode/umask assertions later in that fixture are not yet L2-verified.
  - AI handoff docs [43325869229] / job [131558034089] — SUCCESS; root log=82/119; archive inventory dev-log=7, handoff=22, roadmap=18.
  - AliDDNS [43325869222] / job [131558033911], AdGuardHome [43325869134] / job [131558033780], and infosvr [43325869414] / job [131558034824] — all SUCCESS on this exact SHA.
  - The current follow-up changes the quoted expected-argv heredoc to expand the test's ALD_WORK_DIR, so it remains aligned with the isolated temp path rather than the production default.


- Earlier checkpoint workflow history (not current-head evidence):
  - AliyunDrive WebDAV lifecycle/source contract: run [43312330424] / job [131511324570] — SUCCESS on `0460f705b6ea3bbe1bcc9cfa61d66b677e5612da`. Shell syntax, bounded stop, duplicate-start/argv integrity, lifecycle lock, asynchronous process-presence startup and RC/source contracts passed. The exact-binary checker warned that the bundled daemon lacks its required credential environment-variable interface; this is not a credential-redaction pass.
  - AI handoff docs: run [43312330419] / job [131511324768] — FAILURE on the same SHA because `SESSION_HANDOFF.md` lacked the required `## Active task

The earlier AliyunDrive credential argv-redaction change is L2-verified on exact SHA dbf15f4bd8472da954a9f5ce68e4b4b57f09f2bb: QEMU confirmed REFRESH_TOKEN, WEBDAV_AUTH_USER, and WEBDAV_AUTH_PASSWORD env bindings; the launcher no longer puts those credentials in argv; the exact-head host/source checks and five push workflows passed.

The next source review of upstream aliyundrive-webdav v1.2.1 identified an at-rest and logging issue. Its drive module persists the rotated token with create_dir_all() and tokio::fs::write(), relying on process umask/default mode for a new file and preserving existing file modes. Its main module logs the rotated refresh token at INFO. This is the basis for the current workdir hardening: reject workdir/token symlinks, foreign ownership, non-regular token paths and multiply-linked token files; set workdir 0700 and token 0600; run the child with umask 077; set RUST_LOG=aliyundrive_webdav=warn; close inherited lifecycle-lock FD 9 in the child before daemon exec.

The implementation commit 2cce075b10faf5b044b5a3b75bb06ae7335dd0af passed launcher syntax and QEMU capability validation, but its host test stopped at an out-of-date expected argv fixture: the test still expected the production /tmp/aliyundrive-webdav path while the launcher correctly used the test's isolated ALD_WORK_DIR. The source checker and later host assertions were skipped, so this commit is NOT yet L2-closed. This follow-up changes the expected-argv heredoc to expand ALD_WORK_DIR; it does not alter production code.

After the correction, require a full rerun of shell syntax, QEMU exact-binary capability, workdir/owner/mode/symlink/hardlink tests, inherited umask and RUST_LOG, lifecycle/argv regression and the source/RC checker on one exact SHA. Environment credentials remain visible to privileged root via /proc/PID/environ; process presence is not WebDAV HTTP readiness.

Next after green exact-head Actions: review FTP failed-upload cleanup TOCTOU (remain OPEN/BLOCKED until STOR/APPE/REST/STOU and concurrent external-writer semantics are race-safe), then NFS real-client evidence and the broader FRP/Adbyby audit. Current-main L3 firmware/TRX and L4 hardware remain NOT VERIFIED.

## Completed task: AliDDNS lifecycle process identity and bounded stop

Traced the HTTPD `variables.c` → PID 1 `rc.c` → `services.c` → `/usr/bin/aliddns.sh` control route. The previous `kill_ps()` selected processes with broad `ps | grep | awk | eval` matching and sent TERM/KILL without confirming exit.

Current implementation:
1. Removes generic `kill_ps()`; enumerate `/proc/[0-9]*/cmdline` and match only the exact launcher path followed by the final `keep` argument. Convert NUL separators to newlines and restore the trailing boundary that command substitution strips. Reject nonnumeric/reserved PIDs and the current shell.
2. Revalidate the exact argv immediately before signalling. Send TERM, allow at most five one-second intervals, send KILL only to still-matching keeper PIDs, then allow at most two more seconds and return failure if the keeper remains.
3. Make stop failure a hard gate: a failed close prevents status mutation or replacement keeper startup. Restart/start/stop/check still run through the stable-inode lifecycle lock and the RC restart remains one atomic locked action.
4. The host regression covers same-path/final-`keep` matching, current-shell exclusion, similar-path and wrong-action decoys, extra argv rejection, graceful TERM, bounded KILL escalation and stubborn-process failure preventing restart. The HTTPD umbrella contract now validates the exact keeper-stop helpers rather than requiring the removed grep-based `kill_ps`.

Intermediate CI failures were investigated, not hidden: the first regression failed because the test fixture used a literal dollar path instead of the test shell PID; the next exposed command substitution removing the trailing newline; the next semantic check itself was corrected to assert the final-argument pattern and literal `$$` current-shell exclusion. Current code/test/checker exact-head run [43306180515] is green. The HTTPD workflow's obsolete `kill_ps "$ALIDDNS_HELPER"` assertion was replaced with checks for `aliddns_stop_keeper`, `aliddns_keeper_pid_matches` and absence of `kill_ps`; HTTPD run [43305744811] is green on the same production source tree.

## Verification boundary

- L1: required bootstrap/protocol and recovery sequence completed; live main, handoff, roadmap, root log, archives/index and branch inventory reviewed. Exact packaged v1.2.1 MIPSel binary is run under QEMU; its help confirms all three credential env bindings. Upstream source review confirmed token-file write mode depends on umask and a rotated token is included in an INFO event. This work hardens the workdir/token modes, rejects symlink/hardlink/ownership anomalies, sets umask 077, filters INFO and closes inherited child FD 9.
- L2 credential argv redaction: exact SHA dbf15f4bd8472da954a9f5ce68e4b4b57f09f2bb; AliyunDrive [43321989293] / job [131544724607], AI docs [43321989151] / job [131544725051], AliDDNS [43321989235], AdGuardHome [43321989308], and infosvr [43321989090] all SUCCESS.
- L2 token-at-rest hardening attempt 2cce075b10faf5b044b5a3b75bb06ae7335dd0af: shell syntax and QEMU capability PASS; host regression failed only at the stale expected workdir argv line (/tmp/aliyundrive-webdav vs test ALD_WORK_DIR). Source checker and later host cases were skipped. AI docs [43325869229], AliDDNS [43325869222], AdGuardHome [43325869134], and infosvr [43325869414] SUCCESS on the same exact SHA. This follow-up corrects the test fixture; exact-head reruns are pending.
- No complete local repository checkout was available; no local full-project shell/Python test run is claimed.
- L3 current-main firmware/TRX: NOT VERIFIED; an older ancestor XY-C1 build is not current-head evidence.
- L4 router flashing, boot and runtime: NOT VERIFIED.
- Residual: privileged /proc/PID/environ access, target BusyBox ash/procfs behavior, WebDAV HTTP readiness, Linux 4.4 PID-reuse concerns and current-main L3/L4.

## Exact first action for the next GPT

Fetch live main and inspect the exact-head AliyunDrive and AI-docs runs. Confirm the dynamic workdir argv fixture lets the host regression proceed through private directory/token permissions, symlink/hardlink rejection, umask/RUST_LOG, lifecycle and source/RC contracts. Inspect all path-triggered jobs by exact SHA and fix any real failure directly on main. Once the complete L2 suite passes, resume FTP cleanup TOCTOU, then NFS and FRP/Adbyby whole-tree audits. Current-main firmware/TRX L3 and hardware L4 remain NOT VERIFIED.
