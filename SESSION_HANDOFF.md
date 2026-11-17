# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Confirm the main/source baseline, exact-head Actions, validation boundary, blockers, frozen backup and precise next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Continue one engineering effort across GPT conversations: avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next. Finish safe slices, add regression coverage, inspect exact-SHA Actions, commit atomically and preserve the next task.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, create a development branch/PR, or modify the frozen backup.

## Current repository state

Before context exhaustion, inspect the current source SHA, relevant exact-head Actions, validation boundary and first actionable task; then leave this handoff accurate for the next GPT.

- Repository: `Arlene206/padavan-4.4-2`; development only on `main`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`; legacy PR #1 is closed.
- CPU watchdog tri-state fix: source `aeddc15730e1a23419b1516ed88c330e148ac883` passed policy tests and XY-C1 Build/TRX/artifact (`44000632827`). Process, AI handoff and NVRAM passed on checker/docs-only child `58fe2c5a` (`44001230583`, `44001230567`, `44001230573`).
- USB modem UNKNOWN-probe fix: source `766832746b81a67faaa5d88e3ba78cbd1228faef` passed dedicated readiness, Process, inventory, HTTPD security, infosvr and AI handoff (`44002424147`, `44002424268`, `44002424300`, `44002424357`, `44002424164`, `44002424146`). Its first XY-C1 run failed only an obsolete assertion; checker-only child `f7b975c6` repaired it.
- IPv6 boot aggregation: `init_ipv6()` now returns global sysctl status and `init_router()` merges it. Production source `37e414f2d4f6a80c7addeaca04c14f4fdcdf5f5d` had Process checker run `44003333415` fail only at a double-escaped checker regex; checker-only child `b447358e` fixed it. Process `44003684238`, AI handoff `44003684154`, NVRAM `44003684202`, infosvr `44003684216`, AliyunDrive `44003684191`, AliDDNS `44003684173`, and AdGuardHome `44003684129` passed on b447.
- XY-C1 run `44003333586` for source `37e414f2` was still in Build XY-C1 at the last check; policy tests had run. Confirm its final Build/TRX/artifact state before claiming IPv6 L3.
- Firewall review found that Boolean module probes for `iptable_mangle`, `iptable_raw`, and `ip6table_mangle` treated sysfs UNKNOWN as absent and could skip empty-table restore while stale rules remained active. The current source slice uses strict tri-state decisions; UNKNOWN/invalid marks the core transaction failed so existing logic aborts and rolls back earlier changes. New GNU89/Werror/ASan/UBSan policy tests and source contracts are wired into Firewall Startup Stability.
- Physical-router L4 remains NOT VERIFIED.

## Active task

1. Verify exact-head Firewall Startup Stability regression and source contracts for optional module-table UNKNOWN/invalid-state rollback.
2. Verify same-SHA XY-C1 Build, TRX and artifact upload for this firewall source change; do not carry forward older L3 results.
3. Continue the P1 RC reliability/security review for another concrete false-success or first-error loss in a different subsystem. Do not repeat watchdog, modem, storage, VPN or NVRAM tasks.

## Verification boundary

- Watchdog L2/L3: PASS; product source `aeddc157` passed policy and XY-C1 Build/TRX/artifact.
- USB modem L2: PASS on `76683274`; dedicated policy and related exact-head workflows passed. Its first XY-C1 run failed only the stale checker, fixed in `f7b975c`.
- IPv6 boot aggregation: source committed in `37e414f2`, checker fixed in `b447358e`; Process, handoff, NVRAM and module Actions passed on b447. XY-C1 `44003333586` remained in Build at last check; L3 pending.
- Firewall module-table tri-state: current slice pending exact-head L2/L3 Actions. Earlier CI does not validate this change.
- Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch live main and query push Actions by exact `head_sha`. Inspect Firewall Startup Stability and XY-C1 job logs for this slice; fix the first causal failure and rerun on the new SHA. Do not claim L3 until same-SHA Build, TRX verification and artifact upload pass. Preserve the frozen backup and use one complete Git tree/commit with a non-forced expected-SHA compare-and-swap.
