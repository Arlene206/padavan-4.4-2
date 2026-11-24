## Current repository state

- Active branch: `main`; this deauthentication hardening slice is based on exact parent SHA `0dc7eaed5c1eb2623f5762c9f9b6fb7b93c9a4d0`. Always refresh live main before writing.
- Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify, develop on or delete it.
- No open PRs were found. Legacy PR #1 is closed; its valid CA trust-store bootstrap and AliDDNS HMAC work were ported to main.
- LLDT descriptor/path hardening history: `529cf073`, `8796b356`, `2fdf8e7a`, `9b97f743`, `be370293`, `0b532570`, `b270bcc`. Process integration #45376899681 PASS on `b270bcc`; dedicated LLDT #45376256149 PASS on `0b532570`.
- MTK AP hardening: 802.11r Boolean `72e1e72`; MT76x3 PSK `e32120d`; R0KH-ID `b9d55bf`; R0KH checker follow-ups `b046206` and `2789d637`; AuthMode/WepStatus/SSID/APCLI Boolean copy failures `8f639815`; WSC PSK bounds and secret-log removal `0dc7eaed`.

## Active task — Validate MTK AP deauthentication usercopy failure handling

- MT76x2, MT76x3, MT7615 and MT7915 `OID_802_11_DEAUTHENTICATION` handlers performed exact request-length checks but consumed request fields after ignoring/storing `copy_from_user()` errors. Faulted/partial reads could lead to uninitialized or partially copied addresses/reasons entering MacTableLookup/MLME enqueue and APCLI state transitions.
- This slice adds explicit `-EFAULT` cleanup branches before any request field use or deauthentication side effect. It frees the MLME queue element and any separately allocated APCLI request buffer. MT76x2/MT7915 APCLI branches now check the request allocation and report `-ENOMEM` before copy.
- `scripts/check_mtk_apd_pid_ioctl.py` now checks exact request sizes, every copy-failure branch, cleanup before break, successful-copy ordering and the APCLI allocation guard. The existing focused MTK workflow already monitors all four AP source files and this checker; a fresh exact-head run is required.
- Follow-up review of the new APCLI failure handling found two additional defects in the same source slice: MT76x2 and MT7915 each call `os_alloc_mem()` twice for `pInfo` before checking it, overwriting the first pointer and leaking that allocation on both success and the second allocation's failure path. Both handlers also read `ApCliTab[ifIndex]` / `StaCfg[ifIndex]` before checking `ifIndex >= MAX_APCLI_NUM`, allowing an out-of-range ioctl interface index to cause an array access before rejection. The current source checker checks only that an allocation guard exists before copy; it does not require exactly one allocation or bounds-check-before-index ordering.
- Previous PSK fix in `0dc7eaed`: fixed the MT7615/MT7915 WSC destination overflow (64-byte WSC buffer vs 65-byte security PSK buffer), stages one checked usercopy, caps inputs at 64 and removes plaintext PSK logs. Exact MTK #45381270549, AI #45381270437, Process #45381270630 and NVRAM #45381270585 PASS on `0dc7eaed`.
- XY-C1 #45381270533 on exact source SHA `0dc7eaed5c1eb2623f5762c9f9b6fb7b93c9a4d0` was still in Build XY-C1 at last check; TRX and artifact pending. This deauth source change requires its own exact-SHA firmware validation.
- Earlier R0KH-ID compatibility supports raw 1–48-byte IDs or 2–49-byte inputs containing trailing NUL; focused MTK #45379297940 and AI #45379297910 PASS on `2789d637`. LLDT Process integration #45376899681 PASS on `b270bcc`.

## Verification boundary

- Main source SHA `28bf02ce0d95edbf81db51816c1683805f1cc28c` contains the deauthentication checked-usercopy change, but its focused MTK and XY-C1 exact-head push Actions have not been verified in this session. `get_commit_combined_status` returned no status contexts; the available workflow-run fetch wrapper is PR-event-only and does not expose `push` runs. Do not infer PASS.
- This session confirmed the source-level duplicate-allocation and pre-bounds-check array-access defects described above. No repository checkout was available locally (`git ls-remote` could not resolve GitHub); no local tests or target build were run. No firmware L3 or hardware L4 claim.
- L4 physical-router validation: NOT VERIFIED.

## Exact first action for the next GPT

1. In one atomic multi-file slice, remove the duplicate `pInfo` allocation in MT76x2 and MT7915 APCLI deauth handlers, and move each `ifIndex >= MAX_APCLI_NUM` guard immediately after reading the index and before any APCLI array dereference. Preserve exact `-ENOMEM`/`-EFAULT` behavior and one-time cleanup.
2. Extend `scripts/check_mtk_apd_pid_ioctl.py` to require exactly one `pInfo` allocation in each APCLI deauth path and bounds validation before the first `ApCliTab` / `StaCfg` access. Run the focused checker and its GNU89 test suite, then publish all source/test changes through one complete Git tree and a non-forced `main` compare-and-swap from the latest head; do not use sequential per-file `update_file` commits.
3. Inspect exact `push` Actions on the resulting SHA, including MTK and XY-C1 Build/TRX/artifact. Fix any failure on a new atomic `main` commit, then continue to the next P1 defect outside the AP private-ioctl cluster.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository before ending. Record only material exact-head Action transitions, new source commits, genuine unresolved failures, or a changed first action. Do not manufacture documentation churn or no-op commits.

A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action remain accurate, explicitly conclude the check with no file change and no commit.

# Cross-session execution mandate

Recover once from live repository state and execute development on `main`. Never create or use a development branch or PR; the frozen backup is immutable.

For each multi-file logical slice, create all blobs, one complete tree based on the latest main tree and one commit with the exact main parent, then update `main` using a non-forced compare-and-swap with `expected_sha`. This is one atomic main commit. Never issue sequential partial file commits, force-push, amend, or rewrite published history.

Continue implementation rather than repeating completed audits or stopping at planning. PASS advances; FAIL is investigated and fixed; PENDING does not block independent tasks. Preserve Linux 4.4, GNU89, MIPS/uClibc, router compatibility, security and configuration.

Before context exhaustion, finish the current safe slice and record exact latest main SHA, tests, CI states, unresolved issues and one concrete first action for the next GPT. At session end, inspect live main and exact-head push Actions; update this handoff only when active task or first action materially changes, append substantive milestones to `dev_log.md`, and update roadmap only for material module/status/priority changes. Do not create no-op documentation commits.
