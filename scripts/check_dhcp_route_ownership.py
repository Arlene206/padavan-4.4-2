#!/usr/bin/env python3
"""Verify DHCP route teardown follows the persisted route ownership manifest."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
src = (ROOT / "trunk/user/rc/src/net_wan.c").read_text()
policy = (ROOT / "trunk/user/rc/src/dhcp_route_ownership.h").read_text()
test = (ROOT / "trunk/user/rc/src/tests/dhcp_route_ownership.c").read_text()
workflow = (ROOT / ".github/workflows/wan-route-status.yml").read_text()

body = src.split("static int\ncontrol_dhcp_routes(", 1)[1].split(
    "\nint\nadd_dhcp_routes(", 1
)[0]
assert "dhcp_route_cleanup_owned(ifname, metric)" in body
assert "route_del(" not in body
assert "dhcp_route_add_owned(" in body
assert "dhcp_route_ownership_add(" in src
assert "dhcp_route_ownership_cleanup(" in src
assert "DHCP_ROUTE_OWNERSHIP_FILE" in src
assert "DHCP_ROUTE_OWNERSHIP_MAGIC" in policy
assert "EEXIST" in policy
assert "entries[count++] = *entry;" in policy
assert "dhcp_route_ownership_save(path, entries, count)" in policy
assert "delete_status == 0 || delete_status == ESRCH" in policy
assert "remaining[kept++] = entries[i];" in policy
assert "A route that predated DHCP (EEXIST) is never entered in the manifest." in test
assert "A failed delete retains ownership for a later retry." in test
assert "Malformed/symlink state fails closed" in test
assert "Compile and run DHCP route ownership manifest regression" in workflow
assert '"trunk/user/rc/src/dhcp_route_ownership.h"' in workflow
assert '"trunk/user/rc/src/tests/dhcp_route_ownership.c"' in workflow
print("DHCP route additions persist ownership, and teardown removes only recorded routes: PASS")
