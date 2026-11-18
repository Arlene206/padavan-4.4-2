# 2026-11-19 continuation checkpoint — require verified WAN switch VLAN isolation

## Repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main` only.
- This checkpoint is included in the current atomic main commit (parent: `26f0638d76b94a076278f14cf976a6a015d310dc`); resolve its resulting SHA from live `main` during the next recovery.
- The archival branch `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Never switch to, update, or delete it.
- PR #1 remains closed/unmerged; do not revive it.
- No local checkout or MIPS build was available in this session. Physical-router L4 remains NOT VERIFIED.

## Closed previous candidate

- The prior handoff requested investigation of the five `void start_*` wrappers for AdGuardHome, DDNSTO, Adbyby, AliDDNS and FRP.
- The whole `trunk/user/rc/src/*.c` inventory plus `rc.h` was searched. Those five functions have no actual C callers; `rc.c` dispatches through the status-returning `restart_*` entry points. No prototype change or dead-code refactor is justified. This candidate is closed with no source modification.
- `restart_crond() < 0` is intentional: disabled `start_crond()` returns positive 1 as a benign no-op, while startup failures are negative. Do not convert that condition to `!= 0`.

## Implemented source slice — THIS COMMIT

The router-mode `init_bridge()` previously recorded failure of `switch_config_vlan(1)` in its overall bridge status, but `init_router()` only used that aggregate for logging/boot status. It then allowed `start_wan()` whenever the firewall/NAT policy passed, even when the required switch bridge/VLAN isolation operation had failed.

- `init_bridge()` now exports only the `switch_config_vlan(1)` result through an optional output parameter. AP mode reports zero because it does not enter router-mode WAN startup. Other later bridge/radio/guest errors remain separate and do not accidentally become WAN prerequisites.
- `init_router()` supplies the output parameter to `firewall_startup_policy_allows_wan()`. A nonzero switch VLAN result blocks WAN before `start_wan()`, even when firewall and NAT are explicitly disabled; the existing IPv4 and active-IPv6 forwarding shutdown path is reused.
- The firewall policy regression covers positive and negative switch errors and verifies successful VLAN configuration preserves the previous firewall/NAT decisions.
- The firewall startup workflow's push/pull_request dependency sets are synchronized and now include `net_lan.c`. The XY-C1 workflow includes the production policy header and host regression in its push filter; the firmware-input detector keeps the test path host-only and treats the production header as a firmware input.

Changed paths: `trunk/user/rc/src/net_lan.c`, `trunk/user/rc/src/rc.h`, `trunk/user/rc/src/rc.c`, `trunk/user/rc/src/firewall_startup_policy.h`, `trunk/user/rc/src/tests/firewall_startup_policy.c`, `.github/workflows/firewall-startup-stability.yml`, `.github/workflows/xy-c1-firmware-validation.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.

## Verification boundary

- Parent main before this slice: `26f0638d76b94a076278f14cf976a6a015d310dc`.
- Exact parent-head Actions already observed: Process inventory `44344497118` SUCCESS and Shadowsocks proxy lifecycle `44344497203` SUCCESS.
- The earlier XY-C1 firmware run `44342177852` on product SHA `64a208a94e3609ed73633451b8afc0fe6c5db7a8` has now completed SUCCESS: host checks, Build XY-C1, TRX verification and artifact upload all passed. Artifact ID `14444558335`, size 12,659,242 bytes. This is L3 evidence for that product SHA, not for this new commit.
- The exact-head firewall startup, Process lifecycle, XY-C1 and AI handoff-documentation Actions for THIS COMMIT are PENDING until fetched by exact `head_sha`. Do not infer PASS from the parent.
- No local C regression was executed in this session. The updated firewall workflow must compile/run the GNU89 `-Werror` ASan/UBSan test. XY-C1 target build/TRX/artifact and physical hardware are separate validation levels.

## Exact first action for the next GPT

1. Fetch live `main` and inspect all Actions whose `head_sha` equals the resulting commit SHA. Check firewall startup regression, Process lifecycle regression, XY-C1 firmware validation and AI handoff documentation. Inspect failed job logs before calling any result validated; continue independent work while jobs are pending.
2. Once the firewall policy regression passes, continue the P1 PID-1 startup reliability review at `trunk/user/rc/src/init.c::init_sysctl()`: trace actual implementations/callers of `fput_int()`, `set_tcp_tweaks()`, `set_interface_conf_int()` and `control_if_ipv6_all()`; classify required network/firewall sysctls separately from optional tuning before changing return propagation. Add a focused regression/source contract for any confirmed failure path. Do not turn every best-effort tuning write into a router boot blocker without evidence.
3. Continue cross-userland stability/security work after closing the current exact-SHA failures. Do not repeat the completed startup-wrapper caller search, CI trigger governance audit, or PPPoE-MWAN probe review without new evidence.

## Delivery rules

One coherent task per atomic main commit. Build the complete Git tree from the exact current parent and update `main` only with a non-forced expected-SHA compare-and-swap. Never create development branches or PRs; never modify the archival backup. Do not claim tests, builds, Actions or hardware results that were not actually observed. Update this handoff only when the active task, source baseline, validation boundary or precise next action materially changes.
