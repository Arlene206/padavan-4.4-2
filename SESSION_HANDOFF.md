# Session Handoff — 2026-11-26

## Repository state

- Repository: `Arlene206/padavan-4.4-2`; continue on `main` only.
- Frozen `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; never modify/delete. PR #1 remains closed/unmerged.
- Parent main for this firewall slice: `8431202d8ce320b6df72b87d183749c44bea7f50`. Each multi-file change is one complete Git tree and one atomic commit; update `main` with non-forced expected-SHA CAS only.

## Current source slice: firewall 6in4 endpoint NVRAM

- In `trunk/user/rc/src/firewall_ex.c`, `ipt_filter_rules()` formerly fetched `ip6_6in4_remote` with `nvram_safe_get()` while emitting the IPv4 filter's ICMP echo exception for the 6in4 peer. Backend/close errors collapsed to an empty optional value, silently omitting the configured exception.
- The change snapshots the optional address with `firewall_read_optional_ipv4_setting()` before opening the IPv4 filter ruleset file. Missing/oversized values preserve legacy no-exception behavior; NVRAM read/close/API-result errors propagate and abort the enclosing firewall restore. The emitter uses only the local 16-byte snapshot.
- Contract: `scripts/check_firewall_ip_nvram_status.py` enforces pre-stage ordering, error gate, local snapshot use, no raw getter, and existing optional IPv4 copy regressions. `trunk/user/rc/src/tests/firewall_nvram_match_policy.c` already exercises present/missing, backend/invalid-result and oversized-value cases in the Firewall startup workflow; no workflow change is required.

## Recent exact-SHA validation facts

- SmartDNS UI-key correction SHA `8431202d8ce320b6df72b87d183749c44bea7f50` fixes the prior `bfba964cf61ec016eab7e4c4fd90c6bec1b368e8` DNSMasq failure. Root cause: `smartdns_runtime_nvram_snapshot.sh` still included obsolete `sdnse_tcp` in its failure matrix; config-generation regression had already passed. Commit 843 updates the matrix to `sdnse_tcp_server` and asserts its captured value. DNSMasq `45776763963`, AI `45776764152`, Process inventory `45776764005`, Process fail-closed `45776764177`, USB `45776764153` and XY-C1 `45776764204` were IN_PROGRESS at last poll; close this exact-SHA set later.
- MiniUPnP SHA `588beb5d574127fd4d0027559d9c8cb97e75af3d`: HTTP request framing `45775186800` and AI docs `45775186907` PASS.
- IAPP SHA `3bb7d977b30525c202009630892ea0afa94a0d9a`: IAPP `45773717841`, Process inventory `45773717856`, NVRAM `45773717775`, AI docs `45773717816`, and XY-C1 `45773717864` all PASS including Build/TRX/artifact.
- Firewall exact-SHA host/lifecycle checks begin after this commit is published. No local checkout/test execution or physical-router L4 is claimed.

## Exact first action for the next GPT

1. Fetch live `main`, identify this firewall SHA, and inspect exact-SHA Firewall Startup Stability, NVRAM audit, Process inventory/fail-closed, AI handoff and any triggered XY-C1 Build/TRX/artifact Actions. Diagnose failures from the first causal job log without weakening the error gate/regressions.
2. Close the SmartDNS run set on `8431202d8ce320b6df72b87d183749c44bea7f50`, especially DNSMasq `45776763963` and XY-C1 `45776764204`; don't mix those results with the firewall SHA.
3. Continue a distinct high-risk userland stability/security source task from `REFACTOR_ROADMAP.md`; don't redo startup, SmartDNS key, MiniUPnP or IAPP audits.

## Execution invariants

- Main only; one logical slice = all blobs, one complete tree, one atomic commit, then non-forced expected-parent SHA update. On CAS conflict, refresh and rebuild; never force-push.
- Never modify the frozen backup or reopen PR #1.
- Host Actions do not prove MIPS/uClibc runtime. XY-C1 L3 requires same-SHA Build, TRX verification and artifact upload PASS. Hardware L4 remains unverified absent explicit test.
- Update handoff only when engineering baseline, validation boundary or first concrete action materially changes; no poll-only or SHA-only commits.
