## Current repository state

# Cross-session execution mandate

Continue one autonomous engineering stream on main. For each logical source/test slice, create all changed blobs, one complete Git tree based on the live main root tree, and one atomic main commit; then update main with a non-forced expected-SHA compare-and-swap. Never force-push, create/use feature branches, make PRs, split one logical change into per-file commits, or modify the frozen backup. A PENDING workflow never blocks independent work.

## Branch and checkpoint

- Source parent before this QMI hardening slice: c3d9f29fc94e81e1828c4c854f46658d2111ec3c. Fetch live main before every write; stored SHAs are historical, not a substitute for a live ref check.
- Frozen backup/pre-plugin-overhaul-2026-11-14: 72f4f157a084f8184b9597ca5bf334b8080e5807; read-only and unchanged.
- main is the only development ref. PR #1 remains closed/unmerged and deeply behind main; do not port it.
- The earlier udhcpc Git tree recovery is closed and disclosed below. Never create a Git Data API commit with an empty base tree.

## Active task

### USB modem QMI status-file trust boundary

get_qmi_handle() previously used fopen() on /tmp/qmi-client-id and /tmp/qmi-handle. That follows a symlink and can block when a local user pre-creates a FIFO. A forged PDH marker can make the router believe a QMI session already exists and refuse WAN startup; a forged WDS CID can affect later teardown.

This source slice adds modem_qmi_status_file.h and replaces the reader with modem_qmi_status_file_read(). It opens with O_NOFOLLOW|O_NONBLOCK|O_CLOEXEC, validates the opened descriptor with fstat (regular file, euid owner, exact mode 0600, nlink 1), reads at most 63 bytes plus one overflow probe, rejects embedded NUL and oversized/malformed content, and resets the output handle to -1 on failure. The parser remains the existing strict decimal modem_qmi_parse_handle(). The writer in trunk/user/uqmi/uqmi-status-file.h already publishes atomically with private staging; the reader now enforces the same trust boundary.

Tests in trunk/user/rc/src/tests/modem_qmi_status_file.c cover valid and malformed numbers, file type/owner/mode/link-count policy, symlinks, nonblocking FIFO rejection, hard links, embedded NUL, oversized input, relative/missing paths and directories. The USB modem workflow compiles the test with GNU89, -Wall -Wextra -Werror, ASan and UBSan and checks the source contract. The new exact-head workflow must pass before this slice is considered validated.

### Wireless radio state — checked reads and truthful notification results

Production code at 046025ca928c54b5fe816af4dfc08fd262c06f5a added checked wireless_radio_power_status(), negative-status propagation, checked manual radio-enable reads, and truthful notification results. Follow-up checker commit c3d9f29fc94e81e1828c4c854f46658d2111ec3c normalized declaration whitespace only; it did not change production code. Exact-head Wireless run #45491054451 was in progress at last poll; inspect before closure.

### Completed BusyBox udhcpc DHCPNAK identity handling during REBINDING

- dhcpc.c passes the current state to the NAK server-ID policy.
- Outside REBINDING, NAKs remain pinned to the selected server; in REBINDING, an identified alternate server's NAK is accepted because the DHCPREQUEST is broadcast. Missing server-ID remains rejected; pre-server-selection compatibility is preserved.
- GNU89 policy tests and exact-source contracts passed in udhcpc run #45489316871 on source SHA 717f983f07434cd78a4ad00ebdb944b8f6d30fa5.

### Git object recovery — disclose and preserve

The initial source commit ff9cb0ec0f9143cd398d8de1d9b4eb13dca87be5 was mistakenly built with an empty tree base and omitted unrelated paths. This was caught by inspecting GitHub commit statistics before claiming validation. Non-forced recovery commit 717f983f07434cd78a4ad00ebdb944b8f6d30fa5 restored the full root tree from the actual parent tree and retained the four intended DHCP files. REST comparison from original main 054ff22bf517a45bf29f2da21963a4206f8b3b83 to 717 confirmed exactly those four paths changed. Do not rewrite or erase intermediate history. For future Git Data API commits, fetch the live head commit's commit.tree.sha and use it as base_tree_sha.

## Verification boundary

### Exact source SHA c3d9f29fc94e81e1828c4c854f46658d2111ec3c

- Wireless radio state regression #45491054451: in progress at last poll after checker whitespace normalization.
- AI handoff documentation #45491054663: in progress at last poll.
- XY-C1 firmware validation #45491054498: in progress at last poll. No L3 claim until Build, TRX verification and artifact upload all pass on this exact source SHA.
- NVRAM transaction boundary audit #45490305501 completed PASS on source SHA 046025ca928c54b5fe816af4dfc08fd262c06f5a (bounded scan only; no global atomicity claim).
- Other exact-head checks on 046 were PASS when last checked. Refresh Actions; never promote a parent-SHA result to a later SHA.

### Earlier exact source SHA 7e700e86e191bfbf11d563fce7f9cfbbcd539516

- XY-C1 firmware #45486634245: Build, TRX verification and artifact upload PASS on this SHA only; L3 is verified for 7e only.
- NVRAM audit #45486634206: PASS as a bounded inventory, not proof of global atomicity.
- No physical-router validation has occurred. Hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live main and its exact-head Actions list. Inspect the USB modem identity/QMI workflow, AI handoff documentation check and XY-C1 Build/TRX/artifact steps triggered by this source slice. Fix any actual FAIL; PENDING must not block another independent task.
2. Verify the QMI status-reader test passed with GNU89, -Wall -Wextra -Werror, ASan/UBSan. If the source contract or runtime regression fails, inspect the exact failing line and make a small follow-up commit.
3. Recheck Wireless #45491054451 on its own source SHA c3d9. If PASS, close the whitespace-only checker follow-up; if FAIL, fix the remaining contract without changing production behavior unnecessarily.
4. Continue repository-wide stability/security work. LAN DHCP/AP-mode lease remains PARTIAL/AUDITING (leasefail, full live lease/NAK transitions, target MIPS/uClibc runtime and physical L4 remain open). Do not repeat the completed NVRAM, udhcpc, VSFTPD or process audits.
5. Before context exhaustion, perform the mandatory handoff checkpoint. Update root docs only when a material fact or first action changes.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session must compare live repository state, exact-head Actions, active source baseline, validation boundary, blockers and first next action against this file before ending. Record a relevant PENDING/FAIL gate that changes state. If all facts and first action remain accurate, explicitly conclude the checkpoint with no file change and no commit. Never create documentation churn solely to synchronize a stored SHA with live main.
