# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect live repository state and compare it with this file before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers and precise first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave this file untouched and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task here.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, and do not ask the user to choose the next task.

Every multi-file logical change must be created as one complete Git tree and one atomic main commit parented to freshly fetched main, then update main with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, or modify the frozen backup branch.

## Current repository state

- Latest main parent before THIS COMMIT: `b54a701c35e35549a183eca6bf48c86d5dffb855`. Exact-head Actions at last poll: WAN interface teardown stability regression 43917116176 completed/success; XY-C1 Firmware Validation 43917115679 in_progress; AI handoff documentation contract 43917115649 completed/success; infosvr packet and signal lifecycle stability regression 43917115954 completed/success; USB modem WDM/NDIS physical identity stability 43917115923 completed/success; AliDDNS signed API transport security regression 43917115746 completed/success; AliyunDrive WebDAV lifecycle stability regression 43917115709 completed/success; AdGuardHome config security regression 43917116214 completed/success; Process lifecycle inventory - b54a701c35e35549a183eca6bf48c86d5dffb855 43917116036 completed/success; HTTPD Authentication and Security Regression 43917116031 completed/failure; Process inventory and lifecycle fail-closed regression 43917115898 completed/success; NVRAM transaction boundary audit 43917115833 completed/success
- Frozen backup branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable and read-only.
- Storage mount ownership is now based solely on checked `/proc/mounts` state. The mountpoint transaction uses three tri-state checks (preflight, final-target revalidation, and post-mount source verification); `mountpoint -q` is no longer used.
- The NDIS interface bring-up status gate landed in `b54a701c35e35549a183eca6bf48c86d5dffb855`; its exact-head XY-C1 validation is still in progress.

## Active task

THIS COMMIT repairs a stale assertion in the HTTPD umbrella workflow. On parent `b54a701c35e35549a183eca6bf48c86d5dffb855`, run `43917116031` failed at its filesystem-checker/source-contract step after earlier policy and storage regression steps passed. The Python assertion still required removed `mountpoint -q` calls; it now checks that the mount lock precedes the tri-state owner checks, all three owner queries occur, the helper preserves owned/foreign/absent/UNKNOWN states, the postcondition precedes lock release, and no `mountpoint -q` shortcut remains. No production code changes.

## Verification boundary

- Parent `b54a701c35e35549a183eca6bf48c86d5dffb855`: HTTPD Authentication and Security Regression `43917116031` FAILED at the stale filesystem source assertion; all preceding steps in that job passed. XY-C1 `43917115679` was IN_PROGRESS. Other exact-head statuses at commit time: WAN interface teardown stability regression 43917116176 completed/success; XY-C1 Firmware Validation 43917115679 in_progress; AI handoff documentation contract 43917115649 completed/success; infosvr packet and signal lifecycle stability regression 43917115954 completed/success; USB modem WDM/NDIS physical identity stability 43917115923 completed/success; AliDDNS signed API transport security regression 43917115746 completed/success; AliyunDrive WebDAV lifecycle stability regression 43917115709 completed/success; AdGuardHome config security regression 43917116214 completed/success; Process lifecycle inventory - b54a701c35e35549a183eca6bf48c86d5dffb855 43917116036 completed/success; HTTPD Authentication and Security Regression 43917116031 completed/failure; Process inventory and lifecycle fail-closed regression 43917115898 completed/success; NVRAM transaction boundary audit 43917115833 completed/success
- Parent NDIS source change `b54a701c35e35549a183eca6bf48c86d5dffb855` has its XY-C1 MIPS build/TRX/artifact run still pending; this workflow-only correction does not replace that source-SHA evidence.
- No full local repository run claimed. Frozen backup unchanged; physical-router L4 NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head HTTPD umbrella Actions for THIS COMMIT; confirm the filesystem source-contract step now passes.
2. Inspect exact-head XY-C1 Build, TRX verification and artifact upload for the NDIS source SHA `b54a701c35e35549a183eca6bf48c86d5dffb855`.
3. Resume independent subsystem fixes while Actions are pending, not repeated audits of mountpoint ownership.
4. Preserve frozen backup and physical-router L4 NOT VERIFIED.
