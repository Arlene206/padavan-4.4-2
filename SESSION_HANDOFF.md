# Cross-session execution mandate

Recover live repository state once, then continue the active engineering task. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task: create all blobs, one complete Git tree and one commit parented to freshly fetched main; update main with exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Preserve the frozen backup and archived log shards/index. Mandatory handoff checkpoint: The checkpoint is mandatory; a document edit is conditional.
Before context exhaustion, record exact refs, Action states and the first next action.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Current product-source/test baseline: Redsocks tri-state stop/restart gate `39a2cb22ee211a42ab84214b4268b7d541c1babe` with test harness correction `31a0841d657ef8be12457c828a04c46945d02bf5` (test source passes `check nolog test`; exact-head Actions in progress). Recent exact-head validated slices: Adbyby updater gate `867e4812019a43276249b144a1d49cb1d90691c8` PASS; Shadowsocks watchcat `a8d50172afff0c081eb1b5c0f12d45dd07dc94f8` PASS; AliyunDrive `982ec548a132a5a06f4a81872f5ae920fb685da3` PASS; DDNSTO `7968c1eaeaa324066d850cef020ccebc238c8821` lifecycle/checker PASS. Live main at checkpoint before this documentation commit: `31a0841d657ef8be12457c828a04c46945d02bf5`.
- `main` is the only development branch. The frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify or rewrite it.
- Startup protocol, handoff, roadmap, log archive, branch inventory and legacy PR #1 were recovered once in this session. The only refs are `main` and the frozen backup; do not repeat the initial audit.
- Legacy PR #1 was inspected and is already CLOSED: head `ba48e22609e0c81ad757b06756fe91d05f39cbf6`, base `9d5a5cb4ab574c0e610ed7a0815c0b1ad749da74` (17 commits ahead of its old base; PR head is 765 commits behind current main). All substantive source/test work is already present on main identically or in a stronger successor: AliDDNS HMAC helper/package/tests and CA bootstrap are identical; current `aliddns.sh` has the stronger exact-argv `/proc` keeper rather than the PR's broad `kill_ps`; current `copyscripts.sh` retains CA failure propagation and stronger private FRP-script handling; main's workflow remains push-only. PR-only `pull_request` trigger and stale broad `kill_ps` implementation are intentionally not ported. The PR branch is absent from refs; no valid unmerged work remains to port and no close action is needed.
- This documentation checkpoint updates this file and `dev_log.md` only; no source changes are included.

## Verified engineering progress in this session

1. OpenSSH reload PID safety — commit `952e97ccbf9bc8d913bfa6a084e59ae3cda0d8bc`.
   - Replaced the unvalidated PID-file SIGHUP with strict numeric PID validation, procfs command-name and start-time identity checks, PID-file reread, and signal-status propagation.
   - Added `trunk/user/openssh/tests/sshd_pidfile_lifecycle.sh`; covers a valid sshd PID, PID reuse by another command, malformed/empty/privileged/overflow IDs, symlink and stale PID files, identity change, and signal failure.
   - Exact-SHA Process lifecycle run `43793042766`, process inventory `43793042723`, AI docs `43793042720`, NVRAM boundary `43793042760`, and XY-C1 build/TRX/artifact `43793042756`: PASS.

2. Transmission reload result correctness — commit `8e0556d2379f227da3e68ebf9e75668ddde09367`.
   - `func_reload` distinguishes a clean ABSENT result from pidof errors or malformed PID output, validates the PID list, and returns failed SIGHUP status rather than printing success.
   - Added `trunk/user/transmission/tests/reload_status.sh` and wired it into Process lifecycle CI.
   - Exact-SHA Process lifecycle `43794003957`, inventory `43794003936`, storage hotplug `43794003942`, and NVRAM boundary `43794003930`: PASS. Exact-SHA XY-C1 run `43794003979` was started; check its final conclusion before treating firmware validation as complete.
   - AI docs run `43794003967`: FAIL only because the then-current handoff lacked the mandatory end-of-session checkpoint wording; this checkpoint corrects it.

3. ZeroTier process-state fail-closed — commit `fc867b505ef94cb3579760a53c12923515888d34`.
   - Added RUNNING / confirmed ABSENT / UNKNOWN handling around pidof. Empty-success, malformed output and scan errors are UNKNOWN; they do not authorize signals or cleanup.
   - Unknown state after TERM blocks name-wide SIGKILL escalation; a confirmed still-running result after bounded five one-second polls is required before escalation. Stop failure preserves routes, firewall state and persistent identity.
   - Added error/malformed/empty-success and state-transition fixtures to `trunk/user/zerotier/tests/zerotier_lifecycle.sh`; Process lifecycle test portion passed.
   - Exact-SHA inventory `43794661501` and NVRAM boundary `43794661483`: PASS. Exact-SHA Process run `43794661499` failed at a pre-existing RC boot-source assertion; the subsequent main fix `69bf1373d07fa4c4810a2f122ed9fc9a272e3316` aligned the source contract and its Process lifecycle run `43794762732`: PASS. Do not repeat that audit.
   - The standalone ZeroTier workflow on `fc867b5`, run `43794661493`, initially failed because its static checker still required an inline pidof call. The production shell regression itself passed.

4. ZeroTier contract checker alignment — commit `0731e7aa932661260456a3f95f0fbebbecc42fa2`.
   - Updated `scripts/check_zerotier_lifecycle.py` to assert the tri-state helper and explicitly prohibit SIGKILL escalation when the graceful-stop result is UNKNOWN.
   - Exact-SHA ZeroTier workflow run `43794876436`: PASS, including source/RC integration contract. Exact-SHA AI docs run `43794876451`: FAIL because the previous handoff was stale and lacked the mandatory checkpoint section; the present checkpoint addresses that failure.

5. AliyunDrive WebDAV process inventory — commit `982ec548a132a5a06f4a81872f5ae920fb685da3`.
   - Added RUNNING / confirmed ABSENT / UNKNOWN pidof classification; malformed output, status/output contradictions, command errors and sleep failures are UNKNOWN. UNKNOWN blocks start, stop, KILL escalation and cleanup; a fresh validated name lookup is required before SIGKILL.
   - Extended `trunk/user/aliyundrive-webdav/tests/aliyundrive_webdav_lifecycle.sh` and source checker. Exact-head inventory, process lifecycle, handoff, AliyunDrive lifecycle and NVRAM Actions all PASS.

6. DDNSTO tri-state process lifecycle — source commit `8e1b0a3c10b8518726d2f6333e10887fffff6444`, test extraction `21c4d645b5b5cd0ccd1a1eb555d3181da57955fc`, assertion/checker correction `f78ac4a4542a60988fc295d21e3b5afdf9175171` / `7968c1eaeaa324066d850cef020ccebc238c8821`.
   - Added validated RUNNING/ABSENT/UNKNOWN pidof semantics and bounded waits; UNKNOWN after TERM and sleep errors block KILL. The regression sources the production state helper; start's expected enable-switch NVRAM lookup is distinguished from credential lookup.
   - The first exact run exposed an over-strict checker assertion that expected the state helper inline in `start_zero`; the production gate correctly calls `kill_z`. The checker was aligned to that call boundary. Exact-head DDNSTO lifecycle and handoff checks on `7968c1e` PASS.

7. Legacy scutclient/mentohust wrapper PID safety — commit `1e43d33ce171a0ea4fd5c9cc81ff67a21d017468`.
   - Removed short-lived wrapper PID-file logic that could SIGKILL a stale/reused unrelated PID, rewrite the file with the transient shell PID, and then delete it. Added a regression using a deliberately nonexistent high PID and verified stop/logout behavior remains intact.
   - Exact-head legacy wrapper regression, process lifecycle, inventory, handoff and NVRAM checks PASS. Exact-head XY-C1 firmware build was still IN PROGRESS at the last check; do not claim L3 until Build, TRX verification and artifact upload complete.

8. Redsocks stop/restart gate — source/test/checker commit `39a2cb22ee211a42ab84214b4268b7d541c1babe`; test harness correction `081c53bab84b5850dec61f2fe8cc607c6e6b9d08`.
   - Added tri-state pidof inventory, bounded stop postcondition, fresh pre-KILL revalidation and start/restart fail-closed gates. Regression covers malformed/error inventories, UNKNOWN after TERM, sleep failure, graceful stop, bounded KILL escalation, stuck daemon and replacement-launch prevention.
   - Exact `39a2cb2` Redsocks job failed because sed function extraction cut through a heredoc in `func_start`. The follow-up adds a test-only CLI-dispatch guard and sources the complete production script, preserving heredoc content. Exact-head `081c53b` inventory `43799550922`, process lifecycle `43799550880` and handoff `43799550886` PASS; NVRAM `43799550890` was IN PROGRESS; Redsocks `43799550943` failed only on the missing second script argument. Follow-up `fbc61e1` reruns Redsocks and handoff on exact SHA.

## Verification boundary

- All PASS statements above refer to Actions for the exact SHA named in the same item.
- The `952e97c` XY-C1 build/TRX/artifact result is complete and PASS.
- The `8e0556d` XY-C1 run `43794003979` completed SUCCESS (Build, TRX verification and artifact upload).
- The `fc867b5` XY-C1 run `43794661521` later completed SUCCESS; `8e0556d` XY-C1 run `43794003979` also completed SUCCESS (Build/TRX/artifact). The standalone ZeroTier workflow on `0731e7a`, run `43794876436`, is PASS.
- Exact push Actions for Adbyby updater regression correction `867e4812019a43276249b144a1d49cb1d90691c8`: inventory, process lifecycle, handoff validation, Adbyby host/config, and NVRAM boundary all PASS. No XY-C1 firmware build was triggered or claimed for this shell/test-only correction.
- No physical-router runtime or L4 validation was performed in this session.
- No change was made to the frozen backup branch.

## Active task

Redsocks lifecycle regression correction is the current active slice. Production source is on main at `39a2cb22ee211a42ab84214b4268b7d541c1babe`; regression harness corrections are `081c53bab84b5850dec61f2fe8cc607c6e6b9d08` (test-only CLI-dispatch guard, source complete script) and `fbc61e15eb5365bbcc7b000e99e053ebf1adf734` (pass both required script arguments). The exact `081c53b` Redsocks job failed because `$2` was unbound; `fbc61e1` then failed because `$3` was unbound. Follow-up `31a0841` supplies `check nolog test`. Exact-head Redsocks run `43799793322` and handoff run `43799793399` were IN PROGRESS at checkpoint; do not claim L2 closure until both pass.

Completed source/host slices with exact-head PASS: Adbyby updater gate `867e481`; Shadowsocks watchcat `a8d5017`; AliyunDrive process inventory `982ec54`; DDNSTO lifecycle/checker `7968c1e`; legacy scutclient/mentohust stale-pidfile regression `1e43d33` (XY-C1 build remains pending). The AdGuardHome checker correction `bcc06e5` is already on main. Frozen backup remains unchanged.

Legacy PR #1 is CLOSED and its branch ref is absent. The 17-commit PR diff was compared against current main: HMAC helper/package/tests and CA bootstrap are already present identically; main has stronger AliDDNS keeper identity checks and FRP script hardening. The PR's stale broad `kill_ps` and pull_request trigger are not valid work to port. No unmerged product change remains.

## Exact first action for the next GPT

1. Inspect exact-head push Actions for `31a0841d657ef8be12457c828a04c46945d02bf5`, especially Redsocks run `43799793322` and handoff run `43799793399`; triage any failure from logs and repair on latest main. The prior `081c53b` inventory/process/handoff checks passed; NVRAM was still in progress.
2. If all pass, mark Redsocks L2 host-verified and continue from the latest whole-tree process inventory; do not repeat Adbyby, Shadowsocks, AliyunDrive, DDNSTO or legacy PID-file work.
3. Recheck XY-C1 run `43799216333` on source SHA `1e43d33ce171a0ea4fd5c9cc81ff67a21d017468`; claim L3 only if Build, TRX verification and artifact upload all complete on that exact SHA.
4. Keep all target BusyBox/MIPS/uClibc and physical-router L4 claims separate; none is established by host tests alone.

## Governance and safety constraints

- Preserve security, architecture, compatibility and required test rules.
- Use only `main` for development; no feature branches or PRs.
- Keep `backup/pre-plugin-overhaul-2026-11-14` frozen. Never force-push.
- Do not claim a test, commit, firmware build, artifact or hardware check that was not actually performed.
- Before the next session ends, perform another mandatory handoff accuracy checkpoint. If all checkpoint facts remain accurate, explicitly conclude the check without a no-op documentation edit or commit.
