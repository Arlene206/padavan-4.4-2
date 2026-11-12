## Current repository state

- Repository: \`Arlene206/padavan-4.4-2\`; sole authorized development/commit/push branch: \`main\`. Baseline main SHA for this repair: \`9a508588a33c220b334d1dd40012eb5fd5ac89ec\`. Re-fetch live \`main\` before every subsequent write.
- Remote branch inventory at recovery contains only \`main\`. Legacy PR #1 is CLOSED (not merged) after its valid work was ported directly to main; leave its retained historical ref untouched/read-only. GPT Session Rotator is immutable and unchanged.
- The previous XY-C1 RC storage compile blockers are resolved in \`9a508588a33c220b334d1dd40012eb5fd5ac89ec\`: system \`ctype.h\` is included before the \`isblank\` fallback, \`stop_stor_apps()\` has a matching status-returning declaration, and the Samba shared-header staging regression passes.
- Exact-head XY-C1 run [43263025719] / job [131339067215] failed on \`9a508588a33c220b334d1dd40012eb5fd5ac89ec\` while compiling RC \`net_wan6.o\`: \`net_wan6.c:34\` includes \`../../shared/src/dhcp6c_duid_file_policy.h\`, a source-tree-relative path that does not exist from the staged RC build directory. The compile command already contains \`-I$(STAGING_DIR)/include/shared\`; the shared package installs its root-level headers there.
- On the same exact SHA, AI handoff docs [43263025362] and HTTPD security [43263025674] are SUCCESS. Separate unrelated failures remain visible: storage-hotplug [43263025900] fails its standalone BusyBox staged-header build; WAN DHCP [43263025911] fails a source-contract declaration matcher. Do not conflate either with the XY-C1 RC compiler error.

## Active task — Complete the XY-C1 target firmware build on main

1. Replace the source-relative DUID policy include in \`trunk/user/rc/src/net_wan6.c\` with \`#include <dhcp6c_duid_file_policy.h>\`, using the already configured staged include directory; do not duplicate the header or weaken the compiler.
2. Add and run a GNU89 syntax-only regression that copies the canonical shared header into a fixture \`stage/include/shared\` and compiles it through that include path. Assert shared header installation, RC include flags, shared-before-RC package ordering and push path-filter coverage.
3. Commit directly to \`main\`. Match the next XY-C1 \`push\` run's \`head_sha\` to live \`main\`; inspect the complete job log and fix only the next first-causal build failure.
4. Continue until the MIPS build completes, \`Verify TRX output\` finds at least one non-empty \`trunk/images/*.trx\`, and artifact upload succeeds on the same exact main SHA.
5. Physical router flashing/boot is a separate L4 check and must not be inferred from CI. Reconcile the unrelated storage-hotplug and WAN DHCP contract failures after the target-build blocker is closed.

## Verification boundary

- L1 recovery: bootstrap and development protocol, live main, branch/PR inventory, handoff, roadmap, newest 20 development-log records and archive index reviewed.
- L2 baseline: XY-C1 [43263025719] FAIL on exact SHA \`9a508588a33c220b334d1dd40012eb5fd5ac89ec\` at \`net_wan6.o\`; prior Samba staging, RC ctype and prototype changes passed their pre-build/source checks. AI docs [43263025362] PASS and HTTPD security [43263025674] PASS on the same SHA. Storage-hotplug [43263025900] and WAN DHCP [43263025911] have separate failures recorded above.
- L3 target MIPS firmware, non-empty TRX validation and artifact upload: NOT VERIFIED for the current main. L4 physical router flashing/boot: NOT VERIFIED.

## Exact first action for the next GPT

Fetch live \`main\`; inspect the exact-head XY-C1 run triggered by the DUID staged-include repair. Confirm its \`head_sha\` equals live \`main\`. If the pre-build staged-header contract passes and the compiler exposes another first-causal error, repair that error directly on \`main\` and repeat until firmware build, non-empty TRX verification and artifact upload all pass.
