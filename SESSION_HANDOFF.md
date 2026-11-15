# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create every source/test/workflow/doc blob, one complete Git tree and one commit parented to freshly fetched main; update main with that exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Keep the frozen backup and archived log shards/index unchanged. Before context exhaustion, finish a safe slice and leave this file accurate.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session must record exact main/backup SHAs, source changes, exact-head Action IDs/statuses, validation boundary and the first next action before ending. Do not create a no-op commit when the checkpoint is already accurate.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Main at the start of this source slice: `bfc63087814434feac683b56fa9007d619508829`.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only.
- Direct IPoE DHCP launch status fix `8ee190c1872c4a3a9b6ad98b5da42fe1f2764e68`: captured `start_udhcpc_wan()` status, persisted `wan_err` on spawn failure and preserved independent IPv6 startup. Exact WAN callback #43750573557 failed only in its persistence-order assertion; regression-only correction `bfc63087814434feac683b56fa9007d619508829` changed the later persistence check to `.rindex()`.
- Exact `bfc` WAN callback #43751152794, AI docs #43751152792 and Process inventory #43751152773 were all IN_PROGRESS at last poll.
- Exact XY-C1 #43750573567 for source SHA 8ee was IN_PROGRESS at last poll; verify Build, TRX and artifact.
- Prior physical WAN launcher source `485d7c782c57b3fc0cb9cf4e42dcf0a79ba6665a` had WAN #43749781222 PASS, AI docs #43749781375 PASS and Process #43749781215 PASS.
- Frozen backup and archived logs/index remain untouched. No physical-router/L4 result is claimed.

## Active task

### Fail closed when required physical WAN setup fails

`launch_wanx()` returns dynamic DHCP/ZeroConf spawn status to its caller, but its static MAN branch still ignored `ifconfig()` status and called `man_up()` even if the address had not been applied. A static MAN address overlapping LAN also set `wanx_err` but returned success. In the non-PPPoE path, PPTP/L2TP could consequently launch PPP without the physical WAN setup their own comments say is required.

This slice checks static `ifconfig()` status, prevents route/physical setup publication when the interface address cannot be configured, returns `EINVAL` for the LAN-subnet conflict, and skips the two-second delay on a failed DHCP process launch. Non-PPPoE PPTP/L2TP now restore the physical WAN interface/firewall selection and skip PPP startup on nonzero physical setup status. PPPoE's asynchronous/no-wait behavior and the USB modem branch are left unchanged.

The WAN callback workflow protects the static address guard, conflict status, delay condition and non-PPPoE abort/restore ordering. Exact-head Actions remain required.

## Verification boundary

- This source slice is parented to `bfc63087814434feac683b56fa9007d619508829`; verify exact-head WAN callback, AI docs/Process and XY-C1 Build/TRX/artifact.
- Exact `bfc` WAN callback #43751152794, AI docs #43751152792 and Process #43751152773 were pending at handoff preparation. The earlier 8ee WAN run #43750573557 failed only at the assertion repaired in bfc; 8ee AI docs #43750573605 and Process #43750573434 PASS.
- Exact 8ee XY-C1 #43750573567 was IN_PROGRESS at last poll; do not claim L3 until Build, TRX and artifact pass.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; no physical-router/L4 result is claimed.

## Exact first action for the next GPT

1. Fetch resulting main and exact-head WAN/AI/Process Actions; diagnose any regression assertion or source failure.
2. Verify XY-C1 Build, TRX verification and artifact upload for the exact physical WAN source SHA; do not reuse older source builds.
3. Continue WAN startup auditing from route propagation/error rollback only after this physical interface guard is verified. Preserve PPPoE asynchronous behavior.
4. Keep the mandatory checkpoint phrase intact and never create a no-op handoff commit.
