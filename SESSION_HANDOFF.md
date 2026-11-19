# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume concrete engineering tasks. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning. Before context exhaustion, complete the safe slice and preserve its exact source/CI boundary.

- Create all changed blobs, one complete tree and one commit with the current main parent; update main with non-forced compare-and-swap using that exact SHA.
- Verify Actions by exact `head_sha`. A pending run does not block independent work; a failed relevant run must be triaged before claiming validation.
- Preserve security, architecture, GNU89/MIPS/uClibc compatibility, regression coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` is archival only; never alter or delete it. Develop on main.

## Current repository state

- Current parent for the shared process-inventory I/O slice: `d57cce1bfbcd515f8f4bd0b6f018ac0302fb4129` (`fix(rc): preserve lock contention waits across EINTR`).
- Prior MiniUPnP lock retry source SHA `e736dbfcff830e6ac6baa32026fff7a6334787d1`: Process lifecycle, MiniUPnP config, Process inventory, NVRAM audit and XY-C1 Build/TRX/artifact all PASS. AI docs was repaired by `a9f3c20` and passed run `44629051861`.
- Shared lock retry source SHA `213a077af64819acf40fd44ca5a6df91f7472fb4`: MiniUPnP config `44631039048`, VPN `44631039170`, AI docs `44631039225`, DNSMasq `44631039014`, Inadyn `44631039151`, Process inventory `44631039149`, Process regression `44631039075`, HTTPD security `44631039061`, NVRAM audit `44631039111` all PASS; XY-C1 `44631039226` was still in progress at last poll.
- Signal-safe acquisition wait source SHA `d57cce1bfbcd515f8f4bd0b6f018ac0302fb4129`: AI docs `44631860955`, HTTPD `44631856558`, Process inventory `44631856563`, Inadyn `44631856501`, DNSMasq `44631856635`, Process regression `44631856709`, VPN `44631856608`, MiniUPnP config `44631856559` and NVRAM audit `44631856644` all PASS. Exact XY-C1 `44631856549` remained IN_PROGRESS at last query; do not claim L3 until Build, TRX and artifact steps succeed.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; archival only. PR #1 remains closed/unmerged.
- No physical-router result is available. Host Actions are L2; XY-C1 Build/TRX/artifact on one exact production SHA is L3; device behavior is L4 NOT VERIFIED.

## Active task

- Confirmed a shared process-inventory reliability defect in `proc_default_read_file()`: both the normal procfs read loop and the one-byte overflow probe retried `read(2)` indefinitely on EINTR. A signal storm could leave process inventory scans stuck instead of returning UNKNOWN.
- Added `process_io_retry_policy.h` with an eight-consecutive-EINTR cap. Normal positive/EOF reads reset the retry streak; overflow probing has its own finite budget. Exhaustion retains `errno=EINTR`, so the scanner fails closed rather than misreporting ABSENT.
- Added GNU89 regression coverage for normal retry budget, exhaustion and invalid argument; Process workflow now includes a source contract requiring both read loops to use the policy and watches the new header. XY-C1 also watches the header.
- No local compilation or physical-router result is claimed for this source slice.

## Exact first action for the next GPT

1. Fetch live main and inspect exact-head Process lifecycle, Process inventory, AI-doc, NVRAM and XY-C1 workflows for the bounded procfs read change.
2. On FAIL, inspect the exact job log and fix the demonstrated source/build contract without removing the read retry ceiling or weakening UNKNOWN semantics. XY-C1 L3 requires Build, TRX verification and artifact upload all SUCCESS on the exact same SHA.
3. Continue with an independently evidenced stability/security issue across Padavan userland; do not repeat closed lifecycle-lock EINTR and contention-wait audits without new evidence.

## Verification boundary

- L1: `proc_default_read_file()` now caps consecutive EINTR retries at eight for both regular reads and the overflow probe; exhaustion remains a hard I/O error/UNKNOWN.
- L2: GNU89 test and Process workflow source contract are added; exact-head Process/AI/NVRAM Actions remain pending.
- L3: XY-C1 watches the new process-I/O policy header; Build/TRX/artifact on this exact source SHA remain pending.
- L4: target MIPS/uClibc procfs behavior during repeated signals and physical-router process supervision remain NOT VERIFIED.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST perform a handoff checkpoint before ending. At session end, compare live main, product-source baseline, exact-head Actions, module state and this first action. Update this file only when resumable facts materially change; do not create no-op SHA-refresh commits.

## Development invariants

- Main-only; never modify the frozen backup or develop on a legacy branch/PR.
- One complete Git tree and one atomic commit per logical task; `force=false`, exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits or weaken regressions.
- Report exact SHA Actions and keep L1/L2/L3/L4 distinct. Host success is not firmware/hardware success.
