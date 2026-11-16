# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify main/source baseline, active task, exact-head Actions, validation boundary, blockers, frozen backup and first next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task. Continue one continuous engineering effort across GPT conversations: recover once, avoid repeated audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next.

Every logical multi-file change must be one complete Git tree and one atomic main commit, parented to freshly fetched main, followed by a non-forced expected-SHA compare-and-swap. Never force-push, split a logical change into half-commits, create a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; development branch: main. Parent for the LAN route cleanup slice: 344db6b6cb4309ad1a2cdbf29b2cc0ee50ad5898; last meaningful product-source baseline before this slice: cffc1497fda9f65d9a0b163bad5acc1723c23f81.
- Conntrack flush status propagation is implemented in cffc149. The WAN route regression passed on 44fd after both stale source-contract assertions were corrected; the injected GNU89/ASan/UBSan conntrack policy test passed.
- XY-C1 run 43973861873 on exact product-source SHA cffc149 is SUCCESS, including Build XY-C1, TRX output verification and artifact upload. This verifies product-source L3 at cffc149; later c3b/44fd/0b934/344db docs/checker commits did not alter firmware source.
- Exact 44fd Actions: WAN route 43974652676 PASS; Process 43974652670 PASS; AI handoff 43974652705 PASS; NVRAM 43974652715 PASS; AdGuardHome 43974653150 PASS; AliDDNS 43974652681 PASS; AliyunDrive 43974652664 PASS; infosvr 43974652631 PASS.
- Exact 0b934 WAN DHCP callback 43975364179, AdGuardHome 43975363862, infosvr 43975363999, AliyunDrive 43975363922 and Process 43975363957 PASS. AI handoff 43975363964 failed because root dev_log.md had reached 120 entries. Commit 344db6b rotates the oldest 100 complete records into part-013, retains 20 and fixes the archive index; exact-head handoff rerun is required.
- New LAN teardown defect: stop_lan() discarded clear_if_route4(IFNAME_BR)'s status. This slice captures/logs it and aggregates conntrack, route-flush and bridge-down results without skipping IPv6/interface cleanup. It adds a first-error regression and source wiring assertions.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains read-only at 72f4f157a084f8184b9597ca5bf334b8080e5807. Physical-router L4 remains NOT VERIFIED.

## Active task

Validate router-mode LAN teardown status propagation: stop_lan() must retain clear_if_route4(IFNAME_BR) failure after conntrack cleanup, continue IPv6 cleanup and ifconfig(), and return the first nonzero status. The injected regression covers route-only error, earlier conntrack precedence, and bridge-down-only error; the Process source contract verifies production ordering. Inspect exact-head Process, WAN route, AI handoff and XY-C1 runs.

## Verification boundary

- Product-source L3 remains VERIFIED on cffc149: XY-C1 run 43973861873 completed Build XY-C1, TRX verification and artifact upload.
- Exact 44fd WAN route, Process and AI handoff checks passed. Exact 0b934 WAN DHCP callback, Process, AdGuardHome, infosvr and AliyunDrive passed. AI handoff failed only at root log count=120; 344db6b performs the 100-record rotation and requires an exact-head rerun.
- This LAN source/test/checker slice requires exact-head Process, WAN route, AI handoff and XY-C1. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live main and inspect exact-head Process, WAN route, AI handoff and XY-C1 results for this LAN route cleanup slice. Diagnose first causal failures and confirm same-SHA build/TRX/artifact.
2. Continue auditing status-returning route/firewall/interface cleanup calls in RC teardown paths; preserve cleanup ordering and first-error semantics.
3. Keep full-tree atomic commits and expected-SHA non-forced updates on main. Frozen backup stays untouched; do not claim L4 without hardware evidence.
