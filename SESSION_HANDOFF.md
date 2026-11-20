# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the repository once, then resume the exact active task; avoid repeated audits, planning-only stops and status-only commits.

- Each logical task ends in one atomic main commit. For multi-file changes create all blobs, one complete Git tree and one commit parented to live `main`, then update `main` with `force=false` and exact `expected_sha`.
- Never create/use development branches or PRs, force-push, amend published commits, or modify the frozen backup.
- Check Actions by exact `head_sha`. Diagnose FAIL from causal logs; while checks are PENDING, continue independent work.
- Preserve GNU89, MIPS/uClibc, BusyBox/Linux 4.4 compatibility, fail-closed behavior, process identity checks, bounded operations and meaningful regression coverage.
- Do not ask the user what to do next. Before context exhaustion, finish the current safe slice and leave an accurate next action.

## Current repository state

- Current main before this WAN source-contract repair: `822f88b10ce52e2ba735f4d51cac12d684a0853c`.
- Frozen archival branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only and untouched.
- Product source SHA `68571af7f68f820569400543e80ee7f9e7a3f148`: Process regression/inventory, AI docs, Firewall Startup Stability, NVRAM transaction audit and WAN NVRAM/DDNS checks passed. Exact XY-C1 run `44674653396` passed detector, Build XY-C1, TRX verification and artifact upload (job `136083493218`). L3 is established for `68571af`, not later production code. Physical-router L4 remains NOT VERIFIED.
- Checker/workflow head `0d04cab`: WAN route `44675647123`, HTTPD `44675647093`, miniupnpd `44675647150`, and AI docs `44675647174` passed.
- WAN active-interface source commit `822f88b10ce52e2ba735f4d51cac12d684a0853c` adds fail-closed `wanN_ifname_t` publication to RAS, PPP and IPoE paths. WAN route `44676321120`, Process inventory `44676321232`, AI docs `44676321121`, VPN `44676321034`, WAN teardown `44676321169`, wireless `44676320974`, ZeroTier `44676321181`, dnsmasq `44676320940`, WAN DHCP `44676320992`, miniupnpd `44676321033`, HTTPD `44676320960`, watchdog `44676321080`, Detect WAN ARP `44676320985`, Detect internet `44676320958`, and Detect link `44676321015` all PASS.
- Exact failures on `822f88b`: Process regression `44676321052`, USB modem `44676321204`, Firewall Startup Stability `44676321094`. All three reached `scripts/check_modem_wan_start_status.py:369`, whose exact aggregate count expected 22; the source correctly contains 28 after six new merges (failed publication + recovery status in each of three paths). Earlier relevant C/unit checks and WAN-specific firewall contracts passed before this stale assertion.
- XY-C1 `44676321180` and NVRAM boundary `44676320949` were IN_PROGRESS at last poll. Do not claim L3 for `822f88b` yet.

## Active task completed in this slice

- Fixed the shared startup-status source contract to expect 28 first-error merges rather than 22, matching the six deliberate result accumulations introduced by three active-WAN interface publication failure paths.
- Added per-protocol assertions for RAS, PPP and IPoE. Each must capture the status-returning `ifname_t` write, aggregate its failure, invoke physical-WAN recovery, aggregate the recovery result, log the error and abort with `continue`.
- No production source or compiled test was changed or weakened by this checker correction. Existing Process, USB modem and Firewall Startup workflows already run this shared checker.

## Verification boundary

- This checker-only repair has not yet been validated by exact-head Actions.
- The exact `822f88b` Process regression, USB modem identity and Firewall Startup failures were all caused by the single outdated aggregate-count assertion. The other listed exact-head contracts passed as above.
- XY-C1 `44676321180` and NVRAM boundary `44676320949` were pending at last poll. L3 for the new source SHA is NOT VERIFIED; no physical-router L4 test was performed.

## Exact first action for the next GPT

1. Fetch live `main` and inspect exact-head Actions for this checker repair. Confirm Process regression, USB modem identity and Firewall Startup Stability now pass with the new per-branch assertions; inspect any remaining causal log.
2. Check same-source NVRAM boundary and XY-C1. Require production-input detector, Build XY-C1, TRX verification and artifact upload on one identical SHA before claiming L3 for `822f88b`.
3. Continue the P1 NVRAM publication audit only where firewall/interface/route/daemon behavior consumes the written value. Inspect `set_wan_unit_value_int()`, `set_wan_unit_param()` and direct `nvram_set*_temp()` calls narrowly; do not bulk-convert telemetry or unrelated UI state. Fix the next proven failure-sensitive callsite with an injected regression and source contract.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state before ending. Update this handoff only when resumable facts, validation state, blocker or first action materially change. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

## Evidence discipline

Keep L1 source review, L2 host/Actions, L3 MIPS build/TRX and L4 physical-device evidence separate. A skipped host step is not PASS, a successful cross-build does not establish router runtime behavior, and every claim must name the exact source SHA and run.
