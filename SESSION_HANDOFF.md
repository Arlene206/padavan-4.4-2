# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Confirm the main/source baseline, exact-head Actions, validation boundary, blockers, frozen backup and precise next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Continue one engineering effort across GPT conversations: avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next. Finish safe slices, add regression coverage, inspect exact-SHA Actions, commit atomically and preserve the next task.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, create a development branch/PR, or modify the frozen backup.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`. Current source parent for the IPv6 neighbour-cleanup slice: `79ba2d6f818d0cb541032b3440834b883b1e531d`.
- IPv6 per-interface route/address/neighbour cleanup uses argv-based `eval()`. This slice adds neighbour flushing during native startup rollback when route/address side effects occurred, and during normal WAN teardown between route and address cleanup. Teardown neighbour failures are logged and included in first-error aggregation, preventing saved WAN IPv6 state from being cleared after incomplete cleanup.
- PPP probe/custom WAN hook operands in `net_wan.c`, L2TP/mwan process launches in `net_ppp.c`, and SIT/native WAN IPv6 tunnel/route operations use argv-based `eval()`; PPP lifecycle locks and DHCPv6 callback-generation fencing remain.
- Exact-head parent `79ba2d6f818d0cb541032b3440834b883b1e531d`: VPN, Firewall, HTTPD, infosvr, AdGuardHome, AliyunDrive, main Process inventory, AliDDNS, OpenVPN credential and NVRAM passed; AI handoff failed because the exact mandatory checkpoint phrase was absent, and Process fail-closed failed because its source contract still expected the old `restore_status != 0` after the implementation correctly changed to `restore_status < 0`. XY-C1 was pending. These checker/documentation failures are corrected in this follow-up. Parent `b9850f9917a6eb77b7c6d021abe75c88b4ca444c` passed VPN lifecycle, AI handoff, infosvr, WAN DHCP, Process inventory, AliyunDrive, HTTPD, WAN route, AdGuardHome, AliDDNS and Process fail-closed checks; NVRAM and XY-C1 were pending.
- Same-SHA product build L3 is verified for predecessor `f87343b7e7a2da599c16baf1192ec1561dc44a9b`: XY-C1 Build, TRX verification and artifact upload all PASS (run `43992613368`, job `133736749350`). Later source SHAs require their own full Build/TRX/artifact verification. Physical-router L4 remains NOT VERIFIED.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## Active task

Verify the exact-head Actions for the VPN restore-status correction and its documentation/test follow-up. Resolve the known process-integration assertion drift and handoff wording failure. Then verify the IPv6 neighbour-cleanup change and same-SHA XY-C1 Build/TRX/artifact.

## Verification boundary

- Predecessor `f87343b7e7a2da599c16baf1192ec1561dc44a9b` has product-source MIPS/uClibc L3 VERIFIED.
- The xl2tpd result normalization and VPN restore-sentinel correction are present on parent `79ba2d6f818d0cb541032b3440834b883b1e531d`; VPN and related source checks passed, with the Process fail-closed contract and AI handoff wording failures now fixed here. The IPv6 neighbour-cleanup source/tests and target build require new exact-SHA verification.
- Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch exact-head Actions for current main, especially WAN DHCP, WAN route, Process, Firewall, VPN and XY-C1. Fix the first causal regression from the specific job log.
2. Confirm same-SHA XY-C1 Build, TRX verification and artifact upload. Continue safe PPP/L2TP teardown review while builds are pending.
3. Keep backup frozen and developer-log history intact; use one complete Git tree and non-forced expected-SHA main updates.
