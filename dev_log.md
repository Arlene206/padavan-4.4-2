## 2026-11-23 — Fail-safe VPN client shared-xl2tpd restoration

- Base main SHA: `c6b5dda145894feacbe67a2cabd4f5f60b99b046`; implementation, policy regressions, exact-source checker and this handoff are in one atomic commit.
- The VPN client used error-collapsing NVRAM match getters to determine xl2tpd ownership and decide whether to restore a shared daemon after restart. A transient backend read error could be misread as no dependency, disrupting an independently configured WAN L2TP client or L2TP VPN server.
- Added a strict bounded NVRAM integer reader. VPNC enable/type/client ownership is now captured under one transaction before teardown; malformed, out-of-range or unreadable selector state aborts cleanup before side effects.
- WAN/server L2TP dependencies are captured under a second transaction. When that snapshot is unknown after VPNC has stopped xl2tpd, recovery attempts conservative daemon restoration but still returns the read error. Confirmed active dependencies also restore; confirmed absence remains a no-op.
- Added GNU89 policy cases for defaults, malformed values, overflow and backend failures; added unknown-dependency restore tests and exact-source assertions. No local compiler was available; exact-head Actions supply L2 evidence. XY-C1 must pass Build/TRX/artifact on the same production-input SHA for L3.
- Previous WAN local-IP fix L2 verification: WAN route #45244253977 and AI docs #45244253913 PASS; XY-C1 #45244253978 completed Build, TRX verification and artifact upload on exact workflow descendant `7391f2e`. Physical router L4 remains NOT VERIFIED.
- VPN CI: pending for this commit; backup branch unchanged.

---

