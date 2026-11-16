# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers, frozen backup and precise first action for the next session. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, explicitly conclude the check and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, do not wait for pending CI when independent work can proceed, and do not ask the user what to do next.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main, followed by a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, use a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Main parent for this checker correction: `8956d722b51b7a5b8fd3fab09750bf96dc0753be`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`, unchanged/read-only.
- Exact-head 8956 Firewall Startup `43945597477`, Process regression `43945597663`, and XY-C1 `43945597624` failed at the canonical hook-status count because the raw Python regex was double-escaped and matched literal backslashes instead of source whitespace. The C source contains nine valid first-error merges.
- Exact-head 8956 HTTPD `43945597526` failed because its inline checker still searched for a `static void` hook-stage logger; the implementation correctly changed this helper to return `int` so stage failures reach RC status.
- Exact-head 8956 AliyunDrive `43945597544`, AliDDNS `43945597596`, AdGuardHome `43945597557`, AI handoff `43945597849`, USB modem `43945597613`, and infosvr `43945597566` passed. NVRAM `43945597771` was still in progress at last poll.
- The repair corrects the regex to a single escaped whitespace token and updates the HTTPD helper-signature contract. No result transfers from SHA 8956 to the new commit.

## Active task

The Shadowsocks, persistent post-firewall and Optware hook paths now propagate non-ENOENT staging failures, command failures and staged-file cleanup errors to `start_firewall_ex()`, while continuing independent hooks. Missing optional scripts remain no-op; a successful built-in firewall stays enabled even when an extension fails. Staged-path removal retains its path when unlink fails.

The implementation is present, but the integration suites exposed two stale checker assumptions: a doubly escaped whitespace regex and an outdated `static void` signature. This slice fixes those validation contracts only.

Next P1: audit NAT helper and HW NAT/SFE module load/unload postconditions in `net.c`. Verify required ALG failures remain visible and optional accelerator failures do not block ordinary WAN/firewall service. Add injectable unload postcondition tests.

## Verification boundary

- Exact-head 8956 Firewall Startup `43945597477`, Process regression `43945597663`, and XY-C1 `43945597624` failed before any new firmware build because the canonical Python regex used double backslashes and could not match the nine source status merges.
- Exact-head 8956 HTTPD `43945597526` failed after the hook-staging checks, at an inline source-boundary assertion that expected `static void firewall_log_hook_stage_failure`; the actual helper now returns `int`.
- Exact-head 8956 AliyunDrive, AliDDNS, AdGuardHome, AI handoff, USB modem identity and infosvr passed. NVRAM was in progress at last poll.
- This commit repairs test contracts; all exact-head CI and XY-C1 Build/TRX/artifact checks must run again on the new SHA. No local firmware or physical-router claim; router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect the new commit's exact-SHA Firewall Startup, Process regression, HTTPD, XY-C1 Build/TRX/artifact and remaining NVRAM/AI/service Actions. Diagnose any first new failure.
2. Continue with NAT helper, HW NAT and SFE unload/load postconditions; add deterministic failed-unload and stale-state regression cases.
3. Keep commits main-only and atomic with non-forced expected-SHA CAS. Frozen backup read-only; physical-router L4 NOT VERIFIED.
