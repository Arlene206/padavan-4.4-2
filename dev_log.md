## 2026-11-23 08:59 UTC+8 — Restore required end-of-session handoff checkpoint

- Parent main SHA: `4ecc28778b3605743262514dbce53b7ffbebda2d`.
- Exact-head AI handoff run `45091738990` failed because SESSION_HANDOFF lacked the mandatory `Mandatory handoff checkpoint` heading and exact conditional-edit sentence. Restored the required section and clarified that checkpoint verification is mandatory while edits/no-op commits are conditional on material change.
- Recorded the exact status boundary: Switch CLI `45091738973` PASS; XY-C1 `45091739058` workflow SUCCESS but Build/TRX/artifact SKIPPED for workflow/docs-only changes; AI docs failure corrected by this commit.
- Parent `4ecc2877` includes a follow-up whitespace-normalized Samba source contract and a miniupnpd fix that snapshots LAN IP/netmask/MAC before config generation to avoid shared-NVRAM-cache pointer overwrite. Exact-head Process and related host checks are in progress.
- MTK ESW same-SHA kernel Build/TRX/artifact and physical hardware L4 remain unverified.
- Resulting commit: THIS COMMIT.

---
