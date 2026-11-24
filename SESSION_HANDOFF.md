## Current repository state

- Live parent main before the FRP argv-boundary correction: `e779430780d42896c8172469c5a358babc20355f`.
- Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Product-source SHA `ea7d7e28be9988175fbcc423f9378e5d765de764` passed exact XY-C1 #45358712652, including Build, TRX verification and artifact upload; Process #45358712716 and NVRAM #45358712663 also PASS. Later commits `0c23ad7c82fc2e033f73d4a94f6fb02a77324ca5` (workflow checker only) and `e779430780d42896c8172469c5a358babc20355f` (handoff/dev-log only) did not modify firmware production inputs. L3 is verified for that product-source baseline; no physical L4 validation is claimed.
- FRP checked procfs transform-status fix `0104186915ee6132d99871bede241ecf1835c279`: exact FRP lifecycle #45358242268 and Process inventory #45358242232 PASS.
- VSFTPD SITE CHMOD/UMASK strict octal source `ea7d7e28be9988175fbcc423f9378e5d765de764`: checker-scope correction `0c23ad7c82fc2e033f73d4a94f6fb02a77324ca5`; exact VSFTPD stability #45359061300 PASS. AI handoff wording correction `e779430780d42896c8172469c5a358babc20355f` passed exact AI handoff #45359300706.
- AliDNS POSIX byte encoder source `ca9267b3c71a9fb2a8de96e82a2878fcf31e5dc6`: exact AliDNS #45357686604 PASS. XY-C1 product baseline above includes this source as well.
- Main remains the active development branch; the frozen backup is unchanged.

## Active task — FRP process argv must distinguish literal LF from NUL separators

- The current matcher validates the executable symlink and translates NUL-delimited `/proc/PID/cmdline` into LF-delimited records. Literal LF inside a real argv element was not rejected, so argv[0] `/usr/bin/frpc\n-c` plus the config path produced the same three AWK records as the managed `frpc -c <config>` command. This could make a stop operation signal the wrong FRPC invocation.
- This patch preflights raw `/proc/PID/cmdline` with checked `wc -l`, rejects any literal LF as a definite mismatch, and preserves UNKNOWN when that read fails. The target BusyBox config explicitly enables `wc`; workflow path filters now include the config. The stop test checks both parser return status and zero TERM/KILL signals for the spoofed argv.
- Local BusyBox-sh reproducer: the old matcher returned MATCH for the forged argv; the raw LF guard detects one literal newline. Full exact-head CI for this implementation is pending.

## Verification boundary

- Exact `ea7d7e28be9988175fbcc423f9378e5d765de764`: XY-C1 #45358712652 PASS (Build, TRX and artifact upload); Process #45358712716 PASS; NVRAM #45358712663 PASS; AI handoff #45358712746 PASS.
- Exact `0c23ad7c82fc2e033f73d4a94f6fb02a77324ca5`: VSFTPD stability #45359061300 PASS.
- Exact `e779430780d42896c8172469c5a358babc20355f`: AI handoff #45359300706 PASS.
- Exact `0104186915ee6132d99871bede241ecf1835c279`: FRP lifecycle #45358242268 PASS; Process inventory #45358242232 PASS.
- FRP newline-boundary patch: exact-head CI pending.
- L3 product build is verified for the `ea7d7e2` production tree; no L4 hardware result is claimed.

## Exact first action for the next GPT

1. Check the FRP lifecycle workflow and Process inventory Actions for the exact SHA produced by the argv-boundary patch. If either fails, inspect its first causal assertion and keep literal-LF arguments rejected without weakening UNKNOWN handling.
2. Confirm the new stop regression passes on the actual test runner and the static checker sees `CONFIG_WC=y` in the target BusyBox config.
3. Once the FRP slice is green, continue the next independent P1 user-space stability/security item in `REFACTOR_ROADMAP.md`; do not repeat the completed FRP, AliDNS or VSFTPD audits.
4. Keep the backup branch immutable and update only `main` via a non-forced expected-SHA compare-and-swap.

# Cross-session execution mandate

Treat this as continuous work across GPT conversations. Recover once, then execute on `main`; each logical source or CI change must land as one atomic main commit. Create all changed blobs, one complete Git tree and one commit before updating the branch with a non-forced expected-SHA compare-and-swap. Never force-push or modify the frozen backup.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST perform a handoff checkpoint before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

Every session MUST inspect live repository state before ending. Record only material exact-head Action transitions, new source commits, genuine unresolved failures or a changed first action. Do not manufacture documentation churn or no-op commits.

## Development loop

- Source modification → targeted regression → one complete Git tree/one atomic commit → exact-SHA Actions.
- PASS advances; FAIL is diagnosed and fixed; PENDING does not block independent work.
- Preserve Linux 4.4, GNU89, MIPS/uClibc compatibility, router configuration, security, architecture and backward compatibility.
- Before context exhaustion, record exact latest SHA, tests, CI state, unresolved issues and one concrete first task.
