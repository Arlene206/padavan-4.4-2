# 2026-11-20 continuation checkpoint

## Git state

- Parent main SHA for this atomic RT3352 MII slice: `cb4cb1debdfc2b5ba3b773e2e5fbbb6b58449f96`.
- Frozen `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only; do not move).
- Recent main commits: `cb4cb1de` test(miniupnpd): validate normal-path lease errno resets; `3bdb1d9f` test(miniupnpd): fix lease errno contract ordering; `ee9428dc` fix(miniupnpd): preserve protocol and lease error compatibility; `b6b9903f` test(miniupnpd): correct XML escaping capacity expectation; `a6897a11` fix(miniupnpd): escape SOAP mapping response text.

## Parent exact-SHA Actions

On `cb4cb1debdfc2b5ba3b773e2e5fbbb6b58449f96`, MiniUPnP regression #44743650240 and AI handoff #44743650185 were in progress at last check. The prior `3bdb1d9f` MiniUPnP host regression #44743367478 and AI docs #44743367373 passed. Earlier SOAP numeric SHA `2d124f3d` passed same-SHA XY-C1 Build/TRX/artifact; don't transfer that verification to later product-source SHAs.

No L3 claim unless detector, XY-C1 build, TRX validation and artifact upload all pass on one exact source SHA. Physical-router L4 is NOT VERIFIED.

## Active slice — RT3352 MII runtime radio/isolation transaction

- Defect: `mlme_radio_rt_apply()` ignored `phy_isolate_inic()` status and published MLME/NVRAM plus LED state before switch isolation was verified.
- This slice adds checked power/isolation ordering and best-effort fail-closed rollback; it publishes state only on successful required hardware operations. GNU89/Werror policy tests and Wireless source contracts are included; the XY-C1 trigger now includes the helper header.
- Next: verify exact-head Wireless, Process, AI docs, NVRAM and XY-C1 Actions; diagnose failures from logs. Continue independent stability/security work while jobs are pending.

## Operating constraints

- Main only; one complete tree/commit per logical slice and update with force=false plus exact expected SHA. Never amend, force-push, use branches/PRs or touch the frozen backup.
- Preserve GNU89/MIPS-uClibc/BusyBox/Linux 4.4 compatibility; distinguish host L2, same-source firmware L3 and physical L4.
