# SESSION HANDOFF — 2026-11-23

## Current repository state

- Repository: Arlene206/padavan-4.4-2; writable branch is main only.
- Main parent for this checkpoint: af80debca9112c03b72fbe27dd8cd2838fe82ff2. Re-fetch live main before every write; parallel sessions may advance it.
- USB modem eject production fix is in 9e9efbf6d9651e9bab82e544b02f718655a0a820. USB identity #45154233466 PASS; Process fail-closed #45155481370 and USB storage hotplug #45155481256 PASS on descendant SHA 15c93fe802fc6a45148d04c8d3c7f238ad11fd92.
- Production source baseline before this checkpoint is 0e4976cfbe86c81f5172ce2256ed71801952682d; exact XY-C1 #45156939135 is confirmed PASS for Build XY-C1, TRX verification and artifact upload on that same SHA. Current main parent af80debca9112c03b72fbe27dd8cd2838fe82ff2 is a check/docs descendant of that product source. Wireless #45158308492 and AI handoff #45158308561 passed on af80. XY-C1 #45158308815 skipped product build/TRX/artifact because af80 only changed test/docs files.
- This checkpoint adds strict local source-MAC parsing to detect_wan.c at the ARP packet boundary. It intentionally does not change the legacy WAN MAC override formatter; malformed NVRAM text is rejected only before Ethernet frame construction.
- Frozen backup branch backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807. Read-only; do not update, delete, merge or rename.
- No persistent local checkout is available. Do not claim local compile, local sanitizers, firmware build or physical hardware results unless actually observed.
- Startup recovery has already been performed once. Do not repeat broad startup or completed USB eject/MTK/WSD investigations without new evidence.

## Active task

A malformed-input defect was found in arpping_gateway() in trunk/user/rc/src/detect_wan.c. The WAN MAC override formatter intentionally retains historical behavior and formats a 12-character string into colon-separated pairs without validating hexadecimal characters. The ARP probe then decoded each pair with strtol(tmp, NULL, 16) and ignored where parsing stopped. Invalid nibbles could silently produce zero or partial octets, then be copied into the outgoing Ethernet/ARP source fields and misrepresent the router identity during gateway liveness detection.

The new detect_wan_arp_parse_hardware_address() helper in detect_wan_arp_result_policy.h enforces exactly six colon-separated hex octets (upper/lowercase accepted). Parsing is transactional: no output bytes change on malformed input. arpping_gateway() calls it only when a real probe is needed and returns a checked local probe error before opening the packet socket if parsing fails. The legacy WAN override formatter remains untouched for configuration compatibility.

The existing GNU89 ASan/UBSan executable tests/detect_wan_arp_result_policy.c now covers valid mixed-case values, invalid hex, invalid separators/length, null pointers and output-preservation. The existing WAN ARP workflow pins parser use, malformed-input rejection, removal of strtol, and check ordering before socket creation.

## Exact first action for the next GPT

1. Inspect the exact-head WAN ARP Action for this commit. Confirm GNU89 -Wall -Wextra -Werror ASan/UBSan tests, new parser tests and the full production source contract pass. Fix genuine failures at their source; do not weaken parser or ordering checks.
2. Inspect exact-head XY-C1 Firmware Validation. Since this commit changes product source, Build XY-C1, TRX verification and artifact upload must all PASS on the same commit SHA to claim L3.
3. Continue another independent P1 from REFACTOR_ROADMAP.md only after confirming parallel sessions have not completed it; focus on evidenced faults and add regression tests instead of repeating audits.

## Exact-SHA CI facts at the parent baseline

- Parent main SHA: af80debca9112c03b72fbe27dd8cd2838fe82ff2, commit test(wireless): normalize DBDC peer return lookup.
- On exact parent SHA af80debca9112c03b72fbe27dd8cd2838fe82ff2: Wireless #45158308492 PASS and AI handoff #45158308561 PASS. XY-C1 #45158308815 completed successfully but Build XY-C1, TRX verification and artifact upload were skipped because the parent changed only workflow/docs files.
- Production source SHA 0e4976cfbe86c81f5172ce2256ed71801952682d has exact XY-C1 run #45156939135 PASS, with Build XY-C1, TRX verification and artifact upload all passing on that SHA. This includes USB modem and checked wireless production fixes but not the new ARP source-MAC parser in this checkpoint.
- This commit replaces the permissive strtol MAC decoder in detect_wan.c, adds sanitizer tests and updates the inline ARP source contract. Exact-head WAN ARP and XY-C1 results for this new source change are pending.
- USB modem identity #45154233466 PASS on 9e9efbf; Process fail-closed #45155481370 and storage hotplug #45155481256 PASS on 15c93fe.
- No physical router L4 result is claimed.

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

- L1 source review: PASS for strict six-octet MAC syntax checking, transactional output and error propagation before packet socket creation. Legacy WAN override normalization is unchanged.
- L2 host/Actions: PENDING exact-head WAN ARP regression, including GNU89/Werror/ASan/UBSan and the inline source contract.
- L3 XY-C1 firmware: PASS for parent production source SHA 0e4976cf (#45156939135 Build, TRX and artifact all PASS). NOT VERIFIED for this new source change until exact-head Build XY-C1, TRX verification and artifact upload all pass on one SHA.
- L4 physical router: NOT VERIFIED, including gateway ARP/no-reply, malformed NVRAM/override recovery, WAN failover and router Ethernet hardware behavior.

## Working method

- Each logical task is: inspect a concrete source defect, change production source, add or strengthen a regression contract/test, commit one complete tree, then inspect GitHub Actions for that exact SHA.
- Multi-file writes must create all blobs, one full Git tree and one commit. Update main once using force=false and expected_sha set to the current parent. Never expose a sequence of partially updated files.
- Fetch the live main SHA before writes. If another session moves it, rebase the intended changes onto that new main after inspecting intervening commits; never overwrite someone else's work.
- PASS means only the named workflow for its exact SHA passed. FAIL means inspect logs and fix the failing condition. PENDING means record the run and continue another independent safe task.
- Preserve GNU89, Werror, sanitizers where appropriate, Linux 4.4 / MIPS / uClibc compatibility, hardware isolation order, error normalization and rollback semantics.
- Update dev_log.md for each meaningful milestone. The root log had 102 records before this checkpoint and will have 103 after it. Do not rotate until a new entry would hit 120. Keep records newest first.
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
