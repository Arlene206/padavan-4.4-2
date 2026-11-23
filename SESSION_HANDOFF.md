# SESSION HANDOFF — 2026-11-23

## Current repository state

- Repository: Arlene206/padavan-4.4-2.
- Main SHA before this checkpoint: 15c93fe802fc6a45148d04c8d3c7f238ad11fd92. The latest Wireless exact-head failure was #45155297922 on a456ced; this commit updates only the stale WDS source contract. Always fetch live main before the next write.
- Production source commit 9e9efbf6d9651e9bab82e544b02f718655a0a820 adds checked USB modem WAN identity reads before manual eject. Dedicated USB identity run #45154233466 PASS on that exact SHA. On 9e9efbf, Wireless #45154233785, general Process lifecycle inventory #45154233664, WAN route #45154233637, WAN DHCP #45154233589, AI docs #45154233499 and IPv6 type #45154233468 PASS. Process fail-closed #45154233474 and storage hotplug #45154233569 failed because source-contract scripts still expected the removed unchecked boolean identity call.
- Those stale Process assertions reproduced on later wireless-only source descendants: #45154469315 at 56bcddef and #45154946827 at eac5eeee (the latter was still running at last observation). Independent wireless commits advanced main to e15f67ae9b92b0327a5ad1ef13c9c0cee6382df8 and then this parent a456ced83e2829c17a8953ec0b41608e1cf0d058; they harden checked WDS selector/mode/config reads, not the USB eject call graph.
- On current parent a456ced, AI handoff #45155298252, HTTPD security #45155298151, Process fail-closed #45155298043, watchdog #45155298035, Process inventory #45155297990, iNIC #45155297987, XY-C1 #45155297930, Wireless #45155297922 and NVRAM #45155297969 were not yet complete at last poll (Process fail-closed and NVRAM queued/pending; XY still in progress).
- Frozen backup branch backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807. It is read-only and must not be updated, deleted or merged.
- Work only on main. No branches, temporary refs, force pushes or PRs. Historical PR #1 is closed and unmerged.
- No persistent local checkout was available. Do not claim local builds, firmware artifacts, sanitizer execution or hardware tests unless observed.
- Startup protocol and handoff have already been restored once. Do not repeat startup audits or completed WSD/DNSMasq/MTK investigations without new failure evidence.

## Active task

The next source change addresses error-collapsing radio-enable reads. The previous 802.1X helper read `radio_x` through `get_enabled_radio_*()` and could silently skip authenticator startup on NVRAM backend failure. Guest activation and DBDC peer-band restart made the same unknown-as-disabled assumption. This commit uses checked status reads and fail-closed ordering; exact-head Wireless and XY verification are pending.

The WDS checked-configuration change on a456ced is in production source and policy coverage, but its first exact-head Wireless run failed only because duplicated workflow assertions still expected the pre-resolver variables and immediate returns. This checkpoint removes the obsolete block and replaces it with assertions matching the current checked resolver/cleanup path. The exact-head Wireless rerun is pending.

The production USB modem eject fix is committed in `9e9efbf6d9651e9bab82e544b02f718655a0a820`. It adds `get_usb_modem_dev_wan_status()` backed by `nvram_get_checked()` and refuses modem teardown/reconnect on unknown identity while preserving the real errno. Dedicated USB identity run #45154233466 passed on that exact SHA.

Process inventory/lifecycle and USB storage hotplug workflows exposed stale source contracts, not production regressions: `scripts/check_process_lifecycle_integration.py:983` and `scripts/check_storage_safe_remove.py:413` still required the removed unchecked boolean `get_usb_modem_dev_wan()` gate. This checkpoint updates both contracts to verify the checked reader, shared parser, errno propagation, physical-port/selected-device gates and ordering before `safe_remove_usb_modem()`. It adds the policy header to Process workflow filters and netutils.c/.h plus the policy header to storage hotplug filters. The exact source checker failures are confirmed by the Actions logs; exact corrected reruns are still pending.

## Exact first action for the next GPT

1. Inspect the exact-head Wireless rerun triggered by this commit. Confirm GNU89 policy binaries and ASan/UBSan Ralink config tests pass, then verify the complete WDS checked-read/resolver source contract; if it fails, inspect the exact assertion and preserve the safety checks.
2. Check XY-C1 run #45155481405 on product source SHA 15c93fe: confirm Build XY-C1, TRX verification and artifact upload all succeed on that same SHA. Its result does not transfer to a later product-source commit.
3. Revisit any newly observed exact-head CI failures, then continue another independent P1 from REFACTOR_ROADMAP.md. Do not repeat completed USB modem or WSD audits without new evidence.

## Exact-SHA CI facts at the parent baseline
- Parent main SHA for this checkpoint: `15c93fe802fc6a45148d04c8d3c7f238ad11fd92`. It includes the checked USB modem identity source fix and updated Process/USB source contracts.
- On exact source SHA 15c93fe: Process fail-closed #45155481370 PASS; USB storage hotplug #45155481256 PASS; AI handoff docs #45155481272 PASS. The current commit is a workflow/docs contract-only update after that source tree.
- Wireless radio run #45155297922 on source SHA a456ced83e2829c17a8953ec0b41608e1cf0d058: all GNU89 policy binaries, Ralink config ASan/UBSan, bridge membership and guest-isolation tests PASS; final Python source contract FAIL because the WDS assertions still targeted the pre-resolver implementation. This commit corrects the stale contract.
- XY-C1 Firmware Validation #45155481405 on product source SHA 15c93fe was IN_PROGRESS at last observation; Build XY-C1, TRX verification and artifact upload are not yet confirmed. No L3 claim.
- On a456ced, Process fail-closed #45155298043 also failed at the old USB boolean identity assertion; the updated checker is present in 15c93fe and its exact 15c run #45155481370 PASS. Do not repeat this failure.
- Historical source results remain SHA-scoped: Switch CLI #45150824553 PASS on 3a7582ec; iNIC #45153133934 PASS on 63b15717; the older XY-C1 #45150223763 PASS Build/TRX/artifact on 45980023 only. None verify newer product-source commits.

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
