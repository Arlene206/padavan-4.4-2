## Current repository state

- Live branch: `main` at `a76f771cd56c1ad623769d32400562f853330e34`; RC source fix `7ca46fc7049f3380755f6b2fe5073d86727c6382` is based on `e1631a606564e95a93802123ceba97478ac3197c`. Current main has one subsequent RAETH Clause 45 ioctl bounds commit `a76f771`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. Legacy PR #1 remains closed and unmerged.
- Every logical task must be one atomic main commit: create a complete Git tree and one commit, then update `main` by non-forced expected-SHA compare-and-swap. No additional branches, PRs or force pushes.
- The SmartDNS checker/runtime regression correction was on `79d912269f2ec021e21d1f543fe879140f4aab6e`; its exact-head Process, DNSMasq, USB storage and AI documentation checks passed. Its XY-C1 run reported success for host checks but skipped Build/TRX/artifact, so it is not L3 firmware evidence.

## Recent concurrent RAETH work

- Main also contains `c0ba5c6`, `56f4a10`, `4ab45d3`, `4920912` and `a937cc4`: QDMA queue-index and MMIO-offset bounds, ESW ingress/egress rate ioctl operand bounds, and QDMA register-path validation.
- Switch CLI run #45432229601 on `4920912` failed only because its new source contract extracted the egress case from the ingress case body, so its first matched register read belonged to ingress. The follow-up `a937cc4` changes the checker to extract each case by explicit switch-case boundaries. Parent `a937cc4`: Switch CLI #45432913975 PASS, NVRAM #45432913762 PASS, AI handoff #45432913776 PASS; XY-C1 #45432913772 must be checked for final Build/TRX/artifact results. Follow-up `a76f771` bounds Clause 45 ioctl addresses; validate its exact-head Switch CLI and XY-C1 actions separately.

## Active engineering task

### Preserve firewall startup prerequisite failures in the PID 1 boot result

- In `trunk/user/rc/src/rc.c:init_router()`, checked firewall/NAT NVRAM reads and bridge/IPv6 status already feed `boot_services_status`. But `ipt_nat_default()`, `ipt_filter_default()` and `ip6t_filter_default()` feed the WAN safety gate without their applicable failure statuses being retained in the boot result.
- A required NAT/filter failure can block WAN startup while `init_router()` still returns READY, contrary to PID 1's READY/DEGRADED contract.
- The committed source change extracts a status-returning firewall gate helper, keeps the first applicable selector/VLAN/NAT/IPv6/filter error, and merges it into `boot_services_status` before choosing the safe no-WAN branch. Failures belonging to explicitly disabled features remain non-blocking for compatibility. Exact-head host C tests passed on `7ca46fc`; its workflow static contract failed on an assertion that expected a split assignment to appear on one line. The current correction checks the assignment and helper call independently.
- GNU89 regression coverage checks error precedence, exact errno, disabled-feature compatibility and READY-to-DEGRADED mapping. The Firewall Startup workflow's source contract requires the status merge before WAN launch and its path filters cover both headers used by the new regression.

## Verification boundary and exact next action

1. Fetch the corrective commit SHA and inspect exact-head Firewall Startup Stability Regression and AI handoff Actions; ensure the whitespace-tolerant assignment check passes. Recheck XY-C1 #45434352284 on source SHA `7ca46fc`: Build, TRX and artifact must all PASS before claiming L3.
2. Check exact-head actions for the newer RAETH Clause 45 commit `a76f771` (Switch CLI and XY-C1); do not use parent results to certify a newer SHA.
3. If any exact-head run FAILS, inspect its job log and fix that regression on a new atomic `main` commit. PENDING runs do not block independent implementation.
4. After both current slices are green, continue the broad RC startup-result propagation P1 queue or another higher-risk unfinished userland stability issue outside SmartDNS/DNS Forwarder.

## Mandatory handoff checkpoint

At session end compare the live `main`, frozen backup, active task, source SHA, exact-head Actions, L3/L4 boundary and this exact first action. Do not repeat completed checks or create a no-op documentation commit.

# Cross-session execution mandate

Recover once, implement, and hand off. Keep one atomic main commit per logical task. Never force-push or overstate tests, firmware builds or hardware validation.
