# Development log archive index

- Root `dev_log.md` holds the newest 20 complete records, newest first.
- Archive parts are chronological oldest-to-newest. To reconstruct the old source order, keep the 19 previous newest records from the root first, then reverse the archived record sequence.
- **Original source:** `dev_log.md` blob `6dc04f5aef231f9f4ea0e100abe87fce22c49fc0`, 984527 bytes, 663 records. Every historical record body is byte-identical to its source record.
- **Shard policy:** target 256–384 KiB; hard cap 512 KiB; split only between complete dated records.

| Archive file | Records | Date order | Bytes | Git blob SHA |
|---|---:|---|---:|---|
| `docs/dev-log/2026/11/part-001.md` | 294 | 2026-11-05 → 2026-11-06 | 357382 | `5d2cfa2439d7b1be33333cee6964c005c81dc5ad` |
| `docs/dev-log/2026/11/part-002.md` | 275 | 2026-11-06 → 2026-11-08 | 357977 | `164858e25153406e26e3f2b9bb6f3904557d5cda` |
| `docs/dev-log/2026/11/part-003.md` | 76 | 2026-11-08 → 2026-11-09 | 214022 | `0aaf5c85ba76f2149f3841b1d8fa8ae5e030e345` |


- **Post-migration rotation:** the first evicted root-window record is preserved verbatim in `part-003.md`; its original root source blob was `733d3c678575611642b3f69fe4cfe5e19ece9b57`.