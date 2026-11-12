## Current repository state

- Repository: \`Arlene206/padavan-4.4-2\`; only \`main\` may be used for development and validation. Base main SHA for this repair: \`f80a5f6b89d7e7fca7e3f7d2f5fe55413ce409ad\`. Re-fetch live \`main\` before every subsequent write.
- Remote branch inventory remains main-only. Legacy PR #1 is CLOSED (not merged) after its valid changes were ported to main; leave its historical ref read-only. GPT Session Rotator is immutable and unchanged.
- Exact-head XY-C1 push run [43266929320] / job [131352848613] ran on \`f80a5f6b89d7e7fca7e3f7d2f5fe55413ce409ad\`. The Samba shared-header regression and new DHCPv6 DUID staged-header GNU89 regression both PASSED. RC compilation passed the previous \`net_wan6.o\` missing-header blocker and progressed through multiple later packages, then failed at the first fatal compiler error in \`trunk/build/vsftpd/main.o\`: \`asus_ext.h:55:27: error: unknown type name 'size_t'\`. \`asus_ext.h\` declared a \`size_t\` parameter but included only \`<limits.h>\`; the direct fix is \`#include <stddef.h>\`.
- The same SHA's HTTPD security run [43266928890] failed in its Python source contract, not C compilation: it still expected the old source-relative \`#include "../../shared/src/dhcp6c_duid_file_policy.h"\` in \`net_wan6.c\`. The correct staged include is now \`#include <dhcp6c_duid_file_policy.h>\`; update the assertion to match the shipped include and assert the stale relative form is absent.
- AI handoff documentation run [43266928614] PASSED on \`f80a5f6b89d7e7fca7e3f7d2f5fe55413ce409ad\`. WAN DHCP callback run [43266928617] has a separate source-contract matcher failure; keep it recorded as unrelated follow-up.
- This repair adds \`<stddef.h>\` to \`trunk/user/vsftpd/src/asus_ext.h\`, adds a standalone GNU89 self-containment probe to XY-C1, updates the HTTPD DUID source contract and makes the XY-C1 push filter trigger on these changed paths.

## Active task — Complete the XY-C1 target firmware build on main

1. Run the exact-head XY-C1 push for this repair. Require Samba, DUID staged-header and vsftpd standalone header checks to pass.
2. Inspect the full MIPS build log; fix only the next first-causal compiler, link or image-assembly failure directly on \`main\`.
3. Continue until the firmware build succeeds, \`Verify TRX output\` finds at least one non-empty \`trunk/images/*.trx\`, and artifact upload succeeds on the same exact \`main\` SHA.
4. The HTTPD DUID contract change must also pass on the same SHA. Reconcile the separate WAN DHCP matcher issue after the target build is green.
5. Physical router flashing/boot is a separate L4 check and must not be inferred from CI.

## Verification boundary

- L1 recovery: bootstrap/development protocol, live main, branch/PR inventory, handoff, roadmap, newest 20 dev-log records and archive index reviewed.
- L2 on base SHA \`f80a5f6b89d7e7fca7e3f7d2f5fe55413ce409ad\`: XY-C1 [43266929320] FAIL at vsftpd \`asus_ext.h:55\`; Samba and DUID staged-header tests PASS; RC advanced beyond \`net_wan6.o\`. AI docs [43266928614] PASS. HTTPD security [43266928890] FAIL only because its DUID include assertion still expects the previous relative include.
- L3 MIPS image, non-empty TRX and artifact upload: NOT VERIFIED. L4 physical hardware flashing/boot: NOT VERIFIED.

## Exact first action for the next GPT

Fetch live \`main\`, verify the next XY-C1 \`push\` run's \`head_sha\` equals it, and inspect that run through TRX verification and artifact upload. If another first-causal failure appears, fix it on \`main\` and repeat.
