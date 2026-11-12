#!/usr/bin/env python3
"""Guard the RC/Samba shared-account header's staged include contract."""

from pathlib import Path
import shutil
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[1]


def require(condition, message):
    if not condition:
        raise SystemExit("Samba shared-header contract: FAIL: " + message)


def read(relative):
    path = ROOT / relative
    require(path.is_file(), "missing required input: " + relative)
    return path.read_text(encoding="utf-8")


policy = read("trunk/user/rc/src/samba_share_policy.h")
shared_header = ROOT / "trunk/user/shared/src/include/router_account_policy.h"
shared_make = read("trunk/user/shared/src/Makefile")
shared_package = read("trunk/user/shared/Makefile")
rc_package = read("trunk/user/rc/Makefile")
rc_source_make = read("trunk/user/rc/src/Makefile")
user_make = read("trunk/user/Makefile")
services = read("trunk/user/rc/src/services_stor.c")
workflow = read(".github/workflows/xy-c1-firmware-validation.yml")

require(policy.count("#include <router_account_policy.h>") == 1,
        "samba_share_policy.h must include the staged router_account_policy.h")
require("../../shared/src/include/router_account_policy.h" not in policy,
        "source-tree-relative shared header include is not valid in the staged RC build")
require(shared_header.is_file(), "canonical shared account-policy header is missing")
require("HEADERS += $(shell ls include/*.h)" in shared_make,
        "shared package no longer installs public include/*.h headers")
require("$(INSTALL) -d $(DESTDIR)/include/shared" in shared_make and
        "$(INSTALL) -m 644 $(HEADERS) $(DESTDIR)/include/shared" in shared_make,
        "shared headers are not staged under include/shared")
require("$(STAGING_DIR)/include/shared" in rc_package and
        "$(eval $(call BuildPackage,rc))" in rc_package,
        "RC package does not export the staged shared-header include directory")
require("$(eval $(call BuildPackage,shared))" in shared_package,
        "shared package registration is missing")
require("$(CC) -c $*.c $(CFLAGS)" in rc_source_make,
        "RC source compile no longer receives package CFLAGS")
shared_order = user_make.find("+= shared")
rc_order = user_make.find("+= rc")
require(shared_order >= 0 and rc_order >= 0 and shared_order < rc_order,
        "shared package must be built before RC")
require('#include "samba_share_policy.h"' in services,
        "services_stor.c no longer includes the policy header under test")

required_paths = (
    "trunk/user/rc/src/samba_share_policy.h",
    "trunk/user/rc/src/services_stor.c",
    "trunk/user/rc/src/Makefile",
    "trunk/user/rc/Makefile",
    "trunk/user/shared/Makefile",
    "trunk/user/shared/src/Makefile",
    "trunk/user/shared/src/include/router_account_policy.h",
    "trunk/user/Makefile",
    "scripts/check_samba_shared_header.py",
    ".github/workflows/xy-c1-firmware-validation.yml",
)
for path in required_paths:
    require(workflow.count(f'      - "{path}"') == 1,
            "XY-C1 push filter must include exactly one entry for " + path)
require("on:\n  push:\n" in workflow and "workflow_dispatch:" in workflow,
        "XY-C1 workflow must retain push and manual validation triggers")
require("pull_request:" not in workflow,
        "XY-C1 workflow must remain push-only")
check_step = ("      - name: Verify Samba shared-header staging contract\n"
              "        run: python3 scripts/check_samba_shared_header.py")
build_step = "      - name: Build XY-C1\n"
require(check_step in workflow and workflow.index(check_step) < workflow.index(build_step),
        "staged-header regression must run before the target firmware build")

# Model the exact installed layout and compile the real policy header through it.
compiler = shutil.which("cc")
require(compiler is not None, "host C compiler 'cc' is unavailable")
with tempfile.TemporaryDirectory(prefix="samba-shared-header-") as directory:
    stage_include = Path(directory) / "stage" / "include" / "shared"
    stage_include.mkdir(parents=True)
    shutil.copy2(shared_header, stage_include / "router_account_policy.h")
    probe = Path(directory) / "samba_share_policy_probe.c"
    probe.write_text('#include "samba_share_policy.h"\n', encoding="utf-8")
    subprocess.run(
        [
            compiler,
            "-std=gnu89",
            "-Wall",
            "-Wextra",
            "-Werror",
            "-Wno-unused-function",
            "-fsyntax-only",
            "-I" + str(ROOT / "trunk/user/rc/src"),
            "-I" + str(stage_include),
            str(probe),
        ],
        check=True,
    )

print("Samba shared-account policy compiles through the staged include layout: PASS")
print("Shared package installation, RC include flags and build order: PASS")
print("XY-C1 Samba/header source trigger coverage and pre-build regression: PASS")
