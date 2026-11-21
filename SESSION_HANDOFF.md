# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main, the engineering baseline, exact-head Actions, validation boundaries and the first next action before ending. Update this file only when those facts materially change; do not create a no-op commit or chase documentation-only SHAs. Each multi-file logical slice must land as one atomic main commit. Before context exhaustion, finish the active safe slice and leave an exact first action.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development and validation branch: `main`.
- Parent main at the latest completed source checkpoint: `23520100e3cc74bb7f77a0d023afa361cded2abe`. It fixes the stale five-argument xl2tpd caller of the checked four-output `get_vpns_pool()` API and validates the pool before truncating `/etc/xl2tpd.conf`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`; it is immutable and must not be modified or deleted. PR #1 is closed/unmerged and is not a development path.
- Do not create branches/PRs, force-push, amend history, touch GPT Session Rotator or replay SHA `9d5a5cb`.

## Active task

- OpenVPN client `on_client_ifup()` previously read `vpnc_pdns` using `nvram_get_int()`. A backend read failure was indistinguishable from the disabled value and could silently publish an empty pushed-DNS snapshot/domain, then refresh resolver state.
- Corrective slice: read `vpnc_pdns` with `nvram_get_checked()`, validate it as 0/1 using `vpn_server_config_int_status()`, and preserve the existing resolver/DNS/domain snapshot when the optional setting is unreadable. Still publish `vpnc_state_t=1`, run the mandatory up hook, and clean callback environment variables; report the policy read error. The down callback independently attempts DNS restoration, clears domain state and always runs its down hook.
- The source contract checks the checked read, policy error path, mandatory state publication and hook order. Existing numeric policy tests cover missing/default, invalid, out-of-range and backend failure behavior.

## Verification boundary

- On exact parent SHA `23520100e3cc74bb7f77a0d023afa361cded2abe`, VPN server lifecycle #44937972537 and AI handoff docs #44937972518 passed; WAN route #44937972672, WAN DHCP callback #44937972690, HTTPD #44937972661 and Process inventory #44937972749 passed. NVRAM audit #44937972629 and XY-C1 #44937972691 were still running at the last observation.
- The checked xl2tpd caller fix passed the exact-head VPN lifecycle workflow; XY-C1 build/TRX/artifact still requires the exact-head run result. L3 is NOT VERIFIED until Build, TRX and artifact upload all pass on one SHA. L4 hardware flash/boot/runtime validation remains NOT VERIFIED.
- Inspect exact-head push Actions for the current OpenVPN callback change before declaring it validated. Do not claim local tests or builds that were not run.

## Workflow and preservation rules

- Main only; no feature/fix/checkpoint branches and no PR-based development. For each multi-file logical slice create all blobs, one tree, one commit and a non-forced main ref update with the exact expected SHA. No per-file partial commits, force push, amend, or history rewrite.
- Re-fetch main immediately before each write. If it moved, rebuild the patch from the new head. Verify only Actions whose push event and head SHA match the published main commit.
- PENDING checks do not block independent engineering. Fix relevant FAILs from the first causal error before calling a slice verified.
- Preserve GNU89, MIPS/uClibc, BusyBox, Linux 4.4, ABI, legacy NVRAM defaults, security and device compatibility. Do not weaken regression coverage or workflow path filters.
- `dev_log.md` retains 20–119 newest-first entries. Batch archive oldest 100 before reaching 120; rotate early above 384 KiB and never exceed 512 KiB. Preserve all archives byte-for-byte. Update the log for real milestones; update roadmap only when repository-wide status/priority changes; do not add routine handoff/roadmap snapshots.

## Exact first action for the next GPT

1. Fetch live `main` and inspect exact-head push Actions for the OpenVPN `vpnc_pdns` checked-read fix, plus the pending XY-C1 and NVRAM results for `23520100e3cc74bb7f77a0d023afa361cded2abe`. Diagnose any new failure from its first causal error.
2. Continue the server callback audit: inspect `ovpn_server_script_main()` in `trunk/user/rc/src/vpn_openvpn.c`. Its `vpns_ov_mode` compatibility getter currently collapses read errors to TAP mode; use a checked mode source without suppressing valid client-connect/disconnect lease and hook cleanup.
3. Reconfirm exported server profile device/protocol/port/TLS/auth/cipher/compression parity against the checked server network/options snapshots. Do not reopen completed CCD ACL, credential staging, or client option snapshots absent a same-SHA regression.
