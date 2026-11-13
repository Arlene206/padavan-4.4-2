#!/usr/bin/env python3
"""Guard the RC and BusyBox DHCPv6 DUID shared-header include contracts."""

from pathlib import Path
import shutil
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[1]


def require(condition, message):
    if not condition:
        raise SystemExit("DHCPv6 DUID staged-header contract: FAIL: " + message)


def read(relative):
    path = ROOT / relative
    require(path.is_file(), "missing required input: " + relative)
    return path.read_text(encoding="utf-8")


rc_source = read("trunk/user/rc/src/net_wan6.c")
shared_header = ROOT / "trunk/user/shared/src/dhcp6c_duid_file_policy.h"
shared_make = read("trunk/user/shared/src/Makefile")
shared_package = read("trunk/user/shared/Makefile")
rc_package = read("trunk/user/rc/Makefile")
user_make = read("trunk/user/Makefile")
busybox_make = read("trunk/user/busybox/Makefile")
busybox_source = read(
    "trunk/user/busybox/busybox-1.24.x/networking/udhcp/dhcp6c_common.c"
)
package_make = read("trunk/include/package.mk")
workflow = read(".github/workflows/xy-c1-firmware-validation.yml")

require(rc_source.count("#include <dhcp6c_duid_file_policy.h>") == 1,
        "net_wan6.c must include the public DUID policy header by staged name")
require("../../shared/src/dhcp6c_duid_file_policy.h" not in rc_source,
        "source-tree-relative shared header lookup is invalid in staged RC builds")
require(shared_header.is_file(), "canonical DHCPv6 DUID policy header is missing")
require("HEADERS := $(shell ls *.h)" in shared_make and
        "$(INSTALL) -d $(DESTDIR)/include/shared" in shared_make and
        "$(INSTALL) -m 644 $(HEADERS) $(DESTDIR)/include/shared" in shared_make,
        "shared package must install root-level headers under include/shared")
require("$(STAGING_DIR)/include/shared" in rc_package and
        "$(eval $(call BuildPackage,rc))" in rc_package,
        "RC package must compile against the staged shared-header directory")
require("$(eval $(call BuildPackage,shared))" in shared_package,
        "shared package registration is missing")

shared_order = user_make.find("+= shared")
rc_order = user_make.find("+= rc")
require(shared_order >= 0 and rc_order >= 0 and shared_order < rc_order,
        "shared package must be built before RC")

# BusyBox is ordered before shared, so it intentionally consumes the canonical
# source-tree header. The initial package build receives MAKE_VARS, but its
# custom romfs re-entry must propagate them explicitly as well.
busybox_order = user_make.find("+= busybox")
require(busybox_order >= 0 and shared_order > busybox_order,
        "BusyBox source-header lookup must not assume shared is staged first")
require('#include "dhcp6c_duid_file_policy.h"' in busybox_source,
        "BusyBox DHCPv6 source must include the shared DUID policy header")
require("EXTRA_CFLAGS += -I$(ROOTDIR)/user/shared/src" in busybox_make,
        "BusyBox must expose the canonical shared source-header directory")
romfs_recipe = busybox_make.split("romfs:", 1)[1]
make_vars_pos = romfs_recipe.find("$(MAKE_VARS)")
make_cmd_pos = romfs_recipe.find("$(MAKE) -C $(PKG_BUILD_DIR)")
require(make_vars_pos >= 0 and make_cmd_pos > make_vars_pos,
        "BusyBox ROMFS recursive Kbuild must propagate MAKE_VARS before make")
require('CFLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS) $(TARGET_CPPFLAGS) $(EXTRA_CPPFLAGS)"'
        in package_make,
        "package MAKE_VARS must pass EXTRA_CFLAGS as CFLAGS")

required_paths = (
    "trunk/user/rc/src/net_wan6.c",
    "trunk/user/rc/Makefile",
    "trunk/user/shared/src/Makefile",
    "trunk/user/busybox/Makefile",
    "trunk/user/busybox/busybox-1.24.x/networking/udhcp/dhcp6c_common.c",
    "scripts/check_dhcp6c_duid_header_staging.py",
)
for path in required_paths:
    require(workflow.count(f'      - "{path}"') == 1,
            "XY-C1 push filter must include exactly one entry for " + path)
require("on:\n  push:\n" in workflow and "workflow_dispatch:" in workflow,
        "XY-C1 workflow must retain push and manual validation triggers")
require("pull_request:" not in workflow,
        "XY-C1 workflow must remain push-only")

check_step = ("      - name: Verify DHCPv6 DUID staged-header contract\n"
              "        run: python3 scripts/check_dhcp6c_duid_header_staging.py")
build_step = "      - name: Build XY-C1\n"
require(check_step in workflow and workflow.index(check_step) < workflow.index(build_step),
        "DUID/BusyBox regression must run before the target firmware build")

compiler = shutil.which("cc")
require(compiler is not None, "host C compiler 'cc' is unavailable")

with tempfile.TemporaryDirectory(prefix="dhcp6c-duid-stage-") as directory:
    stage_include = Path(directory) / "stage" / "include" / "shared"
    stage_include.mkdir(parents=True)
    shutil.copy2(shared_header, stage_include / "dhcp6c_duid_file_policy.h")

    staged_probe = Path(directory) / "rc_dhcp6c_duid_stage_probe.c"
    staged_probe.write_text(
        """#include <dhcp6c_duid_file_policy.h>
int rc_dhcp6c_duid_stage_probe(const char *path)
{
    int fd = dhcp6c_duid_open_existing(path);
    return fd < 0 ? dhcp6c_duid_open_create(path) : close(fd);
}
""",
        encoding="utf-8",
    )
    subprocess.run(
        [compiler, "-std=gnu89", "-Wall", "-Wextra", "-Werror",
         "-fsyntax-only", "-I" + str(stage_include), str(staged_probe)],
        check=True,
    )

    busybox_probe = Path(directory) / "busybox_dhcp6c_duid_source_probe.c"
    busybox_probe.write_text(
        """#include "dhcp6c_duid_file_policy.h"
int busybox_dhcp6c_duid_source_probe(const char *path)
{
    int fd = dhcp6c_duid_open_existing(path);
    return fd < 0 ? dhcp6c_duid_open_create(path) : close(fd);
}
""",
        encoding="utf-8",
    )
    subprocess.run(
        [compiler, "-std=gnu89", "-Wall", "-Wextra", "-Werror",
         "-fsyntax-only", "-I" + str(shared_header.parent), str(busybox_probe)],
        check=True,
    )

print("DHCPv6 DUID public header compiles through staged include/shared: PASS")
print("RC package shared include path and shared-before-RC build order: PASS")
print("BusyBox DUID header compiles through source include path: PASS")
print("BusyBox ROMFS Makefile propagates MAKE_VARS/CFLAGS: PASS")
print("XY-C1 source and push-filter coverage: PASS")
