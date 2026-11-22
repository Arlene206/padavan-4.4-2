# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository and complete an end-of-session accuracy checkpoint before ending. Record current source SHA, exact-SHA Actions, test/build boundaries, and the next precise task. If facts and first action remain accurate, explicitly conclude the check with no file change and no commit. Use one atomic main commit for material changes only; never create a no-op commit. Before context exhaustion, leave the next GPT a precise first action and truthful verification status.

- Work only on `main`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update `main` only with `force:false` and a fetched expected SHA.
- Check Actions for the exact source SHA. PASS continues; FAIL requires diagnosis/correction; PENDING does not block independent work.
- Do not claim local tests, target builds, artifacts or hardware checks that were not performed.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; branch `main`.
- Current source baseline: `9a1d4e442da2af1d64cef9397fcd973386831617`. DHCPv6 status getters and pre-stage NVRAM snapshot landed in `e100bb3b88a68e3db6e23721bf6852631126895e`; later commits align its regression/source contracts.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. Legacy PR #1 is closed/unmerged.
- WAN PPP launch mode uses a checked NVRAM transaction snapshot and immutable flags for lock selection, daemon stop/start, multi-WAN startup and rollback. Invalid WAN unit inputs return deterministic EINVAL.
- Modem WAN link-change reconnect uses checked modem readiness/WISP state; Inadyn startup checks DDNS enable state before stopping the existing daemon.
- DNSMasq resolver flags, DHCP integers, static-host snapshots, domain validation, resolver-file creation and lifecycle publication are covered by its workflow; this slice adds DHCPv6 status/snapshot preflight.

## Active task

The DHCPv6 DNSMasq path now uses status-returning LAN6 APIs for RA state, DHCPv6 mode and IRT, plus a checked one-generation snapshot for DNS address and stateful lifetime/interface-ID bounds before staged config open. Its GNU89/ASan/UBSan regression and source contract passed on `69d316e7909fa2143b12e786260d61ed9cb6cf4a`. The remaining current task is to get XY-C1's embedded source contract/build flow green with the updated status-bearing getter.

## Verification boundary

- Local C/Python tests were not run in this connector-only session.
- Source SHA `e100bb3b88a68e3db6e23721bf6852631126895e`: WAN route #45034779732, WAN DHCP #45034779748, USB modem #45034779746, HTTPD security #45034779752, USB storage #45034779678, FRP #45034779741, Samba #45034779738, NFS #45034779726, miniupnpd #45034779731, vsftpd #45034779757, Inadyn #45034779676, process inventory #45034779974 and NVRAM audit #45034779584 PASS. DNSMasq #45034779769 and IPv6 #45034779684 initially failed only in source-checker assertions while their GNU89/ASan/UBSan policy binaries passed; both checker failures were corrected in `69d316e7909fa2143b12e786260d61ed9cb6cf4a`.
- Exact checker-fix SHA `69d316e7909fa2143b12e786260d61ed9cb6cf4a`: DNSMasq #45035039314 PASS; IPv6 service-type #45035039316 PASS; AI handoff #45035039283 PASS. XY-C1 #45034779614 on parent `e100bb3...` failed at the embedded LAN IPv6 mode getter assertion before Build XY-C1; TRX and artifact upload were skipped. This workflow contract is corrected in this commit.
- Prior SHA `7c6ab191bad7e18ec65c8bb8951713dbc24ee3f8` did complete XY-C1 #45033514004 successfully: Build XY-C1, TRX output verification and artifact upload all PASS on that exact older SHA. This does not certify descendants.
- Earlier PPP and Inadyn runs below are historical evidence only; they do not certify current main.
- No router hardware validation or L4 result is claimed. XY-C1 L3 requires Build, TRX verification and artifact upload all PASS on one identical SHA.

## Exact first action for the next GPT

1. Inspect exact-SHA XY-C1 rerun for the updated embedded DHCPv6 getter contract; require Build XY-C1, TRX and artifact upload all PASS on that same SHA.
2. While target build is running, continue the broad userland stability/security rotation outside WAN/PPP, USB modem, Inadyn and DNSMasq.
3. Keep the backup immutable. XY-C1 L3 requires Build, TRX validation and artifact upload all PASS on one SHA; L4 requires physical device verification.
