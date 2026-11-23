# SESSION HANDOFF — 2026-11-23

## Current repository state

- Repository: Arlene206/padavan-4.4-2. Writable branch: `main` only. Live parent before this documentation checkpoint: `db60eb4deb95f7702c63f50563b8cc9b23d350e9`.
- Frozen archive: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Preserve permanently; never update, switch to, merge, delete or rename.
- Branch inventory at this checkpoint: only `main` and the frozen archive. Legacy PR #1 is closed and unmerged; no open PR. Do not create branches or PRs.
- Current product-source commit: `ad56220d066e771be499426708b6c2f744937467`, `fix(firewall): fail closed on URL schedule read errors`. Documentation/checker-only descendant: `db60eb4deb95f7702c63f50563b8cc9b23d350e9`.
- No persistent local checkout is available. No local compile, local sanitizer, MIPS/uClibc build or physical hardware result is claimed.
- Startup recovery has already been performed once. Do not repeat broad bootstrap or completed USB eject/MTK/WSD audits without new evidence.

## Active task

Close the production-source L3 gate for the firewall URL-filter schedule fix. The source change replaces unchecked `nvram_get()` fallbacks with checked presence-aware reads, copies date/time values before the next NVRAM access invalidates the shared pointer, and propagates backend errors. Both IPv4 and IPv6 builders discard staged rules and abort core restore before publication on a schedule read failure. Missing-key defaults remain compatible; present-empty and oversized schedules remain inactive.

Source commit `ad56220d066e771be499426708b6c2f744937467` also adds a policy helper and direct unit coverage. The first Firewall Startup run #45161165818 failed only because its inline checker named a nonexistent test function; checker correction is `db60eb4deb95f7702c63f50563b8cc9b23d350e9`. Exact Firewall Startup #45161387113 PASS and AI docs #45161387099 PASS on db60. Exact WAN DHCP callback #45161165802 PASS on ad562.

## Exact first action for the next GPT

1. Poll XY-C1 run #45161165974 on exact product-source SHA `ad56220d066e771be499426708b6c2f744937467`. At last poll, Build XY-C1 was IN_PROGRESS and TRX verification/artifact upload were pending. Confirm all three pass on that SHA; if Build fails, inspect its job log and fix the actual compile failure before claiming L3.
2. If #45161165974 passes, record its artifact ID/name/size and mark L3 for the firewall URL-schedule slice. If it fails, keep L3 NOT VERIFIED and repair the source/build issue.
3. Continue the next unfinished P1 stability/security slice from `REFACTOR_ROADMAP.md`; prefer work outside the already heavily exercised Wireless/HTTPD/DHCP paths. Do not repeat completed tasks without new failure evidence.

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
- GNU89/Werror/ASan/UBSan policy tests and the complete firewall source-order contract passed in Firewall Startup #45161387113 on db60 (production code from ad562). WAN DHCP callback #45161165802 PASS on ad562. Exact XY-C1 #45161165974 on ad562 was still building at last poll; L3 for this firewall source slice remains pending.

### WAN gateway ARP source identity

- The ARP receive boundary validates exact six-octet syntax and rejects multicast, all-zero and broadcast Ethernet source addresses while accepting locally and globally administered unicast. The legacy `wan_hwaddr` formatter remains unchanged.
- Exact WAN ARP source regression #45159476012 passed on c98; subsequent product-source XY-C1 #45160010405 on descendant edeb passed Build/TRX/artifact, covering the parser in the product firmware baseline. Hardware gateway reachability remains unverified.

### Wireless checked radio-enable state

- 802.1X startup, guest activation and DBDC peer-band restart paths use checked radio-enable reads and fail closed before new authenticator/interface/bridge side effects; DBDC cleanup preserves the first read error.
- Exact Wireless action #45158308492 PASS on af80 after the static-contract whitespace fixes. The product-source baseline #0e4976 and descendants through edeb have same-source XY-C1 Build/TRX/artifact verification; physical radio and switch behavior remains unverified.



## Verification boundary

- L1 source review: PASS for checked URL schedule reads, presence-aware legacy defaults, pointer lifetime/copy ordering and abort-before-publication on both IPv4 and IPv6.
- L2 host/Actions: Firewall Startup #45161387113 PASS on db60 (production source parent ad562), including GNU89/Werror/ASan/UBSan policy helper tests and full URL-schedule source-order contract. WAN DHCP callback #45161165802 PASS on ad562.
- L3 XY-C1 firmware: PASS through prior product SHA edeb (Build, TRX and artifact). NOT VERIFIED for current production source ad562 until exact run #45161165974 completes Build, TRX verification and artifact upload on the same SHA.
- L4 physical router: NOT VERIFIED; no physical firewall schedule/NVRAM fault injection or radio/WAN topology test was performed.

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
