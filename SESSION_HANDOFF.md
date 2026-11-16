# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers, frozen backup and precise first action for the next session. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, explicitly conclude the check and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, do not wait for pending CI when independent work can proceed, and do not ask the user what to do next.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main, followed by a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, use a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Main parent for this validation correction: `8c77a050018df9e3db1dfc3afd73b7f646c47e5a`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`, unchanged/read-only.
- Exact-head 8c77 Firewall Startup `43946213665` and Process regression `43946213545` reached the canonical optional-hook contract but failed because `compact()` collapses whitespace to a single space while the checker expected the impossible literal `errno!=ENOENT`. The source correctly uses `errno != ENOENT`; the assertion now accepts C whitespace.
- Exact-head 8c77 XY-C1 `43946213553` failed at the same source checker before Build/TRX/artifact. HTTPD `43946213772`, NVRAM `43946213544`, AI handoff `43946213616`, AliyunDrive `43946213652`, AliDDNS `43946213539`, AdGuardHome `43946213561`, USB modem `43946213534`, and infosvr `43946213737` passed.
- The 8c77 checker regex and HTTPD signature fixes were effective; HTTPD is now green. This commit only corrects the remaining whitespace-sensitive assertion. New-SHA validation is required.

## Active task

The Shadowsocks, persistent post-firewall and Optware hook paths now propagate non-ENOENT staging failures, command failures and staged-file cleanup errors to `start_firewall_ex()`, while continuing independent hooks. Missing optional scripts remain no-op; a successful built-in firewall stays enabled even when an extension fails. Staged-path removal retains its path when unlink fails.

The implementation is present, but the integration suites exposed two stale checker assumptions: a doubly escaped whitespace regex and an outdated `static void` signature. This slice fixes those validation contracts only.

Next P1: audit NAT helper and HW NAT/SFE module load/unload postconditions in `net.c`. Verify required ALG failures remain visible and optional accelerator failures do not block ordinary WAN/firewall service. Add injectable unload postcondition tests.

## Verification boundary

- Exact-head 8c77 Firewall Startup `43946213665` and Process regression `43946213545` failed at `assert "errno!=ENOENT" in hook_dispatch`. The checker compacts whitespace to one space, whereas the actual C source uses `errno != ENOENT`; the new regex accepts spacing.
- Exact-head 8c77 XY-C1 `43946213553` was blocked by the same source checker before Build/TRX/artifact. HTTPD, NVRAM, AI handoff, AliyunDrive, AliDDNS, AdGuardHome, USB modem identity and infosvr passed on 8c77.
- This is a checker-only correction. No new-SHA Actions or firmware verification claimed yet; physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect new-SHA Firewall Startup, Process regression and XY-C1 Build/TRX/artifact; also confirm the 8c77 exact-head suites that were already green remain understood.
2. Continue NAT helper, HW NAT and SFE load/unload postcondition work in `net.c`, with deterministic injectable unload tests.
3. Keep main-only atomic commits with non-forced expected-SHA CAS. Frozen backup read-only; physical-router L4 NOT VERIFIED.
