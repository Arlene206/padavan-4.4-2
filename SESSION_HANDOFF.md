# Cross-session execution mandate

Every session MUST inspect live repository state and compare it with this handoff before ending. Verify source baseline, active work, exact-head Actions, validation boundary, blockers and precise next action. If nothing material changes, avoid documentation-only commits.

For each logical task, create every changed file in one Git tree/commit parented to freshly fetched main and update main by non-forced expected-SHA compare-and-swap. Never force-push, create partial commits, or touch the frozen backup.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; active development branch: main.
- Parent for the current storage checker correction: 52a5cc63419686af16b23c9aee973d262f3e4529.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14: 72f4f157a084f8184b9597ca5bf334b8080e5807, read-only.
- Root dev log has 43 dated entries before this correction.
- PPP IPCP address/netmask validation, WAN IPv6 teardown identity preservation and DHCPv6 generation fencing remain required. Optional WAN MTU ioctl errors remain diagnostic-only.

## Active task

The USB storage workflow was restored in full and its embedded end-to-end caller graph now passes. Its final safety checker still encoded the old void-return printer helper contract: it expected a direct comparison and bare return, but the current int-returning restart helpers intentionally capture stop_status, return that exact failure and only launch after successful stop. Updated scripts/check_storage_safe_remove.py to assert the actual status-return contract and ordering, rather than weakening the production behavior.

The next concrete P1 defect is USB modem hotplug result loss: try_start_usb_modem_to_wan() is void and discards try_wan_reconnect()'s status; on_deferred_hotplug_dev() clears usb_unplug_md / usb_hotplug_md before the reconnect attempt and does not restore them or schedule a retry when WAN restart fails. Convert the helper to int and re-arm the correct pending flag with the existing five-second alarm on failure.

## Verification boundary

- Current parent 52a5cc63419686af16b23c9aee973d262f3e4529 storage run 43882073147: step 23 whole caller graph, lock ordering and teardown contracts PASS; step 24 scripts/check_storage_safe_remove.py FAIL at line 294 because it still expected the old bare-return API. This correction changes only that stale checker assertion.
- Same product-source SHA cd5ccbefc26a9f71b0c971800b2f5b0d9cad450d XY-C1 run 43880814725: PASS, including Build XY-C1, TRX output verification and artifact upload. Commit 52a5 changed workflow/docs only, not firmware product sources. Firmware L3 is VERIFIED for that product-source tree; physical-router L4 remains NOT VERIFIED.
- On parent 52a5, AI docs 43882073167, infosvr 43882073278, AdGuardHome 43882073332, AliyunDrive 43882073264, NVRAM 43882073244, process inventory/lifecycle 43882073208 and AliDDNS 43882073128: PASS.
- The new storage checker commit must be checked on its exact SHA. The next modem-source slice should trigger both USB modem identity and XY-C1 firmware validation.

## Exact first action for the next GPT

1. Check the exact-SHA storage-hotplug run; its final checker should now accept the int-returning printer restart semantics.
2. Implement USB modem WAN reconnect status propagation and retry-flag re-arming in services_usb.c/services_stor.c, add source contracts and update USB modem + XY-C1 path filters if needed.
3. Require same-SHA XY-C1 Build, TRX and artifact upload to pass for the modem changes; continue independent RC start-result auditing.
4. Keep hardware L4 explicitly NOT VERIFIED.
