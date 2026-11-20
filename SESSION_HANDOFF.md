# 2026-11-20 continuation checkpoint

# Cross-session execution mandate

- Continue main-only development. Every logical source slice is one atomic main commit: create all blobs, one complete Git tree and one commit, then update main using a non-forced compare-and-swap against the exact parent SHA.
- Never amend, force-push, create development branches/PRs, or modify the frozen backup branch.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Implement actual fixes with focused regressions, then inspect Actions for the exact pushed SHA.
- A PENDING Action does not block independent work. Diagnose failures from causal job logs; do not weaken tests to obtain a pass.
- Before context exhaustion, finish a safe slice and leave a factual next task. Do not ask the user what to do next.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

At the end of every session, inspect live main, verify that the frozen backup is unchanged, and check the exact-head Actions relevant to the latest source commit. Update this file only when the active task, source SHA, verification evidence, or first next action has materially changed. If the checkpoint is already accurate, leave it untouched and do not create a no-op commit.

## Current repository state

- Current main before this source slice: `f06fb00373149abe1afc6fcb06c9a0b6f9bd5510`. Recent cross-session updates included checked LAN bridge reload markers and checked HTTPD NVRAM lifecycle publication; exact-head HTTPD and AI-handoff checks passed on `c9b931f0a17bb6673dd94bf7ad51c4c8ba6d7a75`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only; unchanged).
- Exact-head Actions on `c9b931f`: HTTPD daemon lifecycle #44788736506 PASS; AI handoff docs #44788736461 PASS. This slice changes MiniUPnP production source and triggers its sanitizer/source-contract workflow plus XY-C1 firmware validation.
- LAN DHCP leasefail/NAK/release and AP teardown transitions were reviewed against BusyBox event/deconfig behavior; no additional production defect was confirmed.
- Confirmed allocation-exhaustion defect: seven iptables match/target factories dereferenced unchecked `calloc()` results; five rule builders also consumed potentially NULL components and leaked earlier allocations on partial failure.
- Physical-router L4 remains NOT VERIFIED.

## Active task

Make MiniUPnP legacy iptables rule construction fail safely under memory pressure:

- Guard all seven `calloc()`-backed match/target factories before their first dereference; restore allocator errno after diagnostic logging.
- Guard all five rule builders (DNAT, optional MASQUERADE, peer SNAT, peer DSCP and filter ACCEPT). An entry, match, target or final rule-buffer allocation failure must release all prior components, preserve errno and return before iptc append/commit.
- Add GNU89 ASan/UBSan regression coverage for partial-component cleanup and errno preservation, plus a Python source contract for every factory and builder.
- Extend MiniUPnP push/PR path filters and workflow steps so the production and new checks cannot silently skip verification.

After exact-head MiniUPnP and XY-C1 validation, continue reviewing remaining netfilter rule installation/deletion transactions. Do not repeat the completed leasefail/NAK/release or AP restore audit.

## Verification boundary

- Current main before this slice: `f06fb00373149abe1afc6fcb06c9a0b6f9bd5510`; frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Exact-head HTTPD daemon lifecycle #44788736506 and AI handoff docs #44788736461 passed on `c9b931f`.
- New L2 checks: GNU89 `-Wall -Wextra -Werror` ASan/UBSan test covers entry/match/target/buffer cleanup and errno preservation; Python source contract covers seven factories and five builders. Exact-head MiniUPnP Action must pass before validation is claimed.
- XY-C1 Build, TRX verification and artifact upload on the new production-source SHA are required for L3. Physical hardware L4: NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main` and confirm the frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.
2. Check the exact-head MiniUPnP allocation regression and XY-C1 Build/TRX/artifact steps; diagnose any first causal failure without weakening coverage.
3. Continue the MiniUPnP iptables NAT/filter/peer construction and cleanup audit; prioritize confirmed partial-commit and teardown-status defects. Keep physical-router L4 unverified.

## Handoff maintenance

- Keep `dev_log.md` newest-first. Archive only when the 120-record rotation threshold is reached, retaining the newest 20 records.
- Update the roadmap only when module priority/status/evidence changes materially; do not copy it into history on routine updates.
- Host regressions, exact-source firmware L3 and physical hardware L4 are separate verification levels.
