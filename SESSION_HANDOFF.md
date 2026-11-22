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
- Parent of this DHCPv6 preflight slice: `2a92404d63b739c38453b4d15cfbbe8219ae8773`; the IPv4/DHCP snapshot and cleanup commits are already in main.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. Legacy PR #1 is closed/unmerged.
- WAN PPP launch mode uses a checked NVRAM transaction snapshot and immutable flags for lock selection, daemon stop/start, multi-WAN startup and rollback. Invalid WAN unit inputs return deterministic EINVAL.
- Modem WAN link-change reconnect uses checked modem readiness/WISP state; Inadyn startup checks DDNS enable state before stopping the existing daemon.
- DNSMasq resolver flags, DHCP integers, static-host snapshots, domain validation, resolver-file creation and lifecycle publication are covered by its workflow; this slice adds DHCPv6 status/snapshot preflight.

## Active task

The DHCPv6 DNSMasq path previously treated failed RA/mode reads as disabled/default state, used `nvram_safe_get("dhcp_dnsv6_x")`, and read stateful lifetime/interface-ID bounds with unchecked safe getters after staging began. This slice adds status-returning LAN6 APIs and a single checked NVRAM transaction snapshot for the DHCPv6 DNS address and stateful lifetime/range, validating all fields before staged config open. The rollback checker is also aligned with the completed IPv4 WINS snapshot contract.

## Verification boundary

- Local C/Python tests were not run in this connector-only session.
- On `7c6ab191bad7e18ec65c8bb8951713dbc24ee3f8`, AI handoff #45033513966 PASS, process inventory #45033514052 PASS and NVRAM transaction audit #45033514014 PASS. DNSMasq #45033514002 passed its GNU89/ASan/UBSan DHCP snapshot binary and source checker, then failed only because the separate rollback checker expected the now-removed standalone WINS read. This source contract is corrected here.
- XY-C1 #45033514004 was still executing Build XY-C1; TRX and artifact upload were pending. Do not claim L3 until all three pass on the same SHA.
- No router hardware validation or L4 result is claimed.
- Earlier PPP exact SHA `b3caebcbaba0924abcb83339d49a1968e237f042`: WAN DHCP #45032049974 PASS; VPN #45032050106 PASS; WAN route #45032049989 PASS; HTTPD #45032050007 PASS; process #45032050136 PASS; NVRAM #45032049962 PASS. XY-C1 #45032049960 started the production Build, but TRX/artifact completion was not confirmed.
- Later Inadyn descendant `c34e54b200272f17682cab1c98a2ad922ef43728`: DNSMasq #45032859372 PASS; process inventory #45032859409 PASS; miniupnpd #45032859331 PASS; Samba #45032859405 PASS; Inadyn #45032859299 PASS; WAN DHCP #45032859313 PASS; AI docs #45032859440 PASS. HTTPD #45032859349, XY-C1 #45032859323, WAN route #45032859347, process lifecycle #45032859298 and NVRAM #45032859363 were pending at the last recorded poll.
- No current target build/TRX/artifact or router hardware result is claimed. XY-C1 L3 requires Build, TRX verification and artifact upload all PASS on one identical SHA; hardware L4 requires device verification.

## Exact first action for the next GPT

1. Inspect exact-SHA DNSMasq, NVRAM, Process, AI and XY-C1 Actions for this DHCPv6 preflight commit; repair concrete failures.
2. Continue broader Padavan userland stability/security work outside WAN/PPP, USB modem, Inadyn and DNSMasq.
3. Keep the backup immutable. XY-C1 L3 requires Build, TRX validation and artifact upload all PASS on one SHA; L4 requires device verification.
