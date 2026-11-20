# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the repository once, then execute the next verified engineering task; do not repeat completed audits or spend sessions on planning alone.

- One atomic main commit per logical slice. Create all changed blobs and one complete tree based on the live tree, parent the commit to the live SHA, and update with force=false plus exact expected_sha.
- Never force-push, amend published commits, develop on branches/PRs, or modify the frozen backup.
- Match Actions to the exact head_sha. Diagnose failures from causal logs. A pending run is not a stop condition; work on independent tasks.
- Preserve GNU89, MIPS/uClibc, BusyBox/Linux 4.4 compatibility, fail-closed behavior, ABI/config compatibility and meaningful regressions.
- Before context exhaustion, finish the current safe slice, verify the exact SHA and leave one precise next action.

## Current repository state

- Parent main SHA for the current checker correction: 971372f4a8a662eeda42e3d7b80ec6f9be6e3de2.
- The source change under validation is fix(wan): report resolver snapshot clear failures on teardown. In wan_down_internal(), the global vpnc_dns_t and conditional indexed wanN_dns snapshot clears use errno-aware setters, aggregate first-error status, and continue independent VPN stop, route/interface cleanup and resolver refresh.
- The prior PPP ip-up source baseline b4ab85764446ab133873f9d931d411ef25e7761e publishes the five-field volatile ipaddr/netmask/ifname_t/gateway/dns tuple through checked NVRAM writes, restores the previous tuple on partial failure, removes the new interface address and blocks WAN route setup unless publication succeeds. Its stale WAN DHCP/HTTPD source contracts were repaired by b68158c3ee405bcdc3270db0e6adfd9de91828e5; those exact-head checks passed.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807, read-only. Branch inventory contains only main and this backup.
- Legacy PR #1 is CLOSED / NOT MERGED; its AliDDNS HMAC/CA bootstrap functionality is already present and evolved on main. Do not repeat the port audit.

## Active task

Complete the checker-only repair for the checked WAN-down resolver snapshot clear:

1. WAN route run 44686752930 and XY-C1 run 44686753007 failed because the first corrected resolver assertion used startswith() at a slice beginning with the setter assignment; it therefore could not match the later closing conditional. The assertion now checks that the close-of-conditional and unconditional resolver refresh sequence occurs later in wan_down_internal().
2. AI handoff run 44686752989 required exact phrases "one atomic main commit" and "Before context exhaustion"; both are now preserved in this handoff.
3. Detect-internet run 44686752941 passed on the previous checker commit. No production C is changed in this correction. Fetch live main for its exact commit SHA and inspect reruns; no PASS is claimed before Actions complete.

## Verification boundary at parent 971372f4a8a662eeda42e3d7b80ec6f9be6e3de2

- PASS: detect-internet regression on previous checker SHA 971372f4a8a662eeda42e3d7b80ec6f9be6e3de2; NVRAM transaction boundary 44686157053 on product SHA 21e567affba4e992476aa7dc16c30be7fa6d4500.
- FAIL, corrected in this commit: WAN route 44686752930; XY-C1 44686753007; AI handoff docs 44686752989.
- XY-C1 stopped at the WAN source-contract step before the full MIPS build/TRX/artifact gate. Product L3 is NOT VERIFIED. Physical-router L4 is NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live main and inspect exact-SHA WAN route, XY-C1 and AI handoff Actions triggered by this correction. Also confirm detect-internet remains green.
2. If a run fails, inspect the failed step log and correct the actual defect without weakening behavioral assertions. Claim L3 only after the current production-source SHA passes the detector, MIPS build, TRX verification and artifact upload on that same SHA.
3. When this WAN resolver slice is green, resume a separate high-impact cross-userland service/process/security issue. Do not repeat the WAN DNS snapshot or PPP ip-up work. Inspect current callsites, implement a production fix plus focused GNU89/Werror regression, and keep independent work moving while Actions are pending.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state. Update this handoff only when resumable facts or the next action materially change. Otherwise make no no-op documentation commit.

## Evidence discipline

Keep host/source regressions, exact-head Actions, MIPS build/TRX and physical-device evidence separate. Only a same-SHA detector, build, TRX verification and artifact upload establish L3; host tests do not. Never infer L4 from CI.
