# 2026-11-18 continuation checkpoint — WAN teardown and resolver scan failures

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- This documentation checkpoint is based on main `a4fca17d2830ed3e52c0a80ee5850474ed4c4ba5`. Fetch live main before any write; this commit is documentation-only and does not change product source.
- Latest product-source commit is `64a208a94e3609ed73633451b8afc0fe6c5db7a8` (SmartDNS DNSMasq scan error propagation). WAN/RC C-source changes are present by `445789dd488d2f00e76ecb965888044f9421738e`.
- Frozen archival branch `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Do not modify it. PR #1 remains closed/unmerged; do not revive it.
- No local checkout/build or physical-router validation was available. All reported regression evidence below is from exact-head GitHub Actions only.

## Active task

This session completed three source-level stability slices and their regression contracts:

1. WAN/detector coordination: `notify_pause_detect_internet()` now returns normalized status. Controlled AP-LAN, PPP-only WAN and full WAN teardown fail closed before destructive state changes when a live monitor cannot be paused; an absent/already-exited monitor remains a benign no-op. DHCP/PPP deconfig callbacks are different: lease withdrawal is already committed/in progress, so monitor pause is advisory and route/interface cleanup must continue rather than leave stale WAN state.
2. Privileged config scanner reliability: Shadowsocks and SmartDNS DNSMasq helpers no longer convert AWK input/read errors into “no match” or “no managed block”. The helpers preserve the scanner's real exit status and avoid publishing/removing config after a failed preliminary scan. Regression tests inject an AWK command that exits 37 and compare the live config byte-for-byte.
3. Exact-contract repair: the SmartDNS scanner implementation deliberately replaced AWK END exit codes with explicit marker output, but its static checker still required the removed `exit 2`. The checker was corrected, and the exact-head DNSMasq workflow passed.

The PPPoE-MWAN sysfs-aware PPP peer probe is already implemented and covered by its host regression/source contract (prior slice `ba893707`). Do not repeat that candidate investigation unless a new concrete defect appears.

## Verification boundary

- Current main before this documentation checkpoint: `a4fca17d2830ed3e52c0a80ee5850474ed4c4ba5`.
- Exact current-head DNSMasq lifecycle run `44342978278`: completed SUCCESS.
- Product-source SHA `64a208a94e3609ed73633451b8afc0fe6c5db7a8`: the DNSMasq shell regression (including all three injected AWK failure cases) and shell syntax checks passed, but the Python checker failed on one stale assertion expecting the removed `exit 2` implementation. Follow-up `a4fca17d` repairs that checker; its exact DNSMasq rerun `44342978278` is SUCCESS. On `64a208a`, USB-storage run `44342177838` and Process inventory `44342180589` are SUCCESS. XY-C1 run `44342177852` was still IN_PROGRESS at the last poll: all host checks and source contracts completed successfully; Build XY-C1 was running; TRX verification and artifact upload were pending. Recheck that exact run before claiming L3 for the SmartDNS script packaging change.
- WAN/RC product-source SHA `445789dd` had 20 exact-head workflow runs complete successfully, including WAN DHCP callback `44341335030`, WAN route `44341334876`, WAN interface teardown `44341335042`, detector signal `44341334784`, HTTPD security `44341334778`, Process inventory `44341334924`, and XY-C1 firmware validation `44341334940`. The XY-C1 job passed Build, TRX verification and artifact upload on that exact SHA. This verifies the WAN C-source slice, not later SmartDNS script packaging.
- Shadowsocks scanner fix `d0983858`: proxy lifecycle run `44339382141` and Process inventory `44339382041` SUCCESS.
- WAN route checker follow-up `3f475878`: WAN route run `44340687860` and AI handoff run `44340687878` SUCCESS.
- Exact-head Actions for this documentation commit must still be checked after it is pushed. Do not infer a firmware build from a documentation-only commit. L4 physical-router validation remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main`, branch inventory and exact-head Actions once. Inspect XY-C1 run `44342177852` on product SHA `64a208a94e3609ed73633451b8afc0fe6c5db7a8`; record whether Build, TRX and artifact upload completed. If still pending, continue other independent work.
2. Continue the P1 RC startup-result-propagation review by tracing the actual PID 1/RC dispatcher callers of the `void start_adguardhome()`, `start_ddnsto()`, `start_adbyby()`, `start_aliddns()` and `start_frp()` wrappers in `trunk/user/rc/src/services.c`. Establish whether those entrypoints discard required launcher status before changing prototypes or callers; implement only a confirmed loss and add caller-level regression coverage.
3. Continue across independent userland reliability and security surfaces; do not repeat the already completed CI trigger governance audit or PPPoE-MWAN probe work.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state and exact-head Actions before ending. Leave this handoff unchanged when the active task, validation boundary and first next action remain accurate. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit. Do not create a no-op commit.

## Cross-session execution mandate

One logical slice, one atomic main commit. Build the complete Git tree from the exact parent and update `main` only with a non-forced expected-SHA compare-and-swap. Continue independent work while Actions are pending. Do not ask the user what to do next. Keep the archival backup immutable.

## Before context exhaustion

Finish the current safe slice and commit it atomically. Record the exact commit, tests, exact-SHA Actions and unverified boundaries. Leave the next GPT one concrete first action; do not claim local tests, builds, firmware artifacts or hardware checks that were not performed.
