## 2026-11-23 latest implementation checkpoint

- Parent before this cache-lifetime correction: `40ab459a16dfb914d571c8f559723e907a1b59e9`.
- APCLI WISP discovery now snapshots the SSID-present bit before the subsequent `sta_wisp` NVRAM read. The value returned by `nvram_get_checked()` is cache-owned; the pure helper consumes a stable boolean and validates 0/1.
- GNU89 tests cover read failure, absent SSID (WISP read ignored), present SSID with WISP read failure, valid WISP/non-WISP, invalid presence and output-pointer validation. The modem source checker asserts snapshot-before-WISP-read ordering.
- Exact-SHA tests and XY-C1 Build/TRX/artifact for this production source change are pending. Earlier APCLI policy GNU89 test passed on #45217541665. After binding WIRELESS_TEST, downstream USB modem #45217939671, Process #45217939549, Firewall #45217939795 and AI #45217940061 passed on checker SHA `842553492a7d1a3045c8b4fa4ea8fb1fde0e398a`; XY-C1 #45217939694 skipped product steps because it was checker/docs-only.
- No local MIPS/uClibc build or hardware result is claimed.

## Exact first action for the next GPT

Poll Actions for the new source SHA. Verify Wireless policy, USB modem checker and downstream Process/Firewall consumers; for XY-C1 require Build, TRX verification and artifact upload individually on the same SHA. Fix exact failures, then continue another independent stability/security slice. Use main-only expected-SHA commits; preserve frozen backup.

---

# SESSION HANDOFF — 2026-11-23

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; writable branch `main` only. Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; preserve it unchanged.
- Parent before THIS AP-client startup hardening slice: `ac8fc9f83bf0e32b492249e02a1ba5a6347a83f2`.
- AP-client WISP discovery source `7c16ec5c8738e3b8b698c75f5b01b1d9ab41aca7`; checker literal normalization `7a31ac98ebcf9af3762975606364c167687e6060`. On that checker-correction SHA, USB modem identity #45214590856, Process #45214590855, Firewall #45214591229 and AI docs #45214591002 passed. XY-C1 #45214590854 completed success as a job but skipped production Build/TRX/artifact because that commit changed only checker/docs.
- The WAN modem link-state source `a05ef61364ba4c24ec138e2911fd7469290b1648` passed USB modem #45211762454, Process #45211762366, Process fail-closed #45211762281, AI #45211762276, USB storage #45211762307, Firewall #45211762429 and NVRAM #45211762395. XY-C1 #45211762656 subsequently completed success with Build/TRX/artifact on that exact source SHA.
- Main before THIS commit was `ac8fc9f83bf0e32b492249e02a1ba5a6347a83f2`. No persistent local checkout or MIPS/uClibc toolchain is available; no local firmware build or hardware L4 result is claimed.

## Active task

- THIS COMMIT closes a remaining AP-client startup fail-open. Both `start_wifi_apcli_wl()` and `start_wifi_apcli_rt()` previously used lossy `nvram_wlan_get(..., "sta_ssid")` to decide whether to bring up APCLI and lossy `is_apcli_wisp_*()` to decide whether to attach that interface to the LAN bridge. A checked-NVRAM backend failure could be mistaken for empty SSID or non-WISP, potentially skipping APCLI startup or leaking a WAN-side WISP interface into LAN.
- Added `wireless_apcli_wisp_settings_status()` that checks SSID first and only reads the band-specific WISP flag for a nonempty SSID. A read error prevents APCLI startup, best-effort shuts the interface down, removes any LAN bridge membership, records errno and returns a retryable error.
- Added pure `wireless_apcli_start_config_status()` policy and GNU89 regression cases for SSID backend errors, missing/empty SSID, WISP backend errors, positive/negative WISP settings, invalid pointers and fail-closed output initialization. Extended the USB modem source checker and XY-C1 path filters.
- Exact-SHA results on THIS source SHA `3dad528f978904976210ce7c7dcbeda6e81b53fc`: Wireless radio state #45217541665 PASS; AI docs #45217543243 PASS; watchdog #45217542656 PASS; iNIC #45217542047 PASS; process inventory #45217541649 PASS. USB modem #45217541737 FAILS at the added checker assertion because `WIRELESS_TEST` was referenced without a module-level file read. Process #45217541804, Firewall #45217546022 and XY-C1 #45217542897 also fail at that same undefined checker variable after earlier work; XY-C1 Build/TRX/artifact were skipped. HTTPD #45217542023 PASS. THIS COMMIT adds the missing checker input binding; rerun the dependent checks.

## Exact first action for the next GPT

1. Poll Actions for THIS commit SHA, especially wireless-radio-state and XY-C1. Inspect Build, TRX and upload-artifact steps separately.
3. If a regression still fails after the checker input loads, inspect its exact job log and fix the source/test mismatch without weakening the fail-closed contract. If XY-C1 is pending, continue another independent stability or security source slice.
4. Keep atomic expected-SHA updates on `main` only; never force-push or alter the frozen backup.

## Exact-SHA CI facts

- WISP checker source `7a31ac98ebcf9af3762975606364c167687e6060`: USB modem identity #45214590856 PASS; Process #45214590855 PASS; Firewall #45214591229 PASS; AI #45214591002 PASS. XY-C1 #45214590854 was job success but product Build/TRX/artifact steps were skipped because no production firmware file changed.
- Main before the APCLI source commit was `ac8fc9f83bf0e32b492249e02a1ba5a6347a83f2`. Its first Actions pass: Wireless radio #45217541665 PASS, AI #45217543243 PASS, watchdog #45217542656 PASS, iNIC #45217542047 PASS, HTTPD #45217542023 PASS, inventory #45217541649 PASS. USB modem #45217541737, Process #45217541804, Firewall #45217546022 and XY-C1 #45217542897 all hit the same `NameError: WIRELESS_TEST is not defined`; XY-C1 product steps were skipped. Main before THIS checker fix is `3dad528f978904976210ce7c7dcbeda6e81b53fc`; use expected-SHA, no force push or backup mutation.



### SCUT watchdog checked-NVRAM and process-safety source slice

- Source SHA `ce39930eee413d71e0227c7d59e8e539b18e9f98`: SCUT watchdog regression #45181232164 first failed only because the workflow still expected the removed legacy `nvram get` source marker; after the checked-read source-order assertion was corrected, the shell PID/NVRAM/link-state regression passed in #45181232164 but the remaining PID cleanup assertion still expected a literal `# SESSION HANDOFF — 2026-11-23

## Current repository state

- Repository: Arlene206/padavan-4.4-2. Writable branch: `main` only. Parent of this handoff refresh: `ba4280cd91e149a6bde74c383cf4ab046f0ef313`.
- Frozen archive: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Preserve permanently; never update, switch to, merge, delete or rename.
- Branch inventory at this checkpoint: only `main` and the frozen archive. Legacy PR #1 is closed and unmerged; no open PR. Do not create branches or PRs.
- Current main baseline: `ba4280cd91e149a6bde74c383cf4ab046f0ef313`. Latest production-source change is `ce39930eee413d71e0227c7d59e8e539b18e9f98` (`fix(scutclient): reject unknown WAN link state in watchdog`); descendants `bcd5a1d` and `ba4280c` are source-contract/dev-log updates. Main contains the NAT helper, IPv6 NAT, SFE and SYN-cookie selector hardening described below.
- No persistent local checkout is available. No local compile, local sanitizer, MIPS/uClibc build or physical hardware result is claimed.
- Startup recovery has already been performed once. Do not repeat broad bootstrap or completed USB eject/MTK/WSD audits without new evidence.

## Active task

The active source slice is the SCUT watchdog hardening on `ce39930eee413d71e0227c7d59e8e539b18e9f98`:

- `scutclient_watchcat.sh` now uses `nvram get_checked` for watchdog enable, client enable, auth-flag and force-reauth selectors. Empty legacy defaults are preserved; backend errors and unsupported boolean values abort before client restart. Pre-06:00 empty force flag retains the legacy forced re-auth default.
- It now checks the exit status of `mtk_esw 11`; an ioctl error is not misread as "WAN link up". Confirmed WAN-down still skips health probes.
- Cleanup removes the PID file only when its contents equal this watchdog process's PID (`$$`), and retains a concurrent replacement marker. Test-harness NVRAM/log/barrier variables are exported.
- The POSIX lifecycle regression covers NVRAM failures at all four reads, invalid toggles, legacy force defaults, PID replacement, WAN ioctl failure and WAN-down behavior.

Exact SCUT regression #45181478770 and AI handoff contract #45181478768 both PASS on exact main descendant `ba4280cd91e149a6bde74c383cf4ab046f0ef313`. Process inventory #45181232027 and AI docs #45181232286 PASS on source `ce39930`. XY-C1 #45181232264 is still IN_PROGRESS on that exact production-source SHA (Build running; TRX and artifact pending), so do not claim L3 until all three steps pass on the same SHA.

Prior firewall URL-schedule source slice remains L3 VERIFIED: exact XY-C1 #45161165974 passed on source `ad56220d066e771be499426708b6c2f744937467`, Build, TRX and artifact upload included (artifact `14812541338`, 12,759,350 bytes). Do not repeat this closed task.

## Exact first action for the next GPT

1. Poll exact XY-C1 run #45181232264 on `ce39930eee413d71e0227c7d59e8e539b18e9f98`. Require Build XY-C1, TRX verification and artifact upload all PASS, then record artifact ID/name/size. It is the latest product-source descendant covering the NAT helper, IPv6 NAT, SFE (including lower-level mode validation), SYN-cookie and SCUT watchdog product sources.
2. SCUT PID/NVRAM/link-state host regression #45181478770 and AI handoff contract #45181478768 PASS on current main `ba4280c`; Process inventory #45181232027 and AI handoff #45181232286 PASS on `ce39930`. No hardware L4 claim. Do not re-run closed host workflows without new source changes.
3. Continue a new P1 stability/security source task from the roadmap outside the currently active WAN/ZeroTier/HTTPD work. The process lifecycle queue still has independent callers; identify a concrete discarded return/error before editing, and add a focused regression. While XY is pending, do not wait idle.

## Relevant exact-SHA CI facts

- Product-source `ad56220d066e771be499426708b6c2f744937467`: WAN DHCP callback #45161165802 PASS. Its first Firewall Startup run #45161165818 failed at a stale inline test-name assertion after earlier checked-NVRAM/lifecycle/startup/enable/static-route stages passed; corrected checker-only descendant db60 has Firewall Startup #45161387113 PASS and AI docs #45161387099 PASS. XY-C1 #45161165974 was IN_PROGRESS at Build XY-C1; TRX and artifact pending.
- LAN networkmap-retry source `edeb1e8354a97b312d8f3f5dd8787af030abfc6d`: WAN DHCP callback #45160239807 PASS on descendant cbec; XY-C1 #45160010405 PASS on exact edeb for Build, TRX verification and artifact upload. Artifact ID 14811585463, name `XY-C1-firmware-edeb1e8354a97b312d8f3f5dd8787af030abfc6d`, 12,759,150 bytes. This is L3 for product source through edeb, not for later product source ad562.
- WAN ARP source-MAC parser regression #45159476012 PASS on c98; edeb is a later product-source descendant and has the above same-SHA XY-C1 pass.
- Wireless source-contract regression #45158308492 PASS on af80. XY-C1 #45156939135 PASS on product SHA `0e4976cfbe86c81f5172ce2256ed71801952682d` for Build/TRX/artifact; later product-source XY #45160010405 on edeb includes that wireless implementation.
- Exact XY-C1 #45160240051 on cbec was host-only: Build/TRX/artifact were skipped, so it is not L3 evidence. Do not reuse it.

. Workflow assertion correction commit `ba4280cd91e149a6bde74c383cf4ab046f0ef313` changed it to `$`; exact SCUT #45181478770 then PASS, AI handoff #45181478768 PASS, Process #45181232027 PASS. XY-C1 #45181232264 on ce399 remained IN_PROGRESS as of this handoff. No claim of firmware artifact or L4 hardware until that exact run completes.

### NVRAM selector stability chain

- NAT helper selectors source `3e5bcc5b81fc4ebca8ab9a58d6f7d1495bdca2b4`: WAN route #45178293988, Process #45178294031, NVRAM boundary #45178294032 and AI docs #45178294028 all PASS; exact XY-C1 #45178294111 PASS, including Build/TRX/artifact.
- IPv6 NAT selector source `eeaefd869d1bfc8b0eea168e9674242cf8c6bd44`: WAN route #45178885851, Process #45178885839, AI docs #45178885949 and XY-C1 #45178886032 PASS (Build/TRX/artifact). SFE selector range and lower-level policy source `5348b38a00aafd4c64a2c19380801e92aec46a48` / `3a6cb1a370b3073e6a736b4e5405edc8813421bc`: NVRAM #45179216366 and Process #45179926371 PASS; XY-C1 #45179216075 PASS on 5348; exact descendant XY #45179926387 still pending at last poll, with later descendant #45181232264 used to close product-source verification.
- SYN-cookie strict selector source `9eadb3c1e07eebef480a366a882158ee8a3c8d9c`: WAN route #45179723265, Process #45179723106, AI docs #45179723263 and XY-C1 #45179723319 PASS. XY artifact ID `14821522264`, name `XY-C1-firmware-9eadb3c1e07eebef480a366a882158ee8a3c8d9c`, size 12,760,059 bytes. NVRAM boundary check #45179723165 was cancelled; the later descendant NVRAM audit #45179926278 PASS.

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
