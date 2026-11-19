# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume concrete engineering tasks. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning. Before context exhaustion, complete the safe slice and preserve its exact source/CI boundary.

- Create all changed blobs, one complete tree and one commit with the current main parent; update main with non-forced compare-and-swap using that exact SHA.
- Verify Actions by exact `head_sha`. A pending run does not block independent work; a failed relevant run must be triaged before claiming validation.
- Preserve security, architecture, GNU89/MIPS/uClibc compatibility, regression coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` is archival only; never alter or delete it. Develop on main.

## Current repository state

- Parent for this source/doc atomic slice: `39c3db7ba0c4c1ec396a00b619e2c4c69993cc56` (`test(rc): fix repeated sleep EINTR fixture`). The exact new source SHA is the live `main` HEAD carrying this file.
- Parent exact-head runs: AI docs `44634616433`, NFS lifecycle `44634616486`, Samba stability `44634616497`, WAN route `44634616477`, Process inventory `44634616465` and Wireless radio `44634616547` PASS on `39c3db7ba0c4c1ec396a00b619e2c4c69993cc56`. Process umbrella `44634616429` and NVRAM audit `44634616501` were IN_PROGRESS at last poll.
- The preceding source SHA `ba47b5f7425cfef4dab3f191ce971cefce329006` passed AI docs, Process umbrella, GPIO, detect_link, HTTPD, VPN, crond, watchdog and NVRAM. XY-C1 `44634242726` was IN_PROGRESS at last poll; Build/TRX/artifact were not confirmed.
- Earlier shared procfs read fix `53dda2c844e18cfb879abc3bf4b96bb50d46fd2d` passed Process inventory, detect_link, GPIO, DNSMasq, VPN, crond and NVRAM; `16b755661ec1a33cd255dec10d4bac389816a8b3` repaired Process workflow push/PR path parity. The immediately preceding `39c3db7` commit fixes the service-wait seconds-sleep EINTR regression fixture; its AI, NFS, Samba, WAN route, Process inventory and Wireless checks passed.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; archival only. PR #1 remains closed/unmerged. No physical-router result is available.

## Active task

- New firewall defect: `ip2class()` counted set bits from `inet_addr(lan_netmask)` without confirming that the mask was a valid contiguous prefix. Invalid text could become `/32`, while non-contiguous masks could be converted into an unrelated prefix; this can mis-scope LAN NAT/firewall rules.
- Added `firewall_ipv4_netmask_is_safe()` to require a dotted IPv4, nonzero contiguous mask. The main firewall rebuild validates the full NVRAM value before `ip2class()`; the standalone baseline NAT builder performs the same check before opening/replacing its restore file when the LAN address is valid and NAT is enabled.
- The existing filter-policy GNU89 regression covers valid /1 through /32 examples, zero, non-contiguous, malformed and rule-injection strings. The Firewall Actions source contract pins validation-before-subnet-derivation and standalone NAT validation. The isolated policy logic passed local `-std=gnu89 -Wall -Wextra -Werror -fsanitize=address,undefined`; integrated exact-head Actions have not yet been checked.
- All changed source/test files are already in Firewall workflow push and pull_request filters. This commit also updates that workflow's source-order contract, the dev log, the firewall roadmap row and this handoff.

## Exact first action for the next GPT

1. Fetch the live main HEAD (the commit carrying this handoff) and inspect Firewall regression, AI handoff documentation and XY-C1 Actions by exact `head_sha`. On FAIL, inspect the exact failing job and repair the demonstrated issue without weakening netmask rejection or workflow assertions.
2. L3 requires XY-C1 Build, TRX verification and artifact upload all SUCCESS on the same source SHA. Do not call the isolated host policy test a firmware build.
3. Continue with an independently evidenced stability/security fix outside repeated audits of lifecycle locks, procfs read retries, or this completed netmask validation.

## Verification boundary

- L1: policy logic passed the isolated GNU89/Werror/ASan/UBSan test. The committed policy/test/workflow integration is awaiting the exact-head Firewall workflow.
- L2: parent SHA `39c3db7ba0c4c1ec396a00b619e2c4c69993cc56` AI docs, NFS, Samba, WAN route, Process inventory and Wireless runs PASS; Process umbrella and NVRAM were pending at checkpoint. Exact source SHA carrying this handoff needs Firewall and AI-doc Actions checked.
- L3: XY-C1 Build/TRX/artifact for the exact current source SHA is pending/unverified.
- L4: LAN subnet and NAT-loopback behavior on XY-C1 or another physical router remains NOT VERIFIED.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST perform a handoff checkpoint before ending. At session end, compare live main, product-source baseline, exact-head Actions, module state and this first action. Update this file only when resumable facts materially change; do not create no-op SHA-refresh commits.

## Development invariants

- Main-only; never modify the frozen backup or develop on a legacy branch/PR.
- One complete Git tree and one atomic commit per logical task; `force=false`, exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits or weaken regressions.
- Report exact SHA Actions and keep L1/L2/L3/L4 distinct. Host success is not firmware/hardware success.
