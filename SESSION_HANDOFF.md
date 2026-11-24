## Current repository state

- Parent main for this source/test slice: `08d59c49ef1080079830e66302516747e06cf82f`; the live tree was fetched immediately before the atomic tree was assembled.
- Since the preceding handoff, Transmission shutdown and reload were changed to signal only fully validated PID snapshots (`abad0ae5`, `2d87291`). The exact-head storage suite exposed a stale shared test double that accepted only `-9`; commit `cf1850c724c2623e7c0aead2d46d82fc50a320c5` corrected it to model Transmission's per-PID SIGTERM separately from Aria2's killall path. Its USB hotplug and related exact-head Actions were pending at last poll.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- This continuation closes a confirmed SmartDNS shutdown failure boundary: transient checked NVRAM read failures previously exited before explicit stop could restore persisted DNSMasq state or stop SmartDNS. The new helper and targeted regression are in THIS COMMIT.
- All writes remain main-only, complete-tree, single-commit and non-forced expected-SHA updates.

## Active task

Implement and verify the SmartDNS stop NVRAM isolation slice:
- Production: `trunk/user/smartdns/smartdns.sh`, `smartdns_load_lifecycle_nvram_state()`.
- Regression: `trunk/user/smartdns/tests/smartdns_stop_nvram_failure.sh`; source checker: `scripts/check_smartdns_dnsmasq_config.py`.
- Workflow: `.github/workflows/dnsmasq-lifecycle.yml` (push and pull-request trigger sets include the regression).
- Explicit `stop` skips all five startup configuration reads, selects the disabled cleanup branch and relies on `Read_ini`'s persisted resolver snapshot. Start/restart/reset retain fail-closed checked reads, original error status and strict binary enable validation.
- After publication, inspect exact-SHA DNSMasq lifecycle, Process lifecycle and XY-C1 results. Do not claim L3 unless Build, TRX verification and artifact upload all pass on this same source SHA.

## Completed development slices

- Autostart: supplied the missing `fail()` helper and corrected the concurrent-publish fixture newline. Process lifecycle Actions #45332907096 and #45332907032 PASS on `6bc7b0665719835a6f45e54c3dfd13e3516f6b73`.
- AliyunDrive WebDAV: only explicit disabled values stop the service; malformed enable values fail without killing/relaunching a healthy daemon. Lifecycle #45333513857 and Process #45333513772 PASS on `5cfb70929934a800f867a7e21179e11fafb9d86c`.
- LAN DHCP: strict binary validation for `lan_dns_x` before resolver publication and deconfig route/resolver side effects; GNU89 policy test includes boundary/corrupt values. WAN DHCP callback stability #45333806032 PASS on `d0c7bb76c70f23b18046719d1d8ede572dfca05b`.
- AliDDNS: malformed persisted and runtime selectors no longer become implicit enable/disable decisions or terminate the keeper. Exact lifecycle #45334063814 and Process #45334063878 PASS on `248e045b1526868c9324485bfb18234ca268c7cf`.
- DDNSTO: start/restart preflight validates checked NVRAM values before signalling a healthy daemon; fixture restore correction made its selector matrix pass. Lifecycle #45334345903 and Process #45334345858 PASS on `10ca6cb384a9f83864216f5302d5f44ea59fd0cf`.
- AdGuardHome: restart preflights checked enable state before stopping; invalid values/read errors leave the daemon untouched. Config/security #45334530232 and Process #45334530159 PASS on `e3d7f4b0f347f01da2cf4a84e58bee511934ea41`.
- Adbyby: start/restart reject malformed enable selectors and preserve NVRAM read error statuses. Intermediate exact-head failures exposed, in order, a multiline function extraction mismatch, lost read status, and a checker IndentationError; these were corrected in `053393ee`, `560d78e` and `ae615df`. Final Adbyby download/DNSMasq regression #45335321139 PASS on `ae615df132c429ee58373bfa248be2df4d1e828d`.
- SmartDNS: start/restart/reset accept only the binary UI selector (with legacy empty default); this slice makes explicit stop independent of all five checked config NVRAM reads and restores from the persisted resolver snapshot. Exact-head checks for this slice are pending.
- USB storage hotplug regression #45335248143 also PASS on `a89ebc`.

## Verification boundary

- Exact-head Actions for this new SmartDNS source SHA are pending immediately after publication; DNSMasq lifecycle, Process lifecycle and XY-C1 must be inspected independently. AI handoff documentation status must also be checked if triggered.
- The preceding Transmission/storage test-correction SHA `cf1850c724c2623e7c0aead2d46d82fc50a320c5` had USB hotplug #45336564093, XY-C1 #45336564159, Process inventory #45336564109 and AI handoff #45336564080 all IN_PROGRESS at last poll. The prior-head USB failure #45336245210 was caused by the stale Transmission SIGTERM test double and is corrected in the newer source tree; the new exact-head run must confirm it.
- No local MIPS firmware compile is claimed. Physical-router/hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main` and inspect this commit's exact-head DNSMasq lifecycle and Process lifecycle runs; diagnose any first failing step before recording verification.
2. Inspect the XY-C1 run for the exact product-source SHA; claim L3 only after Build XY-C1, TRX verification and artifact upload all pass.
3. Once these are closed or pending, continue an evidence-backed P1 stability pass outside SmartDNS and RAETH, starting with WAN DHCP / route lifecycle and PPPoE-MWAN error propagation. Do not repeat completed selector audits.
4. Keep the frozen backup immutable; preserve main-only atomic commits.

# Cross-session execution mandate

Treat this as continuous work across GPT conversations. Recover once, then execute on main; each logical source or CI change is one atomic main commit. Create all changed blobs, one complete Git tree and one commit before updating the branch with a non-forced expected-SHA compare-and-swap. Never force-push or modify the frozen backup.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST perform a handoff checkpoint before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

Every session MUST inspect live repository state before ending. Record only material exact-head Action transitions, new source commits, genuine unresolved failures or a changed first action. Do not manufacture documentation churn or no-op commits.

## Development loop

- Source modification → targeted regression → one complete Git tree/one atomic commit → exact-SHA Actions.
- PASS advances; FAIL is diagnosed and fixed; PENDING does not block independent work.
- Preserve Linux 4.4, GNU89, MIPS/uClibc compatibility, router configuration, security, architecture and backward compatibility.
- Before context exhaustion, record exact latest SHA, tests, CI state, unresolved issues and one concrete first task. Never claim unverified work.
