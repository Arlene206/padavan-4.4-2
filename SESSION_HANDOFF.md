# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect live repository state and complete an end-of-session accuracy checkpoint. Record exact main SHA, exact-SHA Actions, test/build boundaries and the next precise task. If facts remain accurate, explicitly conclude with no file change and no no-op commit. Use one atomic main commit for material changes only. Before context exhaustion, leave an accurate handoff and complete any safe bounded engineering slice.

- Work only on `main`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update `main` only with `force:false` and a freshly fetched expected SHA.
- PASS continues, FAIL requires diagnosis/correction, PENDING does not block independent work.
- Do not claim local tests, production builds, TRX/artifacts or hardware checks that were not performed.

## Current repository state

- Live `main` at this checkpoint: `447d3683e2ae30c7e82b699d89dc978f75b32136` (tree `8fa8e571c8a8544e00b7d89d41f22494d01bf5ac`).
- Frozen backup branch: `backup/pre-plugin-overhaul-2026-11-14`, last verified at `72f4f157a084f8184b9597ca5bf334b8080e5807`; keep unchanged.
- Recent source work: PID 1 signal mask construction hardening; Adbyby DNS/firewall cleanup error propagation; SmartDNS NVRAM transition failure handling and removal of an unsupported Adbyby switch invocation; storage/swap hardening; WAN teardown workflow contract repair.
- Continue with atomic tree + commit updates guarded by `expected_sha`; never force-push.

## Active task

### Completed implementation slices
- PID 1: `sigemptyset` / `sigaddset` results now checked through `pid1_signal_policy.h`; incomplete lifecycle masks prevent handler installation, and recovery-shell SIGCHLD mask construction fails closed before fork. Regression coverage and workflow contracts were aligned in `616a092a...`, `99c3da5c...`, `ad1837f1...`.
- WAN teardown: fixed workflow source-contract policy bindings after an observed NameError in the inline checker; commit `8728b518...`.
- Storage: removable-media uncertainty and hdparm failures are handled fail-closed; module loader scratch state is isolated; swap inventory parsing and caller source contracts tightened. Key checkpoints: `c8fdaeed...`, `ca8e79ec...`, `d1e0edcf...`, `9c75e1bf...`, `abe7fc71...`, `4c6d1b9c...`, `790fb2ab...`.
- Adbyby: DNS cleanup now attempts all independent stages while preserving first error (`9999fdc5...`); firewall snapshot cleanup failure remains visible (`a633cf30...`) with test event fix in `c9cd34c1...`.
- SmartDNS: NVRAM failures preserve previous in-memory DNS owner and propagate through Start/Stop (`80bfd667...`); checker count corrected (`5c929dbe...`); unsupported `/usr/bin/adbyby.sh switch` no-op calls removed from `Change_adbyby()` with regression/source contracts in the latest main slice `447d3683e2ae30c7e82b699d89dc978f75b32136`.
### Next implementation target
Verify the SmartDNS↔DNSMasq ownership transition end-to-end: trace where `adbyby_add`, `hosts_type`, and generated Adbyby host/DNS files are actually consumed; add a runtime-config contract test for both directions before modifying any further behavior. Then select a new stability defect outside the already-covered PID1/storage/Adbyby slices.

## Verification boundary

Exact-SHA workflow snapshots:
- Current main `447d3683e2ae30c7e82b699d89dc978f75b32136`: USB storage hotplug stability regression=in_progress; XY-C1 Firmware Validation=in_progress; Process lifecycle inventory - 447d3683e2ae30c7e82b699d89dc978f75b32136=completed/success; DNSMasq lifecycle stability regression=completed/success.
- PID 1 follow-up `ad1837f1...`: AI handoff documentation contract=completed/success; PID 1 mdev hotplug configuration security regression=completed/success; HTTPD Authentication and Security Regression=completed/success; Process inventory and lifecycle fail-closed regression=completed/success; XY-C1 Firmware Validation=completed/success.
- WAN checker correction `8728b518...`: WAN interface teardown stability regression=completed/success; AI handoff documentation contract=completed/success.
- Adbyby test correction `c9cd34c1...`: HTTPD Authentication and Security Regression=completed/success; AI handoff documentation contract=completed/success; Adbyby download and dnsmasq configuration stability regression=completed/success; Process lifecycle inventory - c9cd34c1973235544fff1779bc93bc4c16526247=completed/success.
- Storage checkpoint `790fb2ab...`: AI handoff documentation contract=completed/success; Process lifecycle inventory - 790fb2abc36bbda06f1628c0d6c3aae040fcb4fe=completed/success; HTTPD Authentication and Security Regression=completed/success; USB storage hotplug stability regression=completed/success; XY-C1 Firmware Validation=completed/success.
- SmartDNS DNSMasq checkpoint `5c929dbe...`: DNSMasq lifecycle stability regression=completed/success.
- Adbyby DNS cleanup checkpoint `9999fdc5...`: Adbyby download and dnsmasq configuration stability regression=completed/success; HTTPD Authentication and Security Regression=completed/success; Process lifecycle inventory - 9999fdc5ccf193df63e98182069d6aa71d5b6a73=completed/success; Process inventory and lifecycle fail-closed regression=completed/success; XY-C1 Firmware Validation=completed/success.
- A green unit/workflow run does not prove router runtime correctness. Treat the latest XY-C1 firmware build and device validation as unverified unless an exact-SHA run explicitly confirms them.

## Exact first action for the next GPT

1. Fetch the live `main` SHA and its Actions; do not repeat completed PID1/storage audits.
2. Trace SmartDNS DNS ownership through `Change_adbyby()`, `adbyby_add`, `hosts_type`, generated host files and daemon config. Confirm both directions and add regression coverage for the actual runtime config.
3. Make one bounded source/test/checker change, commit the full tree atomically with expected-SHA protection, and verify the exact commit’s Actions.
4. Keep handoff accurate. The checkpoint is mandatory; a document edit is conditional.

