## 2026-11-25 — Restore the exact cross-session execution mandate in handoff

- AI handoff run #45769035962 on documentation commit `80ebd5e1826ead8fe8442edb9c07aba7ea176226` failed because the mandatory checkpoint section was present but the literal `# Cross-session execution mandate` section was absent.
- Restored the required exact heading and explicit atomic-main-commit / before-context-exhaustion rules. The source-SHA verification table remains unchanged; this docs-only correction must pass the AI handoff workflow on its exact SHA.

---

## 2026-11-25 — Adbyby memory-helper process inventory fail-closed correction

- The earlier stop-helper identity check required `/proc/PID/exe` before reading argv. Ordinary kernel threads and other empty-argv non-candidates may have no `exe` symlink; the old ordering classified unrelated entries as UNKNOWN and could block helper cleanup. The identity scan now classifies empty/nonmatching argv as ABSENT first and performs interpreter/cwd proof only for a matching helper argv. Unreadable candidate metadata remains fail-closed.
- Added an empty-cmdline/no-exe regression, retained the exact argv/decoy/interpreter/PID start-time cases, and restored the test-file function boundary so `assert_no_cleanup()` is complete and the daemon stop suite runs exactly once.
- Exact Adbyby run #45768481608 verifies this slice; all other exact-head workflow outcomes and XY-C1 L3 are listed in `SESSION_HANDOFF.md`. No local firmware build or hardware L4 claim.

---

## 2026-11-26 — Distinguish empty proc entries from unverifiable helper candidates

- Exact SHA 44217bb154c0ae26da5c866779702622fb320500 ran the complete Adbyby shell regression suite successfully, but the static contract checker rejected the fixture because it lacked an explicit empty-cmdline/no-exe case.
- Preserve the fail-closed boundary: empty or nonmatching argv is definitely not the optional helper, while a matching helper argv without an executable symlink remains UNKNOWN and must not be signalled. Added dedicated regressions for both conditions and aligned the checker.
- On 44217bb, AI handoff, HTTPD, DNSMasq and Process inventory passed; Adbyby static contract failed only on the missing marker. Process fail-closed and XY-C1 were still running at checkpoint.

---

## 2026-11-26 — Correct Adbyby memory-helper procfs enumeration and CI fixture

- Exact SHA 937b1a96b87c82b1542794262574c6fc56733117 exposed two CI issues: Adbyby #45767811658 stopped at a malformed generated stop-test function block, and AI docs #45767811515 required the exact mandatory handoff checkpoint sentence.
- Rebuilt the regression from the clean parent fixture with bounded insertion points, explicit helper PID events, and no duplicated test body. A numeric proc entry without an exe symlink (for example kernel tasks or an exiting PID) is now a non-candidate rather than an UNKNOWN helper.
- Added coverage for missing exe entries and restored the mandatory handoff wording. The corrected exact-SHA Actions must pass before this slice is treated as verified.

---

## 2026-11-25 — Make Adbyby memory-helper inventory ignore empty non-candidate proc entries

- The memory-helper scan iterated every numeric proc directory but checked `/proc/PID/exe` before reading argv. Ordinary kernel threads have an empty `cmdline` and may have no `exe` symlink; the old ordering classified every such unrelated process as UNKNOWN and prevented safe helper cleanup.
- Reordered identity evaluation to read argv first. Empty or nonmatching argv is a definite non-candidate; only an exact `$PROG_PATH/admem.sh` token or basename/relative token proceeds to shell-executable and (for relative invocation) cwd verification. Unreadable candidate inventory remains UNKNOWN/fail-closed.
- Added a regression for an empty non-helper cmdline with no exe symlink, while retaining exact argv, wrong interpreter/path, malformed stat and PID-reuse tests. Also repaired the concurrent Adbyby test-file insertion that had placed the daemon-test block inside `assert_no_cleanup()`; the single complete daemon postcondition suite remains after the memory-helper fixtures.
- Exact-head Adbyby/Process/AI docs/XY-C1 validation pending; no local firmware build or hardware test claimed.

---

## 2026-11-26 — Bind Adbyby memory-helper teardown to exact process identity

- The old adbyby_close() used ps|grep admem.sh and sent SIGKILL to every matching PID without checking that it was the add-on helper or protecting against PID reuse.
- Teardown now accepts only a shell whose argv contains the exact $PROG_PATH/admem.sh token (or basename relative to the exact program working directory), validates interpreter executable and procfs start_time, revalidates immediately before SIGKILL, and confirms no matching helper remains. UNKNOWN metadata fails closed.
- GNU shell regressions cover absolute/relative helper identity, wrong-path and wrong-executable decoys, malformed proc metadata, PID reuse, and close ordering/first-error handling. Exact-head Actions are pending for this atomic slice.

---

## 2026-11-25 — Make Ralink country-code fallback safe for missing NVRAM values

- `gen_ralink_config()` passed `nvram_wlan_get(..., "country_code")` directly to `get_country_region()`, which called `strcasecmp()` without a NULL guard. The same NVRAM pointer was later passed directly to `strlen()`. An unset WLAN country-code value could therefore crash RC during wireless configuration generation instead of using the existing GB/region-1 fallback.
- Added a shared policy that safely returns region 1 for NULL, empty or unknown country codes, preserves case-insensitive band-specific table lookup, and normalizes missing/non-two-character country strings to the legacy `GB` output. Both band lookups and emitted CountryCode now use the safe normalization path.
- GNU89/Werror/ASan/UBSan regression covers NULL/empty/unknown codes, null tables, zero-length tables, case-insensitive known codes, distinct 2.4/5 GHz regions, and CountryCode fallback. Exact-SHA Wireless #45767215573 PASS (5/5), Process fail-closed #45767215582 PASS (96/96), Process inventory #45767215542 PASS (5/5), HTTPD #45767215454 PASS (53/53), and AI handoff #45767215521 PASS (7/7) on `0699812f6d9cf1934c9206bd3d10dc73e2a206ac`. XY-C1 #45767215486 and NVRAM audit #45767215482 remain IN_PROGRESS; Build/TRX/artifact not yet verified.

---

## 2026-11-25 — Detect bare Adbyby firewall jumps during teardown

- Review of the newly added residual-rule matcher found a valid rulespec it could miss: `-A PREROUTING -j ADBYBY` has no options between the source chain and `-j`, while the original regex required an extra token (and therefore two spaces around the empty match). A leftover rule like this could escape the postcondition if its chain declaration is not present in the captured output.
- Added a dedicated zero-option jump alternative and a regression fixture that emits only the bare jump, independently of the chain declaration and ipset checks. The source checker requires this case. Also removed a duplicated, harmless fixture export line from the prior test-only commit.
- Exact Adbyby and Process reruns pending.

---

## 2026-11-25 — Export Adbyby post-state fixture controls

- Exact Adbyby run #45765982057 on `bd439e46fab885c84a4629088ae8e3efe27a1c49` passed shell syntax and all earlier host/download/lifecycle suites, then failed the new confirmed-absent teardown case. The executable `iptables-save` and `ipset` test wrappers did not receive the state-mode shell variables because they were not exported, so they returned their default stale fixture.
- Exported the two fixture variables before the first call. Subsequent state-mode assignments remain exported and select absent/stale/error cases as intended. Exact rerun pending; no production-source change.

---

## 2026-11-25 — Export Adbyby post-state fixture controls to executable wrappers

- Exact Adbyby run #45765982057 on `bd439e46fab885c84a4629088ae8e3efe27a1c49` passed shell syntax and the prior download/lifecycle suites, then the first idempotent teardown case reported failure. The new state controls were assigned as shell variables but were not exported to the PATH-injected `iptables-save` and `ipset` child scripts, so they fell back to the initial stale-state fixture instead of emitting the intended confirmed-absent state.
- Exported both fixture controls once; subsequent assignments inherit the export attribute and select the requested absent/stale/error output modes. No production code changed in this harness correction. Exact Adbyby rerun pending.

---

## 2026-11-25 — Fix Adbyby firewall teardown regression harness POSIX syntax

- The first exact Adbyby run #45765657822 on `d9d2b515bbb59285130c2d1a1431a80148ea1676` passed the preceding shell syntax, host, lifecycle, process-stop and download regressions, then `firewall_snapshot.sh` stopped at line 190 with `Syntax error: Bad function name`. The fixture attempted to define an `iptables-save()` shell function, but the hyphen is not permitted in a POSIX function name.
- Replaced command-name mocks with the existing PATH-injected `iptables-save` executable fixture and added controlled `list -name` behavior to the `ipset` executable fixture. Tests reset both fixture modes before continuing to the existing firewall setup regressions; output uses portable printf escaping. Production source is unchanged.
- Exact Adbyby/Process/XY-C1 rerun for the new test-only SHA pending. XY-C1 #45765657802 on the previous source SHA was in cross-build; L3 status must be checked per exact SHA.

---

## 2026-11-25 — Verify Adbyby kernel firewall/ipset teardown postconditions

- `del_rule()` previously propagated only private restore-snapshot deletion errors. It ignored every iptables/ipset mutation status and then reported success, even if the PREROUTING redirect, ADBYBY chain, filter references or managed sets remained. The daemon could be stopped while stale redirect/drop state remained active.
- Added a postcondition reader that requires successful `iptables-save` and `ipset list -name`, rejects any remaining ADBYBY chain/jump or references to `adbyby_esc`, `adbyby_wan` or `blockip`, and preserves an earlier snapshot-cleanup failure. Individual deletes remain idempotent/best-effort; verified absence, not an expected delete exit code, determines success.
- Firewall regression covers already-absent idempotency, a surviving chain after an injected failed delete, a surviving ipset, and UNKNOWN post-state read failures. Exact Adbyby workflow and Process inventory validation pending; XY-C1 L3 pending.

---

## 2026-11-25 — Align DNSMasq ARP restart source contract with consolidated locals

- DNSMasq lifecycle #45765299013 on exact source `22562343c3ac2ec40b0c5a535ffa211ba9c605e9` passed the static-host snapshot replay executable, rollback/ordering checks, all preceding DHCP/resolver/lock/identity tests, then failed only in the final source-contract assertion `int arp_status;` for `restart_dhcpd_locked()`.
- The new C function correctly declares the local in a grouped declaration `int stop_status, stop_postcondition, arp_status;`; the checker was still pinned to its earlier standalone declaration form. Updated that exact contract and left production code unchanged. Exact rerun pending.

---

## 2026-11-25 — Restore previous DNSMasq files and static ARP ownership on failed replacement

- `start_dns_dhcpd_locked()` and `restart_dhcpd_locked()` previously discarded the three-file rollback snapshot immediately after clearing old kernel ARP entries, before applying the newly generated `/etc/ethers` set. If an `arp -s` failed, partial new additions were rolled back, but previous static ARP ownership and last-known-good `dhcp-hosts.rc`, `/etc/ethers`, and `hosts.static` files had already been lost; restart could also leave a previously running daemon stopped.
- Added validated replay of immutable saved ARP snapshots, including colon-to-compact MAC conversion and GNU89 regression coverage for success, failed-add reverse cleanup, malformed snapshots and count overflow.
- Start/restart now retain previous file backups until new ARP application succeeds. Clear/apply/stop-postcondition failures restore previous files, remove partial new ownership, replay previous ARP, and relaunch the old daemon only after successful rollback when restart had stopped it.
- Updated the DNSMasq workflow source contract to enforce snapshot → clear → apply → rollback-on-error → commit ordering. Exact DNSMasq, XY-C1 and dependent Actions for this SHA pending. No local checkout/test execution was available.

---

## 2026-11-25 — Remove duplicate XY-C1 self-trigger path

- XY-C1 run #45764744777 on `c39718070e34c3f0c1db9403c599cecf0ed16b63` failed before firmware-input detection: `check_samba_shared_header.py` requires exactly one workflow self-path in the push filter. The workflow already had a self-path at line 538; adding another near the WAN filter created a duplicate. The AdGuardHome package path is valid and retained; removed only the redundant self-path so the filter has exactly one trigger entry.
- AdGuardHome exact-SHA shell/source validation and Process inventory on `190ec54ca9ca81725668fe8c8ccee59affe4f308` PASS. Workflow/Build/TRX/artifact verification for the new path-filter commit remains pending.

---

## 2026-11-25 — Exercise AdGuardHome workdir owner-mismatch rejection

- The persistent-workdir helper explicitly compares the storage directory owner with the launcher UID, but the first regression set covered symlink/type/chmod errors without forcing that branch. Added a deterministic owner mismatch fixture by overriding `id -u` and `stat -c %u` in a subshell; the source checker now requires the fixture.
- This is test coverage only. Exact AdGuardHome/Process workflow rerun pending; XY-C1 firmware source verification is tracked on the workflow-trigger follow-up.

---

## 2026-11-25 — Include AdGuardHome in XY-C1 firmware-validation triggers

- The launcher change affects the firmware-installed `trunk/user/adguardhome/adguardhome.sh` and the package's ROMFS payload, but the XY-C1 workflow push path filter omitted `trunk/user/adguardhome/**`; exact source commits therefore ran AdGuardHome/process tests without dispatching a firmware validation workflow.
- Added the AdGuardHome package tree to the trigger list and added the workflow file itself so future trigger-policy edits can run the workflow. The XY-C1 input detector treats the workflow file as a fail-safe full-build input; the next exact-main SHA should therefore run cross-build, Build XY-C1, TRX verification and artifact upload. This is validation coverage, not an L3 claim until each step succeeds on that SHA.

---

## 2026-11-25 — Restore the shared AdGuardHome process fixture after unsafe-workdir startup

- Exact AdGuardHome run #45764496985 on `09f9ed1cab2cec5ed80659045e4ca8fb1b333471` passed shell syntax, persistent-workdir security, DNS/firewall, lock and redirect regressions, then still failed at duplicate-start. The cause was the new unsafe-workdir startup test: although wrapped in a subshell, it writes the shared state *file* to `stopped`; subshell variable isolation does not roll back file contents. The following duplicate-start test therefore observed ABSENT and attempted the real daemon executable, which is not present in the host fixture.
- Restore the shared process-state file to RUNNING and clear the effect log after the negative-path subtest, then have the source contract enforce that reset. Production launcher behavior is unchanged; exact rerun pending.

---

## 2026-11-25 — Repair AdGuardHome persistent-workdir regression fixture continuation

- Exact AdGuardHome run #45764253186 on `c2129b8af9eb50ec9cf77902ab51f28803a0380b`: launcher `sh -n` and the new persistent-directory symlink/type/mode regression passed, followed by the pre-existing DNS/firewall/lifecycle cases. The shell fixture then failed at the duplicate-start test because the process cmdline `printf` line ended in two literal backslashes, so the command-line placeholders were not passed as the intended two argv arguments and the fake process was classified as absent.
- Corrected the line continuation to one backslash and strengthened the source checker to require the expected `-w "$ADG_WORKDIR"` argument to be quoted as one word. This is a regression-harness fix, not a production runtime change; exact AdGuardHome rerun pending.

---

## 2026-11-25 — Persist AdGuardHome runtime data across service stops and reboots

- The package pins AdGuardHome v0.107.48. Its `-w` working directory stores filter data, query logs and other state, but the launcher passed `/tmp/AdGuardHome` even though it also created `/etc/storage/AdGuardHome`. The former is volatile and was recursively deleted on every successful stop, while the persistent directory was never used.
- The launcher now uses `/etc/storage/AdGuardHome` as its working directory, refuses symlink/non-directory targets, creates missing storage under umask 077, verifies launcher ownership and enforces mode 0700 before firewall/resolver changes. Process identity binds the exact persistent `-w` argument. Stop no longer deletes the work directory or its filter/query/session state.
- GNU shell regressions cover persistent sentinel preservation, exact argv with a path containing spaces, symlink/FIFO rejection, permission failure and fail-before-network-side-effects. The source contract pins persistent workdir preparation and removes the old volatile-directory cleanup.
- The AdGuardHome security workflow, Process inventory and XY-C1 checks for this source SHA are pending; local shell/Python/firmware execution was not available in this GitHub-API-only session. MIPS runtime and hardware L4 remain unverified.

---

## 2026-11-25 — Reconcile FTP firewall state after storage startup even when daemon is absent

- `start_stor_apps()` previously refreshed firewall/UPnP only when a post-start probe found vsftpd running or unknown. When FTP was disabled, or startup failed and rollback left the daemon absent, the function skipped reconciliation even for absent→absent. That contradicted the existing `ftp_firewall_reconcile_needed()` policy and its tests: stale FTP NAT/UPnP rules can outlive a crashed or disabled daemon.
- The FTP storage-start path now snapshots process presence before and after `run_ftp()` and uses the shared tri-state policy. Confirmed absence triggers stale-rule cleanup; any unknown/changed observation is conservative; running→running avoids an unnecessary firewall restart.
- Extended the exact VSFTPD workflow's caller-graph contract to enforce the before/run/after/policy ordering and require the existing absent→absent and unknown-state policy fixtures.
- Historic L3 boundary: XY-C1 run #45762757069 on source SHA `0ef304ddeef0a9e66fb90758e793eaa180138248` completed SUCCESS with Build XY-C1, TRX verification and artifact upload all SUCCESS. Later source commits `8874b60e` and `5df7d87f` are newer than that verified firmware SHA; current-source L3 is not established. Exact VSFTPD, XY-C1 and dependent Actions for THIS commit pending.
- Repository operations use the GitHub API; no local working-tree checkout/test execution was available.

---

## 2026-11-25 — Keep SmartDNS saved resolver-state reads behind enable persistence

- The first exact test for `f5a381bac14579aabc9ad196d5bcb82f1171880e` failed immediately in `dnsmasq_config_preservation.sh`: with `set -u`, the extracted `Start_smartdns()` evaluated `sdns_redirected` before reaching the new NVRAM preflight. Production normally initializes it via `Read_ini`, but the preflight contract should run before even this saved-state read.
- Moved the `smartdns_previous_redirected` snapshot below the checked enable write, and gave the successful test path an explicit saved redirect value. The source checker now asserts NVRAM persist → saved-state snapshot → daemon probe ordering.
- This is a narrow order correction to the previous commit; the prior exact failure was a real regression-test failure, not a firmware/compiler failure. Exact DNSMasq and XY-C1 checks for THIS commit pending.

---

## 2026-11-25 — Persist SmartDNS enable state before touching the running daemon

- `Start_smartdns()` previously stopped the existing instance and removed its managed DNSMasq directives before attempting the `nvram set sdns_enable=1` write; the write was unchecked and silently skipped in the face of NVRAM errors. Startup could then proceed with a running daemon while persistent enable state remained disabled. An empty selector also missed the numeric `-eq 0` branch.
- The enable transition now treats empty/0 as disabled, checks and propagates the NVRAM write before any daemon-state probe or resolver mutation, and accepts only 1 as already enabled. Failed writes return the original status with the daemon and resolver untouched.
- Shell regression injects status 37 for both empty and 0 selectors, asserts no daemon probe is made and memory state is unchanged, then verifies successful persistence precedes the fail-closed daemon-state probe. The source contract checks this order.
- Exact SmartDNS/DNSMasq validation for THIS commit pending; no local execution. Preceding commit `416960b79b0453e206bb585ced2671da996860d4` DNSMasq #45762323209 PASS and AI docs #45762323124 PASS.

---

## 2026-11-25 — Propagate SmartDNS port-53 redirect normalization write failures

- `Change_dnsmasq start` previously ignored `nvram set sdns_redirect=0` when SmartDNS used port 53. It then changed only the shell-local selector and published DNSMasq's `port=0` ownership marker, even if persistent NVRAM still requested redirection. A write failure could report a successful but split-brain resolver transition.
- The normalization now checks and logs the NVRAM status, returns before changing the in-memory selector or staging DNSMasq directives, and lets the existing startup failure path handle the error.
- Added a shell regression with injected status 37 proving NVRAM/local selector and the live DNSMasq file remain unchanged, then verifies the successful normalization path. The Python source contract pins write → local-state update → config-publication order.
- The immediately preceding VPN DNS resolver work established strict 0/1/2 selector validation and correct mode-2 replacement of both IPv4 and IPv6 WAN upstreams. Exact WAN source-checker correction is a separate parent commit; L3 still depends on same-SHA Build/TRX/artifact.
- Exact DNSMasq lifecycle #45762323209 PASS on `416960b79b0453e206bb585ced2671da996860d4`, including the shell run that injects NVRAM status 37; AI handoff #45762323124 PASS and Process inventory #45762323165 PASS. XY-C1 #45762323087 remained in progress before Build at last query; no L3 claim. Local shell/Python execution was NOT RUN.

---

## 2026-11-25 — Match VPN replace-mode checker to the IPv6 resolver guard

- WAN DNS replace-mode implementation in `42eb80db8aea0cdfa1c1b258f59d107d74c07e11` correctly gates the IPv6 WAN-server block with `if (!vpn_dns_replaces_current)`, but the Python source contract still searched for the old comment `/* DNSv6 servers */`. Exact WAN runs #45762008586 and #45762094181 stopped at that obsolete marker; GNU89 tests preceding the checker passed.
- Updated the contract to locate the actual `wan_dns6_snapshot` consumer and require the enclosing replace-mode guard before iterating WAN DNSv6 values. The change is checker-only; no product behavior changed. Exact WAN rerun for THIS SHA pending.
- Same-SHA XY-C1 for the preceding source baseline `19bce65397fbf6eae32f637493221ce9371b2094` was still building at the last poll; TRX/artifact pending.

---

## 2026-11-25 — Reject unsupported VPN DNS modes before resolver publication

- `update_resolvconf()` transactionally reads `vpnc_pdns` with `vpnc_dns_t`, but previously parsed only a decimal integer and then interpreted any positive value as add/replace; values outside the documented 0/1/2 domain could publish an unintended resolver file even though the VPN callbacks rejected them.
- Added a strict WAN resolver selector check immediately after the transaction and before `dnsmasq_file_open()`. Negative or greater-than-2 modes now return `EINVAL`, unlock the resolver mutex, and preserve the last-known-good resolver. Missing/empty keys retain the legacy disabled mode 0.
- GNU89 regression covers modes 0/1/2, -1, 3, backend read errors and invalid output pointers. The WAN source checker asserts the invalid-mode abort occurs before resolver staging. Exact WAN-route and XY-C1 validation for the resulting source SHA pending; no L3 claim.
- This continuation is GitHub-API-only; no local repository checkout/test execution was available.

---

## 2026-11-25 — Complete MiniDLNA daemon identity source contracts and restore live handoff headings

- MiniDLNA source SHA `15f6a0f3eca5c0a157e3bc95edbb09156bc503cd` uses the actual `minidlnad` executable identity for process presence, readiness and rollback. The first storage CI run exposed stale assertions in the workflow caller graph; subsequent contracts were aligned across `.github/workflows/storage-hotplug-stability.yml`, `scripts/check_process_lifecycle_integration.py` and `scripts/check_storage_safe_remove.py`.
- Contract commits: `c8654e1c` readiness selector, `2707f582` process integration rollback identity, `87019bfe` workflow rollback tuple, and `0093302e` safe-remove checker tuples. Exact USB storage hotplug #45761156152 on `0093302e` PASS. The companion XY-C1 #45761156133 was host-only: Build/TRX/artifact skipped, so not L3.
- `87019bfe` AI docs #45761038471 failed because the rewritten handoff omitted the required section headings `## Current repository state`, `## Active task` and `## Verification boundary`. This checkpoint restores the required structure and preserves the archival execution mandate below; exact AI docs validation for this commit remains pending.
- Current main at this record: `cbc867720ed3dcc528e161e16e6006964409cbb3`. Exact ZeroTier #45761309540 PASS after `cbc86772` corrected escaped line-boundary expectations. WAN route #45761312258 was still in progress at last query. Physical router L4 is not verified.

---

## 2026-11-25 — Align the storage workflow rollback contract with MiniDLNA's executable name

- Exact source SHA `15f6a0f3eca5c0a157e3bc95edbb09156bc503cd`: USB storage hotplug run #45760541309 passed its GNU89 sanitizer binaries and earlier shell regressions, then failed in the final whole-storage caller-graph Python contract. The inline checker still expected `minidlna` in the run_dms rollback tuple after the daemon executable, readiness check and rollback were corrected to `minidlnad`.
- Follow-up #c8654e1c corrected the readiness selector, and #2707f582 strengthened the process integration contract for rollback identity. This commit updates the remaining rollback tuple in the storage workflow to `minidlnad`; the unrelated FTP generic daemon policy fixture intentionally retains its test identifier.
- Exact storage-hotplug and AI handoff checks for THIS COMMIT are pending.

---

## 2026-11-25 — Match MiniDLNA lifecycle checks to the installed daemon name

- `is_dms_run()`, readiness polling and failed-start rollback previously used process name `minidlna`, although the package starts `/usr/bin/minidlnad`. That mismatch could report a running server as absent and compromise readiness/rollback accounting.
- Updated the storage service to use `minidlnad` consistently. The process-integration contract now checks presence, wait and rollback against the executable identity.
- Source commit `15f6a0f3eca5c0a157e3bc95edbb09156bc503cd`; MiniDLNA #45760541463, Firefly #45760541292, libdisk #45760541349, NFS #45760541498, storage app #45760541446, Process inventory #45760541368 and related VSFTPD/HTTPD/firewall checks passed. USB storage #45760541309 exposed the remaining stale workflow assertion documented above. XY-C1 #45760541346 was still building at the last check; no L3 claim.

---

## 2026-11-25 — Accept semicolon-free POSIX shell returns in the ZeroTier status contract

- Exact main SHA `1d709141849db219dc2bb237ad2d8a82d452c4e7`: WAN route #45760204592 and ZeroTier lifecycle #45760204650 failed only in the shared source checker, which required the optional semicolon in `return 1;`. The production preflight guards already returned nonzero before signalling the daemon and their shell regression passed.
- Commit `6f6d0fdd2adb616ea90db9a04ec1eff6841eebb2` relaxed the checker to recognize the shell return token. Exact push runs WAN route #45760752161 and ZeroTier #45760752209 both completed SUCCESS on that exact SHA.

---

## 2026-11-25 — Reject truncated procfs lifecycle records and run the shell regression in CI

- The shared process lifecycle fallback previously accepted a newline-terminated but truncated `/proc/PID/stat` observation after checking only PID and task state. It now splits the fields with globbing disabled, restores the caller's IFS/glob mode, requires state through field 22 (20 tokens after the comm) and validates the start-time token as unsigned decimal. Invalid or short records remain UNKNOWN rather than being treated as confirmed RUNNING/ABSENT.
- Added regressions for a short stat record and a non-decimal start time; valid, stale-PID, invalid-state and zombie fixtures now carry the complete stat field set. Source/test commit: `b6ee8a43cec0af29084206b5d65719ab4527f776`.
- The Process workflow previously listed the shell test in path filters but did not execute it. Commit `dc6da082d956c38a35de1518fc3ccac7937403fc` adds explicit `sh -n` checks and runs `trunk/user/shared/tests/process_lifecycle.sh`. Exact Process #45748293107 completed SUCCESS; the added shell regression step completed SUCCESS. Process inventory #45747609939, the process lifecycle regression #45747609955, DNS Forwarder #45747610021 and DDNSTO #45747610055 also PASS on source SHA `b6ee8a43`.
- AI handoff documentation contract #45748293133 PASS on `dc6da082`. Exact XY-C1 #45747610163 on `b6ee8a43` remained IN_PROGRESS at Build XY-C1 at checkpoint; TRX/artifact pending. No L3 claim. Focused local parser checks passed; the complete shell regression was executed by the exact-SHA Actions run.
- This session also reconciled older IAPP/NVRAM gates: #45746217559 (IAPP) and #45746217544 (NVRAM transaction audit) completed SUCCESS on older SHA `27568b72`; this does not establish later-source L3.

---

## 2026-11-25 — Check IAPP event-log append limits and persistence errors

- `IAPP_EventLogHandle()` now rejects a NULL table, invalid event count, negative `ftell()`, and append batches that would exceed the 1024-byte log cap. The size helper divides remaining capacity by record size instead of multiplying an untrusted count. Failed `fwrite()` stops the loop; `fflush()` and `fclose()` errors propagate. Event-log truncation also reports `fclose()` errors.
- Added GNU89 ASan/UBSan coverage for exact-capacity, one-byte-over, already-oversized, zero-record-size and SIZE_MAX arithmetic boundaries; extended the source contract for file-operation ordering/status. Local policy harness PASS. Exact IAPP #45746217559, Process #45746217368 and AI docs #45746217386 PASS on source SHA `27568b72`. XY-C1 #45746217350 Build is IN_PROGRESS; TRX/artifact pending. NVRAM audit #45746217544 is IN_PROGRESS. The optional `IAPP_EVENT_LOG` macro is not enabled in the package Makefile by default, so its production target compile remains unverified.

---

## 2026-11-25 — Restore the IAPP event-log one-second polling interval

- `IAPP_Start()` documented a one-second event snapshot timer but compared elapsed `tv_sec` against 1000, delaying snapshots for roughly 16 minutes 40 seconds. The code now uses an explicit one-second poll policy; tests cover negative/zero and 1/2/999/1000-second boundaries, and the source contract prevents the 1000-second regression.
- Exact IAPP #45745920000, Process #45745919828 and AI docs #45745919824 PASS on source SHA `163e053989c0543baec33c58b3ca4a2f8f218016`. XY-C1 #45745919910 was IN_PROGRESS at last poll; the NVRAM run #45745919901 was cancelled after the next source commit superseded it. Current-source checks are recorded in the entry above.

---

## 2026-11-25 — Reject truncated Linux IAPP datagrams before frame parsing

- The UDP, raw-driver and raw-RRB receive handlers now pass `MSG_TRUNC` to Linux `recvfrom()`, allowing the kernel to report the original datagram size even when the buffer is smaller. Each path checks the returned length against `IAPP_MAX_RCV_PKT_SIZE` before interpreting frame fields. A socketpair regression covers an exact-capacity datagram and a larger datagram while checking surrounding canaries.
- Exact IAPP #45745696836, Process #45745696780 and AI docs #45745696708 PASS on source SHA `fa8947c2de13fa2999c6152c4e22408b22b92cfd`. XY-C1 #45745696794 was still building; NVRAM #45745696727 was cancelled after the next commit superseded it. Current-source XY/NVRAM checks are recorded in the latest entry above.

---

## 2026-11-25 — Bound Linux IAPP event-loop select descriptors

- `IAPP_RcvHandler()` no longer calls `FD_SET` on runtime socket/pipe descriptors before validating them. The Linux event loop rejects negative and `>= FD_SETSIZE` descriptors, updates `SockMax` only from checked registrations, and exits before `select()` on invalid input. The GNU89 ASan/UBSan regression covers fd 0, the last valid fd, negative, `FD_SETSIZE`, INT_MAX, invalid arguments and canary preservation.
- The first source-contract run on `fe7b84c2` exposed an extractor that matched only the forward declaration; `dfc6a98d` corrected it to select the actual function definition and narrowed the Linux branch marker. Exact IAPP #45745181480 and AI docs #45745181481 PASS on `dfc6a98d092e7d56a9cef5e7fbd04bcf1ae3eedc`. Process inventory on the parent source SHA #45745045208 PASS. XY-C1 #45745045235 was still building at last poll; no L3/L4 claim.

---

## 2026-11-25 — Fail closed on malformed VSFTPD share-mode snapshots

- Per-session authorization loaded `st_ftp_mode` with `nvram_get_int()` (`atoi`), collapsing missing/backend errors to zero and accepting numeric prefixes such as `1junk` as mode 1. In the anonymous share path, mode 1 bypasses per-share ACL lookup; the configuration publisher already uses checked reads and normalizes unsupported/missing values to mode 2.
- Read the selector with `nvram_get_checked`, strictly parse the whole decimal string, normalize unsupported valid numerics to mode 2, and refuse the new session on NVRAM or malformed-value errors. Added GNU89 ASan/UBSan coverage for supported/default/error boundaries and exact-source/path-filter contracts. Local GNU89 ASan/UBSan test PASS. Exact-SHA VSFTPD #45742067432, AI docs #45742067378 and Process inventory #45742067392 PASS; NVRAM transaction boundary #45742067411 PASS on source SHA `7f82a3f`; XY-C1 #45742067435 remains IN_PROGRESS. No current-SHA L3/L4 claim.

---

## 2026-11-25 — Update WAN DHCP callback test-name contract after stop-race split

- Exact SHA `50d2df7`: the stop-order checks passed; the WAN callback workflow then failed because an adjacent test inventory still expected the removed one-sided case `test_stop_invalidation_rejects_waiting_old_callback`. The C suite now has distinct failed-stop-preserves-token and successful-stop-invalidates-token cases.
- Replaced the stale inventory entry with both current regression names. No production-code changes; exact-SHA rerun pending.

---

## 2026-11-25 — Fix second stale DHCPv6 stop-order assertion in WAN callback workflow

- The first WAN callback contract repair was still incomplete: the same inline Python step contained an earlier, independent assertion at its line 771 that also required generation advance before daemon stop. The corrected later assertion was reached only after this one passed.
- Updated the early check to require session lock → daemon stop → success guard → generation advance → final unlock. The new DHCPv6 IA regression already passed on `d32af6f`; this reruns the broader WAN callback contract after removing the duplicate stale ordering.
- Workflow contract only; exact-SHA Actions pending.

---

## 2026-11-25 — Update WAN DHCP callback contract for fail-safe DHCPv6 stop ordering

- Exact SHA `d32af6f`: the DHCPv6 IA workflow passed, including the new GNU89 callback-generation race regression. The broader WAN DHCP callback workflow failed only in its inline source contract, which still required generation invalidation before `kill_services()` and expected the former cleanup log wording.
- Replaced that obsolete assertion with the required order: daemon stop → success guard → generation advance → generation-failure unlock/error aggregation; normal return unlock follows. It also verifies an unsuccessful stop preserves its original status while unlocking without invalidating the token.
- Test/source-contract synchronization only; no production changes in this repair. Exact-SHA rerun pending.

---

## 2026-11-25 — Preserve DHCPv6 callback tokens when client stop fails

- `stop_dhcp6c()` advanced the shared callback generation before calling `kill_services("dhcp6c")`. If the daemon stop failed, the still-running client retained its old environment token while queued ip-up/ip-down children were released from the lock with a newer generation; those callbacks rejected themselves and silently skipped lease, address, prefix and resolver updates even though WAN6 teardown correctly aborted.
- The stop path now calls the daemon stop while holding the callback lock and advances the generation only after a successful stop. A failed stop leaves the active generation unchanged, so queued callbacks remain valid; successful stop still fences them before unlocking. Generation-write and lock-release failures retain first-error handling.
- Replaced the old one-sided race fixture with forked failed-stop and successful-stop cases; the DHCPv6 IA workflow compiles the GNU89 ASan/UBSan regression and checks exact production ordering. XY-C1 path filters include the RC implementation, callback policy header and regression. Exact-SHA Actions pending; no MIPS firmware or live-router result is claimed.

---

## 2026-11-25 — Align remaining PPP lifecycle source contracts with post-stop fencing

- Exact source SHA `6c0ea137` passed the WAN DHCP callback and other independent lifecycle checks, but VPN server, Process inventory and HTTPD umbrella checks still encoded the former rule that invalidation preceded `kill_services()`. The VPN checker also required generation advancement before the multi-WAN PPP stop.
- Updated those contracts to require: VPN stop → PPP daemon stop → checked failure/abort branch → callback-generation invalidation; launch generation now follows the successful multi-WAN stop and remains before helper/config publication. The same order is enforced in the VPN, Process, HTTPD and WAN-focused checkers.
- Checker-only follow-up. The production change is unchanged; exact-SHA reruns pending.

---

## 2026-11-25 — Correct post-stop PPP callback fencing source contracts

- Exact SHA `6c0ea137a5e94ab7bbd09c542498c5de52a43bf0`: WAN teardown workflow completed its GNU89 regression binaries but the inline Python contract raised `NameError` because deferred-reconnect marker variables were defined below their first use; its older full-WAN assertion also still demanded the previous pre-kill invalidation order.
- WAN route workflow passed its PPP option policy binaries and failed only because the checker compared the interior `kill_services(...)` token offset against the enclosing `if (...)` offset, which necessarily precedes it. The contract now compares the stop gate and failure return directly.
- Test-contract-only correction; production ordering from `6c0ea13` is unchanged. Exact-SHA reruns pending.

---

## 2026-11-25 — Keep PPP callback generations valid across aborted stop attempts

- Both WAN PPP stop paths advanced the callback generation before deferred-reconnect cancellation, VPN-client shutdown and PPP process termination. If any later gate failed, the old daemon could remain running while its existing ipparam token had already become stale; future ip-up/ip-down callbacks then silently skipped state and route updates.
- Moved callback invalidation after successful PPP callback/daemon shutdown in both teardown paths. The L2TP and multi-WAN launcher paths likewise advance the generation only after the previous transport/pppd stop succeeds, still before shared option-file publication. A failed stop therefore leaves a surviving session's callback identity valid.
- Added source-order regressions for all three paths; PPP option and WAN teardown workflows verify generation advancement follows successful process-stop gates. Exact-SHA Actions pending; no target firmware or live-router result is claimed.

---

## 2026-11-25 — Correct ZeroTier pre-existing-route regression marker

- Exact run #45738322534 passed the complete shell lifecycle regression, including the new pre-existing-route preservation and conflict cases. The Python checker then failed because its required marker said “accepted” while the regression correctly reports “rejected an exact route which was already present.”
- Corrected the expected diagnostic wording only; production and regression logic are unchanged.

---
## 2026-11-25 — Align ZeroTier stop preflight contract with route manifests

- Exact run #45738207061 passed the shell lifecycle regression but the Python source contract still required unconditional NVRAM route preflight before stop. The implementation correctly validates a persisted static-route manifest when one exists and uses checked NVRAM preflight only for legacy no-manifest installations.
- Updated the contract to require the legacy-only guard and preserve its ordering before daemon signalling. No production behavior changed.

---
## 2026-11-25 — Align ZeroTier route ownership source contracts

- The route-ownership fix moved checked selector enumeration and tuple validation into zerotier_static_route_state_save(); the older checker still expected these operations inside zero_route() and expected that rules() rollback used the legacy delete mode.
- Updated the contract to validate the new-start snapshot path, legacy cleanup compatibility, exact-route ownership checks and dedicated rollback operation; added required markers for the pre-existing/conflicting route regressions.

---
## 2026-11-25 — Preserve pre-existing routes during ZeroTier ownership cleanup

- New-start route manifests previously copied every enabled NVRAM tuple before ip route add. If an identical route already existed, a later stop could mistake the user's pre-existing route for a ZeroTier-owned route and delete it.
- The add snapshot now inspects each destination first: exact pre-existing routes are omitted from the owned manifest, absent destinations are recorded, and conflicting/ECMP/unknown route state fails closed before publication. Legacy no-manifest cleanup keeps its compatibility snapshot behavior.
- The rules() rollback path no longer invokes legacy NVRAM cleanup when add preflight failed before a manifest was published. Regression cases cover preservation of exact pre-existing routes, conflict rejection and idempotent no-manifest rollback. Exact-head CI pending; live router runtime remains unverified.

---
## 2026-11-25 — Match ZeroTier route-count checker to the persisted executor

- Exact ZeroTier run #45737539198 completed the shell lifecycle regression successfully, then the Python source contract failed because it still required the pre-manifest NVRAM loop and its local rulesnum variable inside zero_route(). The current executor consumes a persisted manifest; the shared save helper validates and bounds the checked NVRAM count before its seq loop, while manifest loading caps records at 64.
- Updated the source contract to follow the current executor-to-save-helper call chain and assert that shared bound. No production behavior changed.

---
## 2026-11-25 — Reset ZeroTier route-count manifests between compatibility cases

- Exact ZeroTier run on d9bb597 completed the bounded lifecycle tests and stopped in the later route-count table: the empty-string case correctly wrote a header-only route manifest, but the next zero-count case reused that same file and the production add path correctly refused to overwrite an unreplayed checkpoint.
- The count table now removes the known-empty manifest after each successful add, modeling an independent add/stop cycle for every accepted selector. Production checkpoint refusal remains unchanged.

---

## 2026-11-25 — Clear the empty ZeroTier route manifest between independent rules fixtures

- Exact ZeroTier run on 3d586 passed the /32 and NAT-readiness cases but the later partial-route-add case could not reach the route-add stub: the preceding successful NAT-only rules() fixture correctly left a header-only .static-routes-state checkpoint (zero configured static routes), and the next independent fixture reused it.
- Both duplicated test blocks now remove this known-empty manifest before modeling a fresh service start with an enabled static route. Production checkpoint behavior remains unchanged; this is fixture lifecycle isolation.

---

## 2026-11-25 — Isolate ZeroTier rule-publication tests from preserved stop checkpoints

- Exact ZeroTier run on e37ac5d passed the corrected legacy /32 setup but then failed when the later direct rules() fixture reused .static-routes-state and .firewall-state deliberately preserved by preceding stop-failure cases.
- Both copies of the direct rules() fixture now reset the private test configuration directory before their independent checks. This preserves production fail-closed checkpoint assertions and prevents expected leftovers from contaminating NAT readiness coverage; no production source changed.

---

## 2026-11-25 — Restore static-route selectors in the legacy /32 cleanup fixture

- Exact ZeroTier run #45736394732 at SHA ffba20a stopped at the older host-route normalization test after the new manifest cases intentionally reset TEST_STATIC_ROUTE_COUNT and TEST_STATIC_ROUTE_ENABLE to zero. The /32 fixture changed only the destination string, so it no longer configured a route for cleanup and never exercised the intended parser.
- Restored the enabled/count selectors specifically for that /32 case. This is a test-fixture correction; production route-manifest logic is unchanged. Exact-head ZeroTier rerun required.

---

## 2026-11-25 — Persist the exact ZeroTier static routes for later cleanup

- `zero_route add` installed routes from current NVRAM, but `zero_route del` reread current values at stop/restart time. If a user edited or removed a static route while ZeroTier was running, the old destination/gateway was no longer enumerated and could remain in the kernel after firewall cleanup and configuration removal.
- Added a private, same-directory atomically published `.static-routes-state` manifest. It records the interface plus every validated route tuple before any `ip route add`. Stop/restart cleanup loads and validates the exact manifest, not mutable NVRAM; the snapshot is cleared only after all routes are confirmed absent. A malformed or unterminated manifest fails closed. Legacy installs without a manifest retain the checked-NVRAM fallback.
- Extended lifecycle tests for NVRAM route drift, exact old-tuple deletion, truncated manifest preservation and replacement-daemon ordering; source checker enforces snapshot creation/load/clear and stop/replay integration. Exact-head CI pending; no live router route test claimed.

---

## 2026-11-25 — Revalidate VSFTPD LIST symlinks after directory classification

- The listing path authorized a symlink's resolved target, then performed a later followed fstatat() to decide whether the entry was a directory. A link replaced between those operations could inherit the prior target's read ACL and expose an unauthorized share name.
- Preserve the original no-follow entry identity. The directory probe now rechecks that identity and re-resolves symlinks to require the same canonical target before making the entry visible. A deterministic atomic-rename regression proves stale authorization is rejected and a freshly inspected replacement remains valid.
- The exact-head VSFTPD workflow compiles/runs the regression and enforces the source contract; XY-C1 already watches the affected production/header/test paths. Exact-head runs will be recorded after the commit; router runtime remains unverified.

---

## 2026-11-25 — Bound NVRAM rehash to the CRC-validated image length

- `_nvram_init()` checked and CRC-validated only `header->len` bytes, while `nvram_rehash()` put its terminator at `NVRAM_SPACE - 2` and scanned to the physical partition end. Stale/non-FF bytes after the declared image could therefore become settings despite not being covered by the validated CRC.
- Added an overflow-safe helper to compute the final double-NUL offset inside the declared image. Initialization now rejects lengths too short for that terminator, rejects over-capacity images, validates CRC, then passes the bounded offset to `nvram_rehash()`.
- Added GNU89 ASan/UBSan regression cases for short header/data, valid minimum and padded images, exact capacity, over-capacity and `size_t` overflow. NVRAM source checker pins validation/CRC/rehash order; XY-C1 filters now include the kernel driver/helper/test paths. Exact-SHA workflows pending; no hardware flash test claimed.

---

## 2026-11-25 — Repair the DHCP lease-clock integration checker after read-loop hardening

- Exact-SHA DHCP clock workflow on `d5a89c6` compiled and passed its GNU89 ASan/UBSan regression but its source checker failed. The checker still searched for the old `while (full_read(...))` record loop, which had intentionally been replaced by a `for (;;)` loop with explicit COMPLETE/EOF/ERROR classification in the preceding read-framing fix.
- Updated the source checker to assert the new order: timestamp validation, explicit record loop, full-record read, status classification, then clean EOF handling. No production source behavior changed.
- Parent `d5a89c6` read framing, atomic lease writer, AI docs, process regression, USB storage, process inventory and NVRAM workflows PASS. Parent clock C regression PASS/source checker FAIL; XY-C1 firmware build still running at last query. Exact-head result for the repaired checker pending.

---

## 2026-11-25 — Report malformed and truncated DHCP lease-file reads

- `read_leases()` previously treated every non-full `full_read()` result as ordinary loop termination. An empty/partial header, I/O error, or truncated final lease record was silently ignored, making persistent lease corruption indistinguishable from normal EOF.
- Added an explicit read-status classifier and diagnostics. Header EOF/short reads now abort loading with a warning; record EOF ends normally; negative read errors and positive partial/oversized records are reported. Previously completed records remain available when only the trailing record is incomplete.
- Added GNU89 boundary tests for complete records, EOF, short/oversized records and read errors; added a source-order checker and XY-C1 path filters. Exact-head workflow pending at commit creation; no live router test claimed.

---

## 2026-11-25 — Sync DHCP lease directory after atomic replacement

- File fsync plus rename made the new lease data visible atomically but did not guarantee that the directory entry update itself was durable across power loss.
- The atomic writer now opens the destination's parent directory before creating the temp file and fsyncs that directory after successful rename. Parent-directory open failure aborts before changing the live file. A post-rename directory-fsync failure returns a distinct positive status because the new file is already published; `write_leases()` logs the durability warning while still running the post-commit notification.
- Fault-injection coverage adds directory-open failure (old snapshot preserved) and directory-fsync failure (new snapshot visible, durability warning state reported). Local GNU89 test and exact-SHA Actions pending at commit creation; no power-cut hardware test claimed.

---

## 2026-11-25 — Guard atomic lease cleanup after temporary-file creation failure

- Review found that the atomic writer's failure label attempted to unlink the unexpanded `.tmp.XXXXXX` template even when `mkstemp()` itself failed. A pre-existing file with that literal name could be removed.
- Cleanup now unlinks only after `mkstemp()` returned a valid descriptor. Fault-injection regression creates a sentinel at the literal template path and verifies it survives the injected create failure; old destination preservation remains asserted for all pre-rename failures.
- Local GNU89 ASan/UBSan fault-injection test PASS. Exact-head CI pending; no device/storage power-cut result claimed.

---

## 2026-11-25 — Make DHCP lease-file replacement atomic

- `write_leases()` opened the live lease database with `O_TRUNC` and ignored the results of its header/record `full_write()` calls. An interrupted write or storage-full condition could destroy the last valid lease snapshot and cause lease loss after restart.
- It now serializes the current lease snapshot without mutating in-memory entries, writes through a same-directory `mkstemp()` file, loops over partial writes/EINTR, checks `fsync()` and close, then atomically renames only after all preconditions succeed. Failure cleanup closes/unlinks the temp file and preserves the old destination. `notify_file` runs only after replacement succeeds.
- Fault-injection regression covers temp creation, partial write, fsync, close and rename failures and checks the old file remains intact. Local GNU89 `-Wall -Wextra -Werror -fsanitize=address,undefined` test PASS; exact-head CI pending. No device/storage power-cut test claimed.

---

## 2026-11-25 — Preserve DHCP leases across bounded clock rollback

- `read_leases()` computed `time(NULL) - written_at` then cast a negative result to `uint64_t` for the 12-hour age test. A small NTP/backward wall-clock correction therefore looked like an enormous age and discarded every saved lease; the same codebase's `dumpleases` already treats a future write stamp as zero elapsed time.
- Added a bounded time-delta policy: accept nonnegative timestamps within the existing 12-hour window in either direction, clamp a future write stamp to zero elapsed seconds, and continue rejecting larger deltas/invalid negative values. Production lease parsing uses the helper before loading any records.
- Added GNU89 ASan/UBSan boundary tests and an integration checker; XY-C1 path filters include the lease parser, policy header and test. Local policy regression PASS. Exact-head CI pending; no runtime router test claimed.

---

## 2026-11-25 — Keep INIT-REBOOT silent for unknown DHCP clients

- DHCPREQUEST's RFC 2131 text in `dhcpd.c` says an INIT-REBOOT client unknown to the server MUST receive no response. The prior fallback sent DHCPNAK whenever option 50 was present, including when `find_lease_by_mac()` found no local lease.
- The response decision now sends NAK for SELECTING, and for INIT-REBOOT only when a local client lease/static record exists. Unknown INIT-REBOOT and RENEWING/REBINDING paths remain silent. Added policy cases for SELECTING with/without records, INIT-REBOOT with/without records, and renewal/rebinding; the source checker pins the decision before NAK send.
- Local GNU89 `-Wall -Wextra -Werror -fsanitize=address,undefined` policy regression PASS. Exact-head DHCP policy, AI docs and XY-C1 checks pending at commit creation; no client/server/hardware result claimed.

---

## 2026-11-25 — Validate DHCPREQUEST state and field combinations

- `udhcpd` selected DHCPREQUEST lease lookup input from option 50 when present and `ciaddr` otherwise, but did not enforce the RFC 2131 option/`ciaddr` combinations. A malformed mixture could match a lease and receive DHCPACK, extending the lease despite an invalid client state encoding.
- Added a fail-closed check before lease selection and ACK: SELECTING requires server-id plus requested IP and zero `ciaddr`; INIT-REBOOT requires requested IP only and zero `ciaddr`; RENEWING/REBINDING require nonzero `ciaddr` and neither option. The GNU89 regression exercises accepted and rejected combinations, and the source checker pins the guard before lease lookup/ACK.
- Local GNU89 `-Wall -Wextra -Werror -fsanitize=address,undefined` policy regression PASS. Exact-head DHCP policy, AI docs, Process, NVRAM, USB and XY-C1 checks pending at commit creation; no live client/server or L3/L4 result claimed.

---

## 2026-11-25 — Reject DHCPINFORM without a client address

- `send_inform()` constructs an ACK using `packet.ciaddr`. When an INFORM has zero `ciaddr`, the shared sender broadcasts because its destination-selection rule falls back to broadcast when `ciaddr == 0`, exposing configured DHCP options to every client on the segment.
- Added a nonzero-`ciaddr` predicate before `send_inform()`. Tests cover zero, 1 and all-ones values; the integration contract checks that the guard precedes the ACK path. This does not compare `ciaddr` to the IP source address; the existing receive helper discards source-address metadata, and relayed INFORM packets have a relay source distinct from the client's `ciaddr`.
- GNU89 ASan/UBSan test and exact-SHA workflow pending.

---

## 2026-11-25 — Validate DHCPDECLINE and DHCPRELEASE request fields

- BusyBox `udhcpd` documented two unimplemented DHCP server checks in its handlers: DECLINE requires `ciaddr == 0`, and RELEASE must not contain option 50 (Requested IP). The server previously ignored these constraints and could mutate lease state for malformed messages.
- Added small policy helpers and tests for zero/nonzero DECLINE `ciaddr` values and absent/present RELEASE requested-IP option. The handlers now include those predicates before their lease mutation. An integration checker verifies guard ordering and that workflow and XY-C1 path filters include the production source/header/test.
- GNU89 ASan/UBSan regression prepared; exact-head CI pending. No full firmware result for this slice claimed.

---

## 2026-11-25 — Correct BOOTREQUEST checker Python quoting

- Exact `cfde353234ea855d259ab991a4dd310ff81e1d36` BOOTREQUEST workflow #45731440093 compiled and ran the GNU89 ASan/UBSan policy regression successfully, then failed the Python source-contract step with a SyntaxError in a double-escaped assertion matching the log call.
- Replaced the brittle embedded-quote assertion with a check for the distinctive message text `Ignoring non-BOOTREQUEST packet on relay client socket`. Production source, helper and C regression are unchanged. Exact-head rerun pending.

---

## 2026-11-25 — Reject non-BOOTREQUEST opcodes on DHCP relay client ingress

- The client-facing relay loop validated UDP datagram length and DHCP magic but did not validate BOOTP opcode. It proceeded to increment hops, set giaddr and call `pass_to_server()`, whose DHCP option parser accepts both BOOTREQUEST and BOOTREPLY. The downstream BusyBox DHCP server drops non-BOOTREQUEST packets, so bogus client-side BOOTREPLY packets could still reach the relay's XID reservation/forwarding path and consume scarce transaction state without a possible reply.
- Added a strict RFC 1542 BOOTREQUEST opcode check immediately after datagram/cookie validation and before hop mutation, interface IP lookup, option parsing or XID reservation. BOOTREPLY remains valid only on the relay's server-facing reply path.
- Added opcode policy regression cases for 0, 1, 2 and 255; expanded the source contract to require the guard before hop and giaddr logic. Exact-SHA Actions pending.

---

## 2026-11-25 — Avoid XID reservations for one-way DHCP messages

- `pass_to_server()` retained an XID slot for every forwarded message type. However, the DHCP server implementation handles DECLINE and RELEASE without transmitting a reply; relay reply forwarding accepts only OFFER/ACK/NAK. Reserving these transactions therefore consumed one of the relay's 1,024 bounded slots for up to 120 seconds without a possible completion event.
- DECLINE and RELEASE are still forwarded to the server, but bypass XID allocation. DISCOVER, REQUEST and INFORM continue through the existing transaction map because the server replies with OFFER/ACK/NAK/ACK respectively.
- Added a GNU89 policy regression for message types 1/3/8 versus one-way types 4/7, and expanded the Python source contract to require forwarding and early return before `xid_add()`. Exact-SHA XID and XY-C1 checks pending.

---

## 2026-11-25 — Reject oversized datagrams on DHCP relay server ingress; preserve concurrent VPN fix

- The DHCP fix commit `6899af46e9907efc0c40d6f8badc204aba0cf8d9` was created from `d4a965e`. Before its expected-SHA ref update completed, `main` advanced independently to `18cc546ac0eece109a230eb4e81fc019cd52bb59` for the VPN DNS replace-mode fix. The compare-and-swap did not move the ref; after rereading it, the DHCP tree was rebuilt on top of current `main`. The integration commit uses `18cc546` as first parent and `6899af4` as second parent; no force push or concurrent change was overwritten.
- The DHCP server-socket path had used `udhcp_recv_kernel_packet()`/ `safe_read()`, which cannot distinguish a datagram larger than the supplied buffer from one exactly equal to the buffer. It now uses `recvfrom(..., MSG_TRUNC)`, zeroes the packet, checks the full reported length, minimum DHCP option bytes and cookie, and drops invalid datagrams before option parsing.
- Added a GNU89 ASan/UBSan AF_UNIX datagram regression for original-length reporting under `MSG_TRUNC`; expanded the Python receive-path contract to require the server ingress checks while preserving client-ingress assertions. Local test PASS.
- Parent `d4a965e` had all 12 Actions PASS, including XY-C1 Build/TRX/artifact. The concurrent `18cc546` update had 9 PASS workflows and three still IN_PROGRESS at last poll: Process lifecycle #45730461987, NVRAM #45730461818, XY-C1 #45730461930. The new integrated SHA's exact Actions remain to be checked.

---

## 2026-11-25 — Accept the documented VPN DNS replace policy

- The VPN UI exposes `vpnc_pdns=0/1/2` (disabled / add VPN DNS to current list / replace current list). `update_resolvconf()` already preserves this distinction: mode 2 omits WAN DNS fallbacks.
- Both PPTP/L2TP `ipup_vpnc_main()` and OpenVPN `on_client_ifup()` incorrectly used `vpn_client_nvram_read_bool_status()`. Valid mode 2 was rejected as `EINVAL`, preventing the VPN DNS snapshot from being published and leaving stale resolver state.
- Added a strict tri-state policy helper and GNU89 regression for values 0/1/2, missing-key default, malformed/out-of-range values, and backend-error preservation. Both callbacks now use it; source contract requires that integration. Exact-head CI pending.

---

## 2026-11-25 — Align DHCP relay XID source contract with client destination

- Exact `6bfef12` XID workflow #45728716637 failed at `check_udhcp_relay_xid_policy.py`: its integration assertion still required `sendto(..., &item->ip)`, but `pass_to_client()` now constructs a protocol-derived `client_dest` and sends to `&client_dest`. The standalone GNU89 ASan/UBSan XID executable passed.
- Updated the checker to require `client_dest` initialization, client port 68, policy-based address selection and `sendto(..., &client_dest)`. Removed the obsolete BOOTP broadcast TODO and in-place mutation of the saved source sockaddr, preserving the captured source tuple for fallback/retry. Exact-head rerun pending.

---


## 2026-11-25 — WAN IPv6 validation, udhcpc listener recovery and monotonic DHCPv6 IA timers

- `89c92c9` rejects unspecified, loopback and multicast IPv6 interface/next-hop literals and adds a GNU89 address-policy regression. `0622ba5` rejects unspecified and multicast static/DHCPv6 DNS server addresses; `c64608d` aligns the Python source contract with the retained positive `inet_pton()==1` guard. WAN IPv6 DNS exact run #45752311446 PASS on `c64608d`.
- `661e5d0` repairs udhcpc after transient active-listener socket creation failure: the event loop retries the selected raw/UDP listener, limits retry waits to three seconds, and accounts elapsed monotonic time against the existing lease deadline. `f47045a` and `9b0b19e` correct two source-checker assertions without changing production behavior. Exact BusyBox udhcpc workflow #45752972196 PASS on `9b0b19e`; its earlier checker-only failures are superseded.
- `a20fec0cf840bcf8bb697e5a1f5c8673ed12439e` replaces wall-clock preferred-lifetime ageing in DHCPv6 IA `pref_duration()` with monotonic milliseconds. Infinite lifetime remains infinite; finite lifetime saturates at zero and uses whole-second rounding. GNU89 ASan/UBSan regression covers fractional elapsed seconds, expiry boundaries, largest finite value and infinite value.
- Exact `a20fec0cf840bcf8bb697e5a1f5c8673ed12439e` checks: DHCPv6 IA publication #45753771175 PASS; WAN DHCP callback #45753771143 PASS; Process inventory #45753771145 PASS; storage hotplug #45753771153 PASS; NVRAM transaction audit #45753771126 PASS. XY-C1 #45753771146 remained IN_PROGRESS at Build XY-C1; TRX validation and artifact upload had not yet started. No current-SHA L3 claim.
- Main is `a20fec0cf840bcf8bb697e5a1f5c8673ed12439e`; frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`. Next source investigation is the possible DHCP route add/delete ownership mismatch around `control_dhcp_routes()`; validate route restart semantics before patching.

## 2026-11-25 — DHCP route ownership hardening

- Added DHCP route ownership manifest v2 with stable owner prefixes (`wanN_`, `wanx_`, `manv_`). Untracked `EEXIST` routes remain unclaimed, tracked duplicates remain idempotent, and cleanup is scoped to the service owner rather than the current interface or metric.
- Cleanup-by-owner prevents stale DHCP routes surviving PPP/interface/metric changes and prevents WAN teardown from deleting IPTV routes on a shared interface/metric.
- Fixed a manifest parser count bug after adding the owner field (six fields expected), strengthened regression coverage for v2 reload and owner isolation, and aligned WAN route/DHCP callback/HTTPD source contracts.
- Repaired the route-status workflow and its source checker after the internal callback signature changed.
- Main source tree checkpoint: `cc5cde0fcbbb68e62e2a5ad225f9695c9195b3f2`; backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Actions state at checkpoint:
- WAN route operation status regression #45756989235 on `cc5cde0fcbbb`: **completed/failure** (2026-11-25T04:43:58Z).
- XY-C1 Firmware Validation #45756858864 on `8caddb355060`: **in_progress** (2026-11-25T04:42:06Z).
- WAN DHCP callback state stability regression #45756858682 on `8caddb355060`: **completed/success** (2026-11-25T04:42:32Z).
- NVRAM transaction boundary audit #45756858789 on `8caddb355060`: **completed/success** (2026-11-25T04:44:31Z).
- WAN interface teardown stability regression #45756858770 on `8caddb355060`: **completed/success** (2026-11-25T04:42:27Z).
- AI handoff documentation contract #45756858736 on `8caddb355060`: **completed/success** (2026-11-25T04:42:24Z).
- Firmware success is unconfirmed until Build XY-C1, TRX validation and artifact upload all pass for one exact SHA. Physical router runtime remains unverified.

## 2026-11-25 — SmartDNS rollback status propagation

- Parent source checkpoint: `5df7d87f58f00457bc17b4b7b704d0ef1611476f`.
- `8874b60e37b52ae4a33ed609b92e4c2d1be3a660`: fixed `Check_ss()` so a failed `nvram set sdns_enable=0` is returned instead of being masked by `exit 0`; runtime test injects NVRAM status 73.
- `5df7d87f58f00457bc17b4b7b704d0ef1611476f`: consolidated failed-start disable/cleanup; Stop_smartdns is invoked once, cleanup errors outrank NVRAM errors, and the extra unverified DNSMasq restart was removed. Added success, NVRAM failure, cleanup failure, and combined failure regression cases.
- Exact-SHA Actions at handoff checkpoint:
- XY-C1 Firmware Validation: run 45763067333 in_progress/none
- Process lifecycle inventory - 5df7d87f58f00457bc17b4b7b704d0ef1611476f: run 45763067329 completed/success
- DNSMasq lifecycle stability regression: run 45763067219 completed/success
- Process inventory and lifecycle fail-closed regression: run 45763067289 completed/success
- USB storage hotplug stability regression: run 45763067225 completed/success
- Backup branch remains frozen at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
