# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers, frozen backup and precise first action for the next session. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, explicitly conclude the check and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, do not wait for pending CI when independent work can proceed, and do not ask the user what to do next.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main, followed by a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, use a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Main parent for this validation update: `391902a4f936c8967d6e83999892853683333d1f`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`, unchanged/read-only.
- Exact-head 39190 Firewall Startup `43947724068` and Process regression `43947724044` failed because the new NAT module assertions incorrectly read `POLICY_SOURCE` / `POLICY_TEST`, which are bound to the modem-WAN policy files; the wireless module policy/test inputs now have separate names.
- Exact-head 39190 HTTPD `43947724129` failed on old source contracts that expected `hwnat_configure(int wan_vid)` and a one-argument call. The implementation now accepts the current allowed-state flag so a stuck disabled module cannot be logged as enabled; assertions are updated.
- Exact-head 39190 USB modem `43947724775`, AI handoff `43947724226`, process inventory `43947724196`, and WAN route `43947724172` passed. NVRAM `43947724501`, USB storage `43947724065`, and XY-C1 `43947724308` were pending at last poll.
- The prior checker-only commit 09eaad9 exact-head Firewall Startup `43947164711`, Process regression `43947164786`, NVRAM `43947165067`, AI handoff `43947164930`, USB modem `43947164727`, AliyunDrive `43947164893`, AliDDNS `43947164823`, AdGuardHome `43947165007`, and infosvr `43947165154` passed; XY-C1 `43947164750` was still running. The new NAT source SHA must be validated independently.

## Active task

The Shadowsocks, persistent post-firewall and Optware hook paths now propagate non-ENOENT staging failures, command failures and staged-file cleanup errors to `start_firewall_ex()`, while continuing independent hooks. Missing optional scripts remain no-op; a successful built-in firewall stays enabled even when an extension fails. Staged-path removal retains its path when unlink fails.

The implementation is present, but the integration suites exposed two stale checker assumptions: a doubly escaped whitespace regex and an outdated `static void` signature. This slice fixes those validation contracts only.

Next P1: audit NAT helper and HW NAT/SFE module load/unload postconditions in `net.c`. Verify required ALG failures remain visible and optional accelerator failures do not block ordinary WAN/firewall service. Add injectable unload postcondition tests.

## Verification boundary

- 39190 Firewall Startup `43947724068` and Process regression `43947724044` both reached the NAT checks but failed because the canonical script reused the modem-WAN `POLICY_SOURCE/POLICY_TEST` bindings for the newly added wireless-module policy assertions.
- 39190 HTTPD `43947724129` reached switch status regression successfully, then failed at its old `hwnat_configure` function signature assertion. Updated to match the allow-aware two-argument helper and call.
- 39190 USB modem identity, AI handoff, process inventory, and WAN route suites passed; NVRAM, USB storage, and XY-C1 were pending at last poll.
- 09ea checker-only source SHA had Firewall Startup, Process regression and all reported non-XY suites pass; its XY-C1 remained pending. No exact-SHA firmware Build/TRX/artifact claim for 39190 yet. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Check new-SHA Firewall Startup, Process regression, HTTPD and XY-C1 Build/TRX/artifact. Fix any first source/test failure; do not transfer results from 39190.
2. If the module policy tests and cross-workflow assertions pass, inspect the completed NAT unload postcondition behavior and continue the whole-userland stability rotation.
3. Keep main-only complete-tree commits with expected-SHA CAS. Frozen backup remains read-only; physical-router L4 NOT VERIFIED.
