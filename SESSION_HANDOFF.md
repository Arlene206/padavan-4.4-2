# SESSION HANDOFF — Padavan 4.4

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`. Live development ref is `main`; parent for the current MiniDLNA launch-state follow-up is `507d80d8901131f75259a3b1f8d8f7837605fa3c`. Re-read main before every write and use expected-SHA compare-and-swap.
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

### Make MiniDLNA launch decisions fail closed on service-state NVRAM errors

- run_dms() still had two unchecked NVRAM decisions after the configuration and rescan fixes: nvram_match("apps_dms", "1") collapsed a backend error into 'disabled', silently skipping the daemon; nvram_safe_get("il0macaddr") collapsed an identity read failure into an empty MAC and launched MiniDLNA with a zero server identity.
- THIS COMMIT uses storage_read_string_status() for both values before any database-link/config side effects. Missing apps_dms remains the historical disabled default (empty != "1"); read errors return their status. Missing il0macaddr remains an empty string and preserves legacy zero-MAC formatting, while backend/truncation errors abort launch.
- The existing checked dlna_rescan read remains conditional on !force_rescan, preserving caller override semantics. Workflow source contracts pin all three checked reads, no old lossy getter, and error gates before filesystem mutation.
- Prior MiniDLNA fixture/source contract exact runs: 511f1eed... run 45120292969 PASS; 9597283... runs 45120463448 (MiniDLNA) and 45120463437 (AI handoff) PASS. The rescan fix is in commit 507d80d8901131f75259a3b1f8d8f7837605fa3c; exact MiniDLNA run 45120721550 PASS and AI handoff 45120721566 PASS. Exact XY-C1 45120721414 is pending Build on the 507 product-source tree; TRX/artifact pending.
- THIS COMMIT changes production services_stor.c, so it needs fresh exact-head MiniDLNA source/host checks and XY-C1 Build/TRX/artifact. Continue separate userland work while pending; do not re-audit completed MiniDLNA INI/fixture/rescan or other fixed modules.
### Exact parallel baseline

- Earlier source milestones: libdisk ACL L3 PASS on e757 `45114634020`; FRP L3 PASS on d2c `45115500761`; USB printer unplug L2/L3 PASS on 200127 `45116268933` / `45116268960`.
- DHCPv6 product source SHA 80811 has dedicated host/sanitizer/source-contract PASS `45117871967`. WAN integration source-contract rerun after correction commit 8411 passed `45118077270`.
- Main then advanced with firewall source commit `3eba7fc820f0c3f9dc2dd7a80db313b39f63fff1`; it checks the optional IPTV interface using a bounded NVRAM snapshot before TTL side effects. Exact XY-C1 run `45118311710` on 3eba was IN_PROGRESS (ccache stage) at the last lookup; TRX/artifact pending.
- No exact completed XY-C1 target result existed yet for the DHCPv6 retry product source. This path filter correction should start that run. Do not transfer target validation evidence across unrelated source SHAs.
## Verification boundary

- Prior libdisk ACL: L1/L2/L3 PASS on the SHA/run evidence above; L4 NOT VERIFIED.
- FRP private seeding: focused shell regression/source contracts PASS; exact d2c XY-C1 Build/TRX/artifact PASS. L2/L3 PASS; L4 NOT VERIFIED.
- USB printer unplug: storage-hotplug GNU89 ASan/UBSan/source contract and exact 200127 XY-C1 Build/TRX/artifact PASS. L2/L3 PASS; L4 NOT VERIFIED.
- DHCPv6 Reply callback retry source `80811fc53de0257bf44973fa0d703014981fcaf5`: dedicated host run `45117871967` PASS (L2); WAN integration rerun `45118077270` PASS. L3 NOT VERIFIED: exact XY-C1 run on latest tree was still building and the source path coverage gap is fixed by THIS COMMIT. L4 NOT VERIFIED.
- Firewall IPTV TTL interface source commit `3eba7fc820f0c3f9dc2dd7a80db313b39f63fff1`: source review and tests added; exact XY-C1 run `45118311710` pending at## Exact first action for the next GPT

1. Read live main once; verify frozen backup SHA 72f4f157a084f8184b9597ca5bf334b8080e5807; resolve THIS COMMIT.
2. Inspect exact-head MiniDLNA run for checked apps_dms, dlna_rescan and il0macaddr handling; require host test and source checker PASS.
3. Inspect exact-head XY-C1: L3 requires Build XY-C1, TRX verification and artifact upload all PASS on the same product SHA.
4. Continue an independent userland stability/security defect while checks are pending. L4 physical-router validation remains NOT VERIFIED.
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
