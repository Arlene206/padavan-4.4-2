# SESSION HANDOFF — Padavan 4.4

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`. Parent for this offload checker follow-up: `93bbadd21165a9458e97b166a47d4e0cfc1ab447`; production wireless source is `6deab04c8fe625abd785447a7a558d45eebf7766`. Resolve live `main` on recovery; do not assume the checkpoint SHA if another commit landed. Immutable backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.
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

### Correct offload selector source contracts and re-verify the wireless transaction

- Current parent `main`: `93bbadd21165a9458e97b166a47d4e0cfc1ab447`. Production wireless change: `6deab04c8fe625abd785447a7a558d45eebf7766`; Wireless MII assertion-only correction: `93bbadd...`. Frozen backup `72f4f157a084f8184b9597ca5bf334b8080e5807` remains read-only.
- The HW NAT/SFE selector source change (from the earlier `c3894b...` / `849edf...` chain) now obtains `sw_mode`, `hw_nat_mode`, `ip6_lan_auto` and `sfe_enable` in a checked transaction, then assigns local `hw_nat_mode = nat_offload_settings.hw_nat_mode`. It no longer calls `nvram_get_int("hw_nat_mode")` inside `reload_nat_modules()`.
- Exact `6deab04...` CI exposed stale test contracts, not a production compile/runtime failure: `scripts/check_modem_wan_start_status.py` asserted the removed inline getter at line 1319, and `.github/workflows/httpd-security.yml` duplicated that obsolete expectation at line 1365. This caused Firewall Startup `45128851136`, XY-C1 `45128851172`, Process fail-closed `45128851128`, USB modem identity `45128851336`, and HTTPD security `45128850957` to fail at those assertions. Wireless `45128851387` also failed its inline Python contract: earlier wireless/SKU/bridge/guest-LAN checks printed PASS, but line 162 then tripped over the same stale HW NAT selector expectation.
- THIS COMMIT replaces both stale assertions with checks for `rc_nat_offload_settings_read()`, the local `hw_nat_mode` assignment and absence of the old unchecked getter. The shared Python checker is already path-filtered in Firewall Startup, XY-C1 and Process workflows; the HTTPD workflow already filters its inline contract changes. No production source is changed by this follow-up.
- Exact `93bbadd...` follow-up runs at the last lookup: Wireless radio state `45128993444`, XY-C1 `45128993406` and AI docs `45128993523` were IN_PROGRESS. The prior `6deab04...` wireless run failed at the stale selector contract; do not call the production regression itself failed until this new contract version runs.
- After this follow-up, inspect exact-head Firewall Startup, HTTPD security, Process fail-closed, USB modem identity, WAN route/NAT and XY-C1 jobs. No local checkout, local host tests, firmware build or physical-router test is available/claimed. L3 requires Build XY-C1, TRX verification and artifact upload all PASS on one production-source SHA; L4 remains NOT VERIFIED.

### Other open CI items (not to lose)

- NAT helper transactional snapshot tests passed sanitizer binaries on `a408ef...` in `45127938295`, but the checker failed at the workflow-header boundary. `9e8e8ae...` fixed the parser boundary; a new exact-head NAT/WAN result is still needed.
- Parent `849edf...` also had Firewall Startup `45128657449` and HTTPD security `45128657626` failing. Their descendant failures were traced to the same stale HW NAT selector assumptions now corrected above; require clean reruns before marking fixed.
- L4 physical device validation remains NOT VERIFIED.
## Verification boundary

- Historical libdisk ACL, FRP private credentials, USB printer hotplug and DHCPv6 callback milestones retain their earlier exact-source evidence only; no later SHA inherits those claims automatically.
- PID 1 recovery-shell timezone `acb8eff...`: exact timezone workflow `45126854225` PASS and Process/DNSMasq/DNS Forwarder/HTTPD/AI docs passed; XY-C1 L3 evidence was not established.
- Wireless regulatory SKU publication `83145a...`: Wireless regression `45126385953` and AI docs `45126385917` PASS; no exact-source XY-C1 close recorded.
- SYN-cookie integer parser `3a802b...`: WAN route, Process inventory and AI docs PASS; NVRAM audit `45127632188` CANCELLED; XY-C1 `45127632232` was in progress, so no L3 claim.
- Wireless MLME source `6deab04...`: exact Actions included Wireless `45128851387` FAIL at the inline source contract after prior checks printed PASS; WAN route `45128851449` PASS; AI docs `45128851235` PASS; NVRAM audit `45128850916`, storage hotplug `45128851432`, Process inventory `45128851220` and AliyunDrive `45128851473` IN_PROGRESS at the observed snapshot. Firewall `45128851136`, HTTPD `45128850957`, Process failclosed `45128851128`, USB modem `45128851336` and XY-C1 `45128851172` failed at the stale HW NAT selector assertion.
- Wireless checker correction `93bbadd...`: exact Wireless `45128993444`, XY-C1 `45128993406`, AI docs `45128993523` were IN_PROGRESS at the last query. The HW NAT selector stale-contract corrections in THIS COMMIT require new exact-head runs.
- L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main`; don't repeat the startup/handoff audit or touch the backup.
2. Match this checker follow-up's exact SHA, then inspect Firewall Startup, HTTPD security, Process fail-closed, USB modem identity, Wireless radio, WAN/NAT and XY-C1 Actions by exact `head_sha`. Diagnose first causal log lines on any FAIL.
3. Confirm the shared Python source checker and HTTPD inline contract now pass the `rc_nat_offload_settings_read()`/local `hw_nat_mode` contract. Re-run NAT sanitizer regression after the earlier `jobs:` parser correction.
4. L3 only when Build XY-C1, TRX verification and artifact upload all PASS on the same production-source SHA. L4 physical router remains NOT VERIFIED.
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
