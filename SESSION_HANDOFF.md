# Padavan 4.4 — Live session handoff

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`; live main SHA at the start of this round: `1bf8a93bfa47770e8005d5241f7077287c86c4c4` (parent `f39638ac28bae38e6ec8d2694831685b3be98d4e`). Remote branch inventory contains only `main`; no checkpoint branch exists.
- Previous ZeroTier stop/start safety slice is L1/L2 CLOSED on exact SHA `1bf8a93bfa47770e8005d5241f7077287c86c4c4`: ZeroTier lifecycle Actions [43143884566] and AI handoff docs [43143884420] both SUCCESS. All job steps and complete logs were inspected. The five-scenario bounded TERM/KILL, start-gate and persistent-identity preservation regression passed; RC/ROMFS integration contract passed; docs validator reported root log=41/119 and archives dev-log=7, handoff=22, roadmap=18.
- DDNSTO stop/restart hardening is L1/L2 CLOSED on exact SHA `f39638ac28bae38e6ec8d2694831685b3be98d4e`: DDNSTO [43140910335] and AI docs [43140910294] both SUCCESS; five lifecycle scenarios and RC integration contracts passed.
- Root `dev_log.md` has 42 complete records after THIS COMMIT; no rotation or INDEX update is due. Existing archive inventory remains dev-log=7, handoff=22, roadmap=18.
- FTP failed-upload cleanup TOCTOU remains P2 OPEN/BLOCKED pending a Linux 4.4-safe inode/parent proof that preserves STOR/APPE/REST/STOU and concurrent external-writer semantics. NFS async durability/AUTH_SYS/root-squash remain real-client evidence-gated.
- The development container cannot clone GitHub because DNS resolution for github.com fails. No local full checkout or local execution of the whole-repository documentation validator is claimed. Exact-head Actions are authoritative for L2.

## Active task — Bound ZeroTier interface and NAT-route readiness in rules() (P2 stability)

- The previous ZeroTier stop/start slice is closed at L1/L2 on exact parent SHA `1bf8a93bfa47770e8005d5241f7077287c86c4c4`; exact-head lifecycle and handoff-doc Actions [43143884566, 43143884420] both succeeded.
- Continued the same explicitly recorded ZeroTier follow-up. Source review found two unbounded waits in `rules()`: waiting for a `zt*` interface, then waiting for its kernel route when NAT is enabled. The route grep required two spaces between interface and `proto`, while normal iproute2 output has one; this could wait forever. The code also installed some firewall rules before the route wait completed and used an unquoted numeric test on a possibly empty NVRAM value.
- THIS COMMIT adds separate 30-second, deadline-rechecked interface and route waits; identifies the interface by its `zt` prefix; parses route tokens and requires the exact interface plus `proto kernel`; validates the NAT setting; and delays all firewall changes until every required readiness condition is met.
- If readiness fails after launching the daemon, `start_instance()` attempts to stop it and returns failure instead of leaving the RC start path blocked indefinitely. `del_rules()` uses the same token-based route parser.
- The existing ZeroTier regression now covers delayed interface appearance, missing-interface timeout, single-space route output, unrelated-interface route exclusion, delayed NAT-route appearance, missing-route timeout and no partial firewall publication. The existing checker enforces the production call path and path-filter coverage. Exact-head ZeroTier and AI-doc Actions must pass before L2 closure.
- FTP failed-upload cleanup TOCTOU remains P2 OPEN/BLOCKED; do not patch until the Linux 4.4-safe inode/parent design preserves STOR/APPE/REST/STOU and concurrent writers. NFS remains real-client evidence-gated.

## Verification boundary

- ZeroTier bounded stop/start and fail-closed persistent cleanup: L1/L2 CLOSED on exact SHA `1bf8a93bfa47770e8005d5241f7077287c86c4c4`; lifecycle Actions [43143884566] and AI docs [43143884420] both SUCCESS. Full job steps/logs inspected; bounded stop/start regression, persistent-config preservation and RC/ROMFS source contracts PASS; docs validator root log=41/119, archives=7/22/18.
- Current ZeroTier interface/route readiness change: L1 source review PASS for finite waits, exact interface/protocol route parsing, fail-closed NAT config handling, delayed firewall publication and startup cleanup on readiness failure. The expanded host regression and semantic checker are committed in THIS COMMIT; exact-head Actions are pending at commit creation.
- No full repository checkout is available locally, so the new shell regression, source checker and `scripts/check_ai_handoff_docs.py` were NOT RUN locally. Do not report a local result; use exact-head Actions.
- DDNSTO L1/L2 CLOSED on exact SHA `f39638ac28bae38e6ec8d2694831685b3be98d4e`: DDNSTO [43140910335] and AI docs [43140910294] both SUCCESS; every step/full log was inspected.
- L3 production MIPS/uClibc objects, target BusyBox/procfs behavior, complete firmware/TRX and L4 physical-router behavior remain NOT VERIFIED. ZeroTier live daemon timing and target route/interface readiness remain target-side verification gaps.

## Exact first action for the next GPT

1. Fetch live `main` and record its exact SHA. Inspect ZeroTier lifecycle stability and AI handoff documentation Actions for that SHA, including every job step and complete logs. Require delayed interface readiness, 30-second missing-interface timeout, normal single-space `ip route` parsing, exact interface/protocol match, delayed NAT-route success, 30-second missing-route timeout, no partial firewall publication, startup failure cleanup, RC/ROMFS integration and docs validator (root log=42/119, archives dev-log=7, handoff=22, roadmap=18) all to PASS.
2. Once exact-head L2 is green, close the bounded ZeroTier readiness slice and rotate to another under-covered optional-service integration from the roadmap: AdGuardHome, AliyunDrive WebDAV, FRP, AliDDNS or Adbyby. Trace RC callers, process identity/stop confirmation, resource cleanup, publication and error propagation; choose one concrete stability defect, not another broad audit-only round.
3. Keep FTP upload-cleanup TOCTOU P2 OPEN/BLOCKED until an evidenced Linux 4.4-safe inode/parent design preserves STOR/APPE/REST/STOU and external writers. NFS remains real-client evidence-gated. Full MIPS/uClibc firmware/TRX and L4 remain NOT VERIFIED.
