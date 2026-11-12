## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; sole authorized development/commit/push branch: `main`. Baseline main SHA for this repair: `696278e438243af3ee67ab6e403dc27c7e0753df`. Re-fetch live `main` before every subsequent write.
- Repository bootstrap and development protocol were re-read. Remote branch inventory contains only `main`; legacy PR #1 is CLOSED (not merged) after its valid work was ported directly to main. Leave its historical branch untouched/read-only.
- The active task is the XY-C1 full MIPS firmware build. An older WAN DHCP callback source-contract failure [43233088640] on SHA `4890bd4f800d3d1f1f282061f8fe7ea8e533a969` is separate and must not be conflated with the target build result.
- Exact-current-main non-target push workflows on baseline `696278e438243af3ee67ab6e403dc27c7e0753df`: AI docs [43252375787], AliyunDrive [43252375740], AliDDNS [43252375645], infosvr [43252375595] and AdGuardHome [43252375581] all SUCCESS. No XY-C1 run was triggered on that SHA because its previous push path filters omitted the RC/shared-header inputs.
- Latest XY-C1 run before this repair: [43233088562](https://github.com/Arlene206/padavan-4.4-2/actions/runs/43233088562), job [131233221624](https://github.com/Arlene206/padavan-4.4-2/actions/runs/43233088562/job/131233221624), failed on old SHA `4890bd4f800d3d1f1f282061f8fe7ea8e533a969` compiling RC `services_stor.o`. `services_stor.c` includes `samba_share_policy.h`; its relative include to `../../shared/src/include/router_account_policy.h` fails after the RC source is staged under `trunk/build/rc`. The shared package installs the canonical header to `$(STAGING_DIR)/include/shared`, and RC package `TARGET_CFLAGS` provides `-I$(STAGING_DIR)/include/shared`.

## Active task — Complete the XY-C1 target firmware build on main

1. Change the RC Samba share policy header to include the staged public header as `<router_account_policy.h>`; preserve all ACL/account-policy behavior.
2. Add `scripts/check_samba_shared_header.py`: compile the actual policy header with GNU89 against a staged-layout fixture, and assert the shared-package install target, RC staged include flag, package build order and source dependency.
3. Extend the push-only XY-C1 workflow to run the contract before the cross-build and trigger on every RC/shared-header input used by the contract.
4. Commit directly to `main`; match the XY-C1 run's `head_sha` to the exact pushed/current main SHA.
5. Continue repairing only the next first-causal build error until the MIPS build succeeds, `Verify TRX output` finds one or more non-empty `trunk/images/*.trx` files, and artifact upload succeeds on that same SHA.
6. Target firmware/TRX/artifact: NOT VERIFIED until exact-head Actions completes. L4 physical router flash/boot: NOT VERIFIED.

## Verification boundary

- L1 recovery: bootstrap/protocol and required records reviewed; live baseline main SHA `696278e438243af3ee67ab6e403dc27c7e0753df`; remote branch inventory contains only `main`; PR #1 remains closed/not merged; GPT Session Rotator unchanged.
- L2 exact-baseline non-target push workflows: AI docs [43252375787], AliyunDrive [43252375740], AliDDNS [43252375645], infosvr [43252375595] and AdGuardHome [43252375581] SUCCESS on `696278e438243af3ee67ab6e403dc27c7e0753df`.
- Previous XY-C1 run [43233088562] / job [131233221624] FAILED on stale SHA `4890bd4f800d3d1f1f282061f8fe7ea8e533a969` at the RC Samba policy-header include. This is not an exact-baseline-main or post-repair result.
- The source-header correction, staged-layout host compile regression and XY-C1 push path coverage are in the current main repair. Exact-head Actions, MIPS firmware build, non-empty TRX and artifact upload remain PENDING.
- L4 physical router validation: NOT VERIFIED.

## Exact first action for the next GPT

Fetch live `main` and the XY-C1 push run triggered by this repair. Confirm its `head_sha` equals current main, inspect the source-contract step and complete build log. If it fails, isolate the first causal error, make one narrow main-only repair and repeat until the target build, non-empty TRX verification and artifact upload all pass. Do not claim L3/L4 before those checks.
