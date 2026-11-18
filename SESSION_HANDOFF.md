# 2026-11-18 continuation checkpoint — daemon startup wait contract alignment

## Current repository state
- Repository `Arlene206/padavan-4.4-2`, branch `main`. Live main immediately before this checkpoint: `a0e5fe46e279e9356272ca2d1972a5a6e2309398`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` stays read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`. PR #1 stays closed/unmerged; AliDDNS CA/HMAC/bootstrap work has already landed in main. Do not touch the backup, PR #1, or GPT Session Rotator.
- Recent product work: SmartDNS managed DNSMasq preservation (`9bb9fda`), WAN/PPP EINTR-safe settling waits (`e0c8269`), NFSd settling wait (`90cce742`), daemon retry-wait scoping/bracing (`8320547`) and logger startup settling-wait policy (`a0e5fe4`).

## Active task
- Complete source-contract updates after the RC daemon startup functions migrated from direct `sleep()` to `service_start_probe_wait()`.
- Exact Process run `44324115817` on `83205473630449515d6b4c475c4f017adfcd246c`: LLTD/rstats detached-start contract now PASSED after the previous checker fix, but the next assertion failed because `telnetd_start.index("sleep(PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS);")` still expected raw sleep. Current `start_telnetd()` and `start_ttyd()` already use `service_start_probe_wait("telnetd"/"ttyd", ...)`; current `start_syslogd()` and `start_klogd()` likewise use the shared helper. THIS COMMIT changes the duplicated initial-wait ordering assertions for all four functions to their actual helper call, retaining the ordering before bounded process-inventory probes.
- Current head `a0e5fe4` also adds a tested logger startup policy and closes braces around conditional retry waits in multiple RC daemons. Those source changes are not altered by THIS COMMIT.

## Verification boundary
- WAN DHCP callback run `44323940802` on exact SHA `9cd644625e403a4b546d2be2478ba6e3c8c66081`: PASS after both stale DHCP settle assertions were aligned with the shared EINTR-safe `service_wait_seconds(2)` helper.
- AI handoff documentation run `44323940948` on `9cd6446`: PASS.
- Process run `44324115817` on `8320547`: FAIL only at the next stale telnetd initial-wait assertion. It printed PASS for the prior LLTD/rstats contracts, IAPPD status propagation and optional RC daemon feature guards. Process inventory run `44324115721` on the same SHA: PASS.
- On `a0e5fe46e279e9356272ca2d1972a5a6e2309398`, the gateway ARP, ZeroTier, Crond, DNS Forwarder launch path, PID 1 notification, Networkmap, DDNSTO, watchdog signal, HTTPD daemon, detect_internet, AliDDNS signed transport, infosvr packet/signal and GPIO daemon workflows passed; the XY-C1 build, NVRAM audit and remaining module workflows were in progress at last poll.
- XY-C1 run `44322760659` on `90cce742` was still in progress at last poll. SmartDNS L3 is proven only for exact product SHA `9bb9fdaeff826c957b599cc45e695eb3fb8cd32c` (Build/TRX/artifact all PASS). The latest WAN/RC source has not yet been confirmed by a successful XY-C1 build. Target BusyBox/uClibc runtime and physical-router L4 remain NOT VERIFIED.
- No local repository checkout or local tests were available; do not claim them.

## Exact first action for the next GPT
1. Fetch current main and all exact-head Actions. Inspect the Process regression created by THIS COMMIT, plus the XY-C1 build/NVRAM/module results for `a0e5fe4`.
2. If Process still fails, follow the next first failing line and compare the asserted contract with current source. Do not weaken runtime behavior contracts.
3. Continue the P1 WAN/firewall and broader userland stability backlog in the roadmap after shared startup-wait checks are green.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional. Every session MUST compare live main and exact-head Actions before ending. Leave this handoff unchanged if the task and verification boundary remain accurate.

## Cross-session execution mandate
One logical slice is one atomic main commit. Create a complete tree from the exact live parent, then update main non-forced with expected-SHA compare-and-swap. Continue independent work while Actions are pending. Before context exhaustion, leave the next GPT exact first action, SHA, test evidence and CI status. Never claim unobserved local tests, MIPS builds or physical-router validation.
