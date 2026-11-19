# 2026-11-19 continuation checkpoint — RStats publication permission hardening

# Cross-session execution mandate

Continue main-only implementation across conversations. Do not repeat completed audits or stop at a plan.

- one atomic main commit per logical task: create all blobs, one complete tree and one commit with the current main parent, then update main through non-forced compare-and-swap with that exact expected SHA.
- Before context exhaustion, verify the active slice and its exact-head Actions, then leave the next concrete action.
- Never weaken security, compatibility or regression coverage to change run counts.

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; branch `main`.
- Parent main SHA for this RStats security fix: `c4d2080843e9a39d917daf5f03e5ec340d89b039`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 remains closed/unmerged.
- Prior VLAN tri-state fix product SHA `2ff30969d5e326d64fe2ca1857e31b97fbfe90b6` has exact XY-C1 Build, TRX and artifact upload PASS in run `44467380728`; WAN route checker correction `fea88d3f18d7aac9c46dda14fd2c0fc2cc2180bd` passed run `44467726365`; handoff validator correction `c5cc3169e7a3f05562312af44204ee33381aa6ba` passed AI docs run `44468202433`.
- Current main `c4d2080843e9a39d917daf5f03e5ec340d89b039` includes Samba fastpath gating. Its AI docs `44468787338`, Process lifecycle/inventory `44468787409`, USB modem `44468787083`, vsftpd `44468787082`, firewall `44468787038`, HTTPD `44468787064`, process inventory `44468787109`, USB storage `44468787143`, NFS `44468787110`, and Samba `44468787171` runs all PASS. XY-C1 `44468787084` and NVRAM audit `44468787130` were IN_PROGRESS at this checkpoint; do not treat L3 as verified for c4d until Build, TRX and artifact upload all pass.

## Active task

Fix a concrete RStats file-publication permission regression.

- Before this change, both `rstats_file_write_atomic_with_ops()` and `rstats_file_open_stream()` initialized new-target mode to literal `0666`, then applied it with `fchmod()` to the `mkstemp()` file. Unlike the former `open(..., O_CREAT, 0666)`/stdio creation semantics, this explicitly bypassed the inherited umask. Fresh `/etc/storage/rstats-history` and `/var/spool/rstats-*.js` outputs could therefore become world-writable.
- The fix computes the effective creation mode as `0666 & ~umask`, restores the original process umask immediately, and still preserves an existing regular file's current mode. RStats is single-threaded and this helper runs in normal context, not from a signal handler.
- Added a GNU89 regression that creates both an atomic raw-history file and a JSON stream under umask `0027`, requires mode `0640`, and verifies the inherited umask remains unchanged. The RStats workflow now checks this policy and the XY-C1 workflow watches the new production header path.

## Verification boundary

- L1: source-level permission issue and both creation paths identified; fix and regression added in this commit.
- L2: exact-head RStats host regression/source contract is pending; no local checkout/build is available in this session, so do not claim a local test run.
- L3: exact-head XY-C1 Build, TRX verification and artifact upload are pending for this production-header change.
- L4: MIPS/uClibc device behavior and physical-router verification are NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch current main and inspect the exact-head RStats workflow run and XY-C1 run for this commit; confirm the RStats GNU89 ASan/UBSan binary passes and XY-C1 Build, TRX and artifact upload all succeed.
2. If any exact-head check fails, diagnose and fix that failure without weakening the permission test or dropping the XY-C1 production-header trigger.
3. While either run is pending, continue an independent P1 lifecycle/startup-status defect; do not wait idle. After this slice closes, continue the unfinished whole-userland stability/security queue.

## Mandatory handoff checkpoint

Mandatory handoff checkpoint: every session MUST inspect live main, the current product-source SHA, relevant exact-head Actions, current verification boundary, and the next concrete action.

The checkpoint is mandatory; a document edit is conditional. Update only when these facts materially change; leave an accurate handoff untouched.

## Development invariants

- Main-only; never modify the frozen backup.
- One complete Git tree and one atomic commit per logical task; use `force=false` and exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits or weaken tests.
- Actions must be reported by exact SHA; host tests, firmware builds and physical-device checks are distinct validation levels.
