## Current repository state

- Parent main for this Adbyby source-contract correction: `3462bcacba47114b93d3d9da0366513064c13dbb`; live branch/tree read immediately before assembling the atomic commit.
- SmartDNS NVRAM-independent stop slice: source SHA `2294ae89092db384169b533ed6c0ab4e8c6b639c`; DNSMasq #45336719172, Process inventory #45336719252, AI docs #45336719249 and USB hotplug #45336719220 PASS. XY-C1 #45336719186 remains IN_PROGRESS at last poll; Build completed but TRX verification and artifact upload are still pending.
- DNS Forwarder CWD-safe launch/stop tests: exact SHA `569066e9f67b0fab62a8382c2b1b8e9ab9dfdc3f`, launch-path #45336969468 and Process inventory #45336969414 PASS.
- Adbyby per-PID stop test progression: `c0afda5` moved TERM injection to an external stub; `3462bca` exported the multi-PID fixture to the external `pidof` process. On `3462bcacba47114b93d3d9da0366513064c13dbb`, every shell regression (including the two-PID signal case) passed; only the source checker then failed because it searched the pidof assignment in the wrong function. THIS COMMIT corrects that checker.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- All commits remain main-only, one complete Git tree per logical change, CAS expected-SHA updates and no force-push.

## Active task

Close the Adbyby exact-head validation slice:
- The production code validates the result of `pidof adbyby` inside `adbyby_process_snapshot()`; `adbyby_process_state()` only maps its return code to RUNNING/ABSENT/UNKNOWN.
- The shell suite's multiple validated PID regression now exports `ADBYBY_TEST_PID_OUTPUT=123 456` before invoking the pidof subprocess. The external fake signal command prevents the shell `kill` builtin from hijacking per-PID TERM testing.
- This commit moves the source-checker assertion to the function that actually performs the checked pidof read. Inspect exact-head Adbyby and Process Actions after publication, fix any remaining failure, then inspect SmartDNS XY-C1 L3 steps.

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

- Exact SHA `3462bcacba47114b93d3d9da0366513064c13dbb`: Adbyby host regression #45337601785 failed only after all shell tests passed, at the stale Python source-checker assertion (pidof status capture wrongly searched in `adbyby_process_state`). THIS COMMIT fixes its function scope; exact-head rerun pending.
- Exact SHA `3462bca`: Process lifecycle inventory #45337601601 and Process inventory/fail-closed #45337601694 were still IN_PROGRESS at last poll.
- SmartDNS SHA `2294ae89092db384169b533ed6c0ab4e8c6b639c`: DNSMasq/Process/AI/USB all PASS; XY-C1 #45336719186 was IN_PROGRESS at last poll with TRX verification and artifact upload pending. Do not claim L3 until all required same-SHA build steps pass.
- No local MIPS firmware build, physical router runtime or L4 result claimed.

## Exact first action for the next GPT

1. Fetch live `main` and inspect this checker-correction commit's exact-head Adbyby and Process runs; address any first remaining failing step.
2. Poll SmartDNS XY-C1 #45336719186 on exact product-source SHA `2294ae89092db384169b533ed6c0ab4e8c6b639c`; claim L3 only if Build, TRX verification and artifact upload all pass.
3. Continue a concrete P1 stability fix outside SmartDNS/RAETH and avoid re-auditing completed selector work.
4. Keep the frozen backup immutable; all updates to main remain atomic and non-forced.

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
