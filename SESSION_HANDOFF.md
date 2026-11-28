## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch `main`. Mandatory startup recovery was completed once; do not repeat the initial audit.
- Current engineering parent for the Shadowsocks watchdog slice: `1e30efeb62d96cd2fe64d1fa41e03d97d34cde81`. The exact-head AI handoff documentation correction on this SHA passed workflow #46150253122.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains an immutable archive; do not modify. PR #1 remains closed/unmerged and the CA trust-store feature is already integrated.

## Active task

- Replace the remaining `ps | grep` daemon-health counts in `trunk/user/shadowsocks/scripts/ss-monitor` with exact role-specific executable identity. The old `ssr-retcp/reudp` strings are PID-file labels, not executable names; moreover, TCP/UDP/local workers can share one binary, so executable name alone is insufficient. The bounded slice maps each role to its real executable and unique config argv token, counts that identity, and adds shell plus source-contract regressions.
- Preserve tri-state semantics: confirmed absence is a successful zero count and may initiate recovery; incomplete procfs, malformed metadata or a same-comm foreign executable must log and skip that individual recovery path. A different configured role using the same executable must not count as this role.

## Exact-SHA CI checkpoint

- Baseline `1e30efeb62d96cd2fe64d1fa41e03d97d34cde81`: AI handoff documentation workflow #46150253122 PASS on the exact `main` push SHA.
- Production SHA `9fc0a65fdff1a8c315119200efb1976e6e9d181d`: networkmap regression #46149948622 PASS; Process inventory #46149948705 PASS; NVRAM transaction boundary #46149948646 PASS. AI docs #46149948625 failed only on the subsequently repaired handoff wording.
- XY-C1 firmware workflow #46149948587 is PENDING/in progress for exact SHA `9fc0a65fdff1a8c315119200efb1976e6e9d181d`; Build/TRX/artifact must all complete on one production SHA before any L3 claim. It does not block this independent Shadowsocks work.
- The current Shadowsocks source slice is part of the commit being created from the baseline above; its exact-head Actions status must be inspected after the ref update.

## Verification boundary

- The existing process stop path binds signals to captured PID/start-time plus executable identity. This change extends the same fail-closed model to watchdog health and distinguishes roles sharing one executable by checking a complete argv token.
- No local checkout is available in this session; local shell execution and `python3 scripts/check_shadowsocks_lifecycle.py` are NOT RUN. Exact-head GitHub Actions is required for shell regression, `sh -n`, and source contract.
- XY-C1 L3 for the current source baseline is not established here; physical router L4 is NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect the Shadowsocks lifecycle push workflow for the exact SHA created by this slice. Require its shell regression, shell syntax checks and Python source contract to PASS; repair any real failure on a new atomic main commit.
2. Recheck XY-C1 run #46149948587 and report its exact SHA plus Build, TRX and artifact outcomes; do not transfer a green result from another source SHA.
3. Continue the next highest-value userland stability defect outside already completed NVRAM, networkmap packet-field and process-watchdog slices; do not repeat those audits without fresh evidence.
4. Keep the frozen backup untouched and preserve complete-tree, non-forced expected-SHA commits.

## Mandatory end-of-session handoff checkpoint

Mandatory handoff checkpoint. The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect the live repository state and exact-head Actions before ending. Record material PENDING-to-PASS/FAIL transitions. If all checkpoint facts and the first action remain accurate, leave the handoff untouched and do not create a no-op commit.

# Cross-session execution mandate

- one atomic main commit per logical task: create all required blobs, one complete Git tree, one commit, then a non-forced expected-SHA update. Never force-push or publish per-file partial commits.
- Before context exhaustion, inspect the exact-head Actions and leave the next GPT an accurate task, first action, tests and L1-L4 verification boundary.
- Keep working while independent Actions are pending; PASS continues, FAIL requires causal repair, PENDING permits independent work. Do not ask the user what to do next.
