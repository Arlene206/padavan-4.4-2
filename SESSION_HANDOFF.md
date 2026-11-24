## Current repository state

- Live parent `main`: `9ce145922915fa4b09217b1253a5490a9e2fbda9`; this atomic continuation will be its direct child. Re-fetch `main` before every later write and never force-push.
- Current source task: router hostname publication in `trunk/user/rc/src/net_wan.c:update_hosts_router()`. The caller `prepare_dns_dhcpd()` in `services_ex.c` already aborts on its returned error.
- Adbyby executable/PID-start_time stop identity hardening is present on parent `9ce145922915fa4b09217b1253a5490a9e2fbda9`, following parent source `ceda64cdd021684c8b9b876fb29ecafa150a9e81`. Its source regressions and exact-main push Actions still require live verification; do not infer status from earlier SHA results.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Only `main` is the development branch. Branch inventory contains main + this archive; no open PRs were found.

## Active task

Router kernel hostname/domain failure propagation:
- `update_hosts_router()` atomically publishes `/etc/hostname` and `/etc/hosts`, then previously ignored `sethostname()` / `setdomainname()` return values.
- `prepare_dns_dhcpd()` already consumes a nonzero return as a hard preparation gate; preserve that caller contract.
- This slice reuses `lan_kernel_names_apply()` with errno-preserving WAN adapters, attempts both syscalls, returns the first error, and retains a primary kernel error if lock release also fails. It does not roll back a valid published file pair.
- Existing `trunk/user/rc/src/tests/lan_kernel_name_policy.c` is added to Process GNU89/ASan/UBSan CI; source contract pins adapters, publication→kernel update→unlock ordering, and returned status.

## Completed development slices

- Adbyby process identity hardening on parent `9ce145922915fa4b09217b1253a5490a9e2fbda9`: pidof candidates are bound to exact `/proc/PID/exe` and parsed non-zombie `/proc/PID/stat` start_time; stop revalidates both immediately before TERM. Host fixtures cover multiple PIDs, same-name wrong-executable decoys, malformed proc metadata and changed start_time.
- WAN router hostname/domain updater: this atomic commit; source regression integration is included. The on-disk file pair remains published if a kernel name syscall fails; the DNSMasq caller receives the failure before daemon preparation.
- Previously completed unrelated modules and checks are retained in `REFACTOR_ROADMAP.md`; do not repeat their audits without a new failing case.

## Verification boundary

- Local verification: the existing LAN kernel-name policy test is compiled/run in GNU89 mode with ASan/UBSan during this session; Process Actions compile/run the same test on the exact main SHA.
- Exact-head Push Actions for parent `9ce145922915fa4b09217b1253a5490a9e2fbda9` cannot be listed with the available GitHub connector: combined commit status returned an empty status array, and its workflow-run action only exposes PR runs. This means NOT INSPECTABLE, not PASS or confirmed PENDING. No exact-head result is invented.
- The new WAN router change requires exact-main Process regression and XY-C1 production-source Build/TRX/artifact validation. No MIPS target build or physical-router L4 result is claimed.
- The prior Adbyby exact-main Actions should also be verified when push-run listing is available; its source result must not be conflated with the older PASS evidence recorded in history.
- SmartDNS XY-C1 run `45336719186` on `2294ae89092db384169b533ed6c0ab4e8c6b639c` was last recorded as IN_PROGRESS; check its final Build/TRX/artifact only if the run remains accessible and do not infer L3.

## Exact first action for the next GPT

1. Fetch the live `main` SHA and inspect exact-head Push Actions for THIS COMMIT; triage the first failure or record the run IDs/head SHA if still pending.
2. Confirm XY-C1 Build, TRX verification and artifact upload on this production-source SHA before claiming L3; separately inspect the unresolved Adbyby exact-main checks.
3. Continue a concrete P1 stability repair outside already-completed SmartDNS/RAETH scopes, preferably from the WAN/DHCP, DNSMasq, storage or service-lifecycle backlog; do not re-audit resolved items.
4. Keep `backup/pre-plugin-overhaul-2026-11-14` immutable; all source updates remain single-tree atomic commits on `main` using expected-SHA non-forced updates.

# Cross-session execution mandate

Treat this as continuous work across GPT conversations. Recover once, then execute on `main`; each logical source or CI change is one atomic main commit. Create all changed blobs, one complete Git tree and one commit before updating the branch with a non-forced expected-SHA compare-and-swap. Never force-push or modify the frozen backup.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST perform a handoff checkpoint before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

Every session MUST inspect live repository state before ending. Record only material exact-head Action transitions, new source commits, genuine unresolved failures or a changed first action. Do not manufacture documentation churn or no-op commits.

## Development loop

- Source modification → targeted regression → one complete Git tree/one atomic commit → exact-SHA Actions.
- PASS advances; FAIL is diagnosed and fixed; PENDING does not block independent work.
- Preserve Linux 4.4, GNU89, MIPS/uClibc compatibility, router configuration, security, architecture and backward compatibility.
- Before context exhaustion, record exact latest SHA, tests, CI state, unresolved issues and one concrete first task. Never claim unverified work.
