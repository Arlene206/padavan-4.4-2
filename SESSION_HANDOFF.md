## Current repository state

- Parent main for this Adbyby process-identity hardening slice: `ceda64cdd021684c8b9b876fb29ecafa150a9e81`; live branch/tree were fetched immediately before constructing the commit.
- Adbyby test/checker sequence: `c0afda5` moved TERM to an external stub; `3462bca` exported the multi-PID fixture; `8774bb3` corrected the checker to inspect pidof capture inside `adbyby_process_snapshot()`. Exact Adbyby #45337764822 and AI docs #45337762851 PASS on `8774bb3`.
- SmartDNS stop NVRAM-isolation SHA `2294ae89092db384169b533ed6c0ab4e8c6b639c`: DNSMasq #45336719172, Process #45336719252, AI docs #45336719249 and USB #45336719220 PASS; XY-C1 #45336719186 was still building at last poll (TRX/artifact pending).
- DNS Forwarder process-snapshot hardening SHA `569066e9f67b0fab62a8382c2b1b8e9ab9dfdc3f`: launch-path #45336969468 and Process #45336969414 PASS.
- This slice binds Adbyby's name-based `pidof` candidates to the installed executable and process start_time, with immediate pre-signal revalidation.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Main-only atomic complete-tree commits, expected-SHA updates and no force-push.

## Active task

Implement and verify Adbyby executable-bound process identity:
- `adbyby_process_identity()` validates `/proc/PID/exe`, parses `/proc/PID/stat` state/start_time robustly across a parenthesized comm field, and distinguishes unowned decoys from unreadable/UNKNOWN state.
- `adbyby_process_snapshot()` filters pidof candidates and records `PID:start_time`; `adbyby_stop_daemon()` revalidates both immediately before each SIGTERM.
- Host regressions cover two valid PIDs, same-name wrong-executable decoys, malformed metadata and start_time replacement. Source checker pins the identity contract.
- Exact-head Adbyby host and Process Actions must be inspected after publication.

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

- Prior checker-correction SHA `8774bb3d2c5524dc7e427d6c2b8dc25df9f531fb`: Adbyby host #45337764822 PASS and AI docs #45337762851 PASS. Those runs do not validate the new process-identity changes in THIS COMMIT.
- SmartDNS `2294ae8`: DNSMasq, Process, AI docs and USB all PASS; XY-C1 #45336719186 remained IN_PROGRESS at the last detailed poll (Build running; TRX verification and artifact upload pending).
- No local MIPS firmware build, physical-router runtime or L4 result claimed.

## Exact first action for the next GPT

1. Inspect this process-identity commit's exact-head Adbyby host and Process inventory/fail-closed Actions; diagnose any first failing step and repair it.
2. Poll SmartDNS XY-C1 #45336719186 on `2294ae89092db384169b533ed6c0ab4e8c6b639c`; claim L3 only after Build, TRX verification and artifact upload all pass on that SHA.
3. Continue a concrete P1 stability/security fix outside completed SmartDNS/RAETH scope, without repeating selector audits.
4. Keep the backup branch immutable and main updates atomic/non-forced.

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
