# SESSION HANDOFF — Padavan 4.4

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`. Parent `main` SHA for the current bounded SYN-cookie integer parsing slice: `2e2af029329e422135bbbe0f376924beb2f7f251`; the source checkpoint is the commit containing this handoff update. Fetch live `main` on recovery and resolve the exact current SHA. Immutable backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- The only authorized refs are `main` and immutable archive `backup/pre-plugin-overhaul-2026-11-14`. Backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`. Never modify, delete, rename, merge, rebase, cherry-pick, reset, prune or develop from the backup.
- The repository has no local checkout in this session. Source changes are authored through GitHub Git Data API; do not claim local shell tests, host compiles or firmware builds.
- Legacy PR #1 is closed and unmerged (base `9d5a5cb4ab574c0e610ed7a0815c0b1ad749da74`, head `ba48e22609e0c81ad757b06756fe91d05f39cbf6`). Its AliDDNS HMAC/EVP_MAC, CA bootstrap, package and test changes are already present in current main. Do not revive its old head.

## Completed source and validation milestones

### libdisk removable-share ACL records

- Source SHA `e757f86309b19f890797b2f36fb15f44f87a7e95` added unique line-anchored, case-insensitive folder ACL record handling; duplicate and malformed records are rejected. Checker-only descendant `5378fccc2201f4359ea3b0b37f1043d2b813a883` fixed undefined caller-body extraction variables. Exact descendant libdisk run `45114769360` passed both GNU89 ASan/UBSan binaries and the Python source contract.
- Exact-source e757 XY-C1 run `45114634020` completed SUCCESS with firmware build, TRX verification and artifact upload. L3 closed for that source SHA. NVRAM transaction audit `45114633929`, HTTPD security `45114633840` and Process inventory `45114634024` also completed SUCCESS.

### FRP credential storage

- Product commit `d2c388a04f6125666043bf94672a8f4222655d4b` routes `/etc/storage/frp_script.sh` through `seed_storage_file()` with mode 0700 and enforce-existing-mode flag. It preserves operator file content, restores private mode and uses staged exclusive publication instead of the prior check-then-`cp -f` path.
- Shell regressions cover private creation, existing content preservation/mode repair, injected chmod failure and source wiring. Checker correction `f9320f888b7ac284a4b657b1ae58feca40f0428f` made Process run `45115896497` PASS; final FRP checker correction `41285d396d7bd1f984f13c49699ed6dacbd27e92` made exact FRP run `45116127095` PASS. Exact d2c XY-C1 run `45115500761` completed SUCCESS with target build/TRX/artifact; L2/L3 closed for FRP source.

### USB printer unplug event acknowledgement

- Commit `20012786541a3789f0391793345e403eb96c77d8` preserves the result from `stop_usb_printer_spoolers()`, skips marker clear when stopping fails, sends the real status through `usb_hotplug_event_ack_status()` and schedules retry. This prevents acknowledging `usb_unplug_lp` while a printer daemon may still be running.
- Positive `EBUSY` regression and source contract passed exact storage-hotplug run `45116268933`. Exact-source XY-C1 run `45116268960` SUCCESS included target build, TRX verification and artifact upload. L2/L3 are closed for the 200127 source SHA. L4 physical-router test remains NOT VERIFIED.

## Active task

### Bound SYN-cookie policy integer parsing

- Parent `main` SHA: `2e2af029329e422135bbbe0f376924beb2f7f251`. Development remains on `main`; frozen backup `72f4f157a084f8184b9597ca5bf334b8080e5807` remains read-only.
- The checked-read fix prevents NVRAM backend/close errors from being mistaken for an explicit `fw_syn_cook=0`, but its formatter still used `atoi()`. An out-of-range decimal string can invoke undefined behavior and return an implementation-dependent value, potentially weakening the SYN-cookie WAN admission policy.
- THIS COMMIT replaces `atoi()` with `strtol()` and explicit `INT_MIN`/`INT_MAX`/`ERANGE` checks. It preserves legacy base-10 numeric-prefix semantics for representable values, including missing-key zero and nonnumeric-to-zero behavior, while rejecting overflow without changing the caller's initialized secure-required state.
- Added exact INT boundary tests, positive/negative overflow tests, a very-long input test and source-checker assertions. The existing `wan-route-status.yml` already compiles the GNU89/Werror/ASan/UBSan regression and runs the checker; no path-filter change is needed.
- No local checkout is available; no local compile/test or firmware build is claimed. Inspect exact-head WAN route, Process, NVRAM and XY-C1 Actions. L3 requires Build XY-C1, TRX and artifact upload all PASS on the same production-source SHA; L4 remains NOT VERIFIED.
- Resulting source/test/docs tree: THIS COMMIT (parent `2e2af029...`).

### Exact parent-SHA verification baseline

- On `2e2af029329e422135bbbe0f376924beb2f7f251`: Firewall Startup `45127089949`, Crond `45127089701`, Process integration `45127089841`, Process inventory `45127089854`, HTTPD `45127089789`, WAN DHCP callback `45127089717`, DNSMasq `45127089801`, WAN route `45127089737`, watchdog `45127089944`, AI docs `45127089716`, USB hotplug `45127089749`, Router-mode NVRAM `45127089874` and NVRAM audit `45127090003` were all SUCCESS. XY-C1 `45127089883` was IN_PROGRESS at the last exact-head lookup.
- Do not transfer parent-SHA results to THIS COMMIT. No L2/L3/L4 success is claimed until this commit's own matching Actions complete.

## Verification boundary

- Historic libdisk removable-share ACL, FRP private seeding, USB printer unplug and DHCPv6 callback milestones retain exact-source test/build evidence in the earlier handoff history; do not infer fresh validation for later SHAs.
- PID 1 recovery-shell timezone: exact-host workflow `45126854225` PASS on `acb8eff551bc6a189c8d207cd7d01cf9c21edca3`; Process, DNSMasq, DNS Forwarder, HTTPD and AI docs also PASS on that SHA. XY-C1 was still running at last query; no L3 claim.
- Wireless SKU atomic publication: Wireless regression `45126385953` and AI docs `45126385917` PASS on parent `83145a6da831afad15b2676b7086463802f253a6`; later XY-C1 verification for its exact source SHA was not established in this checkpoint.
- SYN-cookie checked-read source `2e2af029329e422135bbbe0f376924beb2f7f251`: Firewall Startup, Crond, Process, HTTPD, WAN DHCP, DNSMasq, WAN route, watchdog, AI docs, USB hotplug and Router-mode NVRAM exact-head Actions PASS; NVRAM audit PASS. XY-C1 `45127089883` was IN_PROGRESS. The follow-up integer-range validation in THIS COMMIT requires its own exact-head regression and firmware checks.
- L4 physical-router validation remains NOT VERIFIED for the above current changes.

## Exact first action for the next GPT

1. Fetch live `main` and match the current SYN-cookie integer parser commit SHA; do not repeat startup-protocol or frozen-backup audits.
2. Inspect the exact-head WAN route workflow: the sanitizer binary and `scripts/check_network_sysctl_status.py` must both pass. On failure, diagnose the first causal assertion or compiler/test error and correct it without weakening policy.
3. Check Process inventory/integration, NVRAM audit and XY-C1 runs by exact `head_sha`. L3 requires Build XY-C1, TRX verification and artifact upload all PASS on one production-source SHA; L4 remains NOT VERIFIED.
4. Continue the next independent high-priority userland issue in the roadmap after this security parsing slice is verified; do not revisit completed timezone, console-loglevel or switch packed-operand work.
# Cross-session execution mandate

- The same maintainer continues the same Padavan repository mission across conversations. Restore the handoff once, then implement; do not repeat completed audits or ask the user what to do next.
- Stability defect discovery and repair remains primary; security hardening continues in parallel. Critical vulnerabilities, data corruption, boot blockers and severe service faults take priority. Expand across the integrated userland rather than staying in one subsystem.
- For each logical source/test slice, fetch latest main, create every changed blob, one complete Git tree and one commit, then advance main with a non-forced expected-SHA compare-and-swap. One logical task is one atomic main commit. Never create per-file partial commits, force-push, amend, rewrite history, create working/checkpoint branches or develop through PRs.
- Verify landed main SHA and complete diff. Inspect only push Actions whose `head_sha` exactly matches the commit. PASS means continue; FAIL means diagnose the first causal failure and fix; PENDING means advance independent work, never idle-poll.
- Preserve safety, architecture, compatibility and necessary tests. Review actual production callers/callees before editing. Do not weaken checks to make Actions green.
- Distinguish L1 source review; L2 host build/regression; L3 MIPS/uClibc target build, TRX and artifact upload on the same production-source SHA; L4 physical router validation. Never transfer evidence between SHAs or overstate the validation level.
- Keep GPT Session Rotator unchanged. Do not reset its workflow/state or replay SHA `9d5a5cb`.
- Before context exhaustion, finish the active safe slice, inspect the full diff, publish it and inspect exact-SHA Actions. Preserve an accurate next first task; never claim uncommitted work was remotely saved.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect live GitHub before ending and compare source baseline, active task, exact-head Actions, validation boundary, unresolved blockers, frozen backup identity and exact first action with this file. Update the handoff only when next-session-critical information materially changes. If it remains accurate, conclude the checkpoint with no file change and no commit. Do not chase documentation-only SHA changes or create a no-op commit.
