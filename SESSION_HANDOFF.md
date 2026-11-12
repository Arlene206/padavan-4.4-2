## Current repository state

- Repository: Arlene206/padavan-4.4-2. Development and commits remain restricted to main; refetch the live head before every write. Latest exact source/test checkpoint is dbf15f4bd8472da954a9f5ce68e4b4b57f09f2bb; the current AliyunDrive token-workdir hardening is based directly on that SHA. Only main is present; legacy PR #1 remains CLOSED/unmerged and GPT Session Rotator remains untouched.
- Remote branch inventory at this checkpoint contains only `main`. Legacy PR #1 remains CLOSED and unmerged; its useful AliDDNS source/helper/package/host tests were already integrated on main. Do not cherry-pick or reopen it. GPT Session Rotator remains immutable.
- Recent AliDDNS commit chain: `1931488bc4767b7abcd4710bee3c6fcce00eb65d` implemented exact keeper PID matching and bounded TERM/KILL; `a8e30e38cf2c35387e5c7e409b314eb03f8878dd` is a tree-identical empty commit (no source change); `f19e8e43369d3e3ae9dba507573626960a7e0478` corrected the self-PID test fixture; `c30e9b29abc093963a4276ca22eeb2c7e18f1001` fixed the command-substitution argv-boundary handling and an obsolete HTTPD workflow assertion; `8eec765e9a91c617eb2eb6fab582321a8e04966f` tightened the keeper action to the final argv token and added an extra-argument decoy; `70db3735b4c129622914cee610ac02e8b99fb017` corrected the semantic checker to assert that exact contract.
- The previous AliyunDrive WebDAV lifecycle task remains L2-closed on exact SHA `18234ff0a41eb5343aaf58207b3cf641bbf4a244`: lifecycle run [43287688178] / job [131425308994] passed bounded stop, duplicate-start/argv integrity, serialized transitions and asynchronous process-presence readiness. This does not prove WebDAV HTTP readiness.
- Exact latest source/test checkpoint Actions on dbf15f4bd8472da954a9f5ce68e4b4b57f09f2bb:
  - AliyunDrive WebDAV lifecycle [43321989293] / job [131544724607] — SUCCESS. Shell syntax, QEMU execution of the exact packaged MIPSel binary, SHA-bound env capability report, bounded lifecycle/environment/argv regression, and full source/RC contract all passed.
  - AI handoff docs [43321989151] / job [131544725051] — SUCCESS; root log=81/119; archive inventory dev-log=7, handoff=22, roadmap=18.
  - AliDDNS signed API security [43321989235] / job [131544724203] — SUCCESS.
  - AdGuardHome config security [43321989308] / job [131544724394] — SUCCESS.
  - infosvr packet/signal stability [43321989090] / job [131544723517] — SUCCESS.
  - All five path-triggered push workflows completed SUCCESS on this exact SHA. This closes L2 for moving AliyunDrive credentials out of argv, but is not a firmware build or L4 result.
  - QEMU reported bundled daemon 1.2.1 and [env: ...] bindings for all three credential values. Binary SHA-256 remains 237c0ab2c14197c5a4e4bd8975f884979a29b35cad31b8e2354e084c19413891.

- Earlier checkpoint workflow history (not current-head evidence):
  - AliyunDrive WebDAV lifecycle/source contract: run [43312330424] / job [131511324570] — SUCCESS on `0460f705b6ea3bbe1bcc9cfa61d66b677e5612da`. Shell syntax, bounded stop, duplicate-start/argv integrity, lifecycle lock, asynchronous process-presence startup and RC/source contracts passed. The exact-binary checker warned that the bundled daemon lacks its required credential environment-variable interface; this is not a credential-redaction pass.
  - AI handoff docs: run [43312330419] / job [131511324768] — FAILURE on the same SHA because `SESSION_HANDOFF.md` lacked the required `## Active task

The previous task, moving the AliyunDrive refresh token and WebDAV credentials out of daemon argv, is now L2-verified on exact main SHA dbf15f4bd8472da954a9f5ce68e4b4b57f09f2bb. QEMU ran the bundled v1.2.1 MIPSel binary; its help exposes the exact REFRESH_TOKEN, WEBDAV_AUTH_USER, and WEBDAV_AUTH_PASSWORD env bindings. The launcher passes secrets only through the child environment, and the lifecycle regression independently verifies exact environment values and no credential argv leakage.

During follow-up review of the exact upstream v1.2.1 source (tag commit 9084cdf87810f22f70bb02d150380db66ca3ec36), a second credential-at-rest/logging risk was identified:
- src/drive/mod.rs saves the rotated refresh token with create_dir_all() and tokio::fs::write(), without setting directory/file permissions. New file permissions therefore depend on the daemon's umask; existing file mode is preserved.
- src/main.rs logs the newly rotated refresh token in an info! event. This router launcher discards stdout/stderr, but the daemon should still avoid emitting the token in normal logs.

This commit hardens the persisted token path before every launch: the workdir must be a non-symlink directory owned by the invoking UID and is forced to mode 0700; an existing refresh_token must be a same-owner regular non-symlink file with link count 1 and is forced to mode 0600. The child runs with umask 077 so newly created token files are mode 0600, and RUST_LOG=aliyundrive_webdav=warn suppresses the upstream token-bearing INFO event while retaining warnings/errors. The background child closes inherited FD 9 before executing the daemon so hardening does not accidentally keep the lifecycle lock held. The fixed workdir path and token persistence behavior remain unchanged.

Host regression coverage added here checks initial directory mode, repair of pre-existing 0755/0644 paths, rejection of workdir/token symlinks without modifying their targets, rejection of multiply-linked token files, the child umask/file mode, and exact RUST_LOG/env values. Source contracts also require BusyBox STAT format/ID/CHMOD/MKDIR support and the new secure-launch ordering. Exact-head Actions for this commit are PENDING; no result is claimed until all path-triggered jobs pass on the resulting SHA.

Residual security boundary: environment values remain readable to sufficiently privileged root via /proc/PID/environ; environment transport protects the ordinary argv/process-list exposure, not a privileged attacker. Process presence still does not prove WebDAV HTTP readiness.

Next after exact-head validation: inspect the AliyunDrive and AI-docs workflows on the resulting SHA. If green, move to the FTP failed-upload cleanup TOCTOU design review (still OPEN/BLOCKED pending a race-safe design for STOR/APPE/REST/STOU and concurrent external writers), then rotate through NFS real-client evidence and the broader FRP/Adbyby audit. Current-main L3 firmware/TRX and L4 hardware validation remain NOT VERIFIED.

## Completed task: AliDDNS lifecycle process identity and bounded stop

Traced the HTTPD `variables.c` → PID 1 `rc.c` → `services.c` → `/usr/bin/aliddns.sh` control route. The previous `kill_ps()` selected processes with broad `ps | grep | awk | eval` matching and sent TERM/KILL without confirming exit.

Current implementation:
1. Removes generic `kill_ps()`; enumerate `/proc/[0-9]*/cmdline` and match only the exact launcher path followed by the final `keep` argument. Convert NUL separators to newlines and restore the trailing boundary that command substitution strips. Reject nonnumeric/reserved PIDs and the current shell.
2. Revalidate the exact argv immediately before signalling. Send TERM, allow at most five one-second intervals, send KILL only to still-matching keeper PIDs, then allow at most two more seconds and return failure if the keeper remains.
3. Make stop failure a hard gate: a failed close prevents status mutation or replacement keeper startup. Restart/start/stop/check still run through the stable-inode lifecycle lock and the RC restart remains one atomic locked action.
4. The host regression covers same-path/final-`keep` matching, current-shell exclusion, similar-path and wrong-action decoys, extra argv rejection, graceful TERM, bounded KILL escalation and stubborn-process failure preventing restart. The HTTPD umbrella contract now validates the exact keeper-stop helpers rather than requiring the removed grep-based `kill_ps`.

Intermediate CI failures were investigated, not hidden: the first regression failed because the test fixture used a literal dollar path instead of the test shell PID; the next exposed command substitution removing the trailing newline; the next semantic check itself was corrected to assert the final-argument pattern and literal `$$` current-shell exclusion. Current code/test/checker exact-head run [43306180515] is green. The HTTPD workflow's obsolete `kill_ps "$ALIDDNS_HELPER"` assertion was replaced with checks for `aliddns_stop_keeper`, `aliddns_keeper_pid_matches` and absence of `kill_ps`; HTTPD run [43305744811] is green on the same production source tree.

## Verification boundary

- L1: bootstrap/protocol and prescribed recovery sequence completed; current main, exact handoff/roadmap/dev-log, archive index, branch inventory and exact-head Actions reviewed. QEMU executed the packaged v1.2.1 MIPSel binary and verified all three credential env bindings. ELF is static ELF32 LE MIPS32R2/O32 (e_flags=0x70001007, no PT_INTERP); all 56 board templates explicitly disable this optional add-on. Upstream v1.2.1 source confirms refresh-token persistence uses create_dir_all + write and emits the rotated token at INFO; private workdir, umask and log filtering are being implemented in this commit.
- L2 credential argv redaction: exact SHA dbf15f4bd8472da954a9f5ce68e4b4b57f09f2bb; AliyunDrive [43321989293] / job [131544724607] and AI docs [43321989151] / job [131544725051] SUCCESS. AliDDNS [43321989235], AdGuardHome [43321989308] and infosvr [43321989090] also SUCCESS on that same SHA. The earlier bf70e759b61e35022aa25bfe89a1c7cb3c0e24c7 attempt had QEMU and host-test PASS but exposed a stale checker marker; dbf15f4 corrected it and all five exact-head workflows passed.
- L2 token-at-rest/log hardening in THIS COMMIT: exact-head Actions PENDING. No host regression, source-contract or firmware success is claimed until the new SHA completes.
- No complete local repository checkout was available; no local full-project shell/Python test run is claimed. Evidence above is from exact-SHA GitHub Actions.
- L3 full firmware/TRX for current main: NOT VERIFIED. An automatic XY-C1 build succeeded only on ancestor c30e9b29abc093963a4276ca22eeb2c7e18f1001; it is not current-main evidence.
- L4 router flashing, boot and runtime: NOT VERIFIED.
- Residual: privileged /proc/PID/environ access; Linux 4.4 keeper PID-reuse window; target BusyBox ash/procfs timing; real WebDAV HTTP readiness; current-main L3/L4 remain open.

## Exact first action for the next GPT

Fetch live main and inspect every path-triggered workflow on the exact current SHA. For AliyunDrive, require shell syntax, QEMU SHA-bound env capability, secure workdir permission/symlink/hardlink/umask cases, credential environment/argv privacy, lifecycle stop/start/lock tests, and the full source/RC checker to pass together. Fix any actual failure directly on main; do not write a docs-only commit merely to record green results. If all L2 checks pass, proceed to design a race-safe FTP failed-upload cleanup strategy without breaking STOR/APPE/REST/STOU or concurrent external writers; then resume NFS and FRP/Adbyby whole-tree audits. Keep current-main XY-C1 firmware/TRX L3 and L4 explicitly NOT VERIFIED until the development body is ready for the planned full build and hardware validation.
