# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Confirm the main/source baseline, exact-head Actions, validation boundary, blockers, frozen backup and precise next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Continue one engineering effort across GPT conversations: avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next. Finish safe slices, add regression coverage, inspect exact-SHA Actions, commit atomically and preserve the next task.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, create a development branch/PR, or modify the frozen backup.

## Current repository state

Before context exhaustion, inspect the current source SHA, exact-head Actions, validation boundary and next concrete task; then leave this handoff accurate for the next GPT.

- Repository: `Arlene206/padavan-4.4-2`; development only on `main`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- CPU-watchdog tri-state source `aeddc157` passed GNU89 regression and XY-C1 Build/TRX/artifact (`44000632827`). USB modem tri-state source `76683274` passed its dedicated readiness and Process/inventory/HTTPD/infosvr/handoff checks. IPv6 boot status propagation source `37e414f2` passed same-source XY-C1 Build/TRX/artifact in run `44003333586`; Process/handoff and dependent checks passed on `b447358e`.
- Firewall module-table UNKNOWN/invalid-state rollback fix is product-source commit `736ebe8638d253ae7b870d8a4554b15b7709f30b`. Firewall Startup Stability `44005295947`, Process lifecycle `44005296027`, inventory `44005296027`, FULLCONENAT `44005295950`, NVRAM audit `44005782297`, and the follow-up exact-head checks listed below passed. HTTPD initially exposed a stale Boolean-probe contract; checker-only fix `49f798836772a802efad324498b875fa6a05b94c` passed HTTPD `44005782392`, Process `44005782244`, AI handoff `44005782352`, infosvr `44005782318`, AliyunDrive `44005782314`, AliDDNS `44005782333`, and AdGuardHome `44005782562`.
- XY-C1 run `44005782493` on `49f798836772a802efad324498b875fa6a05b94c` was still building at the last check; do not claim Build/TRX/artifact until all three steps succeed. Earlier run `44005295961` on product-source `736ebe86` was also still in progress.
- This source slice fixes an RC shutdown omission: `start_services_once()` starts DNS Forwarder under `APP_DNSFORWARDER`, but `stop_services()` did not register its existing checked stop wrapper. The omission could leave DNS Forwarder alive during reboot/halt or firmware update. The fix adds it to the common first-error stop accumulator; this feeds both shutdown storage-persistence and firmware-update abort gates. A source integration regression pins the start/stop guards and both caller gates.
- New P1 RWFS boot-mount fix in THIS COMMIT: `mount_rwfs_partition()` previously discarded UBI module/attach/mkdir/UBIFS mount failures, while Optware start treated directory existence as mount readiness. The new flow verifies tri-state UBI module readiness and the exact MTD attachment before/after attach, validates the mountpoint, propagates its status into PID1 boot results and gates the Optware worker on successful mount; a newly created empty directory is cleaned up on failure. Dedicated GNU89 regression and Process/XY-C1 integration are wired; exact-head Actions pending.
- DNS Forwarder product source remains `8b63b6ea70536e2677204e6da8741a662145ba5f`; docs-only handoff follow-up is `db73892db0692050eca7249e5ac55d4eb35bb3b9`. The current RWFS slice is based on this live main.
- Exact-head Process lifecycle run `44006695435` and DNS Forwarder launch-path regression `44006695180` PASS on `8b63b6ea`. AI handoff run `44006695428` exposed only a missing required handoff phrase; this follow-up restores it. XY-C1 run `44006695535` remains in progress; its Build/TRX/artifact result is not yet claimed.
- Physical-router L4 remains NOT VERIFIED.

## Active task

1. Inspect exact-head Process lifecycle and XY-C1 for this RWFS mountpoint-cleanup checker correction. Confirm policy, integration and boot-module checks pass; resolve any further specific failure.
2. Verify same-production-source XY-C1 Build, TRX validation and artifact upload, then confirm USB storage/hotplug and NVRAM checks. Only then mark RWFS L3 verified.
3. Once RWFS clears, continue P1 RC shutdown/start-result audit across independent services. Do not repeat watchdog, modem, IPv6 sysctl, firewall module-table, NVRAM, storage-eject or VPN work already recorded above.

## Verification boundary

- Watchdog, USB modem and IPv6 boot-status work: verified at their exact source boundaries above; IPv6 includes same-source XY-C1 Build/TRX/artifact.
- Firewall tri-state: policy/source regression PASS on `736ebe86`; HTTPD stale contract repaired and exact run `44005782392` PASS on checker/docs child `49f79883`; Process/inventory/NVRAM/infosvr/AliyunDrive/AliDDNS/AdGuardHome/handoff exact-head checks PASS on `49f79883`. XY-C1 `44005782493` remained in progress; firewall-source L3 is not yet claimed.
- DNS Forwarder RC stop registration: production source and integration committed in `8b63b6ea`; Process `44006695435` and launch-path `44006695180` PASS, XY-C1 `44006695535` was pending at last check. AI handoff docs-only fix `db73892d` restored the required checkpoint phrase.
- RWFS boot mount / Optware readiness: production fix committed in `5f1d87f6`; decoder test correction `fd9ab12e` made the GNU89/ASan/UBSan policy regression PASS. Process inventory run `44008320003` then failed only because the new integration checker assumed same-line callback signatures; XY-C1 `44008319840` failed only because the older boot-module checker required direct assignment of `boot_modules_status`, not the new first-error merge. This commit corrects both source contracts; exact-head reruns pending. Storage hotplug `44007851445` and HTTPD `44007851498` PASS on `5f1d87f6`; no RWFS L3 claim.
- Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch live `main`, obtain this RWFS commit's exact `head_sha` Actions, and inspect Process lifecycle, XY-C1, USB storage/hotplug and HTTPD jobs. Resolve the first causal failure; if any run is pending, continue an independent RC lifecycle defect rather than waiting. Keep the backup frozen and use one complete Git tree/commit with a non-forced expected-SHA compare-and-swap.
