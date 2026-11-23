# SESSION HANDOFF — 2026-11-23

## Current repository state

- Repository: Arlene206/padavan-4.4-2; writable branch: main only.
- Main base SHA for this checkpoint: 0e4976cfbe86c81f5172ce2256ed71801952682d (the live main immediately before this checkpoint commit). Re-fetch live main at the next takeover; never chase a documentation-only SHA in this file.
- Frozen archive: backup/pre-plugin-overhaul-2026-11-14 at 72f4f157a084f8184b9597ca5bf334b8080e5807. Preserve permanently; never modify, switch to, merge, delete or rename.
- Branch inventory at takeover: only main and the frozen archive. Legacy PR #1 is closed and unmerged; there are no open PRs. Do not create branches or PRs.
- No persistent local checkout was available. No local compilation, host regression or firmware build is claimed.
- Startup recovery has been performed once. Do not repeat the completed broad recovery or re-audit the USB eject/MTK/WSD work without new evidence.

## Active engineering baseline

- Product-source commit: 0e4976cfbe86c81f5172ce2256ed71801952682d, fix(wireless): fail closed on radio enable read errors.
- This source slice replaces error-collapsing get_enabled_radio_wl/rt() decisions with checked wl_radio_x/rt_radio_x reads in 802.1X startup, guest-AP activation and DBDC peer-band restarts. Read failures stop new authenticator/guest startup before driver, bridge or interface side effects; DBDC paths attempt fail-closed cleanup and preserve the primary read error.
- Exact Wireless run #45156939172 on 0e4976 failed only in the final Python source-contract ordering assertion after all C policy/ASan-UBSan binaries passed. The checker compared byte offsets from un-compacted and compacted representations of the same function. The present checker correction makes every ordering offset use a single compacted representation; it does not remove or weaken any source safety assertion. Its exact-head rerun is the first action below.
- On exact 0e4976: Process inventory #45156939224 PASS; AI handoff documentation #45156939070 PASS; watchdog #45156939026 PASS; iNIC #45156939083 PASS; HTTPD authentication/security #45156939207 PASS. NVRAM transaction boundary #45156938997 and XY-C1 #45156939135 were IN_PROGRESS at the last observation. XY-C1 had reached the target Build step; TRX and artifact were pending. Wireless #45156939172 FAIL is the checker failure described above.
- The earlier WDS source change on a456ced was followed by checker repairs in 15c93fe/cd52aa09/af14a626; current Wireless exact-head action #45156268364 on af14a626 passed. Do not revisit the closed WDS checker task except if a new failure appears.
- The exact product-source XY run #45155297930 on a456ced was still running when inspected, but the newer product-source XY run #45156939135 on 0e4976 is the relevant L3 gate for the current source baseline.
- L1 review: PASS for checked radio-enable error gates and first-error rollback ordering by source inspection. L2: the current Wireless run failed at a static checker offset bug after its C/policy binaries passed; corrected checker rerun pending. Other exact-head runs above are individually scoped. L3: NOT VERIFIED until same-SHA Build XY-C1, TRX verification and artifact upload all pass. L4 hardware: NOT VERIFIED.

## Active task

Finish the checker repair for the new wireless radio-enable source slice. The current atomic change updates .github/workflows/wireless-radio-state.yml so its guest-activation ordering contract uses a single compacted string for all offsets, adds the exact failure/correction record to dev_log.md, and replaces this handoff checkpoint. Product source and policy tests are unchanged.

## Exact first action for the next session

1. Check the Wireless radio state regression triggered by this commit on its exact new main SHA. Confirm every GNU89 / ASan-UBSan policy executable and the complete source contract pass. If it fails, inspect the exact assertion/log and fix the contract without relaxing source safety checks.
2. Check XY-C1 run #45156939135 on product SHA 0e4976cfbe86c81f5172ce2256ed71801952682d: confirm Build XY-C1, TRX verification and artifact upload all complete successfully on that same SHA. A descendant docs/checker-only run that skips these steps is not L3 evidence.
3. Revisit NVRAM transaction boundary run #45156938997 on exact SHA 0e4976 and repair any causal failure before considering that slice verified.
4. After these gates close, choose the next unfinished high-risk module from the current REFACTOR_ROADMAP.md; prefer unresolved P1 stability/security work outside already heavily exercised Wireless/HTTPD/SkipDB. Do not repeat completed modules without new failure evidence.

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

- L1 source review: PASS for checked selected-modem identity verification and pre-teardown ordering.
- L2 host/Actions: USB modem identity #45154233466 PASS on 9e9efbf. Process and storage workflows failed stale source assertions; Process repeated the same old assertion on 56bcddef. Corrected contracts and workflow filters are in this checkpoint, with exact reruns pending.
- L3 XY-C1 firmware: NOT VERIFIED for production USB source SHA 9e9efbf. The current a456ced XY-C1 run #45155297930 was still in progress at last poll; TRX and artifact completion are not yet observed on that SHA.
- L4 physical router: NOT VERIFIED, including USB modem multi-device/eject/reconnect behavior, hub edge cases and MIPS/uClibc runtime.

## Working method

- Each logical task is: inspect a concrete source defect, change production source, add or strengthen a regression contract/test, commit one complete tree, then inspect GitHub Actions for that exact SHA.
- Multi-file writes must create all blobs, one full Git tree and one commit. Update main once using force=false and expected_sha set to the current parent. Never expose a sequence of partially updated files.
- Fetch the live main SHA before writes. If another session moves it, rebase the intended changes onto that new main after inspecting intervening commits; never overwrite someone else's work.
- PASS means only the named workflow for its exact SHA passed. FAIL means inspect logs and fix the failing condition. PENDING means record the run and continue another independent safe task.
- Preserve GNU89, Werror, sanitizers where appropriate, Linux 4.4 / MIPS / uClibc compatibility, hardware isolation order, error normalization and rollback semantics.
- Update dev_log.md for each meaningful milestone. The root log had 94 dated records before this checkpoint and will have 95 after this entry. Do not rotate until a new entry would hit 120. Keep records newest first.
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
