# Cross-session execution mandate (read before resuming)

Recover live state once, then continue the exact P1 task. Do not repeat completed audits, stop at a plan, or ask the user to select work. Continue independent engineering while CI is pending.

**Mandatory handoff checkpoint:** Every session must verify before ending that this file accurately records the meaningful source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If next-session-critical facts have changed, update this handoff; if already accurate, no no-op commit.

For each multi-file logical slice, publish source, tests/contracts and necessary docs in **one atomic main commit**: create all blobs, one tree, one commit parented to freshly fetched main, then non-forced update of main using that exact expected SHA. No branches/PRs, no force-push, no archive modifications.

## Current repository state

- Repository Arlene206/padavan-4.4-2; development only on main.
- Product source SHA: 5e16404907f16ac2321e69838b250d426c1aedd2 (watchdog daemon launcher failures now logged at the launch site, original eval status preserved).
- Current follow-up parent: 5e16404907f16ac2321e69838b250d426c1aedd2; this follow-up only fixes the watchdog workflow's inline Python contract and updates this checkpoint/log. Resolve current main SHA live.
- Frozen archive `backup/pre-plugin-overhaul-2026-11-14`: 72f4f157a084f8184b9597ca5bf334b8080e5807. Read-only; do not touch.
- No local checkout or MIPS/uClibc cross-toolchain in this session. Do not claim local full tests/builds.

- Current LPD PID identity hardening is parented to 4e0e078cfac3a3db90d7f100e255e65b4bb03cbc; resulting SHA is this changeset. It changes the LPD writer/startup reader and HTTPD's privileged LPD removal signal. Fresh exact-head process-lifecycle, HTTPD security, AI docs and XY-C1 checks are required.

## Active task

### P1 — Boot daemon startup error reporting

- Networkmap `start_networkmap(1)` boot failures are captured/logged in `start_services_once()` on product source e902e806; checker regex corrected in 7343fb3a.
- Watchdog `start_watchdog()` now captures `eval("/sbin/watchdog")`, logs nonzero launcher status and returns it unchanged on product source 5e164049. UNKNOWN process inventory and invalid policy already have diagnostics; RUNNING remains a successful no-op.
- `start_gpio_btn()` has internal logs for UNKNOWN inventory, failed launcher, UNKNOWN postcondition and daemon-not-remain-running; no duplicate caller log needed.
- `start_infosvr()` was completed in commit `4e0e078cfac3a3db90d7f100e255e65b4bb03cbc`: it captures the enabled `eval("/usr/sbin/infosvr", IFNAME_BR)` result, logs nonzero launch failures and returns the original status. The disabled `adsc_enable != 1` sentinel remains 1; UNKNOWN inventory diagnostics remain unchanged. Exact-head infosvr workflow #43732716469 PASS on SHA `72ce1bb97323d4c417e8a5d891bbaf9c6ed3f2f3`. Do not repeat this task.



### P1 LPD printer-removal PID identity hardening

- The process-lifecycle inventory identified HTTPD's trusted lpr_remove handler as a raw numeric-PID signal path: it sent SIGUSR2 to /var/run/lpdparent.pid without checking that the PID still belonged to LPD.
- LPD now publishes the shared strict PID/start_time record; startup validates it with process_pidfile_read_identity(). A narrowly scoped PID-only fallback remains for older firmware compatibility and is gated by O_NOFOLLOW/O_NONBLOCK, regular root-owned single-link checks, non-writable group/world mode, strict decimal parsing and process-inventory validation.
- HTTPD now uses process_pidfile_signal(..., "lpd", SIGUSR2); missing PID files remain harmless, while other signal/identity errors are logged and fail closed. The process-lifecycle source contract and HTTPD privileged-script sink contract were updated in the same changeset.
- Parent main for the LPD production change: 4e0e078cfac3a3db90d7f100e255e65b4bb03cbc; LPD source SHA 72ce1bb97323d4c417e8a5d891bbaf9c6ed3f2f3. The present source-contract correction is parented to 72ce1bb97323d4c417e8a5d891bbaf9c6ed3f2f3. Exact-head process-lifecycle, HTTPD security, AI docs and XY-C1 Actions are required. The Linux 4.4 PID/start_time validate-to-kill TOCTOU remains a documented limitation; this change narrows but does not eliminate it.

## Verification boundary

- On product source 5e164049, watchdog run #43732391911 failed only at the inline Python contract step due syntax error from double-escaped quote characters. Both GNU89 ASan/UBSan policy tests and NTP/service lifecycle checker passed. The separate process lifecycle run #43732391871, infosvr #43732392035, AI docs #43732391844, AdGuardHome #43732392074, DNSMasq #43732391865 and AliyunDrive #43732391870 passed exact-head.
- The current workflow correction used single-quoted Python literals; exact-head watchdog #43732516198 passed. AI docs #43732516259 failed solely because the handoff omitted its exact required `one atomic main commit` phrase; this source/docs commit restores that wording. Verify AI docs again on the new SHA.
- XY-C1 #43732391824 for product SHA 5e164049 was IN_PROGRESS at last check. Require Build, TRX verification and artifact upload for L3. Physical-router L4 is NOT VERIFIED.
- Earlier checker/handoff correction SHA 7343fb3a passed Networkmap #43732137598 and AI docs #43732137652. The parent product SHA e902e806 had XY-C1 #43731654808 still in progress.

- Exact SHA `72ce1bb97323d4c417e8a5d891bbaf9c6ed3f2f3`: process-lifecycle regression #43732716465 FAILED only at its new LPD signal source assertion because it expected the call on one line; all C compile/runtime checks, including shared PID-file and LPD legacy parser regressions, passed. HTTPD security #43732716493 FAILED at the matching privileged sink assertion for the same wrapped call; the preceding security checks passed. The current follow-up formats the call to match the already-exact sink contracts. AI docs #43732716471, process inventory #43732716633 and infosvr #43732716469 PASS; XY-C1 #43732716622 was IN_PROGRESS at last poll.

## Exact first action for the next GPT

1. Check exact-head process-lifecycle regression, HTTPD security, AI docs and XY-C1 Actions for the LPD PID identity changeset; inspect any causal failure and fix it. Do not claim L3 until Build XY-C1, TRX verification and artifact upload all pass for the exact product-source SHA.
2. Continue boot startup-status accounting beyond networkmap, watchdog and infosvr (all now covered); start_gpio_btn() already has internal diagnostics. Inspect remaining start_services_once() calls for discarded status and preserve disabled/no-op semantics before adding logs.
3. Keep development on main with atomic Git Data commits and expected-SHA non-forced ref updates. Do not repeat DNSMasq/VPN lock-order work or the scoped RC C-source process-signal sweep.


## Invariants

- A PENDING run is not a gate on independent work; retain exact run ID/SHA and continue safe work.
- DNSMasq/VPN lock-order audit and scoped 36-file RC C-source process-signal audit are completed; do not repeat. Linux 4.4 PID/start_time validation-to-signal TOCTOU remains unresolved.
- Preserve all boards, compatibility and error semantics. FTP failed-upload cleanup TOCTOU, NFS durability/AUTH_SYS and physical router behavior remain separate unverified topics.
- Before context exhaustion, finish a safe slice and update this handoff only for material changes to active task, source baseline, relevant CI, validation or first action.

Preserve `docs/dev-log/` archive shards and `docs/dev-log/INDEX.md` byte-for-byte.