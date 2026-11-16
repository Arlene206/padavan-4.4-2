# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect live repository state and complete an accuracy checkpoint before ending. The checkpoint is mandatory; a document edit is conditional.

Continue the recorded engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched `main`; update `main` with exact expected SHA using a non-forced compare-and-swap. Never force-push, rewrite history, create partial commits, or touch the frozen backup. Before context exhaustion, record precise live refs, exact Action results and the next concrete task.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`.
- Parent main for THIS modem module readiness slice: `a445812cf9e168d21f24f51e035cddf6934c12d9`. Refetch live `main` before every later write and use expected-SHA non-forced compare-and-swap.
- Frozen archive: `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable.
- Startup protocol, root governance, archive index, backup marker and PR #1 recovery are complete; do not repeat that audit.
- Root `dev_log.md` contains 93 newest-first records before this entry; this commit brings it to 94. Keep 20–119 and preserve archived shards/indexes.

## Active task

The USB modem P1 path now contains:
- Same-device NDIS TTY selection bound to physical USB port ID + devnum. Manual selection is constrained to that identity; automatic preference/fallback stays on that modem.
- A `reload_modem_modules()` family-readiness contract: attempt every optional driver candidate and verify observable `/sys/module` state. One available family member is sufficient; not every alternative is required.
- NDIS candidates: `cdc_ether`, `rndis_host`, `qmi_wwan`, `cdc_ncm` (`prefer_mbim=0`), `huawei_cdc_ncm`, `cdc_mbim`, `sierra_net`. RAS candidates: `cdc_acm`, `qcserial`, `sierra`, `option`. NDIS serial AT/control modules remain supplemental optional loads.
- The readiness result is merged into initial USB boot health and both explicit modem-reconfiguration notification paths.
- Added GNU89/Werror/ASan/UBSan policy coverage and a semantic production-source contract.

Boundary: module presence under `/sys/module` verifies module-family availability only; it does NOT prove the selected USB interface actually bound to that driver. Device binding remains a separate open issue.

## Verification boundary

Exact-parent Action summary:
`XY-C1 Firmware Validation 43847848258=in_progress/PENDING; NVRAM transaction boundary audit 43847848244=in_progress/PENDING; AI handoff documentation contract 43847848285=completed/success; AliyunDrive WebDAV lifecycle stability regression 43847848020=completed/success; HTTPD Authentication and Security Regression 43847848338=completed/success; AdGuardHome config security regression 43847848288=completed/success; infosvr packet and signal lifecycle stability regression 43847848286=completed/success; AliDDNS signed API transport security regression 43847848417=completed/success; Process inventory and lifecycle fail-closed regression 43847847921=completed/success`
No Action result from this parent is a validation of the new module-readiness commit. Do not claim L3 unless exact-SHA Build XY-C1, TRX verification and artifact upload pass. No L4 physical-router result is established.

Previously observed ancestor checks:
- USB identity `43847475018` on parent `b83383f9798d5a3e43d86877f9f37c0b1594ffd0`: SUCCESS; same-device TTY and hotplug metadata source contracts.
- Process lifecycle inventory `43847475193` and AI handoff docs `43847474937` on the same parent: SUCCESS.
- A different parent run `43847474960` had an HTTPD authentication regression failure in a switch-port ioctl contract; unrelated to this module slice.

## Exact first action for the next GPT

1. Fetch live main; inspect exact-head USB identity, module readiness, Process integration, XY-C1 and AI docs Actions. Read all failure logs. Record L3 only after exact-SHA Build XY-C1, TRX verification and artifact upload pass.
2. Determine whether USB sysfs/hotplug reliably maps the selected NDIS interface to its bound driver. A module present in `/sys/module` does not prove device binding; add a device-level check only with evidence.
3. Continue broader user-space stability/security governance beyond the modem path once this slice is verified.

## Governance constraints

Preserve security, architecture, compatibility, target-toolchain and mandatory tests. Use only `main`; frozen backup is read-only. Update root `dev_log.md` for every code/documentation commit; keep 20–119 records and preserve archive shards/indexes verbatim. Distinguish source, CI, firmware and physical-router validation. No force-push, history rewrite, or unrun-test/build/hardware claim.
