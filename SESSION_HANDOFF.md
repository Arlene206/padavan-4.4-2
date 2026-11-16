# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect live repository state and compare it with this file before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers and precise first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave this file untouched and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, and do not ask the user to choose the next task.

Every multi-file logical change must be created as one complete Git tree and one atomic main commit parented to freshly fetched main, then update main with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, or modify the frozen backup branch.

## Current repository state

- Source parent before THIS COMMIT: `74ac0ed17f6ffd61679371258451ab3683118116`. It contains the exact filesystem module-catalog membership fix and prior storage mount-owner hardening.
- Frozen backup branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable and read-only.
- The storage module inventory/catalog and mount-owner checks are fail-closed; exact catalog basenames avoid false matches such as `fat` matching `exfat.ko`.
- NDIS initial interface bring-up failure propagation landed in `b54a701c35e35549a183eca6bf48c86d5dffb855`. On parent `74ac0ed17f6ffd61679371258451ab3683118116`, XY-C1 run `43918175205` passed the GNU89 modem regressions, Build XY-C1, TRX verification and artifact upload.
- The HTTPD ownership contract repair was verified by exact-head HTTPD `43918175175` PASS. USB storage `43918175044`, NVRAM `43918174958`, AI docs `43918175240`, Process inventory `43918175194` and Process regression `43918175085` also PASS.

## Active task

THIS COMMIT fixes a dropped NDIS modem control-session result. `launch_wan_usbnet()` previously ignored `ndis_control_network()` failure and continued to launch DHCP. QMI, Huawei NCM, CDC-NCM and Sierra operations now explicitly mark the control transaction required; a nonzero status sets WAN error and returns before DHCP. The MBIM stub remains an explicitly unsupported/no-user-space-control path and generic Ethernet/RNDIS no-control paths remain optional for compatibility. The stop path now distinguishes optional no-op from required failure. Added GNU89 policy tests and source-contract assertions. No unrelated storage changes.

## Verification boundary

- Parent `74ac0ed17f6ffd61679371258451ab3683118116`: XY-C1 `43918175205` PASS through Build XY-C1, TRX verification and artifact upload. NVRAM `43918174958`, USB storage `43918175044`, HTTPD `43918175175`, AI docs `43918175240`, Process inventory `43918175194`, Process regression `43918175085`, infosvr `43918175001`, AliDDNS `43918175163`, AliyunDrive `43918175349` and AdGuardHome `43918175144` PASS.
- THIS COMMIT changes `usb_modem.c`, the modem WAN policy/header tests and source checker; these paths are already included in USB modem identity and XY-C1 workflow triggers. Exact-head validation must be checked after push.
- No local compiler/firmware run is claimed. Modem hardware behavior, target driver binding, BusyBox/MIPS runtime and physical-router L4 remain NOT VERIFIED. Frozen backup remains unchanged.

## Exact first action for the next GPT

1. Fetch live main and inspect this commit's exact-head USB modem WDM/NDIS regression and XY-C1 validation. Require GNU89/Werror/ASan/UBSan and the semantic checker to pass; then require Build XY-C1, TRX verification and artifact upload.
2. If a test/source contract fails, fix that exact SHA with a small atomic main commit; if checks are pending, continue an independent WAN/RC service-start result-propagation issue.
3. A separate later modem task should inspect `qmi_control_network()` teardown: its stop path currently ignores `uqmi --stop-network` and client-ID release results before deleting handle files. Keep that distinct from this startup fix.
4. Keep the frozen backup branch untouched and physical-router L4 marked NOT VERIFIED.
