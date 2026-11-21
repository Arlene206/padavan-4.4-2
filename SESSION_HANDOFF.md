# Cross-session execution mandate

## Workflow
- Develop on `main` only. One logical slice, one atomic main commit: create all blobs, one tree and one commit, then update main with non-forced expected-SHA compare-and-swap. Never force-push, amend, create branches/PRs, or edit the immutable backup.
- Preserve security, architecture, compatibility and necessary tests. Target GNU89, MIPS/uClibc, BusyBox and Linux 4.4. Never weaken regression contracts.
- Exact-head Actions validate only the matching SHA. Do not wait on PENDING; continue independent work. Fix a real FAIL at its cause.
- Keep root `dev_log.md` newest-first with 20–119 entries. Preserve `docs/dev-log/2026/11/part-021.md` and its index. No no-op commits or handoff snapshot archives.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect the live repository state before ending and compare it with this handoff: engineering/source baseline, active task, exact-head Actions, validation boundary, blockers and the exact first action. If all checkpoint facts and the first action remain accurate, explicitly conclude the check with no file change and no commit. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information when it changes the validation gate or next action.

## Current repository state
- Parent main before this MiniUPnP service-policy fix: `f99d7a43be07851f39b0773b62d91aa29fc05235`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, immutable.
- Firewall fail-closed NVRAM hardening remains committed across `ee01141a`, `4a02532f` and `f99d7a43`: firewall enable, DoS, MAC, URL/webstr, LWF and inverse URL settings now abort staged ruleset publication on checked-read errors while preserving missing-key defaults.
- DNSMasq's five resolver-policy reads are checked before config side effects; DNSMasq lifecycle #44902327424 PASS and AI docs #44902327438 PASS on `5a4366981557ddef27f4bb0717543f84b353b36c`.
- WAN protocol parameter reset uses checked transactional NVRAM copy; exact WAN route #44902460823 PASS and AI docs #44902460812 PASS on `dffca184e64dc44d8cfae11a43bbcebe21d9e0b6`.
- URL/LWF firewall slice #44902745711 PASS on `f99d7a43be07851f39b0773b62d91aa29fc05235`. AI docs #44902745743, WAN DHCP #44902745734, WAN route #44902745725, Process inventory #44902745720, HTTPD #44902745884 and FULLCONENAT #44902745686 also PASS on that exact SHA. NVRAM audit #44902745715 and XY-C1 #44902745804 were IN_PROGRESS at the last poll.
- XY-C1 run #44902070311 on product-source SHA `232e31d7eeaa5a6a5e7510ce74a59d9b2f9a7950` ultimately completed successfully: Build XY-C1, Verify TRX output and artifact upload all PASS. This verifies that earlier production-input baseline at L3; it does not validate newer production commits. Physical router L4 remains NOT VERIFIED.
- New confirmed MiniUPnP defect: `start_upnp_locked()` / `update_upnp_locked()` collapsed errors from `nvram_get_int("upnp_enable_x")`, `nvram_get_int("wan_nat_x")` and `get_ap_mode()` into ordinary false values. This could suppress a needed start or stop a running daemon and drop active mappings on transient NVRAM errors. This slice reads all three under one checked NVRAM transaction and publishes a stop decision only after successful transaction close.
- Legacy PR #1 remains closed/unmerged and is not to be revived. Immutable backup unchanged.

## Active task
- Verify exact-head MiniUPnP lifecycle regression, XY-C1 firmware validation and AI handoff docs for the checked service-policy fix.
- Confirm both start/update gates call `miniupnpd_read_stop_policy()`; `upnp_enable_x`, `wan_nat_x` and `wan_route_x` must remain transactionally checked; there must be no unchecked integer/get_ap_mode read deciding daemon state.
- Ensure read/close errors leave the decision output untouched, return an error, and update releases the process snapshot while preserving the active daemon.
- Then continue a distinct P1 stability/security defect in another Padavan userland subsystem; don't repeat completed firewall/WAN/DNSMasq/UPnP audits without new evidence.

## Verification boundary
- New MiniUPnP service policy and tests are included in the atomic main slice. Exact-head MiniUPnP/AI/XY-C1 Actions pending.
- Existing MiniUPnP regression is CI-compiled with GNU89 and `-Wall -Wextra -Werror`; this policy is not yet covered by a new standalone sanitizer run.
- Full MIPS firmware, TRX and artifact results must be read from the exact SHA run after this slice. Earlier successful build at `232e31d7` does not prove newer product-source builds.
- No local RC compile, full firmware build or physical router L4 is claimed.

## Exact first action for the next GPT
1. Fetch live main and inspect exact-SHA MiniUPnP lifecycle, XY-C1 Build/TRX/artifact and AI handoff runs for this slice.
2. If a gate fails, read its exact job log and fix the underlying contract/source without weakening tests.
3. Continue one distinct confirmed P1 reliability/security defect outside this MiniUPnP service-configuration path.

Before context exhaustion, update this handoff only when active task, exact-head status, validation boundary or first next action materially changes; no no-op commit.
