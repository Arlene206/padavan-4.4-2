# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state and compare it with `SESSION_HANDOFF.md` before ending or handing off. Verify the engineering/source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave the file untouched and make no no-op commit.

Before context exhaustion, finish or reduce the current safe slice, check exact-head Actions, commit any completed work atomically and preserve the precise first next action here.

Continue this engineering task across ChatGPT conversations. Recover repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup. Record exact Action results and the next concrete task before context exhaustion.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; active branch: main.
- Parent for THIS bridge ioctl errno-preservation slice: `7ce997801add3b30e42cd1cc76f14ddc0c45ef5c`; fetch live main and use expected-SHA non-forced CAS on the next write.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. One-time startup comparison found main 588 commits ahead / 1 behind; do not repeat archive recovery.
- Latest LAN/switch module status source baseline: `b715097b8fdb76ed1e96bdeed6bedafde47eeefd`.
- Latest WAN restart switch-VLAN fail-closed fix: `7ce997801add3b30e42cd1cc76f14ddc0c45ef5c`.

## Active task

The new bridge initializer classifies `br_add_del_bridge()` EEXIST using `errno`, then verifies the interface is an existing bridge. The shared helper performed its ioctl and then called `close(sockfd)` before returning without preserving the ioctl error number. A failed close could overwrite the original ioctl `errno`, preventing correct EEXIST classification (or changing the reported failure). This slice saves `errno` immediately after ioctl and restores it after close whenever the ioctl failed.

Process regression now inspects the production helper and asserts that the ioctl errno is saved before close and restored on failure. Existing bridge idempotency policy tests exercise EEXIST/other-error/unknown-inventory cases.

## Verification boundary

On source SHA `7ce997801add3b30e42cd1cc76f14ddc0c45ef5c`:
- Process inventory and lifecycle fail-closed regression `43860044486`: SUCCESS.
- AI handoff documentation contract `43860044408`: SUCCESS.
- HTTPD Authentication and Security `43860044873`: SUCCESS.
- WAN route operation status `43860044772`: SUCCESS.
- WAN interface teardown `43860044356`: SUCCESS.
- AliDDNS signed API transport `43860044474`: SUCCESS.
- AdGuardHome configuration security `43860044911`: SUCCESS.
- detect_link `43860044785`, detect_internet `43860044834`, DNSMasq `43860044694`, VPN `43860044807`, infosvr `43860044473`, USB modem identity `43860044356`, WAN DHCP callback `43860044830`, and secondary Process inventory `43860044744`: SUCCESS.
- XY-C1 Firmware Validation `43860044821` and NVRAM transaction boundary audit `43860044318`: IN_PROGRESS at last inspection; check exact current results, do not assume.
- No same-SHA XY-C1 Build+TRX+artifact L3 or physical-router L4 was established at last inspection.

## Exact first action for the next GPT

1. Inspect exact-head Process and AI handoff checks for this errno fix; check whether the prior XY-C1 run `43860044821` and NVRAM run `43860044318` have completed.
2. Verify Process source contract and bridge policy regression, then continue the RC/shared network utility callsite audit for other newly status-bearing APIs or reliable errno propagation defects.
3. Record XY-C1 L3 only when Build, TRX validation and artifact upload pass on the same production-source SHA. Physical router verification remains L4-only.

## Verification and governance

Preserve security, architecture, compatibility, target toolchain and necessary tests. Main only; frozen backup read-only. One full Git tree and atomic commit per logical task, non-forced expected-SHA update. Update `dev_log.md` for source milestones and retain 20–119 entries. Distinguish host regression, exact-SHA CI, firmware/TRX L3 and physical router L4. Never claim an unrun result.
