# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the live repository once, then resume the exact active task. Avoid repeated audits, planning-only stops and status-only commits.

- One logical task per atomic commit. For multi-file tasks create all blobs, one complete tree based on live main, one commit with live main as parent, then update `main` with `force=false` and exact `expected_sha`.
- Never create/use development branches or PRs, force-push, amend published commits, or alter the frozen backup.
- Check relevant Actions by exact `head_sha`; inspect failure logs, and do not wait idly for independent PENDING work.
- Preserve GNU89, MIPS/uClibc, BusyBox/Linux 4.4 compatibility; retain fail-closed handling, lifecycle PID identity, bounded operations and meaningful regression coverage.
- Frozen branch `backup/pre-plugin-overhaul-2026-11-14` is archival and must remain untouched.

## Live checkpoint and baselines

- Parent main before this test follow-up: `1e09d066d315b36fbd0010eea96bdf68e96e1333`; always fetch the live head on takeover.
- Product-source baseline: `740c82cee671e5bcc133289d9126f75afd654ec3` (secure password staging, boot-status propagation and rollback-capable passwd/group/shadow publication).
- Descendants `775ef93`, `590bc8a` and `1e09d06` are regression, workflow-contract and project-record changes; they do not alter production C behavior.
- Frozen backup tip: `72f4f157a084f8184b9597ca5bf334b8080e5807`. Branch inventory at last recovery showed `main` plus this protected backup only; no open PR was found.

## Account database reliability — current state

Production code paths:
- `trunk/user/shared/src/shutils.c`: password staging and account database generation; boot caller in `trunk/user/rc/src/init.c`.
- `trunk/user/shared/src/passwd_temp_file_policy.h`: exclusive private staging, hard-link recovery points and rollback-capable multi-file publication.
- Root remains locked until `chpasswd` succeeds; account provisioning errors propagate into PID 1 startup status.

Regression coverage in `trunk/user/shared/src/tests/passwd_temp_file_policy.c`:
- Successful multi-file publication and backup cleanup.
- Injected second-rename failure with first-file restoration.
- Failed rollback preserving a recovery hard link.
- Injected third (shadow) rename failure with passwd/group restoration in reverse order.
- Missing-original rollback: the fixture now clears the transaction's `had_original` flag and requires the injected second rename to fail with `EIO`, exactly two rename attempts, removal of the newly published passwd file, consumed passwd stage, preserved group stage, and unchanged original group data.

THIS COMMIT changes only the test and handoff/history records; no production C behavior changed.

## Exact-SHA validation

On parent `1e09d06`:
- Process lifecycle fail-closed regression `44655310242`: PASS, including “Verify Unix account bootstrap status and private password staging”.
- Process inventory `44655310167`: PASS.
- AI handoff documentation contract `44655310303`: PASS.
- NVRAM transaction boundary audit `44655310399`: PASS.
- XY-C1 `44655310544`: workflow PASS for host/build-input checks, but Build XY-C1, TRX verification and artifact upload were SKIPPED because no production firmware input changed. Do not count this as L3.
- Earlier HTTPD security `44654997218` and Process lifecycle `44654997255` on `590bc8a`: PASS.
- L3: XY-C1 run `44654005725` on unchanged production-source SHA `740c82c` completed Build XY-C1, TRX verification and artifact upload successfully.
- L4 physical-router runtime/boot validation remains NOT VERIFIED.
- Exact-head checks for THIS COMMIT are pending.

## Exact first actions for the next GPT

1. Fetch live main and inspect the exact-head Process lifecycle Actions for the strengthened missing-original rollback assertions. Verify the host binary completed successfully; repair any regression without loosening the `EIO`, rename-count, destination and stage postconditions.
2. Confirm no new production-source changes have invalidated the L3 baseline. Do not treat the parent `1e09d06` host-only XY-C1 workflow as a firmware build.
3. Resume P1 whole-userland reliability work: inspect remaining launch/stop result propagation in `trunk/user/rc/src/services_ex.c` and its direct callers. Trace a concrete ignored nonzero return to the resulting state/boot status, add focused failure-injection or source-contract coverage, and commit one bounded slice. Avoid repeating the completed `init_bridge()`, USB modem reconnect and account rollback changes.

## End-of-session checkpoint

Before stopping, compare live main, production-source baseline, exact-head tests and active task. Update this handoff only when its resumable facts materially change; do not chase a documentation-only SHA. Keep `dev_log.md` newest-first with 20–119 complete entries; archive 100 oldest entries together only at the 120-record or early size trigger.

## Evidence discipline

Keep L1 source review, L2 host/Actions, L3 MIPS build/TRX and L4 physical-device evidence separate. Never present a skipped build as PASS or a successful XY-C1 build as proof of router runtime behavior. Confirm commits are remotely reachable before claiming preservation across conversations.
