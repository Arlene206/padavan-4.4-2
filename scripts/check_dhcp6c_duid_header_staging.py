#!/usr/bin/env python3
"""Guard the RC DHCPv6 DUID header's staged include contract."""

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


source = read("trunk/user/rc/src/net_wan6.c")
shared_header = ROOT / "trunk/user/shared/src/dhcp6c_duid_file_policy.h"
shared_make = read("trunk/user/shared/src/Makefile")
shared_package = read("trunk/user/shared/Makefile")
rc_package = read("trunk/user/rc/Makefile")
user_make = read("trunk/user/Makefile")
workflow = read(".github/workflows/xy-c1-firmware-validation.yml")

require(source.count("#include <dhcp6c_duid_file_policy.h>") == 1,
        "net_wan6.c must include the public DUID policy header by staged name")
require("../../shared/src/dhcp6c_duid_file_policy.h" not in source,
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

required_paths = (
    "trunk/user/rc/src/net_wan6.c",
    "trunk/user/rc/Makefile",
    "trunk/user/shared/src/Makefile",
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
        "DUID staged-header regression must run before the target firmware build")

compiler = shutil.which("cc")
require(compiler is not None, "host C compiler 'cc' is unavailable")
with tempfile.TemporaryDirectory(prefix="dhcp6c-duid-stage-") as directory:
    stage_include = Path(directory) / "stage" / "include" / "shared"
    stage_include.mkdir(parents=True)
    shutil.copy2(shared_header, stage_include / "dhcp6c_duid_file_policy.h")
    probe = Path(directory) / "dhcp6c_duid_stage_probe.c"
    probe.write_text(
        "#include <dhcp6c_duid_file_policy.h>\n"
        "int dhcp6c_duid_stage_probe(const char *path)\n"
        "{\n"
        "    int fd = dhcp6c_duid_open_existing(path);\n"
        "    return fd < 0 ? dhcp6c_duid_open_create(path) : close(fd);\n"
        "}\n",
        encoding="utf-8",
    )
    subprocess.run(
        [
            compiler,
            "-std=gnu89",
            "-Wall",
            "-Wextra",
            "-Werror",
            "-fsyntax-only",
            "-I" + str(stage_include),
            str(probe),
        ],
        check=True,
    )

print("DHCPv6 DUID public header compiles through staged include/shared: PASS")
print("RC package shared include path and shared-before-RC build order: PASS")
print("XY-C1 source and push-filter coverage: PASS")
