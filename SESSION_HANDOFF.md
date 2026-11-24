## Current repository state

- Parent main for this Adbyby syntax/postcondition correction: `db75aed4c9005bcbdcb4eba503b3c65be8f99644`; live main/tree fetched before building the complete tree.
- Adbyby process identity code is in `db75aed4c9005bcbdcb4eba503b3c65be8f99644`. Its first exact-head run failed shell syntax; correction `5e9ce769a1f274d99da21686f3b9c0f9c7b43a11` restored the bounded wait loop and corrected fake procfs start_time offsets; exact reruns are pending. Exact Adbyby #45338516192 failed at `sh -n` due an accidentally omitted closing brace and bounded wait tail; exact HTTPD #45338516101 and Process fail-closed #45338516189 failed at dependent Adbyby source checks. DNSMasq #45338516248, AI docs #45338516151 and Process inventory #45338516089 PASS on DB75.
- THIS COMMIT restores the bounded five-probe post-TERM postcondition The resulting exact 5e9ce769 test runs confirmed the stop shell regression PASS but exposed two follow-ups: a checker phrase mismatch and a start-readiness fixture that needed synthetic procfs. THIS COMMIT fixes both. and fixes fake procfs stat layout so the PID-reuse test modifies kernel start_time field 22, not vsize.
- SmartDNS NVRAM-independent stop SHA `2294ae89092db384169b533ed6c0ab4e8c6b639c`: DNSMasq, Process, AI docs and USB passed; its XY-C1 run was still in progress at last poll.
- AdGuardHome per-PID stop/escalation fix `ceda64c`: exact `ce1902a` shell regressions all PASS after resetting the mock PID between scenarios; Python checker then failed only because the test omitted its required success marker. THIS COMMIT emits that marker after the full assertions; exact-head rerun pending.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Main-only atomic complete-tree commits; expected-SHA non-forced updates; no force-push.

## Active task

Close Adbyby process identity regression:
- Inventory binds pidof candidates to `/proc/PID/exe` and `/proc/PID/stat` start_time.
- Stop revalidates executable/start_time before direct TERM, then retains the five-bounded-probe RUNNING/ABSENT/UNKNOWN postcondition before DNS/firewall cleanup.
- Fake procfs fixtures put start_time in field 22; the PID-reuse test changes that exact field.
- Exact-head Adbyby, HTTPD and Process fail-closed Actions are required after this correction.

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

- AdGuardHome #45338988643 on `ce1902a`: shell syntax and lifecycle assertions PASS; only the Python checker marker was missing. THIS COMMIT adds the marker after stop regressions. New exact-head run pending.
- AdGuardHome #45338317817 on `ceda64c`: shell tests passed through the fresh replacement-PID case, then the stuck-daemon expected PID mismatched because test state leaked 13579; THIS COMMIT resets the fixture. New exact-head AdGuardHome rerun pending.
- DB75 exact head: Adbyby #45338516192 FAILED at shell syntax (`adbyby.sh:1891: Syntax error: end of file unexpected (expecting "}")`). HTTPD #45338516101 failed at dependent add-on staging syntax; Process fail-closed #45338516189 failed its Adbyby stop-gate check. DNSMasq #45338516248 PASS; AI docs #45338516151 PASS; Process inventory #45338516089 PASS.
- 5e9ce769 exact head: Adbyby shell syntax and host regressions PASS; its Python checker FAILED only on a `start-time` vs `start_time` marker mismatch. Process fail-closed FAILED because the detached-start fixture lacked synthetic procfs after the new executable-identity check. THIS COMMIT fixes both test-only causes.
- Prior correction 5e9ce769 restored the function brace/wait loop and corrects the procfs test field offset. New exact-head reruns pending.
- XY-C1 DB75 #45338516092 remained IN_PROGRESS at the last poll; no same-SHA L3 claim until Build, TRX and artifact all pass. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect the newest exact-head Adbyby/HTTPD/Process runs for `5e9ce769` and AdGuardHome config-security for this PID-fixture reset; fix any remaining first failure.
2. Poll DB75 XY-C1 #45338516092 for final Build, TRX and artifact-upload outcomes; claim L3 only if every required step passes on DB75's product source.
3. If regressions pass or are pending, continue an evidence-backed P1 stability/security task outside already-completed SmartDNS/RAETH coverage.
4. Keep the backup immutable and main commits atomic/non-forced.

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
