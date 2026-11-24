#!/usr/bin/env python3
"""Verify RWFS mount status, UBI postconditions, and Optware readiness wiring."""
from pathlib import Path
import re
ROOT = Path(__file__).resolve().parents[1]
RC = ROOT / "trunk/user/rc/src"
common = (RC / "common_ex.c").read_text()
router = (RC / "rc.c").read_text()
header = (RC / "rc.h").read_text()
policy = (RC / "storage_rwfs_mount_policy.h").read_text()
test = (RC / "tests/storage_rwfs_mount_policy.c").read_text()
process_workflow = (ROOT / ".github/workflows/process-lifecycle-regression.yml").read_text()
xy_workflow = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()
def body(source, signature):
    start=source.index(signature); opening=source.index("{",start); depth=0
    for pos in range(opening,len(source)):
        if source[pos]=="{": depth+=1
        elif source[pos]=="}":
            depth-=1
            if depth==0: return source[opening+1:pos]
    raise AssertionError("unterminated function: "+signature)
mount=body(common,"int mount_rwfs_partition(void)")
start=body(common,"int start_rwfs_optware(void)")
prepare=body(common,"rwfs_mount_prepare_mountpoint(")
mount_ubifs=body(common,"rwfs_mount_ubifs(")
attach=body(common,"rwfs_mount_attach_ubi(")
module_probe=body(common,"rwfs_mount_ubi_module_status(")
attachment_probe=body(common,"rwfs_mount_ubi_attachment_status(")
router_init=body(router,"init_router(void)")
router_compact=re.sub(r"\s+"," ",router_init)
assert "mtd_rwfs = mtd_dev_idx(MTD_PART_NAME_RWFS);" in mount
assert "if (mtd_rwfs < 0)" in mount and "return ENODEV;" in mount
assert "rc_rwfs_mount_execute(" in mount and "rwfs_mount_ready = 1;" in mount
assert "rwfs_mountpoint_created && rmdir(mp_rwfs) != 0" in mount
assert "errno != ENOENT" in mount
assert 'is_module_loaded_status("ubi")' in module_probe
assert "rwfs_ubi_attachment_presence(mtd_rwfs, presence)" in attachment_probe
assert "ubiattach -p /dev/mtd%d -d %d" in attach
assert "lstat(mountpoint, &st)" in prepare and "S_ISLNK(st.st_mode)" in prepare
assert "S_ISDIR(st.st_mode)" in prepare
assert 'mount(device, mountpoint, "ubifs", 0, NULL)' in mount_ubifs
assert "!rwfs_mount_ready" in start
assert "start_status = rwfs_optware_start_worker();" in start
assert "if (start_status != 0)" in start
assert "return start_status;" in start
assert "int start_rwfs_optware(void);" in header
assert "inline int start_rwfs_optware(void) { return 0; }" in common
assert "inline int mount_rwfs_partition(void) { return 0; }" in common
assert "module_status < 0" in policy
assert "module_status == 0" in policy and "module_status != 1" in policy
attach_pos=policy.index("ops->attach_ubi(context, mtd_index)")
assert attach_pos < policy.index("ops->ubi_attachment_status(context, mtd_index, &presence);",attach_pos)
assert policy.index("ops->prepare_mountpoint(context, mountpoint)") < policy.index("ops->mount_ubifs(context, mtd_index, mountpoint)")
for name in ("test_worker_result_decoder","test_module_states","test_unknown_attachment","test_attach_requires_postcondition","test_no_mount_on_failed_attach","test_mount_errors"): assert name in test
for workflow in (process_workflow,xy_workflow):
    assert '"trunk/user/rc/src/storage_rwfs_mount_policy.h"' in workflow
    assert '"trunk/user/rc/src/tests/storage_rwfs_mount_policy.c"' in workflow
    assert '"scripts/check_rwfs_mount_integration.py"' in workflow
    assert "Compile and run RWFS UBI attach and mount fail-closed policy regression" in workflow
assert "int rwfs_mount_status;" in router_init
assert "rwfs_mount_status = mount_rwfs_partition();" in router_init
assert "boot_services_status = rc_first_service_start_status(" in router_init
assert "rwfs_mount_status, boot_modules_status" in router_compact
assert "int optware_start_status = start_rwfs_optware();" in router_init
assert "boot_services_status, optware_start_status" in router_compact
assert router_compact.index("start_rwfs_optware();") < router_compact.index(
    "boot_services_status = rc_first_service_start_status( boot_services_status, optware_start_status);")
assert "initial MTD Optware worker launch failed; retry is scheduled" in router_init
print("RWFS mount, Optware launch result propagation and retry wiring: PASS")
