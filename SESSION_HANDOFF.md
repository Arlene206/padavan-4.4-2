# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create every source/test/workflow/doc blob, one complete Git tree and one commit parented to freshly fetched main; update main with that exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Keep the frozen backup and archived log shards/index unchanged. Before context exhaustion, finish a safe slice and leave this file accurate.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session must record exact main/backup SHAs, source changes, exact-head Action IDs/statuses, validation boundary and the first next action before ending. Do not create a no-op commit when the checkpoint is already accurate.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Main at start of this source slice: `454f3869fc3c937f496d459ae52017804632378c`.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only.
- Product WAN source `524601f0e86e885f2858f1a5a7b571c09b8d1e1c` gates physical static MAN configuration, reports LAN overlap as `EINVAL`, avoids waiting after failed DHCP launch and aborts required non-PPPoE PPTP/L2TP startup if the physical path is unavailable; PPPoE async/no-wait is preserved.
- The previous exact-main 454f source-checker follow-up is validated: WAN #43752933794 PASS; AI docs #43752933786 PASS; Process lifecycle #43752933808 PASS; infosvr #43752933795 PASS; NVRAM #43752933792 PASS; AliDDNS #43752933810 PASS; AdGuardHome #43752933826 PASS; AliyunDrive #43752933849 PASS.
- Exact XY-C1 #43751765026 on product source `524601f`: Build XY-C1, TRX verification and artifact upload all PASS; L3 is verified for that product source. The current source change below requires a new XY-C1 run; do not reuse this prior result.
- A new concrete defect was confirmed in direct IPoE static IPv4 startup: `start_wan()` discarded `ifconfig()` status and still left `wan_err` clear, allowing `wan_up()` / UPnP activation when static address configuration failed. This source slice captures status, marks WAN error, attempts to clear a potentially partially applied address, logs cleanup failure, and relies on the existing `if (!wan_err)` gate to block route/UPnP activation. Independent IPv6 setup remains attempted.
- Frozen backup, `docs/dev-log/INDEX.md` and archived shards remain unchanged. No physical-router/L4 result is claimed.

## Active task

### Verify direct IPoE static address failure is fail-closed

The WAN static IPv4 branch now captures the result of `ifconfig(wan_ifname, IFUP, wan_addr, wan_mask)`. A nonzero result sets `wan_err`, logs the original error, attempts to clear a partially applied address and logs any cleanup error. The branch persists `wan_err`; the already-existing `if (!wan_err)` gate consequently prevents `wan_up()` and UPnP publication. The unrelated IPv6 setup path is preserved.

The workflow now asserts this exact status/cleanup/error-state flow and route/UPnP gating, including the continuing IPv6 call. The next GPT must inspect exact-head WAN/AI/Process/XY-C1 results, then continue only after these pass.

## Verification boundary

- Source baseline parent: `454f3869fc3c937f496d459ae52017804632378c`; implementation change is in `trunk/user/rc/src/net_wan.c`; regression source contract is in `.github/workflows/wan-dhcp-callback-stability.yml`.
- Previous exact-head baseline 454f: WAN #43752933794 PASS; AI docs #43752933786 PASS; Process #43752933808 PASS; infosvr #43752933795 PASS; NVRAM #43752933792 PASS; AliDDNS #43752933810 PASS; AdGuardHome #43752933826 PASS; AliyunDrive #43752933849 PASS.
- Earlier product source `524601f` XY-C1 #43751765026 Build/TRX/artifact PASS. The new static IPoE C change invalidates reuse of that build for current source; current-source L3 is pending new exact-SHA Build/TRX/artifact.
- New exact-head WAN and AI docs results pending at handoff preparation. No physical-router/L4 result is claimed.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## Exact first action for the next GPT

1. Fetch main and inspect exact-head WAN callback / AI docs / Process / XY-C1 Actions for this static-address fix. Inspect full failing logs before modifying tests/source.
2. Require GNU89 host policy steps, the new static IPoE status/cleanup/gating contracts and current-source XY-C1 Build, TRX verification and artifact upload to pass before claiming L3.
3. Continue WAN startup review by determining whether static MAN route errors from `man_up_internal()` should flow to `launch_wanx()` without making optional route failures incorrectly fatal. Separately review the now-captured direct IPoE static-address error path and its cleanup semantics.
4. Preserve independent IPv6 behavior, PPPoE async/no-wait, frozen backup and archived log index/shards; keep the mandatory checkpoint sentence intact.
