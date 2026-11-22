# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state and complete an end-of-session accuracy checkpoint. Record exact main SHA, exact-SHA Actions, test/build boundaries, and the next precise task. If facts remain accurate, explicitly conclude with no file change and no no-op commit. Use one atomic main commit for material changes only. Before context exhaustion, leave an accurate handoff and complete any safe bounded engineering slice.

- Work only on `main`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update `main` only with `force:false` and a freshly fetched expected SHA.
- Check Actions for the exact SHA. PASS continues; FAIL requires diagnosis/correction; PENDING does not block independent work.
- Do not claim local tests, target builds, artifacts or hardware checks that were not performed.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; parent main SHA: `2b957b311d18f9662a3c0044ca2e15403c2a544a`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807` remains immutable.
- EAPoL orphan-tail compile fix is present in `efb830bce5028536826989c8b5070b68a49e4a4e`; p910nd EINTR/EAGAIN/zero-progress handling passed on `2b957b311d18f9662a3c0044ca2e15403c2a544a`.
- RT3352 MII TxPower now uses checked NVRAM reads and fails closed before driver programming/unisolation/publication. The source policy binaries passed on `2b957b311d18f9662a3c0044ca2e15403c2a544a`; the first Wireless run failed only because an inline source-contract substring also matched a comment. The contract is corrected in the current commit.

## Active task

Rerun Wireless radio state regression on this exact commit and inspect the XY-C1 firmware run inherited from parent `2b957b311d18f9662a3c0044ca2e15403c2a544a`. Then continue an independent confirmed userland stability/security issue while build/checks are pending.

## Verification boundary

- Parent `2b957b311d18f9662a3c0044ca2e15403c2a544a`: NVRAM audit #45041943056 PASS; Process inventory #45041943005 PASS; Process umbrella #45041942953 PASS; HTTPD security #45041942956 PASS; iNIC signal #45041942917 PASS; watchdog signal #45041942875 PASS; AI handoff #45041942863 PASS.
- Parent Wireless #45041942952: compiled and passed three C policy binaries, then FAILED in the inline Python source contract because `nvram_wlan_get_int(` was found in a comment. Narrowed the check to the actual unsafe call in this commit; production source is unchanged. Exact rerun required.
- Parent XY-C1 #45041942984 was IN_PROGRESS at the last poll; inspect Build, TRX and artifact stages on that same SHA.
- No local build or hardware validation was performed. L3 requires Build XY-C1, TRX verification and artifact upload all PASS on one identical production source SHA. Hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Check exact-head Wireless and AI handoff Actions for this checker-only correction, then inspect parent `2b957b311d18f9662a3c0044ca2e15403c2a544a` XY-C1 Build/TRX/artifact results.
2. Fix any actual failing stage at its exact SHA; do not infer product readiness from host policy tests.
3. Continue a bounded stability/security slice outside current wireless, EAPoL, p910nd, WAN/PPP, Inadyn, DNSMasq, ttyd and WSDD2 work. Keep backup immutable and develop only on main.
