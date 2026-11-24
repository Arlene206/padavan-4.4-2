## Current repository state

- Active branch: `main`; parent for the XUPnP configuration-pair transaction: `aae46a6391f1d50c3122ac4e7f4098175023d832`. Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. No open PRs.
- MTK AP private-ioctl, HTTPD selector, WAN source-PHY range and QMI WDS/PDH teardown fixes retain recorded exact-SHA L3 Build/TRX/artifact evidence. Physical-router L4 remains unverified.
- DHCPDECLINE retry/send-result chain: `4ca919b` / `02ddda7` / `4015869` / `aae46a6`. Exact-head udhcpc GNU89 ASan/UBSan regression/source contract #45400755028 PASS after the renamed test runner was fixed. AI handoff #45400755063 and Process lifecycle #45400755089 PASS; USB storage #45400755158 and NVRAM #45400755163 were pending at last poll. XY-C1 for production `4015869` #45400399157 was still building; TRX/artifact pending. Latest `aae46a6` is test/docs-only.
 
## Active task — Publish XUPnP generated Lua configuration as a rollback-capable pair

- `start_xupnpd()` previously published `common.lua` before it opened, read and staged `xupnpd.lua`. A later source/NVRAM/write/fsync/second-rename failure could leave mixed old/new config files even though startup failed.
- Both files are now fully rendered, flushed and fsynced before either destination changes. `dnsmasq_file_publish_pair()` snapshots both prior paths and restores the first if the second rename fails. A preserve-mode sentinel keeps each stage's already-selected permission bits.
- GNU89 ASan/UBSan regression checks distinct mode preservation and forces a real second-rename failure by unlinking the second staging pathname while the stream remains open; both old files and modes must be restored. DNSMasq lifecycle runs a source-order contract and watches `net.c`.

## Verification boundary

- Parent `aae46a6391f1d50c3122ac4e7f4098175023d832`: udhcpc event-script policy/source regression, AI handoff and Process lifecycle PASS; USB storage and NVRAM audit pending at last poll.
- New DNSMasq pair regression/source contract, AI, Process/NVRAM and XY-C1 Build/TRX/artifact must be checked on exact production SHA. No new L3 claim until same-SHA Build/TRX/artifact all pass. Physical-router L4 remains unverified.

## Exact first action for the next GPT

1. Fetch live `main` and query exact-head Actions.
2. Inspect DNSMasq lifecycle pair regression/source contract and compile results; fix the first causal failure atomically.
3. Verify XY-C1 Build/TRX/artifact on the production SHA, then continue the next independent stability/security defect.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. At session end, inspect live main, relevant exact-head Actions, validation boundary, blockers and the first next action. If all checkpoint facts remain accurate, make no file change and no commit.

# Cross-session execution mandate

Recover once and continue on main. Never create a development branch or PR; the backup ref is immutable.

For every multi-file logical slice, create all blobs, one complete tree based on latest main, one commit with the exact parent, then update main with a non-forced expected_sha compare-and-swap. This is one atomic main commit. Never create partial commits, force-push or rewrite history.

Continue implementing safe fixes rather than stopping at audits. PASS advances; FAIL is diagnosed and corrected; PENDING does not block independent work. Preserve Linux 4.4, GNU89, MIPS/uClibc and router compatibility.

Before context exhaustion, finish the current safe slice, record the latest SHA/tests/CI/unresolved issue and one first action, append material milestones to dev_log.md, and update roadmap only for a material module/status/priority change.