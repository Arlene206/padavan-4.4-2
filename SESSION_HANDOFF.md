# Cross-session execution mandate

Continue this engineering task across ChatGPT conversations. Recover repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup. Record exact Action results and the next concrete task before context exhaustion.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; active branch: main.
- Parent for THIS bridge-member branch accounting correction: `aa5d3c9f2566927a4b17fce5d391f525cccc8095`; fetch live main and use expected-SHA non-forced CAS for the next write.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. One-time startup comparison found main 588 commits ahead / 1 behind. Do not repeat backup/archive recovery.
- Root `dev_log.md` had 103 newest-first records before the preceding bridge initialization commit and this correction; maintain 20–119 records and preserve archive shards/indexes.
- Startup governance/archive recovery is complete.

## Active correction

The essential bridge-init status propagation was committed in `aa5d3c9f2566927a4b17fce5d391f525cccc8095`. Post-commit source inspection caught two member-accounting omissions, now corrected by THIS commit:
- Router mode on GMAC2 bridge boards must record the LAN member attach only in the preprocessor-controlled `else` branch. The previous record sat outside that branch and could double-count the `eth2.1` attach result.
- AP mode with `AP_MODE_LAN_TAGGED` must capture the `IFNAME_LAN` bridge member attach result instead of silently discarding it.
- The Process semantic checker now asserts both exact attach/result paths and rejects a bare unchecked `br_add_del_if(IFNAME_BR, IFNAME_LAN, 1);` call.

The prior Process workflow run `43857640159` on exact SHA `aa5d3c9f2566927a4b17fce5d391f525cccc8095` was SUCCESS; job `133265909696` confirms the new bridge initialization regression and bridge membership regression both compiled and ran successfully, and the existing source contract passed. This success predates THIS source correction.

## Verification boundary

On `aa5d3c9f2566927a4b17fce5d391f525cccc8095`, these exact-SHA push checks were SUCCESS at the last inspection: Process lifecycle `43857640159`, secondary process inventory `43857640230`, HTTPD daemon lifecycle `43857640181`, WAN route `43857640132`, DNSMasq `43857640162`, detect_link `43857640209`, detect_internet `43857640179`, Firewall `43857640171`, NVRAM-adjacent-independent service checks including Wireless `43857640185`, rstats `43857640150`, plus multiple application lifecycle/security checks.
- Exact-SHA Process job `133265909696`: SUCCESS; steps `Compile and run bridge initialization status regression`, `Compile and run bridge multicast setup status regression`, and final `Verify LAN startup error propagation through PID 1 and LAN restart` all SUCCESS.
- NVRAM `43857640139`, USB storage `43857640310`, HTTPD authentication `43857640193`, AI docs `43857640173`, VPN `43857640155`, and XY-C1 firmware `43857640160` were IN_PROGRESS at the prior query. Recheck their current status; do not assume outcomes.
- No same-SHA XY-C1 Build+TRX+artifact L3 and no physical-router L4 result was confirmed at that query.
- THIS correction is newly published and requires new exact-SHA checks; no result for THIS SHA is claimed yet.

## Exact first action for the next GPT

1. Fetch live main and inspect exact-head Process, XY-C1, USB storage, NVRAM, AI docs and HTTPD authentication Actions for THIS correction. Fix the first causal failure.
2. XY-C1 L3 requires Build, TRX verification and artifact upload to pass on one identical production-source SHA. Hardware remains L4-only.
3. Continue switch base/storm/link result propagation with careful board/feature capability classification; do not make optional features boot-fatal.
4. Update handoff with exact run IDs and conclusions after the next meaningful source slice.

## Governance

Preserve security, architecture, compatibility, target toolchain and required tests. Main only; backup read-only. One complete Git tree and atomic commit per logical task; non-forced expected-SHA update. Update `dev_log.md` for every source change. Distinguish host regression, exact-SHA CI, firmware/TRX and physical-router evidence. Never claim an unrun result.
