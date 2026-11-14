#!/usr/bin/env python3
"""Supplemental integration checks for process-inventory production wiring."""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
shared_makefile = (root / "trunk/user/shared/src/Makefile").read_text()
shared_header = (root / "trunk/user/shared/src/shutils.h").read_text()
pids_source = (root / "trunk/user/shared/src/pids.c").read_text()
common_ex = (root / "trunk/user/rc/src/common_ex.c").read_text()
policy = (root / "trunk/user/rc/src/service_lifecycle_policy.h").read_text()

assert "proc_inventory.o" in shared_makefile
assert '#include "process_inventory.h"' in shared_header
assert "process_inventory_scan(appname, &snapshot)" in pids_source
assert "PROCESS_PRESENCE_UNKNOWN" in pids_source
assert "process_inventory_scan(service_name, snapshot)" in common_ex
assert "process_identity_validate(service_name, identity)" in common_ex
assert "kill(identity->pid, signal_number)" in common_ex
assert 'doSystem("killall %s %s", signal_option, service_name)' not in common_ex
assert "service_lifecycle_preflight" in policy
assert "service_lifecycle_signal_targets" in policy
assert "SERVICE_LIFECYCLE_UNKNOWN" in policy
print("process inventory and lifecycle integration contract: PASS")
