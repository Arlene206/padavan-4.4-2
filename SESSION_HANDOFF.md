# SESSION HANDOFF — 2026-11-23

## Current repository state

- Repository: Arlene206/padavan-4.4-2.
- Main parent for this checkpoint: afd3e53a9bdc1d1ef72040f823100de465893585. Always fetch live main before editing; parallel sessions can advance it while a task is running.
- RT3352 iNIC health-monitor source hardening is in ae45305fec4c7a012c00e89756a80c184589c472. Exact-head iNIC signal #45152702943 and Wireless radio #45152702887 passed on f0a01f41ed853c98eea4da1ac3f40a77ec42f10c. On descendant 430392cc1ad139abb21d841a040ae4dea9a6f9c9, Wireless radio #45153320688 and AI handoff #45153320557 passed; XY-C1 #45153320689 passed Build/TRX/artifact on that same SHA. Current parent afd3e53a9bdc1d1ef72040f823100de465893585 has iNIC #45153638293, Wireless #45153638473, Process inventory/lifecycle #45153638327, HTTPD security #45153638410 and AI handoff #45153638292 passing; NVRAM #45153638348 and XY-C1 #45153638513 were IN_PROGRESS at last poll.
- Frozen backup branch backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807. It is read-only and must not be updated, deleted or merged.
- Work only on main. No branches, temporary checkpoint refs, force pushes or PRs. Historical PR #1 is closed and unmerged; authorized content has already been ported.
- No persistent local checkout was available. Do not claim local firmware builds, sanitizer execution, artifacts or physical-device tests unless actually observed in a later session.
- Startup protocol and handoff have already been restored once for this takeover. Do not repeat the initial repository audit or revisit completed WSD, DNSMasq, WAN and MTK investigations without new failure evidence.

## Active task

The current source defect is in manual USB modem removal, `safe_remove_usb_device()` in `trunk/user/rc/src/services_usb.c`. It called `get_usb_modem_dev_wan()` in `trunk/user/shared/src/netutils.c`; that legacy helper used `nvram_get_int("wan0_modem_dev")`, so an NVRAM open/ioctl/close error collapsed to zero. The caller then treated the selected modem as absent and returned `ENODEV`, hiding the real backend error. The operation already failed closed, but it prevented a legitimate eject from being retried based on the actual NVRAM failure and lost error attribution.

This checkpoint adds `get_usb_modem_dev_wan_status()` backed by `nvram_get_checked()` and the existing compatibility parser. `safe_remove_usb_device()` now distinguishes no physical modem, a positive NVRAM read error, a known non-matching modem and an exact selected-modem match. The status error is returned before `safe_remove_usb_modem()` or WAN reconnect can run. The old boolean wrapper now delegates to the status API and no longer uses the unchecked getter.

The existing USB modem workflow compiles `tests/usb_modem_wan_policy.c` with GNU89, `-Wall -Wextra -Werror`, ASan and UBSan. This checkpoint adds exact-match, mismatch, absent-key, backend-error and invalid-output cases, and extends the inline source contract to pin the checked reader, compatibility wrapper and pre-side-effect eject ordering. The workflow also covers WDM/NDIS discovery, hotplug metadata, modem reload and deferred events.

## Exact first action for the next GPT

1. Fetch the live main SHA and inspect this checkpoint commit before editing; do not assume it is still the parent recorded above.
2. Inspect the exact-SHA USB modem identity run and storage-hotplug run. Confirm the new devnum policy executable and source contract pass, along with existing modem module, modem start-config, metadata and checked hotplug-event regressions. Fix a real failure rather than weakening the contract.
3. Inspect the exact-head XY-C1 Firmware Validation for Build XY-C1, TRX verification and artifact upload on the same production source SHA. If those steps are pending, move to another independent P1 instead of waiting.
4. Continue an independent P1 from REFACTOR_ROADMAP.md. Avoid duplicate USB eject audits, completed WSD/DNSMasq/MTK slices, and any item already completed by a parallel session. Before changing another module, confirm an evidenced defect and its call graph.
5. For each source change, add a focused regression and workflow path filters, then one complete tree/commit on main; inspect exact-SHA Actions and update this handoff only when the active source baseline or next action changes.

## Exact-SHA CI facts at the parent baseline

- Parent main SHA for this checkpoint: `afd3e53a9bdc1d1ef72040f823100de465893585`, titled `fix(wireless): verify radio mode before bridge changes`.
- On parent SHA afd3e53a9bdc1d1ef72040f823100de465893585, iNIC signal #45153638293, Wireless radio #45153638473, Process inventory/lifecycle #45153638327, HTTPD Authentication/Security #45153638410 and AI handoff #45153638292 passed. NVRAM transaction audit #45153638348 and XY-C1 #45153638513 were IN_PROGRESS at the last poll.
- On descendant SHA 430392cc1ad139abb21d841a040ae4dea9a6f9c9, XY-C1 run #45153320689 completed SUCCESS. Its Build, TRX verification and artifact upload steps passed on that exact product-source SHA; this does not establish L3 for later production changes.
- The new USB modem policy and source-contract results, storage-hotplug result and XY-C1 Build/TRX/artifact result for this checkpoint are pending until exact-SHA Actions are observed.

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

- L1 source review: PASS for the USB selected-modem eject gate and checked NVRAM reader. The eject call graph returns an unknown-identity errno before modem stop/reconnect side effects.
- L2 host/Actions for this USB slice: PENDING exact-head USB modem identity and storage-hotplug regressions, including GNU89/Werror/ASan/UBSan policy binaries and the inline source contract. No local sanitizer result is claimed.
- L3 XY-C1 firmware: NOT VERIFIED for this source SHA until Build XY-C1, TRX verification and artifact upload all pass on the exact product-source SHA. Earlier PASS on 430392cc does not transfer to later commits.
- L4 router hardware: NOT VERIFIED, including actual multi-modem selection, physical eject/reconnect behavior, WDM/NDIS/RAS interoperability and hub edge cases.
- Do not extrapolate XY-C1 to other boards or RT3352 hardware.

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
