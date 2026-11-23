# SESSION HANDOFF — 2026-11-23

## Current repository state

- Repository: Arlene206/padavan-4.4-2; writable branch: main only.
- Main parent for this checkpoint: d9a5f32cf7a38b1eeaf9c26095dca35b7ca4fc04. Re-fetch the live main ref before each write; parallel sessions may advance it during the task.
- Production USB modem eject fix is in 9e9efbf6d9651e9bab82e544b02f718655a0a820. USB modem identity #45154233466 PASS; Process fail-closed #45155481370 and USB storage hotplug #45155481256 PASS on descendant SHA 15c93fe802fc6a45148d04c8d3c7f238ad11fd92. XY-C1 #45154946558 on descendant product SHA eac5eeee93a83905371ea87e00bff40a66fcd828 passed Build, TRX verification and artifact upload on that same SHA.
- Current product-source baseline is 0e4976cfbe86c81f5172ce2256ed71801952682d, which adds checked radio-enable reads in 802.1X startup, guest activation and DBDC peer-band restart. Its exact Wireless run #45156939172 failed a raw/compact offset contract; descendant d9a5f32 fixed that mixed-coordinate issue but run #45157423895 then exposed a second stale contract expecting a space in a whitespace-compacted return expression. This checkpoint fixes the assertion to match the compact representation.
- AI handoff run #45157423846 on d9a5f32 failed because the section heading said "next session" rather than the validator-required "next GPT"; this checkpoint restores the required heading.
- On product SHA 0e4976cf, Process fail-closed #45156939224, HTTPD security #45156939207, Process inventory #45156939102, iNIC #45156939083, AI handoff #45156939070, watchdog #45156939026 and NVRAM #45156938997 passed. XY-C1 #45156939135 remained in progress at Build, with TRX and artifact pending at last observation.
- XY-C1 #45157423889 on descendant d9a5f32 completed successfully but Build XY-C1, TRX verification and artifact upload were SKIPPED because that commit changed only workflow/docs. It is not L3 evidence for the product source.
- Frozen backup branch backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807. It is read-only; do not update, delete, merge or rename.
- No persistent local checkout was available. Do not claim local compilation, local sanitizers, firmware builds or physical hardware tests unless actually observed.
- Startup recovery has already been performed once. Do not repeat broad startup or completed USB eject/MTK/WSD investigations without new evidence.

## Active engineering baseline

- Product source remains SHA 0e4976cfbe86c81f5172ce2256ed71801952682d; current main parent d9a5f32cf7a38b1eeaf9c26095dca35b7ca4fc04 is a checker/docs-only descendant.
- The wireless runtime source and policy tests were not changed by the two checker commits. Guest activation correctly orders the checked radio-enable read/error gate, checked mode read/error gate and mode/radio-disabled gate before interface-state inspection. DBDC restart correctly returns the negative peer-radio status after best-effort cleanup.
- Current Wireless regression failure #45157423895 is another source-contract representation mismatch: the test uses a whitespace-stripped function body but asserted a literal with a space after `return`. The corrected assertion is `return-{peer_status};` in the compact string; do not weaken the status propagation check.
- The AI documentation failure #45157423846 is a required-heading mismatch, repaired in this checkpoint by restoring the exact validator wording.

## Active task

Complete the Wireless checked radio-enable slice's remaining source-contract repair. Exact Wireless #45156939172 on product source 0e4976 passed the C/policy, Ralink config ASan/UBSan, bridge-membership and guest-isolation tests, then exposed a raw/compact offset mix. The parallel checker correction on d9a5f32 normalized the guest activation offsets, but exact rerun #45157423895 reached the DBDC contract and failed because its expected return token contained whitespace even though the searched function body was compacted. This checkpoint changes only that expectation to the compacted `return-{peer_status};` spelling and preserves the actual negative status check.

The same d9a5f32 commit had AI handoff validation #45157423846 fail because the handoff heading was changed to "next session" rather than the exact required "next GPT". This checkpoint restores the canonical required heading. No production C changes are included here.

## Exact first action for the next GPT

1. Inspect the exact-head Wireless run triggered by this commit. Confirm every GNU89 policy executable, Ralink ASan/UBSan test and inline source contract passes, including both guest activation and DBDC checked peer-radio return checks. Fix any remaining genuine stale assertion from its log without weakening runtime safety.
2. Inspect the exact-head AI handoff documentation run; confirm the restored heading plus all archive/index/size/CI governance checks pass.
3. Check XY-C1 run #45156939135 on product source SHA 0e4976cf: Build XY-C1, TRX verification and artifact upload must all pass on that same SHA before claiming L3. At last poll the build was in progress and TRX/artifact pending. A descendant checker-only run with skipped product steps is not L3.
4. Once these gates close, continue another independent P1 from REFACTOR_ROADMAP.md. The WAN ARP gateway-MAC parse in `detect_wan.c` remains a candidate only: establish accepted NVRAM formats and compatibility behavior before implementing strict validation.

## Exact-SHA CI facts at the parent baseline

- Parent main SHA: `d9a5f32cf7a38b1eeaf9c26095dca35b7ca4fc04`, commit `test(wireless): use consistent source contract offsets`.
- On product-source SHA 0e4976cfbe86c81f5172ce2256ed71801952682d, Wireless #45156939172 failed only the inline guest source ordering assertion after its GNU89 policy executables, Ralink config/SKU ASan/UBSan, bridge membership and guest isolation steps passed. Process fail-closed #45156939224, HTTPD security #45156939207, Process inventory #45156939102, iNIC #45156939083, AI docs #45156939070, watchdog #45156939026 and NVRAM #45156938997 passed.
- On checker/docs-only descendant d9a5f32cf7a38b1eeaf9c26095dca35b7ca4fc04, Wireless #45157423895 failed at inline Python line 649 because a compacted source string was compared with a whitespace-containing expected token. AI handoff #45157423846 failed because the required exact heading was absent. Both are corrected in this checkpoint; exact reruns pending.
- XY-C1 #45156939135 on product SHA 0e4976cf was still in progress at Build XY-C1; TRX verification and artifact upload were pending at last poll. Descendant XY-C1 #45157423889 completed workflow success but skipped Build/TRX/artifact.
- USB modem identity #45154233466 PASS on 9e9efbf; Process fail-closed #45155481370 and storage hotplug #45155481256 PASS on 15c93fe. XY-C1 #45154946558 on descendant product SHA eac5eeee completed Build, TRX verification and artifact upload successfully. This proves L3 on eac5eeee only, not on later product source 0e4976cf.
- No physical hardware L4 result is claimed.

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

- L1 source review: PASS for checked radio-enable/mode ordering in guest activation and DBDC fail-closed cleanup/negative peer-status return.
- L2 host/Actions: Wireless #45156939172 on 0e4976 passed its C/policy and ASan/UBSan stages but failed the mixed-offset source assertion. Wireless #45157423895 on d9a5f32 passed those test stages but failed the whitespace-sensitive DBDC return-token assertion. The latter is fixed here; exact rerun pending. AI handoff #45157423846 failed the heading requirement; exact wording is restored here, rerun pending.
- L3 XY-C1 firmware: PASS on descendant product SHA eac5eeee only (#45154946558 Build, TRX and artifact all PASS). NOT VERIFIED for current product source 0e4976cf until #45156939135 Build, TRX and artifact all complete successfully on that exact SHA.
- L4 physical router: NOT VERIFIED, including 802.1X daemon recovery, DBDC restart, guest radio/bridge transitions, RT3352 MII and actual RF hardware.

## Working method

- Each logical task is: inspect a concrete source defect, change production source, add or strengthen a regression contract/test, commit one complete tree, then inspect GitHub Actions for that exact SHA.
- Multi-file writes must create all blobs, one full Git tree and one commit. Update main once using force=false and expected_sha set to the current parent. Never expose a sequence of partially updated files.
- Fetch the live main SHA before writes. If another session moves it, rebase the intended changes onto that new main after inspecting intervening commits; never overwrite someone else's work.
- PASS means only the named workflow for its exact SHA passed. FAIL means inspect logs and fix the failing condition. PENDING means record the run and continue another independent safe task.
- Preserve GNU89, Werror, sanitizers where appropriate, Linux 4.4 / MIPS / uClibc compatibility, hardware isolation order, error normalization and rollback semantics.
- Update dev_log.md for each meaningful milestone. The root log had 100 dated records before this checkpoint and will have 101 after this entry. Do not rotate until a new entry would hit 120. Keep records newest first.
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
