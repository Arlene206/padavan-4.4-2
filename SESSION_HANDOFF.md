# Session Handoff

## Current repository state

- Latest main before this documentation checkpoint: `ae615df132c429ee58373bfa248be2df4d1e828d`; its parent production-source SHA is `a89ebc296b817403086e052b3f1fb4a52ed183b8`. The only change in `ae615` is a checker syntax correction in `scripts/check_adbyby_hosts_config.py`; it does not change firmware inputs.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`. No backup branch edits, force pushes or history rewrites were made.
- This session completed atomic fixes across startup and resolver state handling: Autostart's missing regression helper/fixture; AliyunDrive WebDAV invalid enable selector; LAN DHCP static-DNS selector validation; AliDDNS, DDNSTO, AdGuardHome, Adbyby and SmartDNS binary enable-selector validation. DDNSTO and Adbyby intermediate test/checker failures were diagnosed and corrected in follow-up commits.
- Main-only writes used complete Git trees and non-forced expected-SHA updates. All source and test slices are recorded at the top of `dev_log.md`.

## Active task

Poll the exact production-source XY-C1 run `#45335248142` for SHA `a89ebc296b817403086e052b3f1fb4a52ed183b8`. At the last inspection, its `Build XY-C1` step was still running; `Verify TRX output` and artifact upload were pending. Inspect those three steps independently and claim firmware L3 only if all complete successfully.

The checker-only successor `ae615df132c429ee58373bfa248be2df4d1e828d` has the Adbyby stability regression PASS; no product source changed after the `a89ebc` firmware input SHA. While firmware CI is pending, select a separate evidence-backed stability/security defect from the whole userland ledger rather than waiting or repeating completed audits.

## Completed development slices

- Autostart: supplied the missing `fail()` helper and corrected the concurrent-publish fixture newline. Process lifecycle Actions #45332907096 and #45332907032 PASS on `6bc7b0665719835a6f45e54c3dfd13e3516f6b73`.
- AliyunDrive WebDAV: only explicit disabled values stop the service; malformed enable values fail without killing/relaunching a healthy daemon. Lifecycle #45333513857 and Process #45333513772 PASS on `5cfb70929934a800f867a7e21179e11fafb9d86c`.
- LAN DHCP: strict binary validation for `lan_dns_x` before resolver publication and deconfig route/resolver side effects; GNU89 policy test includes boundary/corrupt values. WAN DHCP callback stability #45333806032 PASS on `d0c7bb76c70f23b18046719d1d8ede572dfca05b`.
- AliDDNS: malformed persisted and runtime selectors no longer become implicit enable/disable decisions or terminate the keeper. Exact lifecycle #45334063814 and Process #45334063878 PASS on `248e045b1526868c9324485bfb18234ca268c7cf`.
- DDNSTO: start/restart preflight validates checked NVRAM values before signalling a healthy daemon; fixture restore correction made its selector matrix pass. Lifecycle #45334345903 and Process #45334345858 PASS on `10ca6cb384a9f83864216f5302d5f44ea59fd0cf`.
- AdGuardHome: restart preflights checked enable state before stopping; invalid values/read errors leave the daemon untouched. Config/security #45334530232 and Process #45334530159 PASS on `e3d7f4b0f347f01da2cf4a84e58bee511934ea41`.
- Adbyby: start/restart reject malformed enable selectors and preserve NVRAM read error statuses. Intermediate exact-head failures exposed, in order, a multiline function extraction mismatch, lost read status, and a checker IndentationError; these were corrected in `053393ee`, `560d78e` and `ae615df`. Final Adbyby download/DNSMasq regression #45335321139 PASS on `ae615df132c429ee58373bfa248be2df4d1e828d`.
- SmartDNS: start/restart/reset accept only the binary UI selector (with legacy empty default) before service-state changes; stop remains available for shutdown. DNSMasq lifecycle #45335248129, Process inventory #45335248118 and AI handoff contract #45335248114 PASS on production SHA `a89ebc296b817403086e052b3f1fb4a52ed183b8`.
- USB storage hotplug regression #45335248143 also PASS on `a89ebc`.

## Verification boundary

- Targeted source/test work above is backed by the listed exact-SHA GitHub Actions results. The intermediate DDNSTO and Adbyby failures are historical and have passing corrections on descendant SHAs.
- Latest production-input SHA `a89ebc296b817403086e052b3f1fb4a52ed183b8`: XY-C1 run #45335248142 remains IN_PROGRESS at last poll (Build running; TRX and artifact upload pending). Older XY-C1 runs do not establish validation for this production SHA. Do not claim L3 until Build, TRX verification and artifact upload all pass on #45335248142.
- No local MIPS firmware compile was run in this session. Physical-router/hardware L4 remains NOT VERIFIED.
- AI handoff contract #45335248114 passed on the preceding handoff state; the current documentation checkpoint requires its own exact-SHA run.

## Exact first action for the next GPT

1. Fetch live main and poll XY-C1 run #45335248142 for production SHA `a89ebc296b817403086e052b3f1fb4a52ed183b8`; inspect Build XY-C1, Verify TRX output and artifact upload separately.
2. Poll the AI handoff documentation contract and Process inventory checks triggered by this documentation checkpoint. If XY-C1 remains pending, continue a different evidence-backed userland stability/security slice without waiting.
3. Keep the frozen backup read-only; use complete trees, one atomic commit per logical task and non-forced expected-SHA updates.

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
