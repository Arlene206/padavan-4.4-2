# Cross-session execution mandate

## Workflow
- Develop on `main` only. One logical slice, one atomic main commit: create all blobs, one tree and one commit, then update main with non-forced expected-SHA compare-and-swap. Never force-push, amend, create branches/PRs, or edit the immutable backup.
- Preserve security, architecture, compatibility and necessary tests. Target GNU89, MIPS/uClibc, BusyBox and Linux 4.4. Never weaken regression contracts.
- Exact-SHA Actions validate only the matching SHA. Do not wait on PENDING; continue independent work. Fix real failures at the first causal point.
- Keep root `dev_log.md` newest-first with 20–119 dated entries. Preserve `docs/dev-log/2026/11/part-021.md` and its index. No no-op commits or handoff snapshot archives.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional. Before context exhaustion, inspect live main and compare this resumable record: source baseline, active task, exact-head Actions, validation boundary, blockers and first action. If all remain accurate, make no handoff commit.

## Current repository state
- Parent main for the current product fix: `b33f359bf38874d530db2a99c7ca345cb55f0a80`. Frozen archival branch `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. Legacy PR #1 is closed/unmerged.
- SSH checked-mode fix is in the product baseline: public start and restart use checked `sshd_enable` parsing; restart reads before inventory/stop and reuses the checked mode. Exact Process/SSH and PID-1 queue checker corrections have been committed; PID-1 #44906182543 PASS and AI docs #44906182540 PASS on `b33f359`. Process lifecycle #44906182539 is still running at the required PID 1 boot-environment failure gate.
- RT3352 MII TxPower inconsistency identified: Ralink AP configuration generation clamps invalid TxPower values outside 0..100 back to 100, but the live radio transition passed raw `nvram_wlan_get_int(0, "TxPower")` directly to `iwpriv`. A stale/corrupt negative or >100 value could make a later radio-enable command fail even though the generated configuration uses the safe default. This commit centralizes the existing 0..100/default-100 policy and applies it to both paths with GNU89 boundary tests.
- The wireless radio workflow now also watches `ralink.c`, and its source contract verifies that both config generation and runtime TxPower use the same helper. Exact-head Wireless and XY-C1 validation are pending.
- Existing pass results on earlier exact heads: router-mode policy/wiring #44904910387 PASS on `0398903`; NFS #44904764717, HTTPD #44904764718 and NVRAM #44905882978 PASS on `79e1932`; USB storage #44905882960 and a broad service suite PASS on `79e1932`. MiniUPnP SOAP internal-port-zero parsing is already implemented in `1eb7581` with tests, preserving AddAnyPortMapping external port zero; do not restart it as a missing task.
- XY-C1 #44905883001 for source SHA `79e1932` remains in progress at Build XY-C1; TRX and artifact steps were pending at last observation. The prior host-only XY-C1 #44905479795 skipped Build/TRX/artifact and is not L3. The current TxPower product commit should trigger a fresh same-SHA firmware run.
- Physical router L4 remains NOT VERIFIED.

## Active task
1. Inspect exact-head Wireless radio state, Process lifecycle, PID-1 notification queue and AI docs runs triggered by the TxPower commit. Fix only real failures; preserve all existing checks.
2. Inspect the new XY-C1 Build XY-C1, Verify TRX output and artifact upload on the exact TxPower product-source SHA. Claim L3 only after all three pass on that same SHA.
3. Continue the next confirmed P1 reliability/security defect across Padavan userland. Avoid repeated audits of completed SSH/NFS/HTTPD/router-mode/USB work and avoid treating the already implemented MiniUPnP parser as missing.

## Verification boundary
- The current TxPower helper, runtime callsite, AP config generator, GNU89 regression boundaries and workflow source/path contract are committed together; exact-head Actions pending.
- PID-1 notification queue and AI docs PASS on `b33f359`. Process lifecycle still pending at the required PID 1 boot-environment checker step.
- XY-C1 Build/TRX/artifact on prior production SHA `79e1932` has not yet completed; no L3 claim for this or the new TxPower source SHA. No physical hardware L4.
- No local RC cross-compile or physical router test is claimed.

## Exact first action for the next GPT
Fetch live main and inspect the exact-head Wireless radio policy, Process lifecycle, PID-1 queue and AI docs checks. Check the newest XY-C1 job's Build, TRX and artifact steps separately; keep working on an independent confirmed source defect while any jobs are pending.

Before context exhaustion, update this handoff only when active task, exact-head result, validation boundary or first action materially changes; no no-op commit.