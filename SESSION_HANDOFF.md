## Current repository state

- Active branch: `main`; current parent for this slice: `0dbf3d2f49133f6cbd471d7b6ba4efe633e5bb96`. Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. No open PRs.
- MTK AP private-ioctl, HTTPS-build HTTPD selector and WAN source-PHY range fixes retain their recorded exact-SHA L3 evidence. Physical-router L4 remains unverified.
- SYN-cookie selector whitespace/sign fix on `5a7b2c4`: WAN route #45394597807, Process inventory #45394597845, AI docs #45394597880 and NVRAM #45394598006 PASS. XY-C1 #45394597932 remains in progress at Build; TRX/artifact pending.
- Latest parent `0dbf3d2f49133f6cbd471d7b6ba4efe633e5bb96` fixes Ralink `regspec_code` NVRAM error handling. Exact SHA: HTTPD auth #45396051105, Process inventory #45396051070, AI docs #45396051142 and wireless radio state #45396051132 PASS; NVRAM audit #45396051150 and XY-C1 #45396051437 remain in progress. XY-C1 had not reached Build yet (wireless lifecycle step running), so no L3 claim.

## Active task — Preserve QMI WDS ownership after PDH cleanup failure

- In `usb_modem.c:qmi_control_network()`, successful `--stop-network` was followed by removal of `QMI_HANDLE_PDH`. If that unlink failed, old code only set `result = 1` and continued to release `QMI_CLIENT_ID` and remove its CID marker.
- The stale PDH makes the next start fail its existing-PDH preflight with `EBUSY`; releasing the CID at that point discards the reusable modem-side WDS identity and worsens recovery.
- This slice gates CID release on `modem_qmi_client_id_release_allowed(cleanup_status)`. Successful unlink and `ENOENT` remain allowed; any other error returns before reading/releasing the CID, preserving WDS ID and marker. GNU89 regression covers success, EACCES, EROFS and EIO. `check_uqmi_private_credentials.py` pins the cleanup gate/return before CID read and release command.
- Changed paths: `usb_modem.c`, `modem_wan_start_policy.h`, `tests/modem_wan_start_policy.c`, `scripts/check_uqmi_private_credentials.py`, this handoff, roadmap and `dev_log.md`. Existing USB modem workflow already lists source, policy/test and checker in both push and pull_request path filters.

## Verification boundary

- Parent `0dbf3d2f49133f6cbd471d7b6ba4efe633e5bb96`: AI docs, Process and wireless-radio-state exact-SHA push runs PASS; HTTPD auth also PASS. NVRAM audit and XY-C1 remain pending.
- New QMI teardown commit: check exact-head USB modem regression/source checker, Process inventory, AI docs, NVRAM audit and XY-C1 Build/TRX/artifact. L2 is not verified until host regression and source checker actually pass on this SHA. L3 requires same-SHA Build, TRX and artifact upload PASS. Physical modem/WWAN L4 is NOT VERIFIED.
- No local full checkout was available during recovery; remote source changes are made as one atomic Git tree/commit update to `main`. Do not claim a local source build unless actually run.

## Exact first action for the next GPT

1. Fetch live `main` and query Actions for its exact SHA; inspect the USB modem workflow's `modem_wan_start_policy` regression and `check_uqmi_private_credentials.py` source-order contract first.
2. Fix any causal exact-head failure atomically on `main`; then inspect XY-C1 Build/TRX/artifact for the same SHA.
3. Once validated, continue the next highest-priority independent whole-userland stability/security defect; do not re-audit completed SYN-cookie/MTK/HTTPD/WAN-selector slices.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Record only material exact-head Action transitions, new source commits, genuine unresolved failures, or a changed first action. Do not manufacture documentation churn or no-op commits. If all checkpoint facts and the first action remain accurate, explicitly conclude with no file change and no commit.

# Cross-session execution mandate

Recover once from live repository state and execute development on `main`. Never create or use a development branch or PR; the frozen backup is immutable.

For each multi-file logical slice, create all blobs, one complete tree based on the latest main tree and one commit with the exact main parent, then update `main` using a non-forced compare-and-swap with `expected_sha`. This is one atomic main commit. Never issue sequential partial file commits, force-push, amend, or rewrite published history.

Continue implementation rather than repeating completed audits or stopping at planning. PASS advances; FAIL is investigated and fixed; PENDING does not block independent tasks. Preserve Linux 4.4, GNU89, MIPS/uClibc, router compatibility, security and configuration.

Before context exhaustion, finish the current safe slice and record exact latest main SHA, tests, CI states, unresolved issues and one concrete first action for the next GPT. At session end, inspect live main and exact-head push Actions; update this handoff only when active task or first action materially changes, append substantive milestones to `dev_log.md`, and update roadmap only for material module/status/priority changes. Do not create no-op documentation commits.
