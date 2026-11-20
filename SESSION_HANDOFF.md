# 2026-11-20 continuation checkpoint

## Git state

- Parent main SHA for this workflow-contract correction: `3bdb1d9f68767aeea29ce14bc93959426ee382e4`.
- Frozen `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only; do not move).
- Recent source commits: `3bdb1d9f` test(miniupnpd): fix lease errno contract ordering; `ee9428dc` fix(miniupnpd): preserve protocol and lease error compatibility; `b6b9903f` test(miniupnpd): correct XML escaping capacity expectation; `a6897a11` fix(miniupnpd): escape SOAP mapping response text; `368f9fa1` fix(miniupnpd): prevent lease-file description injection; `2d124f3d` fix(miniupnpd): validate SOAP numeric input bounds.

## Exact-SHA Actions at the parent

On `3bdb1d9f68767aeea29ce14bc93959426ee382e4`: MiniUPnP regression #44743367478 and AI handoff #44743367373 completed/success. On its parent `ee9428dcaedd7530695ddfeedcac7a326448a705`, MiniUPnP #44743068543 failed only at the source checker: it chose the first `errno = 0;` in a format-error branch rather than the normal-path reset before `fwrite()`. The second reset/close assertion was already corrected on `3bdb1d9f68767aeea29ce14bc93959426ee382e4`; this commit fixes the remaining write assertion. No current-parent L3 claim; same-source XY-C1 build for XML response changes `a6897a11` was pending at last check.

SOAP numeric-input SHA `2d124f3d` passed same-SHA XY-C1 Build/TRX/artifact. Do not transfer target verification across product-source SHAs. Hardware L4 remains NOT VERIFIED.

## Completed slices

- MiniUPnP SOAP numeric parsing: `2d124f3dbf163982c3f6198baf8c176a035585ff`; same-SHA XY-C1 Build/TRX/artifact passed.
- Lease description serialization: `368f9fa103c21efcb0f4837fe3430bfc09e94ea6`; host regression passed.
- SOAP XML response escaping/protocol/index validation: production source `a6897a11e4e19c6342f5a267ec28212467a5f443`; XML unit capacity test corrected in `b6b9903f`, later MiniUPnP suite passed.

## Active work

- This commit repairs the remaining false-negative MiniUPnP lease writer source assertion. Production code resets errno on the successful write/close path; the checker selected an earlier, mutually exclusive format-error branch.
- Next independent implementation slice: RT3352 MII runtime wireless control. `mlme_radio_rt_apply()` discards `phy_isolate_inic()` status and publishes MLME/NVRAM plus LED state before switch isolation is verified. Add checked power/isolation ordering with fail-closed rollback and a GNU89 regression; update Wireless source contract and ensure XY-C1 tracks the changed policy header.
- After commit, inspect exact-head Wireless radio-state, MiniUPnP, Process, AI, NVRAM and XY-C1 Actions. Continue other tasks while any are pending.

## Closed slices — do not re-audit without new evidence

- WAN deferred reconnect marker publication/failure propagation; RADIUS validation, secret ownership and teardown zeroization.
- Samba private-state permissions; NTP readiness persistence.
- FTP FIFO/special-file rejection and APPE append atomicity.
- MiniUPnP PID publication, numeric parsing, lease serialization and XML response hardening.

## Verification boundaries

- Host unit tests, exact-SHA Actions, exact-source MIPS/TRX/artifact L3 and physical-device L4 are separate.
- Previous build artifacts cannot validate a later product-source SHA.

