## 2026-11-23 — Make analog calibration bit cleanup fail-safe

- Base SHA: `87e233d30dd532b6bfd1589d2469ced8d8f42a16`. Switch CLI #45273294951 PASS; AI handoff #45273294849 PASS. XY-C1 #45273294825 and NVRAM #45273294824 were still in progress at the last poll.
- Found a concrete cleanup hazard in `clear_ckinv_ana_txvos()`: three page-7/register-24 read-modify-write steps used legacy value-only wrappers. A failed read is returned as zero, so cleanup could overwrite the register with zero before the final zero write and silently continue.
- Added a reusable checked update-bits policy that performs no write after a failed read and publishes the updated value only after successful write. The cleanup now stops its intermediate RMW sequence on the first MDIO error, but always attempts the original final register-zero write. The legacy void API is unchanged and reports a rate-limited error.
- Regression coverage checks unrelated-bit preservation, read timeout and positive-status normalization, failed writes, output clearing and invalid callbacks. Switch CLI now asserts the three production RMW steps and unconditional final cleanup write.
- This change is limited to the shared cleanup helper; no large GE algorithm rewrite or rollback semantics are inferred. No local MIPS build or hardware L4 is claimed.
- Next: inspect this slice's exact-SHA Switch CLI/AI/XY-C1/NVRAM runs; then trace the remaining calibration save/restore path before changing other long sequences.

---
