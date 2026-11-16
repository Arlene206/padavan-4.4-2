# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state and compare it with `SESSION_HANDOFF.md` before ending or handing off. Verify the engineering/source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave the file untouched and make no no-op commit.

Before context exhaustion, finish or reduce the current safe slice, check exact-head Actions, commit any completed work atomically and preserve the precise first next action here.

Continue this engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; active development ref is main.
- Main at start of this slice: `c3b02529703add66cbd811b8dfa729edef50ad74`. Re-fetch live main before writes and update with a non-forced expected-SHA.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14: 72f4f157a084f8184b9597ca5bf334b8080e5807; read-only. Startup/handoff recovery is complete and must not be repeated.
- Root dev_log.md has 33 records after this entry; part-012.md holds the oldest 100 records.
- WAN reliability fixes cover required physical IFUP, IPv6, MAN routes, PPP/IPoE/USB modem startup, IPTV and PPPoE passthrough; static WAN/LAN overlap is EADDRINUSE.
- Current source fix rejects malformed static IPv4 address/netmask before ifconfig(), which otherwise maps invalid strings to INADDR_ANY and may return success.

## Active task

Validate static IPoE address/netmask using `dhcp_lease_ipv4_is_valid()` and `dhcp_lease_netmask_is_valid()` before interface configuration; invalid config aggregates EINVAL, subnet conflict aggregates EADDRINUSE. Add C policy and source-contract regressions. Also ensure PPP IP-up tests use the five-argument helper signature.

## Verification boundary

- Parent SHA: `c3b02529703add66cbd811b8dfa729edef50ad74`.
- WAN DHCP callback parent run `43870165716` FAILED compiling `ppp_wan_ipup_policy.c` because two test calls omitted the optional `remote_ip` argument after the helper signature extension. The current test content has been normalized so all calls use five arguments.
- Static IPoE malformed-input false-success was confirmed from shared `ifconfig()` parsing behavior; this commit adds validation before it can map bad text to `INADDR_ANY`.
- This change requires fresh exact-SHA Process, WAN DHCP and XY-C1 checks. No same-SHA L3 or physical-router L4 claim.


## Exact first action for the next GPT

1. Inspect exact-head Process/WAN source checker, WAN DHCP callback and XY-C1 Actions for the static IPv4 validation and PPP IP-up test arity repair.
2. Claim firmware L3 only after Build, TRX verification and artifact upload all pass on one exact source SHA.
3. Continue only on concrete additional false-success/stability defects. Do not repeat completed audits.
4. Keep the frozen backup read-only; physical-router L4 remains unverified.
