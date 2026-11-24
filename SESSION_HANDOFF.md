## Current repository state

# Cross-session execution mandate

Continue one autonomous engineering stream on main. For each logical source/test slice, create all changed blobs, one complete Git tree based on the live main root tree, and one atomic main commit; then update main with a non-forced expected-SHA compare-and-swap. Never force-push, create/use feature branches, make PRs, split one logical change into per-file commits, or modify the frozen backup. A PENDING workflow never blocks independent work.

## Branch and checkpoint

- Source parent before this SCUTClient status-hook slice: 347628c7535656c9d80bb466f8271037e1e24a55. Fetch live main before every write; stored SHAs are historical, not a substitute for a live ref check.
- Frozen backup/pre-plugin-overhaul-2026-11-14: 72f4f157a084f8184b9597ca5bf334b8080e5807; read-only and unchanged.
- main is the only development ref. PR #1 remains closed/unmerged and deeply behind main; do not port it.
- The earlier udhcpc Git tree recovery is closed and disclosed below. Never create a Git Data API commit with an empty base tree.

## Active task

### SCUTClient auth-status hook — prevent privileged /tmp symlink writes

Default -E and -Q event hooks in trunk/user/scutclient/scripts/scutclient.sh previously used shell redirection directly to /tmp/scutclient_status. Since the SCUTClient daemon is launched by the privileged service wrapper, a local user could pre-create a symlink at that shared /tmp path and redirect the privileged shell write to an unrelated file.

This slice adds trunk/user/scutclient/scripts/scutclient_status_file.sh: it validates status 0/1 and an absolute path, stages in the same directory under umask 077 using mktemp, enforces mode 0600 and publishes with mv/rename. A pre-existing file symlink is replaced as a directory entry rather than followed; directory targets are rejected. Default success/failure hooks now invoke /usr/bin/scutclient_status_file.sh 0 and /usr/bin/scutclient_status_file.sh 1. Explicit user-supplied NVRAM hooks remain unchanged. The SCUTClient Makefile already installs scripts/ wholesale into /usr/bin.

Regression trunk/user/scripts/tests/scutclient_status_file_atomic.sh covers normal publication, exact 0600 mode and single link, symlink target preservation, hard-link inode preservation, directory targets, invalid arguments, relative destination and temporary-file cleanup. The SCUT watchdog safety workflow runs sh -n, the fixture test and static contract checks. The test passed locally with /bin/sh before commit; exact-head Actions must pass before this slice is validated.

### Completed USB modem QMI status-file trust boundary

Commit 347628c7535656c9d80bb466f8271037e1e24a55 adds a descriptor-based trusted reader for /tmp/qmi-client-id and /tmp/qmi-handle: O_NOFOLLOW|O_NONBLOCK|O_CLOEXEC, fstat requirements (regular file, euid owner, exact 0600, link count 1), bounded binary-safe read, NUL/oversize rejection, strict numeric parser and output reset on error. The old fopen reader is gone. Exact-head USB modem workflow #45491981121 PASS, including GNU89 -Wall -Wextra -Werror, ASan/UBSan reader test and source contract.

### Completed Wireless state error propagation

Production source commit 046025ca928c54b5fe816af4dfc08fd262c06f5a added checked wireless_radio_power_status(), negative-status propagation, checked manual radio-enable reads and truthful notification results. Follow-up checker commit c3d9f29fc94e81e1828c4c854f46658d2111ec3c normalized header declaration whitespace only. Exact-head Wireless #45491054451 and AI docs #45491054663 PASS on c3d9.

### Completed BusyBox udhcpc DHCPNAK identity handling during REBINDING

dhcpc.c passes the client state to the server-ID filter. Outside REBINDING, NAKs remain pinned to the selected server; during REBINDING the DHCPREQUEST is broadcast, so an identified alternate server's NAK is accepted. Missing server-ID remains rejected and pre-server-selection behavior is preserved. GNU89/ASan/UBSan and exact-source contracts PASS in udhcpc workflow #45489316871 on SHA 717f983f07434cd78a4ad00ebdb944b8f6d30fa5.

### Git object recovery — disclose and preserve

The initial source commit ff9cb0ec0f9143cd398d8de1d9b4eb13dca87be5 was mistakenly built with an empty tree base and omitted unrelated paths. This was detected by inspecting GitHub commit stats before claiming validation. Non-forced recovery commit 717f983f07434cd78a4ad00ebdb944b8f6d30fa5 restored the complete root tree from the actual parent and retained the four intended DHCP files. REST compare from original main 054ff22bf517a45bf29f2da21963a4206f8b3b83 to 717 confirmed exactly those four paths changed. Do not rewrite or erase intermediate history. For future Git Data API commits, fetch the live main commit's commit.tree.sha and use it as base_tree_sha.

## Verification boundary

### Exact source SHA 347628c7535656c9d80bb466f8271037e1e24a55

- USB modem QMI identity regression #45491981121: PASS; new secure-reader C test, source contract and all prior USB modem tests passed.
- AI handoff docs #45491981194: PASS; part-033 archive validator passed.
- NVRAM transaction boundary audit #45491981203: PASS (bounded audit only; not proof of global atomicity).
- Process inventory #45491981328 and #45491981119: PASS.
- WAN teardown #45491981147, WAN DHCP callback #45491981235, detect_link signal lifecycle #45491981246 and HTTPD auth/security #45491981225: PASS.
- XY-C1 Firmware Validation #45491981249: at last poll cross-build job was IN_PROGRESS; Build XY-C1 step 38 active; TRX step 39 and artifact upload step 40 pending. Do not claim L3 until Build, TRX verification and artifact upload all PASS on this exact SHA.
- No physical-router validation has occurred; hardware L4 remains NOT VERIFIED.

### Earlier exact source SHA 7e700e86e191bfbf11d563fce7f9cfbbcd539516

- XY-C1 firmware #45486634245: Build, TRX verification and artifact upload PASS on that SHA only; L3 is verified for 7e only.
- NVRAM audit #45486634206: PASS as a bounded inventory, not proof of global atomicity.

## Exact first action for the next GPT

1. Fetch the live main SHA and exact-head Actions. Inspect the new SCUT watchdog/status-hook regression and new XY-C1 run triggered by this commit. The prior 347628 XY build was still in progress; never attribute its result to a later SHA.
2. Confirm the new status-hook regression's symlink/hardlink/directory cases pass on the exact commit SHA. If any fail, diagnose the specific command and correct in one atomic source/test commit.
3. Recheck XY-C1 Build, TRX and artifact upload separately. L3 must be tied to one exact source SHA; PENDING must not block other work.
4. Continue repository-wide security/stability work. LAN DHCP/AP-mode remains PARTIAL/AUDITING (leasefail, full live transitions, target MIPS/uClibc runtime, physical L4 open). Do not repeat completed QMI, Wireless, udhcpc, NVRAM, process, WAN teardown or HTTPD audits.
5. Before context exhaustion, perform the mandatory handoff checkpoint; update root docs only if a material validation result, blocker or first next action changes.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session must compare live repository state, exact-head Actions, source baseline, validation boundary, blockers and the first next action against this file. Record a newly resolved relevant PENDING/FAIL gate. If all facts and the first action remain accurate, explicitly conclude the checkpoint with no file change and no commit. Never create documentation churn solely to synchronize a stored SHA with live main.
