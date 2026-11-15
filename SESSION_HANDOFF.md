# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create every source/test/workflow/doc blob, one complete Git tree and one commit parented to freshly fetched main; update main with that exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Keep the frozen backup and archived log shards/index unchanged. Before context exhaustion, finish a safe slice and leave this file accurate.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session must record exact main/backup SHAs, source changes, exact-head Action IDs/statuses, validation boundary and the first next action before ending. Do not create a no-op commit when the checkpoint is already accurate.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Main at start of MAN required-route status slice: `2529f9a8aadd378347fa47578a1f048a793b0c1c`.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only.
- Product source `20f1bbd6eb138ec87b451cd2d7f424337db798bb` captures direct IPoE static IPv4 `ifconfig()` status, sets `wan_err`, attempts partial-address cleanup and blocks route/UPnP publication on failure; IPv6 remains independent.
- Product source `7ebeaa9e02e6249ac787497429defab2f726e5a0` replaces unchecked shell initial WAN bring-up with checked MTU and interface calls; failed interface bring-up attempts to keep it down and skips protocol startup.
- Product source `2529f9a8aadd378347fa47578a1f048a793b0c1c` checks AP-client WISP migration cleanup; if old MAN address removal fails, it attempts interface shutdown and refuses to publish WISP if shutdown cannot be ensured.
- Exact 2529 Actions: WAN callback #43754701672 PASS; AI docs #43754701543 PASS; Process inventory #43754701614 PASS; NVRAM ioctl audit #43754701523 PASS. XY-C1 #43754701433 was at Build XY-C1 at last poll; TRX/artifact remained pending. Other independent workflows were in progress or queued; do not claim them as PASS.
- New issue addressed in this slice: aggregate `man_up_internal()` status included required MAN gateway/default routes as well as optional user static/DHCP/DNS routes. Static MAN startup discarded the aggregate through the void `man_up()` wrapper, so PPTP/L2TP could not distinguish critical route failure from optional route failure.
- The current change exposes a separate required-route status for only the gateway host route (when off-subnet) and default route. The aggregate result remains for diagnostics and DHCP/ZeroConf callbacks; optional DNS/user routes do not enter the required status. Static MAN `launch_wanx()` persists `wanx_err` and returns the critical failure so the existing non-PPPoE PPTP/L2TP guard aborts before pppd. PPPoE asynchronous/no-wait remains unchanged.
- Frozen backup, `docs/dev-log/INDEX.md` and archived shards remain unchanged. No physical-router/L4 result is claimed.

## Active task

### Verify required MAN gateway/default-route error propagation

`add_man_gateway_routes()` now reports a separate status for the off-subnet gateway host route and the default route. Optional DNS host routes remain outside this status. `man_up_internal()` keeps its aggregate return for existing callback diagnostics while exposing the separate status to its caller.

Static MAN `launch_wanx()` marks `wanx_err` and returns the critical gateway/default-route error when it occurs. The existing non-PPPoE PPTP/L2TP startup gate then prevents pppd launch. Optional configured static/DHCP/DNS route failures remain nonfatal to the PPP launch decision; PPPoE async/no-wait behavior remains unchanged.

The workflow contains source assertions covering the critical-vs-optional split, unchanged DHCP/ZeroConf aggregate behavior, and static MAN propagation.

## Verification boundary

- Parent main: `2529f9a8aadd378347fa47578a1f048a793b0c1c`; implementation in `trunk/user/rc/src/net_wan.c`; regression contracts in `.github/workflows/wan-dhcp-callback-stability.yml`.
- Exact parent 2529 WAN callback #43754701672 PASS; AI docs #43754701543 PASS; Process inventory #43754701614 PASS; NVRAM ioctl audit #43754701523 PASS.
- Exact parent XY-C1 #43754701433 was at Build XY-C1; TRX verification and artifact upload were pending. Previous successful XY-C1 SHA `524601f` does not validate later C changes.
- Exact-head Actions for this route-status commit are pending at handoff preparation. No current-source L3 claim until latest SHA Build, TRX and artifact upload all pass; no L4 claim.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## Exact first action for the next GPT

1. Fetch main and inspect exact-head WAN callback, AI docs, Process inventory, WAN route status, NVRAM and XY-C1 results for the MAN required-route status separation. If WAN fails, inspect the full job log and fix the specific source or test issue.
2. Confirm GNU89/source-contract PASS and current-source XY-C1 Build/TRX/artifact PASS before claiming L3.
3. Verify optional static/DHCP/DNS route failures do not block PPP while required gateway/default-route failures do; preserve PPPoE async/no-wait.
4. Continue remaining WAN startup audit only after the route-status source contracts pass; preserve independent IPv6 and AP-client compatibility.
5. Preserve frozen backup and archived log index/shards and the mandatory checkpoint phrase.
