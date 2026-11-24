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

## 2026-11-24 — Reject invalid HTTPD listener protocol selectors

- The Advanced Services UI offers only `http_proto=0` (HTTP), `1` (HTTPS), and `2` (HTTP+HTTPS). In HTTPS-enabled builds, `httpd_start_config_read()` accepted any signed integer. An invalid value selected neither listener branch, but the startup path could still launch `httpd` without explicit listener arguments while firewall preflight treated the invalid selector as exposing no listener.
- Added an early `EINVAL` domain check immediately after the checked selector read. Invalid values fail before protocol-dependent settings, port normalization, stale-state publication, or daemon launch; HTTP-only builds preserve their existing HTTP default and skip the selector read.
- Added GNU89 ASan/UBSan regression cases for `-1`, `3`, and `99`, asserting exactly one read and that the configuration snapshot remains unchanged. The HTTPD daemon lifecycle workflow now asserts the range guard's ordering and the new test invocation.
- This source slice is published in THIS COMMIT; exact-head HTTPD lifecycle, AI handoff and XY-C1 Build/TRX/artifact Actions must be checked live. L4 hardware behavior remains unverified.

---

## 2026-11-24 — Fix APCLI deauthentication allocation leak and index validation order

- MT76x2 and MT7915 APCLI deauthentication handlers called `os_alloc_mem()` twice for the same `pInfo`, overwriting the first pointer and leaking it. Both handlers also dereferenced `ApCliTab[ifIndex]` / `StaCfg[ifIndex]` before checking `ifIndex >= MAX_APCLI_NUM`.
- Removed the duplicate allocation and moved the bounds check immediately after reading `ioctl_if`, before any APCLI table dereference. Invalid indices now set `-EINVAL`, free the queue element and exit the switch case. The single remaining allocation retains its `-ENOMEM` and `-EFAULT` cleanup.
- Extended the focused MTK source contract to require exactly one APCLI request allocation, bounds-check-before-table-access and allocation-check-before-usercopy. Exact-head Actions are pending.

---

## 2026-11-24 — Fail closed on MTK AP deauthentication ioctl usercopy errors

- Audited `OID_802_11_DEAUTHENTICATION` in MT76x2, MT76x3, MT7615 and MT7915. Each path checked the nominal structure length but consumed the request after ignoring or merely storing the nonzero `copy_from_user()` result. A fault/partial copy could therefore feed uninitialized or partially initialized address/reason fields into station lookup, MLME enqueue, or APCLI link-state changes.
- Added explicit `-EFAULT` exits before any request-field use or deauth side effect, and release the allocated `MLME_QUEUE_ELEM` / APCLI request buffer on failure. MT76x2 and MT7915 APCLI paths now also check their separately allocated request pointer and return `-ENOMEM` before copying if allocation fails.
- Extended the focused MTK source-contract regression to require exact request sizes, checked usercopies, failure cleanup, no use of request fields before a successful copy, and APCLI allocation guards. The MTK workflow already watches all four source files and this checker.
- On the base source SHA `0dc7eaed5c1eb2623f5762c9f9b6fb7b93c9a4d0`, exact Actions MTK #45381270549, AI handoff #45381270437, Process #45381270630 and NVRAM #45381270585 PASS. XY-C1 #45381270533 remained in Build XY-C1; TRX/artifact pending. This deauth fix needs a fresh exact-SHA XY-C1 build/TRX/artifact run; no L3 claim is made.

---

## 2026-11-24 — Bound MT7615/MT7915 WSC PSK copies and remove credential logs

- Extended review of the same AP ioctl dispatchers found an unbounded WSC usercopy in MT7615 and MT7915. The ordinary security PSK field is 65 bytes, but `WSC_CTRL.WpaPsk` is only 64; the code still copied the original caller length even after the main PSK branch rejected lengths >=65. This was a heap/struct-adjacent kernel memory overwrite risk.
- Reworked both handlers to reject lengths >64 before copy, stage exactly one usercopy in a zeroed 65-byte temporary, return `-EFAULT` before changing security/WSC state, then clear and publish the validated key to the 65-byte terminated security field and the 64-byte WSC field. WSC length is updated only after the bounded copy.
- MT76x3 now uses the shared 64-byte predicate too. Removed plaintext PSK `%s` diagnostics from MT76x3/MT7615/MT7915; only lengths remain in logs.
- Added GNU89 ASan/UBSan boundary fixture and source contracts for maximum 64, overlength, UINT_MAX, checked-copy ordering, destination sizes, and secret-log absence. Exact focused/AI/XY-C1/NVRAM Actions are pending.
- Concurrent main checkpoint `8f639815fa60bcfa9a44514832ad895d36dd5a68` also added fail-closed handling for AuthMode/WepStatus/SSID exact-size copies in all four AP driver families and MT76x2/MT7915 APCLI 802.1X Boolean setters. Exact MTK #45380653108 and AI #45380653126 PASS; XY #45380653170 and NVRAM #45380653441 were in progress at last observation.

---

## 2026-11-24 — Fail closed on MTK AP security-selector usercopy faults

- A follow-up review of the four MTK AP ioctl dispatchers found that exact-size copies for `AuthMode`, `WepStatus` and `Ssid` still proceeded after `copy_from_user()` returned a nonzero uncopied-byte count. That allowed partially initialized stack values to influence authentication, cipher and SSID state.
- The MT76x2 and MT7915 APCLI `OID_802_11_SET_IEEE8021X` and `OID_802_11_SET_IEEE8021X_REQUIRE_KEY` setters had the same issue: they published Boolean fields regardless of usercopy failure.
- All these paths now map copy faults to `-EFAULT` and leave the switch case before consuming or publishing the input. The MTK source-contract regression now requires the exact-length guard, checked copy, EFAULT exit and successful-copy gate across all ten handler paths.
- GNU89 ASan/UBSan R0KH-ID boundary regression remains part of the same focused workflow. Exact-head MTK, AI handoff and XY-C1 Build/TRX/artifact verification are pending.

---

## 2026-11-24 19:38 Beijing — Correct R0KH-ID source-contract formatting

- Exact MTK run #45379089623 on `b04620659840fd488a0320543d8e92872f9821f2` passed the GNU89 ASan/UBSan boundary fixture and the vendor-header byte matcher, then exposed a whitespace mismatch in the checker’s expected normalized-call string: the source places a newline immediately after the function's opening parenthesis.
- Updated the source contract to match the actual collapsed whitespace; no production code changed. Exact MTK and AI handoff reruns are pending.

---

## 2026-11-24 19:31 Beijing — Fix R0KH-ID vendor-header contract matcher

- Exact focused MTK run #45378781924 on `b9d55bfcbf23c751c2f548e35ba78e9e05e5118b` passed the new GNU89 ASan/UBSan R0KH-ID C regression, then failed in the Python contract because its raw-byte regex used double-escaped metacharacters and therefore did not match the vendor macro. Production code was not implicated.
- Replaced the regex pair with whitespace-normalized byte-string checks. This keeps non-UTF-8 source safe and directly verifies the 48-byte maximum and 49-byte destination field.
- The NUL-compatible R0KH-ID path is retained: raw lengths 1–48 and NUL-inclusive 2–49 are bounded before copy; effective payload length is normalized after copy, with no stored-state mutation on errors. Fresh exact-head MTK/AI/Process/NVRAM/XY-C1 verification is pending.

---

## 2026-11-24 19:20 Beijing — Normalize R0KH-ID input and make source checks encoding-safe

- The R0KH-ID C boundary fixture passed on `0517ce5`, but the focused MTK workflow failed in `verify_ft_r0khid_policy()` because one legacy vendor header contains non-UTF-8 bytes. Replaced text decoding for the vendor macro/structure headers with byte-based assertions.
- Hardened compatibility for the two common Wireless Extensions string forms: raw payload length (1–48) and length including a final NUL (2–49). The driver first caps the raw copy to 49 bytes, then normalizes after successful usercopy; empty/oversized effective IDs are rejected before publishing. A 49-byte payload without a final NUL is rejected, while a 48-byte payload with or without an extra terminal NUL is accepted.
- Expanded the GNU89 ASan/UBSan regression to cover both encodings, 0/50/UINT_MAX, empty effective payload, missing terminator on the 49-byte form, and NULL arguments. Exact follow-up MTK/XY-C1/AI/Process Actions are pending.

---

## 2026-11-24 19:04 Beijing — Bound 802.11r R0KH-ID private ioctl input

- Confirmed `FT_ROKH_ID_LEN == 48` in all four MTK driver families and `FT_CFG.FtR0khId[FT_ROKH_ID_LEN + 1]` (49 bytes, documented as 1–48 characters). The old `length <= 48` check rejected every legal length and then copied arbitrary caller length directly into this fixed array.
- MT76x2, MT76x3, MT7615 and MT7915 handlers now accept only lengths 1–48, stage user data in a zeroed 49-byte buffer, return `-EFAULT` without publishing partial data on copy failure, and publish the NUL-terminated buffer plus length only after success.
- Added a shared GNU89 length policy, ASan/UBSan test for 0, 1, 48, 49, UINT_MAX and zero maximum, and source-contract checks for all four drivers. The MTK workflow compiles/runs the C test; XY-C1 path filters track the production header.
- Exact-SHA MTK, XY-C1 and AI checks for this commit are pending. No target build or physical-hardware result is claimed.

---

## 2026-11-24 — Refine WSC PSK diagnostic contract

- The second exact-SHA regression run exposed a substring false positive: the checker confused the safe `WpaPskLen` field with logging the non-NUL-terminated `WpaPsk` byte array.
- Tightened the assertion to require the length-only diagnostic actually used by the patched handler. Production source remains unchanged.

---

## 2026-11-24 — Correct PSK regression contract after CI feedback

- The first exact-SHA MTK run exposed an over-escaped regular-expression in the new source checker; production code compiled neither failed nor regressed, but the static contract falsely rejected the correctly-added length guard.
- Replaced the brittle regex with direct source-token and ordering assertions for the 64-byte cap, single user copy, EFAULT early exit, and staged AP/APCLI/WSC writes. The dedicated MTK workflow is re-run on the checker-only commit.

---

## 2026-11-24 — Bound MT76x3 OID_SET_PSK input copies

- The MT76x3 `OID_SET_PSK` case only capped the temporary log buffer; the subsequent AP/APCLI/WSC copies still used the unrestricted user-supplied length. `WPAKeyString[65]` and WSC's `WpaPsk[64]` made oversized requests capable of overflowing fixed storage.
- Added a strict 64-byte maximum before accessing user memory, stage the input once into a 65-byte NUL-terminated local buffer, and return `-EFAULT` on a failed user copy. All persistent destinations are populated from the validated staging buffer; WSC logging now reports the length rather than printing its non-NUL-terminated 64-byte array.
- Extended the MTK source-contract regression to enforce the length cap, single bounded user copy, failure-before-use ordering, and staged copies. The focused MTK workflow already watches this source and checker.
- Exact-SHA MTK regression and same-SHA XY-C1 Build/TRX/artifact validation remain required; no local kernel build or hardware result is claimed.

---

## 2026-11-24 18:50 Beijing — Align Process LLDT path contract with full interface identity

- Main source commit `0b532570d102824d27fd3bb3ab4d6683f956d94f` removes the prior 10-character interface truncation from LLDT PID filenames, validates nonempty slash-free interface names no longer than Linux IFNAMSIZ-1 (15), and tests names sharing the same first ten characters. Dedicated LLDT run #45376256149 PASS.
- Process regression #45376256143 on exact SHA `0b532570d102824d27fd3bb3ab4d6683f956d94f` passed the LLTD PID parser and failed only because its source contract still expected the removed `/var/run/%s-%.10s.pid` formatter. Updated the contract to require `lldt_pidfile_format_path()`, its 15-character interface validation, and the full-name identity fixture.
- Process inventory #45376256035 and NVRAM audit #45376256079 PASS on the same SHA. XY-C1 run #45376256112 remained in Build XY-C1; TRX verification and artifact upload were pending. The checker/docs follow-up awaits exact-head Actions.

---

## 2026-11-24 18:43 Beijing — Restore mandatory handoff checkpoint contract

- Exact AI handoff documentation run #45375798381 on `72e1e72dd494e6d930bdfef46f349f20da7065be` failed because the prior handoff rewrite omitted the validator-required `## Mandatory handoff checkpoint` section and its exact conditional-edit rule. Restored the mandatory section and end-of-session accuracy checkpoint.
- LLDT integration checker alignment is committed in `be37029341b79273e520fda75f3b087b41757372`. Process run #45375797796 on the MTK source commit was cancelled before LLTD PID parser/integration steps; rerun attempt 2 of #45375705202 on `be370293` was still running at last inspection. No Process PASS is claimed.
- MTK AP 802.11r boolean ioctl regression #45375797791 PASS on exact SHA `72e1e72`. Same-SHA Process #45375797796, NVRAM #45375798550 and XY-C1 #45375799312 had not completed; XY Build/TRX/artifact result is not verified.
- This handoff correction's exact-SHA AI docs and other path-triggered Actions are pending. No L3 or L4 claim is made.

---

## 2026-11-24 — Fix partial-copy stack values in MTK 802.11r ioctls

- MT76x2, MT76x3, MT7615 and MT7915 AP ioctl handlers declared a stack `ULONG value` while validating one-byte `BOOLEAN` input for `OID_802_11R_SUPPORT`, `OID_802_11R_RIC` and `OID_802_11R_OTD`. Only the BOOLEAN-sized prefix was copied, leaving the remaining stack bytes uninitialized; the flags were also updated without checking `copy_from_user()` failure.
- Changed each handler to use a `BOOLEAN` destination, copy exactly `sizeof(value)`, and update/log only after a successful copy. Failures now set `-EFAULT` and leave the affected flag unchanged.
- Expanded the MTK source-contract regression to cover all three commands across all four driver variants. Both the focused regression workflow and XY-C1 build workflow now watch every affected production source path.
- Exact-SHA regression and same-SHA XY-C1 Build/TRX/artifact results are required before claiming compile validation; no local kernel build or hardware result is claimed.

---

## 2026-11-24 18:34 Beijing — Align process lifecycle LLDT contract with descriptor policy

- The exact Process lifecycle regression #45374673151 on `8796b356087fe77920a90e43f2889e86a6096013` compiled and passed the LLTD PID-file parser regression, then failed only because its source checker still required two inline `S_ISREG(st.st_mode)` checks after validation had been factored into `lldt_pidfile_validate_regular_file()`.
- Updated `scripts/check_process_lifecycle_integration.py` to verify both PID-descriptor policy calls precede read/truncate, the sidecar descriptor is validated before flock, and the header enforces regular-file type, effective-UID ownership, one link and no group/world write. This preserves the safety contract while matching the shared policy implementation.
- Dedicated LLDT exact-head workflow #45375259525 on `9b97f74377509e2740442475b210bf9921b04915` passed the GNU89 parser/metadata regression and production source contract. This checker/docs commit's Process, AI handoff and related Actions are pending.
- XY-C1 build run #45374510309 on production-source SHA `529cf0739b3089bca88936c4c06c75cbd3eb79da` was still in Build at last observation; TRX and artifact stages were pending. No L3 or L4 claim is made.

---

## 2026-11-24 — Bound adjacent legacy MTK AP ioctl stack copies

- The first APD PID fix exposed the same unbounded `wrq->u.data.length` pattern in adjacent fixed-size destinations in the legacy MT76x2/MT76x3 AP ioctl dispatchers: IAPP PID and WAPI PID scalar locals, plus the six-byte STA address buffer used by `RT_SET_DEL_MAC_ENTRY`.
- Added exact-size rejection before each copy and pass only `sizeof(local_pid)` or `MAC_ADDR_LEN` to `copy_from_user()`. The newer MT7615/MT7915 variants already have equivalent size checks; no behavior change is made for valid callers.
- Expanded the source-contract regression to pin all four fixed-size copy paths in both legacy driver trees.

---

## 2026-11-24 — Bound legacy MTK APD PID ioctl copies

- Found a kernel stack-buffer overrun in the legacy MT76x2 and MT76x3 AP ioctl handlers: `RT_SET_APD_PID` copied the caller-controlled `wrq->u.data.length` bytes into a single `unsigned long` local. Zero/short lengths also left the PID partially or wholly uninitialized before publication.
- Added an exact `sizeof(apd_pid)` length check before `copy_from_user()` and changed the copy length to the fixed destination size in both affected handlers. The newer MT7615 and MT7915 handlers already perform the same exact-size check and were left unchanged.
- Added a source-contract regression for both legacy handlers and made the XY-C1 firmware workflow trigger on these kernel-driver source paths, so target compilation is not silently skipped.
- Exact-SHA source-contract and XY-C1 Build/TRX/artifact Actions are required before claiming L2/L3; no local kernel or router hardware result is claimed.

---

## 2026-11-24 — Enforce EPSV ALL against EPRT active-mode setup

- RFC 2428 section 3 requires a server that accepted `EPSV ALL` to reject all data-connection setup commands other than EPSV, explicitly including EPRT, PORT and PASV.
- The VSFTPD command dispatcher already gated PASV and PORT on `!p_sess->epsv_all`, but the EPRT branch omitted the same session-state check. A client could therefore switch back to active-mode EPRT after declaring EPSV-only operation.
- Added the missing EPRT dispatch guard. Extended the existing VSFTPD source-contract workflow to assert that PASV, PORT and EPRT are all disabled once `epsv_all` is set; ordinary EPRT address/port validation is unchanged.
- Exact-SHA VSFTPD and AI handoff Actions will be used for verification. No local daemon integration or firmware/hardware result is claimed.

---

## 2026-11-24 — Reject malformed EPSV protocol selectors

- `handle_pasv()` previously used `atoi()` for the optional EPSV network-protocol selector. Numeric prefixes such as `1junk` and signed/whitespace forms could therefore be accepted as protocol 1 instead of rejected.
- Replaced the permissive conversion with the existing length-aware bounded decimal parser, preserving `EPSV ALL` semantics and IPv4/IPv6 protocol compatibility. IPv4 accepts protocol 1 only; IPv6 accepts 1 or 2.
- Extended the production-parser host regression with protocol-family cases and malformed selector cases. The VSFTPD workflow already compiles and runs this regression; its source contract now pins strict parser use in `handle_pasv()` and rejects a return to `atoi()`.
- Local checkout is unavailable in this runtime; the exact-SHA Actions run is the authoritative compile/regression gate. No firmware or router-hardware result is claimed.

---

## 2026-11-24 — Serialize forced-mode indirect MMD access as one MDIO transaction

- In forced 1000/2000/2600 mode on PHY address 31, `mii_mgr_read()` and `mii_mgr_write()` issue three Clause-22 commands: select the MMD page, access the low word, then access the high word. Previously every command independently acquired/released `mdio_lock`, so another MDIO user (including Clause 45) could interleave and replace the page selector between phases. The operation could then return or write data from the wrong MMD register. Auto-negotiation polling was also toggled outside that multi-command critical section.
- Split the register-level primitives into caller-locked implementations and one-command lock-taking wrappers. The extended MMD path now holds the shared `mdio_lock` across page select, both data words, and AN-polling disable/restore; it uses only the locked primitives while inside the transaction. Ordinary Clause-22 and Clause-45 access keep their existing locking.
- Added an isolated GNU89 policy regression for exact page/data register order, 32-bit read assembly, 32-bit write splitting, failure short-circuiting and output clearing. Added a source checker that verifies transaction lock scope, locked callback wiring, direct-command wrappers and existing Clause-45 lock scope. The switch stability workflow compiles/runs both checks and now filters the new test/helper/checker paths.
- Local host regression with AddressSanitizer and UBSan: PASS. Exact-head Actions for this commit: pending. Firmware build and router hardware validation are not yet claimed.

---

## 2026-11-24 — Restore separate QMI contract assertions

- Exact main `d8f6092c6af34f1c8ec45c746ce9411e40522577` USB modem WDM/NDIS Action #45362480729 failed before evaluating the client-ID contract because two Python `assert` statements had been concatenated onto one line in the workflow heredoc. The traceback was a genuine `SyntaxError`, not a QMI production failure.
- Restored the missing newline between the persisted-WDS-ID diagnostic assertion and the first `_eval()` command-status assertion. No modem runtime source was changed. Exact-head rerun is pending.

---

## 2026-11-24 — Align HTTPD Samba trust-boundary check with one-read ACL snapshot

- Exact main `d8f6092c6af34f1c8ec45c746ce9411e40522577` HTTPD job #45362480519 failed in the Samba share trust-boundary checker after the Samba writer was improved to snapshot each user's ACL permission once per share.
- The production change now validates account identity in the snapshot loop (five guard occurrences total) and reuses `account_rights[i]` in the four output-list passes. The older HTTPD contract still expected four identity guards and did not enforce the one-read cache. Updated it to require exactly one per-share `get_permission()`, one cached assignment, and four cached consumers. The separate Samba workflow's policy regression already passes on this same source.
- This is a checker-only correction; no Samba production behavior changed. Exact follow-up Action status will be recorded at the next handoff checkpoint.

---

## 2026-11-24 — Prevent FRP argv separator spoofing with embedded newlines

- Reviewing the FRP process matcher after the checked-procfs-read fix exposed a distinct false-positive: it translated NUL argument separators into LF and split the result with AWK, but did not reject literal LF bytes already present inside an argv element. A process with the real `frpc` executable and argv[0] equal to `/usr/bin/frpc\n-c`, followed by the config path, was therefore misidentified as the managed three-argument daemon. `frp_close()` could send TERM/KILL to that unrelated invocation.
- Added a checked, non-pipeline `wc -l < /proc/PID/cmdline` pass before NUL-to-LF conversion. A read/utility error stays UNKNOWN; any raw LF means a definite argv mismatch. The configured target BusyBox has `CONFIG_WC=y`, and the FRP workflow now watches that config dependency.
- Added a stop regression with the forged two-argument cmdline. It proves the old parser accepted the decoy, while the new parser rejects it and `frp_close()` sends no signal. The source checker binds the guard, target applet config and regression.
- Local BusyBox-sh reproduction: old conversion/AWK matcher returned MATCH for the newline-injected argv; checked raw LF count returned 1 and rejected it. Full FRP exact-head Actions are pending. No new firmware or hardware result is claimed for this patch.

---

## 2026-11-24 — Restore exact wording required by AI handoff validator

- Exact `0c23ad7c82fc2e033f73d4a94f6fb02a77324ca5` VSFTPD stability regression #45359061300 PASS, including strict SITE CHMOD/UMASK policy compile/run and source/workflow contract.
- AI handoff contract #45359061366 failed at its explicit string guard because the handoff described “one atomic commit” but did not contain the required literal phrase “one atomic main commit”. The operating rule was already implemented; this change aligns the handoff wording with the validator without changing behavior.
- Exact checker rerun is pending. Root dev log currently contains 75 complete entries (below 120 rotation threshold) and is approximately 85 KiB (below 384 KiB target), so no archive rotation is required.

---

## 2026-11-24 — Fix VSFTPD workflow path-filter checker scope

- Exact `ea7d7e28be9988175fbcc423f9378e5d765de764` VSFTPD runtime compile/regression ran before the inline source contract failed at its path-coverage assertion. The parser-test content checks for `0777` and `UINT_MAX` were valid.
- Root cause: the checker counted each new header/test path across the entire YAML document and expected exactly two occurrences, but each path also occurs in the inline Python source loading and build command. Replaced the global substring counts with entries in the existing `paths` tuple, whose validator separates `push` and `pull_request` filters and requires one occurrence in each. This preserves the intended exact path-filter coverage without counting unrelated references.
- The standalone GNU89 octal parser regression had already compiled and passed locally. Exact-head VSFTPD and AI handoff reruns for this checker correction are pending.

---

## 2026-11-24 — Correct AliDNS checker require() arguments

- Exact e4fdbc7 AliDNS Action #45357277912 completed all functional shell/API/lifecycle/HMAC privacy steps successfully but the static contract failed with `TypeError: require() missing 1 required positional argument: 'message'`.
- Cause: the standalone-`od` regex predicate was not separated from the require diagnostic string by a comma. Corrected the predicate/message argument boundary; encoder and tests remain unchanged.
- Exact e4 AI handoff #45357277937 PASS. Exact 824 AliDNS #45357686604 was still in progress at last observation.

---

## 2026-11-24 — Fix AliDNS require() contract call

- Exact e4fdbc7 AliDNS run #45357277912 passed shell syntax, signed transport, IPv4/IPv6 lookup, lifecycle/NVRAM, CA bootstrap and HMAC argument-privacy tests. The final checker failed with TypeError because its first new `require()` call omitted its second message argument.
- Added the missing comma and a Python regex token-boundary check for a standalone `od` command. Production encoder and test fixture are unchanged.
- Exact e4 AI handoff #45357277937 PASS. Corrected checker SHA requires exact-head re-run.

---

## 2026-11-24 — Fix AliDNS od-command contract false positive

- Exact 32263 AliDNS #45356801815 passed shell syntax and every functional test, then the static check failed because a substring search for "od" matched the word "encode" in the comments.
- Replaced it with a command-token check that splits shell pipeline segments and examines each leading command. Production source/test files are unchanged.
- Exact 32263 AI handoff #45356801657 PASS; corrected exact-head AliDNS check pending.

---

## 2026-11-24 — Fix AliDNS Python checker syntax

- Exact ca9267 AliDNS Action #45356506023 passed shell syntax, signed transport, address lookup, lifecycle/NVRAM and HMAC argument-privacy regression steps. The final source-contract step failed with SyntaxError: unterminated string literal at Python line 127.
- The failure was in a newly added nested-quote assertion for the encoder printf format, not in production encoding or the shell tests. Simplified it to a valid Python double-quoted format-string assertion.
- Exact corrected-head AliDNS and AI handoff runs #45356634683 / #45356634556 were in progress at last observation.

---

## 2026-11-24 — Correct the AliDNS POSIX encoder checker quoting

- The first POSIX encoder commit ca9267b3c71a9fb2a8de96e82a2878fcf31e5dc6 included an invalidly quoted Python source assertion for the printf format expression.
- Replaced the complicated nested-quote check with separate assertions for the %02X format and "'$c" byte-value operand. Production encoder and test fixture are unchanged.
- Exact ca9267 Actions were queued/in progress at last check; this corrected checker commit needs exact-head verification.

---

## 2026-11-24 — Make AliDNS RFC3986 encoder portable to configured ash

- The read -d '' -n1 encoder relied on optional BusyBox ash extensions not enabled by the board configuration; the Bash re-exec in the test fixture masked the target-shell mismatch.
- Replaced it with a POSIX read -r line loop, byte-wise parameter expansion under LC_ALL=C and explicit %0A output for each LF consumed by read. No read -n/read -d, od or Bash is used.
- Removed Bash re-exec/helper from the fixture; sourced production functions are now exercised directly under /bin/sh. Local /bin/sh checks passed for reserved punctuation, quote/backslash, whitespace, UTF-8, controls, single/multiple/empty LF and unterminated final input. Exact full regression pending.
- Parent 825e3ac7faab97eec889dc1e0109b34ba4785547: AliDNS #45356096386 and Process #45356096367 PASS. Parent 21699dc5a854afb6aea0121a64421bb836aa447c: VSFTPD #45356072893 and AI handoff #45356072970 PASS.

---

## 2026-11-24 — Restore VSFTPD decimal parser source-contract binding

- Exact b87 VSFTPD run #45355332629 compiled and ran the GNU89 REST policy and existing parser tests successfully, but the workflow's Python source checker raised NameError: name 'decimal_parser' is not defined: REST assertions had displaced an existing local function-body binding.
- Restored the decimal parser binding before the octet parser binding. This is checker-only; no source/test behavior change. Exact-head validation pending.
- Exact b87 Process #45355332631 and AI handoff #45355332668 PASS; XY-C1 #45355332727 and NVRAM #45355332714 were in progress at last observation. e942 AliDNS #45355770998 failed at the shell regression step; Process #45355770855 PASS.

---

## 2026-11-24 — Reject malformed FTP REST restart markers

- `handle_rest()` passed client text to `str_a_to_filesize_t()`, then checked for a negative result. That converter returns zero for invalid characters and strings longer than 15 bytes, so malformed REST values were accepted with FTP 350 and restart offset zero.
- Added a strict 1–15 digit decimal parser with output preservation on failure and GNU89 coverage for null/empty, signs, suffixes, whitespace, embedded NUL, overlength, zero, leading zeros and the maximum supported 15-digit value. Invalid REST now clears a stale one-shot offset and responds FTP_BADCMD; valid offsets preserve previous behavior.
- Exact parent `ffeba1d20c4f03ed3f46b173cc92c7f253fa67bf`: VSFTPD #45354611125, Process #45354611121 and AI handoff #45354611170 PASS; NVRAM #45354611026 PENDING at last observation. Exact-head validation of this source fix is pending.
- No firmware Build/TRX/artifact or router hardware validation claimed.

---

## 2026-11-24 — Correct the EPRT DEL-byte test fixture

- Exact `b37af6b5caf6fa0346ef27cd94dd7c9e6b5f2fd5` VSFTPD job `#45353862964` compiled the C tests and passed earlier FTP suites, but the parser regression's DEL test contained a double-escaped C `\\x7f`, so runtime input was literal text instead of ASCII 127.
- Replaced it with adjacent string literals containing the bounded octal `\177` escape (also avoiding C hex escape digit coalescing) and fixed the diagnostic newline. Production parser/handler remain unchanged. Exact-head regression pending.
- Exact `b37af6b`: Process inventory `#45353862966` and AI handoff `#45353863108` PASS. Exact `d651b0f`: HTTPD #45354160704 and Process #45354160643 PASS; AliDNS #45354160627 failed on outdated signature expectation; XY-C1 #45354160727 pending at last observation. AliDNS expectation calibration followed in `6a82821`, exact-head verification pending.

---

## 2026-11-24 — Honor RFC 2428 EPRT delimiters

- `handle_eprt()` hard-coded `|` for every field boundary. RFC 2428 section 2 allows any printable ASCII delimiter (33–126); `|` is recommended only when it does not collide with a character used by the address.
- Added a strict EPRT delimiter parser that requires exactly four occurrences and returns the selected byte; the production handler now uses it for all four field splits. GNU89 regression covers standard and alternate delimiters, invalid control/DEL bytes, missing/extra delimiters and unchanged output on rejection. Workflow source contracts pin printable bounds, occurrence count and use of the dynamic delimiter.
- Exact parent main: `b77c7a3aca7a4d7a1a7026488e5ac453c3363dc7`. VSFTPD run `#45353454360`, Process inventory `#45353454331`, and AI handoff `#45353454513` PASS; NVRAM audit `#45353454240` was PENDING at last observation. Exact-head verification for this change is pending.
- No XY-C1 firmware or hardware L3/L4 validation is claimed.

---

## 2026-11-24 — Correct FRP checker typo and assert a definite mismatch

- All FRP shell regressions in exact d2b954 lifecycle run #45351395153 passed, including the added genuine-FRPC/empty-fourth-argv case. The final source-contract checker alone failed due to a JavaScript-style `.indexOf()` call on a Python `str`.
- Replaced `.indexOf()` with Python `.index()`. Strengthened the regression to require return status 1 (definite argv mismatch), rather than accepting status 2/UNKNOWN as sufficient rejection.
- Parent main: d2b954034d1a41de7dd9c170d597fbb6c2fa39b0. Exact-head workflow verification pending on the new commit.

---
## 2026-11-24 — Preserve exact FRP argv boundaries during procfs matching

- Found a process-identity bypass: command substitution strips trailing newlines after `/proc/<pid>/cmdline` is converted from NUL-delimited argv. An additional empty final argument therefore disappeared before the exact-three-arguments awk check and could make a non-exact command look like the owned FRPC/FRPS process.
- `frp_process_matches()` now counts NUL terminators first and rejects any command line that is not exactly three arguments before text conversion.
- Added a regression using the real FRPC executable path, exact config argument and one extra empty final argument; it must not be considered owned. The static checker pins the NUL-count gate before conversion.
- Parent main: 7e69ffb75bd26c854e2c36662185ef9d0815c194. That SHA fixed the previous ZeroTier orbit rollback regression; exact-head ZeroTier lifecycle #45350691842 completed successfully. No router hardware validation is claimed.

---
## 2026-11-23 — Fail closed on QMI modem setup command errors

- Found that SIM PIN verification, `--set-data-format 802.3`, and `--set-network-modes` errors were logged or ignored while QMI startup continued toward CID allocation and `--start-network`. This could ignore required configuration or continue after a rejected PIN.
- Each required setup command now uses the common wait-status normalizer and aborts on a nonzero result. PIN staging cleanup is still attempted before the PIN-command error is propagated.
- Added exact-source workflow assertions for the three checked command calls and fail-fast ordering; existing GNU89 tests exercise positive and negative command-runner statuses.
- Parent main: 1c5d38531a62094501f3ab12b5043ba884b358f0. Exact Actions were still running at last poll; no pending result is claimed.

---

## 2026-11-23 — Preserve ambiguous unreadable QMI packet-data markers

- Review found that ambiguous QMI start cleanup unlinked `/tmp/qmi-handle` when the file existed but strict parsing failed or the read returned EACCES/EIO. It could therefore erase the only recovery marker for a live session.
- Expanded `modem_qmi_start_attempt_preserve_handle()` to preserve any non-ENOENT marker on an ambiguous attempt. Only genuinely missing PDH state is eligible for checked cleanup/retry.
- Added GNU89 regression for EACCES/EINVAL/EIO preservation and ENOENT cleanup; the exact USB modem workflow pins the captured read status and preserve-before-unlink ordering.
- Parent main: 61da9828ecbca9e124bf270d2651a372dd667359. Its exact Actions were still running at last poll; no pending result is claimed.

---

## 2026-11-23 — Preserve QMI network-mode validation before session preflight

- Post-commit source review found a status-variable ordering bug in the previous QMI preflight change: the PDH preflight overwrote `status` before the `modem_nets` parse result was checked, which could turn an invalid `modem_nets` setting into a false successful parse.
- Moved the `modem_nets` parse error gate immediately after parsing and before any PDH/CID decisions. Added an exact-source ordering assertion so future changes cannot reuse the status variable before the selector's error is handled.
- Parent main: d8c800ebf2c6a2cb6e42b654d83e768d5fcba111. On d8c800, USB identity #45349445016, AI handoff #45349444955, HTTPD #45349444927, WAN teardown #45349444890, WAN DHCP #45349445017 and detect_link #45349445165 passed; NVRAM #45349445005, Process fail-closed #45349444915, Process inventory #45349445166 and XY-C1 #45349444899 were still in progress at last poll. No same-SHA firmware L3 claim.

---
## 2026-11-23 — Reuse persisted QMI client IDs and block duplicate sessions

- Found that qmi_control_network_start() always requested a new WDS CID, even when a previous failed/ambiguous start left a kept CID file. The next invocation could leak the old CID by atomically overwriting its only record. It also erased a valid prior PDH marker before another start.
- Added errno-aware status-file reads, a PDH preflight that refuses to start over an existing session marker, and CID reuse when a valid persisted one-byte ID exists. CID allocation occurs only when the file is genuinely absent; malformed or unreadable state fails closed rather than being overwritten. Teardown also preserves invalid/unreadable CID state instead of silently unlinking it.
- Extended GNU89 policy regressions to cover PDH preflight, CID bounds/reuse, and ambiguous starts where the marker read itself fails. Updated the USB modem workflow source contract for the full QMI start function and teardown validation.
- Parent main: f12c4a3292e53378751eefb7c2c8b05364791900. Exact f12c4a3 USB modem identity #45348552081 and AI handoff #45348552061 passed; Process inventory #45348551991 passed. NVRAM audit #45348551978 and XY-C1 #45348552126 remained in progress at last poll; no same-SHA firmware L3 claim.

---

## 2026-11-23 — Validate QMI client IDs before narrowing to the wire field

- `cmd_set_client_id_prepare()` used `strtoul()` without checking ERANGE, no-digit input or the one-byte CID range. Since `qmi_service_connect()` stores the CID in `uint8_t`, values over 255 were silently truncated and negative/empty tokens could select an unintended CID.
- Added `uqmi_parse_client_id()` as a GNU89-compatible helper. It keeps the existing base-0 number syntax but rejects negative, empty, malformed, overflowing and greater-than-UCHAR_MAX values without modifying the output on error.
- Added an ASan/UBSan standalone regression and wired it into both push and pull-request paths of the USB modem identity workflow. Source contract asserts the setter uses the parser.
- Parent main: c8ea6fb62a7186059578e1685535163dfc81ca38. Exact c8ea6fb USB modem identity #45348146003 and AI handoff #45348145979 PASS. Exact 0bf423b Process/HTTPD/WAN/DHCP/detect_link checks PASS; its USB modem failure was a static checker NameError fixed in c8ea6fb. Its NVRAM and XY-C1 builds were still in progress at last poll; no firmware L3 claim.

---

## 2026-11-23 — Preserve a QMI handle when start outcome is ambiguous

- Review found that the ambiguous QMI failed-command/valid-PDH path unlinked /tmp/qmi-handle before exiting. That file may be the only recovery handle for a still-live modem session; deleting it prevents later explicit teardown.
- Added modem_qmi_start_attempt_preserve_handle() and a GNU89 regression matrix. The startup path exits before cleanup when the result is ambiguous and a valid PDH exists. Missing/malformed handles are still removed with checked unlink semantics; confirmed failed commands with no PDH remain retryable.
- Extended the USB modem workflow contract to ensure the preserve decision precedes PDH unlink and that the regression is compiled/executed.
- Parent main: 5de929ef21c09630adffaaeff06f057afb2ba704. At parent SHA, USB modem identity, AI handoff, HTTPD, WAN teardown, WAN DHCP callback, detect_link and Process inventory passed; Process fail-closed, NVRAM and XY-C1 were still running. No firmware L3 or physical modem validation is claimed.

---

## 2026-11-23 — Avoid retrying an ambiguous successful QMI start

- A QMI start command that exits successfully but leaves no valid PDH file is not necessarily safe to retry: the WDS response may have created a live session while local handle publication failed.
- Added `modem_qmi_start_attempt_is_ambiguous()` to stop retries for either a successful command without a parsed PDH or a failed command that nevertheless published a PDH. A failed command with no PDH remains retryable; a successful command with a valid PDH remains success.
- Added GNU89 regression coverage for all command-status/handle combinations and extended the USB modem workflow source contract to pin the policy use.
- Parent main: `08f39868ce700f008ba6501b4388eb06681cac64`. On parent d549f80, Process inventory, Process fail-closed, HTTPD, WAN teardown, DHCP callback, detect_link and AI handoff all passed; its USB modem identity check failed only on escaped-delimiter source slicing, corrected in 08f3986. NVRAM and XY-C1 remained pending at last observation.

---

## 2026-11-23 — Propagate QMI WAN teardown failures and preserve retry state

- Found a teardown reliability defect in `qmi_control_network()`: it ignored `_eval(--stop-network)` and `_eval(--release-client-id)` return values, unconditionally unlinked both QMI state files, and always returned success. A failed network stop could lose the CID/PDH needed to retry cleanup.
- The stop path now returns failure and preserves both files when stop-network fails. After a successful network stop it removes the PDH marker with checked unlink semantics, releases the CID, retains the CID marker if release fails, and checks final CID-file cleanup. Command-runner status is normalized for negative error results.
- Added GNU89 regression for command wait-status normalization and workflow contract assertions for stop-before-cleanup ordering, retained retry state, both checked command calls and unlink handling.
- Exact parent e006c68 USB modem identity #45347148897, Process inventory #45347149044, NVRAM audit #45347148992, AI handoff #45347148865 and XY-C1 #45347148972 were still in progress/queued when sampled; verify their final outcomes separately.
- Parent main: e006c68f711c0fc47e6d46d7f34b3c337a06f7da. Exact USB modem identity rerun required for this change.

---

## 2026-11-23 — Publish UQMI PDH handles atomically

- Follow-up to QMI WAN startup hardening: `cmd_wds_start_network_cb()` still wrote `/tmp/qmi-handle` with unchecked `fopen("w+")`, `fprintf`, and `fclose`. A failed write could truncate or partially publish the PDH status.
- Replaced the callback's direct write with the existing `uqmi_status_file_write()` helper, which uses private `mkstemp` staging, mode 0600, full writes, fsync and atomic rename. The callback now emits an error and omits the handle field if publication fails.
- Extended the USB modem workflow's static contract to require the helper call before publishing the JSON handle and prohibit the unchecked direct file-writer sequence. The existing GNU89 UQMI status-file test covers replacement permissions, symlink safety, failed rename preserving the destination, and staging cleanup.
- Parent main: 42257289849aa454630dfda18b8e1efc1a213411. Exact USB modem identity Action must be checked on the resulting SHA.
- At parent SHA, Process inventory #45346865350, HTTPD #45346865450 and AliyunDrive #45346865525 were still in progress after the MTD checker syntax cleanup; final outcomes remain to be checked.

---

## 2026-11-23 — Correct QMI regression fixture and static checker slicing

- Exact QMI commit 2b7281bca3ed4be30f65b5ec803413e6cd78f86f: USB modem identity #45346405084 reached the modem WAN policy test and failed because the test source encoded backslash+n as two literal characters. Replaced it with explicit ASCII newline bytes and added a separate multi-line rejection fixture.
- Corrected the embedded workflow check to slice qmi_control_network_start by its function signature rather than escaped newline strings; this removes sensitivity to Python/string escaping in the YAML block.
- Production QMI code is unchanged in this follow-up. The exact USB modem workflow must rerun and reach the later static QMI contract assertions.
- On 6904523, AI handoff and HTTPD passed; Process inventory and AliyunDrive failed on additional MTD/PID1 assertions. Exact logs need triage independently.

---

## 2026-11-23 — Reject stale or malformed QMI packet-data handles

- QMI WAN startup had three linked false-success paths: unchecked unlink(QMI_HANDLE_PDH), ignored _eval(--start-network) status, and a plain file-existence success check. get_qmi_handle() also ignored fscanf/fclose and accepted a leading integer from malformed trailing data.
- Added a strict decimal QMI handle parser (one token, nonnegative, within INT_MAX), checked fclose and extra input in get_qmi_handle(), and require both a successful uqmi command and a valid PDH handle before returning startup success.
- The stale PDH file is removed before the first attempt; ENOENT is benign but other unlink errors stop startup. Failed/invalid attempts remove their partial handle before retry. If uqmi returns failure after publishing a valid PDH handle, retries stop to avoid blindly launching a second possible live session.
- Added policy tests for command/handle combinations, unlink errno semantics, malformed/negative/overflow handles and valid whitespace. Extended the USB modem workflow contract to enforce the source checks. Physical modem behavior is not hardware-validated.
- Source parent main: 69045235245d1b9bbdf8e830e9a5570906329929. Exact USB modem Actions will be checked on the resulting commit.

---

## 2026-11-23 — Repair malformed MTD reboot checker follow-up

- Commit 33680078f7b33ac6e166fbeab5ed99d4a5e17558 attempted to narrow the negative check for a standalone MTD helper invocation, but the committed raw-string expression was unterminated. Exact Process/AliyunDrive/HTTPD reruns #45345902653, #45345902730 and #45345902660 were still IN_PROGRESS at last poll.
- Replaced the incomplete expression with a complete, line-anchored Python regex: it rejects only a standalone unassigned `system("/bin/mtd_write -r unlock mtd1");` statement while allowing the correctly assigned `mtd_unlock_status = system(...);` expression. The positive wait-status/log/fallback assertions remain.
- No production source changed in this follow-up. Exact Process, AliyunDrive, HTTPD and AI handoff Actions must be checked on the new commit.
- The next independent source task remains QMI WAN startup: stale/invalid PDH handles and ignored `--start-network` failures must not allow false success.

---

## 2026-11-24 15:18 UTC+8 — Add PID 1 fallback when MTD reboot helper returns

- Exact 3724008 results: ZeroTier lifecycle #45344943818 PASS; Process inventory #45344943892 PASS; WAN route #45344943905 PASS; AI handoff #45344943861 PASS. XY-C1 #45344943825 still IN_PROGRESS at the Build step; TRX/artifact pending, no L3 claim.
- Found a concrete PID 1 failure path: under MTD_FLASH_32M_REBOOT_BUG, handle_sig_fatal() invoked `mtd_write -r unlock mtd1` but discarded the return status, then entered an infinite sleep loop. If the helper cannot execute, fails to unlock, or returns without rebooting, PID 1 never tries the direct reboot syscall.
- The handler now captures and logs a nonzero wait status, and unconditionally attempts `reboot(RB_AUTOBOOT)` if the helper returns. The process-lifecycle checker pins status handling and fallback ordering; the helper-return fallback is still not hardware-tested on the target MTD board.
- Parent main: 372400813c7d0888c508d6e6595353c46ea5d960. Atomic files: init.c, check_dev_init_boot.py, SESSION_HANDOFF.md, dev_log.md.

---

## 2026-11-24 15:12 UTC+8 — Bind ZeroTier process identity to its state directory

- Exact 1bfeb82 Actions: ZeroTier lifecycle #45344505217 PASS; Process inventory #45344505230 PASS; WAN route #45344505319 PASS; AI handoff #45344505295 PASS. XY-C1 #45344505235 remains IN_PROGRESS at Build; no TRX/artifact/L3 claim.
- Follow-up threat review found that matching only executable and argv[0] could still stop a separately launched ZeroTier instance using another data directory. The actual service invocation is `/usr/bin/zerotier-one [args] /etc/storage/zerotier-one`; ownership now also requires argv[1] to be the exact configured state directory.
- A process with `comm=zerotier-one` but missing `/proc/PID/exe` is now UNKNOWN while its proc directory still exists, instead of being silently treated as absent. A readable nonmatching executable remains a confirmed non-owner and is never signalled.
- Regression harness now uses the actual two-argument daemon command line and covers a different state directory and missing executable link; old stale numeric-kill log assertions are removed. Parent main: 1bfeb82bb180116882f8c6f942cad21cd50f548d; this atomic follow-up requires exact ZeroTier and AI handoff reruns.

---

## 2026-11-24 15:04 UTC+8 — Replace ZeroTier name-wide stop with exact executable PID validation

- FRP exact-head run #45343199725 on bc34d1db PASS; AI handoff #45343199794 PASS. The previous FRP checker correction is closed.
- Identified an independent ZeroTier process-ownership defect: zerotier_process_state() trusted pidof's mutable process name, and kill_z() then used name-wide killall for TERM and SIGKILL. A process can set its comm to "zerotier-one" without executing the managed binary; therefore the old stop path could signal an unrelated process.
- Reworked stop inventory to scan the configurable procfs root, require the executable link to resolve to /usr/bin/zerotier-one and argv[0] to match, preserve UNKNOWN on missing/unreadable metadata, and revalidate each PID immediately before numeric TERM/KILL. Bounded waits and UNKNOWN fail-closed escalation remain. Linux 4.4's residual PID validation-to-signal race is not claimed eliminated.
- Added synthetic procfs regressions for same-name impostor, argv mismatch, missing command-line metadata, unavailable proc root, graceful TERM, bounded KILL escalation and stuck process. Updated the ZeroTier source contract to reject any return to name-wide signalling.
- Parent main: bc34d1db08a86f762ffad75bec8c8276629f29c3. Exact ZeroTier lifecycle and AI handoff Actions on the resulting commit remain to be checked.

---

## 2026-11-24 14:55 UTC+8 — Align FRP proc-root UNKNOWN regression message

- Exact FRP run #45343036173 on c8985b2b passed executable startup, crontab, boot/config, process-stop and lifecycle-lock regressions. Its final source checker failed only because it expected "non-directory proc root was not UNKNOWN", while the actual test reports "unavailable proc root was not UNKNOWN" after validating a non-directory proc root and requiring status 2 (UNKNOWN). Corrected the assertion text only; the surrounding fail-closed and no-signal checks remain.
- Exact AI handoff #45343036253 was still in progress at last poll. The canonical required Active task section is present.
- Parent main: c8985b2b9f760eb4089ca996cf2d1e320df00031. This is another checker-only follow-up; FRP runtime implementation remains unchanged.

---

## 2026-11-23 — Verify FRP SIGKILL gate through exit-status semantics

- Exact FRP run #45342815755 on `ebbd4dd406187a3b88b7f270f46fd8771bfc3253` passed all executable regression steps, including async startup, isolated managed-crontab tests, boot/config tests, exact process stop and lifecycle lock. Its only failure was a stale source checker requiring the literal comment/string `verified timeout`; production correctly allows SIGKILL only when `frp_wait_stopped 5` returns status 1 (confirmed still-running timeout), while UNKNOWN is rejected.
- Updated the checker to assert the actual `case "$frp_term_wait_status"` status-1 branch, the unknown-state rejection message and that this gate precedes `frp_signal_all KILL`. No production code was relaxed or changed.
- Exact AI handoff docs #45342815724 PASS on `ebbd4dd`; Process inventory #45342556827 PASS on `4be3f3898faf4665ebe19b1e87cf385bd162c21e`. AI docs also validated the required exact `## Active task` heading.
- Parent main: `ebbd4dd406187a3b88b7f270f46fd8771bfc3253`. This checker-only commit should trigger FRP lifecycle, Process inventory and AI handoff checks on the exact new SHA.

---
## 2026-11-23 — Align FRP process-path source contract with composed procfs paths

- On exact commit `4be3f3898faf4665ebe19b1e87cf385bd162c21e`, the FRP lifecycle run #45342556772 passed every executable regression step, including asynchronous startup, `frp_crontab.sh`, boot/config tests, exact process stop and lifecycle lock. Only the final static source checker failed because it expected a literal `$FRP_PROC_ROOT/$frp_match_pid/exe` string even though the implementation safely builds `frp_match_proc_dir="$FRP_PROC_ROOT/$frp_match_pid"` and then appends `/exe` and `/cmdline`.
- Updated the source checker to validate both path-construction assignments and their subsequent `readlink`/cmdline consumers; this preserves the implementation's path semantics instead of weakening process identity checks.
- The same exact SHA Process inventory #45342556827 PASS. AI docs #45342556719 failed because the current handoff heading was `## Active slice: ...` instead of the required `## Active task`; this commit restores the canonical heading.
- On parent `257ac6399c4c7e8ec3b518f8f8438532d0821c33`, Inadyn #45342407697 PASS, DNSMasq #45342407692 PASS and WAN route #45342407753 PASS after the previous Inadyn contract correction; only AI docs #45342407729 failed on that same handoff heading.
- XY-C1 #45341769654 on production source `79c5389` remained in progress at the last poll in `Build XY-C1`; TRX and artifact steps were pending. No L3 result is claimed.
- Parent main: `4be3f3898faf4665ebe19b1e87cf385bd162c21e`. This commit changes the FRP source checker and corrects the handoff section heading; production code is unchanged.

---
## 2026-11-24 15:18 UTC+8 — Isolate FRP crontab transaction test from host procfs

- FRP exact run #45341607742 on `5687c609137bbba09c707896dd95216c3df9e961` progressed past the corrected process-exit fixtures: both FRP watchdog/async lifecycle cases printed PASS. The later crontab test still failed at `frp_start()` before it could assert preservation/publication.
- Root cause in test setup: `frp_crontab.sh` overrides `frp_wait_started()` but leaves `FRP_PROC_ROOT=/proc`; `frp_start()` always runs `frp_stop_daemons()` first. The fail-closed scanner intentionally returns UNKNOWN if any visible candidate's `/proc/PID/exe` cannot be read. A host-only crontab transaction test must not depend on the GitHub runner's cross-user procfs visibility. It now points `FRP_PROC_ROOT` at an empty temporary directory; the separate lifecycle test retains real fake-process coverage. The FRP source policy is unchanged.
- `check_frp_lifecycle.py` now pins the empty temporary procfs fixture so future test edits cannot silently reintroduce this host-dependent precondition.
- Exact parent `257ac6399c4c7e8ec3b518f8f8438532d0821c33` has Inadyn #45342407697, DNSMasq #45342407692, WAN route #45342407753 and AI docs #45342407729 all IN_PROGRESS at last poll. Parent FRP test failure remains #45341607742; parent Process inventory #45341607727 and AI docs #45341607707 PASS.
- Parent main: `257ac6399c4c7e8ec3b518f8f8438532d0821c33`. This atomic commit only changes FRP host-test isolation and source-contract coverage.

---
## 2026-11-24 15:05 UTC+8 — Align dependent contracts with executable-filtered Inadyn inventory

- On exact SHA `79c5389fe33326d85cc23e9856f39e5ca0ac242f`, Inadyn run #45341769924 failed only in its Python source checker after the GNU89 ASan/UBSan policy executables passed. `body()` found the forward declaration of `inadyn_validate_identity()` and accidentally extracted a later function; it now skips declarations until it reaches a definition.
- DNSMasq lifecycle #45341769848 failed at an embedded source assertion that still expected the raw `process_inventory_scan("inadyn", &snapshot)` inside `notify_ddns_update_locked()`. WAN route #45341769689 had the same stale expectation in `check_wan_ddns_status.py`. Both now require `inadyn_process_inventory_scan(&snapshot)`, preserving the stronger executable-bound inventory gate rather than weakening runtime behavior.
- Exact 79c results: HTTPD #45341769686 PASS; AI docs #45341769695 PASS; Process fail-closed #45341769769 PASS; Process inventory #45341769713 PASS; WAN DHCP #45341769830 PASS; miniupnpd #45341769855 PASS; Samba #45341769858 PASS; NVRAM audit #45341769831 PASS. Inadyn #45341769924, DNSMasq #45341769848 and WAN route #45341769689 failed on the stale contracts described above.
- XY-C1 #45341769654 had passed all pre-build validation and was still building; TRX verification and artifact upload were pending. No L3 result is claimed until all three pass on the same production SHA.
- FRP predecessor fix SHA `5687c609137bbba09c707896dd95216c3df9e961`: Process inventory #45341607727 PASS and AI docs #45341607707 PASS, but FRP #45341607742 failed next at `frp_crontab.sh`: `start rejected a valid crontab`. The lifecycle script's earlier identity/async cases passed. Root cause not yet proven; inspect its use of live `/proc` before making any production change.
- Parent main: `79c5389fe33326d85cc23e9856f39e5ca0ac242f`. This atomic commit changes only dependent source contracts and the current handoff evidence; production source is unchanged.

---
## 2026-11-24 14:47 UTC+8 — Bind Inadyn lifecycle to the installed executable

- Inadyn stop, reload and manual refresh previously trusted `process_identity_validate("inadyn", ...)`, which checks comm/argv/start-time but not `/proc/PID/exe`. A same-name/argv impostor could therefore enter the PID-bound signal path.
- Added an injectable executable identity policy that first preserves the generic process identity check, then requires `/bin/inadyn` from `/proc/PID/exe`. A precise ` (deleted)` suffix is allowed for an already-running daemon during replacement; ENOENT/ESRCH are ABSENT, while EACCES and other inspection errors remain UNKNOWN.
- All Inadyn presence, config-reload and manual-refresh inventories now filter same-name wrong-executable decoys. Stop now uses the shared bounded `service_lifecycle_stop()` policy with the same executable-aware scan/validate callbacks; TERM/KILL are only sent to the captured exact executable identity, and UNKNOWN still blocks escalation.
- Added GNU89-compatible policy regressions for exact executable, deleted executable, wrong executable, vanished process, unreadable executable, base identity UNKNOWN/ABSENT and invalid PID; source integration checker and dedicated workflow filters assert the executable-bound stop/reload/manual signal contract.
- Parent main: `5687c609137bbba09c707896dd95216c3df9e961`. Exact Inadyn, Process, AI handoff and XY-C1 firmware checks are pending on the new source SHA; no firmware/TRX or device result is claimed.
- FRP fake process-exit fixture correction is on parent `5687c609137bbba09c707896dd95216c3df9e961`; its exact FRP/Process/AI runs were in progress at the last poll.

---
## 2026-11-24 14:38 UTC+8 — Correct FRP fake process-exit fixtures

- Exact FRP run #45341083663 on `fa3dd0012cea550b339d01df4ff6ec97d0fdc00a` failed at `client missing`: the fake `kill()` callback removed only `/proc/PID/cmdline`, leaving the PID directory and `/proc/PID/exe` link behind. The production scanner correctly interpreted that as an extant process with incomplete identity (UNKNOWN), so the stop path refused to claim a successful replacement.
- Corrected both FRP lifecycle/stop test helpers to remove the complete synthetic PID directory when a process terminates. Tests that intentionally model post-TERM metadata loss continue to remove only `cmdline`, preserving `exe` and the directory; this keeps the fail-closed regression intact.
- No production FRP shell code was changed. Exact FRP, Process inventory and AI handoff checks are required on the follow-up main SHA; predecessor FRP failed, while Process #45341083645 and AI docs #45341083665 passed.
- Parent main: `56931489867a3772ef4b8f45a8559705ebd0fbbc`.

---
## 2026-11-23 — Exact-head FRP tri-state Actions checkpoint

- Commit: `fa3dd0012cea550b339d01df4ff6ec97d0fdc00a`.
- Actions: FRP asynchronous startup lifecycle regression #45341083663 completed/failure; AI handoff documentation contract #45341083665 completed/success; Process lifecycle inventory - fa3dd0012cea550b339d01df4ff6ec97d0fdc00a #45341083645 completed/success .
- This follow-up records only observed exact-SHA job states; pending workflows are not treated as PASS.


---

## 2026-11-23 — Preserve FRP process-inventory UNKNOWN and block unsafe SIGKILL escalation

- FRP matched the expected `/proc/PID/exe` and complete `frpc/frps -c <managed-config>` argv tuple, but an unreadable/missing cmdline for an extant matching executable was collapsed to “not running”. `frp_service_running()` also discarded the scanner's exit status through command substitution, so failed enumeration could look like confirmed absence.
- The bounded stop path treated every nonzero wait result as a timeout and proceeded to SIGKILL. Procfs read failure after TERM or a failed `sleep` could therefore trigger escalation without proof that the owned daemon was still running.
- Added three-state semantics: exact owned process = RUNNING (0), fully inspected absence/unowned process = ABSENT (1), incomplete/unreadable process metadata = UNKNOWN (2). Inventory helpers preserve UNKNOWN; wait distinguishes verified timeout (1) from UNKNOWN/sleep failure (2); stop permits SIGKILL only after the verified five-probe TERM timeout.
- Added regressions for an unavailable proc root, an exact-executable process with missing cmdline, metadata becoming UNKNOWN after TERM, and `sleep` failure. Existing argv-decoy, multi-service stop, bounded TERM/KILL and crontab preservation tests remain.
- Parent main: `8584845fa7ce78b928bfb6e47c5cf8eb8223982f`. Shell-only source/test/checker change; no MIPS firmware or hardware L4 result is claimed. Exact-head FRP and Process Actions required.

---

## 2026-11-24 14:26 UTC+8 — Correct HTTPD hostname publication contract

- Exact main `a26746d033ffa19d9b576010f4fe4a583b4a859a` Process fail-closed #45339568206 PASS, including `LAN kernel hostname/domain status regression: PASS`; Process inventory #45339568157 and AI docs #45339568440 PASS. WAN DHCP #45339568432, WAN route #45339568336, DNSMasq #45339568424, VPN #45339568175, router NVRAM #45339568171, WAN ARP #45339568324, USB modem #45339568348, miniupnpd #45339568380, ZeroTier #45339568346, Wireless #45339568118, IPv6 #45339568388, watchdog #45339568182, detect_link #45339568318 and detect_internet #45339568405 also PASS.
- HTTPD #45339568430 failed only at the DNSMasq hostname-pair source contract: the checker still ordered publication before a direct `sethostname(lan_hname, ...)` call. This commit updates the contract to verify the errno-preserving wrappers and `lan_kernel_names_apply()` call after atomic file publication and before lock release. No production source changes in this follow-up.
- At last poll, XY-C1 #45339568161, Firewall #45339568335 and WAN teardown #45339568351 remained in progress on `a26746d`; no L3 result is claimed. Main advanced concurrently to `cbbb27636807b7f195d7d87ccefb476366a07995`, which contains the separate AdGuardHome executable/argv/start_time hardening; its config-security #45339988745 and AI docs #45339988750 passed, while Process inventory #45339988758 was still in progress.
- Parent main for this atomic checker correction: `cbbb27636807b7f195d7d87ccefb476366a07995`. Exact HTTPD, AI docs and Process reruns will be checked on the new SHA.

---
## 2026-11-24 — Bind AdGuardHome stop signals to the managed executable and argv

- `adg_process_state()` previously accepted every numeric PID from `pidof AdGuardHome` as owned by this service. Linux comm names are mutable and do not prove executable identity; another binary with the same comm could be signalled, and a real AdGuardHome launched with a different config/workdir could be stopped accidentally.
- Added `adg_process_identity()`: it verifies `/proc/PID/exe` against `/usr/bin/AdGuardHome` (including the exact deleted-image suffix), parses non-zombie `/proc/PID/stat` start_time, and requires the complete `AdGuardHome -c <configured-file> -w /tmp/AdGuardHome -v` argv tuple.
- Inventory now filters pidof candidates and records `PID:start_time`; both TERM and refreshed SIGKILL snapshots revalidate executable, argv and start_time immediately before each per-PID signal. Wrong-executable/config decoys count as ABSENT; unreadable/malformed proc metadata is UNKNOWN and blocks signaling.
- Added synthetic-procfs host regressions for wrong executable, wrong config argv and malformed stat while retaining multi-PID, refreshed-escalation and bounded-wait tests. Linux 4.4 still has a final validate-to-signal race because pidfd is unavailable.
- Parent main: `a26746d033ffa19d9b576010f4fe4a583b4a859a`. Exact-head AdGuardHome and Process Actions required; no MIPS build/hardware result claimed.

---

## 2026-11-24 14:11 UTC+8 — Preserve router kernel hostname/domain update failures

- `prepare_dns_dhcpd()` already aborts when `update_hosts_router()` returns an error, but the router path ignored both `sethostname(2)` and `setdomainname(2)` failures after atomically publishing `/etc/hostname` and `/etc/hosts`. DNSMasq could therefore proceed with stale runtime kernel identity while the updater reported success.
- Reused the tested `lan_kernel_names_apply()` policy in `net_wan.c`; both syscalls now return positive errno/EIO, both are attempted, and the first failure survives a later lock-release failure. The valid on-disk pair is not rolled back; the caller receives the error and blocks DNSMasq preparation for retry.
- Added Process workflow path coverage for the shared policy/test, a GNU89 ASan/UBSan compile-and-run step, and source-contract assertions for the router adapter, ordering and error propagation.
- Parent main: `9ce145922915fa4b09217b1253a5490a9e2fbda9`. This atomic commit includes `net_wan.c`, the Process source checker/workflow, the WAN DHCP roadmap note, and the live handoff.
- Local verification: GNU89/ASan/UBSan LAN kernel-name policy regression passed; exact push Actions for this SHA were queried directly. Process fail-closed, Process inventory, AI docs, WAN DHCP, WAN route, DNSMasq and related module jobs passed. HTTPD security exposed one stale source-contract assertion (fixed in the subsequent checker-only commit); XY-C1 remained in progress. No L3/L4 result was claimed.

---

## 2026-11-24 — Emit AdGuardHome stop-contract marker after host regressions

- Exact-head AdGuardHome run #45338988643 on `ce1902a95b8cb9cf7c8c938daee2caa1684d9018` passed shell syntax and every runtime regression, including the replacement-PID escalation and the newly reset stuck-daemon fixture.
- The only remaining failure was the Python source checker requiring `AdGuardHome stop signals only validated PID snapshots: PASS`, a success marker the shell test had never emitted. The underlying assertions for multi-PID TERM and refreshed per-PID SIGKILL were present and passing; added the missing summary marker after the full stop test sequence so it cannot print before those assertions pass.
- No AdGuardHome production code changed in this test-contract-only correction.
- Parent main: `17d08de1ed22fe2ff102992d28c974b42b77de3a`. Exact-head AdGuardHome and Process inventory reruns pending.

---

## 2026-11-24 — Complete Adbyby identity-aware start fixture and checker marker

- Exact `5e9ce769a1f274d99da21686f3b9c0f9c7b43a11` Adbyby stop regression passed, including wrong-executable decoy, unreadable stat, real start_time replacement and bounded shutdown. Its Python checker alone failed because it expected the marker `PID start_time replacement` while the test prints `PID start-time replacement`.
- The same exact-head Process regression exposed an outdated start-readiness fixture: it mocked `pidof` but left `ADBYBY_PROC_ROOT` at real `/proc`, so the new executable identity filter correctly rejected its fake PID 123 and printed RUNNING as absent. Added a minimal synthetic procfs entry for PID 123 and readable PID 1; existing absent/delayed/unknown pidof modes remain intact.
- Parent main: `ce1902a95b8cb9cf7c8c938daee2caa1684d9018`. Exact-head Adbyby host and Process runs must be rerun.


---

## 2026-11-24 — Reset AdGuardHome PID fixture after replacement-PID escalation

- Exact AdGuardHome config-security run #45338317817 on `ceda64cdd021684c8b9b876fb29ecafa150a9e81` failed only in the stuck-daemon regression after the preceding fresh-snapshot replacement-PID case.
- The preceding `-9-exits` case intentionally changes `TEST_ADG_PID_OUTPUT` from 24680 to 13579 on its first sleep and expects SIGKILL to target 13579. The next independent `stuck` case forgot to restore its initial PID but still asserted `-9 24680`; production correctly signalled the stale fixture's 13579. Reset the mock PID after the replacement test so the stuck case starts with a clean independent snapshot.
- No AdGuardHome production source changed in this test-only slice. Exact-head AdGuardHome workflow must be rerun.
- Parent main: `5e9ce769a1f274d99da21686f3b9c0f9c7b43a11`.

---

## 2026-11-24 — Preserve Adbyby stop postcondition and correct procfs fixture fields

- Exact SHA `db75aed4c9005bcbdcb4eba503b3c65be8f99644` exposed an implementation truncation: while replacing the pre-signal section of `adbyby_stop_daemon()`, the bounded five-probe post-TERM wait and function-closing brace were omitted. `sh -n` caught the missing brace; Adbyby host, HTTPD firewall-hook staging and Process stop-gate workflows consequently failed before dependent tests. DNSMasq and Process inventory passed; AI handoff passed.
- Restored the five-bounded-probe postcondition: after identity-revalidated per-PID TERM, a fresh RUNNING/ABSENT/UNKNOWN inventory is required; only ABSENT succeeds, UNKNOWN/sleep errors fail closed, and five RUNNING probes refuse cleanup.
- Corrected synthetic `/proc/PID/stat` fixtures: after removing `pid) `, state is token 1 and kernel `start_time` (field 22) is token 20; the previous fixture mistakenly placed its supplied value at token 21. The PID-reuse regression now changes the exact field production validates.
- Parent main: `db75aed4c9005bcbdcb4eba503b3c65be8f99644`. Exact-head Adbyby/HTTPD/Process regressions required; no firmware or hardware claim is made.

---

## 2026-11-24 — Bind Adbyby stop signals to executable and PID start_time

- `adbyby_process_snapshot()` previously treated every positive numeric PID from BusyBox `pidof adbyby` as a validated daemon. `pidof` is a name-based candidate enumerator, not executable identity; a same-name task with another executable could be signalled, and a PID could be reused between inventory and `kill -TERM`.
- Added `adbyby_process_identity()`: candidates must resolve `/proc/PID/exe` to `$PROG_PATH/adbyby` (also accepting the exact kernel ` (deleted)` suffix for an already-running old image) and provide parseable, non-zombie `/proc/PID/stat` start_time. Parsing strips through the final `)` so comm names containing spaces/parentheses do not shift field 22.
- Snapshots now retain `PID:start_time`; stop rechecks executable and start_time immediately before each TERM. Same-name decoys and changed/reused PIDs are not signalled; unreadable metadata fails closed. An empty pidof result counts as ABSENT only when procfs PID 1 metadata is readable. Linux 4.4 still has an unavoidable final validate-to-signal race without pidfd.
- Expanded host regression with fake procfs for multi-PID signalling, wrong-executable decoys, malformed stat data and deterministic start_time replacement. Updated source checker to pin the identity contract.
- Parent main: `ceda64cdd021684c8b9b876fb29ecafa150a9e81`. Exact-head Adbyby/Process Actions required; no MIPS build or hardware L4 result claimed.

---

## 2026-11-24 — Fix Adbyby process inventory source-checker scope

- The Adbyby stop test was repaired in two sequential test-only commits: `c0afda5` moved the per-PID TERM mock out of the shell builtin namespace, and `3462bca` exported `ADBYBY_TEST_PID_OUTPUT` before invoking the external mocked `pidof`, so its `123 456` multi-PID case actually reaches both validated PIDs.
- On exact SHA `3462bcacba47114b93d3d9da0366513064c13dbb`, all Adbyby shell regressions, including the multi-PID TERM case, passed. The workflow then failed only in `scripts/check_adbyby_hosts_config.py`: the pidof assignment/status contract was being searched in `adbyby_process_state()`, while the checked command substitution correctly lives in `adbyby_process_snapshot()`.
- Updated the checker to validate the actual function which owns the checked read. Production Adbyby code is unchanged in this slice. Exact-head Adbyby and Process workflow reruns are required.
- Parent main: `3462bcacba47114b93d3d9da0366513064c13dbb`; no firmware, MIPS build or hardware result claimed for this checker-only correction.

---

## 2026-11-24 — Keep SmartDNS shutdown independent of configuration NVRAM reads

- `smartdns.sh` performed five checked configuration reads (`sdns_enable`, `sdns_port`, `sdns_redirect`, `sdnse_enable`, `sdnse_port`) before dispatching every action. A transient NVRAM backend error therefore exited before an explicit `stop` could acquire the lifecycle lock, load its durable `smartdns_conf.ini` resolver snapshot, remove DNSMasq's managed override, reload DNSMasq or stop the daemon.
- Added `smartdns_load_lifecycle_nvram_state()`: explicit `stop` makes no configuration-NVRAM calls, selects the local disabled cleanup path and relies on `Read_ini`'s saved ownership/port/redirect snapshot. Start/restart/reset retain checked, ordered reads and strict selector rejection with original error-status propagation.
- Added a shell regression for all five independent read failures, stop's zero-read path, valid snapshot loading and malformed-selector rejection; wired it into both push and pull-request DNSMasq lifecycle triggers and the source checker.
- Parent main: `08d59c49ef1080079830e66302516747e06cf82f`. This source/test milestone is THIS COMMIT; exact-head DNSMasq/Process/XY-C1 Actions must be checked after publication. No local MIPS build, live DNS test or hardware L4 result is claimed.

---

## 2026-11-24 — Align the shared stop regression with Transmission's per-PID SIGTERM

- Exact-head USB storage workflow #45336245210 failed only in the shared Transmission/Aria launcher lifecycle regression; the preceding storage C and HDD/spindown checks passed.
- Root cause was a stale test double, not a production-source failure: `Transmission.func_stop()` now sends `kill -TERM <validated-pid>`, but `storage_app_start_status.sh`'s shared `kill()` stub accepted only `-9`. The graceful fixture therefore left PIDs alive and incorrectly drove the stop path into SIGKILL.
- Updated the fixture to record/clear validated per-PID SIGTERM separately from SIGKILL, while preserving Aria2's existing killall graceful-stop contract and post-signal PID revalidation. The first exact-head correction run must confirm the shared storage workflow.
- Parent main: `2d87291cd72a25f90d58d12baea545e01609e4c7`. No firmware source changed in this test-only slice; no new firmware build or hardware result is claimed.

---

## 2026-11-24 — Validate service selectors across userland and repair exact-head regression contracts

- Fixed the Autostart lifecycle test's missing `fail()` helper, then corrected its concurrent-publish fixture so the shell writes a real newline. Process lifecycle Actions #45332907096 and #45332907032 passed on `6bc7b0665719835a6f45e54c3dfd13e3516f6b73`.
- AliyunDrive WebDAV startup now treats only explicit disabled values as a stop request; corrupt selectors fail without stopping or relaunching a healthy daemon. Its lifecycle and Process regressions passed on `5cfb70929934a800f867a7e21179e11fafb9d86c`.
- LAN DHCP now validates `lan_dns_x` as a binary selector in both automatic resolver publication and lease deconfig, before route/resolver side effects. GNU89 policy coverage includes valid values and integer boundaries; WAN DHCP callback stability #45333806032 passed on `d0c7bb76c70f23b18046719d1d8ede572dfca05b`.
- AliDDNS now rejects corrupt startup/runtime enable values without converting them to a stop or terminating its keeper; exact lifecycle #45334063814 and Process #45334063878 passed on `248e045b1526868c9324485bfb18234ca268c7cf`.
- DDNSTO now validates checked `ddnsto_enable` before start/restart, leaving an active daemon untouched on invalid values. Its first regression run exposed a fixture that left enable=0 before later launch tests; the fixture was restored to 1. Exact lifecycle #45334345903 and Process #45334345858 passed on `10ca6cb384a9f83864216f5302d5f44ea59fd0cf`.
- AdGuardHome restart now preflights checked enable-state and accepts only empty/0/1 before stopping the daemon. Exact config-security #45334530232 and Process #45334530159 passed on `e3d7f4b0f347f01da2cf4a84e58bee511934ea41`.
- Adbyby now rejects malformed enable selectors before lifecycle mutations and preserves the original status from each checked NVRAM read. Its regression failures exposed a multiline awk extraction mismatch, then the lost NVRAM status, then a checker-only IndentationError; each was fixed atomically. Final Adbyby regression #45335321139 passed on `ae615df132c429ee58373bfa248be2df4d1e828d`.
- SmartDNS now validates its binary UI enable selector before start/restart/reset while leaving stop usable for shutdown. DNSMasq lifecycle #45335248129, Process inventory #45335248118 and AI handoff #45335248114 passed on production SHA `a89ebc296b817403086e052b3f1fb4a52ed183b8`; USB storage hotplug #45335248143 also passed there.
- Latest production-source SHA `a89ebc296b817403086e052b3f1fb4a52ed183b8` XY-C1 run #45335248142 was IN_PROGRESS at checkpoint: Build running, TRX validation and artifact upload pending. Do not infer L3 from older firmware runs. No local target compile or physical-router L4 verification is claimed.
- The subsequent `ae615df` commit is checker-only (Adbyby source-contract syntax); no product source changed after `a89ebc`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807` remains untouched.
- Next: poll #45335248142's Build, TRX and artifact steps; then continue an independent, evidence-backed stability/security fix while pending checks run.

---

## 2026-11-24 — Preserve the pre-PM GE2 register-map failure boundary

- Switch CLI #45326822109 on parent 49c37472f8e6c1c630382869ef0bbb4b12d492a7 failed at its existing inline contract: the GE2 temporary sysctl ioremap is required to occur before pm_runtime_enable, and the mapping-failure branch must return -ENOMEM before acquiring PM state. The clock helper now correctly unwinds later failures, but the temporary mapping's location had changed that tested boundary.
- Moved the GE2 register mapping/program/unmap block to the top of ei_clock_enable, before PM-runtime and any clock prepare operations. The mapping failure again returns -ENOMEM immediately; once the map succeeds it is unmapped before any later failure edge. Clock and PM partial-failure rollback remains unchanged.
- Exact parent outcomes: AI docs #45326822074 PASS; Switch CLI #45326822109 FAIL at the described source contract; NVRAM #45326822151 IN_PROGRESS; XY-C1 #45326822049 IN_PROGRESS (IAPP firewall lifecycle validation). The correction requires a new exact-head Switch CLI and XY-C1 build/TRX/artifact run.
- Next: inspect exact-head actions, then advance outside RAETH while pending jobs run.

---

## 2026-11-23 — Make RAETH clock preparation and SGMII teardown transactional

- ei_clock_enable ignored every clk_prepare_enable return value and continued after TRGPLL clk_set_rate failure. Probe only logged devm_clk_get error pointers, so a required missing/deferred clock could reach the clock API unchecked; later failure had no tracked partial unwind.
- ei_clock_disable unconditionally disabled clock handles instead of pairing only successful enables. It also omitted Leopard MTK_CLK_SGMII_TOP and all four GE2 MTK_CLK_SGMII1_* clocks. If GE1 and GE2 SGMII were both selected, the shared SGMII PLL could be prepared twice but disabled once.
- Added a successful-enable bitmap and per-clock helpers. Required error pointers now propagate, NULL handles remain a no-op for MT7621's deliberately skipped clock acquisition, duplicate shared PLL enables are suppressed, and every partial failure unwinds enabled clocks plus runtime-PM usage/enable state. TRGPLL set-rate failure is fatal to setup. Teardown releases both SGMII groups and the top clock exactly once.
- Added source-contract checks for ERR_PTR handling, success-only bit tracking, PM-runtime rollback, failure unwind and SGMII disable coverage. No local target compile is claimed; exact Actions on this commit must validate GNU89/MIPS integration.
- Parent main: 3ea3089230ea2696e518c525ec6cf1132aa1ea83. Exact parent Actions #45326081842 AI docs, #45326081376 Switch CLI and #45326081336 NVRAM PASS. XY-C1 #45326081305 had completed host checks but Build was still in progress; TRX/artifact pending. Previous completed firmware Build/TRX/artifact on f7bd670 predates later source changes.
- Next: inspect exact-head checks, then move to an independent userland stability/security defect outside RAETH.

---

## 2026-11-24 — Close RAETH switch-node and primary-netdev teardown ownership

- `rather_probe()` now attaches the registered primary netdev to the platform device. Its error path releases `switch_np`, shuts down debug proc state only after successful proc initialization, releases RX/TX descriptor resources before putting a separately allocated QDMA helper device, and never puts the platform-owned device itself.
- `ei_uninit()` now drops the parsed switch phandle after close/uninit and clears the pointer. `raether_remove()` retrieves the netdev from drvdata, unregisters it first (thereby running ndo_stop/ndo_uninit), releases descriptor state, puts only the allocated QDMA helper device, disables clocks, unmaps MAC/sysctl regions, clears wakeup/drvdata/global pointers and frees the netdev.
- This closes the platform driver remove leak: the old remove path neither unregistered/freed the primary netdev nor released descriptor/MMIO state, and directly invoked the QDMA helper device's release callback instead of dropping its reference with `platform_device_put()`.
- Parent main: `d3ae666f5e46809610e19919fa16b3eb874e68ac`. The earlier preinit slice `615d6be` had AI docs PASS; its Switch CLI failure was caused by an overly broad test assertion, corrected on `65103b60` where Switch CLI #45325386250 **PASS**. NVRAM #45325334304 **PASS**; XY-C1 #45325334328 remained pending at last poll. Current source verification is required on the new head.
- Next source task: prove and cover reverse-order cleanup for all RAETH `rather_probe()` failure labels, especially clocks, mappings and helper-device stages.

---

## 2026-11-24 — Balance MTK GSW probe resources

- Audited `mtk_gsw_probe()`: `irq_of_parse_and_map()` returns unsigned zero on failure, but the old code compared it with < 0; successfully mapped IRQs were not disposed on later probe errors or remove. The pctl phandle was not released, the code tested the phandle instead of the returned regmap for IS_ERR, and pinctrl_get's ERR_PTR result was tested as a boolean while its successful non-devm handle was never put.
- Probe now validates IRQ zero, checks the pctl node for NULL, drops its reference after resolving the syscon regmap, checks the regmap/pinctrl handles correctly, and sends all post-map failures through a common pinctrl/IRQ cleanup path. Remove releases both retained resources. Added source-contract coverage for each ownership edge.
- Parent main: `65103b60d8267e769efbc7aff5395f2662a7d83e`. Preinit source commit `615d6be0f8db31905bda066be38804490b02e45f` AI docs #45325334351 **PASS**; original Switch CLI #45325334356 exposed a checker-only range assertion error. Checker correction `65103b60d8267e769efbc7aff5395f2662a7d83e` Switch CLI #45325386250 **PASS**. NVRAM #45325334304 and XY-C1 #45325334328 were still PENDING at the last poll.
- Earlier production source `f7bd670c88520298c8c4628b1ff06e0c95251f12` had Switch CLI, NVRAM and XY-C1 Build/TRX/artifact **PASS**. No L4 hardware verification.
- Next source task: trace `switch_np` phandle ownership and primary RAETH netdev teardown across `rather_probe()`, `ei_uninit()`, `raether_remove()` and failed `register_netdev()`.

---

## 2026-11-24 — Pair MT7622 switch regulator state and release lookup references

- MT7622 `fe_sw_preinit()` called `of_find_device_by_node()` without checking the NULL result, dereferenced missing platform drvdata, and never released the device reference. It also ignored voltage/enable failures and continued opening the netdev; `fe_sw_deinit()` unconditionally disabled rails even when the matching enable failed or never ran.
- Made preinit return an error, validate node/device/driver-data, and release the held device reference through one exit. Main and MCM B3V regulator state is recorded only after successful enable; voltage/enable/GPIO failures stop open and roll back enabled rails. A failed rollback remains recorded so deinit can retry. Shutdown disables only rails whose enable succeeded and clears state only after successful disable.
- `ei_open()` now treats preinit failure as an open failure: reset FE, release DMA, retry outstanding rail cleanup, release module reference and return the original error. The RAETH checker asserts pdev-reference, regulator and GPIO failure ordering plus caller unwind.
- Parent main: `8f141576497dd6c04938cf89cd95bcc5b29de333`. Exact previous production-source SHA `f7bd670c88520298c8c4628b1ff06e0c95251f12`: Switch CLI #45323341869, NVRAM #45323341860 and XY-C1 #45323341920 **PASS**; the firmware job completed Build, TRX validation and artifact upload. No hardware L4 claim.
- AI docs #45324509191 **PASS** on `81b978`. The later #45324629954 on `8f14157` failed because a duplicated Roadmap snapshot pushed the file to 132155 bytes; this commit removes the duplicate snapshot append from the Roadmap, handoff and root log and keeps the module status concise. Exact-head AI documentation check is required.
- Next source task: audit `mtk_gsw_probe()` resource ownership for pinctrl references, IRQ mappings and parsed DT nodes.

---

## 2026-11-24 — Rotate the root development log and refresh RAETH handoff

- AI handoff run #45323806827 on `7cd1ffd3` advanced beyond the Roadmap-size check, then failed because root `dev_log.md` contained 122 complete dated records; the validator requires the 20–119 rolling window.
- Archived the 102 oldest complete records verbatim, oldest first, to `docs/dev-log/2026/11/part-031.md`; the root retains its newest 20 pre-existing records plus this rotation checkpoint (21 total). Updated the index with exact size and Git blob SHA; no history was rewritten or dropped.
- Parent main: `7cd1ffd3eecc05786846be684053428f20e8f535`. Process lifecycle #45323579958 PASS on `24d204e`; AI handoff #45323806827 failed only at the log-retention check. RAETH Switch CLI #45323341869 and NVRAM #45323341860 PASS on source `f7bd670c88520298c8c4628b1ff06e0c95251f12`; XY-C1 #45323341920 remains IN_PROGRESS. No same-SHA TRX/artifact or hardware L4 result is claimed.
- Next source task: audit `ra_switch.c::fe_sw_init()` / `fe_sw_deinit()` for platform-device reference balancing, regulator/clock pairing and complete partial-failure unwind while independent CI jobs run.

---

## 2026-11-24 — Reduce Roadmap size and preserve current LAN DHCP state

- AI handoff runs #45323474834 (`5df6be1`) and #45323579867 (`24d204e`) both failed solely because `REFACTOR_ROADMAP.md` was 132159 UTF-8 bytes vs the 131072-byte hard ceiling.
- Condensed the LAN DHCP/AP-mode lease row into the implemented behaviors, remaining protocol gaps and exact verification requirements. The core fail-closed BOUND/RENEW transaction, AP host/alias handling, networkmap retry and DHCP-first shutdown state remain represented; historical per-commit details already live in `dev_log.md`.
- Parent main: `24d204e991e1ea1b2f1d1af6bb1c96d327e37ca6`; compacted Roadmap now measures 128495 UTF-8 bytes. Exact-SHA AI handoff rerun required.

---
## 2026-11-24 — Fix the metadata test fixture selected by lifecycle integration

- Process lifecycle run #45323274944 on `017ffdf4` advanced through the C and ASan/UBSan regressions, then failed at `check_process_lifecycle_integration.py`: the assertion for `metadata_policy_rejects_unsafe_files` inspected `shared_pidfile_test`, which is the parser test in `src/tests/`, not the metadata test.
- Added a separate `shared_pidfile_open_test` binding for `trunk/user/shared/tests/shutils_pidfile_open.c` and moved the marker assertion to the correct file. This preserves parser assertions against the parser test while checking owner/nlink/mode rejection in the metadata fixture.
- The `017ffdf4` AI docs, legacy wrapper and process inventory checks passed; its XY-C1 build was triggered and remained pending at last poll. RAETH `f7bd670` Switch CLI passed, while its XY-C1/NVRAM runs were still pending.
- Parent main: `5df6be1d45a53c76f9a20ffa524dd4480be65f67`; fresh exact-SHA process lifecycle and AI handoff runs are required.

---
## 2026-11-24 — Align PID metadata source contracts and XY-C1 header triggers

<!-- 2026-11-23 RAETH DEV LOG -->
## 2026-11-23 — RAETH lifecycle and allocation hardening

Baseline before this documentation commit: `f7bd670c88520298c8c4628b1ff06e0c95251f12`. Backup `backup/pre-plugin-overhaul-2026-11-14` remains frozen at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

Implemented small, separately committed source slices covering PDMA/QDMA RX and TX allocation rollback, RSS/HW-LRO descriptor ring teardown, QDMA FQ/SFQ lifetime, staged DMA initialization rollback, IRQ request tracking and unwind, switch IRQ-source shutdown, open/close worker sequencing, pseudo-netdev registration failure handling, primary netdev `ndo_uninit`/NAPI teardown, MT7621 GP0 clock-enable failure propagation, and removal of cancellation of an uninitialized `reset_task`. Added/expanded `scripts/check_raeth_rx_ring_cleanup.py` and related static assertions.

Exact-SHA Actions for `f7bd670c88520298c8c4628b1ff06e0c95251f12`:
- XY-C1 Firmware Validation: in_progress (run 45323341920)
- Switch CLI ioctl failure-status regression: completed / success (run 45323341869)
- NVRAM transaction boundary audit: in_progress (run 45323341860)

No local MIPS firmware compile is claimed. Next: audit `fe_sw_init()` resource/reference unwind and verify XY-C1 firmware Actions against the latest source SHA.


- Process lifecycle run #45322826402 on `fdc7ac1` passed the strict-GNU89 compile stage but failed later in `check_process_lifecycle_integration.py`: the checker still required inline `S_ISREG(st.st_mode)` even though production now calls `shutils_pidfile_metadata_is_safe(&st, geteuid())`. Replaced the stale assertion with checks for the helper and the complete owner, file-type, link-count and mode policy.
- Made `shutils_pidfile_policy.h` self-contained with `<sys/stat.h>` and added the include contract to the legacy wrapper checker.
- Found that XY-C1 firmware push paths included `shutils.c` but not its newly included policy header. Added the header path and a source-contract assertion to prevent future silent skips of header-only target changes.
- Parent main: `776c84b04a194d0260830eb14f8732df864ee7c3`; this correction requires fresh exact-SHA process lifecycle, legacy-wrapper and XY-C1 runs.

---
## 2026-11-24 — Keep pidfile metadata policy portable to strict GNU89 tests

- Process lifecycle regression #45322580591 caught a test-build incompatibility: under the workflow's `-std=gnu89 -Wall -Wextra -Werror`, glibc did not expose `S_ISREG` because the test did not request miscellaneous feature macros. Production build had `_GNU_SOURCE`, but the header-only policy tests deliberately compile without it.
- Replaced `S_ISREG(st_mode)` with the equivalent `(st_mode & S_IFMT) == S_IFREG`, which is available under the test's declared POSIX/GNU89 environment, and updated the source-contract checker. A fresh exact-SHA process lifecycle run must confirm.
- Parent main: `3e13e241219d08b26aeb57795f553504783c1b06`; no new exact-SHA validation exists yet.

---
## 2026-11-24 — Validate legacy PID-file metadata before privileged signalling

- `kill_pidfile_s()` parsed a PID from a no-follow opened path, but only verified that the opened object was a regular file and below the size limit. It did not constrain file owner, hard-link count or group/world write permission. Since the helper invokes `kill(pid, sig)` with caller privileges, an untrusted PID file can direct privileged signalling at an unrelated process.
- Added metadata policy matching the stricter shared process-pidfile checks: regular file, owner equals effective UID, `st_nlink == 1`, and no group/world write bits. Production `kill_pidfile_s()` now fails closed with `EPERM` before parsing/signalling when metadata is unsafe. Existing no-follow and nonblocking open flags remain.
- Extended the existing host regression to cover safe metadata, wrong owner, hardlink, group-write and FIFO; extended the source checker to require production enforcement. Existing legacy-wrapper PID-file workflow builds this test with GNU89 warnings-as-errors and ASan/UBSan.
- Parent main: `dd5e71103ee4ea1e2b312d358f253f662c0ddc0f`; exact-SHA Actions pending after commit.

---
## 2026-11-24 — Fail closed on malformed LAN DHCP static-DNS selector during deconfig

- `lan_down_auto()` read `lan_dns_x` into its transactional deconfig snapshot but selected static DNS with `atoi(snapshot.static_dns_mode) == 0`. The bound/lease resolver path uses `rc_wan_nvram_parse_int_value()` on the same setting. `atoi("0;reboot")` returns zero and incorrectly enables static-DNS mode, potentially replacing `/etc/resolv.conf` during deconfig.
- Reused the strict integer parser immediately after the snapshot and before route deletion, interface reconfiguration, resolver staging or publication. Parse failures now return the parser status and preserve lease identity for retry. The existing Python source contract verifies the fail-closed path and ordering; its workflow already covers `net_lan.c` and `check_lan_dhcp_deconfig.py`.
- Parent main: `69c15a20e8ce91bf8c31e3ac600547421ba3bfd1`. Exact-SHA WAN DHCP regression and XY-C1 are required for this change; no local target build or hardware result is claimed.

---
## 2026-11-24 — Keep the handoff roadmap within the validator byte limit

- AI handoff run #45321100686 on source SHA `a52ded9e6c0982ff7b838a59b4fb62df3fbd1815` failed only because `REFACTOR_ROADMAP.md` was 131250 UTF-8 bytes, 178 bytes above the 131072-byte hard limit. The new RAETH DMA/RX status row was longer than necessary.
- Compressed that row without removing the source status or verification boundary. The revised file is now below the validator limit; this documentation-only commit needs a fresh exact-SHA AI handoff run.
- Source SHA `a52ded9`: Switch CLI #45321100540 PASS; XY-C1 #45321100612 and NVRAM #45321100619 were pending/in progress at last poll. The docs follow-up does not change firmware inputs and must not be treated as a product-source build.

---
## 2026-11-24 — Add reserved headroom to RSS and hardware-LRO RX buffers

- RSS 4-ring/2-ring initialization and hardware-LRO ring setup allocated skb buffers without `NET_SKB_PAD`, although their DMA mappings start at `buffer + NET_SKB_PAD` and the receive path reserves `NET_SKB_PAD + NET_IP_ALIGN`. This left the initial buffers smaller than the corresponding PDMA/QDMA and runtime replacement allocations.
- Added `NET_SKB_PAD` to all three initialization size calculations. Extended `check_raeth_rx_ring_cleanup.py` to require the correct aligned payload + headroom + shared-info capacity for PDMA, QDMA, RSS 4-ring, RSS 2-ring and HW LRO.
- Parent main: `54f23c27f924b31ad8d3caefc25ed9973a82bd08`. This source/checker slice requires exact-SHA Switch CLI, AI handoff and XY-C1 Build/TRX/artifact verification. No local MIPS build or hardware L4 result is claimed.

---
## 2026-11-24 — Validate RAETH descriptor DT properties and handle allocation failures

- `fe_tx_rx_dec()` ignored every `of_property_read_u32()` result and reused `val`, so missing/malformed descriptor properties could feed uninitialized or stale counts into the driver. Queue×descriptor products and sums were unchecked, and every `kmalloc_array()` result was dereferenced without a NULL check.
- Converted the decoder to return status. GMAC1 queue/descriptor and RX counts are mandatory; GMAC2 queue/descriptor properties must be supplied together and are mandatory when `FE_GE2_SUPPORT` is enabled. Zero/partial counts and 32-bit multiplication/addition overflow are rejected before global descriptor counters are published.
- Checked all TX bookkeeping and RX-ring array allocations. Partial allocation failure releases every pointer already allocated and resets published counters. If a later probe stage fails after descriptor setup succeeds, probe rollback now releases descriptor bookkeeping before freeing the net_device.
- Added a pure GNU89 descriptor policy and ASan/UBSan regression for GE1+GE2 and GE1-only configurations, invalid/missing counts, partial GMAC2 properties, invalid GE2 state, NULL outputs and arithmetic overflow. Wired the header/test into push and pull-request Switch CLI path filters and extended the RAETH source checker.
- Parent `fe690de6011c957bdbb860ff0c792d6236faefde`: rollback-contract correction was just committed; exact-head Actions had not yet been observed. This source/test slice requires fresh exact-SHA Switch CLI and XY-C1 checks.

---
## 2026-11-24 — Fix the RAETH frame-engine rollback source assertion

- The exact Switch CLI run #45314640982 on `2391437add2f2d614accc3264ec3e6d83b13d238` failed in the RAETH probe/MMIO source-contract step, not in compilation or the descriptor work. The code correctly checks `IS_ERR(ethdma_frame_engine_base)`, obtains `PTR_ERR`, logs it, and jumps to `err_free_dev`.
- The checker incorrectly required `goto err_free_dev;` to occur within 120 characters after `PTR_ERR`. The intervening `dev_err()` line made that arbitrary text window too short. Replaced it with an ordered lookup of the rollback jump after the PTR_ERR assignment; this still verifies failure propagation without depending on formatting.
- On parent `2391437...`: AI handoff #45314640970 **PASS**, NVRAM #45314641002 **PASS**, XY-C1 #45314641044 was **IN_PROGRESS** at last poll. New exact-head Switch CLI is required. No L3 claim is made for this parent until XY-C1 completes.

---
## 2026-11-24 — Restore ERR_PTR handling for the RAETH frame-engine mapping

- The exact main commit `f44888c6103f2ff3bf0712b4d1bb38eb683fc331` changed the failure check for `devm_ioremap_resource()` from `IS_ERR()/PTR_ERR()` to a NULL check. Linux 4.4's `lib/devres.c` documents and implements this API as returning an `ERR_PTR()` on failure; an error pointer is non-NULL and would have passed the new guard into later MMIO use.
- Restored the error-pointer check and preserved the original errno for probe rollback. Strengthened the Switch CLI source contract to require the mapping call → `IS_ERR` guard → `PTR_ERR` propagation → `err_free_dev` path, and reject a NULL-only check.
- Parent main: `f44888c6103f2ff3bf0712b4d1bb38eb683fc331`. THIS COMMIT requires exact-head Switch CLI, AI handoff, NVRAM and XY-C1 checks. No local MIPS build, firmware image or hardware validation is claimed.

---

## 2026-11-23 — Make AliyunDrive failed-start cleanup regression race-safe

- The exact-head AliyunDrive lifecycle run `#45312872098` on `3160f80` passed the bundled MIPSel CLI compatibility check, then failed in the new HTTP-503 launched-child fixture's combined assertion. The fixture assumed exactly 11 probes despite the child marker being published asynchronously; the initial process poll can legitimately observe ABSENT before the child becomes visible.
- Reset PID/executable mock modes explicitly and bound the HTTP probe count to 1..`ALD_START_WAIT_SECONDS + 1`, while still requiring the full ten-sleep timeout, exactly one TERM, zero KILL, a launched argv file and no remaining process marker. Failure output reports the observed counters.
- The same CI inspection found the RAETH compile-integrity checker counted only `void` definitions; `fe_chip_name_config()` is deliberately `static int`. Updated the matcher to count exactly one actual function body for either supported return type.
- Both corrections require fresh exact-head Actions. No Aliyun lifecycle PASS or RAETH selector-contract PASS is claimed yet.

---
## 2026-11-24 — Remove a stale checker continuation after matcher correction

- The first follow-up fixed the over-escaped Python regex and source-string literals but left the previous assertion's continuation string behind, which would make the Python checker syntactically invalid. Removed the orphaned line; the selector `all(..., message)` assertion now has one balanced call. No earlier checker run is counted as PASS.

---
## 2026-11-24 — Tighten the RAETH DT-selector regression matcher

- The first source-checker commit accidentally double-escaped its Python regex and C-source newline assertions, so those new assertions would search for literal backslash sequences rather than whitespace/newlines. Corrected the matchers before treating them as validation, and strengthened the selector contract to require `!ret` before every `strcasecmp(pm, ...)` comparison in `raeth_arch_setting()`.
- The source change remains `7f7aee3b5df94c6996743e58834ee2ec4d67eafd`; this checker-only follow-up requires a fresh exact-head Switch CLI run. No test PASS is inferred from the earlier checker version.

---
## 2026-11-24 — Fail closed on missing RAETH device-tree selectors

- `raeth_arch_setting()` reused one `pm` pointer across `of_property_read_string()` calls. When `gmac1-support` was absent, the first `else if (!strcasecmp(pm, ...))` dereferenced an uninitialized/stale pointer; when later selector reads failed, their `else if` branches could interpret the previous property's value as the missing selector.
- Guarded every alternate `gmac1`/`gmac2` and mode comparison with the status of its immediately preceding property read. Missing or malformed optional selector values now skip that mode branch instead of consulting stale data.
- `fe_chip_name_config()` now returns an error on failed or unsupported `compatible`, and probe propagates that error to `err_free_dev`; the failed probe clears `dev_raether` before freeing the unpublished net_device. Extended the existing RAETH compile-integrity checker to pin these error/selector guards.
- Parent `3160f80e4e2c82d2188bddc12f88589898a79327`: AI docs #45312872263 **PASS**, Process inventory #45312872116 **PASS**, AliyunDrive lifecycle #45312872098 **IN_PROGRESS**. This new driver change requires exact-head Switch CLI and XY-C1 validation; no target build or hardware result is claimed.

---
## 2026-11-24 — Clean up a newly launched AliyunDrive daemon that never becomes HTTP-ready

- `ald_start_unlocked()` previously returned failure after a full bounded HTTP readiness timeout but left its newly launched, executable-confirmed daemon running. Subsequent starts then treated it as a pre-existing unhealthy process and refused to launch another copy, requiring manual stop.
- Added `ald_cleanup_failed_start()`, restricted to the post-launch readiness-failure path. It revalidates executable identity, uses the existing bounded TERM/KILL routine only for a confirmed RUNNING child, treats ABSENT as already clean, and never signals UNKNOWN. Existing pre-existing unhealthy-daemon and UNKNOWN-state paths are unchanged.
- The lifecycle shell regression adds a permanently HTTP-503 newly launched child; it requires a failed start, eleven bounded HTTP probes, ten readiness sleeps, one TERM, no KILL, and removal of the process marker. This commit's exact-head lifecycle Action was still running when the next RAETH issue was selected; no PASS is claimed.
- Commit `3160f80e4e2c82d2188bddc12f88589898a79327`: Process inventory #45312872116 **PASS** and AI docs #45312872263 **PASS**; AliyunDrive lifecycle #45312872098 **IN_PROGRESS** at last poll.

---
## 2026-11-24 — Unblock XY-C1 RAETH compilation after exact-SHA build failure

- The exact XY-C1 Action on `edb529c4baf1e60df1fd542a4e8192cf608ffbdc` failed in `drivers/net/ethernet/raeth/raether.o`, before TRX/artifact stages. The compiler reported conflicting `set_ge2_gmii` / `set_ge0_gmii` prototypes (`ra_mac.h` said `void`, while `raether.h` and `ra_mac.c` correctly returned `int`), plus duplicate definitions of `fe_chip_name_config`, `raeth_set_wol`, `raeth_arch_setting` and `fe_tx_rx_dec` in `raether.c`.
- Corrected the stale `ra_mac.h` prototypes to `int`, preserving MMIO setup error values already implemented by `ra_mac.c` and consumed by the driver. Removed the second accidental duplicate helper block in `raether.c`, retaining one complete definition of each helper.
- Added `scripts/check_raeth_compile_integrity.py` and wired it into both push and pull-request Switch CLI workflow path filters. It asserts uniqueness of the four helper definitions and matching `int` declarations/definitions for both GMII helpers.
- On the failed `edb529c` SHA, Switch CLI #45311706551 **PASS**, AI handoff #45311706519 **PASS**, NVRAM audit #45311706453 **PASS**, but XY-C1 #45311706484 **FAIL** at kernel compilation; Build failed before TRX or artifact upload. THIS COMMIT requires fresh exact-head XY-C1 Build/TRX/artifact and Switch CLI checks. No L3 or hardware result is claimed.

---
## 2026-11-24 — Scope the GE2 SGMII switch error gate to its optional feature

- `fe_sw_init()` checked the return status of `set_rtl8367s_sgmii()` outside the `if (architecture & SGMII_SWITCH)` branch. When GE2 forced-2.5G was enabled without the external switch feature, the code could inspect stale/uninitialized `status` and incorrectly abort switch initialization.
- Added braces so the RTL8367 call and its error gate execute only when `SGMII_SWITCH` is enabled. `set_sgmii_force_link(2, 1)` remains outside that inner guard, preserving the non-external-switch GE2 forced-link path.
- Strengthened the Switch CLI source contract to parse the nested brace block and require the call plus `status`/`mdio_error` gate inside it, while proving force-link remains outside. The previous proximity-only assertion passed without establishing lexical control-flow ownership.
- Parent main `f38519f5f92f39148cccdfa6c37eb9126346a982`: Switch CLI #45311619652 **in_progress**; AI handoff #45311620761 **success**; NVRAM audit #45311619611 **in_progress**; XY-C1 #45311619583 **in_progress**. This exact-head Switch CLI pass did not detect the unbraced scope defect. THIS COMMIT requires fresh exact-head Switch CLI/AI checks and same-SHA XY-C1 validation. No local kernel build or hardware result is claimed.

---
## 2026-11-23 — Propagate RStats history read descriptor-close failures

- `f_read()` saved the read errno but ignored `close(fd)`'s return value. A descriptor-close I/O error after a successful or short read could therefore be reported as ordinary data, allowing a questionable persistent-history record to be consumed.
- Added `rstats_file_read_close_result()`: preserves a primary read error, converts close-only failures to `-1` with close errno (or `EIO` if unavailable), and otherwise preserves full/short read counts. `f_read()` now captures read and close errno separately. It deliberately does not retry `close()` after EINTR on Linux.
- Added GNU89 policy cases for full/short success, read-error precedence, close-only failure and missing-errno fallbacks; extended the source contract to require errno capture before checked close and helper return.
- Parent `47100dccc0a7128cbc711b31766fedc82ac787ad`: Switch CLI `#45309913398` and AI handoff `#45309913249` **PASS**; NVRAM audit `#45309913323` and XY-C1 validation `#45309913333` were **IN_PROGRESS** at last poll. This RStats slice awaits its own exact-SHA Actions.

---
