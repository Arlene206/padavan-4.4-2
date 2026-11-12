## Current repository state

- Repository: Arlene206/padavan-4.4-2; only main is authorized for development, commits and push validation. Base main SHA for this BusyBox repair: a3247c882afdc66b781d060b47e4182b9ad197e1. Re-fetch live main before every subsequent write.
- Remote branch inventory remains main-only. Legacy PR #1 is CLOSED (not merged) and read-only; GPT Session Rotator remains immutable.
- Exact-head XY-C1 run 43270739238 / job 131366274506 on a3247c882afdc66b781d060b47e4182b9ad197e1: Samba, DHCPv6 DUID staged-header and vsftpd extension-header self-containment checks all PASSED. The MIPS build passed RC net_wan6.o, vsftpd and subsequent package compilation.
- The first fatal error occurred only when ROMFS busybox re-entered Kbuild to install BusyBox: networking/udhcp/dhcp6c_common.c:38 reported fatal error: dhcp6c_duid_file_policy.h: No such file or directory. The earlier BUILD busybox invocation at log lines ~15964-16314 had CFLAGS=-I$(ROOTDIR)/user/shared/src and compiled dhcp6c_common.o successfully. The later custom romfs recipe invoked $(MAKE) ... $(MAKE_FLAGS) install without $(MAKE_VARS); the recursive Kbuild therefore lost CFLAGS/EXTRA_CFLAGS and recompiled without the shared header path.
- BusyBox appears before shared in trunk/user/Makefile, so it intentionally uses the canonical source header path $(ROOTDIR)/user/shared/src rather than relying on a staged shared package that has not yet been built. The narrow fix is to prefix the BusyBox romfs recursive make with +$(MAKE_VARS), preserving the existing shared-source include path.
- The DUID regression script is extended to verify BusyBox source include + package ordering + romfs MAKE_VARS propagation and syntax-compile the header through the actual BusyBox source include path.

## Active task — Complete the XY-C1 target firmware build on main

1. Run the exact-head XY-C1 push for this BusyBox ROMFS include-propagation repair.
2. Require Samba, RC DUID staged include, BusyBox source-header compile, BusyBox MAKE_VARS contract and vsftpd self-containment prechecks to pass.
3. Inspect the complete target build; repair only the next first-causal compile/link/image error directly on main.
4. Continue until Build XY-C1 succeeds, Verify TRX output finds at least one non-empty trunk/images/*.trx, and artifact upload succeeds on the same exact main SHA.
5. Hardware router flashing/boot remains a distinct L4 check and must not be inferred from CI. The separate WAN DHCP source-contract matcher remains follow-up work.

## Verification boundary

- L1 repository protocol/context recovery completed; main-only branch policy retained.
- L2 exact-head 43270739238 on a3247c882afdc66b781d060b47e4182b9ad197e1: all three pre-build checks passed; BusyBox ROMFS failed because its custom recursive install make dropped CFLAGS and could not find DUID header. The initial BusyBox package compile did receive the shared-source include and compiled the same translation unit.
- L3 MIPS image, non-empty TRX and artifact upload: NOT VERIFIED. L4 physical hardware flashing/boot: NOT VERIFIED.

## Exact first action for the next GPT

Fetch live main; verify the next XY-C1 push run's head_sha equals main; confirm the DUID/BusyBox/vsftpd prechecks and continue the full build through TRX verification and artifact upload. Repair the next causal failure if one appears.
