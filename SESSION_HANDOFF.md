## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; sole authorized development/commit/push branch: `main`. Baseline main SHA for this repair batch: `98dedc29c55b2699857c05f3c708188e0f7d56dc`. Re-fetch live `main` before every subsequent write.
- Remote branch inventory at recovery contained only `main`. Legacy PR #1 is CLOSED (not merged) after its valid work was ported directly to main; leave its retained historical ref untouched/read-only. GPT Session Rotator is immutable and was not changed.
- The prior Samba shared-header staging correction is confirmed by exact-head XY-C1 run [43259091641]: its GNU89 staged-layout contract passed and the target build progressed through kernel, BusyBox and HTTPD into RC.
- On baseline SHA `98dedc29c55b2699857c05f3c708188e0f7d56dc`, XY-C1 run [43259091641] / job [131325219123] failed compiling `services_stor.o`. The first C diagnostics were: `shutils.h` defined the `isblank` fallback before `ctype.h` was parsed through `rc.h`, so the macro expanded inside libc's `isblank(int)` declaration; and `services_stor.c` defined `int stop_stor_apps(void)` while `rc.h` still declared `void stop_stor_apps(void)`.
- The same baseline's HTTPD security run [43259091559] / job [131325220100] failed in the Samba share host regression because its compile command omitted `-I trunk/user/shared/src/include`, although the policy header now correctly includes the staged public `router_account_policy.h`.
- This repair batch adds the system `ctype.h` include before the fallback macro, aligns the `stop_stor_apps` prototype with its status-returning implementation/callers, supplies the missing HTTPD host include path, and extends the XY-C1 / storage source contracts and target-build path filters.

## Active task — Complete the XY-C1 target firmware build on main

1. Keep the Samba shared account-policy include as `<router_account_policy.h>`; the staged-layout regression already passed on the previous exact head.
2. Resolve the RC compile blockers without changing runtime behavior: parse system `ctype.h` before the `isblank` fallback macro and declare `stop_stor_apps` as returning `int`, matching its implementation and safe-removal callers.
3. Keep the host Samba test compiling against the same shared public-header directory used by package staging.
4. Commit each independent repair directly to `main`; after every push, match the Actions `head_sha` to live `main` and inspect the full job log.
5. Continue fixing only the next first-causal error until the MIPS target build succeeds, `Verify TRX output` finds at least one non-empty `trunk/images/*.trx`, and artifact upload succeeds on that same exact main SHA.
6. Physical router flashing/boot remains a separate L4 check and must not be inferred from CI.

## Verification boundary

- L1 recovery: bootstrap/development protocol, handoff, roadmap, dev log and archive index reviewed; live baseline `98dedc29c55b2699857c05f3c708188e0f7d56dc`; only remote development branch is `main`; PR #1 remains closed/not merged; Session Rotator unchanged.
- L2 baseline Actions: AI docs [43259091723], infosvr [43259091710], AdGuardHome [43259091774], AliyunDrive [43259091586] and AliDDNS [43259092023] SUCCESS on `98dedc29c55b2699857c05f3c708188e0f7d56dc`. XY-C1 [43259091641] and HTTPD security [43259091559] FAILED on that same SHA for the compiler/test causes documented above.
- The previous Samba staged-header regression passed in the XY-C1 job before the RC compile failure. Full MIPS firmware, non-empty TRX and artifact upload remain NOT VERIFIED. L4 physical router validation remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch live `main` and query the XY-C1, HTTPD security, storage-hotplug and AI handoff push runs triggered by this repair. Confirm every run's `head_sha` equals the current main SHA. Inspect the complete logs; if XY-C1 exposes another first-causal error, fix only that issue directly on main and repeat until the MIPS build, non-empty TRX verification and artifact upload all pass.
