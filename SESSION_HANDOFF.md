# 2026-11-18 continuation checkpoint — Ralink IAPP firewall ownership lifecycle

## Current repository state
- Parent main before THIS source slice: `b4ffae1ce9eae0f05f07bdcd9ed488ac38e101c0`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Factory-reset source `0d58928e`: GPIO run `44197451556`, Process `44197451701`, MiniUPnP `44197451163`, AI docs `44197796498` PASS; XY-C1 `44197454090` completed SUCCESS with Build, TRX and artifact upload.
- Current post-WAN hook source `2a98a8303c26414a8f5084b78f1642ea282d164c`: WAN teardown `44200079921`, WAN gateway ARP `44200079811`, AdGuardHome `44200079919`, AI handoff `44200079479`, infosvr `44200079580`, USB modem `44200079615`, VPN server `44200079546`, WAN DHCP callback `44200079468`, detect_internet `44200079791`, ZeroTier `44200079778`, Process `44200079549`, DNSMasq `44200079712`, detect_link `44200079897`, MiniUPnP `44200079805`, WAN route `44200079558`, AliDDNS `44200079513`, Wireless radio `44200079559`, AliyunDrive `44200079775`, HTTPD security `44200079499`, Firewall Startup `44200079836`, and Process inventory `44200079659` PASS. NVRAM transaction `44200079736` was IN_PROGRESS; XY-C1 `44200079523` was IN_PROGRESS at last poll.
- New P1 finding: `trunk/user/ralinkiappd/iappd.sh` added TCP/UDP port 3517 INPUT allow rules during start but did not remove them on normal stop. Rules therefore survive daemon disable and repeated restart can duplicate them.

## Active task
- THIS slice tags IAPP-owned TCP/UDP rules with explicit comment ownership, removes stale tagged rules before startup, and removes active tagged rules only after the daemon absence postcondition is confirmed. It preserves untagged administrator rules.
- Regression extends the existing POSIX mock with persistent iptables state; tests cover normal stop, stale cleanup, restart idempotency, unowned rule preservation, and cleanup-failure reporting/retry. Dedicated workflow and XY-C1 pre-build check added.

## Verification boundary
- Exact-head IAPP lifecycle and XY-C1 for THIS commit are pending until Actions complete.
- The source SHA `2a98a830` had WAN teardown, WAN route and most userland regression runs PASS; NVRAM transaction and XY-C1 were IN_PROGRESS at last poll.
- Factory reset exact XY-C1 Build/TRX/artifact passed on `0d58928e`; no implication for newer source. Local shell run is NOT RUN (no mounted checkout here). Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch live main and exact-`head_sha` runs for IAPP lifecycle, XY-C1, NVRAM transaction and AI handoff for THIS commit.
2. Require the IAPP shell regression to pass ownership isolation, stale cleanup, normal stop/restart and cleanup-failure retry. For XY-C1, require Build, TRX and artifact completion.
3. Repair the first causal failure in a separate atomic main commit; continue an independent P1 issue while any unrelated run is pending.
4. Keep the backup immutable and hardware L4 marked NOT VERIFIED.

## Mandatory handoff checkpoint
Every session MUST inspect the live repository state, exact-SHA Actions that materially changed, current handoff/roadmap facts, and the first actionable next task before ending. The checkpoint is mandatory; a document edit is conditional. Record new PASS/FAIL/PENDING state accurately. If the facts and first action remain accurate, leave the handoff untouched and do not create a no-op commit.

# Cross-session execution mandate

Continue one engineering effort across GPT conversations. Recover once, then implement concrete defects. Do not wait idly on PENDING Actions or ask the user what to do next.

Every logical multi-file change must use Git data APIs to create all blobs, one complete tree and one atomic main commit, parented to freshly fetched main and published with non-forced expected-SHA compare-and-swap. Never force-push, create development branches/PRs or modify the frozen backup.

Before context exhaustion, finish a safe slice, commit atomically, inspect exact-SHA Actions and leave a precise first action.
