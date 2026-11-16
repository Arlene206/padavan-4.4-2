# Cross-session execution mandate

Recover live repository state once, then continue the active engineering task. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task: create all blobs, one complete Git tree and one commit parented to freshly fetched main; update main with exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Preserve the frozen backup and archived log shards/index. Mandatory handoff checkpoint: The checkpoint is mandatory; a document edit is conditional.
Before context exhaustion, record exact refs, Action states and the first next action.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Earlier source baseline: `0731e7aa932661260456a3f95f0fbebbecc42fa2`; documentation checkpoint: `b3e550a5dadf0607f2d7aba5b93db6da8e1c9d4d`. Main immediately before the current updater slice: `ca6f0786aa35f95331d031e38bca53dea189743a` (`fix(adbyby): capture pidof errors safely under errexit`).
- `main` is the only development branch. The frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify or rewrite it.
- Startup protocol, handoff, roadmap and log archive were recovered once in this session. Do not repeat the initial audit.
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

## Verification boundary

- All PASS statements above refer to Actions for the exact SHA named in the same item.
- The `952e97c` XY-C1 build/TRX/artifact result is complete and PASS.
- The `8e0556d` XY-C1 run `43794003979` completed SUCCESS (Build, TRX verification and artifact upload).
- The `fc867b5` XY-C1 run `43794661521` later completed SUCCESS; `8e0556d` XY-C1 run `43794003979` also completed SUCCESS (Build/TRX/artifact). The standalone ZeroTier workflow on `0731e7a`, run `43794876436`, is PASS.
- Exact push Actions for `ca6f0786aa35f95331d031e38bca53dea189743a`: Adbyby regression `43797496515` PASS; Process lifecycle `43797496441` PASS; process inventory `43797496503` PASS; AI handoff docs `43797496481` PASS; HTTPD security `43797496369` PASS. NVRAM boundary `43797496462` and XY-C1 `43797496451` were PENDING at the latest check.
- No physical-router runtime or L4 validation was performed in this session.
- No change was made to the frozen backup branch.

## Active task

The Adbyby stop contract now uses a tri-state pidof inventory, verifies a bounded post-signal ABSENT state, and refuses teardown on UNKNOWN or timeout. Commit `ca6f0786aa35f95331d031e38bca53dea189743a` repaired the pidof-status capture so expected nonzero statuses are safely handled under callers using `set -e`. Exact-SHA Adbyby regression run `43797496515`, Process lifecycle `43797496441`, process inventory `43797496503`, AI docs `43797496481`, and HTTPD security `43797496369` all PASS.

A new concrete caller gap remains: `adbyby_uprules()` still calls `adbyby_close` without checking its status. An UNKNOWN/timed-out stop can therefore be followed by rule staging and a replacement daemon launch. The current follow-up adds a fail-closed stop gate before `addscripts`, extends the shell regression to prove updater staging/launch does not happen after a failed stop, and extends `check_adbyby_hosts_config.py` to enforce the production/test wiring. Base SHA for this atomic slice: `ca6f0786aa35f95331d031e38bca53dea189743a`.

## Exact first action for the next GPT

1. Check exact-SHA push Actions for this updater-gate commit. Verify Adbyby configuration stability, Process lifecycle, process inventory, AI docs, HTTPD security, NVRAM boundary, and XY-C1; mark only completed/success runs PASS.
2. Confirm the regression covers initial pidof UNKNOWN/malformed, stubborn timeout, post-signal UNKNOWN, sleep failure, confirmed ABSENT, successful stop, and the updater caller refusing staging/relaunch after stop failure.
3. Continue repository-wide lifecycle governance from the next highest-risk remaining direct signal/stop caller in the inventory; do not repeat the completed Adbyby process-state capture or current updater gate.

## Governance and safety constraints

- Preserve security, architecture, compatibility and required test rules.
- Use only `main` for development; no feature branches or PRs.
- Keep `backup/pre-plugin-overhaul-2026-11-14` frozen. Never force-push.
- Do not claim a test, commit, firmware build, artifact or hardware check that was not actually performed.
- Before the next session ends, perform another mandatory handoff accuracy checkpoint. If all checkpoint facts remain accurate, explicitly conclude the check without a no-op documentation edit or commit.
