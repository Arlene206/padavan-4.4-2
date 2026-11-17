# 2026-11-17 continuation checkpoint — fix AP DHCP retry source contract

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; parent main before THIS checker repair: `0af358f3cfd9083994054fa97d22371245246eef`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Exact parent-0af WAN route `44202380619` PASS; AI handoff `44202380778` PASS; DNSMasq `44202380652` PASS; detect_internet `44202380544` PASS; Wireless radio `44202380631` PASS; Process inventory `44202380727` PASS; AliDDNS `44202380589` PASS. NVRAM `44202380759` and XY-C1 `44202380583` were still IN_PROGRESS at last poll.
- Parent WAN DHCP callback run `44202380584` failed only in `scripts/check_lan_dhcp_deconfig.py` at line 89: it compared a C call as a single-line literal, but the implementation intentionally wraps the call across lines. The DHCP lease input policy binary passed before the checker failed.
- THIS repair normalizes whitespace in that assertion while preserving its required call/argument ordering. The WAN DHCP workflow already compiles and runs `lan_dhcp_callback_policy.c` with GNU89, ASan and UBSan after the checker step.
- Prior static IPoE teardown fix `853748bd`: WAN teardown workflow `44201908894` PASS; WAN route's stale declaration source assertion was corrected by `d7eea9c0`, and the exact `d7ee` WAN route rerun passed `44202284751` (check this exact result when needed). NVRAM `44201909080` PASS; XY-C1 `44201909002` was still building at last poll.
- No local MIPS toolchain or physical router: product L3 applies only to the exact SHA whose Build/TRX/artifact all pass; hardware L4 NOT VERIFIED.

## Active task
- The AP LAN DHCP retry implementation on `0af358f3` adds temporary `lan_hosts_retry` state, queues `RCN_RESTART_DHCPD` after an AP hostname-file publication failure and retries only host publication on a later unchanged RENEW. Successful AP host publication clears the marker.
- The code call is deliberately formatted over multiple lines:
  `lan_dhcp_hosts_refresh_required(` followed by `is_changed, is_renew,`. The previous checker assertion incorrectly required those tokens on one line; THIS commit changes the source check to normalize whitespace and still validates exact argument order.
- No production source is changed by this checker repair. The separate GNU89/ASan/UBSan callback policy test is already wired into the WAN DHCP workflow.

## Verification boundary
- Exact `0af358f3` WAN route `44202380619`: PASS.
- Exact `0af358f3` WAN DHCP `44202380584`: FAIL only on the whitespace-sensitive checker assertion after the DHCP lease-policy binary passed; this repair needs its own exact-head rerun. Callback retry policy binary was not reached in the failed run.
- Exact `0af358f3` AI `44202380778`, DNSMasq `44202380652`, detect_internet `44202380544`, Wireless radio `44202380631`, Process inventory `44202380727`, AliDDNS `44202380589`: PASS.
- Exact `0af358f3` XY-C1 `44202380583` and NVRAM `44202380759`: IN_PROGRESS at last poll. Require Build XY-C1, TRX verification and artifact upload before claiming L3.
- This repair's exact WAN DHCP/AI/XY Actions are pending. No local MIPS compile or hardware L4 claim.

## Exact first action for the next GPT
1. Fetch live `main`; inspect exact-head WAN DHCP, AI and XY-C1 runs for THIS checker repair.
2. Require `scripts/check_lan_dhcp_deconfig.py` to pass, then require the GNU89/ASan/UBSan `lan_dhcp_callback_policy.c` binary and full WAN DHCP callback step to pass. Confirm the normalized source contract still checks both `is_changed` and `is_renew`.
3. Recheck parent `0af358f3` NVRAM and XY-C1 runs separately; do not infer their status from this newer SHA.
4. Continue independent P1 reliability/security work while any build is PENDING. Keep the backup unchanged and hardware L4 NOT VERIFIED.

## Mandatory handoff checkpoint
Every session MUST inspect live main, materially changed exact-SHA Actions, current handoff/roadmap facts and the first actionable task before ending. The checkpoint is mandatory; a document edit is conditional. Record PASS/FAIL/PENDING accurately. If current facts and first action remain accurate, do not create a no-op documentation commit.

# Cross-session execution mandate

Continue one engineering effort across GPT conversations. Recover once, implement concrete defects, test and atomically commit. Do not wait idly on pending CI or ask the user what to do next.

Every logical multi-file change must use Git data APIs to create all blobs, one complete tree and one atomic main commit, parented to freshly fetched main and published with a non-forced expected-SHA compare-and-swap. Never force-push, create development branches/PRs or modify the frozen backup.

Before context exhaustion, finish a safe slice, inspect exact-SHA Actions and leave the next GPT a precise first action.
