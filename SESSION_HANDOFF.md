# SESSION HANDOFF — Padavan 4.4

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`. Live development ref is `main`; the parent of the current DHCPv6 source/test slice is `20012786541a3789f0391793345e403eb96c77d8`. Resolve THIS COMMIT from live history; do not create another commit merely to replace its placeholder SHA.
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

### Rerun WAN callback contract after DHCPv6 retry integration

- Product source commit `80811fc53de0257bf44973fa0d703014981fcaf5` implements DHCPv6 Reply script-result retry: nonzero callback status retains the event, resets retry parameters and schedules a five-second retry; Information Refresh Time is armed only after callback success.
- Dedicated DHCPv6 workflow `45117871967` on exact SHA 80811 completed SUCCESS, including GNU89 ASan/UBSan policy regression and source contract.
- Exact WAN callback workflow `45117872004` on 80811 failed only because its final inline source contract still expected the removed old DHCPv6 log strings. All preceding WAN/IPv6/LAN compiled regressions and source assertions passed. THIS COMMIT updates the two stale assertions to require the retry gate, new logs, retry-parameter reset and retry timer; no product source changes.
- After this checker-only correction, require exact-head WAN callback rerun to pass. Also establish exact-SHA XY-C1 MIPS/uClibc build, TRX and artifact status for product SHA 80811; no exact-head XY-C1 run had appeared in the latest returned push-run inventory. Do not claim L3 without it.
- Once these checks are recorded, continue with a separate source-backed defect across another userland subsystem. Do not idle-poll Actions.

### Exact parallel baseline

- Before this DHCPv6 change, product source baseline was `20012786541a3789f0391793345e403eb96c77d8`. Its storage regression run `45116268933` and exact XY-C1 target run `45116268960` both completed SUCCESS.
- FRP checker exact rerun `45116127095` completed SUCCESS on `41285d396d7bd1f984f13c49699ed6dacbd27e92`. The d2c exact XY-C1 run `45115500761` completed SUCCESS. e757 exact XY-C1 run `45114634020` completed SUCCESS.
- No physical router test has been performed; L4 remains NOT VERIFIED.

## Verification boundary

- Prior libdisk ACL: L1/L2/L3 PASS on the SHA/run evidence listed above; L4 NOT VERIFIED.
- FRP private seeding: focused shell regression and source contracts PASS; exact d2c XY-C1 Build/TRX/artifact PASS. L2/L3 PASS; L4 NOT VERIFIED.
- USB printer unplug: exact storage-hotplug GNU89 ASan/UBSan/source-contract PASS and exact 200127 XY-C1 Build/TRX/artifact PASS. L2/L3 PASS; L4 NOT VERIFIED.
- DHCPv6 Reply callback retry source `80811fc53de0257bf44973fa0d703014981fcaf5`: dedicated exact-head host workflow `45117871967` PASS (GNU89 ASan/UBSan + source contract), so L2 PASS. The WAN callback integration workflow `45117872004` failed only in two stale source log assertions; THIS COMMIT updates that contract. L3 remains NOT VERIFIED: no exact-head XY-C1 run was found for 80811 in the latest returned push inventory. L4 NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live main and verify the frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`. Resolve THIS COMMIT and inspect the WAN workflow-only diff; do not assume this saved parent remains live.
2. Inspect the exact-head WAN callback push run for this correction. It must pass the final inline source contract that now expects DHCPv6 retry logs and timer behavior; if it fails, inspect the first assertion and update the contract without weakening product checks.
3. Establish exact-SHA XY-C1 firmware evidence for product SHA `80811fc53de0257bf44973fa0d703014981fcaf5`: verify production-input detector, Build XY-C1, TRX verification and artifact upload. No L3 claim until all three stages pass on that same production-source SHA.
4. The dedicated DHCPv6 host regression `45117871967` is already PASS. Other previous L2/L3 source milestones are recorded above; don't repeat them. Continue another independent, concrete whole-userland stability/security task while any checks are pending.
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
