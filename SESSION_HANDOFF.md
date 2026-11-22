# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state and complete an end-of-session accuracy checkpoint. Record exact main SHA, exact-SHA Actions, test/build boundaries, and the next precise task. If facts remain accurate, explicitly conclude with no file change and no no-op commit. Use one atomic main commit for material changes only. Before context exhaustion, leave an accurate handoff and complete any safe bounded engineering slice.

- Work only on `main`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update `main` only with `force:false` and a freshly fetched expected SHA.
- Check Actions for the exact SHA. PASS continues; FAIL requires diagnosis/correction; PENDING does not block independent work.
- Do not claim local tests, target builds, artifacts or hardware checks that were not performed.

## Current repository state

- Latest parent main SHA: `af2144472068de1d02c5067d968958bdbc926d62`; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- The HTTP readiness parser used escaped dollar signs and compared literal variable placeholders; the production launcher and contract now use real POSIX parameter expansion. The 200/401/503/missing-listener host cases already exist and must rerun on the current exact SHA.

- Repository: `Arlene206/padavan-4.4-2`; parent main SHA: `2144925dd276279d7113dd1e5c16d7855202bcde`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807` remains immutable.
- p910nd I/O EINTR/EAGAIN/zero-write fix committed in 7a136bb; exact GNU89/ASan/UBSan/source-contract Action #45041698560 PASS.
- EAPoL orphan source tail removed in efb830b; Process #45041387825, EAPoL #45041387869, NVRAM #45041387886 and AI #45041387973 PASS on efb; XY-C1 #45041387818 was building at last check.
- WSDD2 strict UTF-8/XML 1.0 validation and ttyd detached-child readiness remain checked/green on exact descendants e2 and 30a1.

## Active task

AliyunDrive WebDAV previously treated validated process presence as service readiness. It now requires the daemon process plus a bounded HTTP HEAD response from the configured host/port. Startup and duplicate-start idempotence no longer claim success when the daemon remains alive but its HTTP listener is unavailable or returns server errors.

## Verification boundary

- Parent SHA 2b957b311d18f9662a3c0044ca2e15403c2a544a: p910nd #45041698560 PASS; Process inventory #45041698513 PASS; AI docs #45041698520 PASS; XY-C1 #45041698556 was building; NVRAM audit #45041698491 running.
- EAPoL source parent efb830bce5028536826989c8b5070b68a49e4a4e: Process #45041387825 PASS, EAPoL #45041387869 PASS, NVRAM #45041387886 PASS, AI #45041387973 PASS; XY-C1 #45041387818 was building. No L3 claim until Build/TRX/artifact all pass on one exact product source SHA.
- The AliyunDrive HTTP readiness change has not yet run its exact lifecycle Action or XY-C1 build. No local tests or firmware image were run.
- The HTTP probe uses BusyBox NC (CONFIG_NC=y); MIPS/uClibc runtime and physical-router L4 remain NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head AliyunDrive WebDAV lifecycle regression, Process inventory and AI docs after the HTTP readiness change; correct only demonstrated failures.
2. Inspect same-SHA XY-C1 Build, TRX verification and artifact upload for the EAPoL cleanup and new AliyunDrive source; do not infer L3 from pending runs.
3. Continue another independent userland stability/security slice beyond completed p910nd, EAPoL, ttyd, WSDD2, WAN/PPP, Inadyn and DNSMasq work. Keep backup read-only and work only on main.
