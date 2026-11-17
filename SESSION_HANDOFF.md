# 2026-11-18 continuation checkpoint — verify MiniUPnP lifecycle lock source contracts

## Current repository state
- Repository `Arlene206/padavan-4.4-2`; parent main before THIS contract correction: `03a993e558b23f5f163ca1a904cbf7b54f6ef7e9`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- SSH PID-1 stop hardening is established in `f9afd25e`; exact Process/PID1 tests and XY-C1 Build/TRX/artifact were later recorded successful in the 03a handoff.
- The 03a MiniUPnP production change adds a stable-inode private fcntl lock around public start/stop/WAN interface/update/restart paths. Exact 03a AI docs `44192259762` PASS. Samba `44192259623`, Inadyn `44192259711`, DNSMasq `44192259636`, infosvr `44192259808`, AdGuardHome `44192259712`, AliDDNS `44192259810`, WAN route `44192259691`, and HTTPD security `44192259847` PASS. NVRAM `44192259875` and AliyunDrive `44192259658` were pending.
- Exact 03a Process `44192259665` and MiniUPnP workflow `44192259644` failed only at stale source contracts after the new lock-wrapper split. XY-C1 `44192259641` stopped pre-build at the same stale WAN checker assertion.

## Active task
- THIS correction retargets old checks from public wrappers to internal `*_locked` operations, while independently asserting lock acquisition → operation → lock release for public wrappers.
- The WAN/Process source checker now expects `stop_upnp_locked` as the restart callback and asserts that the callback executes while the lifecycle lock is held.
- The MiniUPnP dedicated workflow now watches the lock header, lock regression test and source checker, and invokes the lock source contract itself.
- No product runtime source changes in THIS correction.

## Verification boundary
- Exact-head Process, MiniUPnP, AI and XY-C1 checks for THIS contract correction are pending. XY-C1 must pass Build XY-C1, TRX output verification and artifact upload before L3 is claimed for the 03a production source.
- The 03a XY-C1 Build/TRX/artifact steps were skipped because the pre-build source checker failed; no L3 is claimed for 03a.
- Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT
1. Inspect exact-head Process `check_modem_wan_start_status.py`, MiniUPnP config/lifecycle workflow and AI handoff runs for THIS correction.
2. Inspect exact-head XY-C1. Once the stale contract passes, require Build, TRX verification and artifact upload. If there is a new failure, inspect its first causal log and fix only that defect.
3. Continue a separate P1 reliability/security slice while any long build remains pending. Keep the backup branch read-only.

# Cross-session execution mandate


Continue one engineering effort across GPT conversations. Avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next. Finish safe source slices, add regression coverage, inspect exact-SHA Actions, commit atomically, and preserve the next task.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, create a development branch/PR, or modify the frozen backup.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development only on `main`.
- Current main at the start of this slice: `55374c12e124711c0b789ffd8dc3f0767c3343cc`; this commit adds an AliDDNS CA-readiness gate to boot autostart.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- P1 WAN-up DDNS fix `01bc69f3`: configured Inadyn inventory/start/config-publication/HUP failures flow into WAN-up's first-error status; DDNS-disabled and AP-mode remain successful no-ops.
- Integration repair `0c786821` fixed WAN test include paths and updated MiniUPnD/HTTPD PPP rollback contracts to the checked status-returning implementation. Its WAN route, MiniUPnD, HTTPD, infosvr, Process lifecycle, NVRAM, AliyunDrive, AdGuardHome and AliDDNS tests passed. The AI handoff contract passed on descendant `06ed4321`.
- XY-C1 for source baseline `0c786821` completed exact-head Build XY-C1, TRX output verification and artifact upload successfully in run `44026236613`; do not treat this artifact as verification of later source changes.
- P1 ZeroTier slice `d2a41360`: WAN-up now uses the existing enabled/disabled-aware `start_zerotier()` wrapper rather than dropping the result of `doSystem()`. It merges/logs the launch failure. The shell regression injects a launcher failure; ZeroTier lifecycle `44027996550` and Process lifecycle `44027996499` passed. Related DNSMasq `44027996482`, VPN server `44027996498`, WAN DHCP `44027996570`, WAN ARP `44027996951`, MiniUPnD `44027996455`, HTTPD `44027996404`, infosvr `44027996430`, AI handoff `44027996394`, teardown `44027996402`, firewall `44027996280`, detect_link `44027996330`, detect_internet `44027996257`, AdGuardHome `44027996230`, AliDDNS `44027996464`, AliyunDrive `44027996377`, and process inventory `44027996381` passed on exact SHA `d2a41360`.
- The WAN-route run `44027994137` on `d2a41360` failed before creating a job because pull_request path patterns were placed outside `on.pull_request.paths`; commit `42097d6c` repaired the filter, and exact-head WAN run `44028469558` passed.
- Exact-head `42097d6c` push Actions all completed SUCCESS: WAN route `44028469558`, AI handoff docs `44028469593`, Process lifecycle `44028469544`, NVRAM `44028469453`, AdGuardHome `44028469448`, AliDDNS `44028469580`, AliyunDrive `44028469536`, infosvr `44028469559`. The WAN workflow now creates and runs its job; no zero-job YAML failure remains.
- THIS COMMIT also fixes the symmetric WAN-down defect: `wan_down_internal()` now captures `eval("/usr/bin/detect.sh")`, accumulates/logs its status like WAN-up, and returns the accumulated result through `wan_down()`. `scripts/check_pppoe_mwan_status.py` asserts both up/down gates, call ordering, diagnostics and wrapper returns; WAN CI runs it. Current commit's exact-head Actions and XY-C1 workflows are pending.
- P1 PPPoE-MWAN detector slice `4f32e40f`: `detect.sh` preserves the first command failure, safely parses `iptables-save` without `eval`, validates NVRAM/IPv4 inputs, bounds loop counts and owns atomic lock cleanup. Its local Bash syntax/mock regression passed; exact-head WAN run `44037585283` also passed, including the production test step. Same-SHA XY-C1 run `44037585221` was still building; TRX and artifact were pending at last poll.
- XY-C1 run `44027996318` for source SHA `d2a41360` completed SUCCESS through Build XY-C1, TRX output verification and artifact upload (artifact `14304605616`, 12.64 MB; SHA-256 `05e374b72ce3f88eb6a019f50c2b1a85c924e28faaae772103bdd102a1485e8f`). This does not verify the newer PPPoE-MWAN source in THIS COMMIT.
- No full local repository checkout or MIPS toolchain is available. The PPPoE-MWAN host Bash syntax and mock-command regression were run locally and passed. Physical-router L4 remains NOT VERIFIED.
- Parent main `5525430a2ee5ff16f3d5b84912cd183e195808df` had 22/22 exact-head `push` Actions successful before its descendant source slices. WAN route run `44039076856` passed; XY-C1 run `44039076948` completed Build XY-C1, TRX output verification and artifact upload successfully.
- Gateway ARP checker launch failure propagation landed in parent `ac9b8936`: after a clean ABSENT process scan, `eval("detect_wan")` is captured, aggregated and logged. A later poll found 21/22 exact-head push runs SUCCESS with no failures; XY-C1 `44041394404` was still building.
- Parent `9722f273` implemented UNKNOWN gateway-monitor inventory error propagation; the WAN checker ordering issue was repaired by `d20696fc` (8/8 selected exact-head push Actions SUCCESS). Production WAN physical MTU failure handling landed in `bb427148`; WAN route and Process lifecycle passed, and XY-C1 `44043949016` completed Build/TRX/artifact SUCCESS. Test integration `cffd4a91` corrected a stale inline WAN DHCP source assertion and all 8 triggered exact-head Actions passed. MAN static custom-MTU status commit `5b9cc6d1` passed exact-head WAN route `44046257356`, WAN DHCP `44046257418`, Process inventory `44046257192`, and Process lifecycle `44046257377` including the GNU89 service-start regression; XY-C1 `44046257252` remained in progress. DHCP bound/renew MTU application then landed in `02ec8ab3`; its C interface/route regression compiled and passed, but WAN route `44047414939` exposed a renewal-checker ordering assertion and WAN DHCP `44047414751` exposed two stale diagnostic-text assertions. THIS COMMIT corrects those test contracts only.

## Active task

- Parent main before this slice: 23e4563fc9a610c420d5aca60d485372b6718da3. Dynamic MAN/WAN firewall status propagation and the LAN bound post-commit callback policy are already on main.
- Exact-head ee3cb source run: WAN DHCP callback 44101239234 PASS, including the GNU89 post-commit callback status regression and source contract. HTTPD 44101239302 first failed because its inline DHCP checker still required return hosts_status after the callback policy deliberately returns lan_dhcp_bound_callback_status(0, hosts_status). Checker-only correction 23e4563fc9a610c420d5aca60d485372b6718da3 updated that expectation; exact-head HTTPD 44101749923 PASS. AdGuardHome 44101749676, AliDDNS 44101749855, AliyunDrive 44101750160 and AI docs 44101749919 on 23e4563 PASS. NVRAM 44101749700 and Process inventory 44101749921 were IN_PROGRESS at last poll. XY-C1 44101239264 and 44101750130 were still building.
- THIS COMMIT fixes a separate LAN DHCP deconfig defect: DHCP bound publishes its leased address into /etc/hostname and /etc/hosts, but deconfig restored the persistent static LAN address without restoring those files. It now restores static hostname/hosts files after the old route is absent and the static address is installed, even if resolver cleanup needs a retry. A host-file publication failure is propagated so BusyBox's DECONFIG_PENDING loop retries.
- On full deconfig success only, static LAN NVRAM state is updated and DHCP-only DNS/WINS/lease metadata is cleared. The readiness predicate covers route-delete success/ESRCH acceptance and route/interface failure. WAN DHCP CI compiles the GNU89 policy regression and checks source ordering; XY-C1 watches the changed production source.
- The stale process-inventory note was corrected: the PID-only kill_process_pidfile() helper was retired in ba90b6ae21. Remaining tri-state, shutdown and service-status work remains open.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains immutable at 72f4f157a084f8184b9597ca5bf334b8080e5807.

## Verification boundary

- Exact ee3cb source-head WAN DHCP callback run 44101239234 PASS. Exact ee3cb HTTPD 44101239302 failed at a stale bound-callback return assertion; exact HTTPD rerun 44101749923 on 23e4563 PASS.
- On exact 23e4563, AdGuardHome 44101749676, AliDDNS 44101749855, AliyunDrive 44101750160, AI handoff 44101749919 and HTTPD 44101749923 PASS. NVRAM 44101749700 and Process inventory 44101749921 were pending at last poll. XY-C1 44101239264 and 44101750130 were still in progress; Build/TRX/artifact completion not verified.
- This commit's deconfig regression and exact source SHA XY-C1 Build/TRX/artifact are pending. Local host compile NOT RUN. Physical-router L4 NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect the exact-head WAN DHCP callback run for this deconfig change; require the GNU89 policy binary and source contract to pass.
2. Inspect the exact-head XY-C1 run for THIS COMMIT; require Build XY-C1, TRX verification and artifact upload.
3. Then review LAN leasefail/NAK behavior against BusyBox initial-acquisition and DECONFIG_PENDING paths; change only a demonstrated state-transition bug with focused regression.
4. Continue P1 process-lifecycle work after refreshing live roadmap. Keep the backup read-only and use atomic non-forced expected-SHA main updates.

Local host execution NOT RUN; physical-router L4 NOT VERIFIED. Frozen backup remains read-only.

## Current main and exact-SHA evidence
- Repository `Arlene206/padavan-4.4-2`; parent main before THIS correction: `f9afd25e8957de96c5661f9c0d00c29cf5d22da3`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify it.
- Exact 6a5 source/test SHA fully passed: Process `44188951409`, Firewall Startup `44188951449`, USB modem `44188951386`, AI docs `44188951370`, and XY-C1 `44188951477` including Build, TRX verification and artifact upload.
- Exact f9 Process lifecycle `44190334132` and Process inventory regression `44190334289` PASS; SSH policy source/compile passed. AliyunDrive rerun `44190333972` later completed SUCCESS, so the earlier `07b8266e` special-domain failure has cleared on this descendant.
- Exact f9 PID1 notification workflow `44190333392` failed only at its embedded source contract: it expected direct `return eval()` in `start_sshd()`, but startup now captures launcher status and verifies the detached child is RUNNING. THIS correction updates that contract to require captured status, bounded startup postcondition, and the new identity-bound `stop_sshd()`.
- Exact f9 AI docs `44190333813` failed because `dev_log.md` was truncated from 31 entries to one. THIS correction restores the complete 31-entry rolling log from `07b8266e` and prepends an accurate checkpoint.
- Exact f9 XY-C1 `44190333512` was still at Build XY-C1 at last poll; TRX/artifact pending. AdGuardHome `44190334184` and NVRAM `44190334351` were also in progress.

## THIS checker/history correction
- PID1's embedded source contract now expects both SSH launch modes to assign `start_status`, check launcher failure, and verify the detached child via bounded RUNNING postcondition. It also asserts PID 1 stop uses `kill_services(service_names, 3, 1)` and no shell name-wide stop.
- Restore 31 existing complete `dev_log.md` entries; the AI docs validator requires at least 20 complete records.
- No production runtime behavior changes in this correction.

## Validation boundary
- Exact-head PID1 and AI handoff checks for THIS correction must pass. XY-C1 must complete Build, TRX verification and artifact upload for f9 production source; the older 6a5 artifact is not verification of f9.
- f9 Process/SSH and AliyunDrive checks have passed. Do not rerun them unless touched.
- Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT
1. Inspect live `main` and exact-`head_sha`, `push` PID1 notification, AI handoff, XY-C1, AdGuardHome and NVRAM runs for THIS correction. Require PID1 SSH policy/source checks and AI docs >=20-entry/archive checks.
2. Require XY-C1 Build, TRX verification and artifact upload for f9 production source.
3. Continue the next independent P1 stability/security defect after these gates. Keep backup immutable; do not repeat passing f9 Process/AliyunDrive tests.




Mandatory handoff checkpoint: Every session
- P1 LAN restart failure propagation in `0b2168e1`: all STP/FD, PHY power-cycle, restore and nested WAN restart results now flow through the existing first-error recorder. Follow-up Process workflow correction `89aff313` scopes its nested WAN/VPN ordering assertion correctly; exact-head Process run `44054844821` passed. Source-baseline XY-C1 run `44054425508` was still building at last poll; parent NVRAM `44054425712` passed, while descendant NVRAM `44054844849` was in progress at the last poll.
- AP hostname/hosts failure handling was traced: `start_lan()` logs the early `create_hosts_lan()` attempt as best effort, but `prepare_dns_dhcpd()` calls `update_hosts_ap()` again and refuses DNSMasq configuration publication/start on failure; the AP DHCPD restart notification likewise blocks restart if host publication fails. This recovery gate is present; no additional source defect was confirmed in that path.
- THIS COMMIT hardens AP-mode DHCP event-handler setup: verify `SCRIPT_UDHCPC_LAN` is either newly created or an existing symlink whose exact target is `/sbin/rc`; refuse to launch `udhcpc` on link creation errors or conflicting existing paths. The unit regression and Process workflow production wiring check are included; current exact-head Actions pending.
 MUST inspect the live repository state before ending or handing off. Confirm the main/source baseline, exact-head Actions, validation boundary, blockers, frozen backup and precise next action. The checkpoint is mandatory; a document edit is conditional. Before context exhaustion, complete this accuracy checkpoint against the live main SHA and exact-head Actions; update this file only when a fact or next action materially changed. If all checkpoint facts and the first action are already accurate, do not create a no-op commit. This is the required end-of-session accuracy checkpoint.

# Cross-session execution mandate

Continue one engineering effort across GPT conversations. Avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next. Finish safe source slices, add regression coverage, inspect exact-SHA Actions, commit atomically, and preserve the next task.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, create a development branch/PR, or modify the frozen backup.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development only on `main`.
- Current main at the start of this slice: `55374c12e124711c0b789ffd8dc3f0767c3343cc`; this commit adds an AliDDNS CA-readiness gate to boot autostart.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- P1 WAN-up DDNS fix `01bc69f3`: configured Inadyn inventory/start/config-publication/HUP failures flow into WAN-up's first-error status; DDNS-disabled and AP-mode remain successful no-ops.
- Integration repair `0c786821` fixed WAN test include paths and updated MiniUPnD/HTTPD PPP rollback contracts to the checked status-returning implementation. Its WAN route, MiniUPnD, HTTPD, infosvr, Process lifecycle, NVRAM, AliyunDrive, AdGuardHome and AliDDNS tests passed. The AI handoff contract passed on descendant `06ed4321`.
- XY-C1 for source baseline `0c786821` completed exact-head Build XY-C1, TRX output verification and artifact upload successfully in run `44026236613`; do not treat this artifact as verification of later source changes.
- P1 ZeroTier slice `d2a41360`: WAN-up now uses the existing enabled/disabled-aware `start_zerotier()` wrapper rather than dropping the result of `doSystem()`. It merges/logs the launch failure. The shell regression injects a launcher failure; ZeroTier lifecycle `44027996550` and Process lifecycle `44027996499` passed. Related DNSMasq `44027996482`, VPN server `44027996498`, WAN DHCP `44027996570`, WAN ARP `44027996951`, MiniUPnD `44027996455`, HTTPD `44027996404`, infosvr `44027996430`, AI handoff `44027996394`, teardown `44027996402`, firewall `44027996280`, detect_link `44027996330`, detect_internet `44027996257`, AdGuardHome `44027996230`, AliDDNS `44027996464`, AliyunDrive `44027996377`, and process inventory `44027996381` passed on exact SHA `d2a41360`.
- The WAN-route run `44027994137` on `d2a41360` failed before creating a job because pull_request path patterns were placed outside `on.pull_request.paths`; commit `42097d6c` repaired the filter, and exact-head WAN run `44028469558` passed.
- Exact-head `42097d6c` push Actions all completed SUCCESS: WAN route `44028469558`, AI handoff docs `44028469593`, Process lifecycle `44028469544`, NVRAM `44028469453`, AdGuardHome `44028469448`, AliDDNS `44028469580`, AliyunDrive `44028469536`, infosvr `44028469559`. The WAN workflow now creates and runs its job; no zero-job YAML failure remains.
- THIS COMMIT also fixes the symmetric WAN-down defect: `wan_down_internal()` now captures `eval("/usr/bin/detect.sh")`, accumulates/logs its status like WAN-up, and returns the accumulated result through `wan_down()`. `scripts/check_pppoe_mwan_status.py` asserts both up/down gates, call ordering, diagnostics and wrapper returns; WAN CI runs it. Current commit's exact-head Actions and XY-C1 workflows are pending.
- P1 PPPoE-MWAN detector slice `4f32e40f`: `detect.sh` preserves the first command failure, safely parses `iptables-save` without `eval`, validates NVRAM/IPv4 inputs, bounds loop counts and owns atomic lock cleanup. Its local Bash syntax/mock regression passed; exact-head WAN run `44037585283` also passed, including the production test step. Same-SHA XY-C1 run `44037585221` was still building; TRX and artifact were pending at last poll.
- XY-C1 run `44027996318` for source SHA `d2a41360` completed SUCCESS through Build XY-C1, TRX output verification and artifact upload (artifact `14304605616`, 12.64 MB; SHA-256 `05e374b72ce3f88eb6a019f50c2b1a85c924e28faaae772103bdd102a1485e8f`). This does not verify the newer PPPoE-MWAN source in THIS COMMIT.
- No full local repository checkout or MIPS toolchain is available. The PPPoE-MWAN host Bash syntax and mock-command regression were run locally and passed. Physical-router L4 remains NOT VERIFIED.
- Parent main `5525430a2ee5ff16f3d5b84912cd183e195808df` had 22/22 exact-head `push` Actions successful before its descendant source slices. WAN route run `44039076856` passed; XY-C1 run `44039076948` completed Build XY-C1, TRX output verification and artifact upload successfully.
- Gateway ARP checker launch failure propagation landed in parent `ac9b8936`: after a clean ABSENT process scan, `eval("detect_wan")` is captured, aggregated and logged. A later poll found 21/22 exact-head push runs SUCCESS with no failures; XY-C1 `44041394404` was still building.
- Parent `9722f273` implemented UNKNOWN gateway-monitor inventory error propagation; the WAN checker ordering issue was repaired by `d20696fc` (8/8 selected exact-head push Actions SUCCESS). Production WAN physical MTU failure handling landed in `bb427148`; WAN route and Process lifecycle passed, and XY-C1 `44043949016` completed Build/TRX/artifact SUCCESS. Test integration `cffd4a91` corrected a stale inline WAN DHCP source assertion and all 8 triggered exact-head Actions passed. MAN static custom-MTU status commit `5b9cc6d1` passed exact-head WAN route `44046257356`, WAN DHCP `44046257418`, Process inventory `44046257192`, and Process lifecycle `44046257377` including the GNU89 service-start regression; XY-C1 `44046257252` remained in progress. DHCP bound/renew MTU application then landed in `02ec8ab3`; its C interface/route regression compiled and passed, but WAN route `44047414939` exposed a renewal-checker ordering assertion and WAN DHCP `44047414751` exposed two stale diagnostic-text assertions. THIS COMMIT corrects those test contracts only.

## Active task

- Parent main before this slice: 23e4563fc9a610c420d5aca60d485372b6718da3. Dynamic MAN/WAN firewall status propagation and the LAN bound post-commit callback policy are already on main.
- Exact-head ee3cb source run: WAN DHCP callback 44101239234 PASS, including the GNU89 post-commit callback status regression and source contract. HTTPD 44101239302 first failed because its inline DHCP checker still required return hosts_status after the callback policy deliberately returns lan_dhcp_bound_callback_status(0, hosts_status). Checker-only correction 23e4563fc9a610c420d5aca60d485372b6718da3 updated that expectation; exact-head HTTPD 44101749923 PASS. AdGuardHome 44101749676, AliDDNS 44101749855, AliyunDrive 44101750160 and AI docs 44101749919 on 23e4563 PASS. NVRAM 44101749700 and Process inventory 44101749921 were IN_PROGRESS at last poll. XY-C1 44101239264 and 44101750130 were still building.
- THIS COMMIT fixes a separate LAN DHCP deconfig defect: DHCP bound publishes its leased address into /etc/hostname and /etc/hosts, but deconfig restored the persistent static LAN address without restoring those files. It now restores static hostname/hosts files after the old route is absent and the static address is installed, even if resolver cleanup needs a retry. A host-file publication failure is propagated so BusyBox's DECONFIG_PENDING loop retries.
- On full deconfig success only, static LAN NVRAM state is updated and DHCP-only DNS/WINS/lease metadata is cleared. The readiness predicate covers route-delete success/ESRCH acceptance and route/interface failure. WAN DHCP CI compiles the GNU89 policy regression and checks source ordering; XY-C1 watches the changed production source.
- The stale process-inventory note was corrected: the PID-only kill_process_pidfile() helper was retired in ba90b6ae21. Remaining tri-state, shutdown and service-status work remains open.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains immutable at 72f4f157a084f8184b9597ca5bf334b8080e5807.

## Verification boundary

- Exact ee3cb source-head WAN DHCP callback run 44101239234 PASS. Exact ee3cb HTTPD 44101239302 failed at a stale bound-callback return assertion; exact HTTPD rerun 44101749923 on 23e4563 PASS.
- On exact 23e4563, AdGuardHome 44101749676, AliDDNS 44101749855, AliyunDrive 44101750160, AI handoff 44101749919 and HTTPD 44101749923 PASS. NVRAM 44101749700 and Process inventory 44101749921 were pending at last poll. XY-C1 44101239264 and 44101750130 were still in progress; Build/TRX/artifact completion not verified.
- This commit's deconfig regression and exact source SHA XY-C1 Build/TRX/artifact are pending. Local host compile NOT RUN. Physical-router L4 NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect the exact-head WAN DHCP callback run for this deconfig change; require the GNU89 policy binary and source contract to pass.
2. Inspect the exact-head XY-C1 run for THIS COMMIT; require Build XY-C1, TRX verification and artifact upload.
3. Then review LAN leasefail/NAK behavior against BusyBox initial-acquisition and DECONFIG_PENDING paths; change only a demonstrated state-transition bug with focused regression.
4. Continue P1 process-lifecycle work after refreshing live roadmap. Keep the backup read-only and use atomic non-forced expected-SHA main updates.

Local host execution NOT RUN; physical-router L4 NOT VERIFIED. Frozen backup remains read-only.
