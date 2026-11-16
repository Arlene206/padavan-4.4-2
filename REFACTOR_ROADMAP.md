## Current source / CI checkpoint

- Product-source parent: `c3b02529703add66cbd811b8dfa729edef50ad74`.
- Confirmed static-IPoE false-success: shared `ifconfig()` maps malformed IPv4/netmask text to `INADDR_ANY` and may return success. Static IPoE now validates with the shared IPv4 and contiguous-netmask predicates first; invalid values return EINVAL, LAN subnet overlap remains EADDRINUSE.
- Added GNU89 regressions for valid/invalid IPv4 text and contiguous/non-contiguous/zero netmasks; shared WAN checker and WAN DHCP workflow require these checks before ifconfig().
- The concurrent PPP IP-up callback change's first WAN DHCP run `43870165716` failed at test compilation because two test calls still had four args; any remaining 4-argument test calls have been corrected to pass NULL peer IP.
- Exact-SHA verification for this tree is pending; no L3/L4 claim.

