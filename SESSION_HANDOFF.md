# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Confirm the main/source baseline, exact-head Actions, validation boundary, blockers, frozen backup and precise next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Continue one engineering effort across GPT conversations: avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next. Finish safe slices, add regression coverage, inspect exact-SHA Actions, commit atomically and preserve the next task.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, create a development branch/PR, or modify the frozen backup.

## Current repository state

Before context exhaustion, inspect the current source SHA, exact-head Actions, validation boundary and next concrete task; update this handoff accurately.

- Repository: `Arlene206/padavan-4.4-2`; development only on `main`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- CPU watchdog tri-state source `aeddc157` passed GNU89 regression and XY-C1 Build/TRX/artifact (`44000632827`); Process/handoff/NVRAM passed on checker/docs-only descendant `58fe2c5a`.
- USB modem tri-state source `76683274` passed dedicated modem readiness plus Process/inventory/HTTPD/infosvr/handoff Actions. XY-C1's first run failed a stale checker; fixed by checker-only `f7b975c6`.
- IPv6 boot status propagation source `37e414f2` merges global sysctl errors into boot first-error status. Process checker fix is `b447358e`; Process `44003684238`, AI handoff `44003684154`, NVRAM `44003684202`, infosvr `44003684216`, AliyunDrive `44003684191`, AliDDNS `44003684173`, AdGuardHome `44003684129` passed on b447. XY-C1 run `44003333586` for `37e414f2` was still in Build at last check.
- Firewall table UNKNOWN-probe fix is product-source commit `736ebe8638d253ae7b870d8a4554b15b7709f30b`. Firewall Startup Stability run `44005295947` PASS (new GNU89/Werror/ASan/UBSan policy test included); Process lifecycle `44005296116`, inventory `44005296027`, FULLCONENAT `44005295950`, and AI handoff `44005296129` PASS on exact 736. XY-C1 run `44005295961` was in Build XY-C1 after every policy/source-check step passed; confirm Build/TRX/artifact.
- HTTPD umbrella run `44005296061` failed only at a stale source assertion requiring legacy Boolean `is_module_loaded("ip6table_mangle")` in IPv6 mangle; current firewall code intentionally uses the tri-state transaction helper. This commit updates that checker; no production C source changes here.
- NVRAM boundary audit `44005296140` was still running at the last poll. Physical-router L4 remains NOT VERIFIED.

## Active task

1. Verify the HTTPD umbrella rerun after updating its obsolete IPv6 mangle Boolean-probe assertion; inspect any next source-contract failure.
2. Inspect exact-source XY-C1 run `44005295961` for production SHA `736ebe8638d253ae7b870d8a4554b15b7709f30b`; confirm Build, TRX and artifact upload.
3. Check NVRAM run `44005296140` and proceed with the next independent P1 RC reliability/security defect. Do not repeat completed watchdog, USB modem, IPv6 sysctl, firewall, storage or VPN work.

## Verification boundary

- Watchdog L2/L3: PASS on product source `aeddc157`.
- USB modem L2: PASS on `76683274`; exact-source XY-C1 checker correction in `f7b975c6`, production firmware build still needs a completed exact-SHA result.
- IPv6 boot status: product source `37e414f2`; Process/handoff/NVRAM/module workflows PASS on `b447358e`; XY-C1 `44003333586` still in progress at last check.
- Firewall tri-state: Firewall Startup Stability PASS on exact source `736ebe86`; Process/inventory/FULLCONENAT/handoff PASS. XY-C1 `44005295961` in progress, so L3 not yet claimed.
- HTTPD umbrella: `44005296061` failed a stale source assertion; checker-only correction in this commit pending rerun.
- Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch live main and inspect the exact-head HTTPD security rerun and XY-C1 Build/TRX/artifact for `736ebe86`. Fix the first true source/checker failure, not already-completed policy work. Keep backup frozen, maintain per-SHA validation boundaries, and publish each multi-file task with one tree/commit and expected-SHA non-forced update.
