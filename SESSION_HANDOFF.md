# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state and complete an end-of-session accuracy checkpoint. Record exact main SHA, exact-SHA Actions, test/build boundaries, and the next precise task. If facts remain accurate, explicitly conclude with no file change and no no-op commit. Use one atomic main commit for material changes only. Before context exhaustion, leave an accurate handoff and complete any safe bounded engineering slice.

- Work only on `main`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update `main` only with `force:false` and a freshly fetched expected SHA.
- Check Actions for the exact SHA. PASS continues; FAIL requires diagnosis/correction; PENDING does not block independent work.
- Do not claim local tests, target builds, artifacts or hardware checks that were not performed.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; parent main SHA: `7a136bbfd8fdb03b199cf85e534316a88a7d887a`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807` remains immutable.
- EAPoL compile-breaking orphan tail was removed in `efb830bce5028536826989c8b5070b68a49e4a4e`; p910nd stream I/O now handles EINTR/EAGAIN and zero-progress writes in parent `7a136bbfd8fdb03b199cf85e534316a88a7d887a`.
- This slice hardens the RT3352 MII TxPower preflight: a failed `rt_TxPower` read can no longer collapse to zero and authorize radio unisolation/publication.

## Active task

Validate the iNIC TxPower fail-closed correction on its exact source SHA. The previous `wireless_inic_radio_prepare_on()` called `nvram_wlan_get_int(0, "TxPower")`, whose safe-get/atoi wrapper silently maps NVRAM backend errors to zero. The new path uses checked `nvram_get_checked("rt_TxPower")`, preserves the missing/empty default and historic clamp, and rejects backend, malformed and out-of-range integer reads before any driver command. The existing transition policy must then maintain isolation and roll back power on preparation failure.

## Verification boundary

- Parent exact SHA `7a136bbfd8fdb03b199cf85e534316a88a7d887a`: p910nd I/O regression #45041698560 PASS, AI handoff #45041698520 PASS, Process inventory #45041698513 PASS. XY-C1 #45041698556 and NVRAM boundary #45041698491 were IN_PROGRESS at the last inspection.
- Parent EAPoL orphan-tail source correction is present; the previous d2 XY-C1 failure was at `auth.c` before the correction and is not current-SHA evidence.
- No local checkout, host execution or MIPS/uClibc build was available in this session. This wireless source/test/workflow commit requires exact-head Wireless and XY-C1 validation, plus AI and relevant Process/NVRAM results.
- L3 remains NOT VERIFIED until Build XY-C1, TRX verification and artifact upload all PASS on one identical source SHA. Hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main`; inspect this commit's Wireless radio state, XY-C1 and AI handoff Actions, including the precise XY-C1 Build/TRX/artifact stages. Diagnose the first causal failure and fix it.
2. Check the parent `7a136bbfd8fdb03b199cf85e534316a88a7d887a` XY-C1 #45041698556 and NVRAM #45041698491 outcomes as historical context only; they are not evidence for this new SHA.
3. Once the current wireless slice is validated, continue a different unfinished userland stability/security area beyond completed EAPoL, p910nd, WAN/PPP, Inadyn, DNSMasq, ttyd and WSDD2. Keep backup read-only and develop only on main.
