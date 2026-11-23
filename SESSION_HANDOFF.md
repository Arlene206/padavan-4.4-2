# SESSION HANDOFF — Padavan 4.4

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`. Parent for the current exact-prior-radio-state compensation slice: `6e4c7f27cfad65fe437e88055e4f774d068ba437`; previous wireless runtime-state source: `6deab04c8fe625abd785447a7a558d45eebf7766`. Resolve live `main` on recovery. Immutable backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.
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

### Restore the actual previous MLME radio state on failed transitions

- Parent `main`: `6e4c7f27cfad65fe437e88055e4f774d068ba437`. The production power/publish transaction was introduced in `6deab04...`; this follow-up hardens its rollback semantics. Frozen backup `72f4f157a084f8184b9597ca5bf334b8080e5807` remains read-only.
- Defect: `wireless_radio_state_transition()` rolled back to `!target_on`, assuming the prior state was always opposite the requested state. For a same-state request (e.g. radio already ON, requested ON), a failed `mlme_radio_*` NVRAM publication would incorrectly turn a working radio OFF. It also had no explicit guard against an unreadable prior state.
- THIS COMMIT adds a `get_power` callback, snapshots the persisted `mlme_radio_wl/rt` state with `nvram_get_checked()` + `wireless_radio_nvram_flag_status()`, and aborts before issuing any driver command if the snapshot is unavailable/invalid. Both hardware-command and publication failures now best-effort restore the exact snapshot, preserving the primary error and exposing rollback errors separately.
- Regression cases cover read-before-write ordering, restoring ON after an OFF request fails publication, preserving ON after an ON-to-ON publication failure, restoring the prior OFF state after a power command fails, and no hardware side effects on NVRAM read failure. Updated the Wireless workflow source contract to require the checked snapshot, exact-state rollback and new tests. No RT3352 MII isolation logic was changed.
- `6e4c7f2...` checker-only exact-head Actions observed after commit: XY-C1 `45129577565`, AI docs `45129577593`, Firewall Startup `45129577563`, Process fail-closed `45129577560`, HTTPD security `45129577524`, and USB modem identity `45129577627` were queued/in progress. The new HW NAT selector checker changes need those results to finish before claiming PASS/FAIL.
- Wireless source verification: `93bbadd...` Wireless radio regression `45128993444` PASS and AI docs `45128993523` PASS. Its XY-C1 `45128993406` FAIL was triggered by the stale HW NAT selector assertion now fixed in `6e4c7f2...`; re-check the new exact-head XY-C1. No local checkout, local host test or local firmware build is available/claimed. L3 requires Build XY-C1, TRX verification and artifact upload all PASS on one production-source SHA; L4 remains NOT VERIFIED.

### Other open CI items

- NAT helper snapshot: sanitizer test binary passed on `a408ef...` in `45127938295`, but checker failed at workflow-header parsing; `9e8e8ae...` bounds parsing at `jobs:`. Confirm NAT exact-head rerun after current shared checker correction.
- The `6e4c7f2...` Firewall Startup, HTTPD, Process fail-closed, USB modem and XY-C1 workflows must re-run after replacing the stale `hw_nat_mode` inline getter expectation with the checked transaction/local selector contract.
- L4 physical device validation remains NOT VERIFIED.
## Verification boundary

- Historic libdisk ACL, FRP private credentials, USB printer hotplug and DHCPv6 callback milestones retain only their earlier exact-source evidence; do not transfer them to later SHAs.
- PID 1 recovery-shell timezone `acb8eff...`: exact timezone workflow `45126854225` PASS and Process/DNSMasq/DNS Forwarder/HTTPD/AI docs passed; L3 XY-C1 completion not established.
- Wireless regulatory SKU publication `83145a...`: Wireless regression `45126385953` and AI docs `45126385917` PASS; exact-source XY-C1 close not established.
- SYN-cookie integer parser `3a802b...`: WAN route, Process inventory and AI docs PASS; NVRAM audit `45127632188` CANCELLED; XY-C1 was pending, so no L3 claim.
- Wireless MLME checked publication source `6deab04...`: `93bbadd...` Wireless regression `45128993444` PASS and AI docs `45128993523` PASS; XY-C1 `45128993406` failed at stale `hw_nat_mode` selector contract, corrected in `6e4c7f2...`.
- Checker-correction commit `6e4c7f27cfad65fe437e88055e4f774d068ba437`: exact Firewall Startup `45129577563`, XY-C1 `45129577565`, AI docs `45129577593`, Process fail-closed `45129577560`, HTTPD security `45129577524`, and USB modem identity `45129577627` were queued/in progress at the last poll; do not mark any pending run as PASS.
- Exact-prior-state radio compensation in THIS COMMIT: no Actions yet; Wireless sanitizer/source-contract, Process inventory and XY-C1 must be matched to the resulting commit SHA.
- L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main`; don't repeat the startup-protocol or frozen-backup audit.
2. Inspect this commit's exact-head Wireless radio state regression. Verify GNU89/Werror/ASan/UBSan callback tests and the source checker; repair any first causal error.
3. Recheck the `6e4c7f2...` exact-head Firewall Startup, HTTPD, Process, USB modem and XY-C1 jobs after the shared/inline HW NAT contract fixes.
4. Match XY-C1 Build, TRX verification and artifact upload by exact production-source `head_sha`; L3 closes only if all three PASS on that same SHA. L4 physical-router validation remains NOT VERIFIED.
5. Re-run/confirm NAT helper sanitizer and source checker after the `jobs:` parsing fix. Continue other independent stability/security source tasks while queued jobs run.
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
