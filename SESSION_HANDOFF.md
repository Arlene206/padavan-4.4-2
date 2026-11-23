# SESSION HANDOFF — 2026-11-23

## Current repository state

- Repository: Arlene206/padavan-4.4-2.
- Main SHA before this checkpoint commit: be7119cfe74032e33be51ead097f738c0a55c191. The iNIC source fix is ae45305fec4c7a012c00e89756a80c184589c472; this commit corrects two stale source-contract assertions without changing runtime source. The next GPT must fetch the live main ref and latest commit before any edit because other parallel sessions can advance main while a task is running.
- The iNIC source fix is in ae45305fec4c7a012c00e89756a80c184589c472. This checkpoint is one complete main commit containing two corrected regression contracts plus the development log, roadmap and this handoff. Never infer its final SHA from the parent recorded here.
- Frozen backup branch backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807. It is read-only and must not be updated, deleted or merged.
- Work only on main. No branches, temporary checkpoint refs, force pushes or PRs. The historical PR #1 is closed and unmerged; all authorized content was already ported.
- No local checkout was available in this session. Do not claim local builds, firmware images, sanitizer execution or physical-device tests. Use exact-SHA GitHub Actions as the evidence boundary.
- Startup protocol and handoff have already been restored once for this takeover. Do not repeat the initial repository audit or revisit completed WSD, DNSMasq, WAN and MTK investigations unless a new failure is observed.

## Active task

The production fix is already committed in ae45305fec4c7a012c00e89756a80c184589c472. Its first exact-SHA validations exposed two stale source-contract assertions, not production/policy binary failures: iNIC #45152161414 failed because its checker mixed compacted offsets with an un-compacted substring, and Wireless #45152161484 still expected the legacy get_mlme_radio_rt() guard. This checkpoint fixes both contracts while retaining positive assertions for checked reads, errno capture, parser/error gates and operation ordering. The exact reruns are pending.

A concrete fail-open behavior was found in the RT3352 iNIC health callback, check_inic_mii_rebooted() in trunk/user/rc/src/net_wifi.c.

The callback runs every 15 seconds in the iNIC health-monitor daemon. Before this change it called get_mlme_radio_rt() and get_mode_radio_rt(), which read through nvram_get_int()/nvram_wlan_get_int(). Those legacy convenience APIs collapse NVRAM device-node, ioctl and close errors into a numeric zero. The two zero fallbacks had hardware side effects:
- If the mlme_radio_rt read failed, the monitor treated the state as radio disabled and sent iwpriv RadioOn=0. A transient backend failure could therefore power off the iNIC even though the configured state was unknown.
- If the radio-state read succeeded as enabled but rt_mode_x failed, the monitor guessed operating mode zero (AP) and could start/recover the guest interface despite not knowing the actual configured mode.

The source now reads mlme_radio_rt and rt_mode_x through nvram_get_checked(), captures errno immediately, and uses the existing wireless_radio_nvram_flag_status() and wireless_radio_nvram_int_status() parsers. An absent or empty key retains the old zero default. Backend, malformed-value and invalid-API errors are logged and returned before the corresponding hardware or guest-interface action. The source contract in scripts/check_inicd_signals.py pins both reads, the parser/error gates and operation ordering, and rejects the legacy unchecked getters in this callback.

The existing tests in tests/wireless_radio_policy.c already cover the status parsers for missing/empty values, backend errors, invalid API results, malformed numeric strings and integer overflow. They are compiled by the Wireless radio state regression; net_wifi.c also triggers the iNIC signal lifecycle workflow and the wider Process inventory/XY-C1 path filters.

## Exact first action for the next GPT

1. Fetch live main and inspect this checkpoint's exact commit, then inspect the exact-head iNIC signal lifecycle and Wireless radio state Actions. Confirm both static contracts pass and that their GNU89 (including ASan/UBSan) policy binaries remain green.
2. Inspect the exact-head Process inventory/lifecycle and XY-C1 runs. Run 45150223763 on older product SHA 45980023a2894eb0bf13257b02ee8f4cc5b5b4ac is PASS including Build, TRX and artifact upload, but this does not transfer L3 to ae45305, ac770e45 or later product commits. Newer XY runs 45152161385 (ae45305) and 45152197051 (ac770e45) were IN_PROGRESS at last poll. Resolve a real failure at that SHA; if a workflow is PENDING, work on an independent source task rather than waiting.
3. Check XY-C1 Build, TRX verification and artifact upload for the same product-source SHA. A docs or test-only descendant does not confer firmware verification on a later production source commit. L3 is not verified until all three target steps pass on the exact production source SHA.
4. Once the iNIC slice is green, continue the next independent P1 from REFACTOR_ROADMAP.md. Good candidates include the unfinished storage/USB hotplug status propagation, WAN ARP validation L3, Samba account/config identity handling or remaining DNSMasq DHCPv6 transaction boundaries. Before editing, check whether another concurrent session already closed the issue and inspect the relevant workflow.
5. Make a focused source fix and regression for an evidenced defect; do not turn an unproven suspicion into a code change. Trace callers, target compatibility, error conventions and rollback side effects before changing interfaces.

## Exact-SHA CI facts at the parent baseline

The parent main SHA before this checker-correction checkpoint is be7119cfe74032e33be51ead097f738c0a55c191:
- Switch CLI ioctl failure-status regression 45150824553: PASS on this exact SHA after the WAN PHY best-effort first-error source contract was corrected. GNU89 ASan/UBSan MTK ESW policy tests and static contracts passed.
- AI handoff documentation contract 45150824594: PASS on this exact SHA.
- XY-C1 Firmware Validation 45150223763: product-source SHA 45980023a2894eb0bf13257b02ee8f4cc5b5b4ac completed SUCCESS; Build XY-C1, TRX verification and artifact upload all PASS on that exact SHA. This establishes L3 only for 45980023, not later product commits.
- iNIC signal lifecycle stability 45152161414 on ae45305: policy executable PASS; Python source contract FAIL due to mixed compacted/non-compacted offsets, fixed in this checkpoint.
- Wireless radio state 45152161484 on ae45305: GNU89 radio-policy, Ralink config/SKU, bridge membership and guest-isolation executables PASS; final Python source contract FAIL due to the stale get_mlme_radio_rt() assertion, fixed in this checkpoint.
- XY-C1 45152161385 on ae45305 and XY-C1 45152197051 on ac770e45 were IN_PROGRESS at last poll; no newer L3 claim.
- On ac770e45, Process inventory and lifecycle #45152197111 and NVRAM transaction boundary #45152197109 were IN_PROGRESS. A descendant watchdog HTTPD probe/ NTP status source fix was committed on be7119c; inspect exact-head workflow status separately.
- On product-source SHA 45980023a2894eb0bf13257b02ee8f4cc5b5b4ac, the NVRAM transaction audit 45150223232, Process inventory 45150224244, DNSMasq lifecycle 45150223112, WAN DHCP callback 45150222970, WAN route status 45150223168, Firewall Startup 45150223557, Wireless radio state 45150223325, HTTPD authentication/security 45150222876, IPv6 checked-read 45150223345, detect_link signal 45150223229, watchdog signal 45150223005, Process inventory/lifecycle umbrella 45150223022 and detect_internet signal 45150223214 all PASS.
- The above 45980023 Actions precede the present iNIC source edit. Their PASS conclusions remain evidence for only the files and SHA they tested. The iNIC source change required fresh exact-head verification; the first checks exposed only stale contract assertions described above, and this correction commit triggers fresh exact-head reruns.
- Older Switch source failures on 22f6877 and 14623e0 are superseded by the PASS on 3a7582e; do not re-open them.

## Completed source slices to preserve

### MTK ESW / switch / PHY

- Strict RC switch CLI parsing and packed ioctl-field bounds prevent out-of-range or shifted parameter values from aliasing another operation.
- MTK link-mode programming validates reserved speed/flow encodings before MDIO and publishes the cached mode only after the register sequence succeeds.
- MT7530 ATC/MAC-table clear checks command writes and busy-status reads, returning EIO on MDIO errors and ETIMEDOUT when the busy bit remains asserted.
- PHY power operations propagate errno through PORTS_POWER and PORTS_WAN_LAN_POWER. Both selected-port loops continue through all independent PHYs, preserve the first normalized error with mtk_esw_first_error(), and retain successful changed-state flags for bridge recovery.
- Exact Switch CLI run 45150824553 passed on 3a7582ec84255ac7639037591ff54ac9fca97f25. It is an L2 host/action result only; physical MDIO/link/port-power behavior is still not verified on hardware.

### DNSMasq split-DNS

- fill_dnsmasq_servers() takes one bounded NVRAM transaction snapshot of vpnc_pdns, vpnc_dom_t, vpnc_dns_t, wanx_domain and wanx_dns.
- Read, close, parse or truncation errors abort before temporary server-file staging/publication, so a backend fault cannot silently remove VPN/MAN routing and leak those names to the default resolver.
- DNSMasq exact Action 45150223112 passed on 45980023. The policy regression covers transaction failures. Do not repeat this completed audit without a newly observed fault.

### Ethernet LED checked NVRAM

- Commit 45980023a2894eb0bf13257b02ee8f4cc5b5b4ac changed update_ether_leds() to read ether_led0, ether_led1 and led_ether_t via status-bearing checked NVRAM APIs before touching hardware.
- Any read failure preserves current hardware LED state instead of interpreting backend failure as zero. Process inventory 45150224244 and WAN DHCP callback 45150222970 passed on that source SHA.

### WAN callbacks and route operations

- Checked DHCP renew snapshots preserve prior lease/route/MTU state when NVRAM reads fail. Exact WAN DHCP Action 45150222970 passed on 45980023.
- WAN route operation status Action 45150223168 passed on 45980023. Scope the claim to the covered source and test paths.

### WSD HTTP/TCP reliability

- Accepted-client recv errors close only that client instead of restarting the WSD listener. A single five-second deadline spans the request line, headers and declared body, preventing a slow-trickle peer from indefinitely occupying the single-threaded server.
- WSD HTTP/TCP/XML and Process source fixes were verified on exact descendant SHA 940c53921c210898199bfc78b1b1b0cb33c206d6. Do not repeat unless WSD source changes.

## Verification boundary

- L1 source review: PASS for the described iNIC read path and existing status-parser implementation.
- L2 host/Actions for this iNIC monitor change: source policy executables passed on ae45305, but both source-contract workflows failed on stale assertions. This checkpoint corrects those assertions; new exact-head iNIC and Wireless runs are PENDING until observed.
- L3 XY-C1 firmware: PASS only on older source SHA 45980023 (Build, TRX and artifact all succeeded); NOT VERIFIED for ae45305, ac770e45, be7119c or this checker-correction descendant until the corresponding exact-SHA product run passes all three target steps.
- L4 router hardware: NOT VERIFIED, including RT3352 MII PHY recovery, guest AP recovery, MDIO fault injection and electrical radio/switch sequencing.
- Do not extrapolate results from XY-C1 to RT3352 hardware or other boards.

## Working method

- Each logical task is: inspect a concrete source defect, change production source, add or strengthen a regression contract/test, commit one complete tree, then inspect GitHub Actions for that exact SHA.
- Multi-file writes must create all blobs, one full Git tree and one commit. Update main once using force=false and expected_sha set to the current parent. Never expose a sequence of partially updated files.
- Fetch the live main SHA before writes. If another session moves it, rebase the intended changes onto that new main after inspecting intervening commits; never overwrite someone else's work.
- PASS means only the named workflow for its exact SHA passed. FAIL means inspect logs and fix the failing condition. PENDING means record the run and continue another independent safe task.
- Preserve GNU89, Werror, sanitizers where appropriate, Linux 4.4 / MIPS / uClibc compatibility, hardware isolation order, error normalization and rollback semantics.
- Update dev_log.md for each meaningful milestone. It retained 93 dated records before this checkpoint; it now has 94. Do not rotate until a new entry would hit 120. Keep records newest first.
- REFACTOR_ROADMAP.md is the single whole-repository status dashboard. Update materially changed module state; do not append duplicate audit snapshots or grow already-large documents unnecessarily.
- Keep this handoff as one current checkpoint and rewrite it in place. Do not append an old snapshot after it or create additional handoff archives.

# Cross-session execution mandate

- The user expects continuous development across chats. Do not ask what to do next, ask for repeated authorization, or report every micro-step.
- Main is the only writable branch. The backup is frozen. Every logical task must end in one atomic main commit; force push is prohibited.
- Before context exhaustion, complete the current safe slice, check exact-SHA Actions, and leave the next GPT a precise first action, source baseline and verification boundary.
- Never claim local tests, firmware builds, artifacts, commits, Actions or hardware results that were not observed.

## Mandatory handoff checkpoint

Each session must compare the live repository with this checkpoint and update it only when the source baseline, active task, exact-SHA evidence, verification boundary or next action materially changes.

The checkpoint is mandatory; a document edit is conditional. A stale handoff is a defect; a no-op documentation commit is churn.
