# Current Session Handoff — Padavan 4.4

> One actionable checkpoint only. Historical handoffs and roadmap snapshots are preserved verbatim in their archive indexes. Always fetch live \`main\` before relying on any SHA below.

## Current repository state

- **Repository:** \`Arlene206/padavan-4.4-2\` — this fork, not upstream.
- **Branch:** \`main\`; only remote branch observed during recovery. No separate checkpoint branch exists.
- **Base main SHA for this round:** \`347ab8b0392a3c74d85a230e199c2faae27f646c\`.
- **Resulting main SHA:** THIS COMMIT; fetch live \`main\` on the next recovery.
- **Prior governance CI:** [AI handoff documentation contract run 42550725545](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42550725545) completed SUCCESS on exact head \`347ab8b0392a3c74d85a230e199c2faae27f646c\`.
- **Last prior code SHA:** \`8a02953fb60cced248254df86feca85e8bc42d10\`; WAN teardown run 42548284186 and HTTPD/service run 42548284184 both passed on that exact code SHA.

## Last completed bounded task

- **Module:** WAN/PPP route management; this is a bounded fix, not WAN closure.
- **Finding:** static route installation/removal ignored each route operation's errno; DHCP route helper returned void and ignored legacy plus classless route errors and \`strdup()\` failures.
- **Fix:** both helpers aggregate the first nonzero error and keep processing later routes. DHCP public wrappers now return status; failed route operations and list allocation failures are logged. An allocation failure in one list does not prevent attempting the other. Route-delete behavior when dynamic-route addition is disabled is unchanged.
- **Changed paths:** \`trunk/user/rc/src/net.c\`, \`trunk/user/rc/src/net_wan.c\`, \`trunk/user/rc/src/rc.h\`, new \`route_status_policy.h\`, new \`tests/route_status_policy.c\`, new \`.github/workflows/wan-route-status.yml\`.
- **Regression:** local GNU89 \`-Wall -Wextra -Werror\` helper test with ASan/UBSan and leak detection PASS; workflow adds the helper regression and source/caller contracts.
- **Exact-head route Actions:** PENDING at commit creation; next GPT must inspect run ID, conclusion and exact \`head_sha\`.

## Active task — resume after exact-head CI reconciliation

**Next module: Firewall/NAT stability and lifecycle ownership (P1/P2).** Rotate away from WAN and recently patched Wireless/HTTPD paths. Start by re-fetching current main and Actions. Then trace the unresolved FULLCONENAT notifier/reference-count ownership against per-network-namespace registration and module unload, including all success/failure/unwind paths; compare with the Linux 4.4 APIs actually integrated in this tree. Prove a concrete namespace, cleanup or resource-lifecycle defect before editing. Keep independent IPv4/IPv6 ruleset writers and rollback ordering in the same firewall ledger, but implement one bounded fix only.

## Other open whole-repository work

- **Wireless — AUDITING:** \`check_inic_mii_rebooted()\` guest-interface-up result and \`stop_wifi_all_wl/rt()\` interface-down/LED publication remain open.
- **WAN / PPP — PARTIAL / AUDITING:** route result aggregation now has bounded evidence; PPP callbacks, WAN lifecycle and runtime route behavior remain broader open scope.
- **DNS / DHCP — PARTIAL / AUDITING:** resolver snapshots, parser/rebinding and live recovery remain open.
- **Firewall / NAT — AUDITING:** FULLCONENAT global notifier/reference-count versus per-net ownership and independent IPv4/IPv6 writer/rollback parity remain unresolved.
- **Switch / PHY, USB/storage/hotplug, NVRAM/configuration, init/service lifecycle, HTTPD/session/CSRF/ACL, FTP/Samba/NFS and shared/third-party userland:** remain open in the roadmap; a bounded fix does not close a module.

## Verification boundary

| Level | Status |
|---|---|
| L1 source/static review | PASS for the selected static/DHCP route helpers, wrappers and cleanup ordering |
| L2 host regression | PASS for the focused GNU89 aggregation policy regression; exact-head Actions PENDING |
| L3 MIPS/uClibc target build and complete firmware artifact | **NOT VERIFIED** |
| L4 router flash/boot/hardware behavior | **NOT VERIFIED** |

No production RC object, MIPS build, firmware/TRX, live route failure injection or physical-router test is claimed.

## Exact first action for the next GPT

Fetch live \`main\`; inspect the \`WAN route operation status regression\` Actions run and confirm its \`head_sha\` equals the live main SHA. Read this handoff, roadmap, newest 20 log entries and archive index. If the exact-head workflow passes, start the bounded FULLCONENAT namespace/notifier ownership audit described above; do not repeat the route aggregation change or start a new WAN subtask first.

## Recovery rule

After the next milestone, replace this active checkpoint and move this exact version verbatim into \`docs/handoffs/archive/YYYY/MM/\` in the same commit.