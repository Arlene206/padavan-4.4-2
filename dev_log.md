## 2026-11-23 — Restore MTK ESW VLAN helpers and repair compile-time control flow

- XY-C1 #45250584713 on `895a2f060cae5893b9daf3c3b76575bfda5ca479` exposed additional compile failures after the missing ioctl callbacks were restored: undefined `VLAN_ENTRY_ID_MAX`; implicit `find_vlan_slot()` and `find_free_min_pvid()`; missing success return in `esw_vlan_init_vid1()`; and a closing brace in `vlan_create_entry()` that placed its success return at file scope.
- Restored `VLAN_ENTRY_ID_MAX (15)`, bounded VLAN slot lookup and an iterative free VID scan from 2 through 4094. The explicit exhaustion sentinel 4095 is rejected by `esw_vlan_apply_rules()` with `-ENOSPC` before changing the cached WAN PVID or programming hardware.
- Added in-function success returns for VID-1 initialization and VLAN entry creation, plus workflow assertions that declarations precede users, free VID selection is bounded, exhaustion is rejected, and both non-void functions return.
- Switch CLI #45250584832 failed on an obsolete reverse-order assertion. Commit `c730d8dfe9cd37007ca0f11aa90a378f3c69e4a2` removed the contradictory check while preserving reset → VLAN entries → per-port state.
- VPN lifecycle #45250796998 and AI docs #45250796898 PASS on `cdc98747492260b9f59635f77fae8003cec71f48`. The fresh XY-C1 and Switch CLI checks must run after this source correction; no L3 PASS is claimed.

---

