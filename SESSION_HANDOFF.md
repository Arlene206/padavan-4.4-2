## Current repository state

- Live branch: `main` at `a08b86ed129624bf4a56c95a4cc3ec694d88e7fe`. RC production fix `7ca46fc7049f3380755f6b2fe5073d86727c6382` is followed by firewall checker commits `03ca0dbf`, `77dc29c` and `1c4cdd6`. RAETH includes Clause 45 address bounds `a76f771`, MDIO Clause 22 address bounds `7efbd10`, source-order checker correction `2f95d1c` and shared Clause 45 MDIO guard `a08b86e`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. Legacy PR #1 remains closed and unmerged.
- Every logical task must be one atomic main commit: create a complete Git tree and one commit, then update `main` by non-forced expected-SHA compare-and-swap. Do not create extra branches or force-push.
- SmartDNS checker/runtime correction at `79d912269f2ec021e21d1f543fe879140f4aab6e` passed Process, DNSMasq, USB and AI host checks; XY-C1 skipped Build/TRX/artifact, so it is not L3 evidence.

## Active task

### Preserve firewall startup prerequisite failures in the PID 1 boot result

- `rc.c:init_router()` previously let failed required NAT restore or IPv4/IPv6 filter operations block WAN startup without retaining their error in `boot_services_status`. PID 1 could therefore report READY while WAN was deliberately withheld.
- `firewall_startup_policy_wan_failure_status()` now retains the first applicable selector/VLAN/NAT/IPv6/filter error; `init_router()` merges it into `boot_services_status` before the no-WAN branch. Explicitly disabled features remain compatible.
- GNU89 regression tests cover exact errno propagation, first-error precedence, disabled-feature behavior and READY-to-DEGRADED mapping.
- Firewall source assertions normalize wrapped assignments and require that the applicable failure is merged before `start_wan()`.

## Verification boundary

- Firewall source SHA `7ca46fc7049f3380755f6b2fe5073d86727c6382`: Firewall Startup policy and checked-NVRAM host C regressions passed. The source-contract failures on descendants `03ca0dbf` and `77dc29c` were checker formatting/selection defects, not C regression failures.
- Firewall run #45436267565 on `1c4cdd6` passed the firewall-specific source assertions and host C binaries, but failed `scripts/check_forwarding_control_status.py`, whose extraction still expected the retired `firewall_startup_policy_allows_wan_checked()` call. This commit moves that check to the current rejection branch `if (firewall_startup_status != 0)`, keeping its requirements that forwarding-disable errors are captured and WAN is not started inside rejection.
- AI handoff #45436267737 on `1c4cdd6` PASS. On RAETH `2f95d1c`, Switch CLI #45435916372 PASS and AI handoff #45435916408 PASS.
- RAETH shared Clause 45 guard `a08b86e`: AI handoff #45436446803 PASS; Switch CLI #45436447892, NVRAM #45436447356 and XY-C1 #45436447371 were IN_PROGRESS at last poll.
- Firewall-source XY-C1 #45434352284 on `7ca46fc`: Build in progress, TRX and artifact pending at last poll. RAETH Clause 45 XY-C1 #45434537034 on `a76f771`: Build in progress, TRX and artifact pending. Keep these exact SHAs separate.
- L3 requires Build, TRX and artifact PASS on the same exact production source SHA. Physical-device L4 is NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect the exact-head Firewall Startup and AI handoff Actions triggered by this checker correction; fix any remaining source-contract failure without relaxing the safety assertion.
2. Recheck XY-C1 #45434352284 on `7ca46fc` for final Build/TRX/artifact status; do not claim L3 until all three pass.
3. Inspect Switch CLI #45436447892 and NVRAM #45436447356 for the shared Clause 45 guard commit; XY-C1 #45436447371 must pass Build/TRX/artifact for that exact SHA before L3.
4. Once these independent checks are settled, continue the P1 RC startup-result propagation queue or a higher-risk unfinished userland stability issue outside SmartDNS/DNS Forwarder. A pending check is not a stop condition.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session ending must inspect live main, the frozen backup, active source SHA, exact-head Actions, validation boundary and the first action. When all remain accurate, explicitly conclude the checkpoint with no file change and no no-op commit.

# Cross-session execution mandate

Recover once, implement and hand off. Keep one atomic main commit per logical task; create all blobs, one complete tree and one commit, then update main by non-forced compare-and-swap. Before context exhaustion, record exact tests, Actions results and the next precise action. Never force-push or overstate firmware/hardware evidence.
