## 2026-11-28 — Require PID-file identity for WAN monitor readiness

- Review of `detect_wan_main()` confirmed that the detached child calls `process_pidfile_write()` after `daemon()`. A generic procfs scan can see the new child before the PID record exists, so that scan alone is not a strict startup postcondition.
- Replaced the readiness callback with `process_pidfile_read_identity("/var/run/detect_wan.pid", "detect_wan", ...)`. It requires the published PID and recorded start-time to match a live process; ENOENT/stale identity retries as ABSENT, while malformed/unreadable state becomes UNKNOWN and fails closed.
- Strengthened `check_wan_detect_wan_status.py` to enforce the PID-file identity path. Exact-SHA WAN route/ARP and XY-C1 checks for this refinement are pending; no physical-router L4 claim.

---

## 2026-11-28 22:26 CST (UTC+8) — Restore the canonical handoff task heading

- Exact-head AI handoff run #46154469011 on main SHA `296f8a0781c10f51db08655d5683c9ad96496e93` failed because the prior documentation correction renamed the validator-required `## Active task` section to `## Completed source slice`. Restored the canonical heading and clarified that the WAN readiness source change is complete.
- Updated the WAN roadmap evidence: exact-source WAN route #46154268118 and ARP socket #46154268207 are PASS; NVRAM audit #46154268084 is PASS; XY-C1 #46154268098 remains IN_PROGRESS on source SHA `e0efa3a421051d24a21b8519983892ffdef75456`.
- Parent main SHA: `296f8a0781c10f51db08655d5683c9ad96496e93`; result: THIS COMMIT. Local `scripts/check_ai_handoff_docs.py` NOT RUN (API-only session); the exact-head AI documentation workflow is the required validation. Target Build/TRX/artifact remains pending; physical-router L4 NOT VERIFIED.

---

## 2026-11-28 — Keep the handoff roadmap below its byte limit

- Exact-SHA AI handoff check #46154268128 failed because the edited `REFACTOR_ROADMAP.md` reached 131180 UTF-8 bytes, exceeding the 131072-byte hard limit by 108. The validator reported no other defect.
- Condensed the WAN monitoring matrix row without dropping its new detached-`detect_wan` readiness status or the remaining L2/L3/L4 verification boundary. The resulting roadmap is 130807 UTF-8 bytes (265 bytes below the limit).
- Updated `SESSION_HANDOFF.md` with source commit `e0efa3a421051d24a21b8519983892ffdef75456`, exact-SHA workflow IDs/statuses and the next action. Documentation rerun after this follow-up commit pending.

---

## 2026-11-28 — Verify detached WAN gateway monitor startup

- `detect_wan_main()` daemonizes before PID-file publication. The WAN-up caller previously treated a zero `eval("detect_wan")` result as readiness, although that result can belong only to the short-lived daemonizing parent; the child may fail to publish its PID file and exit without the caller noticing.
- WAN-up now routes the launch through a bounded tri-state process-presence postcondition and requires the detached `detect_wan` child to be observed RUNNING. UNKNOWN inventory and timeout failures propagate into the WAN-up result. The sleep callback retains its remaining interval across EINTR.
- Reused the existing GNU89/ASan detached-daemon regression (delayed appearance, absent timeout, UNKNOWN, launcher failure and interrupted wait); strengthened `check_wan_detect_wan_status.py` to enforce the actual production path. Exact-head WAN/ARP/AI/XY-C1 results pending; no physical-router L4 claim. No DHCPACK or storage behavior changed.

---

## 2026-11-28 — Correct duplicate DHCPACK test-registration contract assertion

- The udhcpc host policy binary passed again on #46153023184, but the Python checker then reached a second, duplicate lease-time contract block and failed at line 407. The main() test registration is formatted as `test_lease_time_option_requires_exactly_one_uint32() != 0 ||`; normalize the source and expected token consistently.
- USB storage hotplug workflow #46152870433 passed on descendant `1d74edb` after correcting its stale retry-helper call count. This follow-up changes only the source contract/handoff; no production DHCP behavior changes.

---

## 2026-11-28 — Normalize udhcpc test-registration source assertion

- The exact-head run #46152870458 passed its GNU89/Werror/ASan/UBSan host binary but the Python source contract failed at line 275. The C test is registered in `main()`; only the expected assertion fragment was wrong because the checker had compacted whitespace out of the source and retained spaces in `!= 0`.
- Normalize both source and matcher to verify the actual `test_lease_time_option_requires_exactly_one_uint32()!=0||` condition. The storage workflow call-count fix remains under validation in #46152870433. No production code changed.

---

## 2026-11-28 — Align DHCP source-contract guards with the ACK retry path

- The first checker-only repair passed the host C regression but the exact-head Python checker still failed because it compared a whitespace-normalized policy expression with a spaced expected string. Normalize both sides before testing the absent/present option-length contract.
- The storage hotplug source contract also failed at its assertion that udhcpc contains exactly three retry-helper calls. The added malformed lease-time branch correctly makes four (subnet, lease time, ARP, callback); update the workflow's expected call graph.
- Production DHCP client behavior is unchanged in this commit. Fresh exact-head udhcpc, storage and AI documentation runs are required.

---

## 2026-11-28 — Repair stale DHCPACK lease-time checker contract

- Exact-head udhcpc workflow #46152329755 on `4652b6801ec4c04c0ccfd07c677d7dbec70b39ce` passed the GNU89/Werror/ASan/UBSan host regression and failed only because `scripts/check_udhcpc_script_status.py` still searched for the removed `udhcp_get_option32()` lease-time call. Production now correctly uses the length-reporting option parser.
- Updated the source contract to assert parse → strict status/length validation → retry on malformed presence → omission-only one-hour fallback or unaligned network-order decode. Also check the policy and test registration. No production behavior changed; fresh exact-head Actions pending.

---

## 2026-11-28 — Reject malformed DHCPACK lease-time option lengths

- Found a malformed-presence ambiguity in DHCPACK parsing: `udhcp_get_option32()` returns NULL both when option 51 is absent and when present with a non-four-byte length (including zero). The old code treated all such cases as "no lease time" and silently published a one-hour lease. That fallback remains for a truly absent option, but malformed presence must not be equivalent to absence.
- Switched the ACK path to the length-reporting option parser. Accept only status=absent/length=0 or status=present/length=4; reject malformed parser status and every other length, defer the ACK and use the existing acquisition/renewal retry policy. The four-byte value remains unaligned-safe and network-byte-order decoded; the legacy one-hour omission fallback is unchanged.
- Added GNU89 policy tests for absent, valid four-byte, malformed parser status, zero/one/three/five-byte and contradictory absent-length cases; added source-contract assertions for validation before decode/default. Local isolated GNU89/Werror/ASan/UBSan check PASS; exact-head udhcpc and XY-C1 Actions pending.

---

## 2026-11-28 — Bind Shadowsocks watchdog recovery to executable identity

- `ss-monitor` previously inferred daemon health from `ps | grep` command text. It could count unrelated commands, and its `ssr-retcp/reudp` strings are PID-file labels rather than actual `ss-redir/ssr-redir` executable names.
- Added role-specific targets combining the installed executable and config argv token; TCP, UDP, V2Ray/Xray/Trojan and local proxies sharing one executable no longer mask one another. Counts use the shared complete-procfs identity snapshot; UNKNOWN inventory or foreign same-comm executables log and skip recovery. Regression covers selector mapping, same-executable/different-config processes, text decoys and malformed metadata.
- Local shell/host execution NOT RUN because this API-only session has no checkout. The exact-head Shadowsocks lifecycle workflow must execute the shell regression, syntax checks and source contract; no firmware or hardware validation is claimed.

---

## 2026-11-28 — Throttle zero-interval unlimited udhcpc discovery retries

- Concrete busy-loop: `udhcpc_main()` accepts `-T 0` because `T+` permits zero, while `-t 0` means unlimited discovery retries. In `INIT_SELECTING`, the client sets `timeout = discover_timeout`; with both zero values the select wait becomes zero and the event loop immediately emits DHCPDISCOVER again, consuming CPU and flooding the LAN indefinitely while no offer arrives.
- Clamp the parsed discovery timeout through the existing one-second retry floor before entering the state machine. This preserves `-t 0` unlimited semantics and all positive `-T` values; only a zero/negative interval is throttled. Added GNU89 host policy coverage and a source-contract assertion that normalization occurs after getopt parsing but before `INIT_SELECTING`.
- Local isolated GNU89/Werror/ASan/UBSan regression PASS. Exact-head udhcpc workflow, AI docs and XY-C1 firmware checks pending. No hardware-router L4 claim.

---

## 2026-11-28 — Networkmap ProductID boundary fix reaches exact L1–L3 PASS

- Networkmap ASUS IBOX `ProductID[32]` fix `9fc0a65fdff1a8c315119200efb1976e6e9d181d` is verified: local GNU89/Werror/ASan/UBSan host regression PASS; networkmap source-contract/unit workflow #46149948622 PASS; NVRAM audit #46149948646 PASS; Process inventory #46149948705 PASS.
- XY-C1 workflow #46149948587 completed PASS on the same production-source SHA, including Build XY-C1, TRX verification and artifact upload. This establishes L3 for that exact firmware source tree; physical-router L4 remains unverified.
- AI handoff documentation #46150253122 PASS on documentation SHA `4e5117e18afbd51580f2b05ffa4f1fcf502c9eaf`. Two docs-only validator corrections do not alter the already validated production tree.
- Next source investigation: LAN DHCP client/AP-mode full live leasefail/renew/NAK/deconfig transitions. The `leasefail` script's exit status is intentionally advisory and is already logged centrally; do not change retry behavior without a concrete defect.

---

## 2026-11-28 — Match atomic-commit handoff mandate to exact validator token

- Exact-head AI handoff workflow #46150149400 on `307422cf470385e367e9968735ac08e6d902da6d` passed the checkpoint and archive checks up to cross-session wording, then failed because the required phrase `one atomic main commit` was capitalized in the handoff. Corrected the exact case-sensitive token; no production source changed.
- On the prior production SHA `9fc0a65fdff1a8c315119200efb1976e6e9d181d`, networkmap regression #46149948622 PASS, NVRAM boundary audit #46149948646 PASS and Process inventory #46149948705 PASS. XY-C1 #46149948587 remained in progress at Build XY-C1; TRX/artifact pending.
- Exact-head AI handoff rerun pending on this documentation correction.

---

## 2026-11-28 — Restore handoff accuracy checkpoint wording after exact-SHA validator failure

- Exact-head AI handoff documentation workflow #46149948625 failed on `9fc0a65fdff1a8c315119200efb1976e6e9d181d` because the active handoff omitted the required conditional checkpoint sentence and used a non-canonical active-task heading.
- Restored the exact end-of-session accuracy checkpoint language, `## Active task` section and cross-session execution mandate. This is a validator-driven documentation correction; production source is unchanged.
- At the parent SHA, networkmap packet-field regression/source contract #46149948622 PASS and Process inventory #46149948705 PASS; AI docs failed as above. XY-C1 #46149948587 and NVRAM audit #46149948646 were still in progress. Exact rerun pending.

---

## 2026-11-28 — Bound networkmap IBOX ProductID reads to the packet field

- The IBOX response defines `PKT_GET_INFO.ProductID` as a fixed 32-byte field. `asus_dd_query()` previously passed this peer-controlled field to `is_valid_hostname()` (which calls `strlen()`) and logged it with `%s` without proving a NUL existed inside the field. A malformed response could make those consumers absorb bytes from adjacent firmware-version/mode fields. This is a fixed-field boundary parsing defect, not a demonstrated out-of-buffer read: the receive buffer is zero-initialized.
- Added a bounded NUL-terminator predicate and require it before hostname validation/copy. The debug print now uses a precision equal to the field capacity. Added a GNU89/ASan/UBSan regression proving a NUL in the adjacent field cannot satisfy the ProductID boundary, including final-byte, empty, missing-terminator and null-input cases.
- The infosvr producer uses `snprintf()` with the destination size, preserving valid firmware-generated values. Local isolated sanitizer test PASS; exact-head networkmap and XY-C1 Actions pending. Physical-router L4 remains unverified.

---

## 2026-11-28 — Normalize multiline NVRAM value-slot checker assertions
### WAN/VIPTV DHCP release-before-stop fix

- Found a teardown asymmetry: WAN and IPTV udhcpc are launched without `-R`, but the shared stop helper previously sent only SIGTERM. A BOUND BusyBox client exits on SIGTERM without running `deconfig`; LAN shutdown already documents this behavior and requests SIGUSR2 before termination. WAN/VIPTV stop could therefore leave the DHCP-installed address, routes and lease metadata behind.
- Updated the shared WAN/VIPTV stop helper to request SIGUSR2 using the verified pidfile identity, allow 300 ms for the deconfig callback, and only then terminate gracefully. Missing/exited pidfiles are tolerated; unknown PID identity or a failed grace wait aborts teardown.
- Added GNU89 policy tests for release/wait success, ENOENT/ESRCH, unknown PID identity and wait errors. The WAN stability workflow compiles the new test and checks release → grace wait → termination order for both WAN and IPTV callers.
- Parent main SHA: `eb9b6a54b7116cd88d28fbfd18e237f7de03946e`; result: THIS COMMIT. Exact-head workflow checks pending; local compilation NOT RUN (no local checkout). Physical router behavior L4 NOT VERIFIED.

---


- Exact 93507 NVRAM run #46147635685 passed all CLI/kernel host policy binaries, including maximum assignment and sensitive value-slot replacement/release erasure, then failed the final semantic checker. The check searched for `nvram_sensitive_value_clear_tail(rt->value, rt->val_len, val_len)` as a contiguous substring although production code line-wraps the arguments.
- The checker now removes whitespace from the extracted `_nvram_realloc()` body before testing copy/wipe order. The first raw-regex literal was accidentally over-escaped; THIS COMMIT corrects it from `r"\\s+"` to the actual whitespace class `r"\s+"`.
- Parent main SHA: 13339bed554e334b2e9c0176533d6e4a97c7907e; checker follow-up `9c961e6` used an over-escaped normalizer; result of THIS COMMIT is the corrected single-backslash regex. 13339 AI handoff #46147869419 PASS; 13339 XY-C1 #46147869371 and Process inventory #46147869397 were IN_PROGRESS. 93507 XY-C1 #46147635745 remained IN_PROGRESS at Build XY-C1. Exact NVRAM rerun pending.

---

## 2026-11-28 — Trigger XY-C1 for NVRAM CLI and restore the handoff size limit

- The exact 2b/93507 handoff validator failures were the same measured issue: `REFACTOR_ROADMAP.md` contained 131271 UTF-8 bytes versus a 131072 hard limit. Condensed the NVRAM ledger row while retaining the checked kernel/userland scope, historical scan counts, and explicit external `nvram_get()`/global-atomicity limitations.
- Review of the XY-C1 push filters found a second CI gap: kernel NVRAM driver paths were included but production CLI sources under `trunk/user/nvram/**` were not. Added the subtree and a regression proving `trunk/user/nvram/src/nvram.c` is a production input.
- Parent main SHA: 93507a6a0ecdceb28259127917824b018642ced6; result: THIS COMMIT. On 93507, NVRAM #46147635685 was in progress at final caller audit, XY-C1 #46147635745 was in progress at Build, HTTPD #46147635675 PASS, and handoff #46147635700 failed only on the roadmap size limit. Exact reruns pending.

---

## 2026-11-28 — Erase stale NVRAM value-arena slots on replace and unset

- NVRAM values are appended into a shared kernel arena. `_nvram_realloc()` reused a larger old slot on shorter replacements with `strcpy()`, leaving the previous credential suffix beyond the new NUL; growth allocated a new slot but abandoned the old bytes; `_nvram_free()`/unset freed tuple metadata without wiping the arena slot. Compaction/reset/module-exit erasure happened later and did not protect the mutation interval.
- Added helpers for tail erasure after shorter in-place replacements and full `val_len + 1` slot erasure. Growth now copies into the new slot before wiping the previous slot, so allocation failure leaves the old value intact. Tuple release/unset wipes the full old slot before freeing metadata.
- Extended `nvram_compaction_policy.c` to verify exact wipe lengths, suffix removal, whole-slot release and invalid-range guards; added semantic source assertions to `check_nvram_transaction_boundary.py`. Existing workflow already compiles/runs this host test under GNU89/Werror/ASan/UBSan.
- Parent main: `2b1f66627886472ecd16a3e223505fe10e5a8d4c`. 0827 exact NVRAM CLI secure-free audit #46147228376 PASS; parent CLI assignment change on 2b1f has new exact-head NVRAM #46147499779 in progress. Local host tests NOT RUN in this API-only session; exact-head NVRAM and XY-C1 validation pending.

---
## 2026-11-28 — Allow the complete NVRAM CLI assignment ABI limit

- Found a functional mismatch: the shared parser accepts `NVRAM_MAX_PARAM_LEN - 1` (63) key bytes and `NVRAM_MAX_VALUE_LEN - 1` (4095) value bytes, but `set` and `settmp` each used `char buf[1024]`. Thus valid larger passwords/values were rejected before reaching the kernel.
- Replaced the two small branch-local arrays with one 4160-byte `NVRAM_MAX_PARAM_LEN + NVRAM_MAX_VALUE_LEN` workspace. Both branches wipe the full workspace after each parse/apply attempt, including invalid assignments. Added a max-boundary host regression and exact source guard for the single declaration, both parser callsites and both wipes.
- Parent main SHA: 2bbb25778ea99a2734e7916b4acc989ad38dc74a; result: THIS COMMIT. Local tests NOT RUN (no local checkout); exact-head NVRAM and XY-C1 verification pending.

---

## 2026-11-28 — Correct the NVRAM secure-free checker and source formatting

- Exact cd6 NVRAM run #46146966803 passed the new CLI sensitive-buffer erasure GNU89 regression and every existing NVRAM host test, then failed the added source guard. Its helper marker encoded a literal `\\n`, and the regex `r"\\s*"` sought a literal backslash, so the checker falsely reported missing wipes.
- Changed the helper check to a newline-independent signature fragment, corrected the whitespace regex to `r"\s*"`, and restored the expected indentation on every secure-clear/free pair. The source wipe coverage and buffer lengths are unchanged.
- Parent main SHA: 3652a142a82fd540a35bd871dd78b7e27d72208c; result: THIS COMMIT. Local build remains NOT RUN; exact-head NVRAM and XY-C1 Actions required.

---

## 2026-11-28 — Erase NVRAM CLI profile secrets before heap release

- Found credential-bearing buffers in `nvram_save()`, `nvram_restore()` and `nvram_show()` that were freed without explicit erasure. The restore profile is decoded in place and permanent/temporary snapshots contain the existing NVRAM credential set; the show/save getall buffers also carry NVRAM values.
- Added `nvram_cli_secure_clear()` using volatile byte stores. Every free path for the save buffer, all six restore profile exits, both restore snapshots, and both show-buffer exits now wipes the complete allocated span immediately before `free()`.
- Extended the GNU89 CLI policy regression to verify full-region zeroing and NULL/empty handling, and extended the NVRAM tree checker to pair every free in those three functions with the correct full-span wipe. The existing NVRAM Actions workflow already compiles and executes this host test.
- Parent main SHA: a606c54b09cda3fb70560e44571281ddd49c7f92; result: THIS COMMIT. Local host execution NOT RUN (no local checkout); exact-head NVRAM Actions and XY-C1 Build/TRX/artifact are pending. No L3/L4 claim is made.

---

## 2026-11-28 — Finish compacting Samba V2 recovery-order source assertions

- Exact-head a54 Samba #46146444805 progressed past the V2 tracking-flag matcher fixed in the previous follow-up, then failed because two V2 rollback order snippets still contained whitespace (`passdb_path, passdb_snapshot` and `group_path, group_snapshot`) despite the source body being normalized by `compact()`. Removed that whitespace; the checks still enforce reverse restore order config → passdb → group → passwd.
- Exact a54 AI handoff #46146444809 PASS. Production-source SHA `670e513d99fa0e2d48dfc382ea795ab5c8f6ba1a` XY-C1 #46145600745 SUCCESS (Build, TRX and artifact upload). This commit changes only the Samba checker and handoff/log.

---
## 2026-11-28 — Normalize remaining compact-body assertions in Samba V2 checker

- Exact-head a2 Samba job #46146271742 passed its C transaction regression, service failure-gate and account snapshot parity checks, plus the first two durability-checker sections; it then failed because the checker compared a whitespace-bearing expectation against a function body that had already been compacted.
- Normalized the V1/V2 tracking-flag assertions, V2 restore argument-order snippets and legacy recovery-call signature to the same no-whitespace representation used by compact(). This preserves the underlying state/ordering checks rather than suppressing them.
- Parent main SHA: a2f9fcb43a27ecec971ceea2b61f6ec8e8b0fb02; result: THIS COMMIT. Local execution remains NOT RUN because there is no local checkout. On the parent SHA, handoff validation #46146271744 PASS and HTTPD #46146271738 PASS. Fresh exact-head Samba/HTTPD/AI Actions are required.

---
## 2026-11-28 — Correct the final V2 Samba and HTTPD contract assertions

- Exact-head `b7f6cc478d495488a4ad3d9398ef5f5ac2e5fc57` Samba run #46145970956 passed the transaction C regression, account snapshot guard and earlier durability checks, then exposed a checker mistake: PASSWD/GROUP are fields inside one snprintf format string, so the correct contract is to require the field tokens without an extra leading quote. Repaired `scripts/check_samba_transaction_durability.py`.
- HTTPD #46145970832 failed only at its duplicate inline Samba ACL assertion expecting two `write_smb_conf_unlocked()` calls inside `run_samba()`; corrected it to one journaled publication. AI handoff #46145970861 failed because the new handoff header omitted the validator's required `## Current repository state` section; restored the canonical headings and retained the mandatory checkpoint/cross-session execution sections.
- On exact SHA `b7f6cc`, Process inventory #46145970859 PASS; XY-C1 #46145970921 SUCCESS but Build/TRX/artifact were SKIPPED (no firmware-input changes); NVRAM audit #46145970994 was IN_PROGRESS. Production-source SHA `670e513d99fa0e2d48dfc382ea795ab5c8f6ba1a` XY-C1 #46145600745 was still building at last check.

---
## 2026-11-28 — Repair V2 Samba journal regression expectations and source guards

- Diagnosed exact-head 5fc failures. The V2 COMMITTED-before-cleanup test correctly retains the marker generation, but subsequent first-boot and legacy-V1 rollback checks still expected the previous committed generation; updated them to assert restoration of the latest marker generation and normalized marker literals to newline escapes.
- The Samba workflow's account-snapshot source guard expected two config-writer calls inside `run_samba()` although the function makes one transactional call. Updated it to exactly one. Updated HTTPD identity and the standalone Samba durability checker to require the V2 `read_state_with_accounts` / `recover_with_accounts` / `begin_with_accounts` / `commit_with_accounts` / `abort_with_accounts` lifecycle in `run_samba()`, while retaining V1 checks for legacy callers.
- Parent main SHA: `5fc78cc0e312610ef27c61e42fb80d42b0de520a`. Exact-head Samba, HTTPD and XY-C1 validation is pending; the failing 5fc XY-C1 run stopped in the Samba transaction regression before firmware build.

---
## 2026-11-28 — Cover the V2 COMMITTED-before-cleanup crash window

- Extended the four-file Samba transaction regression to simulate power loss after the durable V2 COMMITTED marker is visible but before snapshot cleanup. Recovery must retain the new passdb, smb.conf, passwd and group contents rather than rolling any of them back.
- The existing regression also exercises PREPARED rollback, all-file commit, missing-original group removal, and V1 journal migration.
- Parent main SHA: `670e513d99fa0e2d48dfc382ea795ab5c8f6ba1a`. Exact-head host/Samba/XY-C1 verification pending.

---

## 2026-11-28 — Harden V2 journal file sync and function-specific CI guards

- Hardened the V2 trailing-byte read against EINTR and changed account/config regular-file synchronization to open writable descriptors before fsync, avoiding filesystems that reject fsync on an O_RDONLY descriptor. Existing no-follow, owner, inode and mode checks remain in force.
- Corrected the Samba embedded regression to select the account-aware transaction APIs specifically for `run_samba()`, while leaving other legacy two-file transaction callers on V1. Added explicit begin-before-passwd-before-credentials and config-before-commit assertions.
- Made the XY-C1 source guard select V2 recovery by inspecting the function body rather than relying on a loop-variable name.
- Parent main SHA: `dce5f27b1787abbf83a712159358215839c3cadc`. Exact-head host/Samba/XY-C1 verification pending; no router test claimed.

---

## 2026-11-28 — Journal Unix passwd/group with Samba passdb and smb.conf

- Found a second-order consistency defect in `run_samba()`: the account snapshot refactor updated `/etc/passwd` and `/etc/group` before the existing durable journal began. If `samba_prepare_private_passdb()`, `samba_run_smbpasswd()`, or smb.conf publication failed, the existing rollback covered only passdb/config and left the newly written Unix account files behind.
- Added V2 PREPARED/COMMITTED journal state with explicit original-file flags for passdb, smb.conf, passwd and group; recovery accepts legacy V1 journals but restores V2 in reverse publication order. V2 cleanup keeps all four rollback snapshots until the marker removal is durable.
- Moved the V2 begin point before `recreate_passwd_unix_snapshot()` and made commit/abort/recovery unconditional for both APP_SMBD36 and legacy Samba builds. Commit fsyncs passdb, smb.conf, passwd and group plus their parent directories before publishing COMMITTED.
- Added host regressions for four-file rollback, four-file commit, removal of a previously absent group file, and V1-to-V2 recovery compatibility. Updated Samba/XY-C1 source contracts to require transaction coverage before passwd generation.
- Parent main SHA: `7998946773b259ed78903896d041e87c260f364c`. Exact-head Samba, Process/AI and XY-C1 checks pending; no physical-router validation claimed.

---

## 2026-11-28 — Split HTTPD duplicate-account and NVRAM-key assertions by function

- Exact-SHA HTTPD regression after the snapshot-body extractor update reached the intended function, then failed because its final assertion still expected the NVRAM key format string `acc_username%d` inside the snapshot consumer. The consumer receives an already copied list; the compatibility wrapper now builds that key and snapshots it under one transaction.
- Corrected the regression to require duplicate-account filtering in the snapshot consumer and the indexed NVRAM-key format plus checked-copy read in the wrapper.
- Parent main SHA: cbbe470cce2e09ff94056c24d8f5682cb2bfaf6c. Re-run exact-head HTTPD security regression; no production code changed.

---

## 2026-11-28 — Update HTTPD account-identity contract for the passwd snapshot consumer

- Exact-SHA HTTPD security regression on the Samba snapshot change passed all earlier HTTPD and file-security checks, then failed in the final Samba/Unix identity contract: it still extracted `recreate_passwd_unix()` through `_eval`, but that wrapper now only snapshots NVRAM while reserved-name and /etc/passwd generation logic lives in `recreate_passwd_unix_snapshot()`.
- Updated the contract to validate account-name safety in the snapshot consumer and independently assert that the compatibility wrapper opens/ends one transaction and routes reads through the transaction callback.
- Parent main SHA: 4bc94f4fbf25492ec9cea221ab81f477ae79648c. Exact-head HTTPD/Samba/Process/XY-C1 checks remain the verification boundary; no production code changed in this regression-maintenance commit.

---

## 2026-11-28 — Align Samba ACL/passwd failure contract with snapshot API

- The corrected Samba workflow syntax ran successfully on SHA 369afd4a and its first Samba staging/failure-gate step passed. Its second passdb/ACL parity step still searched for the retired `recreate_passwd_unix(0)` call, causing a ValueError before it could check the existing failure gate.
- Updated that guard to anchor on `recreate_passwd_unix_snapshot()`; subsequent assertions still require password database rebuild failure to abort before passdb creation, smb.conf publication, cleanup or daemon startup.
- Parent main SHA: 369afd4a220190f0878b2cf120ecc06ec7c55325. The next exact-head Samba workflow run must execute both static steps and the transaction durability checker. No production C code changed in this CI contract commit.

---

## 2026-11-28 — Repair Samba workflow embedded-script indentation

- Exact-head validation found the Samba workflow file itself became invalid YAML: four newly added source assertions were inserted without the ten-space block indentation used by the embedded Python run step. GitHub created a workflow run named by the file path and no jobs were exposed. The assertions are retained and correctly indented inside the Python block.
- This is a CI harness syntax repair only; no production C behavior changed. Parent main SHA: 0a423aad49a2cfac0789f5f2bdc50d45618c04bb. The corrected Samba workflow and the preceding optional-app checker update must be rerun on a new exact SHA.
- Samba snapshot source SHA 9ffd and its follow-up a78 had XY-C1 jobs still in progress at the last poll; their exact build/TRX/artifact result remains outstanding.

---

## 2026-11-28 — Align remaining firmware/Samba regressions with implemented paths

- Exact-SHA CI on a78e02ff exposed two remaining stale source assertions: Samba's lifecycle contract still expected the old recreate_passwd_unix(0) call rather than the explicit snapshot consumer; Process lifecycle's optional-app checker still expected the old service-stop errno assignment instead of the ten-operation first-error helper.
- Updated Samba's contract to check recreate_passwd_unix_snapshot() and require router identity to be read unconditionally before the optional SMB-account loop, including the zero-account case. Updated the DNS Forwarder/optional-app contract to require exactly ten first-error captures, with service-stop capture after that operation and errno restoration before abort.
- Parent main SHA: a78e02ff07c7ea0686d666cd783b754b3b36c27f. Exact-head Samba, Process lifecycle and XY-C1 checks for this follow-up are pending. No local firmware or hardware test was performed.

---

## 2026-11-28 — Complete the Samba shared-account snapshot contract

- Exact-SHA CI on `9ffd787fddc38055a7b0f9af89b7aa7c0082035d` exposed stale integration details after the passwd-writer refactor: Samba workflow still expected the old `recreate_passwd_unix(0)` call; Process regression left file-publication assertions scoped to the compatibility wrapper instead of the snapshot consumer; and the checked-getter adapter was now unused.
- Updated both source-level contracts to check `recreate_passwd_unix_snapshot()`, moved password file publication assertions to that function, and removed the dead static adapter.
- Preserved the configured router identity when SMB account count is zero: `run_samba()` now copies `http_username` unconditionally under its already-held NVRAM transaction. A missing key still yields an empty copied value, and the snapshot consumer falls back to `SYS_USER_ROOT`, matching prior boot behavior.
- Parent main SHA: `9ffd787fddc38055a7b0f9af89b7aa7c0082035d`. Exact-head Process/Samba and XY-C1 checks are pending; the previous SHA's Process inventory and AI handoff checks passed. No local firmware or hardware test was performed.

---

