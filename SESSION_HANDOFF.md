# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository and complete an end-of-session accuracy checkpoint before ending. Record current source SHA, exact-SHA Actions, test/build boundaries, and the next precise task. If facts and first action remain accurate, explicitly conclude the check with no file change and no commit. Use one atomic main commit for material changes only; never create a no-op commit. Before context exhaustion, leave the next GPT a precise first action and truthful verification status.

- Work only on `main`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update `main` only with `force:false` and a fetched expected SHA.
- Check Actions for the exact source SHA. PASS continues; FAIL requires diagnosis/correction; PENDING does not block independent work.
- Do not claim local tests, target builds, artifacts or hardware checks that were not performed.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; branch `main`.
- Parent of the DNSMasq LAN/DHCP snapshot slice: `7f6ccc6246fd9267c35195e3b2187a7a473fcd05`; source commit `22f626cc2ac7f207edd6c554fda6a1d9ab563838`; current error-cleanup correction is THIS COMMIT.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. Legacy PR #1 is closed/unmerged.
- WAN PPP launch mode uses a checked NVRAM transaction snapshot and immutable flags for lock selection, daemon stop/start, multi-WAN startup and rollback. Invalid WAN unit inputs return deterministic EINVAL.
- Modem WAN link-change reconnect uses checked modem readiness/WISP state; Inadyn startup checks DDNS enable state before stopping the existing daemon.
- DNSMasq resolver flags, DHCP integers, static-host snapshots, domain validation, resolver-file creation and lifecycle publication are covered by the existing DNSMasq workflow.

## Active task

The IPv4/DHCP DNSMasq input path now uses one bounded checked NVRAM transaction snapshot for LAN IP/netmask/domain and, when DHCP is enabled, pool endpoints, gateway, DNS1–DNS3 and WINS. The snapshot copies rotating cache-backed values, preserves empty defaults for missing keys, and zeroes the complete struct on begin/read/end/truncation errors before generated files are changed. The APP_VLMCSD mode is also read through a checked integer helper before staging. Regression tests are in `trunk/user/rc/src/tests/dnsmasq_nvram_option_policy.c`; source contract in `scripts/check_dnsmasq_integer_options.py`; CI in `.github/workflows/dnsmasq-lifecycle.yml`.

## Verification boundary

- Local C/Python tests were not run in this connector-only session.
- DNSMasq snapshot commit `22f626cc2ac7f207edd6c554fda6a1d9ab563838`: DNSMasq lifecycle #45033357912 failed only in the new GNU89/ASan/UBSan regression because the high-level snapshot wrapper had not fully erased previously copied struct bytes on failure. That cleanup is fixed by this commit and will be re-run on the new SHA. AI handoff #45033357885 PASS; XY-C1 #45033357945 was building at last poll; WAN route #45033357876, NVRAM #45033357988, process #45033358009, miniupnpd #45033358084, Inadyn #45033358081, WAN DHCP #45033357892, HTTPD #45033357948 and Samba #45033357873 were still pending.
- Earlier PPP exact SHA `b3caebcbaba0924abcb83339d49a1968e237f042`: WAN DHCP #45032049974 PASS; VPN #45032050106 PASS; WAN route #45032049989 PASS; HTTPD #45032050007 PASS; process #45032050136 PASS; NVRAM #45032049962 PASS. XY-C1 #45032049960 started the production Build, but TRX/artifact completion was not confirmed.
- Later Inadyn descendant `c34e54b200272f17682cab1c98a2ad922ef43728`: DNSMasq #45032859372 PASS; process inventory #45032859409 PASS; miniupnpd #45032859331 PASS; Samba #45032859405 PASS; Inadyn #45032859299 PASS; WAN DHCP #45032859313 PASS; AI docs #45032859440 PASS. HTTPD #45032859349, XY-C1 #45032859323, WAN route #45032859347, process lifecycle #45032859298 and NVRAM #45032859363 were pending at the last recorded poll.
- No current target build/TRX/artifact or router hardware result is claimed. XY-C1 L3 requires Build, TRX verification and artifact upload all PASS on one identical SHA; hardware L4 requires device verification.

## Exact first action for the next GPT

1. Fetch live `main` and inspect exact-head DNSMasq lifecycle, AI handoff and XY-C1 Actions for this snapshot commit; fix concrete failures.
2. Continue the separate DHCPv6 preflight slice: status-check DHCPv6 DNS address, stateful range lifetime/port bounds, DHCPv6 server mode and information-refresh timer before opening the staged DNSMasq config. Preserve missing-key defaults and leave the completed IPv4 snapshot intact.
3. Check outstanding Inadyn-descendant WAN route/HTTPD/process/NVRAM/XY-C1 jobs only on their exact SHA, then continue independent stability/security work.
4. Keep the backup immutable and do not claim XY-C1 L3/L4 without same-SHA Build/TRX/artifact and hardware evidence.
