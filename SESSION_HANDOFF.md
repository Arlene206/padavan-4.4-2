# SESSION HANDOFF — 2026-11-23

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`.
- Main before this follow-up: `da0b2cb22dd9e2ebba346f0d34ad1a67071f7764`; frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` and read-only.
- No local checkout is available; do not claim local tests or builds. Only `main` is writable.

## Exact-SHA Actions for the prior ZeroTier slice
- Prior source commit: `da0b2cb22dd9e2ebba346f0d34ad1a67071f7764`.
- At the last poll, ZeroTier lifecycle `45130983855`, AI handoff `45130983929`, XY-C1 `45130984042`, WAN route `45130983999`, and Process inventory `45130983955` were all IN_PROGRESS on that exact SHA. Recheck before claiming any result.
- Wireless regression `45130357263` and AI docs `45130357161` passed on `c79bb0c...`. XY-C1 `45130357194` completed workflow SUCCESS but skipped Build/TRX/artifact; no L3 claim.
- Integrated HNAT/SFE/NAT-helper XY-C1 run `45129983650` on `2d5f218...` was still IN_PROGRESS at the last query; recheck exact status.

## Active task — protect ZeroTier identity and running service on NVRAM read errors
- Defect: legacy `nvram get` conflates a backend/read/close error with an empty value. An unreadable permanent `zerotier_secret` could trigger new identity generation, and other selectors could be silently dropped.
- Commit `da0b2cb...` added five `nvram get_checked` reads and all-reads-before-publish handling. Review identified one ordering gap: `start_zero()` stops the old daemon before calling `start_instance()`, so a checked read only inside `start_instance()` would still take down a working service on a transient NVRAM error.
- THIS FOLLOW-UP moves the preflight to `start_zero()`, ahead of `kill_z()` and firewall replay; `start_instance()` consumes only the already-validated settings. The new shell regression injects a failed secret read and asserts no process signal, network cleanup or relaunch. The source contract pins the same ordering.
- Exact-head ZeroTier lifecycle, XY-C1, AI, WAN route and Process Actions will be checked after this commit. No local test/build claim.

## Next actions
1. Fetch live `main`, inspect THIS FOLLOW-UP's exact-head ZeroTier lifecycle run and fix the first causal failure without weakening tests.
2. Inspect matching XY-C1 Build, TRX and artifact steps; L3 requires all three to PASS on one production-source SHA.
3. Continue with checked reads for `zerotier_nat` and static-route configuration, including fail-closed cleanup semantics.
4. Recheck the integrated HNAT/SFE/NAT-helper build and L4 router validation; L4 remains NOT VERIFIED.

## Validation boundary
L1 source review is complete for the ordering correction. L2 requires the matching ZeroTier shell and Python checks. L3 requires same-SHA Build/TRX/artifact. L4 physical router validation remains NOT VERIFIED.

## Cross-session mandate
Continue small atomic source/test commits, use one complete tree/commit per logical slice, update `main` with `force:false` and expected parent SHA, inspect exact-SHA Actions, never mutate the frozen backup, and never claim an unobserved result.

---
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

### Re-run exact-prior-state wireless radio compensation regression

- Production-source commit: `2d5f218f7bbfcfb0f140be9c04f072173fe328c8` (parent `6e4c7f27cfad65fe437e88055e4f774d068ba437`). The current main also contains a docs-only NAT/offload checkpoint `b2577fe95d0783d6a7168ab94237d582ea43bc6c`. Frozen backup `72f4f157a084f8184b9597ca5bf334b8080e5807` remains untouched/read-only.
- The runtime transition snapshots `mlme_radio_wl/rt` via `nvram_get_checked()` before issuing power commands. It restores the exact prior value on power/publish failure; unreadable/invalid state aborts with no hardware mutation. Regressions cover read ordering, ON restoration after failed OFF, same-state ON publication failure, OFF restoration after command failure, and read-error no-side-effect behavior.
- Exact-head Wireless run `45129983626` on `2d5f218...` FAILED in the inline Python source contract with `NameError: name 'policy' is not defined` at line 134. The workflow already loaded the policy header as `notification_policy`; the new assertions referenced an undefined name. THIS FOLLOW-UP switches those assertions to `notification_policy` and does not change production source.
- Exact `2d5f218...` status from the latest recorded checkpoint: Wireless `45129983626` FAIL (checker NameError); AI docs `45129983606`, Process inventory `45129983638`, Process fail-closed `45129983589`, HTTPD security `45129983625`, iNIC signal `45129983697`, watchdog signal `45129983726`, and NVRAM transaction audit `45129983737` PASS; XY-C1 `45129983650` remained IN_PROGRESS. The follow-up needs a new Wireless run; verify the exact `2d5f218...` XY-C1 result separately.
- No local checkout, local host test or local firmware build is claimed. L3 requires Build XY-C1, TRX verification and artifact upload all PASS on one production-source SHA. L4 physical-router validation remains NOT VERIFIED.

### Related pending verification

- HW NAT selector source checker mismatch fixed in `6e4c7f2...`: shared `scripts/check_modem_wan_start_status.py` and inline HTTPD source contract now expect `rc_nat_offload_settings_read()` plus local `hw_nat_mode`, not the removed inline unchecked getter. Exact `6e4c7f2...` Firewall Startup/HTTPD/Process/USB modem/XY-C1 checks were green after the change; see current action history before repeating claims.
- NAT helper snapshot tests passed sanitizer binaries on `a408ef...` in `45127938295`, but the workflow-header parser in the source contract failed; `9e8e8ae...` bounds parsing at `jobs:`. Confirm a new exact-head NAT/WAN run.
## Verification boundary

- Historic libdisk ACL, FRP private credentials, USB printer hotplug and DHCPv6 callback milestones retain only their earlier exact-source evidence; do not transfer those claims to newer source SHAs.
- PID 1 recovery-shell timezone `acb8eff...`: exact timezone workflow `45126854225` PASS and Process/DNSMasq/DNS Forwarder/HTTPD/AI docs passed; L3 XY-C1 close not established.
- Wireless regulatory SKU publication `83145a...`: Wireless regression `45126385953` and AI docs `45126385917` PASS; exact-source XY-C1 close not established.
- SYN-cookie integer parsing `3a802b...`: WAN route, Process inventory and AI docs PASS; NVRAM audit cancelled; XY-C1 confirmation not recorded.
- HW NAT selector checked transaction: stale shared/inline checker expectations fixed in `6e4c7f2...`; its exact Firewall Startup/HTTPD/Process/USB modem/XY-C1 checks were green at the later recorded checkpoint.
- Wireless exact-prior-state rollback source `2d5f218...`: Wireless run `45129983626` failed only on undefined `policy` in its inline source checker; AI docs `45129983606`, Process inventory `45129983638`, Process fail-closed `45129983589`, HTTPD `45129983625`, iNIC `45129983697`, watchdog `45129983726`, and NVRAM audit `45129983737` PASS. XY-C1 `45129983650` was still in progress at last record.
- THIS FOLLOW-UP corrects the workflow variable reference; no Actions for the correction yet. L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main`; do not repeat startup recovery or touch the frozen backup.
2. Inspect the new exact-head Wireless radio-state workflow after the `notification_policy` variable correction. Verify the sanitizer-built policy executable and source integration contract.
3. Check `2d5f218...` XY-C1 run `45129983650` and record Build, TRX and artifact jobs by exact production-source `head_sha`; L3 requires all three PASS on the same source SHA.
4. Recheck NVRAM audit and Process fail-closed status from `2d5f218...`; continue NAT helper exact-head test/checker verification after the `jobs:` parser fix. L4 remains NOT VERIFIED.
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
