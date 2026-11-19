# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Restore exact live source and CI state once, then resume concrete engineering tasks. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits or stop at planning.

- Create all changed blobs, one complete Git tree and one commit with current main as parent; update main using non-forced compare-and-swap with that exact expected SHA.
- Verify Actions by exact `head_sha`. Pending runs do not block independent work; inspect and repair real failures.
- Preserve security, architecture, GNU89/MIPS/uClibc compatibility, regression coverage, lifecycle identity checks, bounded I/O and fail-closed UNKNOWN handling.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` is archival only; never alter it. Develop on main.

## Current repository state

- Parent main SHA before this focused security slice: 6e7c7034abde6bbb437e7941c751ba40100fe168 (ci(firewall): align shared netmask source contract).
- The preceding handoff was correcting bounded MTD EINTR handling. Its Process fail-closed and AI-docs exact-head Actions were still in progress at last recorded check; re-query them only by their original SHA if relevant.
- Frozen backup branch backup/pre-plugin-overhaul-2026-11-14 remains at 72f4f157a084f8184b9597ca5bf334b8080e5807 and is read-only. Develop only on main.
- This task creates one source commit directly after the parent above. Resolve the new exact source SHA from live main and check all Actions against that exact SHA.

## Active task

- Confirmed a privileged rp-L2TP configuration publication vulnerability. PID 1 sets umask(0000) in init.c; start_rpl2tp() previously called mkdir_if_none("/etc/l2tp", "777") and ignored its result, then opened /etc/l2tp/l2tp.conf with fopen("w"). The directory helper follows an existing symlink-to-directory and does not harden an existing directory, while the leaf open follows a pre-planted symlink. This could leave /etc/l2tp world-writable and let a local user redirect a root config write.
- Added l2tp_config_file.h: reject symlinked/foreign-owned directories, create at 0755, strip group/other write permissions through the opened fd, verify the pathname still names the checked inode, and stage the complete config in a unique same-directory file before atomic rename. A destination symlink is replaced rather than followed; failed write/flush/fsync/close/rename operations preserve the last-good config.
- Routed APP_RPL2TP through the new writer and added GNU89 filesystem regressions for permissive umask, directory-mode repair, symlink-directory refusal, symlink-leaf replacement, last-good preservation and temporary-file cleanup. VPN lifecycle CI compiles/runs the regression and checks production wiring; XY-C1 watches the new production header.
- Exact-head VPN, AI-docs and XY-C1 Actions must be queried after the commit. No local compiler, target MIPS/uClibc result or physical-router behavior is claimed.

## Exact first action for the next GPT

1. Fetch live main once and query exact-head VPN server stability, XY-C1 firmware validation and AI handoff docs for the new source SHA; inspect job logs, especially the rp-L2TP regression/source contract and Build/TRX/artifact stages.
2. PASS is valid only for the exact source SHA. Repair causal failures without weakening security assertions; do not wait on pending XY-C1 before independent work.
3. Continue a separate evidenced cross-userland reliability/security defect. Do not re-audit completed service-wait, process-I/O or MTD EINTR slices without new evidence.

## Verification boundary

- L1: rp-L2TP no longer writes through a symlinked leaf or accepts a writable/foreign-owned/symlinked configuration directory; complete same-directory staging is atomically published at mode 0644. PID 1's umask(0000) is explicitly handled.
- L2: GNU89 regression covers directory mode under permissive umask, mode repair, directory and leaf symlinks, atomic replacement, writer failure, last-good preservation and temporary cleanup. Exact-head workflow results require a post-commit query.
- L3: the new net_ppp.c/header product-source change requires Build XY-C1, TRX verification and artifact upload all PASS on the same new head SHA.
- L4: MIPS/uClibc runtime behavior, concurrent WAN transition behavior and physical-router validation remain NOT VERIFIED.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST perform a handoff checkpoint before ending. At session end, compare live main, production-source baseline, exact-head Actions, module state and the next task. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit; do not create a no-op commit.

## Before context exhaustion

- Finish the current safe source/test slice and commit it atomically before handoff; do not publish related multi-file changes as sequential partial commits.
- Re-fetch live `main`, create one complete Git tree and one commit with that exact parent, then update main only with `force=false` and the exact parent as `expected_sha`. Never force-push.
- Query relevant Actions by exact source SHA and record PASS/FAIL/PENDING with run IDs. Pending validation does not block independent work and must not be reported as complete.
- Record the production-source baseline, test commands/results, exact CI state, unresolved limitations and the next concrete task. Distinguish host L2, firmware L3 and device L4 evidence.
- Confirm all work is committed and remotely present before claiming it is preserved across conversations.

## Development invariants

- Main-only; never alter the frozen backup or develop on a legacy PR.
- One complete tree and one atomic commit per logical task; `force=false`, exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits or weaken tests.
- Keep L1/L2/L3/L4 distinct. Host tests are not firmware or hardware validation.
