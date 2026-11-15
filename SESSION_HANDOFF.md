# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create every source/test/workflow/doc blob, one complete Git tree and one commit parented to freshly fetched main; update main with that exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Keep the frozen backup and archived log shards/index unchanged. Before context exhaustion, finish a safe slice and leave this file accurate.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session must record exact main/backup SHAs, source changes, exact-head Action IDs/statuses, validation boundary and the first next action before ending. Do not create a no-op commit when the checkpoint is already accurate.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Main at the start of this source slice: `97ffb22dd851f3ce3b303302df1a962bc40c7ade`.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only.
- Exact-main WAN DHCP callback #43749391596 PASS, AI handoff docs #43749391650 PASS and Process inventory #43749391694 PASS on `97ffb22dd851f3ce3b303302df1a962bc40c7ade`.
- Exact XY-C1 #43748599786 on product-source commit `5bbaaba493f63b0e58bac035e750907304fd9939` completed successfully: Build XY-C1, TRX verification and artifact upload all PASS. The later `ab9ae` and `97ff` commits only adjusted test contracts/docs; firmware product source remained identical to 5bbaa, so L3 is VERIFIED for that source tree. No physical-router/L4 claim is made.
- Dynamic IPTV client status propagation is committed at `5bbaaba493f63b0e58bac035e750907304fd9939`; exact regression contract now passes on `97ff`.
- Root `dev_log.md` contained 92 entries before this slice; append in-place, no rotation. Keep `docs/dev-log/INDEX.md` and archived shards unchanged.

## Active task

### Propagate physical WAN DHCP/ZeroConf launcher failures through WAN startup

The next stability slice is in `trunk/user/rc/src/net_wan.c`: `launch_wanx()` was `static void` and discarded the int result returned by `start_udhcpc_wan()` and `start_zcip_wan()`. Failures to execute these clients were therefore invisible to the WAN startup caller.

The current source slice changes `launch_wanx()` to return the selected dynamic client launch status. All four existing call sites in the USB-modem and PPP WAN branches now capture and log nonzero results. It deliberately does not abort PPPoE or alter the existing `wait_dhcpc` behavior: PPPoE intentionally starts without waiting for physical DHCP, while PPTP/L2TP retain their existing wait path. Static address setup and separately logged IPv6 setup remain unchanged.

The WAN callback workflow gains source-contract assertions for both return paths, preservation of the wait/sleep branch, all four call-site assignments and both caller-side status logs. Exact-head WAN/XY-C1 Actions for the resulting source commit remain required.

## Verification boundary

- This source commit is parented to `97ffb22dd851f3ce3b303302df1a962bc40c7ade`; verify its exact-head WAN callback, AI docs/Process workflows and XY-C1 Build/TRX/artifact before claiming new-source verification.
- The preceding IPTV source commit `5bbaaba493f63b0e58bac035e750907304fd9939` has exact XY-C1 #43748599786 PASS through Build, TRX verification and artifact upload. This does not cover the new `launch_wanx()` source change.
- No physical-router/L4 result is claimed.
- Frozen backup and archived documentation remain untouched.

## Exact first action for the next GPT

1. Fetch the resulting main SHA and exact-head Actions. Diagnose any WAN source-contract or firmware build failure; do not treat a pending run as a reason to wait.
2. Verify the XY-C1 run on the exact source SHA through Build, TRX verification and artifact upload.
3. Continue WAN launcher auditing from the next independent startup branch; do not repeat the already-passed IPTV and physical WAN launch propagation slices.
4. Preserve the literal checkpoint requirements above and update this handoff only when source SHA, exact Action status, verification boundary or first next action materially changes.
