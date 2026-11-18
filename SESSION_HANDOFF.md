# 2026-11-18 continuation checkpoint — EINTR-safe startup wait contract repairs

## Current repository state
- Repository `Arlene206/padavan-4.4-2`, development branch `main`. Live main immediately before this checkpoint: `f87b0b67cc918c1011714ab09f04497e7566e9a4`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`. PR #1 remains closed/unmerged; useful AliDDNS CA/HMAC/bootstrap changes are already present on main. Do not mutate the backup, PR #1 or GPT Session Rotator.
- Recent product milestones include SmartDNS DNSMasq transactional preservation (`9bb9fda`), EINTR-safe WAN DHCP/PPP settling waits (`e0c8269`), and NFSd restart settling wait (`90cce742`). Current SmartDNS autostart malformed-state/invalid-port fixes are regression-only.

## Active task
- Finish aligning regression contracts with the shared interrupt-safe wait API. WAN source in `e0c8269` replaced raw DHCP settling `sleep(2)` with `service_wait_seconds(2)`, propagates errno/EIO and logs failure. Commit `106af73` corrected one stale assertion in the embedded WAN callback workflow, but exact run `44323140500` exposed a second stale assertion at stdin line 895 / workflow line 1069 that still required the former `if (wait_dhcpc && launch_status == 0)` guard. THIS COMMIT updates that final assertion to require the EINTR-safe guarded wait and no raw sleep.
- Process run `44323338947` on `f87b0b67cc918c1011714ab09f04497e7566e9a4` passed SmartDNS process shell tests and the SmartDNS fail-closed source contract, then failed because `scripts/check_process_lifecycle_integration.py` still expected a direct `sleep(PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS)` in `start_lltd()`. Current LLTD and rstats launchers correctly use `service_start_probe_wait("LLTD"/"rstats", ...)`. THIS COMMIT updates both duplicated LLTD/rstats ordering contracts to follow the current helper call, preserving the ordering check before bounded RUNNING probes.
- This commit modifies test contracts and handoff records only; no production source behavior changes.

## Verification boundary
- Process exact-head run `44323338947` on `f87b0b67cc918c1011714ab09f04497e7566e9a4`: Process inventory PASS; SmartDNS process shell regression and SmartDNS lifecycle source contract PASS; optional autostart suite progressed after launcher-side invalid-port fixture correction; later integration checker failed at LLTD's obsolete direct-sleep lookup (stdin line 1273). Exact-head rerun required for THIS COMMIT.
- WAN DHCP callback `44323140500` on `106af73d32846dc78a552487011dc1141599c8ea`: failed at the second obsolete raw-wait guard after the first replacement had passed. THIS COMMIT corrects the remaining guard; exact-head rerun required.
- WAN DHCP source SHA `e0c8269` had WAN route/teardown/gateway, firewall, DNSMasq, detect_internet/link, VPN, Wireless, HTTPD security, MiniUPnP, USB modem, ZeroTier and Process inventory contracts PASS. NFSd, USB storage, HTTPD security, vsftpd config, NVRAM audit, Process inventory, USB modem and Samba suites passed on `90cce742`. XY-C1 for `90cce742` was still running on the last poll; no L3 claim for that SHA.
- SmartDNS exact product-source SHA `9bb9fdaeff826c957b599cc45e695eb3fb8cd32c` passed XY-C1 Build, TRX verification and artifact upload (L3 verified for that SHA only). BusyBox/uClibc live runtime, current WAN source build and physical-router L4 remain NOT VERIFIED.
- No local checkout-based tests were run in this connector-only session. Only observed GitHub Actions results are claimed.

## Exact first action for the next GPT
1. Fetch live main and exact-head Actions. Inspect the Process fail-closed, WAN DHCP callback and AI handoff runs created by THIS COMMIT; resolve the first causal failure in each.
2. If shared-wait assertions pass, continue the P1 WAN/firewall and cross-userland process lifecycle backlog in `REFACTOR_ROADMAP.md`, not another repeated SmartDNS audit.
3. Reconcile XY-C1 Build/TRX/artifact results on the latest firmware-source SHA; claim L3 only for a fully passing exact SHA. Keep physical-router L4 separate.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect the live repository state and exact-head Actions before ending. If active work and verification boundaries remain unchanged, leave this handoff untouched.

## Cross-session execution mandate
One logical slice is one atomic main commit. Create a complete tree from the exact live parent and update main with non-forced expected-SHA compare-and-swap. Continue independent work while Actions are pending. Before context exhaustion, leave the next GPT the exact first action, commit SHA, observed tests and CI state. Never claim unobserved local tests, target builds or physical-device validation.
