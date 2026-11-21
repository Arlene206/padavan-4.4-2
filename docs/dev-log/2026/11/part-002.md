## 2026-11-07 06:36 CST — Snapshot aliased List removal inputs before mutation

- Baseline: main commit `e3cdfd63b94ac76c02aa8b51c58656c1d61dd48e`; SkipDBv2 Regression run #452 passed source contracts, configure, complete build, and all regression tests.
- Scope: the inline `List_removeItems_()` operation in `basekit/List_inline.h`.
- The self-removal guard handled only identical `List *` addresses. Distinct structs that shared `items` caused the removal loop to traverse an array while `List_remove_()` shifted its contents, so items were skipped; large lists could also compact/reallocate the source allocation during traversal.
- Clone the source pointer sequence before mutating a destination that shares its backing array. Allocation failure leaves the destination untouched; non-aliased calls retain the existing no-copy path. Add regression cases for a shorter shallow view and two identical views. Update existing Actions source/test contracts; no new files or CMake changes.
- No local C compilation or CTest run is claimed; mainline CI is the compile/runtime gate.

---

## 2026-11-07 06:46 CST — Keep Stack mark chains valid when ordinary pop reaches a mark

- Baseline: main commit `00ec98bd155b3df120d76d7af2311949736b33c8`; SkipDBv2 Regression run #453 passed source contracts, configure, full build, and all regression tests.
- Scope: mark lifecycle in `basekit/Stack_inline.h`.
- `Stack_pop()` removed the sentinel slot for a mark without rewinding `lastMark`. Once ordinary pops reached a mark, the mark index could exceed the new stack depth; later push/pop operations were then rejected by the mark-chain validator, leaving a live stack unusable.
- Validate the mark chain before popping and, when the removed top slot is the current mark, restore its recorded predecessor before decrementing the top pointer. Add a nested-mark regression proving normal pop can remove inner and outer mark slots and the stack remains usable afterward. Update existing Actions contracts only; no new files or CMake changes.
- No local C compilation or CTest run is claimed; mainline CI is the compile/runtime gate.

---

## 2026-11-07 06:50 CST — Honor UDB compaction time slices

- Baseline: main commit `12c784c219925e43db8727aa615b14bf4a17b7f1`; SkipDBv2 Regression run #454 passed source contracts, configure, full build, and all regression tests.
- Scope: compaction scheduling in `udb/UDBCompact.c`.
- `UDB_compactStepFor_(self, maxSeconds)` ignored its time budget and performed at most one physical compaction operation; the public `UDB_compact()` consequently re-entered the transaction machinery for every tiny step instead of allowing a bounded slice to make multiple units of progress.
- Isolate the existing atomic operation as `UDB_compactStepOnce_()`; let positive budgets repeat whole operations until the elapsed wall-clock slice expires or progress ends. Nonpositive budgets still perform at most one operation. Clock-read failures fall back to one step, backward clock adjustments do not extend a slice, and the integer operation count saturates rather than overflowing. Extend the existing compaction regression to verify that one positive time slice merges the adjacent hole and truncates the trailing extent. Update existing Actions contracts; no new files or CMake changes.
- No local C compilation or CTest run is claimed; mainline CI is the compile/runtime gate.

---

## 2026-11-07 06:52 CST — Make UDBRecord metadata accessors NULL-safe

- Baseline: main commit `57be964b2e14e224d8bf677c986c0a91b7b85dd4`; SkipDBv2 Regression run #455 passed source contracts, configure, full build, and all regression tests.
- Scope: record identity and metadata accessors in `udb/UDBRecord.c`.
- Several public getters, setters, `UDBRecord_isEmpty()`, and `UDBRecord_show()` dereferenced a NULL record despite the surrounding record API already providing NULL-safe allocation/data/size helpers. `UDBRecord_setPosToEnd()` also cast a failed `JFile_setPositionToEnd()` result of -1 into the unsigned position field.
- Make metadata accessors NULL-safe with zero/no-op sentinel behavior. Only publish an end position when the file reports a nonnegative offset, so a missing file cannot corrupt the in-memory cursor. Extend the existing record-storage regression and its Actions source contract; no new files, exported symbols, or CMake changes.
- No local C compilation or CTest run is claimed; mainline CI remains the compile/runtime gate.

---

## 2026-11-07 06:55 CST — Validate both UDB journals before arming commit recovery

- Baseline: main commit `a3e08fa14a07dd80fc409318f6899b3e5ebdb5a1`; SkipDBv2 Regression run #457 passed source contracts, configure, complete build, and all regression tests.
- Scope: the two-file commit gate in `udb/UDBTransaction.c`.
- The transaction path synced and finalized both journals before persisting the recovery marker, but did not run the existing strict journal parser against both prepared logs. A malformed interior journal record could therefore be declared recoverable before apply/replay later rejected it.
- Validate the complete index and record journals after prepare and before writing marker 1. Add a lifecycle regression that corrupts a persisted journal record while preserving its tail terminator, verifies the commit is rejected, and confirms the preceding committed value survives reopen while the failed write stays absent. Update the existing Actions source/runtime contracts only; no new files, exported symbols, or CMake changes.
- No local C compilation or CTest run is claimed; mainline CI is the compile/runtime gate.

---

## 2026-11-07 07:00 CST — Refuse UDB record moves onto occupied extents

- Baseline: main commit `234cd05b9fd81b71f4e6395deefca51b3775feaf`; SkipDBv2 Regression run #458 passed source contracts, configure, complete build, and all regression tests.
- Scope: record relocation validation in `udb/UDBRecordMove.c`.
- Bounds checks alone allowed `UDBRecord_moveToPos_()` to overwrite a live record if the requested target happened to be inside the file and large enough, even though the function is used by the compactor to move live rows into free extents. It also accepted header-shaped payload bytes as a potential source position.
- Walk the validated physical record chain to resolve exact source/target boundaries. Permit a different target only when its persisted record is an empty extent large enough for the source; same-position moves remain harmless no-ops. Extend the existing record-storage regression with a live-destination collision case and update Actions source contracts. No new files, public symbols, or CMake changes.
- No local C compilation or CTest run is claimed; mainline CI is the compile/runtime gate.

---

## 2026-11-07 07:01 CST — Repair record-storage regression build

- Baseline: main commit `8617adcdbb6704fc3b6933474cf6c665e4a2d027`; SkipDBv2 Regression run #459 failed during compilation because the new record-move regression referenced an undefined `has_value()` helper in `udb_record_storage.c`.
- Replace those helper calls with direct `Datum` length and byte comparisons. Keep the occupied-destination test intact and add workflow source-contract checks for the inline assertions. No production behavior changed in this repair; no new files or CMake changes.
- CI retry is required before further refactoring.

---

## 2026-11-07 07:03 CST — Fix alias-sensitive record-move regression

- Baseline: main commit `01b7d79e6ce8fcd189aa7a8009a80b9a7e720387`; SkipDBv2 Regression run #460 built successfully but failed `udb_record_storage_test`.
- The regression captured two `UDB_at_()` results before checking either one, but both Datum payload pointers refer to the shared records cursor buffer. Reading the destination replaced the source payload before it was asserted.
- Check the source bytes before the next cursor read, then read and validate the destination. The production change and its source contract remain unchanged; no new files or CMake changes.
- CI retry required before further refactoring.

---

## 2026-11-07 07:05 CST — Guard UDB diagnostic entry points

- Baseline: main commit `1f0dccea4366ab9f753f48a2ce56a8bf55f6c569`; SkipDBv2 Regression run #461 passed the full build and all 56 regression tests.
- Scope: public diagnostic helpers in `udb/UDB.c`.
- `UDB_show()` and `UDB_showIndex()` dereferenced the database facade unconditionally, so a NULL or zero-initialized database caused a crash despite the rest of the public API being defensive.
- Return without side effects when the facade or its required subobject is absent. Extend the existing record-API regression and workflow source contracts; no new files, exported symbols, or CMake changes.
- No local C compilation or CTest run is claimed; mainline CI is the compile/runtime gate.

---

## 2026-11-07 07:08 CST — Reject record-header writes beyond the data extent

- Baseline: main commit `357e616178b2da7117f40784b4a896ffbf054ead`; SkipDBv2 Regression run #462 passed source contracts, full build, and all 56 regression tests.
- Scope: physical bounds validation in `udb/UDBRecordWrite.c`.
- `UDBRecord_saveHeader()` rejected metadata offset zero but accepted any larger PID-sized offset up to `LONG_MAX`. In journaled mode, a caller-supplied position beyond EOF could extend the predicted data extent and commit a sparse gap or corrupt later record traversal.
- Reject positions beyond the current logical file end and reject existing-file positions whose full header would cross EOF; retain the exact-EOF case required for valid appends and guard signed-offset overflow. Extend the existing storage regression and Actions source contracts; no new files, public symbols, or CMake changes.
- No local C compilation or CTest run is claimed; mainline CI is the compile/runtime gate.

---

## 2026-11-07 07:11 CST — Enforce append-only record creation in journal mode

- Baseline: main commit `9cf85ee031e186a136a22d7bce580778c625de26`; SkipDBv2 Regression run #463 passed source contracts, full build, and all 56 regression tests.
- Scope: `UDBRecord_saveWithDatum_()` in `udb/UDBRecordWrite.c`.
- The non-journaling path already required a new record to start at EOF, but the journaling path skipped that same invariant. A caller holding a cached live-record position could therefore overwrite its persisted header/payload without updating the PID index or retiring the old row through the replacement path.
- Enforce the EOF precondition before either write mode and test that an existing record position is rejected without changing journal size, logical file extent, or the record cursor. Update the existing Actions source contract; no new files, public symbols, or CMake changes.
- No local C compilation or CTest run is claimed; mainline CI is the compile/runtime gate.

---

## 2026-11-07 07:12 CST — Repair append-only regression source contract

- Baseline: main commit `df418c95d15397aeaf521d49b9b25b2c269c6733`; SkipDBv2 Regression run #464 stopped at the source-contract step before configure/build.
- The new runtime diagnostic `record save accepted a non-append position` was accidentally registered as a required string in the production `UDBRecordWrite.c` contract, even though it correctly belongs to the existing `udb_record_storage.c` test.
- Move the string assertion to the test-file contract and remove it from the production-source contract. No C source or runtime-test logic changed in this repair.

---

## 2026-11-07 07:14 CST — Make UDB journal-path updates atomic

- Baseline: main commit `4891eff043e3d60d0d93254fb467917098340348`; SkipDBv2 Regression run #465 passed integration contracts, configure, full build, and all 56 tests.
- Scope: journal-path configuration in `udb/UDBLifecycle.c`.
- `UDB_setLogPath_()` updated the index and record journals sequentially. If deriving the second path failed (for example, allocation failure or an invalid basename), the first path could already have changed, leaving a split pair.
- Stage both derived paths using temporary `JFile` structs, then swap both only after both allocations/validations succeed. Add a regression that makes the second basename invalid and verifies neither live path changes. Update the existing Actions contracts; no new files, public symbols, or CMake changes.
- No local C compilation or CTest run is claimed; mainline CI is the compile/runtime gate.

---

## 2026-11-07 07:18 CST — Reject reserved metadata offsets in the record reader

- Baseline: main commit `c45f5e180859c4fbfb98602ebd9378b2588a6e77`; SkipDBv2 Regression run #466 passed integration contracts, configure, full build, and all 56 tests.
- Scope: the shared physical-extent predicate in `udb/UDBRecordRead.c`.
- The high-level record cursor, writer, removal, and relocation paths already reserve offset zero for the first-empty pointer, but the low-level header reader only checked file bounds. Crafted metadata bytes that matched a valid `UDBRecordHeader` could still be accepted as a physical record by a direct `UDBRecord_readHeader()` caller.
- Reject offsets below the first physical-record boundary and offsets that cannot be represented by the signed file-seek API. Add a regression that temporarily places a valid EMPTY header over the metadata bytes, verifies the reader rejects it, and restores the original bytes before continuing. Update existing source contracts only; no new files, exported symbols, or CMake changes.
- No local C compilation or CTest run is claimed; mainline CI is the compile/runtime gate.

---

## 2026-11-07 07:22 CST — Bound allocated PID slots to the seekable file range

- Baseline: main commit `7f9303f8488d00fabea1b5253e2ef89df0d7c23f`; SkipDBv2 Regression run #467 passed integration contracts, configure, full build, and all 56 tests.
- Scope: fixed-width PID-slot mapping/allocation in `udb/UDBIndexMap.c` and `udb/UDBIndexFreeList.c`.
- Offset validation checked only whether a slot's starting byte was at or below `LONG_MAX`; the final slot could still extend beyond the signed range accepted by `JFile_fwrite()`. The allocator could consequently hand out a PID that could never be persisted.
- Make the shared offset validator require the entire entry to fit, expose it through the existing internal index header, and reject both newly allocated and reused PIDs whose slot is not seekable. Extend the existing index-layer regression by setting a synthetic near-limit max PID and asserting allocation fails without creating a reservation. Update Actions contracts; no new files or CMake changes.
- No local C compilation or CTest run is claimed; mainline CI is the compile/runtime gate.

---

## 2026-11-07 07:24 CST — Make failed initial metadata writes safe to unwind

- Baseline: main commit `760546b81491bd847a08347b41cd6c295ff12e28`; SkipDBv2 Regression run #468 passed integration contracts, configure, full build, and all 56 tests.
- Scope: empty-file startup cleanup in `udb/UDBRecordsStorage.c`.
- The failure branch called `fclose(self->file->log)` unconditionally after a failed journal write. `JFile_fwrite()` may already close and null the journal stream when it cannot roll back a partial journal record, making that cleanup call undefined behavior and leaving the data stream open.
- Use the existing null-safe `JFile_close()` lifecycle routine to unwind both streams on this failure path. Update the source contract; no new files, public symbols, or CMake changes.
- No local C compilation or CTest run is claimed; mainline CI is the compile/runtime gate.

---

## 2026-11-07 07:26 CST — Keep PID preview within the allocatable index range

- Baseline: main commit `4ac85843f6bbd9b3a5ac150258c88c3b1c187eee`; SkipDBv2 Regression run #469 passed integration contracts, configure, full build, and all 56 tests.
- Scope: PID availability preview in `udb/UDBIndexFreeList.c`.
- The allocator now rejects a PID whose fixed-width slot would extend beyond `LONG_MAX`, but `UDBIndex_nextPid()` still returned that same unpersistable PID from the current index EOF (and returned a free-list top without validating its slot). Callers could therefore be told an identifier is available that allocation must reject.
- Reuse the shared full-slot offset validator for both the free-list and EOF preview paths. Extend the existing index-layer regression with a synthetic near-limit extent and assert preview and allocation both return the failure sentinel without reservations. Update source contracts; no new files or CMake changes.
- No local C compilation or CTest run is claimed; mainline CI is the compile/runtime gate.

---

## 2026-11-07 07:27 CST — Repair next-PID preview source contract

- Baseline: main commit `50ab4c3432a431a5f761d1aa0e46506ff23e44cb`; SkipDBv2 Regression run #470 failed at the integration-contract step before configure/build.
- The assertion text `next-PID preview returned a slot beyond LONG_MAX` was incorrectly attached to the production `UDBIndexFreeList.c` contract even though it is emitted by `tests/udb_index_layers.c`.
- Remove it from the production contract; keep the existing test-file assertion. No C source or test behavior changed.

---

## 2026-11-07 07:29 CST — Bound JFile reads to signed seek offsets

- Baseline: main commit `9111b2a76c306f5358219bcc1dcd21a4184690e4`; SkipDBv2 Regression run #471 passed source contracts, configure, full build, and all 56 tests.
- Scope: low-level read bounds in `jfile/JFile.c`.
- `JFile_fwrite()` rejects byte counts above `LONG_MAX` and spans where `pos + total` exceeds the signed seek range. `JFile_fread()` checked only multiplication overflow and a negative starting position, then called `fseek/fread` with an impossible span; this could move the stream beyond the representable logical cursor and leave `self->pos` stale when `ftell()` failed.
- Apply the same checked total-span limits before touching the stream. Extend the existing journal regression to verify both an oversized request and a start-plus-length overflow are rejected without changing stream or logical position. Update source contracts only; no new files or CMake changes.
- No local C compilation or CTest run is claimed; mainline CI is the compile/runtime gate.

---

## 2026-11-07 07:31 CST — Revalidate storage during safe reverse iteration

- Baseline: main commit `4c50788e432ded69288ad191ffe7312e1c4ca35d`; SkipDBv2 Regression run #472 passed integration contracts, configure, full build, and all 56 tests.
- Scope: `LIST_SAFEREVERSEFOREACH` in `basekit/List_inline.h`.
- The reverse-safe macro validated storage only when iteration started. If callback code made the List metadata inconsistent while retaining its struct (for example, reducing `memSize` below `size`), subsequent iterations still ran; `List_size()` read unchecked metadata and `List_at_()` returned NULL repeatedly instead of terminating.
- Revalidate the storage invariant on every loop pass and clamp against the validated struct's `size`. Add a callback-mutation regression in the existing List bounds test and update the two existing Actions source contracts. No new files or CMake changes.
- No local C compilation or CTest run is claimed; mainline CI remains the compile/runtime gate.

---

## 2026-11-07 07:56 CST — Stop safe reverse iteration after cursor invalidation

- Baseline: main commit `fd7a0b31a8ffbdc6815bb95065f0880653109fea`; SkipDBv2 Regression run #473 passed integration contracts and the full build but failed `list_storage_bounds_test`.
- Scope: `LIST_SAFEREVERSEFOREACH` in `basekit/List_inline.h`.
- When a callback reduced the list to one slot, the reverse cursor still referred past the new logical end. Clamping that cursor to the new size caused another callback invocation at index zero, contrary to the existing mutation regression. Stop traversal when the cursor is beyond the current size instead of remapping it to an unrelated slot; keep per-iteration storage validation.
- Update the existing Actions source contract to require the early-stop guard. No new files or CMake changes.
- No local C compilation or CTest run is claimed; mainline CI remains the compile/runtime gate.

---

## 2026-11-07 08:43 CST — Retry interrupted durability barriers

- Baseline: main commit `73b2c1b693f87abfd3d6d3e9762f7ca39a310c5a`; SkipDBv2 Regression run #474 passed source contracts, configure, full build, and all regression tests.
- Scope: `JFile_hardSyncFileDescriptor_()` in `jfile/JFileSync.c`.
- `fsync()` and `F_FULLFSYNC` may return `EINTR`. The shared helper treated this transient interruption as a fatal persistence error, causing callers to close an otherwise usable data or journal stream.
- Retry only interrupted durability calls; preserve existing error propagation for all other failures. Add the retry invariant to the existing Actions source contract. No new files or CMake changes.
- No local C compilation or CTest run is claimed; mainline CI remains the compile/runtime gate.

---

## 2026-11-07 08:46 CST — Validate UArray layout before sorting

- Baseline: main commit `2b2df6aa6252c894b66d79f6c3d6d6e250e20277`; SkipDBv2 Regression run #475 passed source contracts, configure, full build, and all regression tests.
- Scope: `UArray_sort()` and `UArray_sortBy_()` in `basekit/UArray.c`.
- Both sort entry points checked only for a null buffer, not whether `itemType` and `itemSize` agreed. A malformed public UArray struct could make `qsort()` use an element width larger than its declared backing layout, allowing out-of-bounds memory access and invalidating the cached hash.
- Reuse `UArray_hasValidItemLayout_()` before hashing or sorting; extend the existing malformed-layout regression and Actions source contracts. No new files or CMake changes.
- No local C compilation or CTest run is claimed; mainline CI remains the compile/runtime gate.

---

## 2026-11-07 08:47 CST — Repair UArray layout-contract call count

- Baseline: main commit `3e71d7304d1fb52a1e09c9ea31fe390779f39d1a`; SkipDBv2 Regression run #476 stopped at source-contract validation before configure/build.
- The `UArray_sort()` and `UArray_sortBy_()` layout guards correctly added two calls to `UArray_hasValidItemLayout_()`, but the existing exact-count contract still expected one definition plus three earlier callers.
- Update the contract to expect one definition and five guarded callers (six total occurrences). No additional C or test changes, new files, or CMake changes.

---

## 2026-11-07 09:04 CST — Reject malformed UArray stack-range layouts

- Baseline: main commit `d50b95ba0f2abd1d4262c0125d365c3b9d9e9579`; SkipDBv2 Regression run #477 passed.
- Scope: `UArray_stackRange()` in `basekit/UArray.c`.
- The stack-range constructor checked only `itemSize` and count geometry. A malformed public UArray whose `itemType` width disagreed with `itemSize` could still produce a typed view over invalid storage, allowing subsequent typed access beyond the backing bytes.
- Reuse `UArray_hasValidItemLayout_()` before offset arithmetic; normalize malformed input to a safe empty byte-array view while preserving out-of-range behavior for otherwise valid arrays. Add malformed-layout and valid-slice regression cases to the existing test, and update the Actions source contract. No new files or CMake changes.
- No local C compilation or CTest run is claimed; mainline CI remains the compile/runtime gate.

---

## 2026-11-07 09:12 CST — Track realloc growth in allocator high-water accounting

- Baseline: main commit `f0cb6e3992ac99efff7d320aeeaba990750e7b46`; SkipDBv2 Regression run #478 passed.
- Scope: tracked allocation resize in `basekit/Common.c`.
- `MemoryBlock_newWithSize_file_line_()` updated `maxAllocatedBytes`, but successful `MemoryBlock_reallocToSize_()` only changed the current byte total. A realloc that grew the live heap could therefore leave the advertised peak-memory statistic stale until a later allocation happened to exceed it.
- Update the high-water mark after the realloc succeeds and the current total is committed. Extend the existing `common_alloc.c` regression and Actions source contract to assert that growing 16 bytes to 32 immediately raises the peak to 32. No new files or CMake changes.
- No local C compilation or CTest run is claimed; mainline CI remains the compile/runtime gate.

---

## 2026-11-07 09:16 CST — Clear IPC envelope views on invalid input

- Baseline: main commit `27ec1a6a99d3b91adcc5ed9bad1e7dc50f30e789`; SkipDBv2 Regression run #479 passed.
- Scope: `skipdb_ipc_parse_envelope()` in `mgr/ipc_protocol.c`.
- The parser returned immediately for a null body or zero-length frame before clearing a non-null output structure. Callers that reused a frame view could retain pointers and lengths from a prior successful parse after a failed parse.
- Reject a null output pointer first, clear valid output storage next, and only then reject null/empty input. Add regression coverage for both invalid-input paths and update the Actions source contract to enforce the guard order. The existing CMake/CTest registration remains unchanged.
- No local C compilation or CTest run is claimed; mainline CI remains the compile/runtime gate.

---

## 2026-11-07 09:17 CST — Repair IPC parser source-contract delimiter

- Baseline: main commit `45e1a75f8eea3af4598cd05c3e6bbb9bfd581bae`; SkipDBv2 Regression run #480 failed during source-contract preflight, before configuration or compilation.
- The new IPC parser guard-order contract searched for a literal backslash-plus-\n sequence because its function-boundary delimiter was escaped twice. The contract therefore failed to extract the function body even though the parser's guard order was correct.
- Correct the workflow delimiter to Python's newline escape so the preflight checks the intended function body. This is a CI-only correction; no new files and no C behavior changes.

---

## 2026-11-07 09:20 CST — Reject malformed UArray layouts in Datum conversion

- Baseline: main commit `e6d63bc90e0d8a99b7e34c8926f02002be1b53f8`; SkipDBv2 Regression run #481 passed.
- Scope: `Datum_FromUArray_()` in `basekit/Datum.c`.
- The conversion checked multiplication overflow and the backing pointer, but not whether `itemSize` matched the declared `itemType`. A corrupted UArray could therefore be exposed as an incorrectly sized raw Datum byte span.
- Validate the declared CTYPE width before returning `UArray_bytes()`. Extend the existing Datum boundary test to preserve valid conversions and reject mismatched item widths, and add a workflow source contract for guard ordering. The CMake/CTest test is already registered; no new files.
- No local C compilation or CTest run is claimed; mainline CI remains the compile/runtime gate.

---

## 2026-11-07 09:22 CST — Repair Datum source-contract delimiter

- Baseline: main commit `957c317e258783144fa0b2b647254f12167dfd46`; SkipDBv2 Regression run #482 failed in source-contract preflight before CMake configuration or compilation.
- The new `Datum_FromUArray_()` guard-order contract had the same double-escaped Python newline delimiter issue as a previous contract. Its function-body extraction returned empty and incorrectly failed the preflight.
- Correct the workflow delimiter to the Python newline escape; the production guard and regression assertions remain unchanged. No new files.

---

## 2026-11-07 09:23 CST — Include active DST bias in Windows timezone offsets

- Baseline: main commit `737d63b496a45e427cf64f339aab7b408a6638e5`; SkipDBv2 Regression run #483 passed.
- Scope: Windows adapter in `basekit/PortableGettimeofday.c`.
- The adapter reported only `TIME_ZONE_INFORMATION.Bias`, omitting `StandardBias` or `DaylightBias` when Windows reported the current seasonal mode. Callers requesting `struct timezone` therefore received the base-zone offset instead of the active local offset during adjusted time.
- Apply the bias corresponding to `TIME_ZONE_ID_STANDARD` or `TIME_ZONE_ID_DAYLIGHT`, retaining the base bias for `TIME_ZONE_ID_UNKNOWN`. Extend the existing Actions source contract; no new files or test/CMake changes. The Windows branch cannot be executed by the Linux runner.
- No local Windows or C compilation is claimed; mainline CI remains the available validation gate.

---

## 2026-11-07 09:25 CST — Recover Gaussian sampling from a degenerate RandomGen state

- Baseline: main commit `99ec4482a505c627e05f900d8f8eb84dc7b4437d`; SkipDBv2 Regression run #484 passed.
- Scope: `RandomGen_gaussian()` in `basekit/RandomGen.c`.
- A zero-initialized/corrupted but index-in-range MT state produces only zero random values, so the Gaussian rejection loop can reject forever because its radius remains outside the unit circle.
- Detect an all-zero state before sampling and restore the deterministic fallback seed. Extend the existing state-bounds test to compare its result with a freshly seeded generator and make the Actions source contract enforce recovery before rejection sampling. The CMake test target already exists; no new files.
- No local C compilation or CTest run is claimed; mainline CI remains the compile/runtime gate.

---

## 2026-11-07 09:30 CST — Resolve DST when parsing local Date strings

- Baseline: main commit `8e5d94a562ea0cd8e554fc4c7584eda6aad7fb16`; SkipDBv2 Regression run #485 passed.
- Scope: `EmptyTM()` and `Date_fromString_format_()` in `basekit/Date.c`.
- The parser seeded its `struct tm` from local midnight on January 1, 1970, then reset calendar fields but retained that template's `tm_isdst`. On a DST-observing timezone, parsing a summer local time could force standard time and shift the resulting instant by an hour.
- Set `tm_isdst = -1` in the parsing template so `mktime()` resolves the correct seasonal offset for the parsed date. Extend the existing date test with an isolated POSIX DST timezone regression and update the workflow source contract. Existing CMake/CTest registration is unchanged; no new files.
- No local C compilation or CTest run is claimed; mainline CI remains the validation gate.

---

## 2026-11-07 09:32 CST — Preserve platform timezone declarations in DST regression

- Baseline: main commit `fde8f11122c0c2bb1a909feaa646c169c281e421`; Regression run #486 failed while compiling `date_time_init_test`, before tests ran.
- The test-only feature macro `_POSIX_C_SOURCE` hid glibc's non-POSIX `struct timezone` declaration, breaking existing Date test declarations. Replace it with `_DEFAULT_SOURCE` so the DST fixture exposes `setenv/tzset` while retaining the timezone compatibility types.
- Production Date logic and workflow contracts are unchanged; no new files.

---

## 2026-11-07 09:33 CST — Report deterministic errors for invalid IPC socket paths

- Baseline: main commit `fbcb6aca8b763677dc7dde1ba71602a4a3625f3d`; SkipDBv2 Regression run #487 passed.
- Scope: path validation in `skipdb_ipc_connect_unix()` in `mgr/ipc_transport.c`.
- Null and empty socket paths, and paths too long for `sockaddr_un.sun_path`, previously returned `-1` without setting `errno`, leaving callers with stale/unrelated diagnostics.
- Report `EINVAL` for null/empty paths and `ENAMETOOLONG` for overlong paths. Tighten the existing Unix-socket transport regression and the Actions source contract. Existing CMake/CTest registration is unchanged; no new files.
- No local compilation or CTest run is claimed; mainline CI remains the validation gate.

---

## 2026-11-07 09:35 CST — Harden the legacy array-heap container

- Baseline: main commit `50eb8be552cd66893429b25131c3f0bc6dd435e5`; SkipDBv2 Regression run #488 passed.
- Scope: `mgr/array-heap.c` and its existing public header.
- The container wrote before ensuring capacity, assigned `realloc()` directly to the owning pointer, used an invalid `-1` pointer-allocation check, stored pointers through `size_t`, and lacked null/state/overflow guards.
- Store elements as `void *`, grow before a full-capacity write, use a temporary realloc result, reject invalid capacities and integer overflow, and make cleanup null-safe/idempotent. Add a focused regression target and Actions source-contract checks; no change to active daemon behavior or new production files.
- No local compilation or CTest run is claimed; CI is the validation gate.

---

## 2026-11-07 09:40 CST — Reject missing DBAPI list callbacks before parsing

- Baseline: main commit `e4abcfeb795752e0deb8738e821a3048a11d25f2`; SkipDBv2 Regression run #489 passed.
- Scope: `mgr/dbapi_result.c` list-result argument validation and the existing `dbapi_protocol.c` regression.
- The list parser dereferenced its callback for the first item without checking whether the function pointer was NULL; malformed internal calls could crash the CLI instead of returning a normal argument error.
- Reject NULL client, prefix, or callback before consuming any response bytes. Add a focused NULL-callback regression and enforce both the guard and test in the Actions source contract. The existing CMake/CTest target already covers this test file; no new files.
- No local C compilation or CTest run is claimed; CI remains the validation gate.

---

## 2026-11-07 09:47 CST — Preserve absolute IPC deadlines across EINTR

- Baseline: main commit `1f15eb00a065392db5904d7b1032fc385b485e70`; SkipDBv2 Regression run #491 passed.
- Scope: exact socket I/O retries in `mgr/ipc_transport.c`.
- Direct `recv()` and `send()` interruptions retried immediately without rechecking the monotonic deadline. A continuous signal stream could therefore keep a caller in the retry loop beyond its configured timeout even though poll-based retries already used an absolute deadline.
- Recheck monotonic time after EINTR, return the existing clock-failure code if the clock cannot be read, and stop retrying when the deadline has expired. Strengthen the existing Actions source contract for both read and write paths; existing IPC transport CTest registration remains unchanged. No new files.
- No local compilation or CTest run is claimed; mainline CI remains the validation gate.

---

## 2026-11-07 09:49 CST — Return the durability barrier result

- Baseline: main commit `b250e2433620df062a002b5a1be6cdf24e11215b`; SkipDBv2 Regression run #492 passed.
- Scope: `JFile_hardSyncFileDescriptor_()` in `jfile/JFileSync.c`.
- The helper retried interrupted `fsync()`/`F_FULLFSYNC` calls but fell off the end of a non-void function instead of returning the final status. Callers relied on that undefined return value to decide whether to close persistence streams and abort a transaction.
- Return `result` explicitly, add a valid-descriptor regression to the existing journal test, and enforce function-scoped return ordering in the Actions contract. Existing test target and headers already cover the helper; no new files.
- No local C compilation or CTest run is claimed; mainline CI is the validation gate.

---

## 2026-11-07 09:50 CST — Register database cursors at construction

- Baseline: main commit `f75050a27a81195edda38caf37d537d61201f0b5`; SkipDBv2 Regression run #493 passed.
- Scope: SkipDB cursor ownership and registration lifecycle.
- The exported `SkipDBCursor_newWithDB_` assigned an owning database pointer without registering the cursor in the owner's cursor list. Direct constructor callers could therefore be missed by cache protection and database teardown.
- Route binding through `SkipDBCursor_sdb_`, reject construction when owner registration fails, and remove the factory's duplicate list append. Extend the existing core-layer regression and CI source contracts; its CMake/CTest target already exists. No new files.
- No local compilation or CTest run is claimed; mainline CI remains the validation gate.

---

## 2026-11-07 09:52 CST — Reject PID lookups against a closed index file

- Baseline: main commit `8047b18c8ad0c2ae6f04144b54b2ece5ea5d272c`; SkipDBv2 Regression run #494 passed.
- Scope: `UDBIndex_posForPid_()` in `udb/UDBIndexMap.c`.
- The lookup validated cached size and the `JFile` object but not its open data stream. After close, a lookup returned the same zero used for an unmapped PID and changed the cached seek position before the lower-level read rejected the closed stream.
- Require an open index file before computing or seeking a slot. Extend the existing index-layer test to prove closed-file lookups preserve cached position/size, and make the Actions source contract enforce the descriptor guard. Existing CMake/CTest target is unchanged; no new files.
- No local C compilation or CTest run is claimed; mainline CI remains the validation gate.

---

## 2026-11-07 09:55 CST — Retry interrupted commit-marker directory syncs

- Baseline: main commit `a14e65525c9c1482ba377000d137de3743c46328`; SkipDBv2 Regression run #495 passed.
- Scope: `UDBCommitMarker_syncParentDirectory()` in `udb/UDBCommitMarker.c`.
- The commit-marker durability path called `fsync()` directly, unlike the shared persistence barrier which retries `EINTR`. A harmless signal interruption therefore aborted database startup/commit instead of completing the required directory sync.
- Route the directory descriptor through `JFile_hardSyncFileDescriptor_()` so both marker-directory and journal/file durability barriers share the same platform-specific sync and interruption handling. The existing `commit_marker_test` still covers directory syncing; Actions now enforces the shared helper call. No new files.
- No local compilation or CTest run is claimed; mainline CI remains the validation gate.

---

## 2026-11-07 09:57 CST — Use monotonic time for compaction budgets

- Baseline: main commit `8b5e96b65fe334202e265daf0a46f3a5368db602`; SkipDBv2 Regression run #496 passed.
- Scope: time-sliced compaction scheduling in `udb/UDBCompact.c`.
- The budget used `gettimeofday()`, so wall-clock corrections could abruptly end a time slice even when the requested budget had not elapsed. The negative-elapsed guard prevented overruns but could not prevent premature stops.
- Measure the operation budget with `CLOCK_MONOTONIC` and nanosecond precision; retain the existing fail-safe behavior if clock reads fail. Extend the existing source contract. No new files or CMake changes.
- No local compilation or CTest run is claimed; mainline CI remains the validation gate.

---

## 2026-11-07 09:59 CST — Align compaction contract with monotonic timing

- Baseline: main commit `00ff2c40036da65f6010e59c4764fb6fc1a98b58`; SkipDBv2 Regression run #497 failed in the XY-C1 source-contract step.
- The compaction implementation had switched from `gettimeofday()` to `clock_gettime(CLOCK_MONOTONIC)`, but the pre-existing contract still required the removed wall-clock call.
- Remove the stale assertion; keep both monotonic-clock assertions. No C-source changes.

---

## 2026-11-07 10:02 CST — Preserve cached headers when the record stream is closed

- Baseline: main commit `faa375bd8bf8d92ae35284fbf2d5fceff25978fc`; SkipDBv2 Regression run #498 passed.
- Scope: `UDBRecord_readHeader()` in `udb/UDBRecordRead.c`.
- The reader checked the cached file length but not whether the underlying `FILE *` was open. After close, a read failure was classified as a corrupt header and zeroed the caller's cached metadata while also seeking the closed `JFile` cursor.
- Reject closed streams before any seek or header mutation. Add a closed/reopened stream regression to the existing record-storage test and enforce the descriptor guard in the Actions source contract. No new files or CMake changes.
- No local C compilation or CTest run is claimed; mainline CI remains the validation gate.

---

## 2026-11-07 10:04 CST — Reject tagged pointers wider than intptr_t

- Baseline: main commit `55a299d93bd842bb5f6404cba0f708dd0e7a4abf`; SkipDBv2 Regression run #499 passed.
- Scope: tagged pointer decoding in `basekit/BStream.c`.
- The 8-byte pointer-tag path cast an `int64_t` directly to `intptr_t`. On 32-bit builds, values outside the platform's signed pointer-width range were silently truncated, producing a different pointer value without setting the stream error.
- Decode into a temporary, preserve existing read errors, and reject out-of-range values before the cast. Add an architecture-sensitive regression to the existing BStream boundary test and enforce the guard in the Actions source contract. Existing CMake/CTest target is unchanged; no new files.
- No local compilation or CTest run is claimed; mainline CI remains the validation gate.

---

## 2026-11-07 10:06 CST — Reject record creation with invalid end offsets

- Baseline: main commit `2ed8d3fb51c749fc7828fb15f652b44001719c28`; SkipDBv2 Regression run #500 passed.
- Scope: `UDBRecords_newRecord()` in `udb/UDBRecordsCursor.c`.
- The helper checked that the record stream was open but not that its cached end offset was valid. When `maxPos < 0`, `UDBRecord_setPosToEnd()` correctly declined to update the cursor, yet `UDBRecords_newRecord()` still returned the shared record object at its previous position as though new-record creation succeeded.
- Reject invalid end metadata before returning the cursor. Extend the existing record-storage regression to verify that a rejected request preserves the old cursor and that normal EOF creation still succeeds. Add source-contract checks; existing CMake/CTest wiring is unchanged.
- No local C compilation or CTest run is claimed; mainline CI remains the validation gate.

---

## 2026-11-07 10:08 CST — Reject PID mappings inside the data-file header

- Baseline: main commit `8d0258b9f2a5d4a793705ab918e8deaa9e1473d4`; SkipDBv2 Regression run #501 passed.
- Scope: `UDBIndex_setPos_forPid_()` in `udb/UDBIndexMap.c`.
- The setter rejected offsets beyond `LONG_MAX` but accepted nonzero offsets smaller than `sizeof(PID_TYPE)`. Those bytes are occupied by the records file's leading `firstEmptyPos` field, so an invalid mapping could overwrite metadata and release a PID reservation.
- Reject nonzero record offsets before the data header while preserving zero as the unmapped sentinel. Extend the existing index-layer regression to assert both reservations remain intact after rejection; add source-contract checks. No new files or CMake changes.
- No local C compilation or CTest run is claimed; mainline CI remains the validation gate.

---

## 2026-11-07 10:10 CST — Preserve PID ownership when free-list insertion fails

- Baseline: main commit `b967358bf153983474948e930bf94ed9f033eece`; SkipDBv2 Regression run #502 passed.
- Scope: PID reservation and reusable-hole transitions in `udb/UDBIndexFreeList.c`.
- `UDBIndex_releasePid_()` removed the reservation before attempting a potentially allocating free-list insertion. If the list could not grow, the function returned failure after losing both ownership states. Hole normalization also removed existing entries before allocating their replacement.
- Secure/deduplicate an existing hole without allocation, insert before unreserving, and leave the reservation intact if insertion fails. Add a focused failed-insertion regression by temporarily making the existing list fail its storage-validity guard, plus source contracts enforcing the handoff order. No new files or CMake changes.
- No local C compilation or CTest run is claimed; mainline CI remains the validation gate.

---

## 2026-11-07 10:18 CST — Preserve malloc-compatible alignment in tracked allocations

- Baseline: main commit `39ef3e80488b4c0876028f07a9635e6f41c58c50`; SkipDBv2 Regression run #503 passed.
- Scope: private `MemoryBlock` payload layout in `basekit/Common.c` and the existing `common_alloc` regression.
- `MemoryBlockToPtr()` returns the byte immediately after the tracking header. The prior padding left `sizeof(MemoryBlock)` at 56 bytes on common 64-bit ABIs, so a `calloc()`-aligned base could yield an 8-byte-offset payload and violate `malloc()` alignment for `long double` and other fundamental types in `IO_CHECK_ALLOC` builds.
- Replace byte padding with an alignment-bearing union member, making the private header's size a multiple of the strongest included fundamental alignment. Extend the existing regression to store a `long double` and check both allocation and reallocation alignment. Update the Actions source contracts; the existing CMake target already compiles `Common.c` with `IO_CHECK_ALLOC`. No new files or CMake changes.
- No local C compilation or CTest run is claimed; mainline CI remains the validation gate.

---

## 2026-11-07 10:37 CST — Make UArray insertion preflight and alias handling transactional

- Baseline: main commit `81ddf68ab344420dfb6dd28d6682c773196ac589`; SkipDBv2 Regression run #504 passed.
- Scope: `UArray_at_putAll_()` and the existing `uarray_stack_resize` regression.
- The insertion path checked only nonzero widths and multiplication bounds. Mismatched CTYPE/itemSize metadata could therefore pass preflight, resize the destination, and move its tail before `UArray_copyItems_()` rejected the source or destination. Validate both complete item layouts before any mutation.
- The overlap calculation excluded the allocated trailing NUL. A borrowed view beginning exactly at `self->data + selfBytes` could be overwritten by the tail move or invalidated by reallocation. Include this one-past-logical-end byte in overlap detection and snapshot it before resizing.
- Extend the existing regression for malformed source/destination layouts and an insertion sourced from the owner's terminator. Update the Actions source contracts. The existing CMake target already builds the test; no new files or CMake changes.
- No local C build or CTest run is claimed; mainline CI remains the validation gate.

---

## 2026-11-07 10:42 CST — Refresh UArray validator call-count contract

- Baseline: main commit `80651a37a9b4f4017a6bd4d61a63da1974434ccf`; its Actions run failed in the source-contract step before configuration/build.
- The new `UArray_at_putAll_()` preflight adds two valid calls to `UArray_hasValidItemLayout_()`. The existing Actions contract still hard-coded the previous total of seven (one definition plus six call sites), so the refactor was rejected despite both changed-file needle sets passing.
- Update the in-place source contract to nine occurrences (one definition plus eight guarded call sites). No source, test, CMake, or file-path changes.

---

## 2026-11-07 10:45 CST — Bound Gaussian rejection and recover unusable RandomGen states

- Baseline: main commit `4c9775873b7b86b6b972bf32def21fbe5ed59228`; SkipDBv2 Regression run #42330602360 passed.
- Scope: `RandomGen_gaussian()` and the existing `random_gen_state_bounds` regression.
- The Gaussian polar sampler used an unbounded rejection loop. Its earlier all-zero-state recovery did not cover a nonzero but degenerate/corrupted MT state; such a state could keep producing points outside the unit circle and stall callers indefinitely.
- Cap consecutive rejected pairs at 128, then restore the deterministic default seed and continue. Normal MT19937 rejection behavior is unchanged unless the state fails to produce a usable pair for the full bound.
- Extend the existing test with a nonzero all-ones state. The test verifies recovery yields the same Gaussian sample and subsequent PRNG state as a freshly default-seeded generator. Update both static and function-order Actions contracts. The existing CMake target is reused; no new files or CMake changes.
- No local C build or CTest run is claimed; mainline CI remains the validation gate.

---

## 2026-11-07 11:02 CST — Detach List removal targets from shared backing storage

- Baseline: main commit `94974e71a66fdd59836c5088ee5b18de940d715e`; SkipDBv2 Regression run #42330825341 passed.
- Scope: `List_removeItems_()` in `basekit/List_inline.h` and its existing `list_storage_bounds` regression.
- When two List structs shared an allocation, the old path cloned the source for traversal but still removed items from the shared destination buffer. Automatic compaction could reallocate/free that buffer while the other view retained a dangling pointer; without compaction, the source view was silently mutated.
- Detach the destination into an allocation-checked copy before removals, so the source view remains stable through threshold compaction. Extend the existing regression to check retained source contents, destination results, and immediate-compaction behavior for identical backing views. Update static and function-order Actions source contracts. Existing CMake/CTest target is reused; no new files.
- No local C build or CTest run is claimed; mainline CI remains the validation gate.

---

## 2026-11-07 11:08 CST — Bound Sorting against adversarial median-of-three partitions

- Baseline: main commit `fb8d1667949043a99cb9d26922d8a483db9b8e84`; SkipDBv2 Regression run #508 passed.
- Scope: `Sorting_context_comp_swap_size_type_()` and its existing `sorting_boundaries` regression.
- The three-way median-of-three quicksort avoided duplicate-heavy degeneration and bounded recursive stack depth, but a deterministic even-then-odd permutation still drove repeated highly unbalanced partitions and quadratic comparisons.
- Add an introspective depth budget of twice floor(log2(n)); when exhausted, heapsort the active range. Normal inputs retain the existing sorted fast path, median-of-three partitioning, and duplicate grouping. Add a 4096-element killer-permutation regression with a comparison ceiling; update Actions source/function-order contracts. Existing CMake target is reused; no new files.
- No local C build or CTest run is claimed; mainline CI remains the validation gate.

---

## 2026-11-07 11:30 CST — Commit calendar conversions only after successful mktime

- Baseline: main commit `e9d58845d1eb6a323a59e7c805abd4cb40bb8e3f`; SkipDBv2 Regression run #509 passed.
- Scope: `Date_fromLocalTime_()`, `Date_fromString_format_()`, and all calendar component setters in `basekit/Date.c`, plus the existing `date_time_init` regression.
- These entry points published `mktime()`'s `-1` failure sentinel into the Date even when errno identified an unrepresentable calendar value; component setters could replace a valid prior timestamp with an overflow sentinel.
- Centralize conversion through a checked, copy-first helper that preserves input state on failure, accepts the valid `time_t(-1)` case when errno remains clear, and commits normalized calendar fields and Date timestamps only on success. Add an architecture-aware overflow regression and Actions source contracts. Existing CMake test target is reused; no new files.
- No local C build or CTest run is claimed; mainline CI remains the validation gate.

---

## 2026-11-07 11:35 CST — Align Date setter contract with copy-first calendar conversion

- Baseline: main commit `6b41c8ecec634e21d62aca0f03dcf6e1001e15fa`; SkipDBv2 Regression run #510 failed in the Actions source-contract step.
- The Date setter implementation now changes a private `struct tm calendar` copy and commits only after checked conversion, but the static contract still required the removed in-place assignment `tm->tm_year = tmYear;`.
- Update the existing Date source contract to require `calendar.tm_year = tmYear;`. No implementation, test, CMake, or file-path changes.

---

## 2026-11-07 11:38 CST — Correct Date helper source-contract boundary

- Baseline: main commit `32938d6d2f9c6e41aba802d74f8b916cc2499e67`; run #510 confirmed the preceding Date setter contract mismatch.
- The new helper contract searched for a literal backslash-plus-`n` instead of the newline preceding `Date_new`, leaving its source slice empty. Correct the delimiter so CI checks the actual helper body and its error/commit ordering.
- No C implementation, test, CMake, or file-path changes.

---

## 2026-11-07 11:40 CST — Refresh Date parser source contract for checked conversion

- Baseline: main commit `c030c221549bc98a8d20786302b3fb7f882b83b6`; run #511 failed in the static source-contract step on the old `io_strptime` guard.
- The parser now checks both parse success and `Date_calendarToTime_()` before committing the timestamp. Replace the obsolete exact-line needle and add static assertions for the shared mktime error boundary and its regression case.
- No C implementation, test, CMake, or file-path changes.

---

## 2026-11-07 11:42 CST — Align Date local-time guard contract with checked conversion

- Baseline: main commit `0bceac25cbceed7a73f156ae0e09807feff196cd`; run #513 reached the Date entry-point contract and rejected its obsolete exact guard string.
- `Date_fromLocalTime_()` now validates the receiver and calendar pointer in a combined guard with the checked conversion. Update the source contract to match the guard's new boundary without weakening its ordering check.
- No C implementation, test, CMake, or file-path changes.

---

## 2026-11-07 11:46 CST — Stop futile SHash growth for unplaceable full-hash collisions

- Baseline: main commit `33c777ccb424b99ff736f8006f3c5fc96c6d3b27`; SkipDBv2 Regression run #514 passed.
- Scope: `basekit/SHash.c`, the existing `tests/shash_compact.c` fixture, and its existing Actions source contract.
- A failed insertion with two resident keys sharing the incoming key's full hash recursively grew the cuckoo table through every size up to (2^{20}), although the two candidate slots are a deterministic function of that full hash and therefore remain the same relative to every table size. The third key can never be placed.
- Detect the two occupied candidate slots and confirm both residents have the identical full hash before calling `SHash_grow()`. This preserves the existing table and its entries while avoiding futile repeated allocations. Extend the constant-hash regression through `SHash_at_put_()`. Existing CMake/CTest target reused; no new files.
- No local C build or CTest run is claimed; mainline CI is the validation gate.

---

## 2026-11-07 11:48 CST — Stop futile PHash growth for identical pointer hashes

- Baseline: main commit `514102b8d054386a9f283ed0b163a499f438484b`; SkipDBv2 Regression run #515 passed after the SHash collision guard.
- Scope: `basekit/PHash.c`, the existing `tests/phash_compact.c` fixture, and its existing Actions source contract.
- On 64-bit targets, `PHash_hash()` truncates the transformed pointer to 32 bits. Three distinct pointer values can therefore share the exact hash and are restricted to the same two cuckoo slots at every table size. The old failed-insert path repeatedly grew to the maximum table despite the collision being unresolvable.
- Detect both occupied candidates and verify their full hash matches the incoming pointer before growing. Add a 64-bit regression using distinct pointer values differing only above the retained hash bits; the 32-bit transform is injective and skips this case. Existing CMake/CTest target reused; no new files.
- No local C build or CTest run is claimed; mainline CI remains the validation gate.

---

## 2026-11-07 11:51 CST — Guard UArray bit-count result overflow

- Baseline: main commit `52101f5e858b0d8869aa6e60c6b2822237ff5a23`; SkipDBv2 Regression run #516 passed.
- Scope: `basekit/UArray_math.c` bit-count path, the existing `tests/uarray_bit_access.c` regression, and its existing Actions source contract.
- `UArray_bitCount()` accumulated up to eight set bits per byte into `size_t` without first proving that `byteCount * 8` is representable. A malformed but multiplication-valid UArray geometry could overflow the returned count and drive the scan beyond its representable result range.
- Reject `byteCount > SIZE_MAX / 8` before scanning; add a sentinel-backed regression that reaches the overflowing geometry without allocating or reading a huge buffer. Existing CMake/CTest target reused; no new files.
- No local C build or CTest run is claimed; mainline CI is the validation gate.

---

## 2026-11-07 11:52 CST — Repair UArray bit-count Actions contract placement

- Baseline: main commit `15a34fe809d5b35d7c0cff0aaa8614ca0f301e99`; run #517 failed before configure because a Python ordering assertion had been inserted inside the workflow's source-contract dictionary.
- Remove the misplaced executable assertion from the dictionary while retaining the explicit source guard and regression needles in the existing contract list. The C fix and test remain unchanged.
- No CMake or file-path changes.

---

## 2026-11-07 12:10 CST — Preserve embedded NUL across UTF-16/UTF-32 conversions

- Baseline: main commit `8789c433a53345b74b46d5493de577bf50672397`; SkipDBv2 Regression run #518 passed source contracts, configure, full build, and CTest.
- Scope: `UArray_asUTF16()` / `UArray_asUTF32()` in `basekit/UArray_utf.c`, their existing encoding-conversion regression, and the existing Actions source contract.
- Both conversion paths treated UArray's length-delimited content as a NUL-terminated string after conversion. The post-conversion scan truncated at the first U+0000, silently discarding all following UTF-16/UTF-32 elements even though the converter cursor had already established the exact output length.
- Remove the obsolete NUL-scan helper and retain the converter-derived element count. Extend the existing regression across UTF-8, UTF-16, and UTF-32 inputs to verify both target encodings preserve the complete `A\0B` sequence. Keep source-contract checks aligned with the length-delimited invariant.
- Existing CMake/CTest target reused; no new files or CMake changes. No local C build or CTest run is claimed; mainline CI is the compile/runtime gate.

---

## 2026-11-07 12:11 CST — Align UTF embedded-NUL regression source contract

- Baseline: main commit `da34cf94d4f1c0939174658a355fc522b1e8d95b`; SkipDBv2 Regression run #519 stopped in source-contract preflight before configure/build.
- The regression was expanded to cover UTF-16/UTF-32 output, and its failure diagnostic changed accordingly. The workflow's static test-file contract still required the previous UTF-8-only diagnostic, so preflight rejected the correctly updated test.
- Replace the stale needle with the new UTF-16/UTF-32 regression helper and diagnostic. Production code and test behavior are unchanged; no CMake or file-path changes.

---

## 2026-11-07 12:14 CST — Preflight scalar floating-to-integer division

- Baseline: main commit `fcf717cb8fbb7e8df06b457ba32bb6e882d8a245`; SkipDBv2 Regression run #520 passed source contracts, configure, full build, and all CTest regressions.
- Scope: `UArray_divideScalarDouble_()` in `basekit/UArray_math.c` and its existing `uarray_math_extrema` regression.
- Scalar division previously assigned each floating quotient directly into an integer element. A finite quotient outside the target integer range, or a non-finite quotient, makes floating-to-integer conversion undefined; processing in-place could also mutate earlier elements before a later element proves invalid.
- Reuse the existing integer-quotient range predicate to scan every integer element before the assignment loop. Reject invalid or non-finite quotients atomically; preserve the existing floating-array path, valid integer division, zero-divisor no-op, and cache invalidation behavior.
- Extend the existing regression with a late out-of-range int8 quotient and a NaN quotient, asserting contents and cached hash remain unchanged. Update the existing Actions source contract; no new files or CMake changes. No local C build/CTest is claimed; mainline CI is the verification gate.

---

## 2026-11-07 12:15 CST — Correct scalar-division contract guard ordering

- Baseline: main commit `6cec8277387e1fd6f405be5874612e71eac31c21`; SkipDBv2 Regression run #521 stopped in the source-contract preflight.
- The scalar divide implementation and regression compiled neither way because the source-contract verifier searched for the float-type guard starting at the later range-loop declaration, while the guard correctly precedes that loop.
- Correct the verifier to locate the type guard from the start of the scalar function and explicitly require it to precede the scan. Keep the quotient scan and its placement before mutation unchanged.

---

## 2026-11-07 12:17 CST — Invalidate UArray hash after same-size fread

- Baseline: main commit `1e4da38232e3d9c0f6fd28a691d30bbcf42d3e45`; SkipDBv2 Regression run #522 passed source contracts, configure, full build, and all CTest regressions.
- Scope: direct `UArray_fread_()` writes in `basekit/UArray_stream.c` and the existing `uarray_stream_io` test.
- A complete read overwrites every logical item but leaves the array size unchanged. `UArray_setSize_()` is a no-op when the requested size already matches, so the existing path retained the previously cached hash for entirely different contents.
- Remember the pre-read item count and invalidate the hash after a successful nonempty same-size overwrite. Reads that shrink the array continue to use the existing resize/fallback invalidation path; zero-item reads remain unchanged.
- Add a direct full-overwrite regression that seeds a nonzero cached hash and verifies both payload and hash. Update the existing Actions source contract; CMake target and file set are unchanged. No local C build/CTest is claimed; mainline CI is the verification gate.

---

## 2026-11-07 12:19 CST — Reject malformed UArray layouts before binary stream writes

- Baseline: main commit `7dfc1d1886069458b6c40e357fd3e9351ed307bb`; SkipDBv2 Regression run #523 passed source contracts, configure, full build, and CTest.
- Scope: `BStream_writeUArray_()` in `basekit/BStream.c`, the existing `bstream_tag_boundaries` regression, and its Actions source contract.
- The read-side array validator already required `itemSize == CTYPE_size(itemType)`, but the write-side preflight checked only multiplication geometry. A forged byte-array descriptor with a larger item width could make `UArray_sizeInBytes()` overstate the valid backing extent, causing the stream writer to read beyond the actual byte storage.
- Reject a mismatched source item width before calculating the serialized payload length. Extend the existing write-validation test with padded backing storage so the regression detects acceptance without itself reading outside the fixture. Require an error and an unchanged stream.
- Reuse the existing CMake/CTest target; no new files or target changes. No local C build or CTest run is claimed; mainline CI is the validation gate.

---

## 2026-11-07 12:19 CST — Preserve exact time_t values in Date_fromTime_

- Baseline: main commit `b2c7ee397792acb267658448ea80ea3a3c79d335`; SkipDBv2 Regression run #523 passed source contracts, configure, full build, and all CTest regressions.
- Scope: `Date_fromTime_()` in `basekit/Date.c` and the existing `date_time_init` precision regression.
- The constructor widened an integral `time_t` to `double` and routed it through `Date_fromSeconds_()`. Beyond the exact-integer range of IEEE-754 double, distinct whole-second timestamps collapse to the same value, silently losing seconds.
- Assign the incoming `time_t` directly to `tv_sec` and reset `tv_usec` to zero. This preserves the exact source value without changing timezone metadata or fractional-second APIs.
- Extend the existing precision test with a representable timestamp at 2^53+1 seconds and add source-contract checks for the direct assignment. Existing CMake target reused; no new files. No local C build/CTest is claimed; mainline CI is the verification gate.

---

## 2026-11-07 12:20 CST — Correct Date_fromTime_ source-contract delimiter

- Baseline: main commit `97d3fa3005cf11f0f22983bcac661597e43ed260`; SkipDBv2 Regression run #525 stopped in source-contract preflight.
- The new dynamic contract searched for the end of `Date_fromTime_()` using a Python string containing a literal backslash-plus-`n`, rather than the newline that starts `Date_asTime()`. The source body extraction was therefore empty.
- Correct the delimiter escape so the contract checks the actual function body. Production code and regression test are unchanged.

---

## 2026-11-07 12:23 CST — Reject absent numeric fields in PortableStrptime directives

- Baseline: main commit `5656f3584628978f366ddafe2a65c3bd37cf0375`; SkipDBv2 Regression run #526 passed source contracts, configure, full build, and CTest.
- Scope: numeric `%M`, `%S`, `%Y`, and `%y` parsing in `basekit/PortableStrptime.c`, using the existing `portable_strptime_bounds` test.
- These directives treated an empty or whitespace-only input field as an optional success and left the corresponding `struct tm` field untouched. A truncated timestamp such as a format ending in `%M` could therefore be reported as parsed despite the required numeric field never appearing.
- Remove the silent-skip branches. Existing digit/range checks now reject missing and whitespace-only fields consistently; add regression coverage for all four directives.
- Reuse the existing CMake/CTest target and workflow contract blocks. No new files and no local build/CTest claim.

---

## 2026-11-07 12:40 CST — Remove unreachable legacy snprintf formatter

- Baseline: main commit `e08721f1f5b68b941ae671a24bcbb77a70a1152b`; SkipDBv2 Regression run #527 passed source-contract preflight, configure, full build, and CTest.
- Scope: `basekit/PortableSnprintf.c`.
- This translation unit defined `HAVE_SNPRINTF`, `HAVE_VSNPRINTF`, and `HAVE_C99_VSNPRINTF` unconditionally before the fallback branch, so the roughly 700-line legacy formatter could never be compiled. Normal builds only emitted `dummy_snprintf()`; the standard APIs came from the host C library.
- Remove the unreachable implementation and duplicate setup while preserving the object marker, the optional `TEST_SNPRINTF` libc smoke harness, and the existing source path. CMake uses a recursive basekit source glob, so no build-script or workflow source-contract changes are required.
- No local C build or CTest is claimed; mainline CI remains the verification gate.

---

## 2026-11-07 12:46 CST — Route commit-marker writes through EINTR-safe sync

- Baseline: main commit `a6ff958cf1fc9b94f709370dea98552a458b5261`; SkipDBv2 Regression run #528 passed source contracts, configure, full build, and all CTest regressions.
- Scope: `UDBCommitMarker_write()` in `udb/UDBCommitMarker.c` and its existing Actions source contract.
- Marker writes duplicated direct `fsync()`/`F_FULLFSYNC` calls without the `EINTR` retry already provided by `JFile_hardSyncFileDescriptor_()`. A transient signal could therefore turn a recoverable sync interruption into a failed commit-marker durability barrier.
- Reuse the existing shared barrier after the marker byte is flushed, preserving error propagation and the existing platform-specific sync choice. Add a function-scoped source contract ensuring future edits keep this call in the marker-write path.
- The existing `commit_marker_test` and CMake target are reused; no new files or exported declarations. No local C build/CTest is claimed; mainline CI remains the verification gate.

---

## 2026-11-07 12:48 CST — Detect standard Windows macros for JFile path separators

- Baseline: main commit `6ac2b52e2ff75b60f1b513f5f0409b3a7b713837`; SkipDBv2 Regression run #529 passed source contracts, configure, full build, and all CTest regressions.
- Scope: `JFile_fileName()` in `jfile/JFile.c` and its existing Actions source contract.
- The path helper selected backslashes only when `__WIN32__` was defined. MSVC and MinGW commonly define `_WIN32`, `WIN32`, or their compiler-specific macros instead, so `JFile_setLogPath_()` could treat an entire Windows path as the basename and generate a malformed journal path.
- Match the repository's Windows macro family while retaining the POSIX slash branch. Extend the existing source contract to require both separator branches and the platform guard.
- Existing `jfile_journal_test` and CMake target are reused; no new files or exported declarations. No local C build/CTest is claimed; mainline CI remains the verification gate.

---

## 2026-11-07 12:51 CST — Enforce 32-bit MT state and output width

- Baseline: main commit `3420dd2891a7edf14dd858fc58de4340135d1629`; SkipDBv2 Regression run #530 passed source contracts, configure, full build, and all CTest regressions.
- Scope: `genrand_int32()` in `basekit/RandomGen.c`, its existing state-boundary regression, and the corresponding Actions source contract.
- `RandomGen` stores MT19937 words in `unsigned long`, which is 64-bit on LP64 systems. A corrupted/publicly modified state could therefore leak high bits into the output and violate `RandomGen_randomDouble()`'s `[0,1)` range.
- Mask every twist result and the tempered output to the algorithm's specified 32-bit word width. Add LP64-only regressions covering both output before a twist and state normalization during a twist; standard seeded sequences remain unchanged.
- Reuse the existing `random_gen_state_bounds_test` CMake target; no new files or exported declarations. No local C build/CTest is claimed; mainline CI remains the verification gate.

---

## 2026-11-07 12:53 CST — Prevent UArray item-count ceiling overflow

- Baseline: main commit `0845cf06b9200a2de592ffcb3fd4da1cbdaa7425`; SkipDBv2 Regression run #531 passed source contracts, configure, full build, and all CTest regressions.
- Scope: `UArray_sizeRequiredToContain_()` in `basekit/UArray.c`, its existing byte-size regression, and the matching Actions source contracts.
- The helper computed ceiling division as `(bytes + itemSize - 1) / itemSize`, which can wrap when the byte extent is near `SIZE_MAX`; it also dereferenced a NULL receiver and divided by zero for malformed item metadata.
- Guard the arguments and compute quotient plus a nonzero-remainder flag, avoiding overflowing addition while preserving normal rounding semantics. Extend the existing test with NULL/zero-width cases and a SIZE_MAX boundary without allocating an enormous buffer.
- The existing `uarray_size_in_bytes_test` CMake target is reused; no new files or exported declarations. No local C build/CTest is claimed; mainline CI remains the verification gate.

---

## 2026-11-07 12:55 CST — Keep empty path append as a no-op

- Baseline: main commit `019b6b855c715814ba8bb6bc0c04f08081943ad5`; SkipDBv2 Regression run #532 passed source contracts, configure, full build, and all CTest regressions.
- Scope: `UArray_appendPath_()` in `basekit/UArray_path.c`, its existing path regression, and the corresponding Actions source contracts.
- Joining a non-empty path with an empty component previously inserted a separator before appending zero bytes, silently changing `/safe` into `/safe/`.
- Return early for a valid empty path component and add a regression asserting that appending `""` preserves the destination byte-for-byte. Reuse the current `uarray_path_null_test` target; no new files or exported declarations.
- No local C build/CTest is claimed; mainline CI remains the verification gate.

---

## 2026-11-07 12:57 CST — Make UArray size getters NULL-safe

- Baseline: main commit `4c96969dbb2796c2c22b36a73fbafef036f6fe37`; SkipDBv2 Regression run #533 passed source contracts, configure, full build, and all CTest regressions.
- Scope: `UArray_size()` and `UArray_itemSize()` in `basekit/UArray.c`, their existing metadata regression, and the source contracts.
- These public read-only getters unconditionally dereferenced their receiver, unlike the adjacent `UArray_sizeInBytes()` query that already treats a NULL array as zero bytes.
- Return zero for a NULL receiver while retaining the existing values for valid arrays; add regression coverage for both getters and source-contract checks.
- Reuse the existing `uarray_size_in_bytes_test` CMake target; no new files or public declarations. No local C build/CTest is claimed; mainline CI remains the verification gate.

---

## 2026-11-07 13:00 CST — Preflight float-to-integer array fills

- Baseline: main commit `3f005da247d342a2cdf20a19ea49cf8a9ba21782`; SkipDBv2 Regression run #535 passed source contracts, configure, full build, and all CTest regressions.
- Scope: `UArray_setItemsToDouble_()` in `basekit/UArray_math.c`, its existing assignment-transform regression, and matching Actions source contracts.
- Filling an integer-backed array with `INFINITY`, NaN, or a value outside the destination integer range could invoke undefined floating-to-integer conversion during the element loop and partially mutate the array.
- Reuse the existing range preflight used by integer division; reject unsafe values before touching data or its cached hash, while preserving floating-array support for non-finite values.
- Reuse the existing `uarray_math_extrema_test` CMake target; no new files or exported declarations. No local C build/CTest is claimed; mainline CI remains the verification gate.

---

## 2026-11-07 13:02 CST — Match float-range helper definition in source contract

- Baseline: main commit `932f6f2ca0e0813f250af7da96bbd9ffec956663`; Actions run #536 failed during the source-contract preflight before configure/build/CTest.
- Scope: `.github/workflows/skipdbv2-regression.yml` and this handoff log only.
- Adding the required forward declaration for `UArray_integerFloatQuotientIsOutOfRange_()` made the contract's broad signature search stop at the prototype instead of the implementation, falsely reporting that the helper no longer rejected non-finite/out-of-range values.
- Anchor the contract lookup to the function definition's opening brace so declarations and implementation remain unambiguous. The C change and its regression tests are retained; no build-script or test-target changes are required.
- This is a CI-contract repair only. The prior run did not reach compilation or CTest; the new mainline run is the verification gate.

---

## 2026-11-07 13:03 CST — Correct Python newline escape in helper contract

- Baseline: main commit `dfb9a64b73c4ba0e2958a7542e1becd67038ae34`; Actions run #537 again failed in source-contract preflight, before configure/build/CTest.
- The first contract repair selected the function definition, but encoded the newline escape twice, so Python searched for a literal backslash-n instead of the definition's line break.
- Reduce the escape by one level so the contract search resolves the actual implementation after the forward declaration. No C source, tests, or build targets changed in this repair.
- Mainline CI remains the verification gate.

---

## 2026-11-07 13:06 CST — Align PortableUsleep Windows detection

- Baseline: main commit `ba87ea7a6ca9f8b5a2e08f0316651a58c076316b`; Actions run #538 passed source contracts, configure, full build, and all CTest regressions.
- Scope: `basekit/PortableUsleep.c`, its existing public header, and their workflow source contracts.
- The implementation selected its Windows shim only under `WIN32`, while the header used the project-specific `ON_WINDOWS` guard. Toolchains that provide `_WIN32`/MSVC or MinGW macros without the legacy `WIN32` alias could therefore include a native POSIX header or miss the shim implementation.
- Use one consistent Windows-macro family in the declaration and implementation, preserving the existing high-resolution sleep behavior. Add the header to existing source-contract validation; no new files or CMake targets.
- No local Windows cross-build is claimed; mainline CI verifies Linux integration and the static platform contract.

---

## 2026-11-07 13:08 CST — Preserve Date interval precision at large timestamps

- Baseline: main commit `6ea543e8997971d94527d99cb702814652a9e57a`; Actions run #539 passed source contracts, configure, full build, and all CTest regressions.
- Scope: `Date_secondsSince_()` in `basekit/Date.c`, the existing `date_time_init` precision test, and their source contracts.
- The function subtracted two absolute timestamps only after converting each to `double`; above 2^53, adjacent whole seconds or microseconds can collapse to a zero interval even though `Date_compare()` retains exact ordering.
- Compute the signed interval from the integer seconds delta first, splitting the unsigned delta into high/low 32-bit chunks before converting to `double`, then add the fractional-microsecond difference. This avoids signed integer subtraction overflow and preserves small intervals on platforms where `long double` is not wider than `double`.
- Extend the existing precision test for ±1 second and ±1 microsecond at large timestamps. Existing CMake target reused; no new files or exported declarations. No local C build/CTest is claimed; mainline CI is the verification gate.

---

## 2026-11-07 13:10 CST — Preserve Win32 truncate failure causes

- Baseline: main commit `01cf7aa88f4b2f6a0137e9d4210d85a3a7f887ef`; Actions run #540 passed source contracts, configure, full build, and all CTest regressions.
- Scope: Windows implementation in `basekit/PortableTruncate.c` and its existing Actions source contract.
- Win32 `CreateFileA`, `SetFilePointer`, `SetEndOfFile`, and `CloseHandle` failures previously returned `-1` without setting POSIX `errno`, leaving callers with stale or unrelated error details.
- Map common Win32 file, access, handle, memory, parameter, and disk-full failures to stable errno values; preserve `GetLastError()` before closing the handle and also report close failures.
- The existing source path is unchanged. The Windows-only branch is not locally cross-built; mainline CI checks the error-path contract and Linux build/regressions.

---

## 2026-11-07 13:16 CST — Guard UArray scalar floating-point writes

- Baseline: main commit `8deffc85c39ccb53aad535984c7568e1bfa5fd41`; SkipDBv2 Regression run #540 passed source contracts, configure, the full SkipDBv2 build, and all CTest regressions.
- Scope: indexed scalar mutation in `basekit/UArray.c`, the existing `uarray_math_extrema_test`, and its Actions source contracts.
- `UArray_at_putDouble_()` previously cast NaN, infinities, and out-of-range doubles directly into integer storage. Those conversions are undefined in C and could corrupt scalar writes. The setter now validates the receiver's complete item layout and checks the truncated-value range before growing or mutating the array.
- Preserve valid C truncation semantics at signed/unsigned limits, including the exact `int64_t` minimum and the final representable `uint64_t) double below 2^64; retain non-finite writes for floating arrays. Apply the same layout guard to the adjacent long setter, and cover append through its existing delegation path.
- Reuse the existing regression executable and CMake registration; extend its source contract. No new source files or exported symbols.

### Validation
- Static preflight: verified the conversion guard runs before `UArray_setSize_()` and before any element assignment; checked each supported integer width, `uintptr_t`, floating-type passthrough, malformed layout rejection, and unchanged hash/data on rejected writes.
- No local compiler or CTest run is claimed. The updated mainline Actions workflow is the verification gate.

---

## 2026-11-07 13:21 CST — Align UArray layout-validator source contract

- Baseline: main commit `cbaef56b38fe0c66ae5aae3f6eea72b813359190`; SkipDBv2 Regression run #541 stopped in the XY-C1 source-contract preflight before CMake configure/build/CTest.
- The scalar-write hardening added receiver-layout validation to both `UArray_at_putLong_()` and `UArray_at_putDouble_()`. The workflow still asserted the prior total of one definition plus eight call sites, so its exact occurrence count rejected the intended additional guards.
- Update the existing contract to require one definition plus ten guarded call sites. The validator implementation and all call sites remain unchanged; no source, test, or CMake behavior is altered.
- Static preflight confirms the source has exactly 11 occurrences and 10 call sites, matching the updated contract. The next Actions run is the verification gate.

---

## 2026-11-07 13:24 CST — Preserve UArray scalar-write cache invalidation

- Baseline: main commit `6d425107c2712381b7bfc35a85239b75ae9e4ec1`; SkipDBv2 Regression run #542 passed source contracts, configure, and the full build, but 1 of 57 CTest cases failed in `uarray_math_extrema_test`.
- The failure exposed a second defect in the same scalar-write path: `UARRAY_RAWAT_PUT_` used `return;` in every dispatch arm, exiting `UArray_at_putLong_()` / `UArray_at_putDouble_()` before their trailing `UArray_changed(self)`. Same-size writes therefore left cached hashes stale; growth writes masked the defect because resize already invalidated the hash.
- Change the 11 switch arms to `break;`, allowing both setters to run the shared mutation invalidation. Add same-size double and long write assertions, while retaining the int64/uint64 boundary checks that found the issue.
- Tighten the workflow source contract: require switch breaks and verify both setters invalidate after dispatch; correct the layout-validator call-site count diagnostic. Reuse the existing test and CMake registration.
- Validation: run #542 compiled all targets and isolated the failure to this test; the next Actions run is the post-fix build/CTest gate. No local build is claimed.

---

## 2026-11-07 13:29 CST — Correct UArray source-contract newline escapes

- Baseline: main commit `fa29af2957460c923ff7f000ff88144bb4d24cb2`. Preflight of run #543 found that the new Python source-contract searches were double-escaped; they would search for a literal backslash-n instead of the macro/function closing-line boundary.
- Correct both searches to use a single Python newline escape (`\n`). No C implementation or test code changed in this workflow-only repair.
- Static check confirms both targeted escapes were corrected and no double-escaped variants remain. This commit triggers the authoritative rerun of the scalar-write cache fix.

---

## 2026-11-07 13:31 CST — Stabilize UArray scalar dispatch source contracts

- Baseline: main commit `781ed26360155963e42174e4c60de31ac0c9aaaa`; run #543 failed during the XY-C1 source-contract phase before configure/build. Its new UArray source-contract entries assumed space indentation, while the C switch uses tabs.
- Replace the two formatting-sensitive full-line needles with stable case-label fragments. The behavioral contract remains enforced separately by the Python check for 11 `break;` arms, no early `return;`, and post-dispatch `UArray_changed(self)` in both setters.
- No C implementation or regression test changed. This workflow-only correction triggers another source-contract/build/CTest run.

---

## 2026-11-07 13:32 CST — Validate typed storage before tagged BStream array writes

- Baseline: main commit `33e6da33e482f9ca13fb898d6eee55d7183c2d57`; SkipDBv2 Regression run #545 passed XY-C1 source contracts, configure, the complete build, and all CTest regressions.
- Scope: `BStream_writeTaggedUArray_()` in `basekit/BStream.c` and the existing `bstream_tag_boundaries_test`.
- The plain array writer and array readers reject mismatched `itemType`/`itemSize` pairs, but the tagged writer checked only nonzero item size and multiplication bounds. A malformed public `UArray` could therefore be serialized using a byte extent inconsistent with its declared C type; the tagged writer also accepted a stream buffer whose own CTYPE width did not match its byte-sized storage.
- Validate both source and destination stream CTYPE widths before computing serialized byte counts or testing backing-store overlap. Extend the existing layout regression to exercise both malformed source and malformed stream cases and assert the writer leaves the stream untouched.
- No new files, exported declarations, or CMake changes. Extend the existing Actions source contract to scope the width checks to the tagged writer. No local compiler run is claimed; the mainline Actions run is the build/CTest gate.

---

## 2026-11-07 13:33 CST — Repair tagged BStream source-contract boundary

- Baseline: main commit `354d78f6d391a7741408843a1abe14d0f12c65db`; run #546 passed dependency installation but stopped in the XY-C1 integration/source-contract step before CMake configure.
- The new tagged-writer contract used an over-escaped newline string when locating the end of `BStream_writeTaggedUArray_()`; Python searched for a literal backslash-n and therefore produced an empty function slice.
- Use `chr(10) + "}"` as the function boundary, avoiding escape ambiguity. The C implementation and regression cases remain unchanged.
- Static preflight confirms the boundary expression is present exactly once and the stale expression is absent. The next Actions run is the build/CTest gate.

---

## 2026-11-07 13:35 CST — Recover absorbing all-zero RandomGen state

- Baseline: main commit `705ea49e1d3943ec002cd147a35b5674e9f0b2e4`; SkipDBv2 Regression run #547 passed source contracts, configure, the complete build, and all CTest cases.
- Scope: MT19937 state recovery in `basekit/RandomGen.c` and the existing `random_gen_state_bounds_test`.
- The Gaussian API already detects an all-zero MT state and reseeds it, but the public integer and floating-point generators did not. An all-zero state is absorbing in MT19937, so `RandomGen_randomInt()` and `RandomGen_randomDouble()` could return a constant zero stream indefinitely after zero-initialization or state corruption.
- Reuse the existing state validator at the initial-index and twist-batch boundaries (`mti == 0` or `mti == N`). This catches a zero-initialized instance before its first output and recovers a zeroed state when the next batch begins, while avoiding a 624-word scan on every generated value. Restore the deterministic default seed `5489`, consistent with the existing Gaussian recovery path.
- Add coverage for integer, floating-point, and twist-boundary recovery. Reuse the registered CTest target; no new files, external symbols, or CMake changes.
- Static preflight: confirmed the helper retains one static definition plus a forward declaration, recovery occurs before indexing/twisting, all three tests compare against the deterministic default-seed sequence, and the existing test remains CMake-registered. No local compiler or CTest run is claimed; mainline Actions is the verification gate.

---

## 2026-11-07 13:41 CST — Reject unsupported PortableStrptime directives

- Baseline: main commit `5621c6f0e847c743ba6f5dee99341239b2234840`; SkipDBv2 Regression run #548 passed source contracts, configure, the complete build, and all CTest cases.
- Scope: conversion-specifier dispatch in `basekit/PortableStrptime.c` and the existing `portable_strptime_bounds_test`.
- The parser's switch had no default branch. An unsupported directive such as `%Q` or an unimplemented standard directive such as `%z` was silently skipped, allowing an invalid format to return a successful input cursor without consuming the directive's expected data.
- Add an explicit rejection path for unsupported conversion specifiers. Exercise both an unknown directive and an unsupported timezone directive in the existing boundary test; retain all supported directive behavior unchanged.
- Reuse the existing CMake test registration and add a workflow source contract for the rejection branch. No new files or exported declarations.
- Static preflight: confirmed the default branch is inside the format dispatch switch, invalid formats return NULL, both cases are exercised, and the CTest target is already registered. No local compiler/CTest run is claimed; mainline Actions is the verification gate.

---

## 2026-11-07 13:43 CST — Repair PortableStrptime source-contract mapping

- Baseline: main commit `7f7e090420e834d855d423d35e534053874e2c1e`; Actions run #549 failed immediately with zero allocated jobs, before checkout or build.
- The preceding change accidentally inserted a second `PortableStrptime.c` key into the workflow's Python source-contract mapping and displaced the indentation of the following test key. The duplicate dictionary key also shadowed the existing source-contract list.
- Merge the unsupported-directive assertions into the original C-source contract, remove the duplicate key, restore the test key's mapping indentation, and retain the new `%Q`/`%z` regression needles in the existing test contract.
- No C implementation or test code changed in this CI-only repair. CMake registration remains unchanged.

### Validation
- Static preflight requires exactly one source key and one test key, verifies both are indented as sibling mapping entries, confirms the original source contract remains present alongside the new default-branch needles, and confirms all new regression needles remain in the test block.
- Run #549 failed before a job was created; this push triggers the corrected workflow run and is the next build/CTest gate.

---

## 2026-11-07 13:47 CST — Reject misaligned List backing storage

- Baseline: main commit `8ffeb9ce6e50096f55e9918b0c968c57ce994d02`; SkipDBv2 Regression run #550 passed the source contracts, configure, full build, and all CTest regressions.
- Scope: the shared storage invariant in `basekit/List_inline.h` and the existing `list_storage_bounds_test`.
- `List_hasValidStorage_()` checked logical size against allocated byte capacity and null storage, but accepted a non-NULL `items` pointer that was not aligned for `void *`. The inline readers and mutators could then dereference misaligned pointer elements, which is undefined behavior and can fault on strict-alignment targets.
- Add a `uintptr_t` alignment check before any element access. This brings List's backing-storage validation in line with the established Stack invariant without changing valid List allocation or growth behavior.
- Extend the existing storage-boundary test with a deliberately misaligned shallow view. Verify reads reject it, mutations leave all metadata untouched, and `List_asStackAllocatedUArray()` does not expose it.
- No new files, exported symbols, or CMake changes. The existing `list_storage_bounds_test` remains registered; workflow source contracts now require the alignment guard and its regression coverage.

### Validation
- Static preflight: checked the guard executes inside the central validator before callers access elements, `stdint.h` supplies `uintptr_t`, all misaligned-view operations are expected to fail closed, and cleanup retains ownership of the original allocation only.
- No local compiler/CTest run is claimed. Mainline Actions is the verification gate.

---

## 2026-11-07 13:52 CST — Repair List alignment regression test dependency

- Baseline: main commit `181bc9b8f98fdc07dbf961b9bf391249fa24498b`; SkipDBv2 Regression run #551 failed in the XY-C1 source-contract stage before CMake configure/build/CTest.
- The existing `list_storage_bounds.c` regression uses `uintptr_t` to construct a deliberately misaligned List view, but the test omitted its direct `<stdint.h>` include. The workflow correctly requires the test to declare that dependency itself.
- Added the missing standard header to the existing test. No production logic, new files, CMake registration, or workflow contracts changed.
- Validation: verified the failing contract's missing token against the test source and confirmed the implementation-side `List_inline.h` already includes `<stdint.h>` for its own `uintptr_t` use. Remote CI remains the compiler/CTest gate; no local build is claimed.

---

## 2026-11-07 13:55 CST — Reject misaligned UTF-16/UTF-32 UArray storage

- Baseline: `30e3da52588ef464c7ab7f687e1b97a3edd70d4f`; SkipDBv2 Regression run #552 passed XY-C1 integration contracts, configure, all targets, and CTest.
- Scope: Unicode encoding layout validation in `basekit/UArray_utf.c` and its existing `uarray_encoding_conversion_test`.
- The encoding validator checked element width and byte capacity but not the alignment required when ConvertUTF reads UTF-16/UTF-32 code units through typed pointers. A caller-provided stack-backed or borrowed UArray could therefore pass validation and trigger undefined behavior on strict-alignment targets.
- Added alignment checks using `offsetof` probes for the actual UTF code-unit types; included `<stdint.h>` directly for `uintptr_t`. The existing conversion test now exercises misaligned UTF-16 and UTF-32 inputs across the conversion entry points and requires rejection.
- Updated the existing GitHub Actions source contracts to bind the invariant and regression coverage. CMake registration and source lists are unchanged; no new files or exported symbols.
- Validation: static review confirmed validation precedes all conversion dispatch and the test's deliberately offset byte buffers are never dereferenced as UTF code units. Remote CI remains the build/CTest gate; no local build is claimed.

---

## 2026-11-07 14:00 CST — Reject misaligned typed UArray string storage

- Baseline: `c7771fb5b3cba1afb8ea2ff38d36b37362392784`; preceding Unicode alignment commit passed SkipDBv2 Regression run #553, including configure, all targets, and CTest.
- Scope: typed-item layout validation in `basekit/UArray_string.c`.
- The existing string-item validator checked CTYPE width and allocation bounds but not per-type alignment. String operations such as unescape and trim can read elements through typed UArray accessors, making borrowed or stack-backed misaligned numeric storage undefined on strict-alignment architectures.
- Added exact per-CTYPE alignment probes and reject misaligned backing pointers before typed iteration or mutation. Extended the existing unescape regression test with misaligned uint64-backed inputs that must remain byte-for-byte unchanged.
- Updated the existing Actions source contracts for the invariant and regression. CMake registration and source lists remain unchanged; no new files or exported symbols.
- Validation: source-level review verified alignment is checked before every operation using the shared validator, and test fixtures deliberately offset byte storage while keeping accesses byte-oriented. Remote CI is the build/test gate; no local build is claimed.

---

## 2026-11-07 14:06 CST — Reject misaligned numeric UArray conversion sources

- Baseline: `b2bbd1ff47146eec03713ac249a1ec8ab9fd8aee`; SkipDBv2 Regression run #554 passed after the typed string-storage alignment change.
- Scope: numeric-array conversion validation in `basekit/UArray_utf.c`.
- The number-array formatter uses typed iteration after checking CTYPE width and allocation bounds, but previously accepted unaligned borrowed storage. Since numeric UArrays are converted through this path by all UTF output conversions, misaligned numeric data could trigger undefined typed reads.
- Added per-CTYPE alignment probes to the existing numeric-layout validator and extended `uarray_encoding_conversion_test` with a misaligned uint64-backed numeric source. UTF-8/16/32 conversion entry points must now reject it without modifying the borrowed bytes.
- Updated existing Actions source contracts. CMake registration and source lists are unchanged; no new files, exported symbols, or unrelated modules.
- Validation: static review confirms numeric formatting validates before `UARRAY_INTFOREACH`; remote CI is the build/CTest gate, with no local build claimed.

---

## 2026-11-07 14:11 CST — Validate alignment before UArray numeric math

- Baseline: `876df8eb6db3b47274bff6cbef3297d311509186`; numeric conversion alignment passed SkipDBv2 Regression run #556.
- Scope: shared typed-numeric storage preflight in `basekit/UArray_math.c`.
- Math entry points use typed iteration, scalar writes, reductions, and a float32 vector fast path after validating item width and byte geometry. They did not validate the alignment required by the declared CTYPE, so misaligned borrowed storage could reach typed reads/writes.
- Added per-CTYPE alignment probes to the shared numeric-storage validator. Extended the existing extrema/math regression test to verify clear/fill operations leave misaligned uint64 storage unchanged and dot-product rejects it.
- Updated existing Actions source contracts only; CMake registration and source lists are unchanged. No new files or exported symbols.
- Validation: static review confirms all typed math paths use the shared validator before dispatch; remote CI remains the compiler/CTest gate, no local build is claimed.

---

## 2026-11-07 14:16 CST — Enforce CTYPE alignment in core UArray layout validation

- Baseline: `8d98d011854e7ad1e179be8e4c2654c1522ca5a2`; numeric math alignment passed SkipDBv2 Regression run #557.
- Scope: the shared `UArray_hasValidItemLayout_` guard in `basekit/UArray.c`, which protects typed copy/conversion, scalar writes, sorting, and stack-range creation.
- The core validator checked CTYPE width, extent overflow, and non-empty data presence, but still accepted a correctly sized yet misaligned borrowed pointer. Those call sites subsequently use typed C lvalues or comparators, which can be undefined on strict-alignment platforms.
- Added a per-CTYPE alignment lookup using `offsetof` probes and reject misaligned backing storage before the shared typed operations run. Extended the existing stack/copy regression test to cover misaligned copy sources and destinations, stack ranges, and scalar writes; fixtures are initialized and inspected through byte operations.
- Updated existing Actions source contracts. The test was already registered in CMake; no build files, new files, or exported symbols changed.
- Validation: reviewed every call site of the shared validator to ensure the check precedes typed access. Remote CI is the compile/CTest gate; no local build is claimed.

---

## 2026-11-07 14:19 CST — Reject misaligned UArray pointer arrays

- Baseline: `3b8e9d8315e4bc2143c14ee769b9520e645cde91`; core CTYPE alignment passed SkipDBv2 Regression run #558.
- Scope: pointer-array access guard `UArray_hasValidPointerStorage_` and its existing `uarray_pointer_access` test.
- Pointer getters and setters cast backing bytes to `void **`. The existing guard verified pointer width and allocation bounds but accepted misaligned borrowed storage, leaving undefined typed reads/writes on strict-alignment targets.
- Added a `void *` alignment probe and reject misaligned backing addresses before raw/bounds-checked pointer reads, writes, or appends. The regression uses a byte-offset borrowed pointer slot and verifies getters return NULL while setters preserve the original bytes, size, and cached hash.
- Updated existing Actions source contracts. The test was already registered in CMake; no source lists, new files, or exported symbols changed.
- Validation: source review confirms all pointer getters/setters route through the shared guard before casting to `void **`. Remote CI remains the build/CTest gate; no local build is claimed.

---

## 2026-11-07 14:25 CST — Prevent recursive DynLib free-callback unload

- Baseline: `51cba03394b7bd86a0bdc28f91b0de5df37026ea`; UArray pointer-alignment regression passed SkipDBv2 Regression run #560.
- Scope: `DynLib_close` callback/loader-handle lifecycle.
- The old close path invoked the plugin free callback while `self->handle` still referenced the open library. If the callback re-entered `DynLib_close(self)`, the same callback was resolved and invoked recursively, potentially unbounded, and a nested close could unload code that was still executing.
- Snapshot the original handle, resolve the callback, then detach `self->handle` before executing plugin code. Reentrant close now observes the detached/closing state and is a no-op; the outer call performs the single OS close. If `dlclose` fails, restore the original handle only when the callback did not reopen the object.
- Added a test-only shared-library fixture and extended `dynlib_lifecycle_test` to invoke a callback that recursively calls `DynLib_close`; CMake now builds the fixture and passes its target path to the test. Updated the workflow's source and CMake contracts for the new fixture.
- Validation: source-level review confirms callback lookup happens while the handle is open, detachment precedes callback invocation, and the outer close owns the final `dlclose`. Remote CI remains the compile/runtime gate; no local build is claimed.

---

## 2026-11-07 14:29 CST — Repair DynLib fixture CMake contract gate

- Baseline: `5f571bc7b3109649b292bbba49b4b5a173b69880`; run #561 failed only in the XY-C1 integration-contract step before CMake configure/build.
- The new reentrant DynLib fixture and parameterized CTest command were present in `CMakeLists.txt`, but the workflow's Python contract still asserted the old no-argument `add_test` line.
- Updated that existing assertion to require the fixture shared-library target, source path, test dependency, and generator-expression argument. No production or test logic changed.
- Validation: exact CMake strings were compared against the committed target block; remote CI will validate configure/build/CTest.

---

## 2026-11-07 14:32 CST — Reject impossible UArray extents at the Datum boundary

- Baseline: `4b531381b1dc5a19ed766bac3be67d5334725410`; SkipDBv2 Regression run #565 passed the source contracts, configure, full build, and CTest.
- Scope: `Datum_FromUArray_` and the existing `datum_boundaries_test`.
- The adapter checked multiplication overflow but accepted an exact `SIZE_MAX` byte extent. A forged public UArray view could therefore be exposed as a Datum span even though UArray's storage contract cannot represent that extent with its trailing terminator.
- Compute the byte count only after the division-based overflow guard, reject the exact sentinel before obtaining the raw byte view, and reuse the checked count for the returned Datum. Extend the existing boundary test with a forged one-byte-item UArray extent; restore the fixture metadata before freeing it.
- Extend the existing Actions source and regression contracts. The CTest target was already registered; no new files, public declarations, or CMake changes.
- Validation: source-contract ordering now requires CTYPE validation, overflow validation, multiplication, sentinel rejection, and only then raw-byte exposure. Remote CI remains the compiler/CTest gate; no local build is claimed.

---

## 2026-11-07 14:33 CST — Validate expected BStream tag fields before reading

- Baseline: `f745eb4ace0b51178bd26179effb94d090ce0421`; DynLib callback-reentrancy fixture passed the full SkipDBv2 Regression run #562.
- Scope: the internal `BStream_readTag` field-validation and cursor-consumption boundary.
- The writer rejects tag fields outside the wire format (`array <= 1`, `type <= 3`, `byteCount <= 31`), but the reader constructed its expected tag directly into narrow bit-fields. Invalid caller expectations could therefore truncate and alias a valid wire tag, while consuming the stream byte before reporting the mismatch.
- Added the same range validation to the reader before calling `BStream_readUint8`; invalid expectations now set a deterministic error and preserve the cursor and stream byte. Extended the existing BStream tag-boundary test for all three invalid fields and the maximum valid combination.
- Updated the existing Actions source contracts. The test is already registered in CMake; no build-file changes, new files, or exported symbols.
- Validation: reviewed the guard ordering so caller arguments are checked before any read; remote CI remains the compile/CTest gate, no local build is claimed.

---

## 2026-11-07 14:36 CST — Reject the unrepresentable SIZE_MAX UArray stream extent

- Baseline: `b8bbfbc937a4d81f7ffdbab1f02852fbe33cbfd3`; BStream expected-tag validation passed SkipDBv2 Regression run #563.
- Scope: the shared UArray stream layout preflight in `basekit/UArray_stream.c`.
- UArray storage reserves one extra byte for its trailing NUL, and core allocation paths reject a logical byte extent equal to `SIZE_MAX`. The stream validator rejected multiplication overflow but admitted the exact `SIZE_MAX` result for one-byte items. Direct fread/fwrite could then trust impossible public-struct metadata and issue I/O against an allocation that cannot represent that extent.
- The validator now rejects the sentinel extent after its overflow guard. Extended the existing stream I/O regression with a deliberately forged SIZE_MAX byte-array extent and non-reading/non-writing cookie callbacks; all direct and buffered readers plus direct fwrite must reject it without invoking I/O or mutating the array.
- Updated existing Actions source contracts. The stream test is already registered in CMake; no build-file changes, new files, or exported symbols.
- Validation: the byte-count multiplication is evaluated only after the preceding division bound proves it cannot overflow. Remote CI remains the compile/CTest gate; no local build is claimed.

---

## 2026-11-07 14:37 CST — Validate byte-array geometry before BStream attachment

- Baseline: `247619af3bcb549ac31ef727da6efab8cefaf1e7`; main's SkipDBv2 Regression run #567 completed successfully.
- Scope: `BStream_setUArray_` and the existing `testBStreamSetUArrayOwnership` case.
- The attachment boundary accepted any UArray with `itemSize == 1`, even when its declared CTYPE had a different natural width, and also accepted an exact `SIZE_MAX` logical extent. Attaching such a view could discard the currently owned buffer and make subsequent stream reads trust invalid byte geometry.
- Require the uint8 CTYPE width to match and reject the unrepresentable exact-size sentinel before alias checks or ownership transfer. Add borrowed-view regressions for both malformed metadata cases and verify the original owned buffer, ownership flag, and cursor remain unchanged.
- Extend the existing BStream source and test contracts. The boundary test is already registered in CMake; no new files, public declarations, or build-list changes.
- Validation: the test never reads the forged length and keeps the real external byte buffer unchanged. Remote Actions remains the compiler/CTest gate; no local build is claimed.

---

## 2026-11-07 14:40 CST — Reject impossible SIZE_MAX character-array extents

- Baseline: `6d50d548c067278d1796ba892a41eed6145ad316`; UArray stream extent regression passed SkipDBv2 Regression run #564.
- Scope: the shared byte-array preflight in `basekit/UArray_character.c`.
- The character API accepts only one-byte CTYPEs, but its multiplication-overflow test could never reject the exact `SIZE_MAX` byte extent. UArray storage requires a trailing NUL byte, so such a logical extent is not representable; the predicate/case-map loops could otherwise traverse an impossible public-struct size.
- Added an explicit exact-SIZE_MAX rejection after the overflow guard. Extended the existing ctype test with a shallow borrowed view whose size is forged to SIZE_MAX; every predicate/case-map entry point must leave the underlying bytes and cached hash untouched, while case queries return false.
- Updated the existing Actions source contracts. The test is already registered in CMake; no build-file changes, new files, or exported symbols.
- Validation: the byte-count multiplication is evaluated only after its division bound; the regression uses existing live storage but exercises only the preflight path. Remote CI remains the compile/CTest gate; no local build is claimed.

---

## 2026-11-07 14:44 CST — Close the core UArray SIZE_MAX typed-layout gap

- Baseline: `5bfc80f78043e1ff7a91150b340e01c49cb6108f`; character-array SIZE_MAX regression passed SkipDBv2 Regression run #565.
- Scope: shared `UArray_hasValidItemLayout_` preflight in `basekit/UArray.c`.
- The central CTYPE validator rejected multiplication overflow and misaligned storage, but like the specialized stream/character validators it still admitted an exact `SIZE_MAX` byte extent for one-byte arrays. This left core scalar writes and stack-range construction able to trust metadata that cannot reserve UArray's mandatory trailing NUL byte.
- Added the exact byte-extent sentinel rejection after the division-based overflow guard. Extended the existing stack/resize regression to require scalar writes to preserve the backing bytes and hash, and require stack-range creation to return an empty safe view for a forged SIZE_MAX byte extent.
- Updated the existing Actions source contracts. The test is already registered in CMake; no build-file changes, new files, or exported symbols.
- Validation: every multiplication is evaluated only after the overflow bound; the test uses a two-byte borrowed buffer and exercises only guarded entry points. Remote CI remains the compile/CTest gate; no local build is claimed.

---

## 2026-11-07 14:48 CST — Reject impossible SIZE_MAX string-item extents

- Baseline: `1c7610362f1525d401d4f70e6f4e1a361927fc36`; core UArray SIZE_MAX layout regression passed SkipDBv2 Regression run #566.
- Scope: shared typed-string layout validation in `basekit/UArray_string.c`.
- The string-item validator enforced CTYPE width, multiplication overflow, and natural alignment, but still admitted an exact `SIZE_MAX` byte extent for one-byte strings. String mutators use this guard before indexing or iterating, so forged public metadata could bypass the storage invariant that reserves a trailing NUL byte.
- Added the exact-size sentinel rejection after the overflow bound. Extended the existing unescape/string regression with a borrowed-size forgery and a two-byte swap operation; it must leave the byte sequence and cached hash unchanged.
- Updated existing Actions source contracts. The test is already registered in CMake; no build-file changes, new files, or exported symbols.
- Validation: the regression operation would touch only the two real bytes if the guard were absent, avoiding out-of-bounds test behavior while detecting acceptance. Remote CI remains the compile/CTest gate; no local build is claimed.

---

## 2026-11-07 14:52 CST — Validate alignment in UArray distance vectors

- Baseline: `23a6f0cbbbafa1d0c476624ccfec70cb31058508`; Datum byte-extent validation passed SkipDBv2 Regression run #570.
- Scope: distance-vector preflight in `basekit/UArray_math.c`.
- The general numeric math validator already enforces each CTYPE's natural alignment, but the dedicated float32/float64 distance-vector validator checked only item width, extent overflow, and non-NULL storage. `UArray_distanceTo_` then dereferenced the backing pointer as a typed float array, leaving a strict-alignment undefined-behavior path.
- Reused the existing per-CTYPE alignment lookup in the distance validator. Extended the existing distance precision test with deliberately offset float32 buffers; their mathematically valid distance is 5, so the API must reject the misaligned inputs and return its invalid-input sentinel 0.
- Updated the existing Actions source contracts. The test is already registered in CMake; no build-file changes, new files, or exported symbols.
- Validation: fixture bytes are populated only through `memcpy`; remote CI is the compiler/CTest gate, with no local build claimed.

---

## 2026-11-07 14:55 CST — Reject misaligned UArray path storage

- Baseline: `cee9633ef50b18c87d97cec5155e3a11f2038008`; main's SkipDBv2 Regression run #568 completed successfully.
- Scope: `UArray_hasValidPathStorage_` in `basekit/UArray_path.c`.
- The path preflight checked CTYPE width, extent overflow, LONG_MAX bounds, and backing-storage presence, but not the natural alignment required by typed first/last-item access and typed searches. Offset borrowed views could therefore pass validation and reach undefined typed reads on strict-alignment targets.
- Added static per-CTYPE alignment probes and reject misaligned data before path queries, conversions, mutations, or append operations. Extended the existing path regression with a one-byte-offset uint64 view; bytes are initialized and checked with memcpy/memcmp.
- Extended the existing Actions source and regression contracts. The CTest target was already registered; no new files, public declarations, CMake source-list changes, or exported symbols.
- Validation: source-contract ordering requires the alignment preflight before the shared path component scanner and all public entry points. Remote Actions remains the compiler/CTest gate; no local build is claimed.

---

## 2026-11-07 15:00 CST — Repair UArray path alignment CI source contract

- Baseline: `244450e4c451a94b2fed96a0643775504f561b73`; SkipDBv2 Regression run #569 stopped in the XY-C1 source-contract step before configure/build.
- Failure cause: the workflow contract spelled the static probe as `UArrayPathUint64AlignmentProbe`, while the implementation's identifier is `UArrayPathUInt64AlignmentProbe`. The implementation and regression fixture were not reached by compilation; this was a source-contract token mismatch.
- Corrected only the workflow token to match the actual static probe identifier. No C source, test logic, CMake registration, or public interface changed.
- Validation: compared the exact identifier against `UArray_path.c`; remote Actions remains the authoritative compiler and CTest gate.

---

## 2026-11-07 15:03 CST — Fix path alignment contract function-boundary anchors

- Baseline: `0b29e3d0061ce8d6ee2ce66c688439b868af375a`; run #570 passed the file-content contracts but failed the newly added ordering assertion before CMake configure.
- Failure cause: the assertion located `UArray_hasValidPathStorage_` and `UArray_pathComponentBounds_` by signatures that also matched their forward declarations. With static prototypes present, the extracted range was empty.
- Tightened both anchors to include the function-opening brace and the complete component-bounds definition signature, so the assertion examines the validator definition rather than declarations.
- No C implementation, test behavior, build registration, or public interfaces changed. Remote Actions remains the compile/CTest gate.

---

## 2026-11-07 15:06 CST — Make path alignment CI anchors newline-safe

- Baseline: `6b3d50451b21b87a49ecb9b492c97b7f7bd17ad2`; run #571 reached the regression-contract step, where the source-range assertion still failed.
- Failure cause: the YAML-embedded Python string contained a double-escaped newline, so its search literal represented the two characters `\\n` rather than the source file's line break. The previous function-definition anchors were therefore not found.
- Replaced both escaped-newline literals with explicit `chr(10)` concatenation. This preserves exact function-definition matching without depending on nested YAML/Python/JavaScript escape interpretation.
- No C implementation or test logic changed. Remote Actions remains the compiler/CTest gate.

---

## 2026-11-07 15:08 CST — Preserve UArray path source-contract function discovery

- Baseline: `3613cef7c01adac5480c3fbf1f1f0b45cad847d5`; run #572 completed its file-token checks but a legacy function-body contract failed before configure.
- Cause: the path file already defines each private helper before its first use. The extra forward declarations added during the alignment change were unnecessary and caused an existing source-contract parser to mistake the `UArray_pathComponentBounds_` prototype for the function definition.
- Removed only those four redundant static prototypes. All helper definitions remain file-local and ordered before callers; the per-CTYPE alignment guard and new regression stay unchanged.
- This restores the existing contract's function-body discovery without weakening or rewriting the check. No header, CMake, test behavior, or exported symbol changed; remote Actions remains the compiler/CTest gate.

---

## 2026-11-07 15:20 CST — Detach SkipDB cache records before object-free callbacks

- Baseline: `8d5c717bb6fd31aeb69ed5adf63261aa5a89dade`; SkipDBv2 Regression run #573 completed successfully.
- Scope: `SkipDB_noteWillFreeRecord_` in `skipdb/SkipDBCache.c` and its existing core-layers regression.
- The cache teardown path invoked the user-supplied object-free callback while the record was still linked in the age list and PID cache. If that callback reentered `SkipDB_clearCache()`, nested teardown could discover the same record again, invoke the callback twice, and free the record while the outer deallocator was still using it.
- Validate record ownership, remove the record from the age list and PID cache, decrement the cache count, and clear its database/object association before invoking arbitrary callback code. No database/cache state is accessed after the callback.
- Extend the existing `skipdb_core_layers` test with a callback that recursively clears the same database and requires exactly one finalization plus successful database recovery. Extend the existing Actions token and ordering contracts. The test is already registered in CMake; no new files, exported symbols, or build-list changes.
- Static review confirms the database ownership guard and all cache detachment precede the callback; remote Actions remains the compiler/CTest gate. No local build is claimed.

---
## 2026-11-07 15:23 CST — Correct free-callback contract newline delimiter

- Baseline: `550aef6dc0f938f5074dbeeb7ae592ffd4893def`; run #574 stopped in the Actions source-contract phase before CMake configure.
- The new callback-order contract searched for a literal backslash-n sequence instead of the newline preceding the C function's closing brace, so it extracted an empty function body and rejected the otherwise matching implementation.
- Change the source-contract delimiter to the normal Python newline escape. The C implementation, regression, CMake registration, and public interfaces are unchanged.
- No local build is claimed; rerun the mainline Actions contract, compiler, and CTest gates.

---
## 2026-11-07 15:27 CST — Validate cached SkipDB forward-link identity

- Baseline: `d2771b90bb65ea3465d6934c79638503f9816cab`; SkipDBv2 Regression run #575 passed source contracts, configure, full build, and CTest.
- Scope: `SkipDBRecord_recordAtLevel_()` in `skipdb/SkipDBRecordLinks.c` and the existing `skipdb_core_layers` regression.
- A cache-resident `SkipDBPointer.record` was returned without checking that the pointed record still belonged to the source database or that its nonzero cached PID still matched the pointed record. A stale/cross-database pointer could therefore bypass the authoritative on-disk PID link and send ordered traversal into another database's records.
- Before returning a cached target, require matching database ownership and, when the link carries a nonzero PID, exact PID identity. Invalid cache metadata closes the source UDB rather than being reported as an ordinary end-of-list; PID-zero transient in-memory links remain supported.
- Extend the existing core-layers test with cross-database pointer and same-database PID-mismatch corruption fixtures. Each must fail closed and permit recovery from the persisted source database. Extend existing Actions source contracts; the CTest target is already registered and no CMake or public-header change is needed.
- Static review confirms no new cross-translation-unit symbols or source files; remote Actions remains the compiler/runtime gate. No local build is claimed.

---
## 2026-11-07 15:29 CST — Repair adjacent SkipDB source-contract entries

- Baseline: `5f3e2fa7d3a2e5de4381afe028e3e1c0a02e6dc3`; run #576 stopped in the XY-C1 source-contract phase before configure/build.
- The new regression token was inserted into the existing `skipdb_core_layers.c` contract array without a trailing comma, concatenating it with the following cursor token and making the contract parser search for a nonexistent combined string.
- Restore the array delimiter only. The SkipDB code and regression fixture are unchanged.
- No local build is claimed; rerun source contracts, compile, and CTest on main.

---
## 2026-11-07 15:32 CST — Retry Date formatting when directives expand past the initial buffer

- Baseline: `576b5cfd2e70351adf13f9745c29ae37859f6753`; mainline run #577 passed source contracts, configure, build, and CTest.
- Scope: `Date_asString()` in `basekit/Date.c` and the already registered `date_string_bounds_test`.
- The existing allocation was sized from format-string bytes plus 1024, but `strftime()` output can be substantially longer than its input directives. On a long repeated `%Y-%m-%d` format, `strftime()` returned zero for insufficient capacity and the API silently returned an empty string.
- Retry `strftime()` with geometrically growing UArray capacity, checking each allocation and bounding retries to a saturating 64x format-length ceiling. Preserve the existing empty-result behavior when formatting still returns zero; no exported symbols or CMake targets changed.
- Extend the existing test with 1,500 repeated date directives and verify every expanded token against a single-token result. Extend the Actions source/runtime contracts. No new files or public API changes.
- Static review confirms size arithmetic is guarded, buffer capacity is revalidated after each resize, and the final UArray size comes from the formatter's reported byte count. No local build is claimed; remote Actions is the compiler/CTest gate.

---
## 2026-11-07 15:34 CST — Repair Date directive token quoting in CI source contract

- Baseline: `63c2f096ceebdbec1c0fbb36456569c2f037d5f9`; run #578 stopped inside the Python source-contract script before configure/build.
- The test-token contract wrapped a C macro containing double quotes in an unescaped Python double-quoted string, causing a Python `SyntaxError`.
- Switch that single contract needle to a Python single-quoted string. The Date implementation and C regression are unchanged.
- Rerun the full mainline contract, compiler, and CTest gates.

---
## 2026-11-07 15:38 CST — Reject numeric-read destinations that alias BStream storage

- Baseline: `af336c118844f99de94318e83c5f4e18124f0850`; mainline run #579 passed source contracts, configure, full build, and CTest.
- Scope: `BStream_readNumber_size_()` in `basekit/BStream.c` and the existing `bstream_tag_boundaries_test`.
- The numeric reader copied bytes with `memcpy()` into a caller-provided destination without checking whether that destination overlapped the stream's own backing allocation. Overlapping ranges make the copy undefined; the subsequent error/zero-fill path could also corrupt bytes that are still the stream's input.
- Reuse the existing byte-range overlap helper before the latched-error branch and before `memcpy()`. Reject an aliased destination, preserve an earlier latched error, leave the cursor unchanged, and never zero-fill the aliased buffer because it is stream storage.
- Add a regression that aliases the destination into the stream buffer, checks the first rejection preserves all bytes and cursor state, then repeats the call with the error latched to prove the second call cannot zero stream data. Declare the existing internal function explicitly in the test translation unit and extend the current Actions source/order contracts.
- Existing `bstream_tag_boundaries_test` CMake registration is reused; no new files, exported APIs, or build-list changes. No local build is claimed; Actions is the compiler/CTest gate.

---
## 2026-11-07 15:39 CST — Correct BStream numeric-read contract delimiters

- Baseline: `9be024ebbea4798a96f3f2aba5297f6beef5f394`; run #580 failed in the Python source-contract phase before configure/build.
- The new contract used double-escaped newline and tab sequences when extracting `BStream_readNumber_size_()`. Python therefore searched for literal backslash characters, failed to find the function boundary and alias-guard closing brace, and rejected the valid C implementation.
- Correct the two Python string delimiters to match the actual source newlines and tab indentation. No C implementation or runtime regression code changed.
- Rerun the full contract, configure, build, and CTest gates.

---
## 2026-11-07 15:42 CST — Avoid reading tm_hour for standalone PortableStrptime %p

- Baseline: `17736e0ffdf6ed591947587e8f4a51d93b784c72`; mainline run #581 passed source contracts, configure, full build, and CTest.
- Scope: `%I`/`%l` and `%p` handling in `basekit/PortableStrptime.c`, plus the existing `portable_strptime_bounds_test`.
- `%p` read `tm->tm_hour` even when no 12-hour field had initialized it. Callers can parse a meridiem token into a partially initialized `struct tm`; inspecting that indeterminate field is undefined behavior. Format order also meant `%p %I` lost the meridiem adjustment.
- Track whether `%I`/`%l` has supplied an hour and retain the AM/PM marker independently. Apply the marker only after the 12-hour value exists, so `%p` alone consumes its input without reading or modifying unrelated `struct tm` fields, and `%p` before `%I` still yields the correct 24-hour result.
- Extend the existing regression for standalone `%p` against byte-patterned `struct tm`, 12 AM/12 PM/1 PM conversions, and meridiem-before-hour ordering. Extend Actions source and semantic contracts. Existing CMake target is reused; no new files, exports, or build-list changes.
- No local build is claimed; remote Actions remains the compiler/runtime gate.

---
## 2026-11-07 15:53 CST — Snapshot UArray format strings that alias the destination

- Baseline: `47751ce1ef0306ec1613215a5da4214380afc7ba`; mainline run #582 passed source contracts, configure, full build, and CTest.
- Scope: `UArray_fromVargs_()` in `basekit/UArray_format.c` and the existing `uarray_format_trailing_percent_test`.
- The formatter advanced through the caller's `format` pointer while appending literal and conversion output to the same `UArray`. When `format` pointed into that array's backing bytes, append could overwrite its terminator or reallocate the backing store, leaving the parser to consume mutated data or a dangling pointer.
- Detect overlap between the complete C format string and the receiver's logical storage using checked byte-size arithmetic and `uintptr_t` address differences. Snapshot only an overlapping format before any append, preserve the receiver unchanged if snapshot allocation fails, and release the snapshot through a shared cleanup path.
- Extend the existing regression by using the receiver's own `value %i` bytes as the format and verifying the appended result is `value %ivalue 7`. Extend the existing Actions source-order and runtime contracts. Existing CMake/CTest registration reused; no new files, exports, or build-list changes.
- No local build is claimed; remote Actions is the compiler/CTest gate.

---
## 2026-11-07 15:56 CST — Preserve the argv NULL sentinel in MainArgs

- Baseline: `4ef5aafca2a92b3b5832b11d7974f8a8125928ab`; SkipDBv2 Regression run #583 passed source contracts, configure, full build, and CTest.
- Scope: `MainArgs_argc_argv_()` ownership/allocation semantics and the existing `main_args_lifecycle_test`.
- The setter deep-copied exactly `argc` entries but did not reserve the conventional `argv[argc] == NULL` terminator. It also represented `argc == 0` as a NULL vector, so callers could not safely use the normal C argv termination convention.
- Allocate an overflow-checked `argc + 1` pointer array for every valid replacement, including an empty vector. `io_calloc` initializes the trailing sentinel; the existing transactional copy/publish/free sequence remains unchanged. Document the invariant in the public header.
- Extend the existing lifecycle regression to require the sentinel after initial assignment, replacement, and empty assignment. Extend Actions source/header/runtime contracts. Existing CMake target reused; no new files, exports, or build-list changes.
- Remote Actions remains the compiler/CTest gate; no local build is claimed.

---
## 2026-11-07 15:58 CST — Reject malformed Datum delimiter Lists

- Baseline: `2845f1c94e30be00eae2e27582922507964adfbd`; SkipDBv2 Regression run #583 passed source contracts, configure, full build, and CTest.
- Scope: delimiter traversal in `Datum_find_()` and `Datum_split_()`, plus the existing `datum_boundaries_test`.
- Both APIs trusted the public List size without checking its backing-storage metadata. A forged size beyond the pointer capacity could turn a tiny malformed descriptor into an oversized scan; additionally, the loops narrowed `size_t` indices to `long` even though `List_at_()` accepts `size_t`.
- Validate delimiter List storage before traversal, snapshot the validated element count once, and pass `size_t` indices directly to `List_at_()`.
- Extend the existing Datum boundary regression with a valid lookup and a malformed shallow List descriptor that both lookup and split must reject. Extend Actions source/runtime contracts; reuse the existing CMake target. No new files, public APIs, or cross-translation-unit symbols.
- No local build is claimed; remote Actions remains the compiler/CTest gate.

---

## 2026-11-07 15:59 CST — Repair Datum delimiter source-contract newline

- Baseline: `d57c81841b717e6d63a6ac3f5740422cf28ce53e`; SkipDBv2 Regression run #584 failed in the Python source-contract step before CMake configure/build.
- Failure cause: the newly added function-body assertion encoded its newline delimiter with one extra backslash. Python searched for the literal characters `\\n}` instead of the source newline followed by a closing brace, so it extracted an empty body and rejected the otherwise intended contract.
- Corrected the workflow delimiter to the normal Python newline escape. No Datum implementation or test code changed in this repair.
- Rerun the source contracts, configure, full build, and CTest on main.

---

## 2026-11-07 16:01 CST — Restore caller errno after Date calendar failures

- Baseline: `5b6ddb24de439828ff48f793cd98b5f4641262eb`; SkipDBv2 Regression run #586 passed the full build and all 57 CTests.
- Scope: the private `Date_calendarToTime_()` conversion boundary and existing `date_time_init_test`.
- The helper saved the caller's `errno`, but restored it only after a successful `mktime()`. When an unrepresentable calendar value made `mktime()` return `(time_t)-1` with an error, the early return leaked that internal error into the caller's ambient errno state.
- Capture `mktime()`'s errno separately, restore the caller's errno on both paths, and test the captured conversion error when distinguishing a valid `-1` timestamp from overflow. Calendar and timestamp publication remain all-or-nothing.
- Extend the existing overflow regression to verify that both `Date_fromLocalTime_()` and `Date_setYear_()` preserve a pre-existing errno on conversion failure. Tighten the existing Actions source contract to enforce capture/restore/check order. No new files, public APIs, or CMake changes.
- No local build is claimed; remote Actions remains the compiler/CTest gate.

---

## 2026-11-07 16:02 CST — Align Date overflow source contracts with errno capture

- Baseline: `e4c18fdc66a1e8aa65e6100b1d7ee247419dddda`; SkipDBv2 Regression run #587 failed in the early XY-C1 source-contract step before configure/build.
- The implementation and focused regression now capture `conversionErrno`, restore the caller's `errno`, then classify the `mktime()` result. A separate top-level required-source list still expected the old inline `errno` check and rejected the corrected implementation.
- Updated that remaining source-contract list to require the new capture/restore/failure-check sequence. No C source or test behavior changed in this correction.
- Remote Actions will rerun the full source contracts, configure, build, and 57-test CTest suite.

---

## 2026-11-07 16:19 CST — Keep UDB commit-marker files exactly one byte

- Baseline: `db077c098b219810250a1cbf0b44f6c91191a6d2`; mainline SkipDBv2 Regression run #588 passed the source contracts, full build, and all 57 CTests.
- Scope: the durable marker writer in `udb/UDBCommitMarker.c` and its existing `commit_marker_test`.
- The marker reader rejects any trailing byte, but the writer only overwrote byte zero. A stale tail could therefore survive a successful write and durability sync, leaving a marker that the next startup would reject as corrupt.
- After flushing the new flag, truncate the opened marker to exactly one byte before the shared EINTR-safe sync barrier. Truncation failure propagates as a failed write, preserving the caller's fail-closed behavior.
- Extend the existing marker regression by appending trailing bytes, confirming the reader rejects the malformed representation, then rewriting and verifying the canonical one-byte file. Tighten the Actions source contract to require truncate-before-sync. Existing CMake target reused; no new files, APIs, or declarations.
- No local compiler/CTest result is claimed; remote Actions is the compiler/runtime gate.

---

## 2026-11-07 16:24 CST — Refuse JFile journaling-mode changes while streams are live

- Baseline: `92f20c2df9a7e8c62f4b835103d6f83647ffa20a`; mainline SkipDBv2 Regression run #589 passed the XY-C1 source contracts, CMake configure, complete build, and all 57 CTests.
- Scope: the mode transition boundary in `jfile/JFile.c` and the existing `jfile_journal_test`.
- The public setter could disable journaling while the data and journal streams were open. If a previously committed journal had not yet been replayed, subsequent writes went directly to the data file while the old committed journal remained pending; the next commit/replay could overwrite those newer bytes with stale journal content.
- Permit mode changes only when both managed streams are closed. This preserves the existing pre-open configuration use while making an unsafe live transition a no-op.
- Extend the existing journal regression to reproduce a committed-but-unapplied record, attempt the live mode change, and prove the newer journaled value wins. Also verify that changing modes after close still enables the direct-write path and survives reopen. Extend Actions source, ordering, and runtime contracts. Existing CMake target reused; no new files, public declarations, or build-list changes.
- No local compiler/CTest result is claimed; remote Actions is the compiler/runtime gate.

---

## 2026-11-07 16:34 CST — Use caller-owned calendar storage for Date local-time conversion

- Baseline: `8a97b71efe4b30de934e0b1f130ee6c1061ac2fb`; SkipDBv2 Regression run #590 passed source contracts, configure, full build, and all 57 CTests.
- Scope: the shared calendar conversion boundary in `basekit/Date.c`, all Date component accessors/setters, parsing's `EmptyTM()` template, and the existing Date formatting path.
- The shared `localtime()` result points to process-global static storage. Concurrent Date calls could overwrite that `struct tm` between conversion and use, making independent calendar reads race and setters normalize a different date/time than the one they started with.
- Convert into caller-owned `struct tm` storage via `localtime_r()` on POSIX and `localtime_s()` on Windows. Keep the legacy Symbian fallback isolated behind its platform guard. Route component getters/setters, parsing's epoch template, and `Date_asString()` through the shared conversion helpers; no Date entry point on the supported Windows/POSIX paths reads shared `localtime()` storage.
- Strengthen the existing Actions source/order contracts to require the reentrant platform calls, the three-argument receiver helper, caller-owned calendar fields, and the absence of direct shared-buffer use in formatting/parsing. The existing Date/Date-string CTest targets and CMake registration are reused; no new files, public declarations, or build-list changes.
- No local compiler/CTest result is claimed; remote Actions remains the compiler/runtime gate.

---

## 2026-11-07 16:35 CST — Align Date leap-year source contract with caller-owned calendar

- Baseline: `e3bc72ce0d503882bc8591ef347b43ddeac3ae63`; SkipDBv2 Regression run #591 stopped in the XY-C1 source-contract stage before CMake configure/build.
- The Date refactor correctly replaced the shared `tm` pointer with a stack-owned `calendar`, but the earlier static XY-C1 contract still required the old `tm->tm_year` expression. The dynamic Date contract had already been updated to the new storage.
- Change the single stale Date.c source needle to `calendar.tm_year`. No C implementation, test, CMake registration, or build target changed; the next run is the compile/CTest gate.

---

## 2026-11-07 16:36 CST — Update Date local-calendar NULL guard contract

- Baseline: `4e71b2e311bdd5040acdc1e6b394442b95e241dd`; SkipDBv2 Regression run #592 passed the file/path inventory but stopped in the XY-C1 source-contract stage before configure/build.
- The source-contract map still expected the former two-argument Date local-time helper's NULL check. The reentrant helper now also requires a non-NULL caller-owned `struct tm` destination.
- Update that single static contract needle to require all three pointer checks. No C implementation, CTest, CMake, or build target changed; the next run is the compiler/runtime gate.

---

## 2026-11-07 16:37 CST — Align Date formatting contract with local calendar storage

- Baseline: `6e67144049dac0ddfd1bdf9d00e14cc597bf4730`; SkipDBv2 Regression run #593 stopped in the XY-C1 source-contract stage before configure/build.
- The production formatter now passes its stack-owned `calendar` to `strftime()`, but both the static integration needle and the focused Date formatting contract still described the former shared `tm` pointer.
- Update both contract expressions to require `&calendar`. No C source or test behavior changed; the next Actions run remains the compile/CTest gate.

---

## 2026-11-07 16:38 CST — Replace the active Date parsing template with reentrant conversion

- Baseline: `21caba8bf10a2912114d771d10b10adc17a95435`; SkipDBv2 Regression run #594 reached the focused Date source contract and rejected the still-active `EmptyTM()` implementation.
- Review of the contract exposed that the prior edit had inserted a caller-owned `EmptyTM()` before the format section instead of replacing the original definition. The old, later definition remained the one used by parsing and still called shared-buffer `localtime()`.
- Remove the accidental duplicate and update the sole original `EmptyTM()` to use `Date_localTimeForTimestamp_()`. Verify one definition remains and that the only raw `localtime()` call is isolated inside the Symbian compatibility branch. No public API, test target, or CMake change.

---

## 2026-11-07 16:39 CST — Count the isolated Symbian localtime call, not its comment

- Baseline: `5ad3f6c9d8eb8c1ab64e1bab2678e82e26ba3b4d`; SkipDBv2 Regression run #595 passed the main source needles and failed the focused Date parsing/formatting contract before configure/build.
- The new assertion counted every `localtime(` substring in Date.c, so it counted both the intentional Symbian compatibility call and the explanatory comment describing the legacy API. The function-level checks already ensure parsing and formatting do not call it.
- Narrow the count to the exact Symbian fallback statement, `struct tm *shared = localtime(&timestamp);`. No production C logic, tests, CMake targets, or source lists changed.

---

## 2026-11-07 16:40 CST — Accept Date_second's guarded zero fallback in the accessor contract

- Baseline: `10b77b183a6d4033e93669eba40fcdbb76cd611d`; SkipDBv2 Regression run #596 passed the XY-C1 inventory and earlier Date contracts, then stopped in the Date accessor source-order assertion.
- `Date_second()` correctly checks the reentrant conversion and returns `0.0` when conversion fails. The contract matcher found the start of the `if (!Date_localTime_(...))` guard, which is textually just before the call itself, and mistakenly treated that guard position as the fallback result.
- Keep the accessor check strict: when no ternary `: 0` fallback exists, require `return 0.0;` after the explicit local-time failure guard. No production C or test changes.

---

## 2026-11-07 16:42 CST — Preserve BStream_show's full-width cursor

- Baseline: `1f24db0ecc4328cf76b9f16996d81d5dba367f6a`; SkipDBv2 Regression run #597 passed the source-contract stage, full build, and all 57 CTests.
- Scope: cursor save/restore in `basekit/BStream.c:BStream_show()` and the existing `bstream_tag_boundaries_test`.
- `BStream.index` is a `size_t`, but the debug display routine saved it in an `int`. On 64-bit builds (and at values beyond `INT_MAX`), invoking the diagnostic could truncate the caller's cursor when restoring it.
- Store the saved position as `size_t`. Add a focused regression using a tiny valid tagged stream and a synthetic cursor at `INT_MAX + 17`; `BStream_show()` must scan from zero and restore the exact original position without setting a read error.
- Reuse the current CTest target. Tighten the BStream source contract and add a focused workflow contract for save/reset/restore ordering and the wide-cursor regression. No new files, public declarations, header changes, or CMake changes.
- No local compiler/CTest result is claimed; GitHub Actions is the compiler/runtime gate.

---

## 2026-11-07 17:02 CST — Reject overlapping ConvertUTF source and target ranges

- Baseline: `e4a645471e9c3dbb6437573e84ec4eb6d32165b4`; mainline SkipDBv2 Regression run #598 passed the source-contract stage, full build, and all 57 CTests.
- Scope: the six public buffer converters in `basekit/ConvertUTF.c`, their shared private range check, and the existing `convert_utf8_bounds_test`.
- Each converter walks input and output forward. If caller-supplied source and destination spans overlap, an early output write can overwrite source code units that have not yet been decoded.
- Add one overflow-checked private overlap predicate using byte extents and `uintptr_t` address differences. Reject overlapping spans with `sourceIllegal` before reading or writing caller data or changing the cursors; document the non-overlap contract.
- Extend the existing test across all six conversions, identical-start and partial overlap, state preservation, and a disjoint-range success case. Extend static and function-order Actions contracts. Reuse the current CTest target; no new files, public symbols, or CMake changes.
- No local compiler/CTest result is claimed; GitHub Actions remains the compiler/runtime gate.

---

## 2026-11-07 17:03 CST — Match ConvertUTF overlap regression diagnostics to source contracts

- Baseline: `fec862cb3ed9c5af8fc356d9d67264e25d9b6ae5`; SkipDBv2 Regression run #599 stopped in the static source-contract stage before CMake configuration.
- The new overlap tests built diagnostic messages by concatenating a function label with a common suffix, while the existing workflow contract deliberately requires each full regression diagnostic as a literal source needle.
- Keep the test strict and explicit: make each full error message a literal argument and let the shared macro print it unchanged. No production-code change, test-target change, or workflow relaxation.
- No compiler/CTest result was reached in run #599.

---

## 2026-11-07 17:04 CST — Reject UArray trailing-terminator ownership aliases

- Baseline: `4d221738e60d90f07ccd2e05223fe37531520896`; `SkipDBv2 Regression` run #600 passed the XY-C1 integration contracts, complete build, and all 57 CTests.
- Scope: `UArray_setData_type_size_copy_()` in `basekit/UArray.c` and its existing `uarray_stack_resize_test`.
- Both source-alias checks excluded the exact end of the logical byte range. For owned UArrays, that address is the allocation's trailing terminator: `copy=0` could free the allocation and then publish an interior pointer as the new owner, causing an invalid free later; `copy=1` could realloc before reading the aliased terminator, invalidating the source if realloc moved the block.
- Treat the logical end as an alias boundary. The copying path snapshots exactly the single trailing terminator byte before realloc; other reads extending beyond the logical payload are rejected. The no-copy path rejects pointers at or inside the current logical extent before freeing or publishing any storage.
- Add focused regressions for both no-copy ownership rejection and copy-before-realloc semantics. Reuse the existing CTest target; no new files, public declarations, or CMake source-list changes. Extend the Actions test and source-order contracts.
- No local compiler/CTest result is claimed; GitHub Actions remains the compiler/runtime gate.

---

## 2026-11-07 17:07 CST — Detach array-heap storage before destructor callbacks

- Baseline: `48e8c1d526a9c36a00624854be4de6854a36398b`; SkipDBv2 Regression run #601 passed XY-C1 integration contracts, complete build, and all 57 CTests.
- Scope: the ownership/lifecycle path in `mgr/array-heap.c:array_free()`, its existing `array_heap_test`, and the API comment.
- `array_free()` called element destructors while the array still exposed its owned backing store. A destructor that recursively freed or reinitialized the same array could re-enter the old element walk, double-release the original storage, or have the outer call free newly created storage.
- Snapshot the old backing pointer and valid callback count, reset the public struct before the first callback, run callbacks only over the detached storage, then free that old allocation exactly once. Recursive frees now see an empty array; callback-time reinitialization remains owned by the caller.
- Add a regression whose first destructor recursively frees and reinitializes the array; assert the original callbacks run once and the new allocation survives the outer free. Reuse the existing `array_heap_test` CMake/CTest target; no new files or symbols.
- No local compiler/CTest result is claimed; GitHub Actions remains the compile/runtime gate.

---

## 2026-11-07 17:12 CST — Reject malformed existing SkipDB record levels before resize

- Baseline: `1c45c3cce044c500bfbfa03c30b30f59459d772e`; mainline SkipDBv2 Regression run #601 passed the integration contracts, complete build, and all 57 CTests.
- Scope: `SkipDBRecord_level_()` and the existing `skipdb_core_layers_test`.
- The setter validated the requested level but trusted `self->level` as the old allocation extent. If a record's public metadata was corrupted to a negative level, growing the pointer array formed `new_pointers + old_level` before the allocation and passed a derived oversized length to `memset`, causing an out-of-bounds write.
- Reject an existing level outside `[0, SKIPDB_MAX_LEVEL]` before equality checks, pointer arithmetic, or `realloc`. Leave both the malformed metadata and pointer ownership untouched; normal valid-level growth/shrink behavior remains unchanged.
- Add regressions for negative and over-maximum existing levels. Extend the static source contract and a function-order check so the old-level guard must precede `realloc`. Reuse the current CTest target; no new files or CMake changes.
- No local compiler/CTest result is claimed; GitHub Actions remains the compiler/runtime gate.

---

## 2026-11-07 17:14 CST — Fix SkipDBRecord level contract newline boundary

- Baseline: `6322614f6db948772625987c879a17e60623234f`; run #603 failed in the XY-C1 source-contract step before CMake configuration.
- The new function-body check searched for the two literal characters `\\n` instead of the newline separating `SkipDBRecord_level_()` from `SkipDBRecord_object_()`. The implementation and regression were not reached by the build.
- Use `chr(10)` as the function boundary delimiter. Keep the guard-order assertions unchanged; no production C or test behavior changed.
- No compiler/CTest result was reached in run #603.

---

## 2026-11-07 17:19 CST — Revalidate the SkipDB eviction cursor after object callbacks

- Baseline: `d94121ab3c7116ab5e1244b73ce7d37beecc2fcf`; mainline SkipDBv2 Regression run #604 passed the XY-C1 source contracts, full build, and all 57 CTests.
- Scope: `SkipDB_evictUnmarkedCacheRecords_()` and the existing `skipdb_core_layers_test`.
- The LRU walk cached `record->olderRecord` before deallocation. `SkipDBRecord_dealloc()` detaches the current row, then may invoke an arbitrary object-free callback; a callback that calls `SkipDB_clearCache()` can free the saved next row before the outer walk resumes, causing a use-after-free/double-free.
- Snapshot the next row's PID and integer address before deallocation. After a callback, resume only when the PID cache still maps that PID to the exact same address; otherwise stop the stale walk. Clear all surviving cache marks even when the callback interrupted traversal.
- Extend the existing core-layers test to make the finalizer reenter cache clearing during actual high-water eviction, with adjacent LRU rows and persistence/reopen assertions. Extend static and function-order Actions contracts. No new files, public symbols, or CMake changes.
- No local compiler/CTest result is claimed; GitHub Actions remains the compiler/runtime gate.

---

## 2026-11-07 17:21 CST — Check JFile path sizing additions before arithmetic

- Baseline: `ede6b8582d2f87c2046cc03a73605b6dbd907731`; mainline SkipDBv2 Regression run #605 completed successfully.
- Scope: the two path-construction setters in `jfile/JFile.c` and their existing `jfile_journal_test`.
- The setters used chained expressions such as `SIZE_MAX - name_len - 6` and `SIZE_MAX - ext_len - 2`. If a component length approaches the representable limit, the subtraction itself can wrap before the guard compares it, so later allocation-size additions are not proven safe.
- Replace chained subtraction with sequential checked additions: validate the suffix, separator, and prefix one at a time for `JFile_setLogPath_()`; separately account for the dot, extension bytes, and terminating NUL in `JFile_setPath_withExtension_()`. Make the basename helper file-local and reject a null object/path before dereferencing it.
- Add path-setter alias regressions using `self->path` as the input to both setters. Extend the current Actions source-order contract so each size check must precede its corresponding addition. Existing CMake/CTest registration is reused; no new files or exported declarations.
- No local compiler/CTest result is claimed; GitHub Actions remains the compile/runtime gate.

---

## 2026-11-07 17:25 CST — Reject Windows wall-clock seconds outside timeval range

- Baseline: `bcc82f840626a8594a190d24b888cdbf166ee748`; mainline SkipDBv2 Regression run #606 passed the XY-C1 source contracts, complete build, and all 57 CTests.
- Scope: the Windows `gettimeofday()` adapter in `basekit/PortableGettimeofday.c`.
- On 64-bit Windows, CRT `time_t` can represent dates beyond the signed 32-bit range of `long`, while the adapter's `timeval.tv_sec` field remains a `long`. Directly casting `_timeb.time` can therefore wrap/lose the real timestamp after the 2038 boundary.
- Check both `LONG_MIN` and `LONG_MAX` immediately after the successful clock read. Return `-1` with `EOVERFLOW` before writing either timeval field when the timestamp cannot be represented; normal in-range timestamps retain the existing behavior.
- Extend the existing Actions contract and order assertion to require the range check before assignment. Reuse the existing `portable_gettimeofday_test`; no new files, declarations, or CMake changes. The Linux CI build does not execute the Windows-only branch, so the new Windows behavior is statically contract-checked rather than claimed as runtime-tested here.
- No local compiler/CTest result is claimed; GitHub Actions remains the compiler/runtime gate.

---

## 2026-11-07 17:28 CST — Unify UDBRecords partial-construction cleanup

- Baseline: `5fbf29156cbf1b0a4f1ce7cd94b9d6bc8f82cd39`; mainline SkipDBv2 Regression run #607 passed source contracts, the full build, and all 57 CTests.
- Scope: the `UDBRecords_new()` constructor and its existing `UDBRecords_free()` lifecycle path.
- Allocation failure and default-path initialization failure previously duplicated the same four resource releases. This makes constructor rollback easy to drift from the normal destructor when fields are added or ownership changes.
- Route both failures through one cleanup label and the canonical NULL-safe destructor. The destructor already handles partially initialized `JFile` and `UDBRecord` members; no resource is published and successful construction remains unchanged.
- Tighten the existing Actions source contract and add an ordering/count assertion for the single rollback path. No new files, symbols, CTest targets, or CMake changes.
- No local compiler/CTest result is claimed; GitHub Actions remains the compiler/runtime gate.

---

## 2026-11-07 17:30 CST — Unify UDBIndex partial-construction cleanup

- Baseline: `a76d9ad9138659d72a9dec052ea04d5f8a7dbe72`; mainline SkipDBv2 Regression run #608 passed source contracts, the complete build, and all 57 CTests.
- Scope: `UDBIndex_new()` and its canonical `UDBIndex_free()` lifecycle path.
- Allocation failure and default-path setup failure duplicated cleanup of the JFile and both PID lists. These error branches could drift from the destructor as the index acquires more owned state.
- Route both failures through one rollback label and `UDBIndex_free(self)`. The destructor and `UDBIndex_close()` are NULL-safe for partially initialized fields; successful construction and ownership semantics remain unchanged.
- Extend the existing Actions source map and dynamic constructor-order/count contract. No new files, symbols, CTest targets, or CMake changes.
- No local compiler/CTest result is claimed; GitHub Actions remains the compiler/runtime gate.

---

## 2026-11-07 17:35 CST — Preflight PID-hole capacity before clearing index mappings

- Baseline: `28479b7674e71529c2e0020277284b669a755e7b`; mainline SkipDBv2 Regression run #609 passed the source contracts, complete build, and all 57 CTests.
- Scope: `UDBIndex_setPos_forPid_()` and its existing PID index regression.
- A deletion wrote the zero-valued index entry to the journal before `UDBIndex_releasePid_()` attempted to insert the freed PID into the reusable-hole list. If list growth failed, the API returned failure after mutating the journal, so a failed delete could still be replayed by a later commit.
- Validate the hole-list storage and preallocate one slot before the mapping write when a release is required. Verify the resulting capacity, while avoiding allocation when the PID already exists in the hole list. Failure now exits before touching the index journal; normal release still centralizes deduplication and reservation transfer in the free-list owner.
- Extend `udb_index_layers_test` for both reserved and already-mapped PIDs, asserting failed hole allocation does not advance the journal or clear a committed mapping. Add Actions source-order contracts for the capacity preflight. No new files, exported declarations, or CMake changes.
- No local compiler/CTest result is claimed; GitHub Actions remains the compiler/runtime gate.

---
## 2026-11-07 17:45 CST — Validate length-delimited IPC keys before framing

- Baseline: `ff01036bacfde1078e3287806bdbba23cbbc8152`; SkipDBv2 Regression run #610 passed the strict source contracts, full build, and all 57 CTests.
- Scope: shared IPC protocol-field validation and the `dbus` environment-backed bulk writer.
- `dbus bulk_key()` copied environment-variable names directly into the space-delimited request key. Environment entries can contain spaces/control bytes even if typical shell assignment syntax cannot create them; the receiver then parses the malformed key as a different key plus payload, risking writes to an unintended setting.
- Add a length-aware validator that scans exactly the supplied slice (including NUL/control rejection) and make the existing NUL-terminated helper delegate to it. Validate the complete environment-derived key before copying/framing it.
- Extend existing IPC transport cases to cover non-NUL-terminated valid slices, embedded whitespace/NUL, empty-field policy, and length bounds. Update source contracts to verify both validator ordering and validation-before-copy; all files already participate in existing CMake targets, so no target/file additions are needed.
- No local compiler/CTest result is claimed; GitHub Actions is the compiler/runtime gate.

---
## 2026-11-07 17:48 CST — Reject cached zero-level rows during tail traversal

- Baseline: `4015dc39d12fc59e9e75559f35115587a84737f0`; mainline SkipDBv2 Regression run #611 passed the strict source contracts, full build, and all 57 CTests.
- Scope: `SkipDBRecord_findLastRecord()` and the existing cursor/link regression.
- Disk-loaded non-root records with level zero are rejected, and key search rejects zero-level forward targets. But tail traversal allowed a malformed cached row through `SkipDBRecord_hasValidLevel_()` because level zero is within the generic range; its empty level loop made it appear to be a valid last record.
- Reject a level-zero current record unless it is the database header, clear the cached update vector, and close the UDB on corruption. Extend the existing test to persist a zero-level final row, query the last record while it remains cached, and then reopen to verify on-disk corruption still fails closed.
- Update the workflow's static and dynamic source contracts to enforce the guard before the link walk. Existing test/CMake paths are reused; no new source files or exported declarations.
- No local compiler/CTest result is claimed; GitHub Actions remains the build/runtime gate.

---
## 2026-11-07 17:49 CST — Correct fail-closed regression expectation

- CI run #612 compiled all targets and passed the source/build gates; `skipdb_cursor_previous_test` reached the new corruption case and showed the implementation closed UDB as intended.
- The new test assertion inverted the expected closed-state check (`!SkipDB_isOpen` was mistakenly treated as a failure). Correct it to fail only if last-record traversal returns a row or the database remains open, and align the source contract with that assertion.
- No production-code change; this is the minimal in-place correction for the red mainline test.

---
## 2026-11-07 17:50 CST — Repair workflow contract quoting

- CI run #613 stopped in the early XY-C1 integration-contract step, before compilation: the new assertion needle in the Python source-contract list was missing its closing quote.
- Restore the closing quote only; the C build and CTest stages had not run in this attempt.

---
## 2026-11-07 17:52 CST — Snapshot UArray copies that alias the trailing terminator

- Baseline: `9c8058e133fcd525380fdc77416ced290865694e`; SkipDBv2 Regression run #614 passed the source contracts, full build, and all 57 CTests.
- Scope: `UArray_copyData_()` overlap detection and the existing UArray stream-I/O regression target.
- The copy helper excluded empty destinations and used a strict `< selfBytes` bound for source starts inside the current buffer. A valid borrowed one-byte view of the trailing NUL therefore bypassed the snapshot; resizing could invalidate the source before the copy.
- Include the logical end position in the overlap test and do not gate it on a nonzero logical byte count. This safely snapshots a trailing-terminator alias for both populated and empty arrays before `UArray_setSize_()` runs.
- Extend the existing `uarray_stream_io_test` with populated/empty terminator-view cases and add a workflow source-order contract requiring inclusive-tail detection and snapshot-before-resize. No new files, declarations, or CMake target changes.
- No local compiler/CTest result is claimed; GitHub Actions remains the build/runtime gate.

---
## 2026-11-07 17:54 CST — Validate UArray layouts before insert-every arithmetic

- Baseline: `8906776f32240d305181efa4afa6cec3cb450ba3`; SkipDBv2 Regression run #615 passed all source contracts, the full build, and 57 CTests.
- Scope: `UArray_insert_every_()` and its existing malformed-layout regression.
- The routine checked only `itemSize` and multiplication bounds. When `itemType` claimed a wider C type than `itemSize`, raw byte arithmetic could produce a buffer sized for the forged width, then transfer it to `UArray_setData_type_size_copy_()` using the wider type, whose terminator write could exceed that allocation.
- Reuse `UArray_hasValidItemLayout_()` for both source and destination before deriving byte counts; keep the overflow guard for `itemCount`. Extend the existing test to cover malformed source and destination while verifying data and metadata stay unchanged. Update static/dynamic workflow contracts and the expected validator call count; no new files or CMake changes.
- No local compiler/CTest result is claimed; GitHub Actions remains the build/runtime gate.

---
## 2026-11-07 17:56 CST — Avoid double-committing the journal before truncation

- Baseline: `bd066c3d32c2c36d58e52a24101d797d90849b58`; SkipDBv2 Regression run #616 passed the workflow contracts, full build, and all 57 CTests.
- Scope: `JFile_truncate_()` and the existing journal regression test.
- `JFile_truncate_()` called `JFile_commitToLog()` and then `JFile_commitToFile()`, which itself calls `JFile_commitToLog()`. The first call converts the terminal item marker into a transaction marker; the second then expects an item marker, treats the already committed journal as invalid, closes the journal stream, and returns before truncation.
- Let `JFile_commitToFile()` own the single prepare/replay sequence. Add a regression that truncates with a pending journal entry, verifies both streams remain open and the data extent is empty, then performs a successful write/commit on the same object.
- Extend the existing source contracts to ensure truncation does not directly double-commit and that the regression remains registered. No new files or CMake changes.
- No local compiler/CTest result is claimed; GitHub Actions remains the build/runtime gate.

---
## 2026-11-07 18:00 CST — Preserve record boundaries when compacting into larger holes

- Baseline: `3c631b805bfdefd44fa90d5b937881a59f04e9e0`; SkipDBv2 Regression run #617 passed the source contracts, full build, and all 57 CTests.
- Scope: `UDB_compactStepOnce_()` and the existing UDB record-storage regression.
- The compactor accepted any hole at least as large as the next live row, copied the row to the hole start, and wrote an empty header at `hole_start + row_size`. For a larger hole, that position is still inside the old hole, while the original live row at the old hole end remained non-empty. The persisted free-list pointer, physical scan boundaries, and live row could therefore disagree after a successful compaction.
- Validate both persisted extents before mutation. Reject a nonzero remainder smaller than a complete record header. When the hole is larger, write the correctly sized residual empty extent and tombstone the old source row before changing the PID mapping. Exact-fit moves preserve the existing single-tombstone path.
- Extend `udb_record_storage_test` with a large-hole/small-row fixture, assert the moved value and residual pointer, and walk every physical row through EOF. Add workflow source contracts for split/tombstone/map ordering. No new files, CMake changes, or exported declarations.
- No local compiler/CTest result is claimed; GitHub Actions remains the compiler/runtime gate.

---
## 2026-11-07 18:14 CST — Align time-sliced compaction count with split-hole merges

- Baseline: `23f854636f1bfe1bd073375c46990665ba91b83d`; SkipDBv2 Regression run #618 passed the integration contracts and full build, but CTest failed only in `udb_reserved_pid_test`.
- Scope: the existing time-sliced compaction assertion and its workflow source contract; no production algorithm change or new files.
- The larger-hole fix now preserves the residual free extent and tombstones the old live-row extent. In this fixture, the subsequent time slice performs two successful adjacent-hole merges before the trailing-hole truncation, so `UDB_compactStepFor_(db, 5.0)` correctly reports 2 rather than the old expected count of 1.
- Update the regression's expected compaction count and assert that exact expectation in the existing workflow contract. Keep the database-open, live-value, removed-PID, and tail-shrink assertions unchanged.
- No local compiler/CTest result is claimed; GitHub Actions remains the validation gate.

---

## 2026-11-07 18:16 CST — Make SkipDB record deserialization atomic

- Baseline: `2740ec2ecf0c63a808f54b21cdb171ced29f29d0`; SkipDBv2 Regression run #619 passed the integration/source contracts, full build, and all 57 CTests.
- Scope: `SkipDBRecord_fromStream_()` and its existing record-storage regression target.
- The decoder previously wrote key/value arrays and pointer metadata directly into the caller's record before proving that the tagged stream was complete. A later truncation, invalid PID/link, or trailing byte could return failure after leaving the reused record partially overwritten.
- Decode into a detached record, and swap payload/link fields into the caller only after the full stream validates. Preserve caller identity, database/cache metadata, PID, and old key-ownership semantics; release the old payload through the temporary record's destructor.
- Extend the existing test to verify truncated and trailing-data failures preserve key, value, level, link PID, and suffix metadata. Update the workflow's static contract and add a source-order assertion. No new files, exported symbols, or CMake target changes.
- No local compiler/CTest result is claimed; GitHub Actions remains the compiler/runtime gate.

---

## 2026-11-07 18:17 CST — Restore single-owner stream readiness validation

- Baseline: `6c2ae64a76df92f82f4233d217bd505c4faf55cf`; run #620 stopped at the source-contract gate before configuration, compilation, or CTests.
- The staged decoder repeated `SkipDBRecord_streamIsReady_()` after the public entry point had already validated the caller and stream, changing the established centralized-helper count from three to four.
- Remove the redundant private guard; the private decoder is only called by the checked public wrapper with a newly allocated record. Keep error-buffer reset and all bounds/tag checks unchanged.
- No build or runtime test ran in failed run #620; the new commit will rerun the complete Actions gate.

---

## 2026-11-07 18:20 CST — Centralize tagged-array alias snapshots in BStream

- Baseline: `3bd47426de801b9eb5d0eaad0829cb8e404a9beb`; SkipDBv2 Regression run #621 passed the source contracts, complete build, and all 57 CTests.
- Scope: `BStream_writeTaggedUArray_()` in the BaseKit binary-stream module.
- The tagged UArray wrapper duplicated overlap detection and cloned the array before handing it to `BStream_writeTaggedData_length_()`, which already owns the full framing operation, inclusive trailing-terminator overlap snapshot, and rollback path. The duplicate range logic used a strict end bound and could drift from the shared helper.
- Keep source/stream CTYPE and extent validation in the wrapper, then delegate the byte view directly to the shared writer. This removes the redundant clone/range implementation and leaves alias detection and rollback with one owner.
- Retain the existing `testBStreamAliasedWrites()` trailing-terminator regression and extend the dynamic workflow contract to require delegation and forbid duplicate wrapper snapshots.
- No local compiler/CTest result is claimed; GitHub Actions remains the validation gate.

---

## 2026-11-07 18:23 CST — Preserve UDB record cursor during free-list metadata I/O

- Baseline: `804b982d1a7665b4f6303ad9b7dbde4fe8c3f32d`; SkipDBv2 Regression run #622 passed the XY-C1 source contracts, full build, and all 57 CTests.
- Scope: `UDBRecords_firstEmptyRecordPos()` and `UDBRecords_firstEmptyRecordPos_()` in the existing free-list module.
- The getter sought to offset zero and returned with the tracked cursor at the metadata boundary. The setter restored `pos/maxPos` on a failed write but left the cursor changed after a successful write. Both APIs therefore leaked internal metadata I/O into the caller's record-stream state.
- Snapshot and restore the tracked `JFile` position and extent on successful metadata reads and writes; leave the existing fail-closed read-error behavior and failed-write rollback untouched.
- Extend the existing `udb_record_storage_test` to exercise both APIs inside a transaction with the cursor positioned at a live record. Add workflow source contracts checking the snapshots/restores and test coverage. No new source files or build targets.
- No local compiler/CTest result is claimed; GitHub Actions remains the compiler/runtime gate.

---

## 2026-11-07 18:25 CST — Preserve the UDB PID-index cursor during lookups

- Baseline: `c0e91489cb7946d13bd50b21d421380814895e98`; SkipDBv2 Regression run #623 passed the source contracts, full build, and all 57 CTests.
- Scope: `UDBIndex_posForPid_()` in the fixed-width PID-to-offset mapping module.
- A normal lookup sought to the PID slot and returned without restoring `JFile.pos`. Besides making a read accessor mutate caller state, this caused `UDBIndex_setPos_forPid_()` to snapshot the already-advanced cursor rather than the caller's original cursor for its failure path.
- Snapshot and restore the logical cursor and file extent around the slot read, including read failure; retain the error flag so callers can still fail closed.
- Extend `udb_index_layers_test` and the workflow's strict source contract to cover successful lookup cursor preservation and enforce restore-before-return ordering. No new files or CMake targets.
- No local compiler/CTest result is claimed; GitHub Actions remains the validation gate.

---

## 2026-11-07 18:26 CST — Repair workflow source-contract syntax after run #624

- Baseline: `3cdbd676241342f869ea301f9fb2b6db0a250e26`; run #624 stopped during the source-contract script parse, before configuration, compilation, or CTests.
- The PID lookup regression source-contract list was missing its closing quote on the existing `UDB_close(db);` token, causing Python's “unterminated string literal” error.
- Correct only the malformed workflow token and rerun the unchanged source/build/test gate. The PID lookup implementation and regression remain untouched in this repair.
- No compiler or runtime result was produced by run #624.

---

## 2026-11-07 18:29 CST — Validate PID free-list storage before direct traversal

- Baseline: `f91746bcdb1a6705fe624cdf1d50da93cc9f4544`; SkipDBv2 Regression run #625 passed the integration/source contracts, complete build, and all 57 CTests.
- Scope: PID reservation/free-hole pool in `UDBIndexFreeList.c`.
- Several operations iterated `reserved->items` directly, and allocator paths treated a `List_top()` NULL result as an empty pool. If list metadata described a nonzero size with missing/undersized storage, these paths could dereference invalid memory or allocate a new PID while the free stack was unusable.
- Add one private validator for both PID stacks, use it before list reads/mutations and before reservation iteration in commit preparation, and strengthen the push postcondition. Invalid internal storage now fails closed rather than being interpreted as an empty stack.
- Extend `udb_index_layers_test` with malformed hole/reservation storage fixtures and strengthen the workflow source contract. Existing CMake target reused; no new files.
- No local compiler/CTest result is claimed; GitHub Actions remains the validation gate.

---

## 2026-11-07 18:32 CST — Preserve the UDB PID-index cursor while counting slots

- Baseline: `81a4e8660c2d618b57ee33c8080fe0f31d12b27a`; SkipDBv2 Regression run #626 passed the source contracts, complete build, and all 57 CTests.
- Scope: `UDBIndex_pidCount()` in the fixed-width PID map module.
- Counting slots used `JFile_setPositionToEnd()` and returned the file wrapper's logical position at EOF; malformed extents also returned with the mutated cursor. Traversal callers therefore could unexpectedly disturb an index cursor just by asking for a bound.
- Snapshot and restore the logical position/extent before validating the observed EOF and alignment, keeping invalid counts fail-closed and the cursor unchanged on every return.
- Extend `udb_index_layers_test` for valid and misaligned PID counts, with workflow source-order assertions. Existing test target reused; no new files or CMake changes.
- No local compiler/CTest result is claimed; GitHub Actions remains the validation gate.

---

## 2026-11-07 18:34 CST — Reject foreign records at SkipDB cache-age entry points

- Baseline: `d7a3fae074f2c7972a5819746c3227a830dfed6c`; SkipDBv2 Regression run #627 passed the integration contracts, complete build, and all 57 CTests.
- Scope: `SkipDB_noteNewRecord_()` and `SkipDB_noteAccessedRecord_()` in `SkipDBCache.c`.
- The cache age-list mutators checked only for NULL. Passing a live record owned by another SkipDB could unlink it from its real owner's age list, splice it into the wrong database, and/or inflate the wrong cache count, breaking eviction invariants.
- Require `record->sdb == self` before changing the cached-record count or touching age links. Normal record construction binds the owning database before cache registration, so legitimate insertion remains unchanged.
- Extend the existing two-database core regression to call both entry points with a foreign record and verify both age-list roots, ownership links, and cached counts remain unchanged. Add workflow source-order checks; no new files or build targets.
- No local compiler/CTest result is claimed; GitHub Actions remains the validation gate.

---

## 2026-11-07 18:51 CST — Validate cached forward-link PID accessors

- Baseline: `40917e5041902c20bef3c038f14cb5386e78e2fe`; SkipDBv2 Regression run #628 passed the integration/source contracts, complete build, and all 57 CTests.
- Scope: cached forward-link ownership and PID resolution in `SkipDBRecordLinks.c`.
- The record-pointer accessor already rejected foreign owners and mismatched cached PIDs, but the PID-only accessors could return/cache a foreign PID or allocate through a foreign record before applying the same invariant. The link setter also accepted a cross-database target and deferred failure until a later traversal.
- Centralize the owner/PID check, apply it before PID reads/allocations and disk-loaded cache registration, and refuse cross-owner setters before mutating the pointer slot. Extend the existing two-database core regression and strict workflow contracts; no new files, CMake targets, or exported declarations.
- No local compiler/CTest result is claimed; GitHub Actions remains the build/runtime gate.

---

## 2026-11-07 18:53 CST — Reject foreign records from dirty and PID caches

- Baseline: `e5fbfd7d87efdf80b431b8dbd264d76a378b80b7`; SkipDBv2 Regression run #629 passed the integration contracts, full build, and all 57 CTests.
- Scope: `SkipDB_noteDirtyRecord_()` and `SkipDB_noteAssignedPidToRecord_()` in `SkipDBCache.c`.
- Both notification paths validated NULL/open state but not record ownership. A record from another database could be queued for saving in the wrong transaction or overwrite that database's PID-to-record cache entry.
- Reject records unless `record->sdb == self` before queue insertion or PID-cache mutation. Extend the existing two-database core regression to assert the foreign record leaves dirty-queue length and PID-cache mapping unchanged, and strengthen static/dynamic Actions source contracts.
- Reuse existing files, declarations, and CTest target; no new source files or CMake changes. No local compiler/CTest result is claimed; GitHub Actions remains the build/runtime gate.

---

## 2026-11-07 18:56 CST — Enforce List backing-slot geometry

- Baseline: `be8165380da6b6f824cffccd63c69de3d10222d6`; SkipDBv2 Regression run #630 passed integration/source contracts, full build, and all 57 CTests.
- Scope: `List_hasValidStorage_()` and `List_memorySize()` storage validation.
- The shared List validator accepted byte capacities not divisible by `sizeof(void *)`, and `List_memorySize()` bypassed that validator. Malformed backing geometry could therefore appear valid and report a plausible allocation size.
- Require pointer-slot-aligned capacity bytes in the central validator; route memory-size reporting through the same invariant. Extend the existing storage-bounds test to verify reads, sizing, views, and mutations fail closed without changing the source allocation. No new files or CMake targets.
- No local compiler/CTest result is claimed; GitHub Actions remains the build/runtime gate.

---

## 2026-11-07 18:59 CST — Reject free-list metadata writes on invalid file extents

- Baseline: `52d6fdef7b95662d9e116afd228188b9baff83da`; latest completed SkipDBv2 Regression run #631 passed.
- Scope: `UDBRecords_firstEmptyRecordPos_()` in `UDBRecordsFreeList.c`.
- The setter let `pos == 0` bypass file-extent validation. After a stream extent became negative or represented a truncated metadata prefix, it could still enqueue a write over the reserved first-empty metadata word.
- Reject negative extents and nonempty files shorter than `sizeof(PID_TYPE)` before touching cursor state, in-memory free-list state, or the journal. Preserve valid zero-length-file initialization and validate nonzero free-list offsets against the checked extent.
- Extend the existing `udb_record_storage_test` to prove invalid extents leave the journal, logical cursor, and free-list pointer unchanged; strengthen static and dynamic workflow contracts. No new files or CMake changes; no local build is claimed.

---

## 2026-11-07 18:59 CST — Reject misaligned PHash backing storage

- Baseline: `638680f1bd5268a9e24448aadc90d33842707d53`; SkipDBv2 Regression run #632 passed the integration contracts, full build, and all 57 CTests.
- Scope: `PHash_hasValidStorage_()` and its existing storage-bounds regression.
- The central PHash validator checked capacity, mask, and key-count geometry but accepted a misaligned `records` pointer. Valid-looking table metadata could therefore allow inline lookups and mutations to perform undefined unaligned `PHashRecord` accesses.
- Require naturally aligned backing storage before any inline/C operation may proceed, explicitly include `stdint.h`, and extend the existing PHash test to exercise reads, writes, enumeration, sizing, copy, grow, and compaction against a malformed shallow view while confirming the actual source table remains intact.
- Update the existing GitHub Actions source contracts; no new files, exports, or CMake targets. No local build/CTest result is claimed.

---

## 2026-11-07 19:01 CST — Reject PID-map writes on invalid index extents

- Baseline: `83f9a595ed8841cb9e43f93293330f723622e42e`; SkipDBv2 Regression run #632 passed the integration contracts, full build, and all 57 CTests.
- Scope: `UDBIndex_setPos_forPid_()` in `UDBIndexMap.c`.
- The mapping writer validated the PID offset and reservation state but not the current index-file extent. PID 0 / position 0 could therefore journal a write even when the index extent was negative or ended in a partial fixed-width entry, bypassing the startup alignment check.
- Reject negative and misaligned extents before reading mappings or writing to the journal. Preserve the valid empty index initialization path (`maxPos == 0`).
- Extend the existing `udb_index_layers_test` to assert both invalid extents leave journal length and cursor state unchanged; strengthen workflow source-order and regression contracts. No new files or CMake changes; no local build is claimed.

---

## 2026-11-07 19:02 CST — Restore Actions workflow startup

- Failed head: `6a20f802d6008c11dadadab8ca8d5b208087c165`; run #634 failed immediately and GitHub reported zero jobs/check runs, so no step-level log was available.
- Root cause addressed in-place: the workflow had grown to 512783 characters and contained seven duplicate keys in the Python `checks` dictionary. Python keeps only each key's last value, so the earlier blocks were shadowed and added no effective validation.
- Remove only those seven overwritten blocks, retaining every effective source-contract block, including the new PID-map extent and regression assertions. The workflow falls to 505807 characters; no C sources, tests, build targets, or workflow triggers are changed.
- CI #635 passed after the duplicate-contract cleanup, and #636 subsequently passed source contracts, configuration, the full build, and all 57 CTests.

---

## 2026-11-07 19:05 CST — Guard Stack backing release during teardown

- Baseline: `796411d5f2d86e9df3053c18942dc4d9d470c2ef`; SkipDBv2 Regression run #635 passed integration contracts, full build, and all 57 CTests.
- Scope: `Stack_free()` in the Stack lifecycle module.
- All normal Stack operations used the existing storage validator, but destruction unconditionally passed `self->items` to `io_free`. If the backing pointer was corrupted or misaligned, teardown could hand an invalid pointer to the allocator.
- Validate the existing storage geometry before releasing the backing allocation; always release the Stack object itself. Extend the existing lifecycle test with a deliberately misaligned backing pointer and safely free the saved true allocation after teardown.
- Update existing source-contract assertions only; no new files, declarations, or build targets. No local build/CTest result is claimed.

---

## 2026-11-07 19:07 CST — Initialize the journal fixture cleanup path

- Baseline: `a6b424eca9ad97f4584a06ef14f559685ae0b4f4`; SkipDBv2 Regression run #636 passed the source contracts, full build, and all 57 CTests.
- Scope: failure cleanup in `tests/jfile_journal.c`.
- The two earliest test guards jump to shared cleanup before `link_path` is populated, but cleanup always reads `link_path[0]` and may pass the uninitialized stack buffer to `unlink()`. A failure in an unrelated fixture could therefore trigger undefined behavior or target an unintended relative path.
- Zero-initialize the path buffer; retain the existing conditional cleanup and add a source-contract needle so the invariant remains enforced. No new files or build changes; no local build is claimed.

---

## 2026-11-07 19:09 CST — Validate UArray backing ownership before teardown

- Baseline: `c596819019796a5b32f4383b8551b214345cbc57`; SkipDBv2 Regression run #637 passed integration/source contracts, full build, and all 57 CTests.
- Scope: `UArray_stackFree()` and `UArray_free()` typed backing-storage release.
- UArray already uses `UArray_hasValidItemLayout_()` to validate type width, size arithmetic, and data alignment before typed access, but both destructors bypassed it and unconditionally released any non-stack-allocated data pointer. A corrupted or misaligned typed pointer could therefore reach the allocator.
- Reuse the existing layout guard in both teardown paths. Extend the existing UArray lifecycle test to exercise malformed shallow-view cleanup and heap-object destruction while preserving and explicitly freeing the real owned allocation.
- Update the existing workflow source contracts; no new files, exported declarations, or build targets. No local build/CTest result is claimed.

---

## 2026-11-07 19:10 CST — Repair UArray layout source-count contract

- Failing head: `e6d1a4eb84c657db4d135456614f5bc0b9c5863c`; GitHub Actions run #638 reached the integration-contract checker and failed before configure/build.
- Root cause: the UArray teardown hardening correctly added one file-scope forward declaration and two guarded destructor call sites, but the strict workflow still required exactly 13 textual occurrences of `UArray_hasValidItemLayout_()`. The current file contains 16 (one declaration, one definition, fourteen calls).
- Update only the stale source-count assertion/message; no product code, test behavior, build target, or trigger changed. Re-run full CI as the validation gate.

---

## 2026-11-07 19:11 CST — Locate UArray layout-validator definition precisely

- Failed head: `8989a3bb30a48f46d5dfa164ba9f935b81156cf5`; Actions run #639 failed in the source-contract checker before compilation.
- The preceding count repair passed the dictionary contracts, but a dynamic source-order contract used an unqualified `find()` for the validator signature. The new forward declaration matched before the actual function body, causing the validator-body geometry check to inspect the wrong span.
- Anchor the dynamic check to the signature followed by its opening brace, so it selects the definition rather than the forward declaration. This is a workflow-only correction; no C logic, tests, build targets, or workflow triggers change.

---

## 2026-11-07 19:12 CST — Correct newline escape in UArray source contract

- Failed head: `c0a947efbb97e3ebd221d4bd164eff92ab804c2d`; Actions run #640 still stopped in the source-contract checker before compilation.
- The intended definition-only search had a double-escaped newline in the Python script, so it searched for a literal backslash sequence instead of the actual newline before the opening brace. The guard consequently still found no validator body.
- Correct the Python string to use one newline escape; preserve the existing 16-occurrence contract and all implementation files.

---

## 2026-11-07 19:14 CST — Keep next-PID preview ahead of active reservations

- Baseline: `c7066d819c5ff66873d11663229438d233ba403e`; SkipDBv2 Regression run #641 passed source contracts, configuration, full build, and all 57 CTests.
- Scope: `UDBIndex_nextPid()` in `UDBIndexFreeList.c`.
- During an active transaction, the allocator tracks newly reserved IDs in `self->maxPid`, while index mappings remain journaled and file EOF does not advance. The preview path used only EOF, so it could return a PID already reserved by an earlier allocation in that same transaction.
- Mirror the allocator's `maxPid`-aware fallback with overflow and offset checks, preserving free-hole priority. Extend `udb_index_layers.c` to check preview results after reserving PID 1 and then PID 2; strengthen existing Actions source and ordering contracts.
- Existing source/test/workflow files only; no CMake changes or new declarations. No local build/CTest result is claimed.

---

## 2026-11-07 19:16 CST — Fix PID-preview workflow contract indentation

- Failing head: `e0c680468fb4623067844b069ef469842b9c4633`; Actions run #642 stopped before compilation with a Python `IndentationError` in the newly added PID-preview source-order check.
- The contract body was inserted one space deeper than the surrounding workflow script, so Python rejected the checker before any build/test stage.
- Normalize the inserted block to the surrounding ten-space level; leave the C implementation and regression unchanged. Re-run Actions to validate the contract and build.

---

## 2026-11-07 19:19 CST — Guard BStream read aliases on malformed metadata

- Baseline: `8181b9ded7a74a45ba70726a611981db68d3c0b4`; SkipDBv2 Regression run #643 passed integration/source contracts, full build, and all 57 CTests.
- Scope: `BStream_sourceOverlapsStream_()` and `BStream_readNumber_size_()`.
- The overlap guard ran only after checking that the stream item width was one byte. If `itemSize` was corrupted, the invalid-stream path could zero the caller's output buffer even when it pointed inside the stream's own allocation, corrupting input bytes.
- Check known/possible aliases before any error path writes to the destination; calculate byte extents with overflow guards and conservatively reject output writes when malformed metadata leaves no reliable extent. Extend the existing regression for item widths 2 and 0, checking the cursor and backing bytes stay unchanged.
- Update only existing C, test, workflow, and handoff-log files. No new files or CMake targets; no local build/CTest result is claimed.

---

## 2026-11-07 19:21 CST — Update BStream read-alias CI contract

- Failed head: `79e4fdeb3aca3ad5c209d1efc80f0b8f69f091da`; Actions run #644 reached source-contract validation and failed before configure/build.
- The runtime source check searched for the old one-line alias guard. The refactor moved the alias check ahead of malformed-metadata handling and added conservative extent guards, so that exact expression no longer exists.
- Retarget the checker to the new alias-guard comment, preserving the existing ordering and no-zeroing assertions. Only the workflow contract and handoff log are changed; no source or regression behavior is altered in this CI repair.

---

## 2026-11-07 19:25 CST — Recover RandomGen from an impossible zero index

- Baseline: `c6e4948bc4187189f4d417db1b006ac5409cd6d3`; latest main CI run #645 passed the integration/source contracts, full build, and all 57 CTests.
- Scope: MT19937 state validation in `RandomGen.c`.
- The generator treated `mti == 0` as acceptable whenever any MT word was nonzero. But the index is zero only transiently after the internal twist, and the same call consumes that word before returning. A corrupted object entering the next call at index zero skipped the twist and emitted a state word out of sequence.
- Reseed on every call-entry `mti == 0`, retain the existing all-zero-state recovery at the normal block boundary, and add a deterministic regression comparing the recovery output with a fresh generator seeded to the documented fallback seed.
- Existing files only; update source-contract checks. No local build/CTest result is claimed.

---

## 2026-11-07 19:32 CST — Reject misaligned SHash backing storage

- Baseline: `def32970755e938fa0c119eb2c69639dca95e3e7`; latest main CI run #646 passed the source/integration contracts, complete build, and all 57 CTests.
- Scope: the shared `SHash_hasValidStorage_()` predicate and existing `shash_compact_test` regression.
- The SHash validator checked table dimensions, mask, and key-count bounds but accepted an unaligned `records` pointer. A malformed shallow view could therefore pass every inline guard and access `SHashRecord` slots at an invalid alignment.
- Include the fixed-width pointer-integer declaration and reject backing pointers that are not aligned to pointer slots before any indexed access. Extend the existing compaction test with a misaligned shallow view, covering reads, sizing, clone/copy, enumeration, and mutation fail-closed behavior while confirming the true allocation remains unchanged.
- Update existing source and dynamic Actions contracts; reuse the existing CMake target, with no new files or build targets. No local compiler/CTest result is claimed.

---

## 2026-11-07 19:35 CST — Validate SHash backing storage before teardown

- Baseline: `7ab39ac305831b584354ef380914b3584ffb91f3`; main CI run #647 passed all source/integration contracts, the full build, and all 57 CTests.
- Scope: `SHash_free()` and its existing compaction/lifecycle regression target.
- The table operations now reject a misaligned backing pointer through `SHash_hasValidStorage_()`, but destruction bypassed that shared invariant and unconditionally called `io_free()` on the pointer. A corrupted table could therefore turn cleanup into an invalid allocator release.
- Gate backing release on the existing storage validator while always freeing the SHash object itself. Add a regression that corrupts only the backing pointer alignment, verifies teardown completes, and then explicitly releases the saved genuine allocation.
- Extend existing source and dynamic workflow contracts. No new files, declarations, or CMake targets; no local compiler/CTest result is claimed.

---

## 2026-11-07 19:36 CST — Repair SHash teardown CI contract indentation

- Baseline: `12c6e9493175c90acfb928bbf7fc3172d445bb9e`; run #648 stopped in the Python-based firmware/source-contract step with an `IndentationError` before configuration, compilation, or CTests.
- Cause: the newly added `SHash_free()` source-contract block left the following compaction-contract statement over-indented. Correct that single line to the surrounding Python block indentation; no production C or test behavior changed.
- Re-run the main workflow to verify the strict source contract and all build/test stages.

---

## 2026-11-07 19:38 CST — Validate PHash backing storage before teardown

- Baseline: `aa6094a63f407a3202e93fbe4e741b5cd82df1d4`; main CI run #649 passed after the SHash teardown-contract repair.
- Scope: `PHash_free()` and the existing `phash_compact_test` lifecycle coverage.
- PHash already rejects misaligned backing storage in normal operations, but its destructor bypassed the validator and unconditionally freed `records`. Corrupted alignment metadata could still reach the allocator during teardown.
- Guard backing release with `PHash_hasValidStorage_()`, always release the PHash object, and add a teardown regression that retains and frees the genuine allocation separately after presenting a misaligned alias.
- Update existing static and dynamic Actions contracts; reuse the current CTest target without adding files or targets. No local compiler/CTest result is claimed.

---

## 2026-11-07 19:38 CST — Repair PHash teardown CI contract indentation

- Baseline: `11f9262d847e577da81387376a632b5e8f5284f5`; run #650 failed immediately after adding the PHash teardown contract.
- The dynamic workflow source-contract block inherited eight extra spaces on the following `copy_start` assignment, producing another Python `IndentationError`. Align that one statement with the surrounding block; no production C or regression behavior changed.
- Re-run Actions before resuming the next core-module pass.

---

## 2026-11-07 19:41 CST — Simplify PHash teardown workflow contract

- Baseline: `00e572c6f47e5717ce71669ece8546a05e659394`; run #651 terminated with no jobs, logs, or check records after the indentation correction.
- Keep the existing strict source-contract dictionary assertions for `PHash_free()` and the teardown regression, but remove the added dynamic body-order parser. This avoids extending the already-large Python contract script while preserving the explicit source and test needles; the production fix and regression remain unchanged.
- Trigger a fresh main workflow and do not resume refactoring until it completes.

---

## 2026-11-07 19:41 CST — Keep regression workflow under the Actions file-size boundary

- Baseline: `0b64cf993d17adba15e64a2c42d34a779cd055fe`; runs #650–#652 failed before creating any jobs or logs after the workflow contract additions.
- The workflow grew from 511,665 characters on green run #649 to 512,140 after the PHash static-contract update; the latest failed run had zero jobs/check records and reverted to a path-based workflow name. Treat this as a workflow-definition size/instantiation boundary rather than a C build failure.
- Remove the redundant dynamic SHash destructor body-order scan. Keep its static source/test needles, along with the PHash destructor source/test contract and both production regressions. The resulting workflow is 511535 characters, with no changes to C behavior or CMake targets.
- Re-run Actions; do not resume refactoring until the workflow instantiates and reaches the build/test stages.

---

## 2026-11-07 20:05 CST — Bound MainArgs teardown by its owned argv sentinel

- Baseline: `b329b69d538c28e579cfce18ceba9a9b6831b1ca`; latest mainline SkipDBv2 Regression run #653 passed.
- Scope: `MainArgs_freeArguments_()` and the existing `main_args_lifecycle_test`.
- The owned argument vector is zero-allocated with a terminating NULL slot, but cleanup trusted the public, caller-visible `argc`. A negative count leaked all copied strings; an oversized count could walk beyond the vector and free unrelated memory.
- Make the vector's NULL sentinel the sole cleanup boundary, including partial-copy rollback and replacement of an existing value. Extend the lifecycle test to corrupt `argc` to both -1 and `INT_MAX`, and compile it with the tracked allocator to verify all allocations are released.
- Reuse existing source, test, and CMake targets; update the existing Actions source contracts. No new files or exported symbols. No local compiler/CTest result is claimed; remote Actions is the validation gate.

---

## 2026-11-07 20:06 CST — Retarget MainArgs sentinel source contract

- Failed head: `8fd452a201548b0ca1bae935225a39b8b9aa7ab9`; SkipDBv2 Regression run #654 stopped in the XY-C1 source-contract stage before CMake configuration or compilation.
- The new contract expected a `while` loop, but the implementation uses a `for` loop with the same NULL-sentinel termination condition. The C source and regression logic were not at fault.
- Retarget the contract to the actual invariant token `NULL != argv[i]`. This is a workflow-only CI repair; no production code, tests, or build targets change.

---

## 2026-11-07 20:07 CST — Align MainArgs replacement-order contract with sentinel cleanup

- Failed head: `3d147dd2fe54162b5e02b01b3cd7bb84dbe633de`; run #655 passed the static source/path inventory but failed the dynamic MainArgs replacement-order check before CMake.
- The checker still searched for the removed `oldArgc` parameter in the old-vector release call. Cleanup now derives its bound from the owned vector's NULL sentinel, so the new call is `MainArgs_freeArguments_(oldArgv);`.
- Update only the dynamic source-contract needle; retain its allocate → validate → copy → publish → release ordering assertion. No C source, test, or CMake changes.

---

## 2026-11-07 20:14 CST — Complete UArray percent escaping and execute alias regression

- Baseline: `f8ecb05f3a7d8163139e8db2ae39414439b25196`; SkipDBv2 Regression run #656 passed all XY-C1 contracts, CMake configuration, the full target build, and all CTests.
- Scope: `UArray_fromVargs_()` in `basekit/UArray_format.c` and its existing `uarray_format_trailing_percent_test`.
- The formatter already preserves a trailing lone `%`, but a conventional escaped `%%` was discarded instead of emitting one literal percent. Add the explicit escape case.
- The existing format-alias regression had accidentally been placed inside the failure branch of the preceding ordinary-format assertion, so the successful path never executed it. Close that assertion before the alias case, then add a literal-percent regression.
- Reuse the current CTest target; extend the Actions source contract. No new files, headers, exported symbols, or CMake target changes. Validation is delegated to the current remote Actions run.

---

## 2026-11-07 20:15 CST — Repair escaped-percent source-contract quoting

- Failed head: `4764f40d2af8748d41396c0fc84d67e15567e506`; run #657 failed while parsing the Python source-contract script, before any firmware contract check or C build.
- The added test-source needle had two backslashes before its embedded quotes; Python therefore terminated the string literal early. Reduce this to the single escape used by the surrounding contract entries.
- Workflow-only CI syntax repair. The UArray implementation and test changes are unchanged.

---

## 2026-11-07 20:16 CST — Match escaped-percent contract to the actual branch

- Failed head: `84a3053d3282d53f7ae7a78a3371eff101a898dd`; run #658 passed workflow-script parsing, then failed the static source contract before configuration or compilation.
- The formatter correctly introduces the escape handler as `if (*format == '%')`, followed by `else if` for the other directives. The new contract mistakenly expected the escape handler itself to be an `else if`.
- Correct the single source-token assertion. No production source or regression test changes.

---

## 2026-11-07 20:33 CST — Defer DynLib unload until init callbacks return

- Baseline: `b7f2ea43c11738302e5ae91e4a567f56e2ef6b98`; main SkipDBv2 Regression run #659 passed.
- Scope: the `DynLib_open()` / `DynLib_close()` initializer lifecycle.
- An init callback may receive its owning `DynLib` through `initArg` and call `DynLib_close()`. The old path unloaded the shared object while its initializer was still executing, so the callback could return into unmapped code.
- Track the active init callback and defer close requests until it returns; drain the request before `DynLib_open()` returns. Extend the existing reentrant fixture and lifecycle test to verify callback completion and the final closed state.
- Reuse existing source, fixture, CTest target, and workflow contracts. No new files or CMake targets. No local firmware build is claimed; remote Actions is the validation gate.

---

## 2026-11-07 20:34 CST — Keep Date timezone refresh atomic on clock errors

- Baseline: `3a517037eb67ff816d75f7c7ac202f6936486e72`; main SkipDBv2 Regression run #660 passed the source contracts, full target build, and all CTests.
- Scope: `Date_setToLocalTimeZone()`.
- The helper passed `&self->tz` directly to `gettimeofday()` and ignored its return value. A failed system query could therefore leave the receiver with partially updated timezone metadata.
- Read into a local `struct timezone`, reject a failed query, and publish the complete zone only after success. Extend the existing Actions source contract to require the checked call and the final assignment.
- Existing C source and workflow only; no new headers, symbols, tests, or CMake targets. No local build is claimed; remote Actions is the validation gate.

---

## 2026-11-07 20:57 CST — Preflight integer UArray scalar arithmetic before mutation

- Baseline: `cce040bbed5b339af9272ad011d32843c5f13ebf`; main SkipDBv2 Regression run #661 passed.
- Scope: integer destination handling in `UArray_addScalarDouble_()`, `UArray_subtractScalarDouble_()`, and `UArray_multiplyScalarDouble_()`.
- These operations converted each intermediate double directly into an integer element. A later NaN, infinity, or out-of-range result could invoke undefined floating-to-integer conversion after earlier elements had already changed.
- Generalize the existing range predicate and preflight every integer result before writing any element. Preserve floating-array behavior and hash invalidation semantics; add boundary and non-finite regressions to the existing math test and strengthen its existing Actions source contract.
- Existing C source, test, workflow, and handoff log only; no new declarations, files, or CMake targets. No local firmware build is claimed; remote Actions is the validation gate.

---

## 2026-11-07 20:58 CST — Keep UArray_round restricted to floating-point storage

- Baseline: `a53124fbe978c7c8b9e1753219dcc7ea057f185a`; main SkipDBv2 Regression run #662 passed the source contracts, full target build, and all CTests.
- Scope: `UArray_round()`.
- Integer arrays are already integral; routing 64-bit values through `double` can round them outside their representable range before the typed assignment, invoking undefined floating-to-integer conversion.
- Restrict rounding to float32/float64 arrays. Preserve integer storage and its cached hash unchanged, with signed/unsigned 64-bit boundary regressions and a matching Actions source contract.
- Existing C source, existing math test, workflow, and handoff log only; no new files or CMake targets. No local firmware build is claimed; remote Actions is the validation gate.

---

## 2026-11-07 21:00 CST — Make UArray integer absolute value type-safe

- Baseline: `dcde85885d0013b55f6a70b2448187255aa3a46a`; main SkipDBv2 Regression run #663 passed the source contracts, full target build, and all CTests.
- Scope: `UArray_fabs()` and its `UArray_abs()` alias.
- The generic unary-math macro converted every element to double, then converted the absolute result back to its original integer type. This overflows for signed minima and can round large unsigned values to (2^{64}) before an undefined cast.
- Handle float arrays with `fabs`; handle signed integers with native typed negation after the existing signed-minimum preflight; leave unsigned arrays unchanged. Add signed, minimum, and UINT64 boundary regressions plus explicit Actions source contracts.
- Existing C source, math test, workflow, and handoff log only; no new files or CMake targets. Remote Actions remains the build/test gate.

---

## 2026-11-07 21:02 CST — Repair scalar-math Actions contract syntax

- Baseline: `9099b75d6fa0c70ae07c4657d0fdac4194a2b737`; main run #663 passed, while run #664 stopped in the source-contract step before configure/build.
- Cause: the scalar-operation contract's `type_guard` assignment was accidentally placed inside a Python boolean expression, producing a syntax error.
- Move the assignment before the condition and retain the intended type-guard ordering assertion. Production C and regression-test content are unchanged.
- Existing workflow and handoff log only; no new files or build-target changes.

---

## 2026-11-07 21:04 CST — Test integer absolute value through the public UArray_abs API

- Baseline: `ef57cfc46091a08549157134a384ecb9e90f3096`; run #665 passed the source-contract and configure steps but failed compiling the math regression because `UArray_fabs` is not declared in the public `UArray_math.h`.
- Route the new integer regressions through the declared `UArray_abs()` alias. Keep the implementation unchanged and avoid creating an unnecessary public declaration solely for a test.
- Existing math test and handoff log only; no new files, declarations, CMake targets, or workflow changes.

---

## 2026-11-07 21:13 CST — Preflight integer results in UArray unary math transforms

- Baseline: `15b0e36837eb067af3c28dcc06bc7e81cbe60fbb`; main run #666 passed source contracts, configure, full target build, and all CTests.
- Scope: the `UARRAY_DOP(OP)` family in `UArray_math.c`.
- Unary functions such as `sqrt`, `log`, and `exp` can produce NaN, infinity, or values outside an integer receiver's range. The prior macro cast each result directly into the receiver while iterating, allowing undefined conversion and partial mutation. `floor`/`ceil` could also round INT64/UINT64 boundary inputs through an unrepresentable double.
- For integer arrays, preflight every double result with the existing range predicate before the first write. Preserve the existing floating-array path and valid integer truncation behavior. Add regressions for valid integer sqrt, NaN sqrt, overflowing exp, and signed/unsigned 64-bit floor/ceil boundaries; extend the existing Actions source contract.
- Existing C source, existing math test, workflow, and handoff log only; no new files, API declarations, or CMake targets. Remote Actions is the build/test gate.

---

## 2026-11-07 21:16 CST — Make unary-math source-contract delimiter explicit

- Baseline: `a5160eb3fdbf7e1e672040c8bf2053456e6977a9`; run #667 failed before the connector exposed any job records or logs.
- Review found the new source contract searched for a double-escaped backslash-n sequence rather than the actual newline preceding `UARRAY_DOP(sin);`. Use `chr(10)` to delimit the macro body unambiguously.
- Workflow and handoff log only; the production C and regression-test changes are unchanged.

---

## 2026-11-07 21:18 CST — Keep UArray source contracts below the workflow payload limit

- Baseline: `2db6429c209f9de1e3075f08808f16a8f6654997`; runs #667 and #668 terminated before the connector exposed job records or logs.
- The new unary-math assertions expanded an already very large workflow script. Compress repeated Python source-contract boilerplate while retaining scalar preflight ordering, assignment storage/hash checks, the float-only round guard, unary result preflight, and regression-call coverage.
- Workflow and handoff log only; C implementation and tests remain unchanged.

---

## 2026-11-07 21:53 CST — Add repository-wide AI development governance and cross-chat handoff

- Baseline main HEAD: 412eebb4d0c36f9cbaf43921b4ce1f2056665984.
- Added AI_DEVELOPMENT_PROTOCOL.md as the durable repository-wide development contract.
- Added AI_BOOTSTRAP_PROMPT.md as the short reusable prompt for new ChatGPT conversations.
- Added REFACTOR_ROADMAP.md for whole-repository module inventory and risk-based governance.
- Added SESSION_HANDOFF.md for resumable task state across conversations.
- Preserved the existing dev_log.md history and prepended this entry.
- Documentation-only change: no production C source, firmware build configuration, or tests were modified or executed in this commit.
- Baseline CI reference: SkipDBv2 Regression run 42376349424 on 412eebb4d0c36f9cbaf43921b4ce1f2056665984, success. This is not MIPS firmware-build or hardware-validation evidence.
- Next: inventory the complete source tree and verify historical audit pointers; prioritize a documented HTTPD/HTTPS/CGI/authentication review.

---

## 2026-11-07 21:54 CST — Finalize bootstrap commit reference in handoff

- Parent: 84e2fb77973393dd2804b9feb2d94b2ce74933c4; verified this commit is present on main before updating the handoff metadata.
- Recorded the exact governance bootstrap commit SHA and marked the bootstrap task complete in SESSION_HANDOFF.md.
- Linked the bootstrap commit in REFACTOR_ROADMAP.md.
- Documentation-only follow-up; no production source or tests changed. The last verified regression remains run 42376349424 on the pre-bootstrap parent SHA; no new code test is claimed.

---

## 2026-11-07 22:14 CST — Separate HTTPD private-key disclosure from HTTPS transport trust

- Baseline main HEAD: `8152ec72c39d4a435114806a5e861e2aed925248`.
- Audited the current HTTPD authentication helper and the `web_ex.c::dump_file()` key-file gate. `get_login_safe()` intentionally treated HTTPS as safe for several remote-admin operations, but that transport-level override was also authorizing disclosure of files explicitly described as write-only secret keys.
- Added a dedicated private-key access predicate requiring an active login and the trusted LAN/VPN network flag. The `.key` masking path now uses this predicate; the existing HTTPS behavior for unrelated management operations is preserved.
- Added the standalone C policy regression and a new HTTPD Security Regression workflow with an additional source-wiring contract.
- Production code commit: `1ef811e5cb37613fc5ae0caf59041ecdf294e5d2`.
- GitHub Actions run `42380444876` passed on that exact SHA: C regression compile/run and HTTPD getter/route wiring checks succeeded.
- This round did not run a full HTTPD MIPS build, XY-C1 firmware image build, or hardware validation. Broader HTTPD/CGI/authentication and whole-userland reviews remain open.

---

## 2026-11-07 22:30 CST — Require authentication for the Shadowsocks database export

- Baseline main HEAD: `71ff09d1e01d080b6b3b79e1ececa49ef3589bf5`.
- Audited the outstanding HTTPD MIME authorization exception `dbconf` under `APP_SHADOWSOCKS`. The handler `do_dbconf()` accepted a caller-supplied list of database prefixes and serialized all matching SkipDB keys and values into executable JavaScript. The shipped `Shadowsocks.asp` page consumes `/dbconf?p=ss`; its `ssconf_basic_json_*` records populate server-password and SOCKS server-password fields. The route was therefore an unauthenticated credential-bearing configuration export.
- Fixed the `dbconf` MIME route from `need_auth=0` to `need_auth=1`. This reuses the existing `handle_request()` Basic-auth / same-IP session gate before the `do_dbconf()` output callback; no frontend request or data format was changed.
- Extended `.github/workflows/httpd-security.yml` to assert that exactly one `dbconf` route exists, it is authenticated, and the request authorization gate is before response dispatch. The existing private-key policy C regression and wiring checks remain enabled.
- Production change commit: `7fc9b5d7fee3f5dcb15310456e657e567714d75b` (parent: `8756f91b4bda529e11c3f39f1a71cdb540a3f816`; intended source baseline: `71ff09d1e01d080b6b3b79e1ececa49ef3589bf5`).
- Recovery disclosure: an intermediate commit `8756f91b4bda529e11c3f39f1a71cdb540a3f816` was accidentally created using a stale Git tree. Its HTTPD Actions run `42381654753` failed at the private-key policy test compile because repository paths were missing. A fast-forward recovery commit restored the actual baseline tree plus only the two intended changes; GitHub compare from `71ff...` to `7fc9b5d7fee3f5dcb15310456e657e567714d75b` confirmed exactly `web_ex.c` and the HTTPD workflow differ. The intermediate commit is not a valid source checkpoint.
- Validation: HTTPD Authentication and Security Regression run `42381700507` passed on exact head `7fc9b5d7fee3f5dcb15310456e657e567714d75b`; private-key C regression and new auth/route source contract passed. SkipDBv2 Regression run `42381700499` also passed on the same SHA, including firmware integration contracts, configuration, SkipDB target builds, and regression tests. Do not interpret that workflow as a full firmware image build.
- Not run / not verified: local HTTPD compile, MIPS HTTPD cross-compile, complete XY-C1 TRX firmware build and artifact inspection, protocol fuzzing, and hardware validation.
- Remaining HTTPD risks: legacy `b64_decode()` ignores non-alphabet bytes and does not enforce canonical Basic-auth padding; login ownership remains a global client-IP keyed session with same-IP bypass; other route exceptions, TLS post-handshake I/O, and upload restore bounds remain open.
- Next: continue the HTTPD authentication parser/session audit, then broaden into privileged command/configuration, firewall/NAT, DNS/DHCP, WAN/PPP/IPv6, wireless, persistence, file services, storage, and init/service lifecycle.

---

## 2026-11-07 22:52 CST — Strictly validate HTTPD Basic authentication credentials

- Baseline main HEAD: `bf3aa06428749a33abefcb4a012a834460c65786`; production implementation commit: `b905c4baabd87922075700397c54cdbe6edd03fd`; follow-up CI correction: `3989c878b702595fdba530d79da1b1e327225c1c`; handoff/roadmap checkpoint commit: `bd0f4abd600d0e407bb9747d6ba006d40ed461ba`.
- Scope: `trunk/user/httpd/src/base64.c`, `httpd.c`, new `httpd_basic_auth.h`, new `tests/httpd_basic_auth.c`, and `.github/workflows/httpd-security.yml`. Existing `base64.o` production build registration is reused; no unrelated subsystem or SkipDB source changed.
- Finding (P1): `auth_check()` relied on `b64_decode()`, whose documented behavior ignores non-alphabet bytes and whose loop does not enforce complete quartets, legal padding placement, or canonical unused bits. The caller also compared a decoded C string without rejecting embedded NUL bytes. Malformed Basic credentials could be silently normalized before comparison.
- Fix: add a separate two-pass strict RFC 4648 decoder, explicit signed decode-table storage, bounded HTTP Basic Authorization parsing, case-insensitive Basic scheme matching, token/decoded-size caps, and rejection of embedded NUL or missing user/password separator. Keep the legacy decoder unchanged for other call sites. Replace the HTTPD Basic auth call site with the strict path.
- Regression: add GNU89 C coverage for canonical padded/unpadded input, invalid alphabet/length/padding, non-zero pad bits, output capacity and no partial write, scheme case and OWS/CRLF, malformed scheme spacing, embedded NUL, and missing colon. Extend the existing HTTPD Actions workflow to compile/run the new test and verify production wiring.
- CI recovery disclosure: initial HTTPD run `42383463695` on `b905c4baabd87922075700397c54cdbe6edd03fd` failed at the shell compile step because the inserted continuation lines contained doubled backslashes; the linker attempted to open a literal backslash file. Workflow-only commit `3989c878b702595fdba530d79da1b1e327225c1c` corrected the four shell continuation lines. Final HTTPD Authentication and Security Regression run `42383489924` passed on that exact corrected head. Both GNU89 C executables compiled and ran; source-wiring assertions for private-key policy, dbconf auth gate and strict Basic parser passed.
- Validation: L1 source, header/API, allocation-bound and existing Makefile linkage review completed for this narrow path. L2 focused host regression PASS via run `42383489924`. No separate local compile/CTest is claimed. L3 MIPS HTTPD/XY-C1 firmware build and image inspection NOT RUN; L4 hardware validation NOT RUN; fuzzing NOT RUN.
- Whole-repository status: HTTPD remains AUDITING, CGI/templates remain NOT AUDITED, and privileged management, firewall/NAT, resolver/DHCP, WAN/PPP/IPv6, wireless, NVRAM, sharing/storage, and init/service lifecycle audits remain open. Five stale remote branches were compared; outstanding deltas were SkipDB-only or empty/behind and were not merged.
- Next: resume `httpd.c` login/session lifecycle (`http_login_check()`, `http_login()`, `http_logout()`, global IP ownership/timeout and logout path), then produce and audit the complete conditional `mime_handlers[]`/CGI authorization matrix. Continue into command/config privilege boundaries and other userland services; do not restart routine SkipDBv2 refactoring.

---

## 2026-11-07 23:12 CST — Correct HTTPD management-session inactivity timeout

- Baseline main HEAD: aea93c782f8a1064a91d0d81a5c8d3341286a44a. One bounded module: HTTPD login-owner inactivity/timeout state.
- Paths changed: trunk/user/httpd/src/httpd.c, new httpd_login_policy.h, new tests/httpd_login_policy.c, .github/workflows/httpd-security.yml, REFACTOR_ROADMAP.md, SESSION_HANDOFF.md, and this log.
- Finding: the owner-IP branch in http_login_check() returned before timeout evaluation. login_timestamp was set at initial login only and not updated after successful subsequent protected requests, so another IP could clear the supposed 30-second timeout lock while the original management client remained active.
- Fix: separate login_last_activity from the initial-login login_timestamp; initialize both at successful login, reset both at logout/expiry, check timeout against last activity, and refresh only after a protected request passes Basic authentication. This preserves the existing NVRAM/front-end timestamp meaning and existing auth requirements.
- Regression: added a private helper test for no elapsed time, just-before/at/after the 30-second boundary, and unsigned uptime wraparound. The HTTPD Actions workflow now compiles/runs it and checks that timeout and refresh are wired to the production paths.
- Validation: GitHub Actions run 42385373380 passed on exact code SHA cfd8da8ffa51f29795920f90b68777630a9c81bc (https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42385373380). All three GNU89 -Wall -Wextra -Werror C regressions (private-key policy, strict Basic auth, and session timeout policy) compiled and ran; production source-wiring checks passed. A local equivalent build/run of the new session-helper regression also passed.
- Implementation commit: cfd8da8ffa51f29795920f90b68777630a9c81bc (parent aea93c782f8a1064a91d0d81a5c8d3341286a44a). L1 PASS and L2 focused host regression PASS. Full MIPS HTTPD/XY-C1 build, firmware artifact validation, fuzzing and hardware validation remain NOT VERIFIED. Next: audit the conditional HTTPD MIME/CGI route authorization matrix.

---

## 2026-11-07 23:14 CST — Verify HTTPD idle-timeout regression and finalize checkpoint

- Baseline main HEAD before this documentation-only follow-up: cfd8da8ffa51f29795920f90b68777630a9c81bc (implementation commit; parent aea93c782f8a1064a91d0d81a5c8d3341286a44a).
- GitHub Actions HTTPD Authentication and Security Regression run 42385373380 completed SUCCESS against exact head SHA cfd8da8ffa51f29795920f90b68777630a9c81bc: https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42385373380
- The job compiled and ran the private-key policy, strict Basic-auth, and session-timeout policy regression executables with GNU89 -Wall -Wextra -Werror, then passed source wiring/auth-route assertions. A separate local build/run of the new session helper regression also passed.
- Updated SESSION_HANDOFF.md and REFACTOR_ROADMAP.md to reflect the exact CI evidence and remaining validation boundary; updated the prior code entry in this log with implementation SHA and run result. This follow-up changes documentation only; no new tests or firmware build were run.
- Validation boundary: L1 PASS, L2 focused host regression PASS. L3 MIPS/firmware image and L4 hardware validation NOT VERIFIED. Next: systematically audit the conditional HTTPD MIME/CGI authorization matrix, then broaden the repository-wide audit.

---

## 2026-11-07 23:16 CST — Add HTTPD MIME authorization matrix regression

- Baseline main HEAD: 8fd0a842b9599876b753c2b5b6deda4bd4feecf7. Bounded P1 continuation of the unfinished HTTPD route/auth audit.
- Source reviewed: `trunk/user/httpd/src/web_ex.c::mime_handlers[]` and `trunk/user/httpd/src/httpd.c::handle_request()`. The request path checks `need_auth` before invoking input/output callbacks; the prior `dbconf` exception is now auth-gated. Existing localhost bypass and public asset exceptions are preserved as compatibility contracts.
- Change: extend `.github/workflows/httpd-security.yml` with a regex-based source table parser that verifies callback plus `need_auth` for explicit cached public assets and all known dynamic pages, dynamic-JS fallback, config/storage/log exports, feature-gated exports/APIs, apply/update CGI, firmware upgrade and both restore routes. Also assert that public cached JavaScript routes precede the protected `**.js` fallback. The contract parses feature-gated entries independent of the active firmware configuration.
- Initial run `42385599174` failed on exact SHA `4cef7d95812786dad0d0ac34dce461b0a1c0bb64` because the Python regex did not parse any route rows. Corrective commit `015b820fc61cb8e3cc1283a0a87a48751205eae6` fixed the backslash count. Actions run `42385661621` passed on that exact SHA: C tests and the source-wiring step both succeeded. This verifies the listed route declarations and existing auth-gate order, not handler internals. No target build or firmware/hardware validation is claimed.
- Next: verify this exact run; then audit `do_upgrade_fw_cgi`, `do_restore_nv_cgi`, `do_restore_st_cgi`, `do_apply_cgi`, `do_update_cgi` and sensitive file exports.

---

## 2026-11-07 23:17 CST — Fix over-escaped HTTPD route-matrix regex

- Parent main SHA: 4cef7d95812786dad0d0ac34dce461b0a1c0bb64. Route-matrix test commit under validation: `4cef7d95812786dad0d0ac34dce461b0a1c0bb64`; Actions run `42385599174` (`https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42385599174`) was still in progress during this correction.
- Source review caught that the Python raw-string regex had two backslashes where one was required. The initial Actions run `42385599174` failed its source-wiring step on exact head `4cef7d95812786dad0d0ac34dce461b0a1c0bb64` with `AssertionError: failed to parse the HTTPD MIME route table`. The pattern was corrected to match C initializer lines such as `{ "jquery.js", ..., do_file, 0 },`; local Python checks against representative public, page, download and `dbconf` entries passed.
- Correction commit: `015b820fc61cb8e3cc1283a0a87a48751205eae6`. Its Actions run `42385661621` at `https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42385661621` was still in progress at the last check. No production C source was modified. The corrected run must pass before marking the route matrix contract verified.
- Validation: the full GitHub workflow result for this correction is PENDING at commit creation. L3 MIPS build, complete firmware artifact and L4 hardware remain NOT VERIFIED.

---

## 2026-11-07 23:18 CST — Record HTTPD route-matrix CI failure and recovery state

- Verified first matrix workflow run `42385599174` against exact code SHA `4cef7d95812786dad0d0ac34dce461b0a1c0bb64`: the C regression step passed; the source-wiring step failed with `AssertionError: failed to parse the HTTPD MIME route table` because the new Python regex was over-escaped.
- Corrective workflow commit `015b820fc61cb8e3cc1283a0a87a48751205eae6` is on main. Follow-up Actions run `42385661621` is tied to that exact SHA and was IN_PROGRESS at the last status check. Handoff and roadmap now record both attempts and their distinct SHAs.
- This documentation-only follow-up does not change code or claim the corrected route-matrix run has passed. Next: verify run `42385661621`; inspect its exact job steps/logs if it fails.

---

## 2026-11-07 23:19 CST — Verify HTTPD MIME authorization matrix contract

- Corrective workflow commit `015b820fc61cb8e3cc1283a0a87a48751205eae6` is on main. HTTPD Authentication and Security Regression run `42385661621` completed SUCCESS on exact head SHA `015b820fc61cb8e3cc1283a0a87a48751205eae6`: https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42385661621.
- All three GNU89 `-Wall -Wextra -Werror` policy/basic-auth/session helper regressions compiled and ran. Source contract confirmed the route matrix and auth gate ordering. Previous failed run `42385599174` on the earlier matrix commit is retained and explained as the over-escaped test regex; it did not expose a production C failure.
- Updated the durable handoff and roadmap with the verified result and the explicit test boundary. HTTPD/CGI remain AUDITING; MIPS firmware build, artifact validation, fuzzing and hardware testing remain NOT VERIFIED.
- Next bounded P1 task: audit `upload.c` and firmware/NVRAM/storage restore callers for multipart framing, length bounds, filesystem/cleanup behavior, extraction limits and rollback. Do not resume routine SkipDBv2 work.

---

## 2026-11-07 23:20 CST — Clear stale upload artifacts before parsing restore requests

- Baseline main HEAD: 87ce7c634721c2be7a7b3f52b970ad4f6f991892. Bounded P1 audit of `trunk/user/httpd/src/upload.c` and restore callbacks.
- Finding: `handle_request()` invokes a route's output callback even when its POST input callback fails. `do_upload_file()` used to unlink its destination only after finding the expected multipart file part and skipping its part headers. If a previous request or interrupted operation left an upload artifact, malformed input that failed before that unlink could leave it present for `do_restore_nv_cgi()` or `do_restore_st_cgi()` to pass to privileged restore code. `do_restore_st_post()` also returned on invalid/missing/oversized boundary without clearing an old storage archive.
- Fix: `do_upload_file()` now clears the fixed destination before argument/multipart validation; `do_restore_st_post()` clears its destination before any early boundary rejection. Failure cleanup and existing image/header checks are preserved.
- Regression contract: workflow now checks the shared uploader clears its destination before multipart parsing and the storage path clears the archive before boundary validation. This is source-wiring coverage, not runtime multipart fuzzing or proof of archive extraction safety.
- Validation: HTTPD Authentication and Security Regression run `42385963510` passed on exact head SHA `5877544ea347ab728bd185012187ec09e2aeb76c`; compilation and all source-wiring assertions, including stale-artifact cleanup ordering, passed. L3 full MIPS/firmware artifact and L4 hardware validation remain NOT VERIFIED.
- Next: inspect the matching workflow result, then audit size arithmetic and `/sbin/mtd_storage.sh` restore/extraction behavior.

---

## 2026-11-07 23:22 CST — Verify stale upload-artifact cleanup regression

- Production code commit `5877544ea347ab728bd185012187ec09e2aeb76c` passed HTTPD Authentication and Security Regression run `42385963510` on exact SHA: https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42385963510.
- The job compiled/ran the existing strict Basic-auth, private-key policy and idle-timeout tests with GNU89 `-Wall -Wextra -Werror`, then passed source contract checks for the MIME auth matrix and the new upload cleanup ordering. The shared uploader's destination clearing precedes multipart parsing; the storage restore path clears its target before boundary rejection.
- L1 source review and L2 focused host CI checks PASS for this bounded fix. The test does not claim full multipart fuzzing, tar extraction validation, MIPS build, firmware artifact inspection or hardware testing; L3/L4 remain NOT VERIFIED.
- Next P1 task: inspect image/NVRAM length arithmetic and the storage restore command/extraction implementation. Then continue privileged CGI/configuration dispatch review.

---

## 2026-11-07 23:24 CST — Block symlink-parent traversal during storage archive extraction

- Baseline main HEAD: a6f284111241c28ba1577d12264d3288338e53eb. Scope: BusyBox archive extraction called by `/sbin/mtd_storage.sh` during root storage restore.
- Finding: `func_restore()` executes `tar xjf $tbz -C /tmp/storage`, checks for `start_script.sh`, then copies the extracted tree into `/etc`. BusyBox `get_header_tar.c` strips unsafe member-name prefixes, but `data_extract_all()` does not reject symlink ancestors created by prior archive entries. A tar member `escape -> /etc` followed by `escape/passwd` can cause `ARCHIVE_UNLINK_OLD` plus file creation to target `/etc/passwd` rather than stay under the scratch tree.
- Fix: new `archival/libarchive/safe_extract_path.h` rejects absolute names, `..` components and existing non-directory/symlink parents using `lstat()`. `data_extract_all()` calls the policy before directory creation, unlink and open. Missing parent directories remain allowed. Added standalone host regression `tests/safe_extract_path.c`; it tests normal and missing parents, file/symlink parents, absolute/parent traversal and invalid inputs.
- CI: updated `.github/workflows/httpd-security.yml` path triggers, compiles/runs the policy test with GNU89 `-Wall -Wextra -Werror`, and asserts the production guard occurs before extraction side effects and storage restore fails closed when tar fails. Validation PENDING for the exact commit; do not claim the new test passed until its run finishes.
- Scope limits: host regression/source-contract only planned; no full BusyBox MIPS build, firmware artifact, archive fuzzing or hardware validation. Valid symlink usage and archive restore compatibility need further review.
- Next: verify exact Actions run, then audit archive/link semantics, NVRAM/image header length handling, privileged CGI dispatch, and broaden the whole-userland P1 inventory.

---

## 2026-11-07 23:25 CST — Also protect directory-entry metadata from symlink leaves

- Parent commit `71bfeb3093cd3cdfc6c60352bd011814661dcdf8` passed HTTPD Authentication and Security Regression run `42386491809`, including the initial parent-path policy test. During final source review, found an adjacent case not covered by that initial test: a directory member whose leaf already names a symlink can skip `ARCHIVE_UNLINK_OLD` removal and reach `chown`/`chmod` through the symlink.
- Strengthened the shared policy to `bb_archive_path_has_unsafe_component(path, check_leaf)`. All entries check parents; directory entries additionally validate the final target, rejecting an existing symlink or non-directory leaf while allowing existing real directories and missing directory paths. Production call remains before mkdir/unlink/open. Expanded unit cases for directory-target symlink/file cases and safe/missing directories.
- Local host check: `cc -std=gnu89 -Wall -Wextra -Werror` compiled the expanded `safe_extract_path.c` test, and the executable passed (`PASS: BusyBox archive path policy (parents and directory leaf)`). GitHub Actions run `42386675924` also passed on exact SHA `a0e700bb6e61fbb7c11623821563449ed3d0ffa9`; both C regression and source-wiring steps succeeded.
- No target build or full archive fuzzing claimed. L3 MIPS/firmware artifact and L4 hardware remain NOT VERIFIED.

---

## 2026-11-07 23:31 CST — Verify expanded archive-path policy regression

- Follow-up code commit `a0e700bb6e61fbb7c11623821563449ed3d0ffa9` passed HTTPD Authentication and Security Regression run `42386675924` against the exact SHA: https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42386675924.
- Both CI steps passed: GNU89 `-Wall -Wextra -Werror` compiled and ran the HTTPD private-key, Basic-auth and login-timeout tests plus the new archive-path test; source-wiring verified the safety call precedes directory creation/unlink/open and the storage restore checks tar failure before acceptance/copy.
- The new regression covers symlink and regular-file parents, `..`, absolute/invalid input, and a symlink or regular file at the leaf when the archive entry is a directory. Existing real directories and missing directories remain accepted.
- L1 source review and L2 focused host CI PASS for these policy changes. Full BusyBox/XY-C1 MIPS build, firmware artifact inspection, archive fuzzing, and hardware validation remain NOT VERIFIED. HTTPD/archive restore remains AUDITING.
- Next: review valid archive compatibility and restore/link limits, then continue privileged CGI command/configuration dispatch and the broader userland security ledger.

---

## 2026-11-07 23:32 CST — Sync whole-repository risk ledger with archive-policy CI result

- Updated the P1 table row and top status in `REFACTOR_ROADMAP.md` to include BusyBox `data_extract_all.c`, the new path policy, and the storage restore script, with exact successful host CI runs `42385963510` and `42386675924`.
- This is a documentation-only consistency commit. HTTPD/archive restore remains AUDITING; the full archive and CGI handlers have not been audited, and L3 MIPS/firmware artifact plus L4 hardware validation remain NOT VERIFIED.

---

## 2026-11-07 23:36 CST — Reject unknown service IDs in HTTPD CGI POST handlers

- Baseline main HEAD: 5fa70fcb0547851704001274ec1ba6792cc4d83c. Scope: `trunk/user/httpd/src/web_ex.c::{update_variables_ex,apply_cgi}` and existing `common.c::LookupServiceId()`.
- Finding: POST-controlled `sid_list` parsing used an open-ended `GetServiceId(sid)` loop in both handlers. Unknown IDs left `sid` at the service-table NULL sentinel and still reached `GetVariables(sid)`, `validate_asp_apply()` or `validate_cgi()`, whose loops dereference `v->name`. This could terminate HTTPD on a malformed authenticated management request; this is not described as unauthenticated remote code execution.
- Fix: both paths now call the existing safe `LookupServiceId(serviceId)`. Negative results advance the list and continue before validators, variable lookups or group mutation. Known IDs follow the old path unchanged.
- Regression: new `trunk/user/httpd/src/tests/httpd_service_id.c` links production `common.c` and tests known IDs, unknown/empty IDs and sentinel behavior. Workflow path triggers, GNU89 `-Wall -Wextra -Werror` compilation/run and Python source-contract assertions were added for both production call sites.
- Validation: HTTPD Authentication and Security Regression run `42387172293` passed on exact source SHA `b046e3cbb84d73f4b3420f014c35d7b65925f26e` (https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42387172293). The production `common.c` lookup regression compiled and ran with GNU89 `-Wall -Wextra -Werror`; both CGI call-site wiring assertions and all prior HTTPD/archive policies passed. No local build or target build is claimed in this entry. Full MIPS firmware artifact and hardware remain NOT VERIFIED.
- Next: inspect the exact workflow result, then continue CGI command/configuration dataflow and broader userland modules.

---

## 2026-11-07 23:37 CST — Verify CGI service-ID bounds regression

- Code commit `b046e3cbb84d73f4b3420f014c35d7b65925f26e` passed HTTPD Authentication and Security Regression run `42387172293` on exact SHA: https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42387172293.
- `Compile and run policy regression` passed, including building production `trunk/user/httpd/src/common.c` plus the new `tests/httpd_service_id.c` under GNU89 `-Wall -Wextra -Werror`. `Verify HTTPD auth and route-access wiring` passed, including assertions that both `update_variables_ex()` and `apply_cgi()` resolve service IDs through `LookupServiceId()`, skip `<0` values while advancing `sid_list`, and no longer contain the sentinel-walking loop.
- Existing strict Basic-auth, login inactivity, private-key, MIME authorization, stale-upload and BusyBox archive path regressions also passed in the same workflow.
- L1/L2 PASS for this bounded service-ID fix. Full HTTPD/firmware MIPS build, image validation, archive fuzzing and hardware remain NOT VERIFIED. Continue `sys_script()`/`SystemCmd`, command/config dataflow and whole-userland review.

---

## 2026-11-07 23:39 CST — Validate BusyBox tar hardlink source paths

- Baseline main HEAD: 2b475fca3ac41f59a22059625d5e56dfb9b49fba. Follow-up to the verified archive member-path policy.
- Source review found `get_header_tar.c` strips unsafe prefixes from `file_header->name`, while `link_target` is taken from tar linkname fields; `data_extract_all()` later calls `link(file_header->link_target, file_header->name)` for hardlinks without equivalent validation. This creates a path-resolution escape attempt in root storage restore (actual hardlink success remains subject to Linux filesystem restrictions).
- Fix: before any mkdir/unlink/open side effect, call `bb_archive_path_has_unsafe_component(file_header->link_target, 0)` for hardlink-shaped entries (`S_ISREG`, non-NULL link target, size zero) and terminate extraction if unsafe. This rejects absolute/`..` targets and target paths traversing symlink or non-directory parents.
- The shared host policy regression already covers absolute paths, `..`, and symlink parents; added an explanatory assertion comment. Workflow now asserts link-target policy appears before leading-directory creation, unlink, and the `link()` call.
- Validation: HTTPD Authentication and Security Regression run `42387366333` passed on exact SHA `14c6658c06ed030f84dbdf7afb53c4d66b436540` (https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42387366333). The C policy test and the source-contract step passed, including the new pre-side-effect hardlink-source guard ordering. No full BusyBox MIPS build, tar integration fuzzing, image inspection or hardware validation is claimed.
- Next: verify exact CI, then review hardlink order/valid-archive compatibility before returning to CGI system-command/dataflow audit.

---

## 2026-11-07 23:40 CST — Verify BusyBox hardlink-source path guard

- Code commit `14c6658c06ed030f84dbdf7afb53c4d66b436540` passed HTTPD Authentication and Security Regression run `42387366333` on exact SHA: https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42387366333.
- Both `Compile and run policy regression` and `Verify HTTPD auth and route-access wiring` succeeded. The path-policy C regression and source contract verify hardlink target paths are validated before archive destination directory creation, unlink and `link()` operations.
- L1/L2 PASS for this bounded extraction policy. Full BusyBox/XY-C1 MIPS build, tar integration fuzzing, firmware artifact inspection, and hardware validation remain NOT VERIFIED.
- Next: review restore format compatibility, then focus on CGI `SystemCmd`/script dispatch and network/firewall userland modules.

---

## 2026-11-07 23:46 CST — Firewall URL filter guard checkpoint

- Implementation commit: `259288902383060425b2288f0c1c4bd8f60012e3` (`fix(firewall): reject unsafe URL filter config bytes`).
- Matching HTTPD security workflow run `42387739482` has exact head SHA `259288902383060425b2288f0c1c4bd8f60012e3` and status `completed` / conclusion `failure`: https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42387739482.
- Validation state: **FAIL**. No target build, runtime firewall integration test, firmware artifact or hardware validation is claimed.
- Job `HTTPD authentication and disclosure policy`: `failure`; steps: Set up job=success, Check out source=success, Compile and run policy regression=failure, Verify HTTPD auth and route-access wiring=skipped, Post Check out source=success, Complete job=success.
- Next: inspect the failed CI step and repair the regression contract or implementation as warranted.

---
## 2026-11-07 23:52 CST — Firewall ICMP rule-token guard and CI test repair

- Code checkpoint before documentation update: `895a1700fb498a5f9ec190a83b7282e5adb19416`; baseline for this extension: `ed5b3a270205857220ba49e641b232da0fbf2de0`.
- Recovered prior Actions failure `42387739482`, exact head `259288902383060425b2288f0c1c4bd8f60012e3`: the GNU89 regression step stopped because `NULL` was undeclared in `tests/firewall_filter_policy.c`; the compiler explicitly requested `<stddef.h>`. The earlier HTTPD/archive policy executables had passed; the firewall test did not compile, and the wiring step was skipped. This was a test-source build defect, not evidence that the firewall guard itself failed.
- Corrected the regression source by adding `#include <stddef.h>` in commit `ed5b3a270205857220ba49e641b232da0fbf2de0`.
- Continued the same bounded firewall review and identified a second P1 rule-injection sink: `include_lw_filter()` copied NVRAM `filter_lw_icmp_x` words into `--icmp-type %s` in an `iptables-restore` file. The web page's `validate_portlist()` is client-side only; the HTTPD `validate_cgi()` persistence path writes posted ordinary variables directly with `nvram_set()`. The shared `foreach()` macro splits only on ASCII spaces, so tabs/newlines remain inside a token and were not safe delimiters. An untrusted control character in that token could escape the intended rule line.
- Added `firewall_icmp_type_is_safe()` to `trunk/user/rc/src/firewall_filter_policy.h`: accepts decimal digits only and bounds the value to 0..255 with overflow-safe accumulation. `include_lw_filter()` now rejects invalid ICMP tokens before emitting the rule. The generated `fprintf` retains its original C newline escape.
- Expanded `tests/firewall_filter_policy.c` with positive 0/8/255/leading-zero cases and negative NULL/empty/256/negative/slash/space/CR/LF/tab/injected-rule cases. Extended `.github/workflows/httpd-security.yml` with a source-order assertion that the ICMP token guard precedes the `--icmp-type` rule write.
- Relevant commits, in order: `3f99db8651379751753e6125e917b86b7ba9adc9` (policy helper), `e713e06d65bced142cdd3a00b46007a9a24ce990` (call-site integration), `b0d8e11d3829ac836e577d9b4463e364b498e709` (C cases), `5296929c05bef7d8828612abe1be7cc5228a1d3d` (workflow wiring assertion), and `895a1700fb498a5f9ec190a83b7282e5adb19416` (preserve the actual newline escape in the generated rule).
- Local validation: compiled the exact fetched policy header and regression test using `cc -std=gnu89 -Wall -Wextra -Werror`, then executed it; **PASS**. This is the focused policy unit test only, not a build of production `firewall_ex.c` or the complete workflow.
- Current Actions state: the latest code HEAD `895a1700fb498a5f9ec190a83b7282e5adb19416` has an empty combined commit-status list. The available workflow-run query returns PR-triggered runs only and returned none; no current push-run ID was retrievable through the connected GitHub actions. Therefore the new workflow result is **PENDING / NOT RETRIEVED**, not passed. The only known run remains `42387739482` on the older SHA above and failed for the now-fixed missing include.
- Validation levels: focused GNU89 policy compile/run **PASS**; production source integration check and full CI **PENDING**; MIPS/XY-C1 target build **NOT RUN**; firmware/runtime/hardware **NOT RUN**.
- Residual firewall audit: raw port/protocol values returned by `portrange_conv()` and the TCP flag suffix returned by `protoflag_conv()` are interpolated in `include_lw_filter()`, `include_vts_filter()`, and `include_vts_nat()`; these paths need format-specific validation review. Date/time fields, all other NVRAM-to-restore sinks, command construction, chain lifecycle/rollback and IPv4/IPv6 parity are also incomplete. Firewall is still **AUDITING**, not complete.
- Next exact actions: obtain the workflow run for the current push and verify exact head SHA/steps; continue this firewall pass on the unvalidated port/protocol/date-time rule tokens and failure handling; then prioritize CGI `SystemCmd`/script dispatch and DNS/DHCP/WAN userland. Do not claim a full firmware build or hardware result.

---

## 2026-11-08 00:09 CST — Validate firewall ports, flags, protocol numbers and schedules

- Recovered the unfinished firewall input audit from the prior handoff. The previous bounded fixes covered URL-pattern quoting/control-byte injection and ICMP token injection; this round continued into raw port/protocol/TCP-flag and date/time arguments instead of returning to SkipDBv2.
- Added policy validators in trunk/user/rc/src/firewall_filter_policy.h and production call-site guards in firewall_ex.c:
  - filter-LW source/destination port fields accept only optional empty values, decimal ports 1..65535, or one ordered inclusive low:high range.
  - TCP flag suffixes are limited to the fixed UI enum (empty, ALL, SYN, ACK, FIN, RST, URG, PSH).
  - filter-LW other-protocol and VTS other-protocol values must be decimal IP protocol numbers 0..255.
  - VTS public port/range and optional translated local port are checked before both filter and DNAT rules are emitted.
  - schedule date must contain seven weekday bits; schedule time must contain eight HHMMHHMM digits with valid hour/minute values.
- Added regression cases for numeric boundaries/overflow, invalid/reversed ranges, control-byte/newline injection, unsupported TCP flags, weekday shape and invalid hour/minute values. Added source-wiring assertions for the production rule paths.
- CI recovery trail: run 42389702872 exposed the missing string.h dependency in the new policy header, fixed by 87b732a57d9038d9cc7e233d8d32f9b8fb97338b; run 42389728466 exposed a malformed workflow line continuation, fixed by aa9d424a2d08dc584bac40143d45a871e7a9136d; run 42389752533 exposed an overly exact flag-guard source assertion, fixed by 551ac54f3d605c71f334b7c6163d393de15e93aa.
- Final validation: HTTPD Authentication and Security Regression run 42389819329 completed SUCCESS on exact SHA 551ac54f3d605c71f334b7c6163d393de15e93aa. GNU89 compile-and-run policy regressions and complete HTTPD/firewall source-wiring step both passed. The C step used cc -std=gnu89 -Wall -Wextra -Werror. Logs confirmed HTTPD MIME auth, sid_list validation, stale upload cleanup, archive symlink-parent guard, firewall URL, ICMP and expanded rule-token contracts.
- Scope limit: Actions tested host regression programs and source contracts, not a production MIPS build. Full firewall_ex.c target compile, XY-C1 firmware image, runtime restore behavior and hardware validation remain NOT RUN.
- Schedule caveat: malformed date/time currently produces an empty time-match suffix, so the legacy rule becomes unqualified/all-time. Syntax injection is prevented, but fail-closed schedule semantics remain a separate open issue.
- Firewall remains AUDITING: remaining iptables/ip6tables NVRAM sinks, interface/address arguments, shell construction, chain lifecycle/rollback and IPv6 parity still require review.
- Next P1 module is CGI sys_script() / SystemCmd and privileged script dispatch; then DNS/DHCP, WAN/PPP/IPv6 and service lifecycle. Routine SkipDBv2 expansion remains paused.

## 2026-11-08 00:18 CST — Enforce trusted-session authorization for the HTTPD SystemCmd queue

- Audited Advanced_Console_Content.asp, apply.cgi, web_ex.c::apply_cgi(), web_ex.c::sys_script(), the MIME handler table and get_login_safe(). The console UI disabled its command control with client-side login_safe(), but the server-side apply_cgi() SystemCmd branch accepted a direct authenticated POST into process-global SystemCmd without checking that trusted-session policy.
- The command consumer sys_script('syscmd.sh') did check get_login_safe() before invoking doSystem(), but its denied branch only cleared /tmp/syscmd.log and left SystemCmd queued. The console page automatically calls nvram_dump('syscmd.log','syscmd.sh') while rendering. An authenticated non-safe HTTP session could therefore stage a root shell command, which could execute later when a trusted session opened the console. The apply.cgi MIME route requires authentication; this was a delayed authorization-boundary bypass, not unauthenticated access.
- Hardened web_ex.c: apply_cgi() now checks get_login_safe() before reading/copying SystemCmd. A denied submission clears the global buffer, returns HTTP 403, and does not queue the command. sys_script() also clears the buffer whenever its execution gate denies the command, preventing stale cross-session state.
- Extended .github/workflows/httpd-security.yml with a source-wiring regression proving the server-side trusted-session gate and queue clear/403 occur before SystemCmd request capture/copy, and verifying the denied execution path clears the buffer.
- Relevant commits: production fix e68e1ff12b92d22549d28cb20c9c0256dff1f492; workflow source-boundary correction 5aa2656eb1c8e641d759e453497f338282f13fae; final stable source assertions 31d94312ccac3451392f8c46fd901b295287dca4.
- CI run 42390628940 completed SUCCESS on exact head SHA 31d94312ccac3451392f8c46fd901b295287dca4. Both GNU89 host policy regression compile/run and HTTPD/firewall/SystemCmd source-wiring checks passed. The logs include “HTTPD SystemCmd trusted-session queue and stale-command guard: OK”.
- Validation boundary: the workflow compiles/runs focused policy tests and checks production source ordering; it does not compile the complete production web_ex.c for MIPS or exercise a live router. Target build, firmware image and hardware remain NOT RUN.
- The earlier firewall follow-up remains open: malformed schedule values are filtered out as input bytes but timematch_conv() returns an empty suffix, which means the legacy rule is unqualified/all-time. The complete firewall and IPv6 audits are not closed.
- Next scope: audit generic sys_script() /tmp _eval dispatch and request-controlled action_script callers, scripts.* file mutation/ap_script.sh execution and remaining privileged eval/_eval/doSystem flows; then DNS/DHCP, WAN/PPP/IPv6 and service lifecycle. SkipDBv2-only work remains paused.

## 2026-11-08 00:38 CST — Require trusted sessions for generic HTTPD /tmp script dispatch

- Baseline main HEAD: `b06608b9f99a9df035257339015591bf5d4014c4` (the immediately preceding checkpoint was docs-only; source/workflow checkpoint was `31d94312ccac3451392f8c46fd901b295287dca4`). Scope: `trunk/user/httpd/src/web_ex.c::sys_script()`, its request-controlled `action_script` callers, and `.github/workflows/httpd-security.yml`.
- Finding: after handling a set of fixed script names, `sys_script()` accepted any other name, constructed `/tmp/<name>`, parsed optional arguments and invoked `_eval()` as root. Both `update_variables_ex()` and `apply_cgi()` take `action_script` from the POST body and call this dispatcher; their `update.cgi` / `apply.cgi` routes require authentication, but the generic fallback had no additional trusted-session gate. Existing character/argument checks reduce shell-token injection but do not authorize the selection of an executable already present in `/tmp`. Exploitation therefore required valid credentials and an existing matching executable; this is not unauthenticated command execution.
- Fix: the generic fallback now returns unless `get_login_safe()` is true, before request-name parsing and before the `_eval(script_argv, ...)` call. Fixed-name handlers above the fallback retain their previous behavior and policy; the separate SystemCmd queue gate remains unchanged.
- Regression contract: extended `.github/workflows/httpd-security.yml` to locate the generic fallback and assert `get_login_safe()` precedes token parsing and `_eval()`, that the fallback constructs `/tmp/%s`, and that both POST-controlled `action_script` handlers still flow through `sys_script()`. Existing MIME-route/auth-order assertions cover authenticated CGI route entry.
- Commits: production fix `3ec7ab30c91b7fb95ad8834695c0f2b18bec9daf`; initial workflow contract `fc8f1af48cdc66ac007e86e7c7d1de5f49a8c587`; corrected escape-independent fallback locator `fb488c486462bdbf1dbedbaad559f21bd7f1785c`; roadmap update `b2fb0e0120e352650308f74c6281d2ec41a9cf91`. The source/test checkpoint is `fb488c486462bdbf1dbedbaad559f21bd7f1785c`.
- Validation: L1 source review completed and the final workflow contract was inspected against current source text. No local compiler or C regression was run. Combined commit statuses were empty; the available commit-to-workflow query is limited to PR-triggered runs and returned none. The push-triggered HTTPD Authentication and Security Regression is now confirmed SUCCESS: run 42392318537 completed on exact head SHA fb488c486462bdbf1dbedbaad559f21bd7f1785c; its compile/run regression and source-wiring steps both passed. This remains focused host/source-contract verification, not a target MIPS build or hardware validation.
- L3 target MIPS build / firmware image inspection and L4 router hardware validation: NOT RUN. Live runtime behavior and /tmp executable inventory are also NOT VERIFIED.
- Remote checkpoint comparisons against current `main`: `refactor/bstream-tag-wire` 1 ahead / 468 behind; `refactor/dbapi-result-20261105` 8 ahead / 549 behind; `refactor/dynlib-lifecycle` 0 ahead / 472 behind; `refactor/sorting-boundaries` 0 ahead / 468 behind; `refactor/uarray-character-ctype` 2 ahead / 466 behind. Deltas remain limited to old SkipDBv2 branches; no unrelated branch work was merged.
- Next: retrieve and verify the exact-head Actions run for `fb488c4`; then inventory legitimate generic `/tmp` scripts and all `sys_script()` callers, audit `scripts.*` textarea writes plus `ap_script.sh` execution and event-mask gates, and continue the remaining `eval`/`_eval`/`doSystem` trust-boundary review. Keep firewall schedule fail-closed semantics and whole-userland coverage open.

---

## 2026-11-08 00:58 CST — Require trusted sessions for HTTPD executable-script writes

- Baseline main HEAD: 5025bcb42d92c715d55595c15ff161cf13eb920e. Source commit: 5025bcb42d92c715d55595c15ff161cf13eb920e (fix(httpd): restrict executable script writes to trusted sessions). Scope: trunk/user/httpd/src/web_ex.c, trunk/user/httpd/src/variables.c, new httpd_script_policy.h / tests/httpd_script_policy.c, and .github/workflows/httpd-security.yml.
- Finding: write_textarea_to_file() classifies .sh uploads as executable files and writes them to root-managed storage with mode 0755, but the shared writer previously had no get_login_safe() check. Although the six General custom-script fields already carried EVM_BLOCK_UNSAFE, both wireless scripts.ap_script.sh variable definitions (2.4 GHz and 5 GHz) omitted that bit. validate_asp_apply() also called doSystem("/etc/storage/ap_script.sh") for that field without a server-side trust check. This required a valid authenticated management session; it was an authorization gap for executable script mutation/execution, not unauthenticated access.
- Fix: added a focused httpd_script_file_write_allowed() policy helper. The shared .sh writer now denies untrusted sessions before temp-file or destination-file operations; both wireless AP-script variable entries add EVM_BLOCK_UNSAFE; direct ap_script.sh execution also requires get_login_safe(). The existing HTTPS behavior of get_login_safe() is intentionally preserved. Non-script config/cert/key file classes retain their prior authorization behavior.
- Regression: added tests/httpd_script_policy.c for trusted/untrusted script writes and unaffected non-script file classes; extended HTTPD workflow path filters, GNU89 compile/run and source-to-callsite assertions for gate ordering, both variable entries and the direct-execution guard.
- Validation: local cc -std=gnu89 -Wall -Wextra -Werror compile/run of the focused policy regression PASS. HTTPD Authentication and Security Regression run 42393893775 completed SUCCESS on exact code SHA 5025bcb42d92c715d55595c15ff161cf13eb920e; compile/run and source-wiring jobs both passed. Log confirms “HTTPD executable script write and execution trust boundary: OK”. Earlier generic /tmp dispatcher gate is also confirmed by run 42392318537 SUCCESS on exact SHA fb488c486462bdbf1dbedbaad559f21bd7f1785c.
- Validation boundary: L1 targeted source review PASS; L2 focused host regression / exact-head Actions PASS. Full production web_ex.c MIPS compile, XY-C1 firmware image generation and inspection, live runtime and hardware validation are NOT RUN / NOT VERIFIED.
- Remaining: enumerate all generic /tmp targets and template/request callers; trace every scripts.* consumer and alternative file-write path; continue the remaining eval/_eval/doSystem trust-boundary audit, HTTPD session/logout/ACL, firewall invalid-schedule fail-closed behavior and IPv6 parity. Whole-repository inventory and audit remain IN PROGRESS.

---
## 2026-11-08 01:27 CST — Fail closed firewall schedule dispatch

- Baseline main HEAD: `5e465c371fe4ff585280ee6724152355be42fa94`. Latest production-source checkpoint: `166274833a1df1ca8a8312fa1d250df72934b738`.
- Scope: `trunk/user/rc/src/firewall_filter_policy.h`, `firewall_ex.c`, `tests/firewall_filter_policy.c`, and `.github/workflows/httpd-security.yml`.
- P1 finding: `timematch_conv()` previously returned an empty match string for malformed date/time values and for the explicit all-zero weekday mask. The MAC, IPv4/IPv6 URL, and IPv4 LAN-to-WAN consumers then emitted a rule/jump without a time qualifier, making a scheduled rule effectively all-time.
- Fix: added `firewall_schedule_is_active(date, time)` to the policy header. It accepts only valid weekday/time formats with at least one selected weekday. `timematch_conv()` now returns an activity result: invalid or disabled schedules return false; valid all-days/all-day schedules return true with the legacy empty match suffix. MAC filter entries, URL jumps on IPv4 and IPv6, and the LAN-to-WAN filter jump now suppress inactive schedules. The MAC list count is retained before skipping an inactive rule so whitelist mode retains its existing terminal-drop behavior.
- Commit sequence: policy helper `01b0aae8758a638347cc8968a4e81bd021032697`; production dispatch fix `e39d4556136ce6982117bcb81a4e6c6ea0c3d40e`; regression cases `fd8f5306fb23b315fabb2ec8621ee4d21b3f14fa`; Actions source-wiring assertions `034d4de4d98103402c9980b7a9dfefaa877fa85f`; indentation cleanup `166274833a1df1ca8a8312fa1d250df72934b738`.
- Regression additions: active all-day and selected-day schedules; inactive zero-weekday schedules with valid times; malformed date, malformed time, and NULL argument cases. Local standalone policy harness compiled and ran with `cc -std=gnu89 -Wall -Wextra -Werror`: PASS. The harness directly exercised the corresponding policy-helper logic; it was not a full production `firewall_ex.c` compile.
- Source wiring review: verified the active-status gate in `timematch_conv()` and all four consumers (MAC, IPv4 URL, IPv6 URL, LAN-WAN), plus the new regression/workflow contract strings. PASS as a source review, not as an Actions result.
- GitHub Actions: the available commit status for exact source SHA `166274833a1df1ca8a8312fa1d250df72934b738` was empty; the exposed commit-workflow query is limited to PR-triggered runs and returned none. Exact push-triggered workflow result PENDING / NOT RETRIEVED; do not claim CI success.
- Validation not performed: production `firewall_ex.c` MIPS compilation, XY-C1 firmware build/artifact inspection, runtime iptables/ip6tables restore testing, and hardware validation. Whole firewall and whole-userland audits remain incomplete.
- Remote checkpoints compared against main at the recovery baseline were all stale/diverged and confined to SkipDBv2 history; none was merged.
- Next: verify this exact-head Actions run when it becomes retrievable; then continue the unreviewed iptables/ip6tables NVRAM sinks, interface/shell argument validation, chain lifecycle/rollback and IPv6 parity. In parallel, resume HTTPD fixed-handler/script-consumer and session/ACL audit, then expand DNS/DHCP, WAN/PPP, wireless, file sharing, storage/hotplug and init/service lifecycle inventory.

## 2026-11-08 01:37 CST — Make fixed-name HTTPD script dispatch explicit

- Baseline main HEAD: `271cc9b7adfef3765a0c33ad25bbdb13b3b4c533`. Source commit: `3859fc59fa3b890a6ce08a2956719c6640a88337` (`fix(httpd): make fixed script dispatch allowlist explicit`).
- Recovered the precise unfinished item in SESSION_HANDOFF: audit the fixed-name `sys_script()` branches separately from the generic `/tmp/<name>` root-execution fallback.
- Reviewed `sys_script()` and all three production call sites (`ej_dump()`, `update_variables_ex()`, `apply_cgi()`), MIME route table, EJ template dispatch, active-session trust helper and relevant ASP examples. Dynamic ASP/HTM and apply/update CGI routes in the audited MIME table require authentication. The fixed handler list has bounded semantics: no-op/read-only handlers, fixed process/RC signals, and printer FIFO operations that write fixed bytes; `syscmd.sh` keeps its separate trusted-session gate. No free-form user command is added to the fixed-name handlers.
- Change: introduced an exact fixed-name allowlist and dispatch policy helper in `httpd_script_policy.h`; `sys_script()` evaluates the policy before branching. Unlisted scripts still require `get_login_safe()`; the existing generic fallback input validation and `_eval()` argv dispatch remain in place. Existing fixed-handler behavior is deliberately unchanged.
- Regression: extended `tests/httpd_script_policy.c` to cover every fixed handler name, unknown names for trusted/untrusted sessions, and NULL. Workflow source contract asserts all allowlisted names and that the gate precedes the first dispatch branch. Added `variables.c` to the pull-request path filter, where it was missing.
- Prior checkpoint CI recovery: firewall schedule exact-head Actions run `42396303871` is SUCCESS on SHA `166274833a1df1ca8a8312fa1d250df72934b738`; its job's compile/run and source-wiring steps both passed.
- Current change exact-head Actions run `42397188114`, head SHA `3859fc59fa3b890a6ce08a2956719c6640a88337`: SUCCESS; job `128476978918` passed the GNU89 policy test compile/run and source-wiring assertions. This confirms L2 host regression only. L3 MIPS/firmware and L4 router/hardware validation: NOT RUN. This bounded fixed-name audit does not complete HTTPD/CGI or the wider userland audit.
- Exact-head result retrieved and verified: Actions `42397188114` SUCCESS on the exact code SHA; both job steps passed.
- Next: trace every `scripts.*` shell file from the HTTPD write path/variable to its RC/init/service consumer and enumerate alternate writers plus root `eval`/`_eval`/`doSystem` inputs.

## 2026-11-08 01:57 CST — Remove the HTTPD script-writer shared-/tmp symlink race

- Baseline main HEAD: `dea8ed96eed030a77e8f1fc416330b2ec563348f`. Source commit: `0bbd8062e8bf7842e4879032aca3c1781bd393fa` (`fix(httpd): create upload staging files safely`). Scope: `trunk/user/httpd/src/web_ex.c` and `.github/workflows/httpd-security.yml`.
- Continued the previous handoff's script-consumer trace. Enumerated all 28 `scripts.*` File entries in `variables.c` (27 distinct names; the AP script is listed once per wireless band). Confirmed the common HTTPD writer maps each `scripts.*` payload to `/etc/storage/<filename>`, while `ej_dump()` reads the same prefix from that directory; `validate_cgi()` skips File entries. Initial RC execution mapping covered startup/shutdown, WAN up/down, Internet-state changes, the firewall post-script, VPN callbacks, button handling and AP auto-start. DDNS/FRP/Shadowsocks/Adbyby downstream consumers remain to be fully traced.
- P1 local TOCTOU finding: the old writer unlinked a predictable `/tmp/.<filename>` and then reopened that pathname with `fopen(..., "w")`. A local process able to create an entry in shared `/tmp` could race the gap with a symlink and redirect the privileged HTTPD write. This is a static risk finding; a concurrent exploit stress test was not performed.
- Fix: factored CRLF conversion into `write_file_dos2unix_fp(value, FILE *)`; the compatibility path wrapper still serves the final target file. Staging now uses `mkstemp("/tmp/httpd-upload-XXXXXX")` and `fdopen()` on the returned descriptor. The random unique staging file is created atomically and held through its original descriptor instead of unlinking and reopening a predictable path. The helper reports stream write/close failures; error and success paths remove the staging file.
- Regression contract: workflow now checks trusted-session gate → `mkstemp` → `fdopen` → descriptor-backed writer order and asserts the former predictable temp path is absent.
- Exact-head CI: HTTPD Authentication and Security Regression run `42398764761` completed SUCCESS on exact code SHA `0bbd8062e8bf7842e4879032aca3c1781bd393fa`; job `128481780467` completed SUCCESS. The GNU89 policy regression compilation/execution passed, as did source-wiring checks for the new staging contract and existing auth/session/CGI/firewall defenses.
- Validation boundary: L1 targeted source review PASS; L2 existing GNU89 host policy tests and exact-head Actions PASS. The workflow's new staging check is a source contract, not a complete `web_ex.c` compile. No symlink-race stress test, MIPS target build, XY-C1 image, live router or hardware test was run.
- Next: finish add-on script consumer mapping (DDNS, FRP, Shadowsocks, Adbyby), enumerate alternate writers and executable `/tmp` producers/callers, then continue HTTPD session/logout/ACL and firewall restore/IPv6 audits. Broaden next into DNS/DHCP, WAN/PPP, file sharing, storage/hotplug and init/service lifecycle; keep routine SkipDBv2 expansion paused.

## 2026-11-08 02:23 CST — Isolate AP and AliDDNS root-managed temporary executable staging

- Baseline main HEAD: `c3caea4906db771fada2388e5fad569202697041`. Source commit: `bd89f748a4c86c0f8267f4fa31d40c99dd4a448f` (`fix(security): isolate root script staging in private tmpfs`).
- Resumed the prior handoff's exact next action: trace add-on consumers for the HTTPD `scripts.*` variables and identify additional root-generated executable temporary files. The 28 File rows (27 unique names) were traced from `variables.c` through the common HTTPD writer/read path and into RC/service/add-on consumers.
- Consumer findings: `aliddns.sh` sources `/etc/storage/ddns_script.sh`; built-in inadyn's `/sbin/ddns_updated` is a different callback. `frp.sh` executes `/etc/storage/frp_script.sh`. Shadowsocks domain/IP/host lists, Adbyby rule/block/host lists and SmartDNS custom lists are read as data/config in the inspected consumers, not evaluated as shell. RC callbacks cover startup/shutdown, WAN up/down, Internet state, post-firewall, VPN up/down, button and AP events.
- P1 static issue: stock `ap_script.sh` wrote the AP monitor daemon to predictable `/tmp/sh_apauto.sh`, then chmodded it 0777. Changed it to atomically claim a per-run directory below `/home/root` (tmpfs; dev_init creates that parent mode 0700), write the generated daemon there, chmod it 0700, and clean up on EXIT and termination signals.
- P1 static issue: AliDDNS wrote its `_aliddns` PATH wrapper into predictable shared `/tmp/script` with mode 0777. Moved it to `/home/root/.aliddns/_aliddns`, mode 0700 for directory and file, retained PATH lookup via the new private directory, and updated cleanup to target the new helper.
- CI changes: workflow push/PR filters now include `dev_init.sh`, the stock AP script and AliDDNS script; PR filters also include the previously omitted HTTPD script-policy header/test. Added outer-script `sh -n`, generated AP daemon body syntax parsing, and source-contract assertions for private paths and permissions.
- Exact-head Actions: `42400922809` completed SUCCESS on exact code SHA `bd89f748a4c86c0f8267f4fa31d40c99dd4a448f`; job `128488273622` SUCCESS. GNU89 policy regression compile/run, new staging step and HTTPD auth/route wiring checks passed.
- Validation boundary: source/call-path review and L2 host Actions PASS. The source contracts do not prove a live symlink-race exploit or AP/ALiDDNS runtime behavior. No MIPS target build, firmware image, router boot or hardware test was performed.
- Upgrade caveat: `autostart.sh` runs an existing executable `/etc/storage/ap_script.sh` and only copies the ROM default if that file is not executable. No auto-migration was added because an existing executable may be user-customized; define a stock-only migration separately.
- Next: review migration semantics for old AP copies without clobbering user settings; continue HTTPD logout/session/ACL and same-NAT ownership review, then firewall restore/IPv6 parity. Broaden next into DNS/DHCP, WAN/PPP, FTP/Samba, storage/hotplug and service lifecycle; routine SkipDBv2 expansion remains paused.

## 2026-11-08 02:35 CST — Migrate only byte-identical legacy AP script copies

- Baseline main HEAD: `8a83af5f33680f326e3904c06bd6843dea00e7f5`. Final tested checkpoint on main: `150e7869077bf7b4a275d61b57648a319c1849b8`; implementation includes the AP migrator, autostart call and regression introduced in the preceding code commits.
- Recovered the latest handoff's exact next action: design an upgrade path for existing executable `/etc/storage/ap_script.sh` files without overwriting user customizations. The old `autostart.sh` branch launches any existing executable storage copy and only copies the ROM default if no executable copy exists.
- History check: the previous tracked stock AP script is commit `4d3da07289441a229611f0f12fe5d8efbb9b10fa`. Its exact SHA-256 (`8d94a244022c46b6f5e921065fd6b6cadcaa58134a034f8332a8fc12b5af3251`) is pinned; the historical file is committed as `tests/fixtures/ap_script_legacy_stock.sh`.
- Change: added `ap_script_migrate.sh` and wired it before the existing executable AP script is launched. It updates only regular executable non-symlink files whose bytes match that exact historical stock digest. Custom/unknown copies and symlinks are untouched. The new ROM template is staged inside an atomically claimed 0700 directory under `/etc/storage`, chmodded 0700, and moved over the known stock path on the same filesystem.
- Target primitive audit: `trunk/configs/boards/busybox.config` confirms `CONFIG_SHA256SUM=y` and the required CUT/DIRNAME/MKDIR/CP/CHMOD/MV/RM/RMDIR/LOGGER applets. It also confirms `CONFIG_MKTEMP` is disabled; the final helper therefore avoids MKTEMP and uses exclusive `mkdir` with a PID/attempt suffix. The workflow asserts this target compatibility boundary.
- Regression: shell syntax for autostart/helper/test; exact-stock migration; 0700 result; customized script unchanged; executable symlink unchanged; no staging leftovers; migration call ordered before the persistent script launch. Existing GNU89 HTTPD/firewall policy tests and other source contracts remain in the same workflow.
- Exact-head Actions: HTTPD Authentication and Security Regression run `42402389122` SUCCESS on exact SHA `150e7869077bf7b4a275d61b57648a319c1849b8`; job `128492784540` passed every step, including the new migration regression and the existing C policy compile/run and HTTPD wiring checks.
- Intermediate CI failures while developing the regression were corrected before the final exact-head run: the first harness used a literal, unexpanded TMPDIR string; the later source-contract assertion was corrected to compare the two-dollar PID token accurately. The failed attempts are not treated as passing validation.
- HTTPD session/ACL review (no code change): `http_login_check()` recognizes the owner by source IP, so same-NAT clients are indistinguishable. Protected routes still call Basic `auth_check()`, therefore the lock is not an authentication bypass. The unauthenticated `Logout.asp` exception triggers a 401 to clear browser Basic credentials; `http_logout()` only resets for the recorded IP, but a same-NAT peer can clear the shared lock and cause a re-login. The idle timeout only releases the lock to a different IP; same-IP requests are still checked with Basic auth. Treat this as session arbitration/forced-logout behavior, not a proven protected-route bypass; review compatible CSRF mitigation separately.
- Validation boundary: L1 targeted source/config/call-path review PASS; L2 exact-head Actions PASS. No MIPS target build, XY-C1 firmware image, live router migration or hardware validation was run.
- Next: complete HTTPD logout/session/ACL and conditional `need_auth` route review; then continue firewall restore/NVRAM sinks, chain rollback and IPv6 parity. Broaden the whole userland beyond HTTPD to DNS/DHCP, WAN/PPP, file sharing, storage/hotplug and service lifecycle. Keep routine SkipDBv2 work paused.

## 2026-11-08 02:57 CST — Enforce an exhaustive HTTPD MIME authorization matrix

- Baseline main HEAD: 2f57e866271fc5b9c7cc0253275330594b605e06. Source/test commit: 1e305137d9b2e2f0add305a323038f2d722dd162 (test(httpd): require exhaustive route auth matrix).
- Resumed the exact SESSION_HANDOFF action: finish the current HTTPD conditional need_auth route matrix and review logout/session ownership; the previous AP stock-only migration was already complete.
- Audited the current mime_handlers[] table and HTTPD request path. It contains 35 unique patterns: 19 intended public static resources and 16 protected dynamic page, JS, export, config/API and upload/restore routes. Explicit public JavaScript patterns precede the protected **.js catch-all. Each currently protected route passes the Basic-auth gate before its callback for non-local clients.
- Strengthened .github/workflows/httpd-security.yml so the route pattern set must equal the explicit public/protected inventories. New/unknown routes, removed routes, duplicate patterns, unexpected auth flags and route-count drift now fail the source-wiring check. This is a test/CI contract improvement; no production C change was needed because all current routes were classified correctly.
- Logout review: Logout.asp is the intentional route exception and initiates a HEAD /logout 401 challenge for browser Basic-auth cache clearing. http_logout() only resets the management lock when the HTTPD-observed source address matches login_ip. Clients indistinguishable behind upstream NAT/proxy/SNAT can force re-authentication by invoking logout or triggering the existing same-source failed-auth reset. Protected routes still require valid Basic credentials; no auth bypass was identified. This is recorded as a limited forced-logout/availability residual; no potentially incompatible cookie/CSRF token redesign was introduced.
- Exact-head Actions: run 42403799683 completed SUCCESS on exact SHA 1e305137d9b2e2f0add305a323038f2d722dd162; job 128497069078 completed SUCCESS. Its logs show HTTPD exhaustive MIME authorization matrix: OK; the GNU89 C policy compile/run, AP stock-migration regression, root-managed executable staging and HTTPD/firewall source-wiring checks also passed.
- Validation levels: L1 source/route/call-path review PASS; L2 exact-head GitHub Actions PASS. The workflow does not compile the full production HTTPD/RC code for MIPS. L3 target MIPS build/firmware image and L4 live router/hardware validation remain NOT RUN.
- Remote checkpoint branches were compared with the same main SHA: BStreamTag 1/499, DBAPI 8/580 and UArray 2/497 ahead/behind respectively; dynlib lifecycle 0/503 and sorting 0/499. The ahead-only file deltas are confined to SkipDBv2 source/tests/workflow; none was merged.
- Next: continue the firewall P1 review: remaining iptables/ip6tables NVRAM sinks, restore-file input composition, shell/interface/address argument validation, chain rollback/cleanup and IPv6 parity. Then proceed to DNS/DHCP, WAN/PPP, FTP/Samba, storage/hotplug and service lifecycle. Keep routine SkipDBv2 expansion paused.

## 2026-11-08 03:10 CST — Harden built-in IPv4/IPv6 firewall restore-file staging

- Baseline main HEAD: `a32605822a3c0028cf7b593068d34392d773d81a`. Production/test/workflow commit: `876d2740ce0bf2398b69906c9051401464308f11`; final source/test checkpoint after IPv6 cleanup indentation correction: `18993430b0cea13d94c706dc5d2157ed8849127c`.
- Scope: `trunk/user/rc/src/firewall_ex.c`, new `firewall_rule_file.h`, new `tests/firewall_rule_file.c`, and `.github/workflows/httpd-security.yml`.
- P1 static finding: enumerated 10 built-in IPv4/IPv6 restore-file writers. They used predictable root-written `/tmp/ipt_*.rules` and `/tmp/ip6t_*.rules` paths with `fopen(..., "w")`, which follows a pre-existing symlink. A local process with write access to shared `/tmp` could plant a fixed-name symlink before a firewall restart and redirect the privileged writer. This was identified by source review; no target exploit was executed.
- Fix: replaced all 10 fixed names with unique `XXXXXX` templates and a shared `mkstemp()` + `fdopen()` helper. The new descriptor is explicitly mode 0600. The finish helper checks `fflush()`, `ferror()` and `fclose()`; each restore is invoked only after a successfully completed file, and the generated file is removed after the restore attempt. Optional IPv4/IPv6 mangle/raw module conditions are retained.
- Regression: new C test plants a symlink at a legacy fixed-name path to a sentinel file and confirms that the staging helper does not follow/overwrite it; it also validates file mode 0600, contents, and cleanup. The workflow adds the header/test to push and PR path filters, compiles/runs the test using `cc -std=gnu89 -Wall -Wextra -Werror`, and asserts all ten production writers use the helper and no legacy fixed names remain.
- Local host test: `cc -std=gnu89 -Wall -Wextra -Werror -I /tmp /tmp/firewall_rule_file.c -o /tmp/firewall_rule_file && /tmp/firewall_rule_file` — PASS.
- Exact-head Actions: HTTPD Authentication and Security Regression run `42404900208` completed SUCCESS on exact source/test SHA `18993430b0cea13d94c706dc5d2157ed8849127c`; job `128500370999` completed SUCCESS. The GNU89 policy compile/run step and firewall source-wiring assertions passed, including `Firewall IPv4/IPv6 restore files use exclusive 0600 staging and checked cleanup: OK`.
- Validation boundary: L1 focused source/call-site review PASS; L2 focused host C regression and exact-head Actions PASS. No full RC/production `firewall_ex.c` compile, MIPS target build, XY-C1 firmware artifact, live restore, router or hardware test was performed.
- Remaining: NVRAM-derived interface/address/rule argument provenance, VPN lease parsing, add-on restore file producers at `/tmp/shadowsocks_iptables.save` and `/tmp/adbyby.save`, post-firewall hooks, restore return-code handling, cross-table rollback and IPv6 semantic parity remain open. Firewall and whole-userland audits remain IN PROGRESS.

## 2026-11-08 03:25 CST — Harden firewall IPv4 restore tokens and VPN PPP lease trust

- Resume baseline: `main` HEAD `ed49070e7496fe982098dc1520f3bca09c3183e5`; resumed the exact handoff item after the already-verified restore-file staging checkpoint. Final source/test/workflow checkpoint: `53186f1753a5c8ef55a8f144980b62af742f4722`.
- Scope: `trunk/user/rc/src/firewall_ex.c`, `firewall_filter_policy.h`, `tests/firewall_filter_policy.c`, `vpn_server.c`, and `.github/workflows/httpd-security.yml`.
- P1 static finding — IPv4 rule tokens: persistent `lan_ipaddr` is exposed as a normal NVRAM variable without a validation callback in the inspected HTTPD variable table. `start_firewall_ex()` copied it into a rule-generation buffer and passed it through NAT/filter paths, where it was interpolated as an unquoted iptables-restore token. A malformed value containing restore syntax could alter the generated transaction. This was a source-level finding; no router exploit was attempted.
- Fix: added `firewall_ipv4_address_is_safe()` using `inet_pton(AF_INET)`; replaced the eight permissive IPv4 checks in `firewall_ex.c` with strict parsing, including virtual-server destinations, DMZ, WAN/MAN and 6in4 peer values. `start_firewall_ex()` now validates LAN IPv4 before `ip2class()` or any rule-table generation and logs/falls back to the safe token `0.0.0.0` on invalid input. The helper rejects malformed, appended, CR/LF and non-IPv4 values.
- P1 static finding — VPN lease trust: `ipup_vpns_main()` ran with `umask(0000)` and used `fopen("/tmp/vpns.leases","a+")`, potentially creating world-writable firewall policy state. `ipdown_vpns_main()` used a predictable `/tmp/.vpns.leases` replacement path. `include_vpns_clients()` trusted lease-file contents and used the permissive `ppp_ifindex()`, which accepts suffixes such as `ppp10+` that carry iptables interface-wildcard semantics.
- Fix: lease creation now uses `open(O_CREAT|O_APPEND|O_NOFOLLOW, 0600)`, verifies a root-owned regular single-link inode and enforces 0600 before `fdopen()`. Lease reading uses `O_NOFOLLOW` plus `fstat` owner/type/link/mode checks. Disconnection rewrites through unique `mkstemp()` staging with explicit 0600, checked flush/error/close, and rename only after a complete write. Added a strict canonical `ppp<decimal-unit>` validator (bounded length, no suffix/sign/leading-zero aliases, unit >= VPN server base) and applied it before any lease interface becomes a restore rule.
- Regression/CI: extended the GNU89 policy test with IPv4 injection and PPP wildcard/malformed cases; extended the workflow path filters and source-to-call-site contracts for the LAN validation ordering, lease file ownership/mode, no-follow open, exclusive rewrite, and strict interface gate.
- Exact-head Actions: intermediate run `42406037598` on `559ab43377d446df5986c52d08aba61cf3b9f77a` failed in the source-wiring step because one DMZ assertion expected the wrong branch polarity (production correctly uses a positive `if (firewall_ipv4_address_is_safe(dmz_ip))`). Corrected that assertion in `53186f1753a5c8ef55a8f144980b62af742f4722`. Final run `42406069140` is SUCCESS on that exact SHA; job `128503923243` passed the GNU89 C policy compile/run and source-wiring checks. Logs explicitly contain `Firewall strict IPv4 restore-token validation and LAN fallback: OK`, `VPN lease file private descriptor and strict PPP token checks: OK`, and the existing IPv4/IPv6 restore staging assertion.
- Validation boundary: L1 focused source/call-site review PASS; L2 focused GNU89 host regressions and exact-head GitHub Actions PASS. The workflow does not compile the complete production RC files for MIPS. No MIPS target build, firmware image, live firewall apply, live VPN connection/disconnection test, exploit stress test, router runtime or hardware test was performed.
- Remaining firewall work: audit producers and permissions for `/tmp/shadowsocks_iptables.save` and `/tmp/adbyby.save`, `SCRIPT_POST_FIREWALL`, and `/opt/bin/update_iptables.sh`; trace all residual interface/NVRAM tokens; check restore exit-code handling, multi-table rollback and independent IPv6 parity. WAN/PPP as a whole remains unaudited; only the VPN lease trust boundary received a bounded fix. DNS/DHCP remains unaudited. No SkipDBv2 checkpoint was merged.
- Remote checkpoints compared with main `53186f1753a5c8ef55a8f144980b62af742f4722`: BStreamTag 1 ahead / 518 behind; DBAPI 8 / 599; dynlib lifecycle 0 / 522; sorting 0 / 518; UArray 2 / 516. All ahead-only files remain SkipDBv2-related; none was merged.
- Next exact action: enumerate the repository producers/consumers of the two add-on restore files and post-firewall hook scripts, establish their writer identity, permissions, content grammar and command dispatch; then handle restore return values/rollback and IPv6 parity. Continue broadening into DNS/DHCP and WAN/PPP. Keep routine SkipDBv2 expansion paused.

## 2026-11-08 04:20 CST — Isolate optional add-on firewall restore inputs

- Source/test/workflow commit: `699639f7e94eba48ea2f54cbad3c52d227ca1eec`; baseline: `918c07bcde23d1b698ba1fcf5aecec5b9e73525a`. Exact-head Actions run `42410593918` is running against the source commit.
- Scope: `trunk/user/rc/src/firewall_ex.c`, new `firewall_hook_file.h` and `tests/firewall_hook_file.c`, `trunk/user/adbyby/adbyby.sh`, new `tests/firewall_snapshot.sh`, and `.github/workflows/httpd-security.yml`.
- P1 finding: tracked Shadowsocks scripts contained no producer for `/tmp/shadowsocks_iptables.save`, but the firewall service executed that predictable shared-`/tmp` path as root. A local user could plant the path and obtain execution after a firewall restart. The consumer now rejects symlinks, non-root ownership, non-regular/multiply-linked files, group/other-writable mode and inputs above 1 MiB; it copies from the validated descriptor into a unique 0600 file below `/home/root`, executes the snapshot and removes it.
- P1 finding: Adbyby's root shell redirected a selected `iptables-save` transaction to predictable `/tmp/adbyby.save`; the firewall later applied it via privileged `iptables-restore -n`. The producer now uses PID-scoped temporary files in `/home/root`, `umask 077`, explicit mode 0600 and same-directory atomic rename to `/home/root/.adbyby.save`; failed capture/filtering leaves the previous final snapshot unchanged. Producer and consumer paths match.
- Tests: GNU89 C coverage for descriptor staging, private directory/file permissions, symlink/hardlink rejection and size bounds; shell coverage executes the actual Adbyby snapshot function with stubbed commands, checks rule grammar/insertion behavior, safe replacement of a planted destination symlink and preservation of a known-good snapshot on capture failure. Workflow now runs these regressions and checks source wiring.
- Remaining: `/etc/storage/post_iptables_script.sh` and optional Optware `/opt/bin/update_iptables.sh` provenance/trust; restore exit statuses, rollback, IPv6 parity and residual interface/NVRAM tokens. Continue into DNS/DHCP and full WAN/PPP; whole-userland coverage remains IN PROGRESS.
- Validation status: code and tests committed; exact-head Actions run `42410593918` is still running. No complete production RC compile, MIPS build, firmware artifact, live iptables restore, router runtime, race/exploit stress or hardware test has been performed.

## 2026-11-08 04:27 CST — Exact-head host verification of add-on firewall hook isolation

- Final code/test checkpoint: `6949cdf53ba9ba682c169b30eee2fab77949b04b`; implementation commit `699639f7e94eba48ea2f54cbad3c52d227ca1eec`; exact-head Actions run `42410980863` SUCCESS. All job steps passed.
- Test iteration notes: earlier exact-head runs first caught a test harness `set -u` mismatch with the legacy Adbyby script, then a fixture line that did not match Adbyby's intentional `ADBYBY` grep filter. Both were corrected. The last test-only commit also explicitly exports the simulated iptables-save failure to the child stub. Treat run `42410980863` on `6949cdf53ba9ba682c169b30eee2fab77949b04b` as the authoritative final result.
- Source fix: root-executed Shadowsocks `/tmp/shadowsocks_iptables.save` is now validated by no-follow descriptor open, owner/type/link/mode/size checks and copied to a unique 0600 file under `/home/root` before execution. The tracked subtree had no producer for that exact filename; invalid input fails closed.
- Source fix: Adbyby no longer writes `/tmp/adbyby.save`; it writes a private snapshot to `/home/root/.adbyby.save` using restrictive umask, unique intermediate files, explicit 0600 and atomic rename. Firewall restore consumer uses the same path. Failed capture preserves the previous snapshot.
- Host coverage: GNU89 C test and actual Adbyby snapshot shell test pass via exact-head Actions. Existing route/auth, firewall filter/file, AP storage migration and root-temp-script staging tests in the same workflow also passed.
- Limitations: no full RC build, MIPS target build or image, no router run/live iptables restore/hardware test. Persistent `/etc/storage/post_iptables_script.sh` and external `/opt/bin/update_iptables.sh` still need writer/deployment provenance and exit-contract audit; firewall restore error handling/rollback and IPv6 parity remain open. Whole userland audit is not complete.

## 2026-11-08 04:42 CST — Report firewall restore and extension-hook command failures

- Source/test checkpoint: implementation `79daaef39110c33078d747a017aefb6845ffbca6`; exact tested source/workflow SHA after a workflow assertion correction: `f7e3a3270a4f90ecd296bdb09313843d958d85af`. Exact-head GitHub Actions run `42412279665` completed SUCCESS; job `128522798720` passed all steps. The local GNU89 status unit test compiled with `-Wall -Wextra -Werror` and passed.
- Implemented `trunk/user/rc/src/firewall_command_status.h` plus `tests/firewall_command_status.c`. The status helper understands the wait status returned by `system()`, distinguishes success, exit code, terminating signal and error/unknown status, and is covered for normal success, non-zero exit, 127, signal and `-1`.
- Modified `firewall_ex.c`: wrap/check all 10 built-in IPv4/IPv6 restores (filter/default, mangle, raw, NAT/default as feature-guarded) and the Adbyby `iptables-restore -n` invocation; report failures from the Shadowsocks, persistent storage and Optware update hooks as well. Failure logs identify operation and distinguish system error/errno, exit code, signal or raw wait status. CI now compiles/runs the helper test and statically asserts all 11 restore invocations and 3 additional hooks are wrapped. This does not change normal command order or implement rollback.
- First run `42412253048` on `79daaef...` failed at the existing “Verify add-on firewall hook staging” step because it asserted the previous direct Shadowsocks `doSystem` spelling. That stale workflow assertion was corrected; final run `42412279665` on `f7e3...` is the authoritative SUCCESS. All final steps passed: AP migration, existing policy C tests, add-on hook staging, new command status test and static contracts, root-managed script staging, and HTTPD route/auth wiring.
- Completed source inventory of the persistent hook: `dev_init.sh` calls `mtd_storage.sh fill`; that script generates `/etc/storage/post_iptables_script.sh` only when missing and chmods the generated default to 0755. The HTTPD storage variable uses `EVM_RESTART_FIREWALL|EVM_BLOCK_UNSAFE`, and the common script writer requires a trusted session and sets mode 0755. The firewall invokes the executable path without args. Runtime dispatch does not independently validate file owner/type/link count/mode; consider this intentional admin root-extension behavior but keep restored-storage and runtime trust checks in the audit.
- Optware path: `opt-mount.sh` generates `/opt/bin/update_iptables.sh` only if absent, chmods that default 0755 and links `/opt/etc/init.d/S10iptables`; it binds `/opt` from the configured storage tree and `opt-start.sh` launches executable init.d entries. Firewall calls the external hook with `update`. Existing external files are not owner/mode-validated, and deployment filesystem ownership was not measured.
- Limits: this is host-level helper/workflow verification; CI did not compile production `firewall_ex.c` for MIPS. No firmware image, router runtime, live iptables restore, failure-injection on hardware or hardware validation. Errors are logged but firewall setup continues; multi-table rollback and IPv6 semantic parity remain open.
- Remote branches compared with tested SHA `f7e3...`: BStreamTag 1/531, DBAPI 8/612, dynlib 0/535, sorting 0/531, UArray 2/529 ahead/behind. Their ahead-only changes remain SkipDBv2-scoped; none merged.
- Next: determine the proper run-time trust contract for `/etc/storage/post_iptables_script.sh` and externally mounted `/opt/bin/update_iptables.sh`, then design/tests for partial restore failure and any feasible rollback, audit IPv6 semantic parity, and move to DNS/DHCP and WAN/PPP/service lifecycle. Whole-userland inventory remains IN PROGRESS; routine SkipDBv2 work stays paused.

## 2026-11-08 04:51 CST — Validate root extension hooks before firewall dispatch

### Baseline and CI
- Work resumed from the firewall restore/hook trust-boundary handoff. The production dispatch change was committed in `1010cef088825972272039103fc7ccafa7467ade`; interpreter/execute-mode compatibility was refined in `6a0a196b0ec7423b6c3108f102c6e809f74892d4`. The source-contract check was corrected in `e934796333ccf184b1bc3e0c1479abc065fb5bff` and finalized in `dd9326876226ebf7948710a66a38ee883d5fb576`.
- Exact-head GitHub Actions run [`42412978452`](https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42412978452), head SHA `dd9326876226ebf7948710a66a38ee883d5fb576`, completed SUCCESS. Job `128524878720` completed every step successfully.
- Intermediate runs `42412856071` (head `6a0a196b0ec7423b6c3108f102c6e809f74892d4`) and `42412921372` (head `e934796333ccf184b1bc3e0c1479abc065fb5bff`) failed in the command-status source-contract step: assertions expected old direct hook paths, then an over-escaped whitespace regex. The production/helper host test and preceding workflow steps passed; the final contract now checks multiline hook labels and staged-command fragments separately. Only the final exact-head PASS is authoritative.
  
### Finding and implementation
- The persistent `/etc/storage/post_iptables_script.sh` and externally managed `/opt/bin/update_iptables.sh` were root-dispatched after checking path existence, without independently validating the opened inode. The first hook is stored as an admin extension through the trusted HTTPD script writer; the second is on the mounted Optware storage tree. These are privileged extension points, not evidence of a proven unauthenticated exploit.
- Updated `trunk/user/rc/src/firewall_ex.c` to apply the existing `firewall_hook_file_stage()` trust gate to both paths. The helper is now included unconditionally instead of only under `APP_SHADOWSOCKS`. On a staging failure other than missing-file `ENOENT`, the firewall logs the failure and does not execute the source path.
- `trunk/user/rc/src/firewall_hook_file.h` opens the source with `O_NOFOLLOW | O_NONBLOCK`, validates the descriptor via `fstat` (regular file, effective-UID ownership, single link, no group/other write bits and size at most 1 MiB), checks the root-private directory, and copies from that descriptor to a unique staged file under `/home/root`. Execution uses the staged path and cleanup follows the command. The snapshot mode is `0600` for non-executable source and `0700` if the owner-execute bit is present; group/other permissions are never copied, and the original shebang/interpreter and Optware `update` argument remain supported.
- Expanded `trunk/user/rc/src/tests/firewall_hook_file.c` for group-write, other-write, FIFO and owner-executable staging behavior. Updated `.github/workflows/httpd-security.yml` to check the new dispatch contract without line-wrapping dependencies.

### Validation and limits
- Exact-head workflow PASS includes host C regressions built with `cc -std=gnu89 -Wall -Wextra -Werror`, helper execution tests, shell snapshot and AP migration regression, source contracts and HTTPD auth/route-access checks.
- L1 static review PASS; L2 focused host regression PASS. The workflow does not compile production `firewall_ex.c` into the firmware. Production RC/MIPS compilation, firmware image generation/validation, actual iptables restore, and router/hardware testing are NOT RUN.
- Still open: persistent multi-table restore failures are logged but not rolled back; investigate safe partial-failure/rollback semantics and compare IPv6 rule semantics. Whole-userland inventory remains IN PROGRESS, with DNS/DHCP and broader WAN/PPP/service lifecycle audits outstanding. SkipDBv2-only checkpoint changes were compared but not merged.

## 2026-11-08 05:07 CST — Log firewall rule-file staging failures and preserve errno

### Baseline and Actions
- Base: `dda496751d51324469249079c58c7ee1db28f5ca`; implementation/test/workflow commit: `af4cb4a10fdcf8989321a7c41df2f261df1544a0`.
- Exact-head Actions run [`42414162783`](https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42414162783), head SHA `af4cb4a10fdcf8989321a7c41df2f261df1544a0`. All executable test steps are marked SUCCESS, including compilation/execution of the policy regression and the firewall rule-file, hook-file, and command-status checks; the workflow run itself was still `in_progress` at the last poll because its final post-checkout step had not completed. Final conclusion remains PENDING; re-check the exact run before relying on it.
- Source changes were re-fetched from GitHub after commit. Static preflight confirmed 10 labelled open wrappers and 10 labelled finish wrappers, no direct unlogged rule-file calls remain in the 10 table writers, and the workflow asserts IPv6-specific restore/ICMPv6 wiring.

### Changes
1. `trunk/user/rc/src/firewall_rule_file.h`: `firewall_rule_file_finish()` now preserves the first meaningful flush/stream/close error across cleanup, supplies `EINVAL` for a null stream and `EIO` when an I/O failure has no usable errno, and still unlinks any incomplete rule file.
2. `trunk/user/rc/src/firewall_ex.c`: all 10 IPv4/IPv6 rule-table writers use operation-labelled wrappers that log failure to create the private staging file or flush/close it separately from the existing restore-command failure logger. An incomplete rule file is still never applied.
3. `trunk/user/rc/src/tests/firewall_rule_file.c`: added a regression that forces a descriptor/write-close failure, asserts a nonzero errno is preserved, and asserts the incomplete staged file is removed.
4. `.github/workflows/httpd-security.yml`: source contracts cover all 10 wrappers and assert the bounded IPv6 family checks, including `ip6tables-restore`, ICMPv6 type handling and the `ip6table_mangle` gate.

### Restore/IPv6 audit and validation boundaries
- Bounded static review compared the IPv4 and IPv6 filter/restore paths. The inspected IPv6 path uses protocol number 58 with `--icmpv6-type`, uses the IPv6 restore binary, and checks `ip6table_mangle`; family-specific differences such as DHCPv6 and the IPv6 forwarding/pinhole path remain intentional candidates for continuing rule-by-rule review. This is not a complete firewall semantic-parity certification.
- No automatic cross-table rollback was added. The startup path applies individual IPv4 and IPv6 tables with sequential restore commands. A correct rollback would need a coordinated snapshot/generation boundary across tables and add-on/dynamic rule writers; simply replaying stale snapshots can overwrite concurrent UPnP or service rule changes. Existing command failures remain logged per operation. Treat rollback/transactionality as unresolved pending a safe design, not as fixed.
- L1 static review/preflight: PASS. L2: the workflow's compile/test steps were SUCCESS at last poll, but overall run conclusion PENDING. L3 MIPS/firmware build and L4 router hardware validation: NOT RUN.

### Next action
Re-check run `42414162783` on exact SHA `af4cb4a10fdcf8989321a7c41df2f261df1544a0`. Do not add uncoordinated multi-table rollback. Close the bounded IPv6 restore-dispatch checks, then broaden to DNS/DHCP and WAN/PPP/service lifecycle instead of returning to SkipDBv2-only work.

## 2026-11-08 05:09 CST — Confirm exact-head firewall staging regression PASS

- **Source/test/workflow SHA:** `af4cb4a10fdcf8989321a7c41df2f261df1544a0`; source/test implementation was already committed, so this is a verification/governance checkpoint only.
- **Exact Actions result:** run [`42414162783`](https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42414162783), exact head SHA `af4cb4a10fdcf8989321a7c41df2f261df1544a0), completed **SUCCESS**. Job `128528514637` succeeded on every step, including `Compile and run policy regression`, add-on firewall hook staging, firewall command status reporting, root-managed executable staging, HTTPD auth/route-access wiring, checkout cleanup and job completion.
- This confirms host-side CI for the bounded changes from the preceding entry; it does **not** constitute a MIPS/firmware build, image validation or router test. Cross-table rollback remains intentionally unresolved pending a coordination design.
- The three project records are refreshed to replace the earlier accurate-at-the-time PENDING state with the completed exact-head result.
- **Next module:** DNS/DHCP/service startup lifecycle. SkipDBv2 checkpoint branches remain unmerged.

## 2026-11-08 05:17 CST — Harden DNSMasq runtime file staging; record compiler follow-up

### Finding and implementation
- Audited `trunk/user/rc/src/services_ex.c`, `net_wan.c` and the shared config/file helpers. Privileged DNSMasq setup used predictable `fopen(...,"w")` paths for `/tmp/hosts.static` and `/tmp/dnsmasq.servers`; `update_hosts_router()` ingests the static host file into `/etc/hosts`. Lease setup used `create_file("/tmp/dnsmasq.leases")`, which follows the final symlink and doesn't validate the opened inode.
- Commits:
  - `b3d9a48663ede904114c8b7587ea250aa49095b9`: unique temp staging, checked finalization + atomic publish for the static hosts and DNS server snapshots; safe lease-path helper and first host tests.
  - `b503f6a7bea848d314335dc5a8a2b5791ba986ea`: fail closed when no safe DNS server snapshot exists; stop WAN-triggered DNS reload if the snapshot is unsafe.
  - `e21332e41652794a47963e79cd714aa97fb58a06`: validate `/tmp/hosts.static` before reading it into router hosts.
  - `314a9b31df790d70b99be98334c26a46109f878f`: attempted to preserve restrictive existing lease modes; its exact-head Action exposed a compiler warning and FAILED, so that commit alone is not a verified checkpoint.
  - `c59a43645cca746cb7c8c6496b55ff4324ae5870`: corrected the warning by passing explicit `0644` creation-mode bits to `open()`, then computing final lease permissions only after `fstat()`.
- `dnsmasq_file.h` now stages replacements with `mkstemp`, preserves I/O/close errors, rejects unsafe file types/owners/link counts and publishes by rename. Existing lease-file permissions are narrowed/preserved as appropriate, retaining owner read/write without widening group/other permissions unnecessarily.
- `tests/dnsmasq_file.c` uses a disposable temp directory to cover: replacing a destination symlink without writing through its target; mode checks for staged and final files; flush/close failure cleanup while retaining the old published file; lease symlink/hardlink/FIFO rejection; existing content preservation; and 0600 permission preservation. `.github/workflows/httpd-security.yml` compiles with GNU89 `-Wall -Wextra -Werror` and verifies the DNSMasq and WAN fail-closed wiring.
- The earlier implementation Actions completed SUCCESS: `42414660244` (`b3d9a486...`), `42414820481` (`b503f6a...`) and `42414860072` (`e21332e...`). Run [`42414909753`](https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42414909753) on `314a9b31...` completed FAILURE at the DNSMasq regression step: `lease_mode` was used as the creation-mode argument before being initialized. The follow-up commit `c59a43645...` fixes that path.
- Exact-head run [`42414963649`](https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42414963649), on `c59a43645cca746cb7c8c6496b55ff4324ae5870`, was in progress at last poll. Do not assert this final correction passes until checking the run result.
- Verification limits: source/static checks passed for current code; L2 CI of the newest correction PENDING at checkpoint time. No MIPS target firmware build, final image or physical router test was run. The firewall multi-table rollback remains OPEN; there is no safe cross-table transaction design yet.

### Next action
Poll exact-head run `42414963649`; fix any compiler/regression failure. Then audit WAN/PPP client events, route/DNS update propagation and service start/stop ordering. Keep progress anchored to whole userland rather than returning to routine SkipDBv2 refactoring.

## 2026-11-08 05:19 CST — Confirm DNSMasq exact-head host CI PASS

- Re-checked run [`42414963649`](https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42414963649), exact head `c59a43645cca746cb7c8c6496b55ff4324ae5870`: **COMPLETED SUCCESS**. Every step passed, including `Compile and run policy regression`, `Verify DNSMasq generated file trust boundaries`, firewall hooks/status reporting, root-managed script staging and HTTPD auth/route-access contracts.
- The run validates the compiler correction to lease-file mode initialization. Earlier run `42414909753` failure is retained in historical notes; the corrected head is green, not a suppressed failure.
- Current main at this documentation update was `bab623b9a559799f3544eea33933a308152e990a`; this commit only refreshes governance status. No firmware/MIPS build or router test was performed.
- DNSMasq task is host-verified. Proceed to a bounded WAN/PPP/service lifecycle audit, especially DHCP event input validation, route application, and resolver update ordering.

## 2026-11-08 05:25 CST — Harden PPP credential file writes and verify exact-head CI

### PPP finding and implementation
- In `trunk/user/rc/src/net_ppp.c::launch_wan_pppd()`, the PPP options file contains clear-text `user`/`password` values. Existing code used `fopen(options, "w")`, wrote secrets, closed and only then applied `chmod(options, 0600)`; this left a disclosure window and did not validate a pre-existing final symlink/hardlink/FIFO.
- Code commit [`bd7d7fbbd69156cdd563579aaa85ee8762824719`](https://github.com/xiaobei206/padavan-4.4-2/commit/bd7d7fbbd69156cdd563579aaa85ee8762824719): add `trunk/user/rc/src/ppp_options_file.h` and use it from `net_ppp.c`. The helper repairs/verifies the root-owned `/tmp/ppp` directory, checks pre-existing file type/owner/link count and tightens old modes before opening, uses `O_NOFOLLOW | O_NONBLOCK`, confirms the opened descriptor is a regular single-link file owned by the effective uid, sets 0600 before truncating/writing, and treats flush/error/close failure as fatal while unlinking the partial credentials file.
- Regression file `trunk/user/rc/src/tests/ppp_options_file.c` covers directory permission tightening; converting a previous 0644 file to 0600 before writing a new secret; symlink/hardlink/FIFO refusal and target preservation; and forced flush/close error cleanup with a nonzero errno.
- Workflow adds the `Verify PPP credential-file confidentiality` step, compiling/running tests with `cc -std=gnu89 -Wall -Wextra -Werror` plus source contracts for the real call sites.
- The first exact-code Action, run [`42415603629`](https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42415603629), FAILED only in the Python source-contract section: an assertion expected the entire C `assert((st.st_mode...) == 0600)` token, whereas the actual condition was one part of `assert(stat(...) == 0 && (st.st_mode...) == 0600)`. The C compile/run step had succeeded before the contract step. Workflow-only commit [`73f921259959dbdf377943f46cb567051c83ef7c`](https://github.com/xiaobei206/padavan-4.4-2/commit/73f921259959dbdf377943f46cb567051c83ef7c) fixes the test to assert the meaningful mode expression rather than the incorrect full source substring. Exact-head Action [`42415669510`](https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42415669510) completed SUCCESS; every step passed, including the PPP C test, DNSMasq C test, firewall hooks/status, root-managed script staging and HTTPD auth/route checks.
  
### DNSMasq domain injection guard already verified
- Commit [`a2182bcdcf5a4069a9b5b3f924ce1f89fe474ab4`](https://github.com/xiaobei206/padavan-4.4-2/commit/a2182bcdcf5a4069a9b5b3f924ce1f89fe474ab4) added `dnsmasq_valid_server_domain()` and validation of the VPN-client and MAN domain strings before composing `server=/%s/%s`. Rejected strings include slash, invalid DNS label syntax and CR/LF-style directive-injection payloads.
- The first test literal escaped the newline too deeply; commit [`c81e0742a031e8fd722da522cb438d5b69f85e3c`](https://github.com/xiaobei206/padavan-4.4-2/commit/c81e0742a031e8fd722da522cb438d5b69f85e3c) corrected the C literal and made the workflow source check robust. Exact-head Actions run [`42415272701`](https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42415272701) passed every step, confirming tests exercise actual escaped newline input and slash payload.
- DNSMasq runtime staging prior to this entry is also host-verified: unique temporary files + atomic publish for `/tmp/hosts.static` and `/tmp/dnsmasq.servers`; safe lease file path/inode validation; checked close and last-known-good DNS snapshot behavior; validation before consuming static hosts. Exact-host checkpoints and the earlier compiler warning are documented above in the existing log history and were not overwritten.
- Firewall log/finalization checks for all ten IPv4/IPv6 rulesets have an exact-head host PASS (`42414162783`). No MIPS target build, firmware image, or physical-router test was performed. Firewall multi-table rollback is still intentionally unresolved pending a coordinated generation snapshot to avoid overwriting concurrent dynamic rules.
  
### Next bounded task
Continue WAN/PPP DHCP input and resolver/route lifecycle review, then choose the next highest-risk unaudited module from system management/network services. Keep the full userland audit active and keep historical failed Actions recorded honestly; don't fall back to routine SkipDBv2-only work.

## 2026-11-08 05:39 CST — Validate DHCP lease callbacks and reconcile stale WAN routes

### Scope and implementation
- Baseline was main `aaad71a9076fc5f7b683753f312056f806fd1788`, resuming the handoff's explicitly pending DHCP event/route/resolver ordering review. This is a real WAN/DHCP RC task; no SkipDBv2 checkpoint was merged.
- Added `trunk/user/rc/src/dhcp_lease_policy.h` and `tests/dhcp_lease_policy.c`. The host-testable policy checks canonical IPv4 addresses, contiguous non-zero IPv4 masks, bounded whitespace-delimited IPv4 lists, strict decimal ranges/overflow, and bounded route option text.
- `net_wan.c` now validates required DHCP IPv4/netmask input before applying it; normalizes router lists; gates DNS/WINS, MTU, lease lifetime, route options and DHCP domain; checks ZeroConf IPv4 input; and reconciles dynamic/default/host gateway routes across renew/deconfig paths. Lease renewal re-runs the bound path for route-option/MTU changes; DNS/domain changes trigger resolver/dnsmasq refresh.
- Follow-up `man_down()` removes the old MAN default route, off-subnet gateway/DNS host routes and static/dynamic routes before clearing lease-derived NVRAM state, then refreshes resolver state and requests WAN firewall refresh. IPTV DHCP deconfig retires its default/dynamic routes and clears route metadata.
- Code commits: [`ddd69bd4969fa95f6bd541f99f4ea1b31c8844e7`](https://github.com/xiaobei206/padavan-4.4-2/commit/ddd69bd4969fa95f6bd541f99f4ea1b31c8844e7) and [`5d531a09ec030265828b2ad33969904cc077f3c0`](https://github.com/xiaobei206/padavan-4.4-2/commit/5d531a09ec030265828b2ad33969904cc077f3c0).

### Validation and limits
- Exact-head GitHub Actions [run 42416794701](https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42416794701) completed SUCCESS on `5d531a09ec030265828b2ad33969904cc077f3c0`. The `Verify DHCP lease input and renewal reconciliation` step passed; its C regression compiles with `cc -std=gnu89 -Wall -Wextra -Werror`, and the source contract checks bound validation, renewal decisions, route cleanup, resolver refresh and MAN-down ordering. Existing PPP, DNSMasq, firewall and HTTPD checks in the same job also passed.
- The test compiles the standalone policy helper, not a production MIPS RC object. **Target MIPS build, firmware/TRX artifact validation and physical-router testing: NOT RUN.** Do not promote DNS/DHCP or WAN/PPP to fully audited: remaining DNS-daemon configuration/rebinding, PPP failover, static-route interactions and IPv6 state transitions are open.
- Repository-wide next step: select the next highest-risk unaudited module (file sharing, storage/hotplug or init/service lifecycle) and inspect its build manifest/init/callers. Keep routine SkipDBv2 expansion paused; firewall cross-table rollback and IPv6 policy parity remain separate open items.

## 2026-11-08 05:43 CST — Reject unspecified DHCP hosts and align CI event filters

### Follow-up findings and fixes
- Source review compared the new lease parser with the existing shared `is_valid_ipv4()` implementation: the latter treats `0.0.0.0` as `INADDR_ANY`, while the first DHCP helper revision only checked syntax. The policy header now separates dotted-quad text validation from host-address validation, rejects `0.0.0.0` for assigned addresses and router/DNS/WINS lists, and allows it only for a classless route with prefix length `/0`. Added regressions for zero host rejection, `/0` acceptance, and invalid nonzero-prefix zero route.
- Workflow review found the new `rc.h`, `dhcp_lease_policy.h` and `tests/dhcp_lease_policy.c` path filters had landed only in `push.paths`. Added the identical entries to `pull_request.paths`; extracted lists now match exactly (51 entries each, no push/PR path differences). The workflow-only trigger correction commit [`8daa47c339c26e34e950ef36d4815d9c542fdd47`](https://github.com/xiaobei206/padavan-4.4-2/commit/8daa47c339c26e34e950ef36d4815d9c542fdd47) passed exact-head run [42417046785](https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42417046785).
- Semantic follow-up source/test/workflow commit [`048b540edbfc418b6f1bb8053692425f18b86d66`](https://github.com/xiaobei206/padavan-4.4-2/commit/048b540edbfc418b6f1bb8053692425f18b86d66) passed exact-head run [42417162009](https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42417162009); all job steps passed, including the GNU89 `-Wall -Wextra -Werror` standalone C policy regression and DHCP source-wiring contracts.
- Residual verification boundary unchanged: no production MIPS RC build, firmware/TRX validation or physical-router test. The broader DNS package/rebinding, PPP failover and IPv6 state-machine audit remains open.
- Latest checkpoint-branch comparison against current main: BStreamTag 1/557 ahead/behind, DBAPI result 8/638, dynlib lifecycle 0/561, sorting 0/557, and UArray character 2/555. Ahead-only file differences remain SkipDBv2 source/workflow plus branch log material; no checkpoint was merged.
- Next: continue the remaining P1 firewall restore failure/rollback and IPv6 policy gaps, then rotate to FTP/Samba, USB/storage/hotplug and init/service lifecycle instead of routine SkipDBv2 expansion.

## 2026-11-08 05:52 CST — Isolate HTTPD privileged upload and storage-restore staging

### Findings and implementation
- **Verified source/test/workflow SHA:** `f39ec03b4b9c898151048f4244276136d2c1fe33`; exact-head Actions run [`42417878853`](https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42417878853) completed SUCCESS. Documentation commits after this code checkpoint do not alter the tested source/workflow files.
- `trunk/user/httpd/src/upload.c::do_upload_file()` deleted a fixed-name output and opened it later using `fopen(fn, "w+")`, allowing a local shared-`/tmp` symlink race to redirect a privileged write. New `httpd_upload_file.h` uses `open(O_CREAT | O_EXCL | O_NOFOLLOW)`, enforces file mode 0600 via `fchmod`, and uses `fdopen` on the returned descriptor. The uploader no longer opens its target with `fopen`.
- Moved HTTPD NVRAM-upload/download, storage archive and firmware-image staging into `/tmp/.httpd-private/`. In `dev_init.sh`, explicitly set `/tmp` sticky mode 1777 and create `/tmp/.httpd-private` mode 0700 before the storage initialization path. This keeps potentially large firmware uploads on the existing tmpfs rather than moving them to the 1 MiB `/home` tmpfs.
- In `mtd_storage.sh::func_restore()`, extracted files now go into a uniquely allocated mode-0700 private directory through a bounded `mkdir` loop; the operation quotes path arguments, rejects scratch-allocation exhaustion, and copies the extracted directory contents into `/etc/storage`. Storage tar, compressed archive, lock marker and hash staging are also under the private directory.
- Added `trunk/user/httpd/src/tests/httpd_upload_file.c`: a planted target symlink must be refused without altering the victim; a normal new upload must produce a regular mode-0600 file. Workflow triggers include the helper/header/shared board macro on both push and PR (the path sets compare equal), and CI runs `sh -n` against `mtd_storage.sh` and `dev_init.sh`.

### Verification and correction history
- Exact-head run [`42417878853`](https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42417878853), head `f39ec03b4b9c898151048f4244276136d2c1fe33`, passed its complete job. This included GNU89 `-Wall -Wextra -Werror` compile/run of the symlink/file-mode regression, source-contract assertions and both shell syntax checks.
- Earlier runs `42417600632` and `42417688730` exposed a test-script `NameError`: the restore source slice was referenced before it was defined. Fixed by `515223ee4f0c9178662e06b67a6bd1879fd47a9a`; exact-head run `42417796130` passed. The follow-up `sh -n` gate committed as `f39ec03b4b9c898151048f4244276136d2c1fe33` then passed run 42417878853.
- Implementation history: `226f20b880a557004d1e55a20d37717517fb8aa1` first added exclusive creation and restore staging; `7211a6be9067751bae7a94f30c5c4d6af93a938d` relocated staged paths to private tmpfs because `/home` is only 1 MiB. Subsequent workflow-only commits fixed the contract test and added shell syntax checks.
- **Not run:** production MIPS build, firmware/TRX validation and physical-router test. No claim is made that all archive-member semantics, HTTPD/TLS runtime paths or alternate writers are fully audited.

### Remaining scope
- Updated `SESSION_HANDOFF.md` and `REFACTOR_ROADMAP.md` with the exact-head checkpoint. HTTPD/archive restore remains P1 AUDITING, system init/service lifecycle remains AUDITING, and FTP/Samba plus USB/storage/hotplug remain NOT AUDITED.
- Five `refactor/*` checkpoint branches remain far behind current main with their ahead-only changes restricted to SkipDBv2/CMake/test/workflow/dev-log areas; none was merged.
- **Next:** continue remaining P1 firewall restore/error and IPv6 policy gaps, then rotate across FTP/Samba, USB/storage/hotplug and init/service lifecycle. Do not restrict the project to SkipDBv2.

## 2026-11-08 06:09 CST — Validate firewall interface names before restore-token generation

### Finding and implementation
- **Source/test/workflow checkpoint:** `54fdde79391504ff34e67dcbe2a2e5a53436609b` on `main`; exact-head Actions run [`42419220664`](https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42419220664) completed **SUCCESS**.
- In `trunk/user/rc/src/firewall_ex.c::start_firewall_ex()`, runtime MAN/WAN interface names are inserted into bare `-i`/`-o` arguments in IPv4/IPv6 restore rule files. No common raw-token validation was applied at the generation boundary. This round treated it as an input-integrity hardening gap; it did not establish that a remote attacker can directly control those runtime values.
- Added `firewall_interface_name_is_safe()` to `firewall_filter_policy.h`. It accepts 1–15 ASCII bytes, requires an alphanumeric first character, accepts alphanumeric/underscore/dot/colon/hyphen thereafter, and retains only a final `+` wildcard for iptables interface-prefix matching. Control bytes, whitespace, separators, option-like prefixes and overlong names are rejected.
- `start_firewall_ex()` validates nonempty MAN and configured WAN values before copying them into fixed-size arrays, checks the WAN fallback and LAN interface before table generation, and validates MAN again when `i_use_man` makes it active. Invalid names are logged without echoing raw input and the rebuild returns before any rule-table writer can run.
- Added host regression cases in `trunk/user/rc/src/tests/firewall_filter_policy.c` for common wired/PPP/Wi-Fi names, interface alias and trailing-wildcard compatibility, null/empty/overlong inputs, option-like first characters, slash/semicolon/space payloads and a CR/LF-style restore-line injection payload. The workflow adds source-contract assertions proving the source guards precede `ipt_raw_rules()`; changed source/test files were already covered by both workflow event path filters.

### Validation and limits
- Exact-head Actions run [42419220664](https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42419220664) completed SUCCESS. The GNU89 `-Wall -Wextra -Werror` policy regression compiled and ran, and all source-contract / existing regression steps passed.
- **L1 static/source-contract and focused L2 host validation: PASS.** Production `firewall_ex.c` MIPS compilation, firmware/TRX build and validation, live `iptables-restore`/`ip6tables-restore` exercise and physical-router testing were **NOT RUN**.
- Firewall remains P1 **AUDITING**. Alternative add-on restore producers, interface provenance beyond this guard, restore-failure recovery, cross-table rollback and independent IPv4/IPv6 semantic parity remain open. Do not infer remote exploitability or whole-module closure from this bounded fix.
- Remote SkipDBv2 checkpoint branches were compared against the tested source SHA: BStreamTag 1/566 ahead/behind; DBAPI result 8/647; dynlib lifecycle 0/570; sorting 0/566; UArray character 2/564. Ahead-only deltas remain SkipDBv2/CMake/test/workflow/dev-log work; none was merged.
- **Next bounded module:** audit FTP/Samba/Aidisk authentication, path normalization, permissions, privileged shell calls, share management and lifecycle; then USB/storage/hotplug and init/service management. Keep firewall rollback and IPv6 parity tracked as separate open work, and do not restrict future rounds to SkipDBv2.

## 2026-11-08 06:23 CST — Validate FTP/Samba generated-configuration tokens

### Finding and implementation
- **Tested source/test/workflow checkpoint:** `309fa56ce881c1c3b1bea755c057acdbb73b3fa9`; exact-head GitHub Actions [run 42419984033](https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42419984033) completed SUCCESS. Implementation commit: `d46cc92f61dcda300307d3a2eb0b119a41a1d7b8`; workflow contract correction and tested head: `309fa56ce881c1c3b1bea755c057acdbb73b3fa9`.
- Audit rotated from the prior firewall boundary into the explicitly open FTP/Samba/file-sharing module. In `services_stor.c::write_smb_conf()`, removable-storage mount names, folder labels, disk tags and account tokens were written directly into Samba section/directive lines. Control bytes in a media-derived name could create additional lines; section delimiters and malformed ACL token delimiters could change configuration semantics. This review establishes the missing output validation at those sinks, not a proved remote execution path.
- Added `samba_share_policy.h` with C89 policy checks for config text, share/path components and account tokens. The Samba generator now checks `/media/` mount paths and safe section names before emitting either whole-partition or per-folder shares; skips unsafe metadata with a fixed message; validates usernames before generating all four ACL lists, setting the guest account, and passing an account name to `smbpasswd`. The FTP banner similarly validates `productid` before emitting it as a `vsftpd.conf` value, with a fixed fallback.
- Added `tests/samba_share_policy.c`: valid ordinary/UTF-8 names, null/empty values, control/CRLF/newline/backslash/percent payloads, section/path traversal and length limits, mount-root/component constraints, and ACL username separators/options. The workflow compiles and runs it using `cc -std=gnu89 -Wall -Wextra -Werror`, then checks source-to-sink guard ordering. Both push and PR filter lists include the relevant paths (57 entries per list, identical path sets).
- **CI debugging trail:** initial exact-head run [42419930875](https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42419930875) failed at the new Python source-contract check because the assertion searched for `/* ... */` although the existing source comment began with `//`; the C policy binary had already compiled and passed. Commit `309fa56ce881c1c3b1bea755c057acdbb73b3fa9` corrected the assertion to match the actual source marker. Exact-head run [42419984033](https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42419984033) then passed all 15 job steps.

### Validation limits and next actions
- **L1 source review and focused L2 host-policy/source-contract validation: PASS.** No production MIPS compile of `services_stor.c`, target firmware/TRX build, live service test or physical-router test was run.
- Module remains **P1/P2 AUDITING**. Open items include FTP anonymous/authentication modes, write permissions, writable chroot and `nopriv_user=root`, Samba ACL correctness/account lifecycle and persistent custom config, NFS `no_root_squash`, plus other service restart/failure paths.
- Compared remote checkpoint refs to `309fa56ce881c1c3b1bea755c057acdbb73b3fa9`: BStreamTag 1/569 ahead/behind; DBAPI result 8/650; dynlib lifecycle 0/573; sorting 0/569; UArray character 2/567. Ahead-only deltas are confined to SkipDBv2 code/CMake/tests/workflow/log; none was merged.
- **Next:** continue FTP/Samba privilege-drop, anonymous-mode, credential and ACL review with compatibility evidence; then USB/storage/hotplug and init/service lifecycle. Keep firewall cross-table rollback and IPv6 policy parity open. No routine SkipDBv2-only pivot.

## 2026-11-08 06:26 CST — Fail closed on invalid FTP share modes and stop root workers

### Finding and implementation
- **Source/test/workflow checkpoint:** `c258be09506ef66523924d3ed618bf6d5d3c7ae5`; exact-head GitHub Actions [run 42420569075](https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42420569075) completed SUCCESS.
- `trunk/user/rc/src/services_stor.c::write_vsftpd_conf()` consumed `st_ftp_mode` without checking it. The UI exposes only 1, 2, 3 and 4, but invalid values entered the fallback branch that explicitly enabled anonymous login. Added `ftp_share_policy.h::ftp_share_mode_normalize()`: supported modes are unchanged, while zero/negative/out-of-range values fail closed to mode 2 (local authentication enabled, anonymous login disabled).
- The generated `vsftpd.conf` had `nopriv_user=root`, negating the purpose of the daemon's totally unprivileged worker identity. It now uses `nobody`, an existing firmware account emitted as UID/GID 99 with `/bin/false` and locked in shadow; the bundled vsftpd default also names `nobody`. This is a material improvement over root, but upstream documentation recommends a dedicated service account rather than sharing `nobody`, so that remains a follow-up.
- Inspected the semantics of `allow_writable_root=YES` in `trunk/user/vsftpd/src/secutil.c`: this custom tunable bypasses refusal to chroot when the chroot root is writable by the unprivileged account. It was deliberately left unchanged pending actual removable-media permission/runtime compatibility evidence. FTP anonymous-write modes and `anon_umask=000` / `local_umask=000` remain open rather than being changed blindly.
- Added `trunk/user/rc/src/tests/ftp_share_policy.c`, covering all four supported modes plus 0, -1, 5, `INT_MIN` and `INT_MAX`. Updated `.github/workflows/httpd-security.yml` to compile/run the regression with `cc -std=gnu89 -Wall -Wextra -Werror`, assert production wiring and the non-root identity, and include the two new paths in both push and pull-request filters. The event path lists contain 59 identical entries.

### Verification and limits
- Exact-head Actions run [42420569075](https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42420569075) is **SUCCESS** on source/test/workflow SHA `c258be09506ef66523924d3ed618bf6d5d3c7ae5`. The complete job finished successfully, including policy compilation/execution, the FTP/Samba source contract and all existing HTTPD/DNS/DHCP/PPP/firewall regressions.
- A standalone local GNU89 `-Wall -Wextra -Werror` harness for the normalization policy compiled and ran successfully. Source-contract review confirms supported mode values normalize before mode-specific config generation and the generated nopriv user is non-root.
- **Not run:** production MIPS RC build, firmware/TRX build or inspection, live vsftpd/Samba runtime, removable-filesystem permissions or physical-router testing. This does not close the FTP/Samba module.
- Remote checkpoint branches compared with the tested SHA: BStreamTag 1/571 ahead/behind; DBAPI result 8/652; dynlib lifecycle 0/575; sorting 0/571; UArray character 2/569. Ahead-only changes remain in SkipDBv2/build/test/workflow/dev-log scope; none was merged.

### Next bounded task
- Continue FTP/Samba account/password lifecycle, anonymous-write/chroot/umask compatibility and the dedicated unprivileged service identity decision. Then rotate to USB/storage/hotplug and init/service lifecycle. Firewall cross-table rollback and IPv6 semantic parity remain separate open tasks; routine SkipDBv2-only development stays paused.

## 2026-11-08 06:41 CST — Keep Samba guest shares off the root identity

### Finding and implementation
- **Source/test/workflow SHA:** `966efa0380ffaa2ce18c702f0d2131e9e10b492d`; implementation commit `92bc47073d64f90c097ff7e8f2ce762eb878292b`, followed by the workflow source-contract correction at `966efa0380ffaa2ce18c702f0d2131e9e10b492d`.
- In `trunk/user/shared/src/shutils.c::recreate_passwd_unix(0)`, the configured NVRAM `http_username` is emitted as UID/GID 0. The Samba public modes in `trunk/user/rc/src/services_stor.c::write_smb_conf()` set `guest ok = yes` and `guest only = yes`, and had assigned `guest account` to this management username. Unauthenticated SMB guest requests could consequently be mapped to the root account. This is a generated-configuration privilege boundary finding; no live exploit was attempted.
- Added `SAMBA_GUEST_ACCOUNT "nobody"` to `samba_share_policy.h` and replaced the management-username mapping in the generated guest configuration with that fixed account. The tracked account generator creates `nobody` as UID/GID 99, `/bin/false`, and a locked shadow user. A focused test now asserts the fixed account identity, and the workflow checks the generated guest branch does not reference `http_username`.
- Changed paths: `trunk/user/rc/src/services_stor.c`, `trunk/user/rc/src/samba_share_policy.h`, `trunk/user/rc/src/tests/samba_share_policy.c`, `.github/workflows/httpd-security.yml`. Existing push/pull-request path filters already covered all four paths.

### Verification and limits
- Exact-head GitHub Actions run [42421747255](https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42421747255) completed **SUCCESS** on `966efa0380ffaa2ce18c702f0d2131e9e10b492d`; all 15 steps passed. Samba and FTP standalone policy tests compiled with `cc -std=gnu89 -Wall -Wextra -Werror` and both printed PASS; source-contract checks and the existing HTTPD/DNS/DHCP/PPP/firewall workflow checks also passed.
- Debug trail: prior exact-head run [42421698737](https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42421698737) failed because the newly added Python assertion over-escaped the expected backslash spelling. The C regression binaries had already passed. The workflow assertion was corrected in `966efa0380ffaa2ce18c702f0d2131e9e10b492d`, and the exact-head run above passed.
- **L1 static/source review: PASS. L2 focused host policy regression and source-contract workflow: PASS.** The workflow does not compile the complete production `services_stor.c` for MIPS. Production RC/MIPS compile, firmware/TRX build/inspection, live Samba login/share/write-permission checks and physical-router testing were **NOT RUN**.
- **Compatibility/residuals:** using UID 99 rather than UID 0 intentionally removes anonymous guest root privileges; read/write behavior on media with ownership/modes that relied on root access may differ and requires target testing. FTP dedicated identity, writable chroot/anonymous-upload and umask semantics, Samba ACL/password lifecycle, custom `/etc/storage/smb.conf`, and NFS export semantics remain open. Whole FTP/Samba and storage lifecycle are not closed by this bounded change.
- Remote checkpoint comparison against tested SHA `966efa0380ffaa2ce18c702f0d2131e9e10b492d`: BStreamTag 1/573 ahead/behind; DBAPI 8/654; dynlib 0/577; sorting 0/573; UArray 2/571. Ahead-only deltas remain SkipDBv2-only and none was merged.

### Next bounded task
- Continue FTP/Samba account/password provisioning, ACL semantics, custom configuration loading and FTP chroot/anonymous-write compatibility; then rotate to USB/storage/hotplug and init/service lifecycle. Keep firewall cross-table rollback and IPv6 policy parity as separate open tasks; routine SkipDBv2-only work stays paused.


## 2026-11-08 06:53 CST — Unify storage-account identity checks across Unix and Samba consumers

### Finding and implementation
- **Code/test/workflow SHA:** `9702c96d355e712aad876b6371933974608bd96d`; exact-head GitHub Actions [run 42422590307](https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42422590307) completed **SUCCESS** with all 16 steps passed.
- Continued the open P1/P2 FTP/Samba file-sharing audit. The account lifecycle spans libdisk/NVRAM, `recreate_passwd_unix()` passwd/group materialization, Samba ACL list generation and `smbpasswd` provisioning. Their identity policies were not aligned: account create/rename accepted names colliding with system identities or the active management username; Samba ACL/provisioning guards did not consistently reject the system accounts `root`, `nobody`, `sshd` and `anonymous`. Separately, raw NVRAM account slots used by passwd/group materialization did not receive the same common byte-level syntax validation or duplicate suppression.
- This is a privileged identity-consistency/defense-in-depth finding. This round did **not** demonstrate that an unauthenticated HTTP request can directly write arbitrary NVRAM or exploit these paths.
- Added `trunk/user/shared/src/include/router_account_policy.h`: locale-independent ASCII account names, 1–31 bytes, only alphanumeric/`_`/`-`/`.`, excluding `.` and `..`; reserved identity predicate for `root`, `nobody`, `sshd` and the current management username.
- `trunk/user/libdisk/src/disk_share.c` now uses the shared syntax helper and rejects reserved identities in `add_account()` and as the destination of `mod_account()` (including the existing FTP anonymous restriction). `trunk/user/shared/src/shutils.c::recreate_passwd_unix()` validates raw NVRAM account slots before passwd/group output, excludes system/management/anonymous names and suppresses duplicate rows. `samba_share_policy.h` uses the common policy for ACL names; `services_stor.c` additionally filters the current `http_username` before ACL generation and `smbpasswd` provisioning.
- Added `trunk/user/shared/src/tests/router_account_policy.c` for valid/invalid names, null/empty, delimiter/control/backslash/percent/UTF-8 inputs, length limits and reserved identities. Expanded `trunk/user/rc/src/tests/samba_share_policy.c` to reject `root`, `nobody`, `sshd` and `anonymous`. Workflow compiles/runs both standalone policy binaries with GNU89 `-Wall -Wextra -Werror`, checks source wiring and includes changed code/header/test paths in both push and pull-request filters.
- **Changed paths:** `.github/workflows/httpd-security.yml`; `trunk/user/libdisk/src/disk_share.c`; `trunk/user/shared/src/shutils.c`; new shared account-policy header and C regression; `trunk/user/rc/src/samba_share_policy.h`; `trunk/user/rc/src/services_stor.c`; expanded Samba policy regression.

### Verification and limits
- Exact-head Actions run 42422590307 is **SUCCESS** on `9702c96d355e712aad876b6371933974608bd96d`. All 16 job steps passed, including the new account-identity test step and existing HTTPD/DNSMasq/DHCP/PPP/firewall/script tests. The two standalone C policy tests compiled and ran with `-std=gnu89 -Wall -Wextra -Werror`; source-contract and push/PR trigger checks passed.
- **L1 source review: PASS. L2 focused host policy regression/source contracts: PASS.** No complete production libdisk/shared/RC MIPS compile, firmware/TRX build or inspection, live SMB/FTP account test, storage-permission compatibility test or physical-router test was run.
- Reserved legacy NVRAM account records are not automatically migrated or deleted; rename/delete recovery and already-provisioned Samba passdb cleanup require separate review. Existing FTP service identity, writable chroot/anonymous-write/umask behavior, ACL correctness and persistent custom `/etc/storage/smb.conf` also remain open. Do not treat the FTP/Samba module as closed.
- **Remote checkpoint comparison against code/test/workflow SHA 9702c96d355e712aad876b6371933974608bd96d:** `refactor/bstream-tag-wire` 1 ahead / 583 behind; `refactor/dbapi-result-20261105` 8 / 664; `refactor/dynlib-lifecycle` 0 / 587; `refactor/sorting-boundaries` 0 / 583; `refactor/uarray-character-ctype` 2 / 581. All ahead-only deltas remain confined to SkipDBv2 code/build/tests/workflow/dev-log work; none was merged into this userland security task.

### Next bounded task
- Inspect SMB/FTP password handling, credential exposure through argv/logging, `smbpasswd` return-code handling, stale passdb cleanup on account rename/delete, ACL semantics and user-maintained `/etc/storage/smb.conf` precedence. Preserve compatibility questions around writable chroot/umasks until removable-media evidence exists. Then rotate to USB/storage/hotplug and init/service lifecycle; retain firewall rollback/IPv6 parity as separate open items, and keep routine SkipDBv2-only work paused.

## 2026-11-08 07:15 CST — Remove Samba plaintext argv credentials and rebuild passdb atomically

### Finding and implementation
- **Code/test/workflow SHA:** `819d195594394fb9e17e243db957b215db0f7fd1`; exact-head GitHub Actions [run 42424372958](https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42424372958) completed **SUCCESS** with all 16 job steps passed.
- Resumed the exact open item from the preceding storage-account identity checkpoint. `services_stor.c::run_samba()` placed the NVRAM plaintext password in the `smbpasswd` argv and ignored `_eval()` status. The custom Samba 3.6 `owrt_smbpasswd` shim accepted `USERNAME PASSWORD` on the command line, used a default-mode passdb file, and created an unnecessary `strdup()` copy. The cold-start cleanup also removed `/etc/samba/*`, which included the passdb.
- Added `trunk/user/rc/src/samba_passdb_policy.h` and `tests/samba_passdb_policy.c`: rejects null/empty, 128-byte-or-longer, C0 and DEL values while preserving the existing 127-byte limit and UTF-8 byte sequences.
- `services_stor.c` now feeds passwords through `smbpasswd -s` stdin using an exclusive 0600 temporary file descriptor under /tmp; the parent unlinks the pathname immediately after fork, so the plaintext is absent from argv; the next checkpoint moves unlink before fork to close the possible child-before-unlink scheduling window. It checks `waitpid()` and the child's exit status, rejects duplicate NVRAM account identities, suppresses service reload/start on provisioning failure, and stops when `write_smb_conf()` fails. The `/etc/samba` directory is verified as a root-owned non-symlink directory and forced to 0700. Cold-start cleanup preserves `smbpasswd` and removes only TDB and abandoned hidden staging files.
- For `APP_SMBD36`, the service provisions the complete current account set into a new 0600 file in the same private directory and atomically renames it to `/etc/samba/smbpasswd` only after all valid users succeed. A successful rebuild removes stale passdb rows after account rename/delete; a failed rebuild unlinks the candidate and preserves the previous database. The custom Samba 3.6 shim patch now accepts `-s [-f PASSDB] USERNAME`, rejects the old positional plaintext syntax, reads and compares two bounded/control-free stdin lines, clears its password buffers, uses a 0600 passdb file under `umask(077)`, checks write/flush/close errors and avoids the previous `strdup()` leak.
- Updated `.github/workflows/httpd-security.yml`: both push and pull-request filters include the new policy/test and the modified Samba patch; the job compiles/runs the new C regression and checks the credential, passdb, exit-status and source-wiring contracts.

### Verification and limits
- Local host command `cc -std=gnu89 -Wall -Wextra -Werror` compiled and ran the passdb password-policy regression: **PASS**.
- Exact-head Actions run [42424372958](https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42424372958) on `819d195594394fb9e17e243db957b215db0f7fd1`: **SUCCESS**, all 16 steps. The new policy regression, Samba share/FTP checks, account identity checks and existing HTTPD/DNS/DHCP/PPP/firewall/script checks passed.
- **L1 source review: PASS. L2 focused host policy regression/source-contract CI: PASS.** The complete production `services_stor.c` RC/MIPS object was not compiled. The modified Samba 3.6 patch queue was not applied to a freshly extracted Samba 3.6.25 source tree in this round. Production firmware/TRX build and inspection, live SMB authentication, argv/process-list inspection, ACL/share permission tests, removable-media compatibility and physical-router testing were **NOT RUN**.
- **Compatibility/residuals:** the custom Samba 3.6 shim no longer accepts the previous positional `smbpasswd USERNAME PASSWORD` form; in-repository service provisioning uses `-s`, but external scripts or manual invocations were not exhaustively audited. Atomic complete-table replacement is currently conditional on `APP_SMBD36`; other Samba variants use stdin and check child status but can still partially update credentials before a later account fails. FTP dedicated identity, anonymous-write/chroot/umask compatibility, Samba ACL inheritance, persistent `/etc/storage/smb.conf` precedence, NFS export semantics and whole FTP/Samba closure remain open.
- Remote checkpoints compared with tested code SHA `819d195594394fb9e17e243db957b215db0f7fd1`: BStreamTag 1 ahead / 586 behind; DBAPI result 8 / 667; dynlib lifecycle 0 / 590; sorting 0 / 586; UArray character 2 / 584. Ahead-only files remain in SkipDBv2 code/CMake/tests/workflow/dev-log scope; none was merged.

### Next bounded task
- Inspect Samba ACL/inheritance semantics and how user-maintained `/etc/storage/smb.conf` is loaded, merged or overridden. Then check non-APP_SMBD36 passdb lifecycle and the external command compatibility surface. Continue with FTP dedicated identity and chroot/anonymous-write/umask compatibility, followed by NFS exports. Rotate afterward to USB/storage/hotplug and init/service lifecycle; keep firewall rollback and IPv6 parity open, and keep whole-userland scope broader than SkipDBv2.

## 2026-11-08 07:18 CST — Unlink Samba password input before forking the child

### Review correction
- **Code/test/workflow SHA:** a389e3feb19e3623c11326f1b35fa4bc60b55392; exact-head GitHub Actions [run 42424599053](https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42424599053) completed **SUCCESS**, all 16 steps passed.
- A final race review found that the prior commit unlinked the password-input path in the parent immediately after fork. Although the file was mode 0600, the child could theoretically run exec before the parent removed the name. Moved unlink(input_path) to after writing/seeking but **before fork**; the open file descriptor remains valid across unlink and is inherited by the child. The child also avoids closing the descriptor when mkstemp() returns STDIN_FILENO.
- Added workflow source-contract checks proving unlink occurs before fork and the child preserves fd 0. The full 16-step exact-head Actions job passed, including the standalone passdb policy test and all prior HTTPD/DNS/DHCP/PPP/Samba/firewall/script checks.

### Verification and limits
- **L1 source review: PASS. L2 focused host policy regression and source-contract Actions: PASS.** The earlier local GNU89 policy test remains PASS; exact-head run 42424599053 passed all 16 steps.
- This correction was not a live runtime test. Production RC/MIPS compilation, Samba 3.6 patch application against an extracted upstream tree, firmware/TRX build, process-list inspection, SMB authentication/ACL/media-permission tests and physical-router validation were **NOT RUN**.
- No credential value is placed in argv. The pathname is now removed before child creation; the private descriptor is the child's stdin input channel.

### Next bounded task
- Continue ACL semantics/inheritance and /etc/storage/smb.conf load/merge precedence. Then audit the non-APP_SMBD36 passdb path, external smbpasswd callers, FTP dedicated identity/anonymous-write/chroot/umask compatibility and NFS export semantics. Rotate afterward to USB/storage/hotplug and init/service lifecycle; keep firewall rollback/IPv6 parity open and do not return to routine SkipDBv2-only work.

## 2026-11-08 07:22 CST — Fail closed on Samba config generation and validate global config inputs

### Finding and implementation
- **Base main SHA:** `0f44a88f2944c3795360a24370ef3dbd44460f0d`. This resumes the Samba ACL/configuration work explicitly left open by the previous a389e3 handoff; no SkipDBv2 branch changes were merged.
- Audited the actual Samba service path: `write_smb_conf_header()` always regenerates `/etc/smb.conf`; `write_smb_conf()` emits dynamic partition/folder ACLs; `run_samba()` already checks the writer's return value before updating daemons. `/etc/storage/smb.conf` was not referenced by these generators, the inspected RC dispatch path, or `mtd_storage.sh`; no supported merge/override precedence was found, so no unsafe include was added. The existing file, when present, is not loaded by the inspected service path.
- Reviewed libdisk permission values: 0=deny, 1=read, 2=write, 3=read/write. Missing permission files produce 0; malformed/lookup errors produce -1. Samba's write list intentionally overlaps read list for writers; Samba 3.6 gives write-list membership precedence, so the overlap is not itself an authorization defect. The relevant Samba 3.6 behavior is documented by its smb.conf manual.
- **Confirmed defect:** `write_smb_conf()` returned success unconditionally at `confpage`, including when disk discovery returned NULL, account-list validation failed, a later append-open failed, or stream close reported an I/O error. This defeated `run_samba()`'s fail-closed check and could reload an incomplete configuration after a prior truncation.
- **Fix:** propagate failure for required disk/account discovery, append open, and stream close; check the close before reopening for append; return the actual writer result. Add explicit tested helpers for ACL right-to-read/right-to-write semantics and use them at guest/user list emission. Document the intentional Samba 3.6 read/write-list overlap. Added a shared config-value policy and apply it before emitting NVRAM workgroup and hostname values in the global section, rejecting control bytes, DEL, backslash and percent substitution.
- **Changed paths:** `trunk/user/rc/src/samba_config_value_policy.h`, `trunk/user/rc/src/samba_share_policy.h`, `trunk/user/rc/src/services_ex.c`, `trunk/user/rc/src/services_stor.c`, `trunk/user/rc/src/tests/samba_share_policy.c`, `.github/workflows/httpd-security.yml`; plus `REFACTOR_ROADMAP.md`, `SESSION_HANDOFF.md`, and `dev_log.md`.

### Validation and limits
- L1 source review: PASS for the scoped generator/ACL/config-loading paths.
- L1 source review PASS. L2 focused host policy regression and exact-head Actions PASS on code/test SHA `70da64b461dd17d431d8da1ee693c8b1eb26ddb7`, run [42425349410](https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42425349410). The final guest ACL-predicate follow-up is verified separately on SHA `389f62001b607d70c0f4ba3906a830d4c0000ed0`, run [42425408455](https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42425408455).
- Full production RC/MIPS compilation, Samba 3.6 patch application to a fresh upstream tree, firmware/TRX build, live SMB authentication/ACL/permission inheritance tests, filesystem compatibility, and physical-router validation: NOT RUN.
- **Remaining:** Samba account lifecycle for non-APP_SMBD36 variants, guest media permission compatibility and filesystem ACL inheritance, external smbpasswd callers, FTP dedicated identity/chroot/umask/write behavior, and NFS export policy (including no_root_squash) remain open. HTTPD/CGI/authentication, firewall rollback/IPv6 parity, USB/storage/hotplug and init/service lifecycle remain separate repository-wide risks.

### Next bounded task
- Continue FTP/Samba residuals only if necessary to close the file-service audit; then rotate to USB/storage/hotplug and init/service lifecycle. Keep P1 HTTPD/CGI/auth and firewall rollback/IPv6 parity in the top-level ledger; do not return to routine SkipDBv2-only work.

## 2026-11-08 07:24 CST — Use the shared ACL grant predicate for every guest decision

- Final source review after the first Samba config commit found one remaining direct `guest_right >= 1` branch on the `guest ok` directive. It was logically equivalent for the validated libdisk range but bypassed the new explicit helper and weakened the source-contract guarantee. Replaced it with `samba_acl_right_can_read(guest_right)`; the workflow now rejects remaining raw guest/user right-threshold comparisons inside the config writer.
- This is a consistency/verification follow-up, not a newly discovered privilege escalation. It keeps ACL error values (`-1`) and out-of-range values fail-closed across every guest/user grant emission.
- **Final code/test SHA:** `389f62001b607d70c0f4ba3906a830d4c0000ed0`. Exact-head GitHub Actions run [42425408455](https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42425408455) completed **SUCCESS**; all 16 recorded steps succeeded, including GNU89 Samba policy compilation/execution and source-contract checks. The preceding code commit `70da64b461dd17d431d8da1ee693c8b1eb26ddb7` separately passed run [42425349410](https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42425349410). L1 source review PASS; L2 exact-head host CI PASS.

## 2026-11-08 07:29 CST — Secure USB modem hotplug metadata staging against /tmp symlink races

### Finding and change
- **Base main SHA:** `faf672ce13f4b759ce8b8bfefca2fa92c365b5c8`. This rotates the governance audit from the completed Samba config checkpoint into USB hotplug/storage rather than returning to SkipDBv2-only work.
- **P2 local trust-boundary finding:** `mdev_wdm_main()`, `mdev_net_main()` and `mdev_tty_main()` created predictable `/tmp/modem/<device>` metadata with `mkdir_if_none(...,"777")` and `fopen(node_fname,"w+")`. The directory was world-writable and not sticky. A local account able to place a symlink in that directory could make a later root hotplug event follow the link and truncate another root-writable file. This is a local filesystem race in the modem metadata path, not unauthenticated remote access.
- **Fix:** new `modem_node_file.h` validates device basenames, creates/checks `/tmp/modem` as an effective-UID-owned real directory with mode 0755, writes through a unique `mkstemp` staging file, and atomically renames it into place. A pre-existing destination symlink is replaced as a directory entry rather than followed. Unplug cleanup uses the same validated directory/name helper. The only reader, `usb_modem.c::find_modem_node()`, now opens metadata read-only (`"r"`) instead of `"r+"`.
- Added `tests/modem_node_file.c` for basename validation, ownership/mode, atomic publication, destination-symlink victim preservation, symlink-parent rejection and cleanup. The workflow compiles it with GNU89 `-Wall -Wextra -Werror`, runs it, and verifies all three production writer/remover call sites and both push/PR path filters.
- **Scope boundary:** this does not audit the separate block-storage `hotplug_stor.c` → `automount.sh` mount/unmount path, disk enumeration, root-partition rejection, filesystem helpers, or every other /tmp producer. Those remain open. Production MIPS build, firmware/TRX and hardware are not run.

### Verification
- L1 source review of the three modem metadata producers and the shared reader: PASS. A focused read of `automount.sh` also confirmed device/share-name allowlists, separately-read blkid fields (no eval), sanitized labels, and same-device mountpoint checks; this is not a complete block-storage lifecycle audit.
- **Code/test SHA:** `59f69e8869b4cdd03c8b2b3caa78f6c202bc57e9`. Exact-head GitHub Actions run [42425967651](https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42425967651) completed **SUCCESS**; all 17 recorded job steps passed, including GNU89 `-Wall -Wextra -Werror` helper compilation/execution and production source contracts. The Samba run above is a separate prior checkpoint.
- L3 production RC/MIPS build, L4 router/hardware test: NOT RUN.

### Next
- Confirm the exact-head Actions result. Then continue block-storage hotplug (`hotplug_stor.c`, `automount.sh`, mount/unmount and swapoff ordering), `libdisk` path ownership, and destructive-operation gates. Keep Samba's remaining FTP/NFS compatibility gaps, HTTPD/CGI/auth, firewall rollback/IPv6 parity and init/service lifecycle in the global ledger.

## 2026-11-08 07:54 CST — Fail closed on unavailable block-storage partition inventory

### Finding and implementation
- **Base main SHA:** `98bb2a1952f80b8c27658969a309a8457c263209`; **code/test/workflow SHA:** `3fbde30bbdf91a37380752b8e33148bd88e0d2a6` on `main`.
- **P2 data-integrity finding:** `trunk/user/rc/src/hotplug_stor.c::check_root_partition()` returned zero when `/proc/partitions` could not be opened. Both SCSI and MMC whole-disk handlers interpret zero as evidence that there are no child partitions and may proceed to pass the raw device to `automount.sh`. An unavailable or unreadable partition inventory must not authorize whole-disk mounting.
- **Fix:** added `storage_partition_probe.h` with strict SCSI/MMC whole-disk-name validation, exact numeric child-partition parsing and tri-state results (partition found / no partition / error). The production wrapper now returns an error on invalid names, failed open, stream read error, and close error if no partition was found. Existing SCSI and MMC whole-disk call sites stop on any nonzero result, so an inventory failure cannot take the unpartitioned-disk path.
- **Regression and workflow:** added `trunk/user/rc/src/tests/storage_partition_probe.c` covering SCSI/MMC partition discovery, unpartitioned disks, prefix lookalikes (`sdaa1`, `sdaX`), invalid device names and missing input. Extended `.github/workflows/httpd-security.yml` with GNU89 `-Wall -Wextra -Werror` compile/run plus source wiring assertions; both push and PR path filters include the production source, helper and test.
- **Changed paths:** `trunk/user/rc/src/hotplug_stor.c`; new `trunk/user/rc/src/storage_partition_probe.h`; new `trunk/user/rc/src/tests/storage_partition_probe.c`; `.github/workflows/httpd-security.yml`.

### Validation and limits
- L1 scoped source review: PASS.
- Exact-head L2 GitHub Actions [run 42427177541](https://github.com/xiaobei206/padavan-4.4-2/actions/runs/42427177541) completed **SUCCESS** on code/test SHA `3fbde30bbdf91a37380752b8e33148bd88e0d2a6`; the single job's 18 recorded steps all completed successfully, including compilation and execution of the new C regression and production source-contract checks.
- L3 production RC/MIPS object/firmware/TRX build and artifact validation: **NOT RUN**. L4 physical-router flash/boot and runtime storage tests: **NOT RUN**.
- This fix does not close the storage audit. `automount.sh` mountpoint concurrency and filesystem-check gates, permission/option compatibility (including post-mount `chmod 777`), `services_stor.c::umount_dev()` ignored optware/umount status, swapoff ordering, stale mount cleanup and `libdisk` removable-device path provenance remain open.

### Next
- Audit the complete `automount.sh` → mountpoint → optware → unmount/eject lifecycle, including `services_stor.c` status handling and libdisk device enumeration. Make one bounded tested change for a confirmed defect. Then rotate to init/service lifecycle and keep the higher-risk HTTPD/CGI/auth, firewall rollback/IPv6 and WAN/PPP rows active; do not pivot back to routine SkipDBv2 work.

## 2026-11-08 07:59:19 CST — Add fail-closed e2fsck status policy

- **Base main SHA:** `5b0e24f0e73ca7cd441dbf700497003b2fdcf952`; **helper commit:** `7bdcb7f98e76fa84766c603adfb862509cf879d4`.
- Added `trunk/user/scripts/files/sbin/storage_fsck_status.sh` with a locale-independent shell case policy that permits only e2fsck exit statuses 0 (clean) and 1 (errors corrected). All other values are denied.
- Confirmed `trunk/user/scripts/Makefile` copies `files/*` into ROMFS, so this helper is included by the existing package install rule without a separate file manifest.
- **Validation:** local `sh -n` passed; subsequent policy regression exercised allowed and rejected results.

## 2026-11-08 07:59:22 CST — Add shell regression for e2fsck mount status policy

- **Base main SHA:** `7bdcb7f98e76fa84766c603adfb862509cf879d4`; **test commit:** `098a94755907029ff5a3c4ef97429ad26c8c8e20`.
- Added `trunk/user/scripts/tests/storage_e2fsck_status.sh`. It sources the shipped policy helper, accepts statuses 0/1 and rejects reboot-required, unresolved-error, operational, usage, cancellation, library-error, empty and malformed values.
- **Validation:** local `sh -n` and test execution PASS. The test is invoked with `sh` and does not depend on executable file mode.

