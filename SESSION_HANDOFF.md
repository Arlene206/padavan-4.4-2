## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch `main`. Startup protocol and one-time handoff recovery are complete; do not repeat the initial audit.
- Latest product-source commit: `e0efa3a421051d24a21b8519983892ffdef75456`. Current parent for the PID-identity refinement: `ae33ceced794ded4c70c72feb74dd03afa3f3f4b`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify. PR #1 remains closed/unmerged; CA trust-store bootstrap is integrated.

## Active task

- Tighten detached WAN gateway monitor readiness. A plain procfs presence scan may observe the daemonized child between `exec` and PID-file publication; that does not prove `detect_wan_main()` completed its required startup publication.
- The readiness callback now calls `process_pidfile_read_identity("/var/run/detect_wan.pid", "detect_wan", ...)` and requires the PID plus recorded procfs start-time to match a live process. Missing/stale identity is ABSENT and retried; unreadable/malformed/indeterminate state is UNKNOWN and fails closed; bounded ABSENT becomes ETIMEDOUT.

### Completed initial WAN source slice

- Commit `e0efa3a421051d24a21b8519983892ffdef75456` added the launch wrapper around `eval("detect_wan")`, bounded detached-daemon postcondition, tri-state propagation and EINTR-preserving wait.
- The current refinement replaces the weaker generic process-presence scan with PID-file/start-time identity. `process_pidfile_read_identity()` validates the secure PID record against the live process inventory.
- The existing GNU89/ASan detached-daemon regression covers delayed appearance, absent-child timeout, UNKNOWN, launcher failure and interrupted waits. `check_wan_detect_wan_status.py` asserts the PID-file identity call and exact launch order.

## Exact-SHA Actions checkpoint

- On exact source SHA `e0efa3a421051d24a21b8519983892ffdef75456`, WAN route #46154268118 and WAN ARP socket #46154268207 both PASS. These verify the initial process-scan version; rerun both for the PID-identity refinement.
- NVRAM transaction boundary #46154268084 PASS on `e0efa3a`, including all host regressions and the raw userland ioctl/kernel API caller audit.
- Supporting exact-SHA checks on `e0efa3a` PASS: Process inventory #46154268105, Process lifecycle fail-closed #46154268187, DNSMasq lifecycle, VPN server lifecycle, WAN DHCP callback, WAN teardown, detect_link/detect_internet, HTTPD security, USB modem identity, firewall startup, miniupnpd publication, WAN sysfs counters, router-mode NVRAM, IPv6 service mode, wireless radio and ZeroTier.
- XY-C1 Firmware Validation #46154268098 remained IN PROGRESS at last check on `e0efa3a`; this PID-identity refinement requires a fresh exact-SHA Build, TRX verification and artifact upload.
- AI handoff run #46154268128 failed on `e0efa3a` because the roadmap exceeded the byte cap. Run #46154469011 failed after the first doc correction because the canonical `## Active task` heading was missing. Commit `ae33ceced794ded4c70c72feb74dd03afa3f3f4b` restored that heading and exact-head AI handoff run #46154702646 PASS; this new handoff/roadmap update requires its own exact-head run.
- Older XY-C1 baseline `4652b6801ec4c04c0ccfd07c677d7dbec70b39ce` passed Build/TRX/artifact but does not verify later `net_wan.c` changes. Physical-router L4 remains NOT VERIFIED.

## Verification boundary

- L1/L2: initial process-scan version passed WAN route and WAN ARP on `e0efa3a`; the PID-file identity refinement requires fresh exact-SHA runs.
- L3: fresh exact-SHA XY-C1 Build/TRX/artifact required; the run on `e0efa3a` does not cover this source refinement.
- L4: physical WAN/gateway-loss and router runtime behavior NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-SHA WAN route, WAN ARP, AI handoff and XY-C1 Actions for the PID-identity refinement. Read any failed job log; do not wait on pending jobs.
2. After the WAN and XY-C1 checks complete, resume an independent high-priority userland stability/security defect. NVRAM transaction boundary already passed on `e0efa3a`; do not repeat that audit without new evidence.
3. Keep the backup branch immutable and use complete-tree single-commit writes with a non-forced expected-SHA update.

## Mandatory end-of-session handoff checkpoint

Mandatory handoff checkpoint. The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect the live repository state and exact-head Actions before ending. Record material PENDING-to-PASS/FAIL transitions. If all checkpoint facts and the first action remain accurate, leave the handoff untouched and do not create a no-op commit.

# Cross-session execution mandate

- one atomic main commit per logical task: create all required blobs, one complete Git tree, one commit, then a non-forced expected-SHA update. Never force-push or publish per-file partial commits.
- Before context exhaustion, inspect the exact-head Actions and leave the next GPT an accurate task, first action, tests and L1-L4 verification boundary.
- Keep working while independent Actions are pending; PASS continues, FAIL requires causal repair, PENDING permits independent work. Do not ask the user what to do next.
