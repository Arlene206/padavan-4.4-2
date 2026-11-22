# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect live repository state and complete an end-of-session accuracy checkpoint. Record exact main SHA, exact-SHA Actions, test/build boundaries and the next precise task. If facts remain accurate, explicitly conclude with no file change and no no-op commit. Use one atomic main commit for material changes only. Before context exhaustion, leave an accurate handoff and complete any safe bounded engineering slice.

- Work only on `main`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update `main` only with `force:false` and a freshly fetched expected SHA.
- PASS continues, FAIL requires diagnosis/correction, PENDING does not block independent work.
- Do not claim local tests, production builds, TRX/artifacts or hardware checks that were not performed.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Parent main SHA before this atomic physical-eject parser slice: `4c6d1b9c4c0258641da90f7a1511b775a5fb70ea`.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; keep immutable. Only `main` and this backup branch exist.
- Exact-head USB storage run #45059907996 on `4c6d1b9c4c0258641da90f7a1511b775a5fb70ea` PASS, including BusyBox 1.24.2/ash, automount swap inventory, global cleanup GNU89 policy tests, caller graph, and safe-remove checker. AI handoff #45059908027 on the same SHA PASS.
- Process inventory #45059757436, HTTPD security #45059757425, fail-closed Process #45059757420 and Adbyby #45059757404 passed on ancestor production-source SHA `d37cf8aa903dbcfdfc5b4359976fe875b07f326f`. NVRAM audit #45059664659 passed on `abe7fc7160544a5d10a684ecb7c4645a3cc8152f`.
- XY-C1 run #45059757452 for product-source `d37cf8a` was still building XY-C1 at last check; TRX and artifact upload had not been confirmed. It cannot verify the new physical-eject parser header/test introduced here; a fresh exact-head XY-C1 run is required.
- The prior Storage failure #45059691298 on `7c22cb8` was a stale workflow string assertion for the space before the `swapon` semicolon; workflow-only commit `4c6d1b9` corrected it, and #45059907996 then passed all storage steps.
- No local checkout or router hardware is available. Device L4 remains NOT VERIFIED.

## Active task

The automount and global all-storage cleanup swap-inventory paths are now validated and passed exact Storage CI on `4c6d1b9c4c0258641da90f7a1511b775a5fb70ea`. A deeper physical-unplug trace found the same semantic validation gap in `trunk/user/rc/src/storage_ejected_inventory_policy.h::storage_ejected_parse_swap_line()`, which feeds `storage_stage_ejected_swaps()` before `swapoff()`: it previously accepted any five tokens, including unsupported Type values and malformed Size/Used/Priority columns.

This slice makes the physical-eject parser enforce Type exactly `partition` or `file`, unsigned decimal Size/Used and signed decimal Priority, with no extra fields. It retains `storage_ejected_next_token()` octal path decoding. The GNU89 ejected-inventory regression now rejects invalid type/numeric columns and accepts a swap-file path containing `\\040` with a positive signed priority. `scripts/check_storage_safe_remove.py` pins the semantics.

First verification step: inspect exact-head USB storage and AI handoff Actions for this new SHA, then XY-C1 detector/Build/TRX/artifact. Resolve failures from their first causal logs. After verification, trace other independent eject cleanup paths only if a concrete new issue is found.

## Verification boundary

- Parent main before this slice: `4c6d1b9c4c0258641da90f7a1511b775a5fb70ea`; resulting SHA must be resolved from live main after publication.
- Exact parent USB storage #45059907996 PASS and AI docs #45059908027 PASS. These prove the automount and global cleanup parser work before this physical-eject change.
- This slice adds semantic checks to `storage_ejected_parse_swap_line()`, tests invalid Type/Size/Used/Priority plus valid octal-decoded swap-file path, and extends the Python storage source contract. Existing storage workflow path filters already include the policy header, its C regression, and checker for push and pull_request.
- Local execution NOT RUN because no repository checkout is available. Exact-head Storage must compile/run the ejected-inventory GNU89 regression and complete `check_storage_safe_remove.py`.
- XY-C1 #45059757452 on ancestor product-source `d37cf8a` was still building at last inspection. This parser header is a firmware source input; same-SHA Build XY-C1, TRX verification and artifact upload are required before L3 VERIFIED.
- L4 hardware NOT VERIFIED.

## Exact first action for the next GPT

Fetch live `main` and match new USB storage, AI handoff and XY-C1 runs to its exact SHA. Inspect the ejected-inventory GNU89 test and safe-remove checker result, then the firmware detector, Build XY-C1, TRX and artifact upload. Diagnose any failure from its first causal log. Preserve the frozen backup. If this parser slice passes, continue the repository-wide storage removal review only for a separate confirmed defect.

