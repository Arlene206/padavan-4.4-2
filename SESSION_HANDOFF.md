# SESSION HANDOFF — 2026-11-23

## Current repository state

- Repository: Arlene206/padavan-4.4-2. Writable branch: `main` only. Parent of this handoff refresh: `3e5bcc5b81fc4ebca8ab9a58d6f7d1495bdca2b4`.
- Frozen archive: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Preserve permanently; never update, switch to, merge, delete or rename.
- Branch inventory at this checkpoint: only `main` and the frozen archive. Legacy PR #1 is closed and unmerged; no open PR. Do not create branches or PRs.
- Current source baseline: `3e5bcc5b81fc4ebca8ab9a58d6f7d1495bdca2b4` (`fix(rc): reject invalid NAT helper boolean selectors`), parent `b8313201f520ecaa00f2bf76cb89821b208824c9`.
- No persistent local checkout is available. No local compile, local sanitizer, MIPS/uClibc build or physical hardware result is claimed.
- Startup recovery has already been performed once. Do not repeat broad bootstrap or completed USB eject/MTK/WSD audits without new evidence.

## Active task

Close exact-SHA validation for strict NVRAM boolean selectors used by `reload_nat_modules()`. The NAT helper snapshot previously interpreted every nonzero `wan_nat_x` as enabled and converted malformed PPTP/H.323/SIP ALG values to disabled unless they were exactly `1`. A corrupted value such as `2` could silently unload a configured helper during a reload.

Source commit `3e5bcc5b81fc4ebca8ab9a58d6f7d1495bdca2b4` now enforces the supported empty/0/1 domain for all four selectors before publishing the snapshot or changing module state. Missing and empty values preserve the legacy disabled default. The GNU89 test exercises `2`, `-1` and malformed text in every selector and verifies the caller's previous snapshot remains untouched. The existing WAN route workflow compiles this policy using `-Wall -Wextra -Werror` and ASan/UBSan, then runs `scripts/check_nat_helper_settings.py`.

The previous firewall URL-schedule task is closed at L3: exact XY-C1 #45161165974 passed on source SHA `ad56220d066e771be499426708b6c2f744937467`, including Build XY-C1, TRX verification and artifact upload (artifact `14812541338`, 12,759,350 bytes). Do not repeat that task.

## Exact first action for the next GPT

1. Poll these exact-source runs for `3e5bcc5b81fc4ebca8ab9a58d6f7d1495bdca2b4`: WAN route #45178293988, XY-C1 #45178294111, Process inventory #45178294031 and NVRAM boundary #45178294032. AI handoff contract #45178294028 already PASSed. If any test fails, inspect the corresponding job log and fix the actual issue; do not loosen the regression.
2. Confirm the WAN route GNU89/Werror/ASan/UBSan binary and source contract pass. For L3, require Build XY-C1, TRX verification and artifact upload all PASS on the same source SHA; record the artifact ID/name/size. Until then, mark L3 PENDING.
3. While runs are pending, continue an independent P1 stability/security task from the roadmap, preferably outside the heavily exercised Wireless/HTTPD/DHCP paths. Never wait idle or repeat a closed audit without new evidence.

## Exact-SHA CI facts at the parent baseline

- Product-source `ad56220d066e771be499426708b6c2f744937467`: WAN DHCP callback #45161165802 PASS. Its first Firewall Startup run #45161165818 failed at a stale inline test-name assertion after earlier checked-NVRAM/lifecycle/startup/enable/static-route stages passed; corrected checker-only descendant db60 has Firewall Startup #45161387113 PASS and AI docs #45161387099 PASS. XY-C1 #45161165974 was IN_PROGRESS at Build XY-C1; TRX and artifact pending.
- LAN networkmap-retry source `edeb1e8354a97b312d8f3f5dd8787af030abfc6d`: WAN DHCP callback #45160239807 PASS on descendant cbec; XY-C1 #45160010405 PASS on exact edeb for Build, TRX verification and artifact upload. Artifact ID 14811585463, name `XY-C1-firmware-edeb1e8354a97b312d8f3f5dd8787af030abfc6d`, 12,759,150 bytes. This is L3 for product source through edeb, not for later product source ad562.
- WAN ARP source-MAC parser regression #45159476012 PASS on c98; edeb is a later product-source descendant and has the above same-SHA XY-C1 pass.
- Wireless source-contract regression #45158308492 PASS on af80. XY-C1 #45156939135 PASS on product SHA `0e4976cfbe86c81f5172ce2256ed71801952682d` for Build/TRX/artifact; later product-source XY #45160010405 on edeb includes that wireless implementation.
- Exact XY-C1 #45160240051 on cbec was host-only: Build/TRX/artifact were skipped, so it is not L3 evidence. Do not reuse it.

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

### LAN DHCP networkmap refresh retry

- A failed initial BOUND or changed-address RENEW networkmap refresh is now recorded in PID 1's secure incomplete-action queue. The PID 1 notification handler retries the refresh and retains its marker on failure; subsequent BOUND/RENEW callbacks, including same-address renewals, requeue a pending marker. The lease commit remains independent of this ancillary refresh.
- Exact WAN DHCP callback #45160239807 PASS on cbec; exact XY-C1 #45160010405 on product-source SHA edeb PASS for Build, TRX verification and artifact upload (artifact ID 14811585463, 12,759,150 bytes).

### Firewall URL-filter schedule read

- IPv4 and IPv6 URL-filter schedules now use checked, presence-aware NVRAM reads and copy values before the next shared-cache lookup. Missing keys retain historical all-days/all-day defaults; present-empty or oversized values stay inactive; backend errors abort staged rule publication rather than silently falling back to defaults.
- Firewall Startup #45161387113 passed on checker descendant db60 and WAN DHCP callback #45161165802 passed on source ad562. Exact XY-C1 #45161165974 completed SUCCESS on exact product SHA ad562: Build XY-C1, TRX verification and artifact upload all passed. Artifact ID `14812541338`, name `XY-C1-firmware-ad56220d066e771be499426708b6c2f744937467`, size 12,759,350 bytes. L3 is VERIFIED for that firewall source slice; no hardware L4 claim is made.

### WAN gateway ARP source identity

- The ARP receive boundary validates exact six-octet syntax and rejects multicast, all-zero and broadcast Ethernet source addresses while accepting locally and globally administered unicast. The legacy `wan_hwaddr` formatter remains unchanged.
- Exact WAN ARP source regression #45159476012 passed on c98; subsequent product-source XY-C1 #45160010405 on descendant edeb passed Build/TRX/artifact, covering the parser in the product firmware baseline. Hardware gateway reachability remains unverified.

### Wireless checked radio-enable state

- 802.1X startup, guest activation and DBDC peer-band restart paths use checked radio-enable reads and fail closed before new authenticator/interface/bridge side effects; DBDC cleanup preserves the first read error.
- Exact Wireless action #45158308492 PASS on af80 after the static-contract whitespace fixes. The product-source baseline #0e4976 and descendants through edeb have same-source XY-C1 Build/TRX/artifact verification; physical radio and switch behavior remains unverified.



## Verification boundary

- Firewall URL-schedule L1/L2/L3: VERIFIED on source ad562 (Firewall Startup #45161387113 on db60 descendant; WAN DHCP #45161165802 and XY-C1 #45161165974 on ad562; Build/TRX/artifact PASS, artifact 14812541338). L4 hardware NOT VERIFIED.
- NAT helper selector hardening L1: SOURCE IMPLEMENTED on 3e5bcc5; one coherent NVRAM snapshot, strict empty/0/1 selectors and no-publication-on-invalid-input are reviewed. L2 WAN route #45178293988 and XY-C1 #45178294111 are IN_PROGRESS; Process #45178294031 and NVRAM #45178294032 are IN_PROGRESS; AI handoff #45178294028 PASS. L3 NOT VERIFIED pending same-SHA Build/TRX/artifact.
- L4 physical router: NOT VERIFIED; no physical firewall, NAT helper, NVRAM fault-injection or radio/WAN topology test was performed.

## Working method

- Each logical task is: inspect a concrete source defect, change production source, add or strengthen a regression contract/test, commit one complete tree, then inspect GitHub Actions for that exact SHA.
- Multi-file writes must create all blobs, one full Git tree and one commit. Update main once using force=false and expected_sha set to the current parent. Never expose a sequence of partially updated files.
- Fetch the live main SHA before writes. If another session moves it, rebase the intended changes onto that new main after inspecting intervening commits; never overwrite someone else's work.
- PASS means only the named workflow for its exact SHA passed. FAIL means inspect logs and fix the failing condition. PENDING means record the run and continue another independent safe task.
- Preserve GNU89, Werror, sanitizers where appropriate, Linux 4.4 / MIPS / uClibc compatibility, hardware isolation order, error normalization and rollback semantics.
- Update dev_log.md for each meaningful milestone. The root log had 103 records before this checkpoint and will have 104 after it. Do not rotate until a new entry would hit 120. Keep records newest first.
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
