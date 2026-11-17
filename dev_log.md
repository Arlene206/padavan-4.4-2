## 2026-11-16 — Align PPP peer-probe source contracts with argv execution

- Parent `5327f7626fc307c8db5e2de670e5433cd0eef55b` exact-head WAN DHCP `43990883611` PASS; Firewall `43990883471`, WAN route `43990883432`, Process inventory `43990883425` and XY-C1 `43990883486` FAIL; NVRAM `43990883715` was still running.
- Firewall, Process and XY-C1 failure logs converged on `scripts/check_modem_wan_start_status.py:203`: the checker still located the shell-formatted ping after `wait_ppp_up()` had migrated to `eval("ping", "-c1", ppp_gate)`. WAN route's embedded checker had the same stale expression at Python line 300. Prior host regressions in those jobs passed before this source-contract failure.
- Updated the shared checker and WAN route workflow to require the argv ping command, reject the old `doSystem()` expression, and continue checking the success return occurs after the peer probe. No production C change was needed for this CI drift.
- Exact follow-up Actions and the XY-C1 build gate remain pending. The prior XY-C1 failure occurred in the stale host assertion before target compilation; it is not a firmware-build result.

---

