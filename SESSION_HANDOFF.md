# Cross-session execution mandate

## Execution policy
Continue main-only implementation across conversations. Recover the repository once, then resume the exact active task; avoid repeated audits, planning-only stops and status-only commits.

- Each logical task ends in one atomic main commit. For multi-file changes create all blobs, one complete Git tree and one commit parented to live `main`, then update `main` with `force=false` and exact `expected_sha`.
- Never create/use development branches or PRs, force-push, amend published commits, or modify the frozen backup.
- Check Actions by exact `head_sha`. Diagnose FAIL from causal logs; while checks are PENDING, continue independent work.
- Preserve GNU89, MIPS/uClibc, BusyBox/Linux 4.4 compatibility, fail-closed behavior, process identity checks, bounded operations and meaningful regression coverage.
- Do not ask the user what to do next. Before context exhaustion, finish the current safe slice and leave an accurate next action.

## Current repository state

- Current main before this WAN startup publication correction: `0d04cabd439f6f965ef78195821043b82b471d7e`.
- Frozen archival branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only and untouched.
- Checker-contract repair commit `0d04cab`: WAN route, HTTPD security, miniupnpd config and AI handoff docs all PASS on exact SHA in runs `44675647123`, `44675647093`, `44675647150`, `44675647174`.
- WAN PPP recovery production fix commit `68571af`: WAN NVRAM status regression + DDNS contract, Process regression/inventory, AI docs, firewall startup, NVRAM transaction audit all PASS. XY-C1 run `44674653396` PASS on that exact production SHA: detector, Build XY-C1, TRX verification and artifact upload all PASS in job `136083493218`. This establishes L3 for `68571af` only; physical-router L4 remains NOT VERIFIED.
- The earlier source contracts were corrected without weakening assertions. No current-source firmware build has yet been run for the new startup-publication fix in this slice.

## Active task completed in this slice

- Confirmed a second failure-sensitive unchecked active-WAN identity publication defect in `start_wan()`. Three protocol branches assigned `wanN_ifname_t` using the legacy void setter and continued toward `start_firewall_ex()` or protocol launch without checking whether NVRAM accepted the interface name.
- This is security/stability-relevant: `firewall_ex.c::start_firewall_ex()` directly reads `wanN_ifname_t` to select the WAN interface in firewall rules. If the update fails, a stale previous PPP/physical interface can still be a syntactically valid value, so firewall rebuild may succeed against the wrong WAN device.
- Changed USB modem RAS, ordinary PPPoE/PPTP/L2TP and IPoE setup to use the existing private checked setter. On failure each path records the first WAN error, logs the interface and errno, invokes the existing fail-closed physical-WAN recovery wrapper, records recovery status without overwriting the first error, and `continue`s before firewall rebuild or client/protocol activation.
- Extended `scripts/check_wan_firewall_status.py` to require exactly two checked PPP-interface publications and one checked IPoE publication, and require failure aggregation + physical-WAN recovery + branch abort before firewall rebuild. The existing GNU89/Werror/ASan/UBSan unit already covers `nvram_set_temp` result/errno normalization; the existing workflow runs this integration checker. No legacy public setter ABI changed.
- Exact-head tests for THIS commit and a production-source XY-C1 run are pending. The preceding 0d04 workflow-contract commit passed its four exact-head Actions checks; the preceding 68571 production source passed L3 as above.

## Verification boundary

- Earlier L2 and L3 evidence is tied to the exact SHA listed above and must not be misattributed to this new product-source change.
- No local repository checkout/full firmware build is available in this session. The new code uses existing GNU89-compatible syntax and helper APIs; the exact WAN route regression and current-source XY-C1 build/TRX/artifact must validate it.
- No physical router test is claimed.

## Exact first action for the next GPT

1. Fetch live `main` and inspect Actions by exact `head_sha` for THIS checked-startup-publication commit. Require WAN route (including the NVRAM binary and extended `check_wan_firewall_status.py` integration contract), Process regression/inventory and AI docs to pass. Diagnose any first failure from its log.
2. Require the XY-C1 detector, MIPS build, TRX verification and artifact upload to pass on one identical source SHA before claiming L3 for this fix.
3. Continue the P1 NVRAM publication audit only where later firewall/interface/route/daemon behavior consumes the written value. Inspect `set_wan_unit_value_int()`, `set_wan_unit_param()` and direct `nvram_set*_temp()` calls narrowly; do not bulk-convert telemetry or unrelated UI state.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state before ending. Update this handoff only when resumable facts, validation state, blocker or first action materially change. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

## Evidence discipline

Keep L1 source review, L2 host/Actions, L3 MIPS build/TRX and L4 physical-device evidence separate. A skipped host step is not PASS, a successful cross-build does not establish router runtime behavior, and every claim must name the exact source SHA and run.
