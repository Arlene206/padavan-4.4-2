## Current repository state

# Cross-session execution mandate

Continue one autonomous engineering stream on main. For each logical source/test slice, create all changed blobs, one complete Git tree based on the live main root tree, and one atomic main commit; then update main with a non-forced expected-SHA compare-and-swap. Never force-push, create/use feature branches, make PRs, split one logical change into per-file commits, or modify the frozen backup. A PENDING workflow never blocks independent work.

## Branch and checkpoint

- Source parent before this SCUTClient status-hook slice: 347628c7535656c9d80bb466f8271037e1e24a55. Fetch live main before every write; stored SHAs are historical, not a substitute for a live ref check.
- Frozen backup/pre-plugin-overhaul-2026-11-14: 72f4f157a084f8184b9597ca5bf334b8080e5807; read-only and unchanged.
- main is the only development ref. PR #1 remains closed/unmerged and deeply behind main; do not port it.
- The earlier udhcpc Git tree recovery is closed and disclosed below. Never create a Git Data API commit with an empty base tree.

## Active task

### Active LAN DHCP deconfig retry — treat an already-removed route as complete

A concrete retry deadlock was found in `lan_down_auto()`: it removed the saved DHCP default route with raw `route_del()`, then could fail a later static-interface restore/resolver/hosts step and return to BusyBox's `DECONFIG_PENDING` retry. On the next callback the same route was already absent, so Linux `SIOCDELRT` returns `ESRCH`; without idempotent-delete normalization, deconfig aborted before reaching the remaining cleanup and could repeat forever. The same raw delete existed in the BOUND/RENEW transaction's old-gateway callback.

This slice wraps both LAN DHCP route-delete operations with the existing `route_status_idempotent_delete()`, which maps only `ESRCH` (already absent) to success and preserves real failures. The source contract now asserts both call sites use the helper; the existing route-policy regression already distinguishes `ESRCH` from `EIO`/`EPERM`. Exact-head WAN DHCP callback workflow must confirm the change. Target MIPS/uClibc runtime and physical-router L4 remain unverified.


### SCUT watchdog auth-marker reader trust boundary

The preceding commit 64a2e4b7147f01a85f0068189b94e4ec552ca22b replaced privileged shell redirection in the default SCUTClient -E/-Q hooks with an atomic, mode-0600 status writer. This follow-up closes the reader side: watchcat_auth_flag_enabled() now accepts only a non-symlink regular file owned by the same UID as the watchdog's own PID file, one hard link, exactly two bytes, and no group/other write bits. It validates metadata before reading; sticky /tmp semantics prevent an unprivileged pathname swap after owner validation. Regression cases cover symlink, hard-link and writable-mode spoof markers. A ping executable override gives deterministic, offline test behavior. The exact-head SCUT watchdog workflow must pass before this slice is validated.


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

### Exact source SHA 1708fe61dbed08802f8c5b4ea29126c4ad6b6ca4

- This was a documentation-only handoff checkpoint; no firmware build was triggered on this SHA.
- HTTPD #45495732037 and AI docs #45495731574 PASS on child source tree SHA 7c77568b8148c31604b107972c2d8427e6b04d5d.
- Product-source tests remain tied to 402fc1cc455490c9c678be28966bf30f5597e1e3; XY-C1 #45495287536 remained in Build XY-C1 at last poll.

### Exact source SHA 7c77568b8148c31604b107972c2d8427e6b04d5d

- HTTPD Authentication and Security Regression #45495732037: PASS; this run validates the corrected duplicate LAN DHCP source contract.
- AI handoff documentation contract #45495731574: PASS.
- No firmware build was triggered for this workflow/doc-only child; the product-source validation remains tied to 402fc1cc455490c9c678be28966bf30f5597e1e3.

### Exact source SHA 402fc1cc455490c9c678be28966bf30f5597e1e3

- BusyBox udhcpc event-script status #45495287614: PASS.
- WAN DHCP callback state stability #45495287778: PASS (includes the LAN lease callback source/test contract).
- AI handoff documentation #45495287791: PASS.
- HTTPD Authentication and Security #45495287685: FAIL due stale duplicated assertion expecting the old LAN lease parser; corrected in the follow-up workflow commit.
- XY-C1 Firmware Validation #45495287536: Build XY-C1 IN_PROGRESS; TRX/artifact pending; no L3 claim.
- Other exact-head passes: IPv6 service-type #45495287907, detect_link #45495288207, Wireless #45495287862, watchdog lifecycle #45495287837, detect_internet #45495287792, Firewall startup #45495287999, WAN route #45495287572, DNSMasq #45495287781, Process inventory/fail-closed #45495287558 and lifecycle inventory #45495287737, USB storage #45495287972.
- NVRAM transaction boundary #45495287577 was still IN_PROGRESS at last poll.
- No physical-router validation; hardware L4 NOT VERIFIED.

### Exact source SHA 2a54ebbc7030fb69d2d60ac144a3fbc76a043f22

- SCUT watchdog auth-marker/stat regression #45494687100: PASS on this exact SHA.
- AI handoff documentation contract #45494687184: PASS on this exact SHA.
- Process lifecycle inventory #45494687146: IN_PROGRESS at last poll.
- XY-C1 Firmware Validation #45494687091: cross-build job IN_PROGRESS before product-input detector; no L3 claim.
- No physical-router validation; hardware L4 remains NOT VERIFIED.

### SCUT watchdog marker reader follow-up on parent 0d9b3b51d174ef281a5a379c12c59b29811b9734

- Parent run #45494178433: shell syntax, watchdog lifecycle regression (including symlink/hardlink/writable-mode marker spoof tests), and atomic status writer regression all PASS; only the final source-contract step failed because assertion 58 omitted the actual stat command's stderr redirection.
- Commit 0d9b3b51d174ef281a5a379c12c59b29811b9734 corrected that assertion. AI handoff validation #45494472680 PASS on 0d9. The SCUT workflow did not start on this checker-only push, so the next source-path-triggered exact-head SCUT run must confirm the corrected contract.
- XY-C1 build run #45494178359 remained in progress at last poll (build step 38); TRX and artifact upload were not complete. No L3 claim.



### SCUT watchdog auth-marker reader follow-up (parent SHA 495da8f95041bee67025fe52ede7cba526b133c9)

- Exact-head run #45494178433 failed only in the final Python source-contract block at assertion 58: the static expectation omitted the intentional 2>/dev/null on the stat call. The actual reader code is correct.
- The exact-head runtime test steps all passed on this SHA, including the extended watchdog lifecycle suite with symlink, hard-link and writable-mode spoof cases. Shell syntax and the prior atomic status writer test also passed.
- Follow-up commit updates only the contract literal to match the source. AI handoff #45494178404 PASS; Process inventory #45494178432 PASS. XY-C1 #45494178359 finished host-side preparation but the production-input detector and build path had not yet run at last poll; recheck on this SHA rather than attributing results across commits.


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

1. Verify the LAN DHCP deconfig-route idempotency fix in exact-head WAN DHCP callback state stability; inspect the route helper regression and source checker. Any failure must be tied to this commit SHA and fixed atomically.
2. Recheck XY-C1 Firmware Validation #45495287536 on source SHA 402fc1cc455490c9c678be28966bf30f5597e1e3: Build XY-C1 was active at last poll; TRX and artifact upload still need separate confirmation. No L3 until all three PASS on that SHA.
3. Continue LAN DHCP/AP-mode recovery edge cases after the route fix; do not reopen the closed DHCPNAK REBINDING identity fix. Target MIPS/uClibc runtime and physical-router L4 remain open.
4. DHCP lease parity already passed udhcpc #45495287614 and WAN callback #45495287778 on SHA 402fc; corrected HTTPD #45495732037 and AI docs #45495731574 passed on SHA 7c77568b8148c31604b107972c2d8427e6b04d5d. Do not repeat completed tests absent changed source.
5. Before context exhaustion, refresh main/backup refs and exact-head Actions, then checkpoint the exact commit SHA and first actionable task.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session must compare live repository state, exact-head Actions, source baseline, validation boundary, blockers and the first next action against this file. Record a newly resolved relevant PENDING/FAIL gate. If all facts and the first action remain accurate, explicitly conclude the checkpoint with no file change and no commit. Never create documentation churn solely to synchronize a stored SHA with live main.
