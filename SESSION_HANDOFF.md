# 2026-11-18 continuation checkpoint — make DDNSTO UNKNOWN-startup regression deterministic

## Current repository state
- Live parent `main` SHA: `b00ff4908ca3d3d45964c983fe974c359f95f5de`. Latest product-source baseline is `989409e1a02a92ea5018f3550d52d75c33b028f8` (BusyBox Telnetd detached-child startup postcondition); `b00ff490` is its Process checker-only follow-up.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`, unchanged and read-only. Develop only on `main`.
- Exact `b00ff4908ca3d3d45964c983fe974c359f95f5de` Process lifecycle regression `44165568972`, AliDDNS `44165568791`, infosvr `44165568767`, AdGuardHome `44165569096`, and AI handoff `44165568764`: SUCCESS. NVRAM `44165568763` and AliyunDrive `44165569031` were IN_PROGRESS at last observation.
- The new Telnetd source slice's exact `989409e` Process checker repair passes on `b00ff4908ca3d3d45964c983fe974c359f95f5de`; parent USB storage `44164998300` and NVRAM `44164998225` are SUCCESS. XY-C1 `44164998281` for the Telnetd source baseline was IN_PROGRESS at last observation.
- Networkmap's detached-child startup implementation passed its exact-head regression on `34c0268`; XY-C1 run `44163173638` for that source SHA completed Build XY-C1, TRX verification and artifact upload successfully. That does not include the later Telnetd source change.

## Active task — DDNSTO startup UNKNOWN regression fixture (test-only)
- Exact-head DDNSTO run `44163173235` on `34c02687139afc6db6484c2eb888cca2f8f01612` failed in `trunk/user/ddnsto/tests/ddnsto_lifecycle.sh` after shell syntax passed: `UNKNOWN startup inventory was incorrectly retried`. The semantic checker was skipped.
- Root cause is a test race: the mocked daemon is launched asynchronously and writes `error` to fixture state after scheduling; the first post-launch `pidof` can observe the old `stopped` state (confirmed ABSENT), which production `ddnsto_wait_started()` must retry. This was not evidence that production retries UNKNOWN.
- This slice makes the fixture deterministic: its pre-launch inventory returns ABSENT, while the first post-launch inventory is forced to UNKNOWN by a per-test call-count gate. A bounded test-only wait observes the mock's state publication. No production source changes.

## Verification boundary
- Exact `34c0268` DDNSTO run `44163173235`: FAILED at the asynchronous fixture assertion; shell syntax passed; Python source contract did not run.
- Exact `34c0268` Networkmap `44163174212`, Process inventory `44163173313`, Process lifecycle `44163176396`, AI handoff `44163173533`, USB storage `44163176401`, HTTPD security `44163176352`: SUCCESS. XY-C1 `44163173638`: Build XY-C1, TRX verification and artifact upload SUCCESS.
- Exact `b00ff4908ca3d3d45964c983fe974c359f95f5de` Process lifecycle `44165568972`, infosvr `44165568767`, AliDDNS `44165568791`, AdGuardHome `44165569096`, AI handoff `44165568764`: SUCCESS; NVRAM `44165568763` and AliyunDrive `44165569031` were pending at the last check.
- XY-C1 `44164998281` for the Telnetd product-source baseline `989409e` remained IN_PROGRESS at the last check. No current-source L3 for `989409e` is claimed until Build/TRX/artifact stages complete. Physical router L4 remains NOT VERIFIED.

## Exact first action for the next GPT
1. Inspect the exact-head push-triggered DDNSTO lifecycle run after this fixture repair. Require shell syntax, full lifecycle regression and `scripts/check_ddnsto_lifecycle.py` all PASS; diagnose the actual first failure if not.
2. Resolve XY-C1 `44164998281` for exact product-source `989409e` and check whether NVRAM `44165568763` and AliyunDrive `44165569031` on `b00ff4908ca3d3d45964c983fe974c359f95f5de` have completed.
3. Continue the next independent P1 task from the roadmap (DHCP leasefail/NAK/release state transitions or firewall/NAT caller-status propagation); do not repeat completed storage, infosvr or networkmap audits. Preserve the frozen backup and commit atomically to main only.
# Cross-session execution mandate

Mandatory handoff checkpoint: Every session
- P1 LAN restart failure propagation in `0b2168e1`: all STP/FD, PHY power-cycle, restore and nested WAN restart results now flow through the existing first-error recorder. Follow-up Process workflow correction `89aff313` scopes its nested WAN/VPN ordering assertion correctly; exact-head Process run `44054844821` passed. Source-baseline XY-C1 run `44054425508` was still building at last poll; parent NVRAM `44054425712` passed, while descendant NVRAM `44054844849` was in progress at the last poll.
- AP hostname/hosts failure handling was traced: `start_lan()` logs the early `create_hosts_lan()` attempt as best effort, but `prepare_dns_dhcpd()` calls `update_hosts_ap()` again and refuses DNSMasq configuration publication/start on failure; the AP DHCPD restart notification likewise blocks restart if host publication fails. This recovery gate is present; no additional source defect was confirmed in that path.
- THIS COMMIT hardens AP-mode DHCP event-handler setup: verify `SCRIPT_UDHCPC_LAN` is either newly created or an existing symlink whose exact target is `/sbin/rc`; refuse to launch `udhcpc` on link creation errors or conflicting existing paths. The unit regression and Process workflow production wiring check are included; current exact-head Actions pending.
 MUST inspect the live repository state before ending or handing off. Confirm the main/source baseline, exact-head Actions, validation boundary, blockers, frozen backup and precise next action. The checkpoint is mandatory; a document edit is conditional. Before context exhaustion, complete this accuracy checkpoint against the live main SHA and exact-head Actions; update this file only when a fact or next action materially changed. If all checkpoint facts and the first action are already accurate, do not create a no-op commit. This is the required end-of-session accuracy checkpoint.

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
