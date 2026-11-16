# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers, frozen backup and precise first action for the next session. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, explicitly conclude the check and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, do not wait for pending CI when independent work can proceed, and do not ask the user what to do next.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main, followed by a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, use a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Parent main: 03ad069f57518208d4546ce844b9e5e09b00a365; preceding commit 03ad069 made restart_firewall() return update_upnp() status and covered notification/CLI/SSH/HTTPD callers. THIS COMMIT completes remaining storage, IPv6, VPN and UPnP-restart callback consumers.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14: 72f4f157a084f8184b9597ca5bf334b8080e5807; immutable and read-only.
- Parent exact-head firewall-startup 43932064517, AI docs 43932065074, USB modem 43932065139, WAN route 43932065079, WAN DHCP 43932065052, VPN server 43932064898 and multiple suites passed. Process aggregate 43932065043, XY-C1 43932065369, USB storage 43932065453, SkipDBv2 43932064473 and NVRAM audit 43932064356 were pending at last poll.
- Historical PR #1 remains closed/unmerged and superseded by mainline commit 74b66ff960457379518d13c723c44a3515e6a228; do not resume.

## Active task

The generic restart_firewall() result is already returned by the preceding commit, and PID 1, CLI, SSH and HTTPD now preserve it. THIS COMMIT closes the remaining dropped-return paths: FTP/Transmission/Aria/storage, IPv6 disable, VPN server restart and restart_upnp() callback policy. Existing service/teardown status retains first-error precedence. The firewall rebuild helper itself remains void, so the reported result specifically covers the UPnP mapping refresh.

## Verification boundary

- Added the upnp_restart_policy.c failure-injection case verifying that a successful UPnP stop followed by firewall/UPnP refresh failure returns that error; stop errors still prevent firewall restart.
- Extended scripts/check_modem_wan_start_status.py to assert status propagation through storage, IPv6, VPN and UPnP restart callbacks; firewall workflow compiles the policy with GNU89/-Werror/ASan/UBSan and runs the checker.
- No local full-tree checkout/build is available. THIS COMMIT exact-head firewall, Process, XY-C1 and AI docs results must be inspected. L3 only after same-SHA XY-C1 Build/TRX/artifact all pass. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head firewall-startup, Process, XY-C1 and AI docs runs for THIS COMMIT; diagnose failures from the first causal job step.
2. Confirm the new UPnP policy test, complete source-contract checker and same-SHA XY-C1 Build/TRX/artifact results.
3. Continue P1 investigation of reload_nat_modules() and start_firewall_ex(): trace module-load/unload postconditions and each configured vs optional feature before defining a failure policy.
4. Keep the frozen backup read-only and physical-router L4 marked NOT VERIFIED.
