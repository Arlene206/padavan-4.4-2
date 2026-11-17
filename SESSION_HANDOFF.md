# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Confirm the main/source baseline, exact-head Actions, validation boundary, blockers, frozen backup and precise next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Continue one engineering effort across GPT conversations: avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next. Finish safe slices, add regression coverage, inspect exact-SHA Actions, commit atomically and preserve the next task.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, create a development branch/PR, or modify the frozen backup.

## Current repository state

The latest storage hotplug source change rolls back previously stopped storage daemons when a later daemon fails its pre-eject stop check; the original stop error remains authoritative. Storage exact-head run `43998879671` failed only in the embedded workflow source contract, which still expected the old combined `stop_stor_apps()` condition; standalone storage source validation and GNU89 policy compilation/regression completed earlier in the job. The embedded contract is corrected in the current follow-up. XY-C1 run `43998879438` is still building on source `10380657`.

Before context exhaustion, inspect the current source SHA, relevant exact-head Actions, validation boundary and first actionable task; then leave this handoff accurate for the next GPT.

- Repository: `Arlene206/padavan-4.4-2`; latest main source slice is `10380657db522193a781d7e3068c0c2f02a8b78f`, adding rollback for partially stopped storage applications when the safe-eject stop preflight fails.
- PPP/L2TP argv-based launch and xl2tpd result normalization are complete; VPN server restore accepts status 1 (already running) and fails only for negative status.
- IPv6 native rollback and WAN teardown flush neighbour entries between route and address cleanup; regression coverage is present on source SHA `f943ccbd8efe3e125afcec8165bf9e235b390e06`.
- IPv6 neighbour-cleanup source SHA `f943ccbd8efe3e125afcec8165bf9e235b390e06` completed XY-C1 Build, TRX verification and artifact upload successfully in run `43994632505`; this establishes product-source MIPS/uClibc L3 for f943, not for later production source changes.
- The c221420d NVRAM audit `43996118191`, HTTPD `43996118234`, AliDDNS `43996118325`, AliyunDrive `43996118148`, standalone Process inventory `43996118524`, and Process lifecycle `43996118185` completed PASS; infosvr `43996118183` and AdGuardHome `43996118336` also PASS. AI handoff `43996118214` failed only on the missing `Before context exhaustion` marker; exact-head follow-up run `43996365607` passed.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## Active task

1. Verify the storage-hotplug and Samba workflow reruns after fixing the two stale source assertions; fix any remaining causal failure.
2. Confirm XY-C1 Build/TRX/artifact for production source `10380657` (run `43998879438`) and ensure no newer production-source changes bypass that evidence.
3. Continue the P1 RC boot/startup status-propagation audit: identify a concrete enabled service or module startup whose failure is discarded, add a focused policy/integration regression, and preserve first-error/rollback semantics.
4. Do not repeat the completed IPv6 neighbour, NVRAM ioctl-close, or VPN shared-xl2tpd partial-teardown audits.

## Verification boundary

- Predecessor `f87343b7e7a2da599c16baf1192ec1561dc44a9b` has product-source MIPS/uClibc L3 VERIFIED.
- The xl2tpd result normalization and VPN restore-sentinel correction have passed exact-head Process lifecycle checks. The f943 IPv6 neighbour-cleanup source has same-SHA Build/TRX/artifact L3 VERIFIED (XY-C1 run `43994632505`). The c221420d NVRAM ioctl/close production change passed its exact-head NVRAM audit, but subsequent PPP restore changes mean current-source MIPS/uClibc L3 still requires a new same-SHA Build/TRX/artifact run.
- Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Read the failed/in-progress exact-head c221420d workflow logs, fix the first causal defect and verify the new NVRAM close-result regression.
2. Confirm same-SHA XY-C1 Build/TRX/artifact for c221420d; do not infer it from predecessor f943.
3. Continue the bounded PPP/L2TP partial-teardown restoration review and add a policy regression if the failure path is confirmed.
4. Keep the backup frozen and use one complete Git tree/commit with a non-forced expected-SHA compare-and-swap.
