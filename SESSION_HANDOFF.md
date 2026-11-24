## Current repository state

- Parent main for this FRP tri-state shutdown fix: `8584845fa7ce78b928bfb6e47c5cf8eb8223982f`; branch and tree were fetched immediately before building the complete atomic tree.
- Adbyby executable/start_time hardening: exact host #45339125309 and Process fail-closed #45339125213 PASS on `17d08de1ed22fe2ff102992d28c974b42b77de3a`.
- AdGuardHome executable/config/workdir argv + start_time identity: config-security #45339988745, Process inventory #45339988758 and AI docs #45339988750 PASS on `cbbb27636807b7f195d7d87ccefb476366a07995`.
- Latest main before this slice `8584845fa7ce78b928bfb6e47c5cf8eb8223982f`: HTTPD security #45340186122 and AI handoff #45340186109 PASS.
- This FRP slice fixes loss of UNKNOWN process state and prevents TERM→KILL escalation when procfs metadata or the wait primitive fails.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- All writes remain main-only, complete-tree, single-commit, expected-SHA non-forced updates; no force-push.

## Active task

Implement and verify FRP tri-state process inventory and safe escalation:
- `frp_process_matches()`: 0 for exact owned executable+argv; 1 only for vanished/unowned; 2 when an extant matching executable has unreadable/incomplete cmdline or procfs identity cannot be inspected.
- `frp_service_pids()`, `frp_service_running()` and `frp_any_running()` propagate UNKNOWN rather than converting command-substitution failures to ABSENT.
- `frp_wait_stopped()`: 0 confirmed stopped, 1 verified still-running timeout, 2 UNKNOWN or failed sleep.
- `frp_stop_daemons()` permits SIGKILL only after the five-probe TERM wait returns verified-timeout status 1.
- New tests cover bad proc root, missing cmdline, post-TERM metadata loss and sleep failure.
- Exact-head FRP lifecycle, Process inventory/fail-closed and AI handoff Actions are required after publication.

## Completed development slices

- Adbyby executable/start_time identity: host #45339125309 and Process fail-closed #45339125213 PASS on `17d08de1`.
- AdGuardHome executable/config/workdir argv + start_time identity: config-security #45339988745, Process #45339988758 and AI docs #45339988750 PASS on `cbbb2763`.
- SmartDNS stop independent of config NVRAM reads: DNSMasq #45336719172, Process #45336719252, AI docs #45336719249 and USB #45336719220 PASS on `2294ae89`; its XY-C1 build was not confirmed completed at last poll.
- HTTPD security + AI handoff on the immediate parent main: #45340186122 and #45340186109 PASS.
- Router hostname/domain callback fixes on predecessor: WAN DHCP #45339568432, WAN route #45339568336 and DNSMasq #45339568424 PASS on `a26746d`.

## Verification boundary

- This FRP source/test/checker change is pending publication and exact-head Actions; no FRP/Process/AI result is claimed for it yet.
- No MIPS/uClibc firmware build, TRX artifact or physical-router L4 result is claimed for this shell-only slice.
- Linux 4.4 lacks pidfd; process identity checks reduce risk but cannot eliminate the final procfs-check-to-kill(2) race.

## Exact first action for the next GPT

1. Fetch live `main` and inspect this commit's exact-head FRP lifecycle, Process inventory/fail-closed and AI handoff results; fix the first failing regression before marking it complete.
2. If these checks pass or remain pending, continue an independent P1 stability/security fix outside completed SmartDNS, Adbyby, AdGuardHome and RAETH areas.
3. Keep the frozen backup immutable; all main changes remain complete-tree atomic commits with expected-SHA non-forced ref updates.

# Cross-session execution mandate

Treat this as continuous work across GPT conversations. Recover once, then execute on `main`; each logical source or CI change is one atomic main commit. Create all changed blobs, one complete Git tree and one commit before updating the branch with a non-forced expected-SHA compare-and-swap. Never force-push or modify the frozen backup.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST perform a handoff checkpoint before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

Every session MUST inspect live repository state before ending. Record only material exact-head Action transitions, new source commits, genuine unresolved failures or a changed first action. Do not manufacture documentation churn or no-op commits.

## Development loop

- Source modification → targeted regression → one complete Git tree/one atomic commit → exact-SHA Actions.
- PASS advances; FAIL is diagnosed and fixed; PENDING does not block independent work.
- Preserve Linux 4.4, GNU89, MIPS/uClibc compatibility, router configuration, security, architecture and backward compatibility.
- Before context exhaustion, record exact latest SHA, tests, CI state, unresolved issues and one concrete first task. Never claim unverified work.
