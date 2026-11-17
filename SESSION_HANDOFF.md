# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Confirm the main/source baseline, exact-head Actions, validation boundary, blockers, frozen backup and precise next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Continue one engineering effort across GPT conversations: avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next. Finish safe slices, add regression coverage, inspect exact-SHA Actions, commit atomically and preserve the next task.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, create a development branch/PR, or modify the frozen backup.

## Current repository state

Before context exhaustion, inspect the current source SHA, relevant exact-head Actions, validation boundary and first actionable task; then leave this handoff accurate for the next GPT.

- Repository: `Arlene206/padavan-4.4-2`; current source head: `c221420d22838ec1632c975095c3bafa3830d6c9` (parent `a7e3eda16eca5d20701a0240287ebecffed443d7`).
- PPP/L2TP argv-based launch and xl2tpd result normalization are complete; VPN server restore accepts status 1 (already running) and fails only for negative status.
- IPv6 native rollback and WAN teardown flush neighbour entries between route and address cleanup; regression coverage is present on source SHA `f943ccbd8efe3e125afcec8165bf9e235b390e06`.
- IPv6 neighbour-cleanup source SHA `f943ccbd8efe3e125afcec8165bf9e235b390e06` completed XY-C1 Build, TRX verification and artifact upload successfully in run `43994632505`; this establishes product-source MIPS/uClibc L3 for f943, not for later production source changes.
- Exact current-head `c221420d` push runs: infosvr `43996118183` and AdGuardHome `43996118336` PASS. AI handoff `43996118214` failed because the handoff omitted the exact `Before context exhaustion` marker; this commit repairs the marker. HTTPD `43996118234`, AliDDNS `43996118325`, AliyunDrive `43996118148`, NVRAM `43996118191`, standalone Process inventory `43996118524`, and Process lifecycle `43996118185` were in progress at last inspection.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## Active task

1. Inspect the exact-head NVRAM workflow `43996118191` and the related current-head actions. Diagnose the first causal failure from its job log; continue independent work while runs are pending.
2. Verify the new pure ioctl/close result policy regression and the SET/COMMIT/CLEAR source contracts on SHA `c221420d`.
3. Verify same-SHA XY-C1 Build, TRX and artifact upload for the NVRAM production source. The earlier f943 build does not cover this source change.
4. Continue the PPP/L2TP lifecycle review. A concrete candidate is restoration of shared xl2tpd if VPN teardown partially succeeds then aborts during later cleanup; verify the exact path and tests before changing it.

## Verification boundary

- Predecessor `f87343b7e7a2da599c16baf1192ec1561dc44a9b` has product-source MIPS/uClibc L3 VERIFIED.
- The xl2tpd result normalization and VPN restore-sentinel correction are present on the source history; current process integration exact-head run on a7e3 passed (`43995311251`). The f943 IPv6 neighbour-cleanup source has same-SHA Build/TRX/artifact L3 VERIFIED; current NVRAM ioctl/close production change on c221420d still requires same-SHA L3.
- Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Read the failed/in-progress exact-head c221420d workflow logs, fix the first causal defect and verify the new NVRAM close-result regression.
2. Confirm same-SHA XY-C1 Build/TRX/artifact for c221420d; do not infer it from predecessor f943.
3. Continue the bounded PPP/L2TP partial-teardown restoration review and add a policy regression if the failure path is confirmed.
4. Keep the backup frozen and use one complete Git tree/commit with a non-forced expected-SHA compare-and-swap.
