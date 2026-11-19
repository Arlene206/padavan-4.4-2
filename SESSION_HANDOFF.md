# 2026-11-19 continuation checkpoint — switch CLI invalid-argument hardening

## Current repository state

- Repository `Arlene206/padavan-4.4-2`, development on `main`.
- Previous WAN checker fix commit: `cc3f3d9d3c9261e394f87a20c93dd9874f870c79`; fetch live refs before every write.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; never modify. PR #1 is closed/unmerged.
- WAN first-error implementation merges USB NDIS/RAS and callback/VLAN/APCLI/WISP preflight failures into one chronological accumulator. The latest checker count fix changes expected VLAN-result merges from 19 to 21; its exact-head USB/Firewall/Process/AI docs checks are pending.

## Active task

A P1 switch CLI defect was found: `switch acl dip`, `acl etype`, `acl port` and `acl L4` with argc 3 passed the old `argc < 3` guard, then dereferenced missing `argv[3]`. Also, `usage()` printed help and exited 0 for invalid commands.
This slice requires `argc >= 4` before any ACL `argv[3]` access and changes invalid usage to `EXIT_FAILURE`. The existing Switch CLI workflow will statically assert guard ordering and nonzero usage status; existing push/PR filters already cover both touched paths, with no new workflow or trigger expansion.

## Verification boundary

- L1: production guard and invalid-command exit status reviewed.
- L2: exact-head Switch CLI action pending; workflow includes existing GNU89/Werror and ASan/UBSan host tests for parsers and bounded waits, plus new source contracts for ACL arity and usage exit.
- L2 WAN: exact-head USB modem, Firewall, Process lifecycle and AI docs checks for the count-fix SHA must be checked separately.
- L3: XY-C1 Build, TRX and artifact upload pending on exact source SHA.
- L4: actual switch PHY/MDIO and router behavior NOT VERIFIED. No local tests or MIPS build run in this session.

## Exact first action for the next GPT

1. Fetch live main/backup; inspect exact-head Actions for the latest commit.
2. Inspect and fix demonstrated test/checker failures; verify Switch CLI, USB modem, Firewall, Process lifecycle/inventory, WAN route and AI docs.
3. Verify XY-C1 Build, TRX and artifact upload on one identical SHA before recording L3 PASS.
4. Then continue an independent P1 stability/security defect elsewhere in Padavan userland; do not repeat the WAN first-error audit.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state and exact-head Actions before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit. Do not create a no-op commit.

## Cross-session execution mandate

One logical slice, one atomic main commit. Create all blobs and one complete tree, then one commit with exact live main as parent; update main using non-forced expected-SHA compare-and-swap. Continue across chats and do not wait on unrelated pending checks. The backup is immutable.

## Before context exhaustion

Commit safe completed slices and record exact source SHA, tests, Actions evidence and unverified boundaries. Never claim tests, builds, TRX, artifacts or hardware checks that did not pass.
