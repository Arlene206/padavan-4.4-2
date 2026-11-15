# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create every source/test/workflow/doc blob, one complete Git tree and one commit parented to freshly fetched main; update main with that exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Keep the frozen backup and archived log shards/index unchanged. Before context exhaustion, finish a safe slice and leave this file accurate.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session must record exact main/backup SHAs, source changes, exact-head Action IDs/statuses, validation boundary and the first next action before ending. Do not create a no-op commit when the checkpoint is already accurate.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Current main at start of test-contract correction: `41ea56088a7c8cb8bddd16b3d59ce168c0784c91`.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only.
- Production source `41ea56088a7c8cb8bddd16b3d59ce168c0784c91` adds separate required MAN gateway host/default route status propagation. Optional configured static/DHCP/DNS route failures remain in the aggregate status; static MAN startup returns only gateway/default failures to PPTP/L2TP startup gating. PPPoE async/no-wait and DHCP/ZeroConf callback aggregate semantics are retained.
- Exact 41ea WAN callback #43755418926 FAILED at a stale checker expression that searched for `route_setup_status = man_up_internal(wan_ifname, unit, 0)` after the callback was intentionally changed to pass `NULL` for the new optional status output. Its six preceding GNU89/host policy tests all PASS.
- Exact 41ea WAN route status #43755418941 FAILED at a stale `body()` signature still requesting the old three-argument `add_man_gateway_routes()`; the C function now accepts the additional status pointer. Compile/sanitizer, resolver, bridge ownership and VLAN/bridge propagation steps all PASS before the source contract.
- Exact 41ea AI handoff #43755418869, Process inventory #43755418919, Process lifecycle #43755418852, infosvr #43755418948, USB identity #43755418970, detect_link #43755418966, WAN teardown #43755418845, WAN ARP #43755418839, VPN #43755418832, AliDDNS #43755418826, AliyunDrive #43755418815, HTTPD #43755418812, detect_internet #43755418808, miniupnpd #43755418910 and DNSMasq #43755418936 all completed SUCCESS. NVRAM #43755418947 was still running at last poll.
- Exact XY-C1 #43755418990 was in progress during checkout/setup; Build, TRX and artifact steps had not yet started.
- Earlier product-source XY-C1 success on `524601f` does not validate later C source changes. No current-source L3 and no physical-router/L4 claim.
- Frozen backup, `docs/dev-log/INDEX.md` and archived shards remain untouched.

## Active task

### Align route-status workflows with the extended MAN helper API

The MAN route implementation is in place. Two exact-head workflow failures are test-only stale references:
- WAN callback workflow must expect `man_up_internal(..., 0, NULL)` for DHCP callbacks.
- WAN route workflow must use the extended helper signatures and preserve checks of aggregate return behavior, while asserting only gateway-host/default route failures feed the new required status; DNS routes remain outside that status.

No production-source change is required by these two failures.

## Verification boundary

- Test-only correction parent: `41ea56088a7c8cb8bddd16b3d59ce168c0784c91`; update `.github/workflows/wan-dhcp-callback-stability.yml` and `.github/workflows/wan-route-status.yml`, plus current checkpoint docs.
- Exact WAN #43755418926 failed at one stale callback substring; all preceding six host policy/compile tests passed.
- Exact WAN route #43755418941 failed at stale function-signature extraction (line 131 of workflow Python); compile/sanitizer, resolver refresh, bridge ownership and VLAN/bridge status steps passed.
- Other 41ea exact-head workflows listed above completed SUCCESS; NVRAM was running at last poll.
- Exact 41ea XY-C1 #43755418990 was still at checkout/setup; no Build/TRX/artifact result.
- Backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; no L3 for latest source, no L4 claim.

## Exact first action for the next GPT

1. Inspect exact-head WAN callback and WAN route workflow results for the checker-only updates. If they fail, inspect full logs and identify the precise remaining stale contract.
2. Confirm current-source XY-C1 Build, TRX verification and artifact upload; never reuse earlier SHA artifacts.
3. Ensure the critical route status covers gateway host/default routes and excludes optional DNS/user routes as currently specified; confirm the non-PPPoE PPTP/L2TP startup gate remains the only new fatal path.
4. Continue another independent stability/security audit outside WAN when CI is pending; do not wait idle.
5. Preserve backup and archive shards/index plus mandatory checkpoint wording.
