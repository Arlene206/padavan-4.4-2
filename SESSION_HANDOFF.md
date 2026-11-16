# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers, frozen backup and precise first action for the next session. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, explicitly conclude the check and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, do not wait for pending CI when independent work can proceed, and do not ask the user what to do next.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main, followed by a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, use a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Main: `2edafb7eede75ce643bcccf7170aa4ed1111bb93`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, unchanged/read-only.
- Helper commit `c0ffc7d3a1dc90a8c208662cf03f57e7f2b3cf15` exact-head Firewall Startup `43944200570`, HTTPD `43944200665`, NVRAM `43944200581`, AI handoff `43944200578`, Process inventory `43944200566`, and Process regression `43944200736` passed.
- Exact-head 2ed Actions: Firewall Startup `43944601860`, XY-C1 `43944601903`, HTTPD `43944601745`, NVRAM `43944601815`, AI handoff `43944601727`, Process inventory `43944601706`, Process regression `43944601739`, FULLCONENAT `43944601842`, and USB modem identity `43944601920` were all IN_PROGRESS at the last poll.
- 5e486e exact-head Firewall Startup, HTTPD, NVRAM, AI and XY-C1 validation all completed successfully. Current 2ed source changes still require their own exact-SHA verification.
- Historical PR #1 remains closed/unmerged. Frozen backup remains read-only.

## Active task

Optional firewall hook failures are now surfaced without disabling a successfully applied built-in firewall. Absent optional scripts (ENOENT) are a no-op; invalid/unreadable staging, nonzero command status and staged-file cleanup errors are merged using first-error semantics, while the other independent Shadowsocks, persistent and Optware hooks continue. The staged-file remove helper preserves the path when unlink fails and treats already-removed paths as success.

Next P1: audit NAT helper and HW NAT/SFE module load/unload postconditions in `net.c`. Verify that required PPTP/H.323/SIP/FTP helpers do not falsely report success after failed load/unload, that FTP conntrack/NAT dependency ordering is safe, and that optional hardware NAT/SFE failures do not block ordinary WAN/firewall startup. Add focused fault-injection coverage where the state transitions are testable.

## Verification boundary

- Exact-head helper commit `c0ffc7d3a1dc90a8c208662cf03f57e7f2b3cf15`: Firewall Startup, HTTPD, NVRAM, AI handoff, Process inventory and Process regression all PASS.
- Exact-head current commit `2edafb7eede75ce643bcccf7170aa4ed1111bb93`: Firewall Startup, XY-C1 Firmware Validation, HTTPD, NVRAM, AI handoff, Process inventory/regression, FULLCONENAT and USB modem identity were IN_PROGRESS at the last poll.
- No same-SHA XY-C1 build/TRX/artifact success is claimed for 2ed yet. No local full firmware build or physical-device test is claimed. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-SHA 2ed Firewall Startup and HTTPD hook checks, XY-C1 Build/TRX/artifact, and NVRAM/AI/Process jobs; fix the first genuine failing step.
2. Continue the NAT helper and optional HW NAT/SFE load/unload postcondition audit in `net.c`, with deterministic tests for failure paths where feasible.
3. Expand stability coverage into another userland subsystem after this security/reliability slice. Main-only complete-tree atomic commits with non-forced expected-SHA CAS; frozen backup read-only; physical-router L4 NOT VERIFIED.
