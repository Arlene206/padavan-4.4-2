## Current repository state

- Active branch: `main`; current parent for this slice: `b05eae997a88760cbb70437f5ab87d84567bf3d4`. Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. No open PRs.
- MTK AP private-ioctl, HTTPS-build HTTPD selector and WAN source-PHY range fixes retain their recorded exact-SHA L3 evidence. Physical-router L4 remains unverified.
- QMI teardown/credential fix on `b05eae9`: USB modem #45396493190 (including GNU89 WAN-start policy test and the Python source-order/credential contract), Process inventory #45396493113/#45396492751, AI docs #45396493061, NVRAM audit #45396492703, WAN DHCP callback #45396492710, WAN teardown #45396492829, detect-link #45396493228 and HTTPD #45396492941 all PASS. XY-C1 #45396493118 is still building; TRX/artifact pending. Host L2 is verified; no L3 claim.
- SYN-cookie source fix on `5a7b2c4` and Ralink regspec read fix on `0dbf3d2` have their recorded exact-host Actions; corresponding firmware evidence remains tied to the exact production SHAs and must not be inferred from descendants.

## Active task — Honor DHCPRELEASE during pending manual-renew cleanup

- In `dhcpc.c`, repeated SIGUSR1 while `RENEW_REQUESTED` invokes `deconfig` before returning to INIT. If that callback fails, the client enters `DECONFIG_PENDING` while the old lease and its server/IP tuple are still active.
- A later SIGUSR2 in `DECONFIG_PENDING` previously only changed `deconfig_resume_state` to RELEASED; it bypassed `perform_release()` and never sent DHCPRELEASE for the still-active lease.
- This slice carries a one-bit “lease still active” state only from the failed repeated-manual-renew path. SIGUSR2 sends DHCPRELEASE before retrying/departing the old lease, while NAK, lease expiry, ARP conflict and an already-sent normal release do not send duplicate packets. The marker clears when the pending deconfig completes. GNU89 policy regression covers the allow/deny matrix; the Python contract pins the origin-state gate and send-before-resume order.
- Changed paths: BusyBox `dhcpc.c`, `udhcpc_script_policy.h`, its GNU89 test, `check_udhcpc_script_status.py`, this handoff, roadmap and `dev_log.md`. The existing udhcpc workflow already filters all source/test/checker paths on push and pull_request.

## Verification boundary

- Parent `b05eae997a88760cbb70437f5ab87d84567bf3d4`: QMI USB suite, Process/AI/NVRAM/WAN/DHCP/detect-link/HTTPD host workflows PASS on the exact SHA. XY-C1 #45396493118 had Build in progress; TRX/artifact pending.
- New DHCPRELEASE commit must pass the exact-head udhcpc GNU89 ASan/UBSan test and Python source contract, plus same-SHA Process/NVRAM/AI and XY-C1 Build/TRX/artifact. Do not claim L3 until Build, TRX and artifact upload pass. Live DHCP server behavior and physical-router L4 remain NOT VERIFIED.
- No local full checkout was available during recovery; source is committed by one atomic Git tree to `main`. Do not claim local BusyBox/MIPS compilation unless actually run.

## Exact first action for the next GPT

1. Fetch live `main` and query Actions by its exact SHA; inspect `udhcpc-script-status-stability.yml` test and source contract first.
2. Fix any causal failure atomically; inspect XY-C1 Build/TRX/artifact for that same production SHA.
3. Continue the next independent high-priority userland stability/security defect; do not re-audit the completed QMI, SYN-cookie, MTK, HTTPD or Ralink fixes.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Record only material exact-head Action transitions, new source commits, genuine unresolved failures, or a changed first action. Do not manufacture documentation churn or no-op commits. If all checkpoint facts and the first action remain accurate, explicitly conclude with no file change and no commit.

# Cross-session execution mandate

Recover once from live repository state and execute development on `main`. Never create or use a development branch or PR; the frozen backup is immutable.

For each multi-file logical slice, create all blobs, one complete tree based on the latest main tree and one commit with the exact main parent, then update `main` using a non-forced compare-and-swap with `expected_sha`. This is one atomic main commit. Never issue sequential partial file commits, force-push, amend, or rewrite published history.

Continue implementation rather than repeating completed audits or stopping at planning. PASS advances; FAIL is investigated and fixed; PENDING does not block independent tasks. Preserve Linux 4.4, GNU89, MIPS/uClibc, router compatibility, security and configuration.

Before context exhaustion, finish the current safe slice and record exact latest main SHA, tests, CI states, unresolved issues and one concrete first action for the next GPT. At session end, inspect live main and exact-head push Actions; update this handoff only when active task or first action materially changes, append substantive milestones to `dev_log.md`, and update roadmap only for material module/status/priority changes. Do not create no-op documentation commits.
