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
- Exact source SHA `e100bb3b88a68e3db6e23721bf6852631126895e`: AI handoff #45034779639 PASS and wireless radio #45034779547 PASS. DNSMasq #45034779769 failed in `check_dnsmasq_integer_options.py` after its GNU89/ASan/UBSan policy binary passed: the new test-name assertion called undefined `read()` instead of the already loaded `TEST` text. IPv6 checked-read #45034779684 passed its GNU89/ASan/UBSan C policy binary, then its source checker failed because it still expected the old DHCPv6 mode getter to contain the NVRAM read; the production code has moved that read to the new status-bearing getter. Both checker failures are corrected in the follow-up commit.
- On the same `e100bb3...` source SHA, XY-C1 #45034779614 was in progress (Build/TRX/artifact not yet confirmed); NVRAM audit #45034779584 and Process inventory #45034779974 were also in progress at the last poll. No L3 result is claimed for this SHA.
- Prior SHA `7c6ab191bad7e18ec65c8bb8951713dbc24ee3f8` did complete XY-C1 #45033514004 successfully: its Build XY-C1, TRX output verification and artifact upload steps all PASS on that exact older SHA. That does not certify descendants.
- Earlier PPP and Inadyn runs below are historical evidence only; they do not certify the current source.
- No router hardware validation or L4 result is claimed.
- Earlier PPP exact SHA `b3caebcbaba0924abcb83339d49a1968e237f042`: WAN DHCP #45032049974 PASS; VPN #45032050106 PASS; WAN route #45032049989 PASS; HTTPD #45032050007 PASS; process #45032050136 PASS; NVRAM #45032049962 PASS. XY-C1 #45032049960 started the production Build, but TRX/artifact completion was not confirmed.
- Later Inadyn descendant `c34e54b200272f17682cab1c98a2ad922ef43728`: DNSMasq #45032859372 PASS; process inventory #45032859409 PASS; miniupnpd #45032859331 PASS; Samba #45032859405 PASS; Inadyn #45032859299 PASS; WAN DHCP #45032859313 PASS; AI docs #45032859440 PASS. HTTPD #45032859349, XY-C1 #45032859323, WAN route #45032859347, process lifecycle #45032859298 and NVRAM #45032859363 were pending at the last recorded poll.
- No current target build/TRX/artifact or router hardware result is claimed. XY-C1 L3 requires Build, TRX verification and artifact upload all PASS on one identical SHA; hardware L4 requires device verification.

## Exact first action for the next GPT

1. Fetch live `main` and inspect the exact-SHA DNSMasq and IPv6 checker reruns, then check XY-C1 Build/TRX/artifact, NVRAM and Process Actions for that same source SHA. Fix any concrete failure.
2. While independent jobs are pending, continue the broad userland stability/security rotation outside WAN/PPP, USB modem, Inadyn and DNSMasq.
3. Keep the backup immutable. XY-C1 L3 requires Build, TRX validation and artifact upload all PASS on one SHA; L4 requires physical device verification.
