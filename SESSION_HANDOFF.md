# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Confirm the main/source baseline, exact-head Actions, validation boundary, blockers, frozen backup and precise next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Continue one engineering effort across GPT conversations: avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next. Finish safe slices, add regression coverage, inspect exact-SHA Actions, commit atomically and preserve the next task.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, create a development branch/PR, or modify the frozen backup.

## Current repository state

Before context exhaustion, inspect the current source SHA, relevant exact-head Actions, validation boundary and first actionable task; then leave this handoff accurate for the next GPT.

- Repository: `Arlene206/padavan-4.4-2`; IPv6 boot-status source base `f7b975c6bbe3f891498602c6363017cf6e6cc4c8`; result commit `37e414f2d4f6a80c7addeaca04c14f4fdcdf5f5d`. Checker correction base `37e414f2d4f6a80c7addeaca04c14f4fdcdf5f5d`; result: THIS COMMIT.
- CPU-watchdog UNKNOWN-probe correction is in product source `aeddc157`. Same-source XY-C1 run `44000632827` passed the GNU89 policy regression, Build XY-C1, TRX verification and artifact upload. Later checker/docs-only child `58fe2c5a` does not change that firmware source.
- USB modem module readiness uses tri-state `is_module_loaded_status()`: only exact 1 counts available; UNKNOWN preserves probe errno when no alternative is confirmed; invalid positive states fail closed. Dedicated USB modem run `44002424147`, Process lifecycle `44002424268`, inventory `44002424300`, HTTPD security `44002424357`, infosvr `44002424164`, and AI handoff `44002424146` passed on source `76683274`. Its first XY-C1 run `44002424380` failed only at an obsolete source assertion; checker correction was pushed in `f7b975c6`.
- IPv6 initialization now returns the result of `control_if_ipv6_all()`, and `init_router()` merges it into its first-error `boot_services_status` before later network startup. Exact-head source/policy verification on `37e414f2` is pending.
- Exact-head Process run `44003333415` on `37e414f2` failed only at the new integration assertion because its regular expression was double-escaped. The C source and preceding integration checks passed. This commit changes that assertion to whitespace-normalized semantic checks; no production C files change here.
- XY-C1 run `44003333586` on product-source SHA `37e414f2` was in progress at last check; IPv6 policy test had started, but Build/TRX/artifact must be inspected. This checker-only child retains the same production firmware source.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Legacy PR #1 is closed; its head has no unmerged commits relative to its old base.

## Active task

1. Fetch live main and inspect exact-`head_sha` push Actions for this checker correction. Confirm Process integration reaches PASS after the regex repair.
2. Inspect XY-C1 run `44003333586` on product-source SHA `37e414f2`; confirm IPv6 GNU89 policy/source steps, Build XY-C1, TRX verification and artifact upload. Do not carry L3 from watchdog-only source `aeddc157` or prior USB source builds.
3. Once this slice clears, continue P1 RC startup/service-lifecycle audit for the next concrete false-success/error-loss defect. Do not repeat watchdog, USB modem tri-state, storage eject, VPN restoration, NVRAM close, or IPv6 neighbour cleanup.

## Verification boundary

- Watchdog L3: PASS for product source `aeddc157`, run `44000632827` (Build, TRX and artifact). It does not validate later USB modem or IPv6 source changes.
- USB modem L2: dedicated GNU89 policy/source regression plus Process and related workflows PASS on source `76683274`. The exact-head XY-C1 checker failed on `76683274`; its corrected checker is in `f7b975c6` and needs an exact-source firmware build.
- IPv6 boot-status integration: source change committed in `37e414f2`; Process run `44003333415` exposed a checker-only regex defect, now corrected. XY-C1 `44003333586` was still running; no current-source L3 claim yet.
- Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch current `main` and query push Actions by exact SHA. Inspect Process lifecycle and XY-C1 runs/logs; fix the first causal failure and verify the new SHA. Only claim L3 after same-source Build, TRX verification and artifact upload all succeed. Preserve the frozen backup and use one complete Git tree with a non-forced expected-SHA compare-and-swap.
