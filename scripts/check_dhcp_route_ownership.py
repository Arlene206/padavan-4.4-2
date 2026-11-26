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
assert "dhcp_route_cleanup_owned(owner)" in body
iptv_deconfig = src.split("manv_route_deconfig(", 1)[1].split("\\n}", 1)[0]
assert "route_result = control_dhcp_routes(" in iptv_deconfig
assert '"manv_", route_storage[0]' in iptv_deconfig
assert "strcmp(entries[i].owner, owner) != 0" in policy
assert "entries[i].ifname, ifname" not in policy
assert 'DHCP_ROUTE_OWNERSHIP_MAGIC "PADAVAN-DHCP-ROUTES 2"' in policy
assert 'sscanf(line, "%15s %15s %15s %15s %15s %15s %c"' in policy
assert 'metric_value = strtol(metric_text, &metric_end, 10);' in policy
assert "errno == ERANGE" in policy
assert r"*metric_end != '\0'" in policy
assert "metric_value < 0" in policy
assert "metric_value >= INT_MAX" in policy
assert "entry.metric = (int)metric_value;" in policy
assert "&entry.metric" not in policy
assert "Service ownership survives changes to interface and metric." in test
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
assert 'dhcp_route_ownership_owner_is_valid("wan0_")' in test
assert 'dhcp_route_ownership_owner_is_valid("wan0-x_")' in test
assert "symlinked lock file must fail closed" in test
assert "fake.add_count == add_before" in test
for metric in ("2147483648", "2147483647", "999999999999999", "-1", "7junk"):
    assert 'write_metric_manifest(path, "' + metric + '")' in test
assert "Compile and run DHCP route ownership manifest regression" in workflow
assert '"trunk/user/rc/src/dhcp_route_ownership.h"' in workflow
assert '"trunk/user/rc/src/tests/dhcp_route_ownership.c"' in workflow
print("DHCP route additions persist ownership, and teardown removes only recorded routes: PASS")
