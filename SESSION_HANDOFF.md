# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Confirm the main/source baseline, exact-head Actions, validation boundary, blockers, frozen backup and precise next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Continue one engineering effort across GPT conversations: avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next. Finish safe slices, add regression coverage, inspect exact-SHA Actions, commit atomically and preserve the next task.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, create a development branch/PR, or modify the frozen backup.

## Current repository state

Before context exhaustion, inspect the current source SHA, exact-head Actions, validation boundary and next concrete task; then leave this handoff accurate for the next GPT.

- Repository: `Arlene206/padavan-4.4-2`; development only on `main`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- CPU-watchdog tri-state source `aeddc157` passed GNU89 regression and XY-C1 Build/TRX/artifact (`44000632827`). USB modem tri-state source `76683274` passed its dedicated readiness and Process/inventory/HTTPD/infosvr/handoff checks. IPv6 boot status propagation source `37e414f2` passed same-source XY-C1 Build/TRX/artifact in run `44003333586`; Process/handoff and dependent checks passed on `b447358e`.
- Firewall module-table UNKNOWN/invalid-state rollback fix is source `736ebe86`; Firewall Startup Stability `44005295947` passed. The stale HTTPD Boolean-probe checker was corrected in `49f79883`; HTTPD `44005782392` and firmware-identical XY-C1 `44005782493` passed, including Build/TRX/artifact.

- This source slice fixes an RC shutdown omission: `start_services_once()` starts DNS Forwarder under `APP_DNSFORWARDER`, but `stop_services()` did not register its existing checked stop wrapper. The omission could leave DNS Forwarder alive during reboot/halt or firmware update. The fix adds it to the common first-error stop accumulator; this feeds both shutdown storage-persistence and firmware-update abort gates. A source integration regression pins the start/stop guards and both caller gates.
- P1 RWFS boot-mount/Optware readiness fix product-source commit `5f1d87f66e0247ef192d3f375834a1a7f766a2e0`: `mount_rwfs_partition()` previously discarded UBI module/attach/mkdir/UBIFS mount failures, while Optware start treated directory existence as mount readiness. The new flow verifies tri-state UBI module readiness and the exact MTD attachment before/after attach, validates the mountpoint, propagates its status into PID1 boot results and gates the Optware worker on successful mount; a newly created empty directory is cleaned up on failure. Dedicated GNU89 regression and Process/XY-C1 integration are wired; exact-head Actions pending.
- DNS Forwarder source `8b63b6ea70536e2677204e6da8741a662145ba5f` and handoff follow-up `db73892db0692050eca7249e5ac55d4eb35bb3b9` are verified; same-source XY-C1 run `44006695535` passed Build/TRX/artifact.
- The RWFS product source is `5f1d87f66e0247ef192d3f375834a1a7f766a2e0`; its GNU89 policy + focused source checker PASS. Process umbrella `44009960277` passed on `b95bb735`. The RWFS + Ralink IAPP production-source descendant `f5a42763` passed storage hotplug `44009838525` and same-source XY-C1 `44009838585` completed SUCCESS including Build XY-C1, TRX verification and artifact upload.
- Process lifecycle `44009960277` on main `b95bb735` PASS, including RWFS and the Ralink IAPP shutdown result wiring. AI handoff `44009960341` PASS. NVRAM transaction audit `44009960343` remains in progress; its result has not been claimed.
- Physical-router L4 remains NOT VERIFIED.

- GPIO button daemon lifecycle: `start_gpio_btn()` had a SIGTERM-aware loop but `stop_misc()` omitted `gpio_btn`. THIS COMMIT adds it to the common graceful stop list and extends the GPIO checker to pin the SIGTERM handler and shutdown persistence gate; exact-head GPIO/Process runs pending.

## Active task

1. Inspect exact-head DDNSTO lifecycle and Process Actions for the DDNSTO fixture correction in THIS COMMIT. Base `9c7c393d` DDNSTO run `44011291797` failed because the mock PID was the literal `# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Confirm the main/source baseline, exact-head Actions, validation boundary, blockers, frozen backup and precise next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Continue one engineering effort across GPT conversations: avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next. Finish safe slices, add regression coverage, inspect exact-SHA Actions, commit atomically and preserve the next task.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, create a development branch/PR, or modify the frozen backup.

## Current repository state

Before context exhaustion, inspect the current source SHA, exact-head Actions, validation boundary and next concrete task; then leave this handoff accurate for the next GPT.

- Repository: `Arlene206/padavan-4.4-2`; development only on `main`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- CPU-watchdog tri-state source `aeddc157` passed GNU89 regression and XY-C1 Build/TRX/artifact (`44000632827`). USB modem tri-state source `76683274` passed its dedicated readiness and Process/inventory/HTTPD/infosvr/handoff checks. IPv6 boot status propagation source `37e414f2` passed same-source XY-C1 Build/TRX/artifact in run `44003333586`; Process/handoff and dependent checks passed on `b447358e`.
- Firewall module-table UNKNOWN/invalid-state rollback fix is source `736ebe86`; Firewall Startup Stability `44005295947` passed. The stale HTTPD Boolean-probe checker was corrected in `49f79883`; HTTPD `44005782392` and firmware-identical XY-C1 `44005782493` passed, including Build/TRX/artifact.

- This source slice fixes an RC shutdown omission: `start_services_once()` starts DNS Forwarder under `APP_DNSFORWARDER`, but `stop_services()` did not register its existing checked stop wrapper. The omission could leave DNS Forwarder alive during reboot/halt or firmware update. The fix adds it to the common first-error stop accumulator; this feeds both shutdown storage-persistence and firmware-update abort gates. A source integration regression pins the start/stop guards and both caller gates.
- P1 RWFS boot-mount/Optware readiness fix product-source commit `5f1d87f66e0247ef192d3f375834a1a7f766a2e0`: `mount_rwfs_partition()` previously discarded UBI module/attach/mkdir/UBIFS mount failures, while Optware start treated directory existence as mount readiness. The new flow verifies tri-state UBI module readiness and the exact MTD attachment before/after attach, validates the mountpoint, propagates its status into PID1 boot results and gates the Optware worker on successful mount; a newly created empty directory is cleaned up on failure. Dedicated GNU89 regression and Process/XY-C1 integration are wired; exact-head Actions pending.
- DNS Forwarder source `8b63b6ea70536e2677204e6da8741a662145ba5f` and handoff follow-up `db73892db0692050eca7249e5ac55d4eb35bb3b9` are verified; same-source XY-C1 run `44006695535` passed Build/TRX/artifact.
- The RWFS product source is `5f1d87f66e0247ef192d3f375834a1a7f766a2e0`; its GNU89 policy + focused source checker PASS. Process umbrella `44009960277` passed on `b95bb735`. The RWFS + Ralink IAPP production-source descendant `f5a42763` passed storage hotplug `44009838525` and same-source XY-C1 `44009838585` completed SUCCESS including Build XY-C1, TRX verification and artifact upload.
- Process lifecycle `44009960277` on main `b95bb735` PASS, including RWFS and the Ralink IAPP shutdown result wiring. AI handoff `44009960341` PASS. NVRAM transaction audit `44009960343` remains in progress; its result has not been claimed.
- Physical-router L4 remains NOT VERIFIED.

## Active task

, not a numeric PID; production correctly rejected it as UNKNOWN. Confirm the corrected fixture makes the delayed-start regression pass.
2. Inspect exact-head DDNSTO/Process Actions for `6e997ddb`; if they pass, continue P1 RC start/stop audit. GPIO button daemon shutdown registration is also committed in THIS COMMIT; inspect GPIO lifecycle and Process runs by exact SHA. RWFS L3 is verified on `f5a42763` by same-source XY-C1 `44009838585` (Build/TRX/artifact); DDNSTO's later source edit has no same-SHA XY-C1 result yet.
3. Continue the P1 RC startup/shutdown status audit across independent services; do not repeat already completed watchdog, modem, IPv6, firewall, NVRAM, storage-eject or VPN work.

## Verification boundary

- Watchdog, USB modem and IPv6 boot-status work: verified at their exact source boundaries above; IPv6 includes same-source XY-C1 Build/TRX/artifact.
- Firewall tri-state: policy/source regression PASS on `736ebe86`; HTTPD stale contract repaired and exact run `44005782392` PASS on checker/docs child `49f79883`; Process/inventory/NVRAM/infosvr/AliyunDrive/AliDDNS/AdGuardHome/handoff exact-head checks PASS on `49f79883`. XY-C1 `44005782493` remained in progress; firewall-source L3 is not yet claimed.
- DNS Forwarder RC stop registration: production source and integration committed in `8b63b6ea`; Process `44006695435` and launch-path `44006695180` PASS, XY-C1 `44006695535` was pending at last check. AI handoff docs-only fix `db73892d` restored the required checkpoint phrase.
- RWFS boot mount / Optware readiness: production source `5f1d87f6`; GNU89 policy, focused source checker and Process umbrella `44009960277` PASS. Same-production-source descendant `f5a42763` XY-C1 run `44009838585` PASS including Build/TRX/artifact; USB storage `44009838525` PASS on the same source.
- Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch live `main` and inspect exact-head DDNSTO lifecycle, Process and XY-C1 Actions for this bounded startup-postcondition change. Fix the first causal failure and verify same-source Build/TRX/artifact; if Actions are pending, continue another independent P1 service lifecycle defect. Keep the backup frozen and publish each slice atomically with a non-forced expected-SHA compare-and-swap.
