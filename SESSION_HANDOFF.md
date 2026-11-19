# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume the concrete engineering task. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning. Before context exhaustion, complete the safe slice and preserve its exact source/CI boundary.

- One atomic main commit per logical task: create all blobs, one complete tree and one commit with the current main parent, then update main using a non-forced compare-and-swap with that exact SHA.
- Verify push-triggered Actions by exact `head_sha`; a pending run does not block independent work, while a failed relevant run must be triaged before the slice is called validated.
- Preserve security, architecture, compatibility, existing test coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- The frozen backup `backup/pre-plugin-overhaul-2026-11-14` is archival only and must never be modified or deleted. Development stays on main.

## Current repository state

- Main parent when this slice began: `dec2079fcbf9d45731dff1ce03b2765a23bd3a31` (`test(wan): align teardown contracts with status aggregation`).
- Exact-head detect-internet `44603838266`, WAN route `44603838310`, and AI handoff docs `44603838305` all PASS on `dec2079fcbf9d45731dff1ce03b2765a23bd3a31`; the earlier monitor-pause checker failures are closed.
- Product-source SHA `2e61e6b4608d30127692f5d26e939bbce92f52fc` passed XY-C1 build, TRX verification and artifact upload `44600896324`. SkipDBv2 `44600896667` and NVRAM audit `44600896559` also passed on that exact SHA. The intervening `2cdd33f` and `dec2079` commits were checker/documentation changes; this new services_ex.c source slice requires a fresh exact-head XY-C1 run.
- Backup remains `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; frozen. PR #1 remains closed and unmerged.
- No local checkout or MIPS toolchain is available through the current GitHub connector workflow. Physical-router/runtime L4 remains NOT VERIFIED.

## Closed recent task — WAN monitor-pause teardown status contracts

- Production commit `a7eae5a115dae74685e9130ff6d339b8202e5725` captures and accumulates monitor-pause errors during MAN/WAN DHCP teardown while still completing cleanup.
- Checker correction `dec2079fcbf9d45731dff1ce03b2765a23bd3a31` updates detect-internet and WAN route contracts. Exact detect-internet, WAN route and AI docs Actions listed above PASS. Do not repeat this slice.

## Active task — fail closed when DNSMasq cannot prepare its resolver file

- Parent main SHA: `dec2079fcbf9d45731dff1ce03b2765a23bd3a31`.
- In `prepare_dns_dhcpd()`, router mode previously ignored the nonzero return from `create_file(DNS_RESOLV_CONF)` even though the generated configuration points DNSMasq at that resolver file. If the file could not be created, startup could still publish the config and report a running DNSMasq process with no usable resolver input.
- This slice normalizes the create status with the original errno (or EIO fallback), aborts preparation before config publication/daemon launch on failure, and adds a GNU89 ASan/UBSan policy regression plus source-order assertions. AP mode is unchanged.
- The DNSMasq regression and XY-C1 firmware Build/TRX/artifact checks must be inspected by exact head SHA after push. Host policy success is L2 only; XY-C1 Build/TRX/artifact is L3; physical router behavior remains L4 NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect the exact-head DNSMasq lifecycle regression, AI handoff docs and XY-C1 Actions for the current main SHA. Do not call this slice verified until the relevant jobs finish.
2. Fix any failing source-contract/test issue without weakening the resolver-file failure gate or the existing DNSMasq transaction and process-postcondition checks.
3. After this slice is validated, continue the P1 RC service-start result-propagation queue with a separate source defect; do not revisit the closed WAN monitor-pause or Infosvr slices.

## Mandatory handoff checkpoint

Every session must perform an end-of-session accuracy checkpoint before context exhaustion. Compare live main, source baseline, exact-head Actions, module status and this first action. Update this file only when those resumable facts materially change; do not create SHA-refresh or routine snapshot commits.

## Development invariants

- Main-only; never modify the frozen backup or resume PR #1.
- One complete Git tree and one atomic commit per logical task; use `force=false` with the exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits, or weaken regression coverage.
- Report exact SHA Actions and separate L1/L2/L3/L4. Host success is not firmware/hardware success.
