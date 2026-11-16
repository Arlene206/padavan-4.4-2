# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state and compare it with `SESSION_HANDOFF.md` before ending or handing off. Verify source baseline, active task, exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave the file untouched and make no no-op commit.

Before context exhaustion, finish or reduce the current safe slice, check exact-head Actions, commit completed work atomically and preserve the precise first next action here.

Continue this engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active ref: `main`.
- Parent main for the storage workflow restoration: `cd5ccbefc26a9f71b0c971800b2f5b0d9cad450d`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only. Startup recovery is complete; do not repeat.
- Root dev log has 42 records before this restoration entry; archive part-012 retains older entries.
- PPP ip-up validates canonical local/peer IPCP IPv4 addresses and contiguous netmask before interface setup or lease publication.
- WAN IPv6 teardown preserves the last live runtime `ifname6` through RA/autoconfiguration and route/address cleanup; DHCPv6 callback generation fencing remains required.
- Optional WAN MTU ioctl failures remain diagnostic-only and do not set DHCP, interface or route failure state.

## Active task

The current main commit `cd5ccbefc26a9f71b0c971800b2f5b0d9cad450d` truncated `.github/workflows/storage-hotplug-stability.yml` after the embedded Python line `push_filters, pr_filters = filters.split("`; its storage-hotplug Action failed with an unterminated-string SyntaxError before the remaining storage-caller contracts and persistence gate could run. The complete suffix is being restored from the last intact version at `0409c2b90d8da55531b8370dbb16e7a1ed978d67`, retaining the newer USB printer exec/aggregation contracts and the required push/PR path filters. The repaired file is validated as 1,163 lines with the embedded Python heredoc and final storage gate intact.

## Verification boundary

Exact-head Actions for parent `cd5ccbefc26a9f71b0c971800b2f5b0d9cad450d`:
- USB storage hotplug `43880814702`, job `133345880304`: FAIL at the embedded Python source-contract step. The preceding assertions, including first-error propagation, optional printer-hook aggregation, and deferred retry ordering, passed. The script then hit a literal newline inside `filters.split("` at line 139 because the committed workflow was truncated. This commit restores the complete 1,163-line workflow from intact parent content, while preserving the latest USB contract additions.
- XY-C1 Firmware Validation `43880814725`: IN_PROGRESS at last inspection; no same-SHA Build/TRX/artifact outcome yet.
- AI docs `43880814989`, infosvr `43880814630`, NVRAM `43880814712`, AdGuardHome `43880814663`, AliDDNS `43880814812`, AliyunDrive `43880814843`, process lifecycle `43880814751`, process inventory `43880814602`: PASS on the parent.
- Firmware product-source L3 was previously verified on `f6fba229aba497988d2308c8c99879aca5da1214` only; do not transfer it to newer SHAs without their own Build, TRX and artifact upload. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head storage-hotplug on the restored workflow; repair any first causal source-contract/runtime failure without weakening storage checks.
2. Inspect exact-head XY-C1 Build, TRX verification and artifact upload; claim firmware L3 only if all three pass on the same SHA.
3. Continue P1 broad RC startup-result propagation across independent userland modules, tracing all real callers and preserving process/lock contracts.
4. Preserve the frozen backup branch and leave physical-router L4 NOT VERIFIED until hardware evidence exists.
