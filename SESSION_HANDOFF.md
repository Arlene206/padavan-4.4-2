## Current repository state

# Cross-session execution mandate

Continue one autonomous engineering stream on `main`. For each logical source/test slice, create all changed blobs, one complete Git tree based on the live main root tree, and one atomic main commit; then update main with a non-forced expected-SHA compare-and-swap. Never force-push, create/use feature branches, make PRs, split one logical change into per-file commits, or modify the frozen backup. A PENDING workflow never blocks independent work. Do not spend another takeover repeating completed audits.

## Branch and checkpoint

- Live main at the start of this correction: `046025ca928c54b5fe816af4dfc08fd262c06f5a`, source commit `fix(wireless): preserve radio state read failures`. Fetch live main before every write; stored SHAs are historical, not a substitute for a live ref check.
- Frozen `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only and unchanged.
- `main` is the only development ref. PR #1 remains closed/unmerged and deeply behind main; do not port it wholesale.
- The source tree recovery from the earlier udhcpc commit is complete and verified as recorded below.

## Active task

### Wireless radio control — checked state reads and truthful notifications

The production source slice at `046025ca928c54b5fe816af4dfc08fd262c06f5a` adds checked `wireless_radio_power_status()`, negative-status propagation from radio control, checked `wl_radio_x`/`rt_radio_x` enable reads for manual toggles, and `wireless_radio_notification_status_result()` so state-reader or operation errors cannot become successful no-ops. Legacy boolean readers remain available for display-only callers.

The exact-head Wireless workflow `#45490305618` on this SHA failed only at the final Python source-contract assertion. The checked declaration in `trunk/user/rc/src/rc.h` is intentionally column-aligned as `int  wireless_radio_power_status(...)`; the checker expected exactly one space after `int`. All preceding GNU89 unit, ASan/UBSan and source tests in that job passed. This correction normalizes whitespace in the declaration assertion; it does not alter production code. Re-run the entire exact-head workflow and require PASS before closing the wireless slice.

### Completed BusyBox udhcpc DHCPNAK identity handling during REBINDING

- `trunk/user/busybox/busybox-1.24.x/networking/udhcp/dhcpc.c` passes the current state to the NAK server-ID policy.
- `udhcpc_script_policy.h` keeps strict server identity outside REBINDING, while accepting an identified alternate server's NAK in REBINDING because that DHCPREQUEST is broadcast. Missing server-ID remains rejected; the pre-server-selection compatibility case is preserved.
- Added GNU89 policy tests and exact-source checker assertions.
- Exact-head udhcpc regression `#45489316871` on `717f983f07434cd78a4ad00ebdb944b8f6d30fa5` passed, including `-Wall -Wextra -Werror`, ASan/UBSan and source contracts.

Why it matters: a strict comparison against the original selected server can discard an authoritative DHCPNAK from a replacement server during REBINDING, leaving an invalid old address configured until lease expiry.

### Git object recovery — disclose and preserve

The initial source commit `ff9cb0ec0f9143cd398d8de1d9b4eb13dca87be5` was mistakenly built with an empty tree base and therefore omitted unrelated paths. This was caught by inspecting GitHub commit statistics before claiming validation. Non-forced recovery commit `717f983f07434cd78a4ad00ebdb944b8f6d30fa5` restored the repository tree from the real parent tree and retained the four intended DHCP files. REST compare from original main `054ff22bf517a45bf29f2da21963a4206f8b3b83` to 717 confirmed exactly those four files changed. The 717 tree was verified to contain the expected repository paths; do not rewrite or erase the intermediate history. For future Git Data API commits, fetch the live head commit's `commit.tree.sha` and use it as `base_tree_sha`; never use an empty base tree.

## Verification boundary

### Exact source SHA `046025ca928c54b5fe816af4dfc08fd262c06f5a`

- Wireless radio state run `#45490305618`: FAIL at one final static-contract whitespace assertion described above; the C policy binaries and prior source contracts passed. This commit fixes that assertion; verify the new exact-head run before closing it.
- AI handoff documentation `#45490305409`: PASS, including the required phrase `one atomic main commit` and mandatory checkpoint rule.
- XY-C1 firmware validation `#45490305564`: in progress at last poll; cross-build job `138786530898` had not completed. Do not claim L3 until Build, TRX verification and artifact upload all pass on the same exact source SHA.
- NVRAM transaction boundary audit `#45490305501`: in progress at last poll; do not block independent work waiting on it.
- The other exact-head runs in the 046 Actions list were PASS when last checked (including WAN callback, DNSMasq, HTTPD auth/security, firewall, VPN, VSFTPD and lifecycle checks). Refresh the live exact-head list before final claims.

### Earlier exact source SHA `7e700e86e191bfbf11d563fce7f9cfbbcd539516`

- XY-C1 firmware `#45486634245`: PASS through Build, TRX verification and artifact upload on this SHA only; L3 is verified for 7e only.
- NVRAM transaction audit `#45486634206`: PASS as a bounded scan (4,581 userland sources; raw ioctl references remained inside the locked adapter; no global atomicity guarantee).
- VSFTPD, AI handoff and process inventory exact-SHA checks passed.
- No physical-router validation has occurred. Hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch the live main SHA and exact-head Actions. Confirm the new Wireless regression run and the AI handoff documentation run both PASS; inspect logs and fix any real FAIL rather than relying on the parent run.
2. Recheck XY-C1 `#45490305564` and NVRAM audit `#45490305501` on their exact SHA. XY-C1 L3 requires Build, TRX and artifact upload all PASS on one SHA. A pending run is not a reason to stop other work.
3. Continue the next independent security slice: harden QMI status-file reads in `trunk/user/rc/src/usb_modem.c`. `get_qmi_handle()` currently uses `fopen()` on root-owned status paths under `/tmp`, following symlinks and potentially blocking on a pre-created FIFO. Add a dedicated reader using `O_NOFOLLOW|O_NONBLOCK`, validate regular-file type, euid ownership, exact mode 0600 and link count 1, and use bounded reads with NUL/oversize rejection. Add GNU89/ASan/UBSan tests for symlink, FIFO, permissions, hardlinks, malformed and oversized contents. The writer `trunk/user/uqmi/uqmi-status-file.h` already publishes atomically with restrictive metadata; make the reader enforce the same trust boundary.
4. Keep LAN DHCP/AP-mode lease PARTIAL/AUDITING: `leasefail`, full live lease/NAK transitions, target MIPS/uClibc runtime and physical L4 remain open. Do not reopen the completed DHCPNAK policy or repeat prior NVRAM/VSFTPD/process inventories.
5. Before context exhaustion, perform the mandatory handoff checkpoint and update root docs only when a material fact or first action changes.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session must compare live repository state, exact-head Actions, active source baseline, validation boundary, blockers and first next action against this file before ending. Record a relevant PENDING/FAIL gate that changes state. If all facts and the first action remain accurate, explicitly conclude the checkpoint with no file change and no commit. Never create documentation churn solely to synchronize a stored SHA with live main.
