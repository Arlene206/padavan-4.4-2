## 2026-11-24 — Align MiniUPnP SOAP policy workflow marker with the expanded regression

- Exact MiniUPnP run #45404676427 on `e1fb2a714b8bbe225294d8f58d8996c7750658c2` passed the GNU89 ASan/UBSan SOAP input regression and the new DeletePortMappingRange source-order contract. The enclosing inline workflow contract still expected the old banner `MiniUPnP SOAP unsigned input policy: PASS`; update it to the test's expanded `...unsigned input and range-delete result policy...` banner so integrated CI checks the added behavior instead of rejecting a valid regression result.
- Exact AI handoff contract #45404676277 on the same SHA passed after the rolling-log restoration. No product C source changed in this workflow-contract correction.

---

## 2026-11-24 — Correct MiniUPnP range-delete ordering contract and rotate the dev log

- Exact MiniUPnP run #45403990490 on `f12d4d58c6cff4370deb62f3e12f4695bf07a976` compiled and passed the GNU89 ASan/UBSan SOAP policy regression, then failed the Python source contract: its first `free(port_list)` match was the empty-range early-return cleanup, not the release after the deletion loop. The checker now searches for the post-loop free starting after the per-result recording call.
- Exact AI handoff run #45403990489 on the same SHA found 120 root log entries. This commit archives the 100 oldest complete records verbatim in `part-032.md`, retains the newest 20 plus this new record (21 total), and updates the archive index. No product source changed.

---

## 2026-11-24 — Repair MiniUPnP range-delete contract and restore batched history

- MiniUPnP SOAP range-delete policy C test compiled and passed under GNU89 + ASan/UBSan on c2dc6d3. The first exact run #45403376487 stopped at the new Python source checker because a raw-string escape produced an invalid Python quote sequence; this follow-up fixes the checker literal.
- AI handoff run #45403376294 exposed that a previous dev-log update had fetched only a short line range and replaced the 119-entry archive with four entries. Restored the complete log from the preceding full-history commit and added this current validation record; the documentation contract requires at least 20 complete newest entries.
- Process lifecycle checker correction commit 364383be7889a9339887c1b4f04ce9ff9a273a10 passed exact Process lifecycle run #45402861054. The follow-on Process inventory run #45403376647 on c2dc6d3 also passed.
- XUPnP pair product-source commit eb0fbb1f3d858876e754efc2815c3880b74f764a has exact XY-C1 workflow #45401700209 PASS for firmware build, TRX verification and artifact upload. L3 is complete for that source slice only; physical-router L4 remains unverified.

---

## 2026-11-24 — Align lifecycle integration checks with staged XUPnP pair

- Exact Process lifecycle run #45401700271 on `eb0fbb1f3d858876e754efc2815c3880b74f764a` passed the XUPnPD storage-directory and file-safety C regression plus multiple other daemon regressions, then failed only because `check_process_lifecycle_integration.py` still required two calls using `tmp2/tmp1` and sequential `rename(tmp1,tmp2)`. The product code now stages `common_path/common_stage` and `runtime_path/runtime_stage`, then calls `dnsmasq_file_publish_pair()`.
- Updated the lifecycle checker to require the two distinct staged paths, both source inputs and the rollback-capable publisher, and to reject the old sequential-rename contract. This is a validator-only fix.
- The separate DNSMasq lifecycle checker was also fixed in `2fd4c74` after #45401700240 exposed a raw-versus-compacted offset comparison bug. Both exact-head workflows must rerun; no new product source change in these checker corrections.

---

## 2026-11-24 — Fix XUPnP source-checker offset comparison

- Exact DNSMasq lifecycle run #45401700240 on source SHA `eb0fbb1f3d858876e754efc2815c3880b74f764a` passed the GNU89 ASan/UBSan DNSMasq file regression and failed the XUPnP source-order checker. The checker compared character offsets from un-compacted `body` text to offsets from whitespace-compacted `compact` text; it therefore rejected correctly ordered fsync/NVRAM operations.
- Corrected all four relative-order comparisons to use the same compact source string. No production C or test code changed. Exact-head DNSMasq and documentation checks must rerun on this correction SHA.

---

## 2026-11-24 — Publish XUPnP Lua configuration as a rollback-capable pair

- `start_xupnpd()` previously renamed staged `common.lua` live before preparing `xupnpd.lua`. A later source/NVRAM/write/fsync failure could leave a mixed config generation while startup returned an error.
- Both files are now fully read, rendered, flushed and fsynced before either destination changes. `dnsmasq_file_publish_pair()` snapshots both old paths and restores the first if the second rename fails. A preserve-mode sentinel retains each tempfile's preselected destination mode.
- Added GNU89 ASan/UBSan regression for distinct mode retention and forced second-rename rollback, plus a Python source-order contract. DNSMasq lifecycle now watches `net.c` and runs the new contract after the pair regression.
- Parent `aae46a6`: udhcpc regression/source contract, AI handoff and Process lifecycle PASS; USB storage and NVRAM audit pending at last poll. XY-C1 for production `4015869` still needed Build/TRX/artifact confirmation.

---

## 2026-11-24 — Fix stale DHCPDECLINE regression invocation

- Exact SHA `4015869` udhcpc workflow #45400399177 failed GNU89 compilation: the regression was renamed to `test_failed_packet_send_blocks_dhcp_reselection_until_retry()`, but `main()` still invoked the removed `test_failed_dhcpdecline_blocks_reselection_until_retry()` name. This also triggered an unused-function error for the new definition.
- Updated the test runner to invoke the existing renamed function. Production DHCP code and source-contract assertions were already aligned; this is a regression-harness compile fix.
- On parent `4015869`, AI handoff #45400399146 and Process lifecycle #45400399189 PASS. USB storage, NVRAM audit, XY-C1 and Process inventory were pending at the last poll.

---

## 2026-11-24 — Honor sendto byte-count semantics in DHCP retries

- Review of BusyBox packet.c confirmed udhcp_send_raw_packet() and udhcp_send_kernel_packet() return sendto()/safe_write() byte counts on success and -1 on failure. The new DHCPDECLINE retry predicate initially treated every nonzero result as an error, which would have retried successful transmissions forever. The recent perform_release() diagnostic similarly mislabeled successful DHCPRELEASE sends as failures.
- Replaced the predicate with udhcpc_packet_send_failed(status < 0), using it for both DECLINE retries and RELEASE logging. Regression now covers positive byte-count success, zero success and negative failure; source checker pins the release diagnostic to the same convention. This is a correctness repair to the DHCPDECLINE slice, not merely a wording change.
- Exact SHA 4ca919b's first udhcpc run passed GNU89/ASan/UBSan but failed the Python contract on its outdated literal INIT_SELECTING assertion. AI docs also flagged the handoff checkpoint wording; both validator fixes were committed in 02ddda7 and have exact-head reruns queued at last poll.

---
## 2026-11-24 — Repair exact-head DHCPDECLINE and handoff validator failures

- Exact push SHA 4ca919b compiled and passed the GNU89 ASan/UBSan udhcpc policy binary, but its Python source contract retained one old assertion requiring a literal INIT_SELECTING argument after the ARP-conflict retry path became a status-dependent resume_state. Updated the contract to check the new resume_state call.
- Exact AI docs workflow also caught that SESSION_HANDOFF.md used equivalent but non-canonical checkpoint wording and omitted the required exact phrase “one atomic main commit.” Restored those mandatory handoff contract strings. Both failures were validator-only; DHCPDECLINE product code remains unchanged in this correction.

---
## 2026-11-24 — Retry failed DHCPDECLINE before requesting another address

- ARP conflict handling ignored the return value from send_decline(). It immediately restarted DHCP selection even when DHCPDECLINE could not be sent, so the server could offer the same conflicting address again; the source violated the required decline-before-restart sequence.
- Added an ARPING-only DECLINE_PENDING state retaining the original server and address. A failed send blocks discovery and retries after a bounded delay. If old-lease deconfiguration also fails, DECONFIG_PENDING resumes into DECLINE_PENDING only after cleanup succeeds. Successful send then returns to INIT_SELECTING. GNU89 policy regression covers send status and retry-delay floor; the Python source contract pins saved tuple, cleanup ordering and retry-before-rediscovery.
- Parent 94960ce: exact BusyBox udhcpc regression/source contract #45398493359 PASS; AI docs #45398493347 was pending at last check. Product-source XY-C1 #45398034078 is building on SHA 694732a; TRX/artifact pending. QMI firmware L3 is confirmed on b05eae9: #45396493118 Build, TRX verification and artifact upload all SUCCESS.

---
## 2026-11-24 — Correct udhcpc pending-release source contract

- Exact SHA 694732a passed the GNU89 ASan/UBSan state-policy binary, but the Python checker failed one stale assertion: it still expected the SIGUSR2 branch to assign deconfig_resume_state = RELEASED directly. The production code now correctly routes that path through perform_release() and udhcpc_schedule_deconfig_retry(RELEASED, 0, ...), so the source contract was updated to pin the shared routine and retry-before-continue order rather than the obsolete assignment.
- This is a checker-only correction; no product source or test semantics changed. Exact-head rerun pending. Parent XY-C1 #45398034078 is still building with TRX/artifact pending; Process inventory #45398034058, umbrella Process #45398034079, USB storage #45398034361 and NVRAM #45398034033 were in progress at last poll; AI docs #45398034008 PASS.

---
## 2026-11-24 — Unify pending DHCP release and -R exit handling

- Follow-up state-path review found that the initial SIGUSR2 pending-state repair bypassed `perform_release()`; a daemon exiting through `-R` while the same deconfig was pending could still skip DHCPRELEASE. The release routine now recognizes the saved “lease still active” bit while `DECONFIG_PENDING`, and the signal handler reuses that routine instead of duplicating packet logic.
- Normal BOUND/RENEWING/REBINDING/RENEW_REQUESTED release behavior is preserved. Pending NAK/expiry/ARP cleanup and an already-sent release do not emit duplicates; pending release send errors are now logged. Source contracts pin the shared SIGUSR2/`-R` path.
- Parent `afba6d0005263387d20539f6719c1befbe8b223b`: udhcpc GNU89 ASan/UBSan and Python source contract #45397570960 PASS; AI docs #45397570943 PASS. XY-C1 #45397571031 still pending at cache/build preparation; Process inventory #45397571017 PASS; USB storage #45397570936 and NVRAM #45397571009 in progress. L3 remains pending.

---

## 2026-11-24 — Honor DHCPRELEASE during pending manual-renew cleanup

- Found a state-machine gap in BusyBox udhcpc: a repeated SIGUSR1 while RENEW_REQUESTED can fail its deconfig callback and enter DECONFIG_PENDING with the prior lease still active. A subsequent SIGUSR2 only changed the resume state to RELEASED; it skipped the DHCPRELEASE packet that the normal perform_release() path sends.
- Carry a narrowly scoped “prior lease still active” bit only from that manual-renew failure path. SIGUSR2 sends DHCPRELEASE before resuming pending cleanup; NAK, lease expiry, ARP conflict and already-sent release paths do not send a duplicate. Clear the bit on resume. Added GNU89 policy tests and source-order/call-site checks.
- QMI teardown fix on exact SHA `b05eae9`: USB modem #45396493190, Process inventory #45396493113/#45396492751, AI docs #45396493061, NVRAM #45396492703, WAN DHCP #45396492710, WAN teardown #45396492829, detect-link #45396493228 and HTTPD #45396492941 PASS. XY-C1 #45396493118 remains in Build; TRX/artifact pending. No L3 claim; physical-router L4 unverified.

---

## 2026-11-24 — Preserve QMI WDS ownership when PDH cleanup fails

- After `--stop-network` succeeded, failure to unlink `QMI_HANDLE_PDH` only set an error flag; teardown still released the WDS client ID and removed its marker. The stale PDH then made future starts fail with `EBUSY`, while the released CID was no longer reusable.
- Gate CID release on successful or already-absent PDH cleanup. Any other unlink failure now returns immediately, preserving the WDS ID and marker for recovery. Added GNU89 policy regression and a source-order contract; the existing USB modem workflow already includes the source/test/checker paths.
- Parent `0dbf3d2f49133f6cbd471d7b6ba4efe633e5bb96` adds the checked Ralink regulatory-SKU read. Its AI docs, Process inventory and wireless-radio-state workflows PASS; NVRAM audit and XY-C1 remain in progress (XY build/TRX/artifact not yet completed). New QMI exact-head Actions are pending; no L3 claim. Physical modem L4 remains unverified.

---

## 2026-11-24 — Preserve Ralink regulatory SKU selection errors

- `symlink_sku_file()` used `nvram_safe_get("regspec_code")`, masking a missing key and an NVRAM backend failure as the same empty value. A transient read error could silently select a different regulatory SKU.
- Switched to `nvram_get_checked()` and explicit status validation: missing optional key still falls through, present values remain table-validated, and backend/API errors abort config generation. Added GNU89 regression coverage for present/missing/error/invalid API states and a workflow source-order contract.
- Parent `5a7b2c4`: WAN route regression, AI docs, Process inventory and NVRAM audit PASS; XY-C1 Build remains in progress with TRX/artifact pending.

---

## 2026-11-24 — Reject whitespace and signs in the SYN-cookie selector

- Found that `strtol()` accepts leading whitespace and optional signs, so malformed `fw_syn_cook` values such as `" 0"` and `"-0"` could be mistaken for an explicit disable.
- Added a first-byte decimal-digit guard before `strtol()`; complete-token, boolean-domain and overflow checks remain intact. Regression adds space/tab/newline and +/- cases; the source checker verifies validation occurs before parsing.
- Parent `aa3d55b`: VSFTPD #45393626354, AI #45393626335, Process #45393626312 and NVRAM #45393626295 PASS. XY-C1 #45393626376 Build in progress, TRX/artifact pending. No L3 claim for recent VSFTPD source fixes yet.

---

## 2026-11-24 — Close stale FTP data endpoints on EPSV ALL

- `handle_pasv()` accepted `EPSV ALL` by setting `epsv_all` and returning before active/passive cleanup. Existing `PORT`/`EPRT` destination or a `PASV` listener survived; later transfer code could consume that endpoint although new non-EPSV setup commands were blocked.
- The branch now calls `pasv_cleanup()` and `port_cleanup()` before setting EPSV-only state or sending success. Passive cleanup is idempotent in the privilege operation; active cleanup clears the stored sockaddr.
- Extended the VSFTPD stability source contract to require each cleanup once and before the state flag, 229 reply and return. Both VSFTPD and XY-C1 track `postlogin.c`; unknown production paths trigger full firmware validation.
- Parent `5a7500e`: VSFTPD #45392889702, AI #45392889910 and Process #45392889923 PASS; NVRAM #45392889664 pending and XY-C1 #45392889756 building. Exact-head verification required; physical L4 unverified.

---

## 2026-11-24 — Fail closed on truncated VSFTPD hidden-share marker paths

- Found `test_if_hidden_share()` building `<mount>/.hidden` in a 256-byte static array without checking truncation; a valid long mount component could make `stat()` miss the marker and expose a hidden share.
- Reused `ftp_acl_path_build_child_path()`, expanded the marker buffer to `PATH_MAX`, and fail closed on construction errors. Added normal, 255-byte component, undersized-output, full-path and invalid-leaf regression cases; the VSFTPD workflow checks production ordering.
- Concurrent checkpoint on parent `18af4fb`: detect-link, AI docs, Process, USB modem, HTTPD auth and NVRAM PASS; XY-C1 Build is still running with TRX/artifact pending. No firmware L3 claim.

---

## 2026-11-24 — Preserve UNKNOWN WAN link state until a valid PHY sample

- `detect_link.c` initialized `dl_status_wan=0` and `dl_status_wan_nvram_old=-1`. A failed first switch PHY query correctly left the cached value unchanged and scheduled retry, but the same timer tick still called `dl_handle_link_wan()`, which published the synthetic zero to `link_wan` before any successful hardware sample.
- Added a tri-state known flag updated only by nonnegative PHY results. The WAN transition/NVRAM handler is deferred until that first valid sample; after it becomes known, subsequent query errors continue retaining the last valid link state while retrying. Independent WISP/LAN/USB handling remains active.
- Added GNU89/`-Werror`/ASan/UBSan policy coverage for unknown retention on negative results, valid link-down/up initialization and preserving known state through later read errors. The detect-link workflow adds an exact source-order/handler-gating contract.
- Prior WAN source PHY selector production SHA `d3733ec` is L3 verified: XY-C1 #45390205170 completed Build/TRX/artifact PASS; host and documentation corrections on test/docs-only descendant `c954e19` passed detect-link #45390666226, AI #45390666278, Process #45390666391 and NVRAM #45390666354. This new product-source change needs fresh exact-head firmware validation; physical L4 remains unverified.

---

## 2026-11-24 — Correct WAN selector test contract and handoff size

- Exact SHA `d3733ecbec2e0cf652c00eadf1db4645ae5ff13e`: detect-link run #45390205127 compiled and passed the new GNU89 ASan/UBSan selector regression, but the final Python contract expected direct test calls. This repository's test main wraps each case in `CHECK(test() == 0)`; THIS FOLLOW-UP matches that convention and fixes the test diagnostic newline.
- AI docs run #45390205125 caught `REFACTOR_ROADMAP.md` at 131111 bytes, 39 bytes above the 131072-byte hard limit. THIS FOLLOW-UP compresses the new priority entries and verifies the encoded byte count below the ceiling.
- XY-C1 #45390205170 was still building at last poll (TRX/artifact pending). Exact-head reruns are required; no L3 claim is made for the WAN selector yet.

---

## 2026-11-24 — Reject unsupported persisted WAN source PHY selectors

- `get_wan_ether_link_direct()` used a checked NVRAM read for `wan_src_phy` but passed all nonnegative integers to the switch UAPI. In the MTK kernel, `get_port_from_uapi()` returns a sentinel for unsupported IDs and `esw_status_link_port_uapi()` translates that to success with link=0. A corrupt selector could therefore be mistaken for an actual Ethernet link-down, allowing detect_link to publish a false WAN disconnect and run teardown logic.
- Added a range policy for the generic external switch UAPI: WAN=0 and LAN1..LAN5=1..5 are accepted; legacy negative values still fall back to WAN=0; unknown/CPU port IDs above 5 fail with `ERANGE`. The userland caller rejects before issuing the ioctl and returns `-1` with errno, preserving the last known link state and retry path.
- Added GNU89/`-Werror`/ASan/UBSan tests for 0..5, negative compatibility including INT_MIN, and invalid/CPU IDs 6/13/14/15/INT_MAX with no output mutation. The detect-link workflow checks policy ordering and path coverage; XY-C1 path filters include the production caller, helper and test.
- Parent exact SHA `659b91b`: HTTPD lifecycle #45388114470, AI docs #45388114468, Process lifecycle #45388114507, and XY-C1 #45388114515 all PASS, including Build/TRX/artifact. New exact-head CI results must be checked on this commit. Physical-router L4 remains unverified.

---

## 2026-11-24 — Correct HTTPD selector contract and ensure XY-C1 build coverage

- On exact SHA `86d3078c1ead2dfd5578830e069b684e12a4b072`, HTTPD run #45387713984 compiled and passed the GNU89 ASan/UBSan policy regression, then failed its Python source contract. The new assertion used escaped backslash-n/tab text rather than the C source's actual whitespace; production code was not implicated.
- The same SHA had no XY-C1 push run because `.github/workflows/xy-c1-firmware-validation.yml` did not watch `trunk/user/rc/src/httpd_daemon_policy.h`, despite that header being included by the firmware's `services.c`.
- THIS FOLLOW-UP normalizes whitespace before the source assertion and adds the header to both XY-C1 push and pull-request path filters. Exact-head HTTPD, XY-C1 Build/TRX/artifact and AI/Process/NVRAM results must be checked on THIS COMMIT.

---

