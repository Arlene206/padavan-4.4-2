## 2026-11-21 — Keep HTTPD/Samba security workflow aligned with generic FTP readiness callbacks

- Parent main SHA: `d83cb745c4a0f9b502cc5a4d8c13e3e2cfac98da`; HTTPD Authentication and Security run #44880376165 failed in the Samba/FTP trust-boundary source checker with `ValueError: substring not found`.
- The checker still used the old end marker `ftp_daemon_is_running`, which was intentionally replaced by the shared `storage_wait_for_daemon` adapter when the FTP wait policy was generalized for MiniDLNA/Firefly. Updated the FTP configuration slice boundary to the surviving `ftp_wait_for_daemon` wrapper; the configuration assertions themselves are unchanged.
- This is a checker-only alignment, not a security-policy relaxation. The exact-SHA HTTPD/Samba security workflow must be rerun.

---

## 2026-11-21 — Wait for detached MiniDLNA and Firefly children before declaring storage startup success

- Parent main SHA: `1fd04b9224460af23f6f63fa667c95e0edcbc045`.
- Fixed a remaining race in `trunk/user/rc/src/services_stor.c`: MiniDLNA is launched without its foreground/debug option and mt-daapd without `-f`, so both daemonize. RC previously inspected `/proc` only once immediately after `_eval`/`eval`; a child not yet scheduled could be classified ABSENT and trigger a false rollback.
- Extracted the existing FTP stable-presence policy into generic `service_daemon_wait_policy.h` while preserving FTP's specific failed-start policy. The common wait retries confirmed absence, requires a second RUNNING sample after 500 ms, and fails closed on UNKNOWN / timeout. MiniDLNA and Firefly now wait for their own daemon processes after successful launcher return; failed launcher returns still use immediate inventory/rollback. The primary process error is preserved explicitly through logging/rollback.
- Extended the generic policy regression for `minidlna` and `mt-daapd`, and added workflow static contracts for both call sites and the fail-closed wait. Exact-SHA storage Actions will execute GNU89 policy compilation and the full storage hotplug regression; no firmware or physical-router result is claimed.

---

## 2026-11-21 — Fix delayed-daemon fixture marker detection

- Parent main SHA: `2647a9e4e90828fbae69d0f2d63413acc3908e1a`; the diagnostic rerun (#44879745837) reproduced the failure: the mock sleep did publish the delayed-child marker, but the mock `pidof` checked it with `-s`. The marker is intentionally an empty file, so `-s` incorrectly kept reporting ABSENT and made successful simulated startup time out.
- Changed the fixture's presence checks to `-e` (existence, not nonzero file size) and corrected the fake launcher's PID marker. The production bounded wait logic is unchanged. Failure-only launch output/trace remains available for any later regression.
- The c82 run therefore failed in the test fixture, not because of a source syntax or target compile error; re-running all storage checks on this new exact SHA.

---

## 2026-11-21 — Isolate Transmission/Aria daemon-readiness CI failure

- Parent main SHA: `331d94f73dd21eeb49a806357ec6f560ee6afac0`; storage-hotplug run #44879557620 failed in the first new Transmission successful-start regression. The Action output identified the failing assertion but the launcher output had been redirected away.
- Added failure-only launcher output and a bounded pidof/sleep/launch trace to expose the exact post-launch transition on the next exact-SHA run; corrected the fake launcher's PID marker formatting. This is test diagnostics only and does not alter service startup behavior.

---

## 2026-11-21 — Verify detached Transmission and Aria2 children before successful storage-app startup

- Parent main SHA: `b0381ae36cc38f2963448681d28f97ecf41a5202`.
- Fixed a second daemonization race in the storage applications: Transmission launches `transmission-daemon` and Aria2 explicitly passes `-D`, but both wrappers previously returned success as soon as `start-stop-daemon` returned. The storage RC caller then immediately inventoried the daemon, so a child not yet scheduled could be treated as absent and unnecessarily rolled back.
- Both wrappers now require a valid `pidof` PID list after launch, with four bounded probes (three one-second waits). Confirmed absence is retried; malformed, inconsistent or failed `pidof` is UNKNOWN and fails immediately. This keeps the existing storage rollback path authoritative instead of reporting a false success.
- Expanded `storage_app_start_status.sh` with delayed-child success, no-child bounded-timeout and post-launch UNKNOWN cases for both applications. The existing configuration, stop and concurrency cases remain enabled. Exact-SHA Storage Actions will provide the host/BusyBox runtime results; no firmware target build or router test is claimed.

---

## 2026-11-21 — Align NFS source contract with daemon readiness regression

- Parent main SHA: `c7e52aac41df195d68bcf49c9cd33e2153b2810a`; NFS daemon lifecycle run #44879119916 failed only in its final Python source-contract step. Shell syntax, the full NFS start/reload integration (including the delayed rpc.mountd child test), IPv4/IPv6 export-policy binary, and EINTR-safe service-wait policy all passed.
- Root cause: the static checker still required the removed one-shot readiness log string. Updated the checker to require the bounded `func_wait_process_running rpc.mountd` call and its explicit not-ready failure log. Runtime assertions are unchanged; no test was weakened.
- Re-run the exact-SHA NFS workflow after this checker-only follow-up. No target firmware build or physical-router validation is claimed.

---

## 2026-11-21 — Wait for daemonized rpc.mountd child before treating NFS startup as failed

- Parent main SHA: `32101ce99fa668c986871b5eff306fe90d18683d`.
- Fixed an NFS startup race in `trunk/user/nfs-utils/nfsd.sh`: `rpc.mountd` daemonizes by default, but the script previously checked procfs only once immediately after its launcher returned. A scheduled-later child could be misclassified as a false-success/failed start and trigger avoidable teardown. Startup now uses four bounded process-inventory attempts with three one-second settling intervals; RUNNING succeeds, UNKNOWN fails immediately, and continued ABSENT fails closed into existing cleanup.
- Added a shell integration regression that delays publishing the mock rpc.mountd child until the first post-launch poll; it requires startup success only after the child appears. Existing false-success and cleanup cases remain in place.
- Shell syntax and host regression are delegated to exact-SHA NFS Actions; no local target build or physical-router test is claimed. Exact-head Actions are pending at commit time.

---

## 2026-11-21 — Cross-service detached-daemon rollback and CI handoff checkpoint

- Source parent SHA: 8f21e7fc31609caae6ab0d2260e4b345f3e6d487; frozen backup: 72f4f157a084f8184b9597ca5bf334b8080e5807 (unchanged).
- Added fail-closed rollback after detached daemon launch/readiness failures across HTTPD, MiniUPnP, DNSMasq, Samba (nmbd/smbd/wsdd2), vsftpd, storage apps and Inadyn. Each cleanup attempt is followed by authoritative process inventory where applicable and preserves the initiating error; policy/source checks cover UNKNOWN and invalid inventory.
- Source commits in this window: HTTPD `ab866c3e93c5131ed6723c7a2a7d3de5df0e693c`; MiniUPnP `b55d97afc5b09fed7658785f6415023ddb2934e7`; DNSMasq `ba695e6f523b1218bd7486fb77b9b42e4e1e4138`; Samba `f8fc45e7ad1479e2544e093a6e52a074c35bf256` / `3da546fd4e58d09e50cdbc0c872289e7a5504dd0`; vsftpd `66edbcb149c7332c76330b4cb33c8847db21ac31`; storage apps `acbc93decc462e5e0360552ef0da83310fed5512`; Inadyn `41b18c57237e70171e338ec6f416b5a613643fda`.
- Storage safe-remove checker now needs to validate the revised start-status/rollback control flow; current checker update presence: confirmed by source inspection. Exact current source Actions: XY-C1 Firmware Validation: in_progress (#44878734942); USB storage hotplug stability regression: in_progress (#44878734891). Exact Inadyn Actions: XY-C1 Firmware Validation: in_progress (#44877952694); HTTPD Authentication and Security Regression: success (#44877952674); miniupnpd atomic configuration publication regression: success (#44877952721); Process lifecycle inventory - 41b18c57237e70171e338ec6f416b5a613643fda: success (#44877952692); DNSMasq lifecycle stability regression: success (#44877952738); Samba share configuration stability regression: success (#44877952776); Inadyn fail-closed lifecycle regression: success (#44877952666); WAN DHCP callback state stability regression: success (#44877952702); WAN route operation status regression: success (#44877952689); Process inventory and lifecycle fail-closed regression: success (#44877952774); NVRAM transaction boundary audit: success (#44877952690). Exact Storage checker Actions: AI handoff documentation contract: success (#44878146492); USB storage hotplug stability regression: failure (#44878146416).
- Historical HTTPD lifecycle/security, Process inventory and WAN route status passes are documented in SESSION_HANDOFF.md; they apply only to their original SHA. XY-C1 L3 and physical-router L4 remain NOT VERIFIED.
- No local target build or cross-compile was run.
- Resulting documentation commit: THIS COMMIT.

---

## 2026-11-21 — Roll back HTTPD when startup settling wait fails; repair handoff contract

- Parent source SHA: ab866c3e93c5131ed6723c7a2a7d3de5df0e693c. Fixed a real HTTPD lifecycle gap in services.c: after _eval() launched a detached listener, failure of the initial settling wait returned directly while httpd_started remained cleared. The wait-failure path now attempts stop_httpd_locked() under the existing lifecycle lock, performs an authoritative process scan, republishes running state for RUNNING/UNKNOWN (fail-closed) and preserves the original wait errno.
- Added HTTPD failed-start policy tests for ABSENT, RUNNING, UNKNOWN and invalid enum outcomes, plus a source-order contract in the HTTPD workflow. Exact-head HTTPD lifecycle run #44875048202 PASS; HTTPD authentication/security #44875048156 PASS; Process lifecycle inventory #44875048297 PASS; WAN route status #44875048033 PASS.
- Exact-head AI documentation run #44875048197 failed because SESSION_HANDOFF.md omitted the literal phrase “one atomic main commit” and the required top-level “# Cross-session execution mandate” heading. This checkpoint fixes both without weakening the validator.
- XY-C1 run #44875048030 was still executing the build pipeline at checkpoint; Build, TRX and artifact must all finish PASS on one identical SHA for L3. NVRAM transaction audit #44875048211 and whole-repository Process lifecycle regression #44875048100 were in progress.
- Frozen backup remains backup/pre-plugin-overhaul-2026-11-14 at 72f4f157a084f8184b9597ca5bf334b8080e5807. No local checkout/build was available; PASS claims above are from exact-SHA Actions.
- Resulting commit: THIS COMMIT.

---

## 2026-11-20 — Repair DHCP timestamp regression fixture and handoff wording

- Exact-head WAN DHCP run #44874862762 caught an ASan/UBSan stack overflow in the new 12-field full-WAN tuple test: its local dhcp_lease_nvram_value array remained length 11 while the fixture and loop were expanded to 12. Corrected the declaration; production code was not implicated by this failure.
- Exact-head AI docs run #44874862864 rejected the handoff because it paraphrased the mandatory checkpoint sentence. Restored the exact required wording: "The checkpoint is mandatory; a document edit is conditional."
- MiniUPnP run #44874862770 PASS on parent 4e4b1e51ec58958823e2c94890d4e073450bcb87. XY-C1 run #44874862823 and several independent workflows were still running at the last poll; Build/TRX/artifact conclusions remain to be checked.
- Resulting commit: THIS COMMIT.

---

## 2026-11-20 — Commit WAN DHCP timestamp only with an accepted lease

- Parent main SHA: b2971b23154c1dac6794cf3ff1295ae0632e7cf4. MiniUPnP exact-head regression #44874295877 PASS. XY-C1 run #44874295896 passed host validation only; Build XY-C1/TRX/artifact were skipped, so this is not new L3 evidence. Last complete L3 evidence remains #44809783977 on product-source SHA 0b5eef8bd00fd6e732449bcf241949df84b4f40c.
- Fixed a consistency defect in net_wan.c: physical WAN BOUND and fast RENEW previously updated wanN_dltime at callback entry through a void setter, before validating/committing DHCP state. BOUND now includes dltime in the compensatable lease tuple; fast RENEW publishes dltime only after changed options commit, using checked transactional publication. MAN/IPTV behavior is unchanged.
- Extended the DHCP lease tuple regression to require dltime rollback and added a one-value dltime test. Added scripts/check_wan_dhcp_dltime_publication.py and the WAN callback workflow hook. Exact-head tests/Actions are pending.
- Resulting commit: THIS COMMIT.

---

## 2026-11-20 — Make MiniUPnP errno-assignment checker whitespace-independent

- Exact-head MiniUPnP run #44874160863 on parent 7be6d83357ab31795b446010aaa9571aac5000dc passed the GNU89/Werror/ASan/UBSan policy suite, including both legacy and IPTABLES_143 production IPTC errno helper harness variants, but the delete checker failed at its assignment inventory: function_body() normalizes C whitespace to one line, so splitlines() found no assignment statements.
- Updated the checker to scan semicolon-delimited statements, exclude the two local declarations, and require the first-error helper on every actual primary/secondary errno assignment. The assignment-bypass guard remains strict; no product source or runtime behavior changed.
- Exact-head AI docs run #44874160857 PASS. XY-C1 run #44874160943 was still in progress at checkpoint; current-head L3 remains pending until Build/TRX/artifact all pass on the same source SHA.
- Resulting commit: THIS COMMIT.

---

## 2026-11-20 — Repair MiniUPnP delete checker and restore execution checkpoint wording

- Exact-head AI handoff run #44810641058 on parent 5a5981318b1b15c6d1d16d4ddf9725206c0f5c3d failed because the handoff omitted the required "Before context exhaustion" rule and used a non-canonical current-state heading; both are restored in this checkpoint.
- MiniUPnP run #44810270387 on bb4a1910acd73c2ae9433d66971bdcd4313d4f23 passed its GNU89/Werror/ASan/UBSan policy binaries, including both IPTC errno API variants, then failed at scripts/check_miniupnpd_iptcrdr_delete.py:74 because its ENOENT/secondary errno assertions expected direct assignments removed by the first-error helper refactor. Updated those assertions to require miniupnpd_record_first_failure_errno(); the helper/bypass contracts remain strict.
- XY-C1 run #44809783977 passed Build XY-C1, TRX verification and artifact upload on product-source SHA 0b5eef8bd00fd6e732449bcf241949df84b4f40c; L3 is verified for that exact product tree. No physical-router L4 claim.
- Resulting commit: THIS COMMIT.

---

## 2026-11-20 — Exact-source and Actions handoff checkpoint

- Checkpoint parent main SHA: bb4a1910acd73c2ae9433d66971bdcd4313d4f23; latest product-source commit touching iptcrdr.c: 0b5eef8bd00fd6e732449bcf241949df84b4f40c; latest errno-runner path commit: c492d33b10baf96d8a971c4adc6773b3df5808f3.
- Frozen backup branch SHA: 72f4f157a084f8184b9597ca5bf334b8080e5807 (unchanged).
- Exact-head main Actions: MiniUPnP 44810270387:completed/failure; errno harness step completed/success; AI docs 44810270268:completed/failure; Process inventory not observed; NVRAM not observed.
- Product-source XY-C1 run for 0b5eef8bd00fd6e732449bcf241949df84b4f40c: 44809783977:in_progress/pending; steps: Build XY-C1=in_progress/pending; Verify TRX output=pending/pending; Run actions/upload-artifact@cf430e030ddbb5b0abf93d22962f4752f3646cd9=pending/pending. L3 is verified only if Build, TRX and artifact upload all passed on this source SHA.

- The latest working tree includes IPTC helper errno injection under both legacy and IPTABLES_143 handle conventions, syslog.h in the C harness, first-error capture policy in the production transaction, and source contracts that disallow direct primary/secondary errno overwrites.
- Current product L3 and physical-router L4 are not claimed unless their respective exact-source gates pass.

---

## 2026-11-20 — Reject unguarded primary/secondary errno overwrites

- Parent main SHA: c492d33b10baf96d8a971c4adc6773b3df5808f3.
- Tightened the standalone and workflow-inline MiniUPnP source contracts: every assignment to primary_failure_errno or secondary_failure_errno inside delete_redirect_and_filter_rules_ex() must call miniupnpd_record_first_failure_errno(). This guards against a future later failure silently overwriting the first causal error even when the helper's presence and final selector remain intact.
- The check filters only assignment statements, not the local variable declarations. It does not alter production behavior or relax any condition.
- The production IPTC errno harness now includes syslog.h for LOG_ERR and exercises both API handle conventions; exact-head MiniUPnP CI remains the required proof for those compiled variants.
- Frozen backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807. Exact-source firmware L3 remains unverified until the current product SHA's Build, TRX and artifact gates pass.

---

## 2026-11-20 — Include the syslog constants in the IPTC errno harness

- Parent main SHA: 0b5eef8bd00fd6e732449bcf241949df84b4f40c.
- Static review of the generated C fixture found its extracted production helpers call syslog(LOG_ERR, ...), but the harness preamble only declared the syslog mock and did not include syslog.h. Added the required header so LOG_ERR is defined while the fixture's mock still captures and deliberately clobbers errno.
- This is a host-test harness correction only; production C is unchanged. The harness continues to compile both legacy and IPTABLES_143 signatures with GNU89/Werror/ASan/UBSan.
- Frozen backup rechecked unchanged at 72f4f157a084f8184b9597ca5bf334b8080e5807. Exact-head CI must verify both variants after this correction; current-source firmware L3 remains unverified until same-SHA Build/TRX/artifact evidence is complete.

---

## 2026-11-20 — Preserve the first error within primary and peer cleanup phases

- Parent main SHA: b6c1519c69cedefba6483e51a810106f185bef1a.
- Source review found that the enclosing delete transaction captured primary and secondary errno, but later failures in the same phase could overwrite the earlier causal errno. In particular, a filter initialization failure could replace a prior NAT failure, and a later mangle-stage failure could replace a peer NAT-stage failure.
- Added miniupnpd_record_first_failure_errno() and routed every primary/secondary error capture through it. The first observed errno is now retained within each phase; errno-less failures become EIO. The final selector still prefers the primary phase over the peer phase.
- Added a compiled policy regression for first-error retention and strengthened standalone and inline source contracts to require both primary and secondary captures to use the helper.
- Parent HEAD at patch construction: b6c1519c69cedefba6483e51a810106f185bef1a. Frozen backup confirmed unchanged at 72f4f157a084f8184b9597ca5bf334b8080e5807. The production helper errno injection matrix is in the preceding commit and is independently wired to both IPTABLES_143 and legacy API builds.
- Resulting commit: THIS COMMIT. Exact-source firmware L3 remains NOT VERIFIED until Build XY-C1, TRX and artifact upload pass for this product SHA. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-20 — Inject errno-clobbering IPTC backends for both handle APIs

- Parent main SHA: 4a7d3c3e3b1966d1574f703ffbfc498bce292542.
- Added scripts/test_miniupnpd_iptc_errno.py. It extracts the actual production delete_rule_and_commit() and insert_rule_and_commit() definitions from netfilter/iptcrdr.c, compiles them under GNU89/Werror/ASan/UBSan, and executes the same failure matrix twice: legacy pointer-to-handle API and IPTABLES_143 by-value API.
- The mock iptc_strerror(), syslog() and iptc_free() deliberately overwrite errno. Regressions cover delete operation ENOSPC, delete commit EROFS, missing backend errno -> EIO, insert table-init EACCES, insert operation EINVAL and insert/rollback commit EROFS. Each failure must retain its original errno after logging and handle cleanup.
- Wired the runner into MiniUPnP push and pull-request path filters and the focused workflow. This adds executable backend coverage without modifying production code.
- Parent 4a7d3c3e3b1966d1574f703ffbfc498bce292542 had MiniUPnP #44808660926, AI #44808661003, Process #44808661091, NVRAM #44808660908 and XY-C1 #44808661058 all in progress at last poll. The exact XY run must pass Build, TRX and artifact before L3 is claimed for source 4a.
- Resulting commit: THIS COMMIT. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-20 — Match UDP peer DSCP cleanup on the internal source port

- Parent main SHA: 062d09b646f294d09947c986aadcfc9e6619e6c9.
- addpeernatrule() and addpeerdscprule() both build UDP tuples with get_udp_match(rport, iport); get_udp_match(dport, sport) stores the internal port in spts[0] and the remote port in dpts[0].
- delete_redirect_and_filter_rules_ex() incorrectly copied UDP dpts[0] into iport before searching the mangle DSCP rule by spts[0]. Peer SNAT removal could therefore leave a stale UDP DSCP entry without reporting a failure.
- Corrected UDP peer SNAT scan to read spts[0]; TCP/UDP DSCP comparisons use miniupnpd_peer_internal_port_matches(). Added GNU89 regression cases for internal-port match versus remote-port mismatch, and expanded the dedicated source checker to assert both rule writers, both cleanup branches and the unit test use consistent tuple semantics.
- Parent 062d09b646f294d09947c986aadcfc9e6619e6c9: MiniUPnP #44808409986 PASS and AI #44808409401 PASS. Process #44807976713 and NVRAM #44807976901 PASS on source-equivalent parent 5ca. XY-C1 #44807976845 on 5ca was still Build IN_PROGRESS (TRX/artifact PENDING); L3 remains NOT VERIFIED for the current product source.
- This change updates production netfilter code and requires new exact-head host and XY-C1 validation. Physical-router L4 remains NOT VERIFIED.
- Resulting commit: THIS COMMIT.

---

## 2026-11-20 — Normalize the inline NAT snapshot errno contract

- Parent main SHA: 4e8bbf9be606d4a9280767146ccaae53bdbaea66.
- The MiniUPnP atomic-configuration workflow #44808194708 reached its final writer/start-gate contract and failed at the newly added NAT snapshot errno assertion. The production return is split across two lines, while the inline Python check compared a single raw line. The standalone checker already normalizes function whitespace and correctly accepts this control flow.
- Added a normalized cleanup_backend_flat view in the workflow and moved only this assertion to the normalized source. The required control-flow expression remains identical; this is a checker formatting correction, not a weakened condition.
- The preceding compiled policy tests, including errno snapshot/restore and primary/secondary precedence, passed in the same workflow before the inline source contract failed.
- Exact-head MiniUPnP #44808194708 is therefore FAIL due only to this raw-whitespace mismatch. Fresh MiniUPnP/AI/Process/NVRAM/XY checks will run on the corrected commit. Firmware L3 and physical-router L4 remain NOT VERIFIED for the new source.
- Resulting commit: THIS COMMIT.

---

## 2026-11-20 — Align NAT snapshot failure contract with errno restoration

- Parent main SHA: 5ca967f2dd9da9ee20408dde1b413cfaeac9265c.
- Exact-head MiniUPnP #44807976684 on 5ca967f passed the compiled C policy/ASan regressions, allocation, ownership, capacity, update and description tests, then failed the standalone delete checker at a stale assertion requiring the old early return "if (nat_backup_failed) return r;".
- Production now correctly returns through miniupnpd_restore_errno_on_failure(r, primary_failure_errno), so the original malloc/snapshot errno survives the early exit. Updated the standalone checker to require that path and added the same assertion to the workflow's duplicate inline contract. No production behavior or test expectation was weakened.
- Exact-head AI #44807976939, Process #44807976713 and NVRAM #44807976901 on 5ca were still running at the last poll; XY-C1 #44807976845 was queued. The corrected contract commit requires fresh MiniUPnP and product Build/TRX/artifact checks.
- Resulting commit: THIS COMMIT.

---

## 2026-11-20 — Keep the missing-chain errno in the delete transaction

- Parent main SHA: e3578d8b09eb5555aa726726184d21eafa25dbb8.
- Exact-head MiniUPnP #44807553369 failed at the standalone delete source contract after the compiled policy/ASan regressions passed. The assertion correctly exposed that the ENOENT assignment had been inserted into the earlier get_redirect_rule_by_index() helper rather than delete_redirect_and_filter_rules_ex(); the former had no primary_failure_errno local, creating a target compile error. Moved the assignment to the owning delete transaction and removed the undeclared reference from the lookup helper.
- This is a genuine source-scope correction, not a relaxed checker. The standalone contract remains unchanged and must now pass against the corrected function body.
- Exact-head AI #44807553503 PASS and Process #44807553198 PASS on e357. XY-C1 #44807553718 and NVRAM #44807553409 were still running at the last poll; their result does not include this correction. The new commit requires its own exact-head MiniUPnP and XY-C1 verification.
- Resulting commit: THIS COMMIT.

---

## 2026-11-20 — Preserve first errno across primary and peer IPTC cleanup

- Parent main SHA: 5b9fcaa09b621e880517232b24a0f35f9b296827.
- The previous commit added errno preservation inside the delete/insert handle helpers. A second review found the enclosing delete transaction could still lose errors at NAT/filter/SNAT/mangle iptc_init failures because syslog/iptc_strerror run before the errno is saved. Later independent peer cleanup could also overwrite the primary failure errno before the aggregate status returned.
- Capture primary and secondary operation errno separately at the failure site, before logging. Keep ENOENT for a missing primary NAT chain, restore direct NAT-init failures and snapshot failures, preserve filter-init errno through NAT rollback, and preserve peer NAT/mangle-init errno through peer rollback. The final return now restores the primary failure errno first, or the secondary errno only when primary cleanup succeeded.
- Added miniupnpd_select_cleanup_failure_errno() with policy tests for primary precedence, secondary fallback and EIO fallback; strengthened the standalone delete checker and duplicate inline workflow contract to pin the final restoration path.
- Parent 5b9fcaa09b621e880517232b24a0f35f9b296827: MiniUPnP #44807130506 PASS, AI #44807130434 PASS, Process #44807130342 PASS; NVRAM #44807130385 and XY-C1 #44807130410 were in progress at last poll. This source change requires a fresh exact-head test/build.
- Resulting commit: THIS COMMIT. L3 for this source SHA NOT VERIFIED; L4 NOT VERIFIED.

---

## 2026-11-20 — Preserve IPTC operation errno across handle cleanup

- Parent main SHA: d7a937541226425de4254d90498c35f3c5859994.
- Source review found that both delete_rule_and_commit() and insert_rule_and_commit() returned failure after calling iptc_free() without preserving errno. The logging path also called iptc_strerror(errno) before handle cleanup. Because those library, logging and cleanup calls are not required to preserve errno, callers could receive a stale error instead of the original delete, commit, insert or rollback failure.
- Added miniupnpd_restore_errno_on_failure() to the shared GNU89 cleanup policy. Both mutators now snapshot errno immediately at the failing IPTC operation (and before logging), release the handle, then restore the original errno; a failure with errno unset deterministically reports EIO. The rollback helper also preserves errno when table initialization fails.
- Added compiled policy regression for successful-status errno neutrality, original-error restoration and EIO fallback. Extended the standalone IPTC delete checker and duplicate inline workflow contract to require snapshot-before-logging and restore-after-free in both production mutators.
- Parent SHA d7a937541226425de4254d90498c35f3c5859994: MiniUPnP #44805637861 PASS, AI #44805637957 PASS, Process #44805637806 PASS, NVRAM #44805637802 PASS. Product-source XY-C1 #44805637821 remained IN_PROGRESS at last poll (Build running; TRX/artifact pending); this commit changes production C and requires a fresh exact-head XY-C1 build.
- Resulting commit: THIS COMMIT.

---

## 2026-11-20 — Record exact-source XY-C1 L3 boundary

- Parent main SHA: eccdab54081a0f28c072025a55e5e2f6a8159b88.
- XY-C1 run #44804291706 for product source SHA 30af417ed8d7665e5da0f630e79b38bbb9f92296 completed SUCCESS. The actual Build XY-C1, Verify TRX output and artifact-upload steps each completed SUCCESS. This is valid L3 evidence for source 30af417.
- This evidence does not extend to later production changes: commit 7d9b12c added the primary-metadata finalizer, and current source commit eccdab54081a0f28c072025a55e5e2f6a8159b88 added the peer-cleanup gate. Exact-source XY-C1 run #44805637821 is still building on eccdab; Build XY-C1 is IN_PROGRESS, TRX and artifact upload are PENDING.
- On exact SHA eccdab, MiniUPnP #44805637861 PASS, AI handoff #44805637957 PASS, process inventory #44805637806 PASS, and NVRAM transaction audit #44805637802 PASS.
- Frozen backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807. No production files changed in this evidence checkpoint.
- Resulting commit: THIS COMMIT.

---

## 2026-11-20 — Keep peer SNAT/DSCP when primary redirect deletion fails

- Parent main SHA: 7d9b12c46927b2fbb3aba6ea0346de0cc1b7d0a1.
- Review of delete_redirect_and_filter_rules_ex() found a state inconsistency: after primary NAT deletion failed, or filter deletion failed and NAT rollback succeeded, the function still proceeded to delete the peer SNAT rule and DSCP rule. That could leave the still-present primary DNAT/filter mapping without its supporting hairpin/peer rules. It also attempted secondary NAT/mangle operations after a missing/failed primary-chain scan.
- Gate the complete peer SNAT + DSCP + peer rollback transaction on miniupnpd_peer_redirect_cleanup_allowed(primary_mapping_removed). A primary failure now preserves all peer state and first-error/errno; when the primary mapping is confirmed gone (including an initial no-match scan), independent peer cleanup still runs and its errors remain aggregated.
- Added GNU89 policy regression for the allow/deny decision and extended scripts/check_miniupnpd_iptcrdr_peer_delete.py plus the workflow's inline source contract to require the entire secondary transaction, including both tables and peer rollback, to sit inside the gate.
- Previous main 7d9b12c exact-head MiniUPnP/AI/Process/NVRAM/XY Actions are queued/in progress at last poll. This source fix triggers fresh exact-head workflows and an XY-C1 production build. Firmware L3 remains NOT VERIFIED; physical-router L4 remains NOT VERIFIED.
- Resulting commit: THIS COMMIT.

---

## 2026-11-20 — Test primary redirect metadata cleanup through the production policy

- Parent main SHA: e085c6fb89a2906d234c27248d4daebf232e4fcc.
- Exact-head MiniUPnP #44804604643 and AI documentation #44804604650 both PASS on e085c6f. The injected _upnp_delete_redir caller regression passed under GNU89/Werror/ASan/UBSan.
- Added miniupnpd_finalize_redirect_cleanup() to the shared cleanup policy and routed production del_redirect_desc() through it. Description metadata is now removed exactly when the primary NAT/filter mapping is confirmed gone, including a secondary peer SNAT/DSCP failure, while the aggregate firewall status is preserved. A mock callback regression covers primary removed/not removed, success/failure status, callback argument identity, exactly-once invocation and a NULL callback.
- Updated both standalone and inline source contracts to require the tested production finalizer path; no coverage was weakened.
- XY-C1 #44804291706 on product SHA 30af417 was still building at last observation; that earlier source did not contain this finalizer. Exact XY #44804604713 on e085 succeeded overall but skipped Build/TRX/artifact because e085 changed only tests/checker/docs. This production C change requires a fresh same-SHA Build XY-C1, TRX and artifact gate. Firmware L3 remains NOT VERIFIED.
- Resulting commit: THIS COMMIT. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-20 — Repair NAT scan source contract and restore handoff invariant

- Parent main SHA: ce8daa92ec5a57bcef2d8a662be32c00c6b369c1.
- Exact-head MiniUPnP run #44804291730 on product SHA 30af417ed8d7665e5da0f630e79b38bbb9f92296 passed the compiled cleanup-policy test and preceding allocation/ownership/capacity/update/description tests, then failed in check_miniupnpd_iptcrdr_delete.py. The checker normalizes function whitespace before inspecting it but searched for a literal escaped-newline sequence; changed the match to the normalized loop-close marker immediately before nat_scan_completed.
- Exact-head AI docs #44804291590 on the same SHA failed because the live SESSION_HANDOFF.md had lost the exact required phrase "one atomic main commit". Restored the cross-session invariant explicitly, preserving the main-only/non-forced expected-SHA workflow.
- The injected production-caller test fixture is repaired in ce8daa92ec5a57bcef2d8a662be32c00c6b369c1: C source now includes function signature and actual newline separators. Its exact-head run is triggered by this new commit and must validate it.
- Product source SHA 30af417ed8d7665e5da0f630e79b38bbb9f92296 moves the NAT scan completion marker after the NAT loop. Exact-SHA XY-C1 #44804291706 was still in progress at last poll; the earlier source 35e run #44803257077 was also not terminal at previous observation. Firmware L3 remains NOT VERIFIED.

---

## 2026-11-20 — Repair the injected delete caller C fixture

- Parent main SHA: 30af417ed8d7665e5da0f630e79b38bbb9f92296.
- Exact-head MiniUPnP run #44804109446 on the previous test commit failed at the new integration step. Its generated C had literal backslash-n separators and omitted the production function signature, so the compiler reported a stray backslash and an implicit declaration. No production source failure was involved.
- Corrected the runner to emit actual newlines and prepend the function's int return type and signature around the extracted production body. Tightened escape handling in its C-aware brace parser.
- Concurrent product commit 30af417ed8d7665e5da0f630e79b38bbb9f92296 also moved nat_scan_completed to after the NAT loop; its new exact-head build and source checks must be evaluated separately.
- Firmware L3 remains NOT VERIFIED until the current source SHA's Build XY-C1, TRX and artifact steps pass.

---

## 2026-11-20 — Mark NAT scan complete after iteration

- Parent main SHA: cbe06d311a01a6da73891df2c7a5587e03f312fc. Exact-head MiniUPnP workflow #44803703734 passed after the inline append contract was updated to the fd-based transaction helper. A semantic review then found that the source contract accepted nat_scan_completed before the NAT rule loop.
- A scan cannot be complete before iteration. Moved the marker to immediately after the NAT loop terminates, preserving fail-closed behavior if the NAT chain is missing. Now an absent matching rule is treated as already removed only after the chain has actually been enumerated.
- Updated the source checker to enforce chain validation → rule iteration → loop close → completion marker → optional peer cleanup.
- Production source changed; new exact-head MiniUPnP and XY-C1 Build/TRX/artifact runs are required.

## 2026-11-20 — Exercise production UPnP delete caller with injected backends

- Parent main SHA: `72b914095e14c099f60d6ac8f20150b4ca5d29e9`; MiniUPnP host regression #44803703734 and AI handoff validation #44803703663 both completed SUCCESS on this exact SHA.
- Added `scripts/test_miniupnpd_redirect_delete_integration.py`, which extracts the actual `_upnp_delete_redir()` function from `upnpredirect.c`, compiles that production function under GNU89/Werror/ASan/UBSan, and injects firewall/lease backends. It exercises the central regression: aggregate peer/DSCP cleanup failure after primary DNAT/filter removal must still remove the lease while preserving the aggregate failure. It also checks preservation of leases when primary removal is false, successful no-op behavior, lease-file failure errno propagation, and exactly-once cleanup.
- Wired the test into MiniUPnP push/PR path filters and runs. This is a host-only test change; the production source remains the 35e SHA.
- XY-C1 run #44803257077 for source SHA `35e319a57af04e113fc9a3ccaa31628764ce4cfd` remained in progress at the latest poll, with the target build active and TRX/artifact pending. Do not infer L3 until those exact steps finish.
- Resulting commit: THIS COMMIT. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-20 — Align MiniUPnP workflow contract with transactional lease append

- Parent main SHA: `b12d6c4abb2b95f901098425229eb736243335a2`.
- Exact-head MiniUPnP run #44803257122 on `35e319a57af04e113fc9a3ccaa31628764ce4cfd` passed all 25 preceding compile/regression/source-contract steps, including cleanup-policy ASan/UBSan tests, IPTC ownership, NAT/filter and peer SNAT/DSCP compensation, lease serialization, lease cleanup and replay checks. It failed only at the final embedded writer/start-gate source contract, which still required direct `fwrite(record, ...)` and `fclose(fd)` calls inside `lease_file_add()`.
- The current writer deliberately delegates to `miniupnpd_lease_append_transaction()`, which handles short writes, EINTR, sync and rollback-to-original-EOF. The dedicated lease checker and compiled regression already exercised that helper; only the duplicate inline workflow assertions were stale. Replaced them with checks for transaction delegation, no direct stdio write, close/errno handling, rollback operations and partial-write ENOSPC tests, while retaining description-boundary checks.
- The source-side scan-state relocation is covered in commit `35e319a57af04e113fc9a3ccaa31628764ce4cfd`. Its exact-source XY-C1 run #44803257077 was still running at last poll; parent `8f88445490dd8df6b600743ec0ff7716707df115` failed target compilation because the scan marker was placed in the wrong function. This is corrected in 35e, but same-SHA Build/TRX/artifact evidence is still required.
- PR #1's security-related AliDDNS content is already present on current main: the stdin-only OpenSSL 3 HMAC helper and test, the CA archive/bootstrap failure tests, synchronous `copyscripts.sh` execution and startup-status propagation all exist. The PR versions of the helper, its test and CA test are byte-identical to main; its script/workflow/checker variants are older. No PR cherry-pick is needed.
- Resulting commit: THIS COMMIT. Firmware L3 and physical-router L4 remain NOT VERIFIED until exact-source gates pass.

---

## 2026-11-20 — Bind MiniUPnP scan state to the delete transaction

- Parent main SHA: `64d76a2c98a4fb0ed98f8aba899a71c44750f407`.
- The latest MiniUPnP run #44802827202 on `64d76a2c98a4fb0ed98f8aba899a71c44750f407` passed the IPTC handle-ownership checker after its function target was corrected, then failed at the peer SNAT/DSCP checker: it still extracted the legacy one-line compatibility wrapper instead of `delete_redirect_and_filter_rules_ex()`. The checker now inspects the status-aware implementation.
- Reviewing the new deletion-state implementation exposed a production defect before target compilation: `nat_scan_completed = 1` had been inserted into `get_redirect_rule_by_index()`, where the name was not in scope, while the local flag in `delete_redirect_and_filter_rules_ex()` was never set. The result was an undeclared identifier at compile time and, if hidden by a declaration, a false-negative primary-removal result. Moved the assignment into the verified NAT-chain branch and mark a missing NAT chain as an error.
- Strengthened the deletion source contract to require the scan marker exclusively inside the delete transaction, after the chain check and before enumeration. Added the new cleanup-policy header to the `iptcrdr.o` Makefile prerequisites so incremental builds cannot retain an object built against an older policy header.
- Exact-source XY-C1 run #44802582897 for the parent product source `8f88445490dd8df6b600743ec0ff7716707df115` was still in progress at the last poll. The new production change requires its own Build XY-C1, TRX and artifact validation on THIS COMMIT. MIPS/uClibc L3 and physical-router L4 remain NOT VERIFIED.
- Resulting commit: THIS COMMIT. No frozen-backup branch changes.

---

## 2026-11-20 — Rebuild the MiniUPnP lease checker around the shared rewrite primitive

- Parent main SHA: 9d3d8760f8111d0c0eda5244365470a110325336. The checker in 9d3d8760f8111d0c0eda5244365470a110325336 still contained stale assertions tied to the removed lease_file_remove() implementation and referenced rewrite_entry before assignment. Replaced it with a consistent function inventory covering append rollback, staged rewrite/fsync/rename, exact-key policy tests, update rollback, and delete cleanup ordering.
- The rewrite helper now has one static int declaration; the checker also rejects the prior duplicate form.
- Exact-head Actions for 9d3 are new/pending at this checkpoint. The host stream transformation tests passed on ec42 before its old checker failed. This checker correction must pass on its own SHA.
- Physical router L4 remains NOT VERIFIED.

## 2026-11-20 — Correct MiniUPnP rewrite-helper declaration and source contract

- Parent main SHA: ec42fa99a15d617e181fe366324b8a18a7ff37ba. Exact-head MiniUPnP run #44800659014 on ec42fa99a15d617e181fe366324b8a18a7ff37ba passed the new GNU89 ASan/UBSan stream policy tests but failed the source checker because it still expected lease-key matching inside lease_file_remove(), which now delegates to the common rewrite helper.
- While correcting that contract, source inspection caught a duplicate static int left at the rewrite helper definition by the mechanical function replacement. Removed the extra specifier before the production compile can reach it; the checker now guards this declaration shape and checks exact-key filtering in the shared helper rather than the wrapper.
- The atomic rewrite regression itself passed in Actions: replacement of multiple matching keys, preservation of adjacent keys, removal and missing-source replacement. The prior failure was the stale checker assertion; the duplicate specifier is a separate source syntax defect found in review and fixed here.
- Exact-head XY-C1 run #44800659212 on the pre-fix ec42fa99a15d617e181fe366324b8a18a7ff37ba was in progress at last check. Its compile result cannot validate the corrected source. Physical router L4 remains NOT VERIFIED.

## 2026-11-20 — Atomically replace MiniUPnP lease records on mapping updates

- Parent main SHA: 3f28c8fd9cebb2cccf28b9926a1422d3bbfed427. This commit makes lease-file update a single atomic rewrite and compensates the live firewall/metadata update if persistence publication fails.
- Existing update path committed NAT/filter or redirect metadata first, then removed the old lease line in one rename and appended the replacement separately. If append failed after removal, the old on-disk record was already lost; if either persistence step failed, the live firewall could remain updated while AddPortMapping returned failure.
- Added a stream-level lease rewrite policy that filters every exact protocol/port record, preserves unrelated entries, and appends at most one replacement. Production writes the transformed stream to a same-directory mkstemp file, flushes/fsyncs/closes it, and atomically renames it; a missing lease file can be created by replacement. The remove helper now uses the same rewrite primitive.
- Existing mapping update now retrieves the old description and timestamp with the prior internal port. It atomically replaces the lease record instead of remove+append. On failure, it restores the prior firewall port or prior description/timestamp when the port is unchanged; rollback failures are logged without masking the original persistence errno.
- GNU89 host tests cover replacement of duplicate exact-key records, preservation of adjacent longer-port/other-protocol records, deletion, missing-file creation and invalid arguments. The source checker verifies the fsync/rename and rollback ordering. Physical-router L4 remains NOT VERIFIED.

## 2026-11-20 — Align infosvr dispatch contracts with supported-opcode response layout

- Parent main SHA: c4e6b53c0b5a78ae841118a960bb1f137b162623; fixes the exact-head infosvr regression failure reported by run #44799414652 on 0d9719e7960f83d26a2df931902f61bee5b94bfe.
- The dispatch host binary itself passed, including ASan/UBSan. The remaining source checker failed because it still required the pre-refactor incoming-MAC comparison removed by the new read-only opcode allowlist, and its comment check was case-sensitive although the source comment begins with "Only".
- Removed the stale source-order requirement; the checker now asserts the implemented response MAC comes from info->MacAddress, that request-header MAC bytes are not consulted by process_packet(), and that the supported-opcode comment check is case-insensitive.
- Strengthened the dispatch regression to assert the legacy wire layout explicitly: the extended response structure's MacAddress field aliases the first six bytes of PKT_GET_INFO.PrinterInfo because GETINFO payload begins after the 8-byte IBOX_COMM_PKT_RES base header. This preserves the existing protocol layout; no production wire bytes changed.
- Parent exact-head infosvr run #44799414652 failed only at scripts/check_infosvr_packets.py after the packet test, dispatch test, SIGTERM and RC lifecycle binaries had passed. Rerun required. Parent XY-C1 #44799414860 succeeded on the test-only source SHA 0d9719e; its changed input was test-only, so this does not replace the production-source XY validation pending on #44799072655.
- XY-C1 build #44799072655 on 7065465d000b0bc17763b51c61533cebba243f62 remained IN_PROGRESS at last poll. Physical hardware L4 remains NOT VERIFIED.

## 2026-11-20 — Make MiniUPnP lease declaration guard match actual source bytes

- Parent main SHA: 0d9719e7960f83d26a2df931902f61bee5b94bfe; checker-only correction to the declaration repair in 7065465d000b0bc17763b51c61533cebba243f62.
- Exact-head MiniUPnP run #44799072749 on 7065465d000b0bc17763b51c61533cebba243f62 FAILED at the new declaration assertion. The job log showed doubled backslashes in the raw regex, so it matched a literal backslash+n instead of a line break. The production duplicate-static fix itself is intact.
- Replaced the fragile regular expression with an explicit source-prefix contract: lease_file_remove() must be preceded by one static int line and must not be preceded by two consecutive copies. Added positive/negative fixture checks locally.
- Exact XY-C1 run #44799072655 on 7065465d000b0bc17763b51c61533cebba243f62 was still running at last check and contains the actual MiniUPnP production source correction; L3 remains PENDING until Build/TRX/artifact finish. The full production translation unit was not compiled locally.
- Independent infosvr run #44799216221 FAILED in its dispatch test at an invalid expectation that the extended response MacAddress field remain zero; that field overlaps the start of the legacy response payload. Correct that test separately. Physical L4 remains NOT VERIFIED.

## 2026-11-20 — Correct duplicated MiniUPnP lease cleanup declaration

- Parent main SHA: `b90cdf4f6cd8ce0725bdee19dc7b605c0163a775`. Production source, its source-contract regression, this log entry and the materially corrected handoff are committed together in THIS COMMIT.
- Source review found two consecutive `static int` specifiers before `lease_file_remove()`, introduced in the lease-file persistence hardening. GCC rejects this declaration, so an XY-C1 firmware compilation would fail even though the earlier MiniUPnP host workflow did not compile the full production translation unit.
- Removed the redundant specifier. Extended `scripts/check_miniupnpd_lease_file_cleanup.py` with an anchored declaration-shape check that accepts the valid GNU89 definition and rejects the duplicated form.
- Local focused checks: the malformed two-`static` declaration fails under `gcc -std=gnu89 -Wall -Wextra -Werror -fsyntax-only`; the corrected declaration compiles. The new guard's positive and negative Python fixtures pass. The complete production translation unit was not compiled locally.
- Exact-head parent evidence: MiniUPnP #44798143654 PASS and XY-C1 #44798143782 PASS on `545d8ca3ddbe85e95271ed5e21a3990e81f2bc76`; the XY-C1 job completed Build, TRX verification and artifact upload. Those results predate the later infosvr commit `d1e668dc294601fcc4ce5ecd45241aa470236409` and do not cover this fix. On `d1e668dc`, AI handoff #44798836961 PASS; infosvr #44798836931, Process #44798836880 and NVRAM #44798836998 were in progress at last check.
- XY-C1 path filters do not currently include `trunk/user/infosvr/src/infosvr.c`, although that production file changed on `d1e668dc`; add this missing firmware-validation trigger as the next independent CI-governance slice. MiniUPnP/XY-C1 reruns are required on THIS COMMIT. Physical hardware L4 remains NOT VERIFIED.

## 2026-11-20 — Restore mandatory handoff checkpoint preamble after documentation edit

- Parent main SHA: `de521606d8b1ccfabbb67cb0e2e006ec8bbc5939`.
- Exact-head MiniUPnP #44795187554 passed the redirect metadata allocator regression and source-order contract. The AI handoff check #44795187315 failed only because the prior handoff rewrite accidentally removed the required `# Cross-session execution mandate` and `## Mandatory handoff checkpoint` preamble.
- Restored the full handoff from its parent revision, then updated only current state, active task, verification boundary and first next action. Kept the mandatory checkpoint instruction intact; did not change or weaken the validator.
- AI handoff exact-head rerun required. XY-C1 #44795187275 and NVRAM #44795187414 were pending at last poll. Physical-router L4 remains NOT VERIFIED.

## 2026-11-20 — Make redirect lease metadata allocation transactional

- Parent main SHA: `22a47cebeb8c09a51e6eb40919a4f0bca05d7875`.
- `add_redirect_desc()` silently ignored allocation failure after committing DNAT/SNAT, so a finite-lease mapping could lose its in-memory timestamp and fall back to timestamp 0 (infinite lifetime). `update_portmapping_desc_timestamp()` deleted the existing record before trying the replacement allocation and always reported success, so OOM could also erase the old description/expiry record.
- Added checked size-overflow/allocation handling and split descriptor preparation from allocation-free publication. New NAT/peer mappings allocate metadata before firewall mutation; metadata-only updates allocate before replacing the old record; changed-port updates stage metadata before the NAT commit and publish only after filter commit.
- Added a GNU89 ASan/UBSan allocator regression (success, injected ENOMEM, invalid allocator and size overflow), a source-order contract, Makefile header dependency and MiniUPnP/XY-C1 workflow coverage.
- Local GNU89 `-Wall -Wextra -Werror` ASan/UBSan allocator test PASS. Exact-head MiniUPnP, AI handoff and XY-C1 Build/TRX/artifact checks are pending; physical-router L4 remains NOT VERIFIED.

## 2026-11-20 — Remove unused peer rollback flag before compile

- Parent main SHA: `46d79504f70236b403ac3153ca155715d9c65bb0`.
- The peer SNAT/DSCP rollback path already uses `peer_rule_found` and `peer_delete_status` to suppress the mangle phase when snapshot allocation fails. Its extra `peer_backup_failed` local was set but never read, which could trigger GCC's `-Wunused-but-set-variable` under firmware warning policies.
- Removed the redundant declaration/assignment and added a source-contract assertion that the dead flag does not return.
- Parent exact-head Actions: #44794517934 NVRAM transaction boundary audit queued/pending; #44794518071 AI handoff documentation contract in_progress/pending; #44794517908 XY-C1 Firmware Validation in_progress/pending; #44794518177 Process lifecycle inventory - 46d79504f70236b403ac3153ca155715d9c65bb0 in_progress/pending; #44794517972 miniupnpd atomic configuration publication regression in_progress/pending. Exact-head rerun required after this correction; XY-C1 Build/TRX/artifact remains unverified.
## 2026-11-20 — Clear consumed peer IPTC handle before recording delete status

- Parent main SHA: `721857934de255342b4cdb66e08155aacc65acc2`.
- Exact-head MiniUPnP #44794249501 passed all earlier sanitizer and source checks but failed the IPTC ownership contract at the new peer SNAT deletion call: the code recorded `peer_delete_status` before clearing the by-value handle consumed by `delete_rule_and_commit()`.
- Moved the local `h = NULL` immediately after the helper call, then copied the independent result status. This keeps handle lifetime explicit and satisfies the existing all-callsite invariant without weakening the checker.
- Exact-head MiniUPnP rerun required. XY-C1 #44794249117 is still pending Build/TRX/artifact at last poll. Physical-router L4 remains NOT VERIFIED.

## 2026-11-20 — Restore peer SNAT when its paired DSCP deletion fails

- Parent main SHA: b7a803dcc95ae88566ffe01ff8fd8ea4c5cefd33.
- Confirmed a second cross-table deletion inconsistency in `delete_redirect_and_filter_rules()`: peer SNAT in nat POSTROUTING was committed for deletion before the paired mangle DSCP removal. Mangle initialization or matched DSCP delete/commit failure left an orphan DSCP rule and a retry could no longer locate the peer entry.
- Added a pre-delete copy of the complete peer SNAT entry and saved its original POSTROUTING index. After successful peer deletion, a failed mangle operation now reinserts and commits the saved SNAT rule at that index. Snapshot allocation failure aborts peer mutation.
- Compensation is skipped when no matching DSCP rule exists; the primary DSCP failure/errno is retained if SNAT restoration fails, and restore failure is logged. The existing insert helper covers both IPTABLES_143 handle signatures.
- Extended the GNU89 ASan/UBSan policy test and added a dedicated source contract. MiniUPnP and XY-C1 workflow triggers include the checker and regression paths.
- The preceding NAT/filter deletion fix `b7a803d` passed exact-head MiniUPnP #44793663690. XY-C1 remains host-only for these userland paths; no firmware L3 or physical-router L4 claim.

## 2026-11-20 — Restore NAT mapping when the paired filter deletion fails

- Parent main SHA: ea745da6e423a778a65c0aba6ce85424e6a642a3.
- Confirmed a deletion-side cross-table inconsistency in `delete_redirect_and_filter_rules()`: NAT deletion was committed first; filter initialization or matched filter delete/commit failure could leave an orphan forwarding ACCEPT rule, and a retry could no longer identify its NAT entry.
- Added a pre-delete copy of the full NAT rule and saved chain index. Snapshot allocation failure aborts before firewall mutation. If the later filter operation fails after NAT deletion, a new IPTC insert/commit helper restores the saved NAT rule at its original index.
- Compensation is skipped when no corresponding filter rule exists. Primary filter-side error/errno is preserved if NAT restoration also fails; compensation failure is logged. Both IPTABLES_143 and legacy handle-pointer signatures are supported.
- Extended the GNU89 ASan/UBSan policy regression for delete rollback decisions and error precedence; added a source contract for snapshot/rollback ordering and ABI variants; wired checks into MiniUPnP and XY-C1 workflows.
- Parent exact-head MiniUPnP #44792212170 and AI handoff #44792212195 passed. XY-C1 #44792212585 was host-only: Build/TRX/artifact SKIPPED; no firmware L3. Physical-router L4 remains NOT VERIFIED.

## 2026-11-20 — Fix whitespace handling in NAT/filter rollback source contract

- Parent main SHA: bfc1c1663f9d3dc6cdb4349ba1b3601f175e27ad.
- Exact-head MiniUPnP Action #44791960712 on bfc1c1663f9d3dc6cdb4349ba1b3601f175e27ad compiled and passed the new rollback policy binary and all prior allocation/capacity regressions. The source contract alone failed because the rollback update_rule_and_commit() call is line-wrapped immediately after its opening parenthesis.
- Changed the checker to anchor on the function-call token and separately assert the NAT chain/index/entry arguments after it. The restore-before-commit ordering check remains enforced.
- XY-C1 #44791960769 on bfc1c16 was still in progress before Build. No firmware L3 claim; physical-router L4 remains unverified.

## 2026-11-20 — Roll back NAT when the corresponding filter update fails

- Parent main SHA: f5807d72e7b0e240660650432a9af846f61e94c7.
- Confirmed a cross-table consistency defect in update_portmapping(): NAT was committed with the new internal port before filter-table open/chain lookup/rule allocation/commit; every later failure returned without restoring NAT. DNAT and forwarding ACCEPT rules could therefore target different internal ports.
- Retained the changed NAT entry and original port through the filter operation, used a distinct filter-entry copy, and routed post-NAT filter failures through a compensating NAT commit at the saved NAT index. A failed rollback is logged while the original failure status/errno remains authoritative.
- Added GNU89 rollback-decision/error-priority tests and a source contract checking post-NAT failure routes and the restore call. Wired the test/checker into MiniUPnP and XY-C1 path filters.
- Capacity regression #44791232803 on c5e4169 passed. This transaction fix needs exact-head MiniUPnP verification; XY-C1 #44791232838 was still building at last poll. Physical-router L4 remains NOT VERIFIED.
## 2026-11-20 — Fix Python escape handling in the IPTC ownership checker

- Parent main SHA: `7faa238fe5df2605e4805a04187c866f9691528f`.
- Exact-head MiniUPnP run `44791361153` passed the existing configuration, daemon, PID-file, redirect-cleanup and GNU89 ASan/UBSan allocation tests, then failed in the new ownership checker before its assertions: the Python regex was over-escaped and raised `re.PatternError: missing ), unterminated subpattern`.
- Removed the unnecessary regex dependency and now locate all four helper calls with bounded literal scanning; corrected the C string/comment parser's escape handling. This repairs only the regression checker, not the production C fix.
- Exact-head MiniUPnP rerun and XY-C1 Build/TRX/artifact validation remain required. Physical-router L4 remains NOT VERIFIED.

## 2026-11-20 — Clear IPTC handle ownership after the initial NAT deletion

- Parent main SHA: `c5e41697bfeb593223a8c7c46b9e1e972fdc7be9`.
- `delete_rule_and_commit()` always releases the supplied `IPTC_HANDLE`, whether deletion/commit succeeds or fails. The initial NAT deletion caller left its local `h` non-NULL; on failure the later `if(h) iptc_free(...)` path could free the already-consumed handle again. The filter, peer-NAT and DSCP call sites already clear their local handle.
- Set `h = NULL` immediately after the initial NAT helper call. Added a focused source contract that requires all four call sites to mark the handle consumed and confirms both IPTABLES API free variants remain covered. Wired it into the MiniUPnP push/PR workflow filters and job.
- Exact-head MiniUPnP checks and XY-C1 Build/TRX/artifact verification for this source SHA are required. Physical-router L4 remains NOT VERIFIED.

## 2026-11-20 — Fix MiniUPnP port-mapping enumeration capacity overflow

- Parent main SHA: 2ce64bd716a56fc3dafee0ab022de9f55e29c10e.
- Confirmed a heap-buffer-overflow in get_portmappings_in_range(): at 128 matching NAT entries it called realloc(array, sizeof(unsigned short) * capacity) with unchanged capacity and never advanced the limit; the 129th match writes out of bounds.
- Added bounded capacity doubling that rejects unsigned-int and size_t byte-count overflow. The caller reallocates to next_capacity, advances capacity only after success, and on overflow/OOM frees the array, clears the returned count and restores errno after logging.
- Added a GNU89 ASan/UBSan regression that stores and verifies 4096 entries across multiple growth steps, plus a source contract rejecting the former fixed-size realloc. MiniUPnP and XY-C1 workflow filters include the policy/test/checker paths.
- MiniUPnP allocation regression #44789878471 and AI handoff #44789878451 passed on 8dbb096; current-parent XY-C1 #44790573472 was in progress before this commit. No local full firmware build; exact-head Actions required. Physical-router L4 remains NOT VERIFIED.
## 2026-11-20 — Place NTP reset locals in the actual NVRAM conversion function

- Parent SHA: `efebf98af5380a7edf6b3161a4aed822d522e9d0`.
- The first scope correction still removed the newly inserted declaration rather than the later `init_router()` declaration because the same text occurred twice. Exact-head watchdog job `136475249794` caught this with its source contract after the policy binaries passed.
- Corrected the placement with function-specific context: both NTP reset status locals now belong to `nvram_convert_misc_values()`, where the two writes and fatal-return gate execute; `init_router()` no longer owns unused locals. This restores GNU89 compilation and keeps the caller's pre-autostart fatal propagation unchanged.
- The NTP source checker now verifies the declaration in the same function as the writes. Exact-head watchdog/XY-C1 firmware build and TRX validation are still required.

## 2026-11-20 — Align modem/firewall source checks with HTTPD status writers

- Parent main SHA: `1ac1d7e9aa823c6344bdde223b33015eaa05ad3b`.
- Exact-head XY-C1 and firewall startup jobs were stopping in `check_modem_wan_start_status.py`: the checker still required direct `nvram_set_int_temp("httpd_started", ...)` writes after HTTPD had been migrated to the checked `httpd_started_state_publish_status()` wrapper and persistent port writes to `httpd_nvram_set_int_status()`.
- Updated the shared modem/firewall source contract to assert checked port normalization, stale-state clearing before launch, checked running-state publication, and failure handling in the firewall-error branch. The prior direct NVRAM setter is explicitly forbidden in that start path.

## 2026-11-20 — Include MiniUPnP netfilter production changes in XY-C1 build triggers

- Parent main SHA: `01f56e97c65d45fe998be26c69569208a92f7670`.
- Exact-head MiniUPnP allocation regression #44789878471 passed all steps on `8dbb0963cb6b816bf13cf13726f5de0ce3c867fb`, including the new ASan/UBSan cleanup binary and source contract. AI handoff #44789878451 also passed. Process lifecycle #44789544079 and NVRAM transaction #44789543972 passed on the previous production-source SHA `2ffb847`.
- The XY-C1 workflow's push path list did not include `netfilter/iptcrdr.c` or the new allocation-policy header, so no XY-C1 run had been scheduled for this firmware source change.
- Added both production paths to the XY-C1 trigger list. The workflow already watches its own YAML, so the current push triggers a fresh exact-head firmware build; Build, TRX verification and artifact upload remain the L3 acceptance criteria.

## 2026-11-20 — Correct boot NTP-reset local scope and call-path contract

- Parent main SHA: `8dbb0963cb6b816bf13cf13726f5de0ce3c867fb`.
- Exact-head watchdog run `44789384547` exposed that the new reset assignments are in `nvram_convert_misc_values()` (the NVRAM setup phase called by `init_router()`), while the local status variables had mistakenly been declared in the later `init_router()` function. This was a real GNU89 compile defect; the XY-C1 pre-build regression stopped before firmware compilation.
- Moved both status locals into `nvram_convert_misc_values()`, preserving the early dual reset and its fatal status propagation. Corrected the source contract to inspect the function that actually owns the writes and separately prove `init_router()` aborts before system-ready/autostart when NVRAM setup fails.
- Physical-router L4 remains NOT VERIFIED; exact watchdog and XY-C1 Build/TRX/artifact reruns required.

## 2026-11-20 — Normalize MiniUPnP allocation checker whitespace

- Parent main SHA: `2ffb8479a33ad55a4138b3384a31510f5b7c534e`.
- Exact-head MiniUPnP run `44789544201` on `2ffb8479a33ad55a4138b3384a31510f5b7c534e` passed the GNU89 ASan/UBSan allocation-cleanup binary and the existing configuration, daemon, PID-file and redirect-cleanup regressions. The source-contract checker alone failed at the entry-cleanup assertion because the C call spans multiple lines.
- Normalized extracted function bodies before matching entry, match, target and realloc cleanup calls. Assertions still enforce each guard before first dereference and verify exact cleanup arguments.
- No production C changed. Exact-head MiniUPnP rerun required; XY-C1 L3 not yet confirmed.

## 2026-11-20 — Fail safely when MiniUPnP iptables rule components cannot be allocated

- Parent main SHA: `f06fb00373149abe1afc6fcb06c9a0b6f9bd5510`.
- Confirmed an OOM crash path in `netfilter/iptcrdr.c`: TCP/UDP match factories and DNAT/SNAT/DSCP/MASQUERADE/ACCEPT target factories dereferenced unchecked `calloc()` results. Five rule builders also consumed potentially NULL components and leaked earlier allocations on partial failure.
- Added guards to all seven factories and all five builders. Entry, match, target and final rule-buffer failures release all previously allocated components, preserve allocator errno (defaulting to ENOMEM), and return before iptc append/commit.
- Added GNU89 ASan/UBSan cleanup/errno regression coverage and a Python source contract for every factory and builder. Both production and check/test paths are included in MiniUPnP push/PR workflow filters.
- No full local firmware checkout is available in this session; the exact-head workflow is the sanitizer/source-contract gate. XY-C1 L3 and physical-router L4 remain pending/not verified.
## 2026-11-20 — Fail closed when boot cannot invalidate stale NTP readiness

- Parent main SHA: `c9b931f0a17bb6673dd94bf7ad51c4c8ba6d7a75`.
- `init_router()` cleared temporary `ntpc_counter` without checking it, then reset persistent `ntp_ready=0` much later and also ignored that write. If either write failed, an old `ntp_ready=1` (and possibly stale counter) could make watchdog time-based scheduling trust the previous boot's clock state.
- Both freshness signals are now reset and checked together before router services, watchdog work, or user/autostart scripts proceed. Any failure returns the existing fatal router-init result. The persistent-vs-temporary semantics of the two keys are preserved.
- Fixed the misplaced include-guard terminator in `init_router_result_policy.h`, added GNU89 policy tests, and extended the watchdog lifecycle source contract/workflow. Exact watchdog/XY-C1 Build/TRX/artifact/Process Actions required; physical L4 NOT VERIFIED.

## 2026-11-20 — Align HTTPD marker helper contract with its checked NVRAM wrapper

- Parent main SHA: `1356dfd010bfc851336e2c5ab1f5b03abfd8119a`.
- Exact-head HTTPD Action `44788533266` on `1356dfd010bfc851336e2c5ab1f5b03abfd8119a` again passed the GNU89 daemon-policy binary but failed the integration contract at its assertion for `httpd_nvram_write_status` inside `httpd_started_state_publish_status()`. The latter correctly delegates through `httpd_nvram_set_int_status("httpd_started", started, 1)`; only the source assertion was looking at the wrong abstraction layer.
- Updated the contract to assert that exact delegation. The lower-level writer contract still verifies persistent and temporary backend writes plus errno normalization.
- Exact-head HTTPD rerun required; no production code or firmware source changed.

## 2026-11-20 — Correct HTTPD port-normalization source-check ordering

- Parent SHA: `18b14734be53ff7e53a974a6216d4a4294282cf0`. The HTTPD daemon workflow exposed one stale assertion still looking for a direct `nvram_set_int("http_lanport", ...)` call after the new checked wrapper was introduced; the source had correctly moved that write into `httpd_nvram_set_int_status()`.
- Updated the source contract to verify that process-presence scanning precedes the checked normalization writer. The same workflow commit also already asserts setter persistence semantics and lifecycle marker rollback.

## 2026-11-20 — Align HTTPD lifecycle source checks with checked NVRAM publication

- Parent main SHA: `c90407da0280365bdd41a17061b5b4d960bd4bce`.
- The HTTPD production slice now uses `httpd_nvram_set_int_status()` and `httpd_started_state_publish_status()` instead of direct unchecked NVRAM setters. Updated the lifecycle Action's embedded source contract to check persistent and temporary setter semantics, errno propagation, fail-closed port normalization, pre-launch stale-state clearing, post-start rollback, and checked stop/firewall marker publication.
- Exact HTTPD daemon lifecycle, XY-C1 Build/TRX/artifact and Process lifecycle runs on this workflow/source SHA remain required; physical L4 NOT VERIFIED.

## 2026-11-20 — Make HTTPD configuration and lifecycle-state NVRAM writes fail closed

- Parent main SHA: `12d3351f3deb024aa1a835aec9a30883225066f2`.
- `start_httpd_locked()` previously ignored failures while persisting normalized HTTP/HTTPS ports and while clearing/publishing the `httpd_started` runtime marker. A daemon could bind a fallback port while persistent configuration/firewall state remained stale, or a confirmed running/stopped process could be represented incorrectly.
- Added one status-preserving NVRAM integer writer for persistent configuration and temporary state. Port normalization now aborts before launch if its persistent write fails; marker writes before start and after confirmed start/stop are checked. If running-state publication fails after the listener is confirmed live, startup attempts a stop rollback and returns the publication error. Stop now propagates marker-publication failures after confirmed process exit.
- Firewall-error recovery uses the same checked marker publication and no longer performs a redundant ignored stopped-state write. Added backend errno normalization regression and strengthened the HTTPD source-contract workflow. Exact HTTPD, XY-C1 Build/TRX/artifact and Process lifecycle Actions required; physical L4 NOT VERIFIED.

## 2026-11-20 — Propagate LAN bridge reload-marker NVRAM failures

- Parent main SHA: `b9be4009afd514e2d56c160d777b9d7ad1e487c1`.
- `init_bridge()` recorded failures from switch, bridge, wireless and guest-isolation operations, but silently ignored both final `reload_svc_wl` and `reload_svc_rt` NVRAM writes. A device-node/driver error could leave stale reload state while bridge initialization falsely returned success.
- Both temporary NVRAM writes now feed the existing first-error/failure-count accumulator. The function continues the independent second write and returns the first failure, matching its existing aggregate-status contract. Added a GNU89 ASan/UBSan bridge-policy regression, a callsite source contract, and workflow trigger/compile coverage.
- Exact-head WAN-DHCP, XY-C1 Build/TRX/artifact and Process lifecycle Actions are required. Physical router L4 remains NOT VERIFIED.

## 2026-11-20 — Preserve modem-priority NVRAM persistence semantics while checking the write

- Parent SHA: `d6241fe7630a8df996127c7b295e69a675656641`. Review after the first passing source-contract run found that the initial checked implementation had used the volatile `nvram_set_int_temp()` helper, changing the prior call’s `nvram_set_int()` persistence semantics.
- Added a status-preserving persistent integer helper and switched the modem reconnect priority publication back to `nvram_set_int()` semantics. The reconnect still fails closed when the write fails; only the previously ignored status is changed. The WAN source contract now enforces the persistent setter and errno propagation.
- Exact WAN-route/Process and XY-C1 Build/TRX/artifact reruns required.

## 2026-11-20 — Match split-line modem-priority source assertion

- Parent SHA: `34235bef7131e72d1d006df5678a4729138501e8`. Static source assertion was checked against the actual C body before commit and exposed that the function call’s parameter is on the following line; the prior Python checker expected a space-normalized one-line prefix and would fail before reaching the runtime regression.
- Changed the checker to find the function-call prefix and the quoted `modem_prio` argument independently, preserving explicit ordering through failure gate/log and reconnect enqueue. No production code change in this follow-up; rerun the exact WAN-route source workflow.

## 2026-11-20 — Gate automatic USB-WAN reconnect on modem-priority publication

- Parent main SHA: `9a5506133875a00f31de8a96a4948b9f0befd93b`.
- `notify_on_internet_state_changed()` previously ignored the return from `nvram_set_int("modem_prio", ...)` and enqueued automatic WAN reconnect even if the priority write failed. That could reconnect the wrong modem/WAN using stale selection state.
- Replaced the unchecked persistent setter with the existing status-preserving volatile NVRAM helper `wan_nvram_int_temp_status()`. The reconnect notification is now skipped and the failure logged unless the priority was published successfully; unrelated user internet-state hook execution remains independent.
- Added a source contract for write → error gate/log → enqueue ordering and for preserving API errno. Exact WAN-route/Process and XY-C1 Build/TRX/artifact validation required; physical L4 NOT VERIFIED.

## 2026-11-20 — Scope the BusyBox DHCP daemonization ordering assertion

- Parent source SHA: `2150854923895688840e00a74d9510b07c2e65e5`; exact-head WAN-DHCP run `44785778326` failed only because the checker chose the first textual `write_udhcpc_pidfile()` occurrence inside the earlier `client_background()` definition instead of the main startup write after the `-d` fork branch.
- The assertion now searches for the PID publication after the actual branch, checks that `client_background()` daemonizes before rewriting the PID identity, and confirms the WAN launcher exercises both `-d` and `-b` options. This is a checker-only correction; exact-head callback/Process/XY-C1 verification remains required.

## 2026-11-20 — Correct DHCP readiness source-check formatting assertion

- Parent source SHA: `dc0e09f7924f415a4bdcfbae4fda07d888994a4b`; exact-head WAN-DHCP run `44785621612` FAILED only at the newly-added Python source assertion: `process_presence_start_postcondition()` is split across two C lines, and the checker’s whitespace-compacted expected text omitted the remaining space before `launch_status`.
- All earlier GNU89 `-Wall -Wextra -Werror` builds and the DHCP lease policy sanitizer binary passed in that run. This is a checker expectation correction, not a production C change; exact-head WAN-DHCP, Process, and XY-C1 reruns required.

## 2026-11-20 — Share detached DHCP child-readiness verification across LAN, WAN and IPTV

- Parent main SHA: `881f42a4d5173ac4de45299f6361cf65d00c6df8`.
- The LAN-specific postcondition is consolidated into `udhcpc_start_readiness.h` and applied to all three RC DHCP launchers: LAN `-d`, WAN `-d/-b`, and IPTV `-d`. BusyBox backgrounds both paths before publishing the PID/start_time record, so launcher exit alone cannot establish that the DHCP client survived.
- The shared helper waits within the existing four-probe policy and validates the exact `udhcpc` PID/start_time identity recorded at each interface-specific PID file. UNKNOWN inventory, wait errors, launcher failures, and missing-child timeout propagate to callers. The source contract asserts all three call sites and the BusyBox fork-before-PIDfile ordering.
- Exact-head WAN-DHCP, Process lifecycle and XY-C1 Build/TRX/artifact validation required. Physical L4 remains NOT VERIFIED.

## 2026-11-20 — Verify the detached LAN udhcpc child before accepting startup

- Parent main SHA: `e91c3a18c0dd676c4d4d7cd7b552b30792aaf655`.
- BusyBox `udhcpc -d` calls `bb_daemonize()` before writing `/var/run/udhcpc_lan.pid` with its PID/start_time identity. Consequently `_eval()` waits only for the short-lived daemonizing parent; a post-fork PID-file or signal-setup failure could make RC report success even though the AP DHCP child exited.
- `start_udhcpc_lan()` now performs a bounded readiness probe using `process_pidfile_read_identity()` and the shared RUNNING/ABSENT/UNKNOWN policy. It accepts only the exact live PID/start_time identity, retries ABSENT for the shared four-probe window, and propagates UNKNOWN, interrupted-wait, timeout and launcher failures. No broad process-name-only match can satisfy the gate.
- The WAN-DHCP source contract now asserts this startup postcondition. The shared process-presence policy binary already covers launcher failure, RUNNING success, ABSENT rejection and UNKNOWN rejection. Exact-head Process, WAN-DHCP and XY-C1 validation required; physical L4 remains NOT VERIFIED.

## 2026-11-20 — Log LAN DHCP acquisition failures without disturbing lease state

- Parent main SHA: `8e3c5143e5bcfd24e5b9886e7aa35df013adf463`.
- BusyBox calls the LAN `leasefail` hook when initial discovery/request retries exhaust their current attempt window; there is no active lease to deconfigure, and the client itself schedules another acquisition attempt. The hook was a silent no-op, leaving AP/router logs without any indication why DHCP was not obtaining an address.
- Added an explicit retry-status log while preserving its successful callback status. The source contract asserts that `leasefail` is observational only and must not run deconfig. The NAK callback remains log-only because BusyBox invokes the separate deconfig event after a valid NAK when an existing lease needs withdrawal; its contract now protects against duplicate teardown.
- This is an observability fix, not a change to DHCP retry or lease semantics. Exact-head WAN-DHCP callback and XY-C1 validation are required. Physical L4 remains NOT VERIFIED.

## 2026-11-20 — Refresh networkmap after an address-changing LAN DHCP renewal

- Parent main SHA: `9d8a9036cdd350a3a45971d8cd5d766d5ec93762`.
- `udhcpc_lan_bound()` applies a changed IP/netmask on a DHCP RENEW, but the post-commit `restart_networkmap()` gate only ran on initial BOUND. A changed subnet could therefore leave the networkmap inventory using the previous LAN range.
- Added a tested policy that refreshes on initial BOUND and on address-changing RENEW, while avoiding a restart for an unchanged RENEW. Updated the production callback and source contract; the address tuple is already committed before this ancillary refresh, so its failure remains logged and does not roll back a valid lease.
- Local GNU89/`-Wall -Wextra -Werror` ASan/UBSan policy regression: PASS. Exact-head WAN-DHCP callback, XY-C1 and related Actions must validate THIS COMMIT.
- Rolling-window rotation: oldest 100 complete records are preserved in `docs/dev-log/2026/11/part-020.md`; root retains the newest 20.

---

## 2026-11-20 — Default MiniUPnP pinhole counters when the kernel rule is missing

- Parent main SHA: `a6a0175208fcbd073d1bbdb424de501a5885cf4b`.
- A live registry entry whose ip6tables rule had disappeared made `get_pinhole_info()` return success without assigning the requested packet/byte outputs. The public wrapper then copied an uninitialized packet count to SOAP callers; `CheckPinholeWorking` could report a false positive and `GetPinholePackets` could expose an indeterminate value.
- Added a shared null-safe counter initializer and call it before the netfilter lookup. A found rule still overwrites the zero defaults; a missing rule deterministically reports zero counters. Added regressions for both outputs and each optional NULL pointer, and pinned the production ordering in MiniUPnP CI.
- Local GNU89/`-Wall -Wextra -Werror` ASan/UBSan policy regression: PASS. Exact-head MiniUPnP and XY-C1 Actions must be checked on THIS COMMIT; L3/L4 for this source change are not yet verified.

---

## 2026-11-20 — Make the OpenVPN domain source assertion Python-safe

- Parent main SHA: `30dec22ecde555e6d6617a15e1820d13ea15bb68`.
- VPN server lifecycle Action #44766269853 was exercising the corrected DNS branch checks when a later OpenVPN source assertion exposed another escaping defect in the checker: nested backslashes before double quotes made its Python expression invalid/incorrect.
- Rewrote the expected domain-publication fragment using `chr(34)` for the two C string-literal quotes. This keeps the required literal `'\\0'` intact without nested quote escaping; no production source changed.
- Exact-head VPN and AI Actions will validate this follow-up.

---

## 2026-11-20 — Match the quoted OpenVPN domain empty-string literal in CI

- Parent main SHA: `3dc44ff3378c9b29f6ec73076bc85015877f6b33`.
- VPN server lifecycle Action #44765846007 passed all GNU89 C policy binaries, then the source contract reached the OpenVPN client-up check and failed because its expected token omitted the C single quotes around `'\\0'`.
- Production `on_client_ifup()` correctly publishes the domain snapshot when policy permits it, otherwise publishes an empty value before the state-up marker. Updated the whitespace-normalized checker to assert the exact quoted C literal; production code and runtime behavior are unchanged.
- Exact-head VPN and AI Actions required for this checker-only repair.

---

## 2026-11-20 — Correct VPN DNS cleanup checker branch matching

- Parent main SHA: `d83eb501bd09fd56f2662e08ded2c434069f8bf1`.
- The first checker update corrected the stale NVRAM getter condition, and its exact-head VPN run #44765563839 still failed because the newly added resolver-branch assertion matched a literal backslash-n instead of the actual newline/tab sequence in the Python-extracted C body.
- Reworked that check to construct the expected branch text with `chr(10)` and tab characters; it still requires resolver refresh to be the `else` branch after the DNS snapshot clear status check.
- This is a second checker-only correction. The VPN C policy regression and other compiled steps passed in both runs; no product source changed.

---

## 2026-11-20 — Align VPN DNS cleanup source contract with checked getter semantics

- Parent main SHA: `acc1a3d1025f8826af2ea4ebfe524fb09b5609c4`.
- VPN server lifecycle Action #44763108307 passed all seven compiled C regression steps, then failed in `scripts/check_vpn_client_nvram_state.py`: the checker still expected the old getter condition `api_result == 0 || vpnc_dns[0] == '\\0'`.
- The checked-getter policy defines return 1 plus a present value as “present”, and return 0 plus an absent value as “not present”. Production cleanup correctly clears the DNS snapshot only for a successful present, non-empty value, then rebuilds resolver state only after the clear succeeds.
- Updated the source contract to assert both helper semantics and the production non-empty/present guard, including that resolver refresh is only on the successful-clear branch. This is a checker-only correction; no production source changed.

---

## 2026-11-20 — Normalize IPv6 pinhole wildcards and stage firewall registry updates

- Parent main SHA: `71cfd2831e0c6d959638d82243306bb680d23053`.
- Defect: the netfilter backend treated RemoteHost="*" as an IPv6 parse failure but still installed an all-ones source mask, creating ::/128 instead of a wildcard. Duplicate lookup used the same invalid token against an uninitialized address. Invalid IPv6 arguments were not rejected at this backend boundary.
- Added a shared IPv6 policy: NULL/empty/"*" consistently normalize to ::/0; malformed remote/internal values fail closed. UID allocation checks at most 65,535 values and returns ENOSPC instead of looping forever.
- Allocate UID/bookkeeping before installing the kernel rule, then publish the preallocated registry record only after a successful firewall commit. Added NULL checks for match/target allocation and release the libip6tc handle on successful commit.
- Added GNU89/Werror/ASan/UBSan host regression coverage and MiniUPnP source contracts; added production backend/header to XY-C1 firmware input filters and the Makefile dependency. Exact-head MiniUPnP and XY-C1 runs required after commit.

---

## 2026-11-20 — Restore mandatory end-of-session handoff checkpoint contract

- Parent main SHA: `2cb3ee3d27446aca096b14f9ac0324110c0afa8b`.
- Exact-head AI handoff run #44763259666 failed because `SESSION_HANDOFF.md` no longer contained the required end-of-session checkpoint heading and the rule that the checkpoint is mandatory while a document edit is conditional.
- Restored that explicit contract and synchronized the live checkpoint, exact-SHA Actions state, active MiniUPnP backend task and verification boundary. This is a documentation/CI repair only; no firmware L3 or hardware L4 claim.

---

## 2026-11-20 — Clear stale VPN domain snapshots across client sessions

- Parent main SHA: `84199a2fe17d774b8bcdba0a1918e7fa10a33160`.
- Defect: `restore_dns_from_vpnc()` cleared the runtime DNS snapshot but left `vpnc_dom_t`; OpenVPN client-up wrote that key only when a non-empty DOMAIN option was supplied. A disconnect/reconnect without DOMAIN could retain a stale domain from the previous session.
- Added checked domain-only cleanup and a regression. VPN cleanup attempts domain clearing even after DNS read/clear failure, preserves the first error, and rebuilds resolver state only after successfully clearing a non-empty DNS snapshot. OpenVPN client-up now publishes the current domain or an explicit empty value before the up-state marker.
- GNU89/Werror/ASan/UBSan policy regression and `scripts/check_vpn_client_nvram_state.py` are wired into VPN stability CI. Exact-head VPN and XY-C1 results are required; no firmware L3/hardware L4 is claimed.

---

## 2026-11-20 — Align WAN route callback contract with MTU retry state

- Parent main SHA: `125c64c1b38f9290500904bc5de73fcfb0ed3e83`.
- WAN route exact-head run `#44761848737` failed because the embedded source contract still expected the pre-MTU expression `callback_status = route_setup_status;`. The DHCP callback now aggregates route and MTU status before publishing retry metadata.
- Updated that assertion to the production aggregation expression. This is a checker-only repair; no product source changed.

---

## 2026-11-20 — Align WAN renewal source contracts with lease/route/MTU retries

- Parent main SHA: `533de8ffee96d62ac4c2a82fe7a31bd7cdaa0ccc`.
- The new retry policy now triggers a full DHCP bound re-entry for any retained lease/address, route, or advertised-MTU setup error. The WAN-route and HTTPD source contracts still searched for the older route-only helper, so they failed despite the production retry path being present.
- Updated both checkers to require the three-state helper and verify that WAN/IPTV renewal reads each corresponding NVRAM retry marker before parsing the changed lease tuple. No production source changed in this CI repair.
- Exact-head WAN-route, HTTPD and XY-C1 evidence must be checked separately; previous failures on `5c2cbb39` are not transferable.

---

## 2026-11-20 — Reject zero internal ports in MiniUPnP mapping requests

- Parent main SHA: cd6734bf52ef0bcf7429ee0b7f8978fc361660f3.
- SOAP AddPortMapping accepted NewInternalPort=0 because it checked only the external port after decimal conversion. AddAnyPortMapping had a parallel parse-plus-zero check. Port zero is not a valid internal TCP/UDP endpoint.
- Added `miniupnpd_soap_parse_nonzero_port()`, which validates into a temporary so failed parsing does not clobber the caller's output. Both mapping creation handlers use it; external-port zero semantics are unchanged.
- Added boundary tests for 1, 65535, zero, NULL and 65536, plus source contracts proving validation occurs before redirect creation. Exact-head MiniUPnP and XY-C1 checks pending.

---

## 2026-11-20 — Strictly parse numeric arguments in RC switch front-ends

- Parent main SHA: `9f2b19c2dac08dc1fb03d0da1e282a549d3c8594` (its parent source SHA is `bc0d9316777baa125e2ba22ef776d9ea812e3dd4`; intervening commit refreshed documentation only).
- Defect: `rtl8367_main()` and `mtk_esw_main()` used `atoi()` for command IDs and `get_param_int_hex()` for operands without checking complete-token consumption or overflow. Malformed numeric input could silently become a different ioctl command/argument, and extra operands were ignored.
- Added `switch_cli_parse_policy.h`: strict unsigned decimal for command selectors, decimal or explicit `0x` hexadecimal operands; rejects signs, whitespace, trailing data and overflow without changing the output value. Both front-ends now reject argc above four and parse every provided operand before ioctl dispatch.
- Added `tests/switch_cli_parse_policy.c`; local `-std=gnu89 -Wall -Wextra -Werror -fsanitize=address,undefined` run passed. The Switch CLI workflow compiles/runs the test and checks source wiring; XY-C1 production-input filters include both front-ends and the policy header/test.
- New exact-SHA Switch CLI/XY-C1 Actions pending; no MIPS firmware L3 or physical switch/router L4 claim.

---

## 2026-11-20 — Exact-head checkpoint bc0d9316777baa125e2ba22ef776d9ea812e3dd4

- Wireless: completed/success (#44746584209)
- AI handoff docs: completed/success (#44746584183)
- XY-C1: in_progress (#44746584073)
- NVRAM: completed/success (#44746584217)
- Process regression: no exact-SHA run listed
- Process inventory: completed/success (#44746584108)

- Source tasks: required RT3352 MII TxPower failure propagation, rollback error reporting and single-shot MLME publication. No L4 claim.

---

## 2026-11-20 — Treat RT3352 MII TxPower failure as an enable-gate failure

- Parent main SHA: `31de47640faed47555e830d86cd69f7228887d7e`.
- Review found the MII enable transaction still ignored the `iwpriv ... TxPower` result. It could unisolate and publish an enabled radio even when the configured transmit-power value was rejected, leaving the driver at an unintended default.
- The preparation callback now returns its command status. The policy keeps the port isolated, attempts RadioOn=0 and returns the original TxPower failure instead of unisolating/publishing. Added GNU89 regression coverage for rollback order and no publication.
- Wireless #44746039419, AI handoff #44746039183 and Process inventory #44746039325 passed on `31de4764`; NVRAM #44746039252 was still running. On production source `d250aff4`, NVRAM #44745706021 passed and XY-C1 #44745705732 remained in progress. This changes production code and requires fresh same-SHA XY-C1 validation; no L3/L4 claim.

---

## 2026-11-20 — Correct RT3352 MII unisolation-rollback test event count

- Parent main SHA: `d250aff4e337f5c919e11c44cf5a1e8e5c1cb5a8`.
- Wireless run #44745705779 compiled the policy test successfully, then the C test binary returned FAIL. Inspection of the expected sequence showed the unisolation-failure rollback records six events (initial isolate, radio-on, TxPower preparation, unisolate attempt, rollback isolate, radio-off), while the assertion compared only five and therefore never checked the final radio-off rollback.
- Corrected the expected event count to six. No production source change in this test-only correction.
- On `d250aff4`, AI handoff #44745705703 and Process inventory #44745705762 passed; NVRAM #44745706021 and XY-C1 #44745705732 were still running. New exact-head Wireless/AI reruns pending; firmware L3 and physical L4 not claimed.

---

## 2026-11-20 — Establish iNIC isolation before enabling RT3352 MII radio

- Parent main SHA: `9075dc6385307f5df83ce9d13039462648395583`.
- Follow-up review found a boot/out-of-band state gap: enable assumed the RTL8367 iNIC port was already isolated and turned on radio power before checking switch state. A stale/unexpected unisolated port could expose the newly enabled radio during the transition.
- Enable now explicitly asserts switch isolation first; any initial-isolation error aborts enable, performs best-effort RadioOn=0, preserves the isolation error and does not publish state. Only then does it enable the driver, apply TxPower and unisolate. Existing power-on/unisolation rollback remains.
- Extended GNU89 regression coverage with initial-isolation failure and event-order assertions. Strengthened the Wireless source contract to require isolation-before-power-before-unisolation.
- Parent exact-SHA Wireless #44744845103, AI handoff #44744844991 and Process inventory #44744845149 passed on `9075dc6385307f5df83ce9d13039462648395583`; NVRAM #44744845007 was pending. Product-source XY-C1 #44744529158 on `9e9d643d` was still running. New exact-head regression pending; no MIPS L3 or hardware L4 claim.

---

## 2026-11-20 — Move RT3352 MII policy regressions to file scope and restore handoff checkpoint contract

- Parent main SHA: `9e9d643dd8c3a78d135c8dae908857f8b90c0276`.
- Wireless radio-state run #44744529117 on `9e9d643dd8c3a78d135c8dae908857f8b90c0276` failed at GNU89 compilation: the added MII mock callbacks and test functions had been inserted inside `main()`, producing invalid storage-class errors. Moved the complete helper/test block to file scope while retaining the seven regression calls inside `main()`.
- AI handoff check #44744529015 failed because the shortened SESSION_HANDOFF omitted the mandatory checkpoint language and four required state/task/verification/next-action sections. Restored the exact governance phrases and required headings without changing bootstrap/protocol rules.
- On the source SHA, Process inventory #44744529131, Process regression #44744528956, HTTPD security #44744528974, watchdog #44744529024 and iNIC signal #44744528955 passed. XY-C1 #44744529158 and NVRAM #44744528984 were pending. Corrected Wireless/AI exact-head Actions and same-source L3 remain pending.

---

## 2026-11-20 — Make RT3352 MII radio/isolation transitions transactional

- Parent main SHA: `cb4cb1debdfc2b5ba3b773e2e5fbbb6b58449f96`.
- `mlme_radio_rt_apply()` ignored the `phy_isolate_inic()` switch ioctl result and published MLME/NVRAM plus LED state even when the RTL8367 iNIC isolation boundary had not transitioned successfully.
- Added an RT3352 MII-only injectable transition policy. Enable keeps the port isolated until radio power is on; failed power/unisolation triggers best-effort isolate/off rollback. Disable attempts isolation before radio-off but still powers the radio down if isolation fails. MLME/LED publication is gated on successful required hardware operations.
- Added GNU89 regressions for event ordering, failed power-on rollback, unisolation/isolation failure, power-off failure and no-publication behavior. Updated Wireless source-contract checks and the XY-C1 production-input trigger for the helper header.
- Local validation: standalone transition-policy harness passed `-std=gnu89 -Wall -Wextra -Werror -fsanitize=address,undefined`. Exact-head Wireless/Process/AI/NVRAM/XY-C1 checks are pending; no target MIPS L3 or physical L4 claim.

---

## 2026-11-20 — Fix MiniUPnP lease write errno source contract

- Parent main SHA: `3bdb1d9f68767aeea29ce14bc93959426ee382e4`.
- Exact-head MiniUPnP regression #44743068543 on `ee9428dcaedd7530695ddfeedcac7a326448a705` compiled and passed every C policy/sanitizer binary, then failed its Python source contract at the lease writer. The checker compared the first `errno = 0;` (inside the mutually exclusive record-format error branch) against `fwrite()`, so it did not test the reset on the normal write path.
- Corrected the contract to select the nearest errno reset before the write call; the final-close ordering assertion was already corrected by parent commit `3bdb1d9f`. Verified the write and close reset positions against current `upnpredirect.c`.
- Parent exact-SHA MiniUPnP regression #44743367478 and AI handoff #44743367373 PASS. The failed #44743068543 run is on the preceding SHA and is not a current-head failure. New exact-head Actions are pending; no L3 claim.

---



## 2026-11-20 — MiniUPnP primary mapping deletion state

- Added `delete_redirect_and_filter_rules_ex(..., int *primary_removed)` while preserving `delete_redirect_and_filter_rules()` as a compatibility wrapper.
- Distinguish a completed NAT scan with no matching DNAT mapping, a successfully removed NAT/filter pair, a successful NAT restoration after filter failure, and a failed restoration. An incomplete NAT scan or failed NAT deletion keeps the lease state.
- The UPnP caller now removes description/lease metadata based on primary mapping state, not aggregate status that can also reflect peer SNAT/DSCP cleanup. Aggregate error reporting for secondary cleanup is preserved.
- Added policy regression cases for all primary-state branches; updated the IPTC delete checker and workflow path/source-contract checks.
- Checker follow-up: `616b02a8b441038ac67db94fa106df1831c178dd` fixes the multi-line lease replacement assertion; `64d76a2c98a4fb0ed98f8aba899a71c44750f407` updates the handle-ownership checker to the new function signature.
- Checkpoint HEAD: `35e319a57af04e113fc9a3ccaa31628764ce4cfd`. Exact-SHA Actions status:
- nvram-transaction-boundary: in_progress
- inventory: completed/success
- validate-handoff-records: completed/success
- Cross-build XY-C1 firmware: in_progress
- miniupnpd-config-regression: completed/failure
- NVRAM transaction boundary audit (#44803257169): in_progress
- XY-C1 Firmware Validation (#44803257077): in_progress
- AI handoff documentation contract (#44803257093): completed/success
- Process lifecycle inventory - 35e319a57af04e113fc9a3ccaa31628764ce4cfd (#44803257124): completed/success
- miniupnpd atomic configuration publication regression (#44803257122): completed/failure
