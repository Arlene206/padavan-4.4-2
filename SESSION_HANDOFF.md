## Current repository state

- Parent main before THIS COMMIT: `59681e0b5edf6803a47280c0c0256f596d217392`. Parent exact-head Process regression run `43927478287` and AI handoff documentation run `43927478510` passed; USB modem identity run `43927478496` also passed. Parent XY-C1 firmware run `43927478225` was still in progress on this exact SHA.
- Current source includes the static-IPoE `update_upnp()` result fix from `1b06d3178c366ae764f894d6bb15847b8f5f6e21`; parent `59681e0b5edf6803a47280c0c0256f596d217392` only corrected its source-checker regex. The current-source XY-C1 Build/TRX/artifact result is still pending.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable and read-only.
- PR #1 is closed/unmerged historical work. Its old head `ba48e22609e0c81ad757b06756fe91d05f39cbf6` is superseded by the main-line port recorded in the PR comment; the port commit `74b66ff960457379518d13c723c44a3515e6a228` is an ancestor of current main. Do not resume the PR branch.
## Active task

THIS COMMIT fixes a WAN startup-result gap in `trunk/user/rc/src/net_wan.c`: configured PPPoE MAN modes (DHCP, ZeroConf or static physical setup) call `launch_wanx()`, but its nonzero result was previously merged only inside the PPTP/L2TP abort guard. A failed configured physical-WAN adjunct could therefore be logged but omitted from `start_wan()`'s return status while PPPD continued. The common WANX error block now preserves the first error. PPPoE still attempts PPPD best-effort because it does not require a physical IPv4 lease; PPTP/L2TP keep their existing abort/restore path. Added a GNU89 policy regression and a source-contract assertion for both status reporting and continued PPPoE launch.
## Verification boundary

- Parent Process run `43927478287` passed the modem WAN policy binary and `scripts/check_modem_wan_start_status.py`; parent AI docs run `43927478510` passed. Parent XY-C1 run `43927478225` was `IN_PROGRESS`.
- This commit's exact-head Process, XY-C1 and AI docs Actions are pending. The existing workflows compile `tests/modem_wan_start_policy.c` with GNU89/`-Werror`/ASan/UBSan and execute the semantic source checker; XY-C1 then builds firmware, verifies TRX output and uploads an artifact. No local checkout was available to run the full test locally.
- L3 is NOT VERIFIED for THIS COMMIT until Build, TRX verification and artifact upload all pass on its exact SHA. Physical-router L4 remains NOT VERIFIED.
## Exact first action for the next GPT

1. Inspect exact-`head_sha` Process, XY-C1 and AI docs push runs for THIS COMMIT. Triage any failed job at its first causal step; treat pending work as pending and continue independent work.
2. Confirm the manual-PPPoE policy regression and production source contract pass, and record XY-C1 Build/TRX/artifact separately from host L2.
3. If the slice passes, continue the P1 RC startup-result audit at `reload_nat_modules()`: trace each enabled module load/unload result and every caller before deciding which failures must reach the WAN/boot status.
4. Keep the frozen backup read-only and physical-router L4 marked NOT VERIFIED.
