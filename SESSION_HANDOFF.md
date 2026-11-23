# SESSION HANDOFF — 2026-11-23

## Current repository state

- Repository: Arlene206/padavan-4.4-2; work on main only.
- Main parent for this checkpoint: 280e959c899772cb85d76935a13de8e626f1f8f7. Fetch live main before every write; parallel sessions can advance it quickly.
- USB modem eject production fix is in 9e9efbf6d9651e9bab82e544b02f718655a0a820. Dedicated USB identity #45154233466 PASS; Process fail-closed #45155481370 and USB storage hotplug #45155481256 PASS on descendant SHA 15c93fe802fc6a45148d04c8d3c7f238ad11fd92. XY-C1 #45154946558 on descendant product SHA eac5eeee93a83905371ea87e00bff40a66fcd828 passed Build, TRX verification and artifact upload on that same SHA.
- Current product-source baseline is 0e4976cfbe86c81f5172ce2256ed71801952682d, which adds checked radio-enable reads in 802.1X startup, guest activation and DBDC peer-band restart. Wireless #45156939172 passed policy tests but failed a mixed raw/compact guest offset assertion. Descendant #45157423895 corrected offsets but exposed a compact-return spacing assertion; descendant #45157966807 on 280e959c passed all policy tests but exposed the same spacing mismatch in the subsequent DBDC return-order lookup. This checkpoint fixes that remaining lookup.
- AI handoff docs #45157423846 failed because its exact required heading was missing. The canonical heading was restored in 280e959c; AI handoff #45157966954 then PASS on that SHA.
- On product SHA 0e4976cf, Process fail-closed #45156939224, HTTPD security #45156939207, Process inventory #45156939102, iNIC #45156939083, AI handoff #45156939070, watchdog #45156939026 and NVRAM #45156938997 passed. XY-C1 #45156939135 remained in progress at Build at last poll.
- XY-C1 #45157423889 completed workflow success but skipped product Build/TRX/artifact. XY-C1 #45157966804 on checker/docs descendant 280e959c was in progress at last poll, with Build not yet complete and TRX/artifact not yet reached. The latest fully confirmed same-source Build/TRX/artifact PASS is #45154946558 on eac5eeee, not on 0e4976cf.
- Frozen backup branch backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807. Read-only; do not update, delete, merge or rename.
- No persistent local checkout was available. Do not claim local compilation, local sanitizer execution, firmware builds or physical hardware tests unless actually observed.
- Startup recovery has already been performed once. Do not repeat broad startup or completed USB eject/MTK/WSD investigations without new evidence.

## Active engineering baseline

- Product source stays at SHA 0e4976cfbe86c81f5172ce2256ed71801952682d; main 280e959c is a workflow/docs-only descendant.
- Wireless guest activation correctly checks radio enable and mode before interface state inspection/bridge work. DBDC restart correctly checks the peer radio, performs best-effort auth/interface cleanup and returns its negative peer-status error.
- The remaining current Wireless source-contract failure is representation-only: the workflow searches a whitespace-stripped function body, and every expected token/index must use the compact spelling. The previous checker fix corrected the direct token assertion; the second `compact.index()` call for the return-order check still contained a space. This checkpoint corrects that second occurrence. Do not weaken or remove the fail-closed return assertion.
- AI handoff checker requires the exact section heading `## Exact first action for the next GPT`; this heading is restored and its exact-head docs run passed.

## Active task

Close the last known false failure in the Wireless radio-state source contract. Exact Wireless #45157966807 on 280e959c passed all compiled policy tests, the Ralink config ASan/UBSan test, bridge membership, guest-isolation tests and initial DBDC presence checks, but failed with `ValueError: substring not found` at inline Python line 656. The failure was traced to a second lookup of `return -<peer_status>;` against a whitespace-stripped source string. The first presence check already uses `return-<peer_status>;`; this checkpoint fixes the corresponding ordering lookup the same way. No runtime C source changed.

The AI handoff heading correction is already verified: #45157966954 PASS on SHA 280e959c.

## Exact first action for the next GPT

1. Inspect exact-head Wireless radio Action for this checkpoint. Confirm both compact return checks, the guest radio/mode ordering contracts, all policy executables, bridge/isolation tests and Ralink ASan/UBSan steps pass. Diagnose any remaining assertion/log without weakening runtime checks.
2. Inspect the exact-head AI handoff documentation action and ensure it remains PASS after the refreshed handoff/log/roadmap.
3. Check XY-C1 run #45157966804 on SHA 280e959c and product run #45156939135 on source SHA 0e4976cf. Record Build XY-C1, TRX verification and artifact upload separately. Require all three on the same product-source SHA for L3; a descendant run that skips those stages is not a pass.
4. Continue an independent P1 from REFACTOR_ROADMAP.md only after verifying a concrete defect and its call graph. WAN ARP gateway-MAC parsing in `detect_wan.c` remains an investigation candidate; first establish accepted NVRAM MAC formats and legacy compatibility before making a strict parser.

## Exact-SHA CI facts at the parent baseline

- Parent main SHA: `280e959c899772cb85d76935a13de8e626f1f8f7`, commit `test(wireless): align compact DBDC status contract`.
- Wireless #45157966807 on 280e959c passed policy executable, Ralink config ASan/UBSan, bridge membership, guest isolation and initial DBDC checks, then failed at line 656 when a compact source string was searched for a whitespace-containing return literal. The remaining lookup is corrected in this checkpoint; exact rerun pending.
- AI handoff #45157966954 on 280e959c PASS after restoring the validator-required section heading.
- XY-C1 #45157966804 on 280e959c was IN_PROGRESS at Build when last polled; TRX/artifact had not been reached. Product-source XY-C1 #45156939135 on 0e4976cf was also IN_PROGRESS at Build at the last poll.
- On 0e4976cf, Process fail-closed #45156939224, HTTPD security #45156939207, Process inventory #45156939102, iNIC #45156939083, AI docs #45156939070, watchdog #45156939026 and NVRAM #45156938997 PASS.
- USB modem identity #45154233466 PASS on 9e9efbf; Process fail-closed #45155481370 and storage hotplug #45155481256 PASS on 15c93fe. XY-C1 #45154946558 on product descendant eac5eeee passed Build, TRX verification and artifact upload on that same SHA. This L3 result does not transfer to 0e4976cf or later product-source changes.
- No physical hardware L4 result is claimed.

## Completed source slices to preserve

## Completed source slices to preserve

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

- L1 source review: PASS for checked guest radio/mode ordering and DBDC peer-read error cleanup/negative status return.
- L2 host/Actions: Wireless #45157966807 passed all compiled and sanitizer-backed policy tests but failed the second compact-string return assertion. This checkpoint fixes the remaining whitespace mismatch; exact rerun pending. AI handoff #45157966954 PASS.
- L3 XY-C1 firmware: PASS on descendant product SHA eac5eeee only (#45154946558 Build, TRX and artifact upload all PASS). NOT VERIFIED for product SHA 0e4976cf; #45156939135 was still building, and descendant #45157966804 had not completed Build/TRX/artifact at last poll.
- L4 physical router: NOT VERIFIED, including DBDC restart, 802.1X daemon recovery, guest bridge/radio transitions, RT3352 MII and actual RF behavior.

## Working method

- Each logical task is: inspect a concrete source defect, change production source, add or strengthen a regression contract/test, commit one complete tree, then inspect GitHub Actions for that exact SHA.
- Multi-file writes must create all blobs, one full Git tree and one commit. Update main once using force=false and expected_sha set to the current parent. Never expose a sequence of partially updated files.
- Fetch the live main SHA before writes. If another session moves it, rebase the intended changes onto that new main after inspecting intervening commits; never overwrite someone else's work.
- PASS means only the named workflow for its exact SHA passed. FAIL means inspect logs and fix the failing condition. PENDING means record the run and continue another independent safe task.
- Preserve GNU89, Werror, sanitizers where appropriate, Linux 4.4 / MIPS / uClibc compatibility, hardware isolation order, error normalization and rollback semantics.
- Update dev_log.md for each meaningful milestone. The root log had 101 dated records before this checkpoint and will have 102 after this entry. Do not rotate until a new entry would hit 120. Keep records newest first.
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
