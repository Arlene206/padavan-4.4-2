# Cross-session execution mandate (read before resuming)

Recover live state once, then continue the exact P1 task. Do not repeat completed audits, stop at a plan, or ask the user to select work. Continue independent engineering while CI is pending.

**Mandatory handoff checkpoint:** Every session must verify before ending that this file accurately records the meaningful source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If next-session-critical facts have changed, update this handoff; if already accurate, no no-op commit.

For each multi-file logical slice, create all blobs, one tree, one commit parented to freshly fetched main, then non-forced update of main using that exact expected SHA. Keep source, tests/contracts and necessary docs atomic. No branches/PRs, no force-push, no archive modifications.

## Current repository state

- Repository Arlene206/padavan-4.4-2; development only on main.
- Product source SHA: 5e16404907f16ac2321e69838b250d426c1aedd2 (watchdog daemon launcher failures now logged at the launch site, original eval status preserved).
- Current follow-up parent: 5e16404907f16ac2321e69838b250d426c1aedd2; this follow-up only fixes the watchdog workflow's inline Python contract and updates this checkpoint/log. Resolve current main SHA live.
- Frozen archive `backup/pre-plugin-overhaul-2026-11-14`: 72f4f157a084f8184b9597ca5bf334b8080e5807. Read-only; do not touch.
- No local checkout or MIPS/uClibc cross-toolchain in this session. Do not claim local full tests/builds.

## Active task

### P1 — Boot daemon startup error reporting

- Networkmap `start_networkmap(1)` boot failures are captured/logged in `start_services_once()` on product source e902e806; checker regex corrected in 7343fb3a.
- Watchdog `start_watchdog()` now captures `eval("/sbin/watchdog")`, logs nonzero launcher status and returns it unchanged on product source 5e164049. UNKNOWN process inventory and invalid policy already have diagnostics; RUNNING remains a successful no-op.
- `start_gpio_btn()` has internal logs for UNKNOWN inventory, failed launcher, UNKNOWN postcondition and daemon-not-remain-running; no duplicate caller log needed.
- Next inspect/fix `start_infosvr()`: when `adsc_enable != 1`, return 1 is a disabled sentinel; optional-daemon helper already logs UNKNOWN inventory. Enabled `eval("/usr/sbin/infosvr", IFNAME_BR)` failure currently lacks a specific diagnostic. Preserve the return value and disabled no-op semantics.

## Verification boundary

- On product source 5e164049, watchdog run #43732391911 failed only at the inline Python contract step due syntax error from double-escaped quote characters. Both GNU89 ASan/UBSan policy tests and NTP/service lifecycle checker passed. The separate process lifecycle run #43732391871, infosvr #43732392035, AI docs #43732391844, AdGuardHome #43732392074, DNSMasq #43732391865 and AliyunDrive #43732391870 passed exact-head.
- The current workflow correction uses single-quoted Python literals; new exact-head watchdog run required. The source is unchanged from product SHA 5e164049.
- XY-C1 #43732391824 for product SHA 5e164049 was IN_PROGRESS at last check. Require Build, TRX verification and artifact upload for L3. Physical-router L4 is NOT VERIFIED.
- Earlier checker/handoff correction SHA 7343fb3a passed Networkmap #43732137598 and AI docs #43732137652. The parent product SHA e902e806 had XY-C1 #43731654808 still in progress.

## Exact first action for the next GPT

1. Fetch live main; check exact-head watchdog-signal-stability and AI docs actions triggered by this workflow-only correction. Diagnose any failure from its job log. Recheck XY-C1 #43732391824 for product SHA 5e164049; don't wait idly.
2. Continue P1 at `start_infosvr()` in `services.c`: add focused failure logging for enabled launcher errors, keeping disabled `adsc_enable` status 1 benign. Add a relevant source contract to infosvr workflow; verify XY-C1 includes `services.c`.
3. Continue other boot service start return contracts only after checking whether nonzero is disabled/no-op or already logged internally.

## Invariants

- A PENDING run is not a gate on independent work; retain exact run ID/SHA and continue safe work.
- DNSMasq/VPN lock-order audit and scoped 36-file RC C-source process-signal audit are completed; do not repeat. Linux 4.4 PID/start_time validation-to-signal TOCTOU remains unresolved.
- Preserve all boards, compatibility and error semantics. FTP failed-upload cleanup TOCTOU, NFS durability/AUTH_SYS and physical router behavior remain separate unverified topics.
- Before context exhaustion, finish a safe slice and update this handoff only for material changes to active task, source baseline, relevant CI, validation or first action.

Preserve `docs/dev-log/` archive shards and `docs/dev-log/INDEX.md` byte-for-byte.