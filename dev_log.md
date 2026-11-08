## 2026-11-09 04:21 CST — Reject failed switch PVIDs in WAN VLAN and Hardware NAT callers

- **Base main HEAD:** `5196541495674cce0173c3c3e8882d53966adb0b`.
- **Implementation commit:** `f73b64108617772a8614fbbb7aaa89a69d2768cf` — fixed RC PVID caller error propagation, WAN VLAN fail-closed setup, Hardware NAT VID handling and LAN restart comparison.
- **Module / priority:** WAN VLAN setup + MTK/RTL switch status consumers; P2 stability/availability. Wireless/switch remains **AUDITING**.
- **Recovered task:** resumed the precise next action saved by the preceding HTTPD switch-status handoff; did not repeat the completed HTTPD wrapper or wireless scan fixes.
- **Inventory:** enumerated all 36 C translation units under `trunk/user/rc/src/`. Within that bounded directory, direct `phy_vlan_pvid_wan_get()` consumers were in `net_wan.c` and `net_lan.c`; the `get_vlan_vid_wan()` result was consumed by two Hardware NAT paths in `net.c`. Repository-wide/external consumer closure is not claimed.
- **Finding:** after the prior getter fix changed switch PVID read errors to negative errno, callers could treat that error as a VLAN ID. WAN VLAN fallback could feed it to hardware VLAN TX mapping/interface naming/VLAN creation; Hardware NAT could load or set a negative `wan_vid`; a failed PVID comparison could force an unnecessary full WAN restart.
- **Fix:** validate PVID fallback values and propagate errors from `get_vlan_vid_wan()`; make CPU and IPTV VLAN setup return status and validate before side effects; abort unsafe primary WAN startup and clean callback links; isolate IPTV failure; sample the WAN VID once for Hardware NAT and do not load/update it on error; compare LAN reconfiguration PVIDs only when both reads succeeded.
- **Changed paths:** `trunk/user/rc/src/{net_wan.c,net.c,net_lan.c,switch_status_policy.h,tests/switch_status_policy.c}`; `.github/workflows/httpd-security.yml` (symmetric push/PR paths + source-order contracts).
- **Tests / CI:** expanded the standalone switch-status policy regression for explicit configured VLAN compatibility, valid fallback, negative and invalid PVID failure, unchanged output on error, and PVID-change comparison. Exact-head Actions [42519616962](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42519616962) on `f73b64108617772a8614fbbb7aaa89a69d2768cf` completed **SUCCESS 39/39**, including GNU89 `-Wall -Wextra -Werror` test compilation/execution and RC source contracts.
- **Validation boundary:** L1 source/caller review and L2 host policy test + exact-head Actions PASS. Production RC translation-unit builds, MIPS/uClibc, complete firmware/TRX, WAN integration and physical router/switch tests NOT RUN.
- **Next:** rotate to IPv6 Firewall/NAT parity, mapping all IPv6 filter writers, startup gates and rollback behavior against the IPv4 transaction path. Keep Firewall/NAT and Wireless AUDITING; review the independent `trunk/user/utils/src/switch/` CLI/register tools as a separate remaining surface.

---

