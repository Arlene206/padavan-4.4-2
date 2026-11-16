# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state and compare it with `SESSION_HANDOFF.md` before ending or handing off. Verify the engineering/source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave the file untouched and make no no-op commit.

Before context exhaustion, finish or reduce the current safe slice, check exact-head Actions, commit any completed work atomically and preserve the precise first next action here.

Continue this engineering task across ChatGPT conversations. Recover repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup. Record exact Action results and the next concrete task before context exhaustion.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; active branch: main.
- Parent for THIS bridge membership ioctl errno slice: `1ce4ffe3b71e87bdc5143ccc802d252f190ab5b0`; fetch live main before the next write and use expected-SHA non-forced CAS.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. One-time startup comparison found main 588 commits ahead / 1 behind; do not repeat backup/archive recovery.

## Active task

The prior slice fixed `br_add_del_bridge()` preserving ioctl errno across close. Sibling audit found `br_add_del_if_internal()` had the same hole for SIOCBRADDIF/SIOCBRDELIF:
- After a failed add/delete ioctl, a non-idempotent errno could be overwritten by `close(sockfd)`, so callers receive an unreliable cause and duplicate-race decisions/logging lose accurate failure information.
- Now save errno immediately after the bridge member ioctl and restore it after close whenever the operation remains failed. If the idempotency re-probe proves the requested membership is already satisfied, the operation still returns success.
- Process source contract now checks the shared bridge-create and bridge-member helpers both save and restore ioctl errno across close.

Previous reliability fixes remain on main: first-error bridge/VLAN/switch startup aggregation, required iNIC module verification, optional hw_nat offload verification, full WAN restart fail-closed switch VLAN status.

## Verification boundary

On exact parent SHA `1ce4ffe3b71e87bdc5143ccc802d252f190ab5b0`, runs were pending at last query:
- Process inventory and lifecycle fail-closed `43860541389`
- NVRAM transaction boundary audit `43860541296`
- AI handoff documentation contract `43860541304`
- WAN DHCP callback `43860541545`
- AliDDNS signed transport `43860541276`
- AliyunDrive WebDAV `43860541216`
- AdGuardHome config security `43860541356`
- infosvr packet/signal `43860541489`
- Wireless radio state `43860541030`
- WAN route result `43860541365`
- Process lifecycle inventory `43860541379`.
On the previous exact source SHA `7ce997801add3b30e42cd1cc76f14ddc0c45ef5c`, Process `43860044486` and AI docs `43860044408` passed; HTTPD auth `43860044873`, WAN route `43860044772`, DNSMasq `43860044694`, VPN `43860044807`, AdGuardHome `43860044911`, AliDDNS `43860044474`, detect_link `43860044785`, detect_internet `43860044834`, infosvr `43860044473`, WAN DHCP `43860044830`, secondary Process inventory `43860044744` and USB modem identity `43860044356` all passed. XY-C1 `43860044821` remained in progress at last inspection; prior NVRAM `43860044318` subsequently passed.
- No same-SHA XY-C1 Build+TRX+artifact L3 or physical-router L4 has been established.

## Exact first action for the next GPT

1. Inspect exact-head Process and AI docs for this commit and return to prior pending XY-C1 run `43860044821`; check remaining service/security Actions without waiting idly.
2. Verify both bridge ioctl errno source assertions and existing bridge-init/member policy regressions pass.
3. Continue source-backed reliability audit of shared network ioctl wrappers and callers; preserve error contracts across cleanup calls.
4. XY-C1 L3 requires Build, TRX verification and artifact upload on the same source SHA; hardware behavior remains L4-only.

## Verification and governance

Preserve security, architecture, compatibility, target toolchain and mandatory tests. Main only; frozen backup read-only. One full Git tree and one atomic commit per logical task; non-forced expected-SHA update. Update `dev_log.md` for source milestones and retain 20–119 entries. Distinguish host tests, exact-SHA Actions, firmware/TRX L3 and physical router L4. Never claim an unrun result.
