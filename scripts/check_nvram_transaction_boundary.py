#!/usr/bin/env python3
"""Audit the NVRAM shared-userland locking boundary and kernel callers."""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
USER_ROOT = ROOT / "trunk/user"
KERNEL_ROOT = ROOT / "trunk/linux-4.4.x"
BOARD_ROOT = ROOT / "trunk/configs/boards"
ADAPTER = "trunk/user/shared/src/nvram_linux.c"
DRIVER = "trunk/linux-4.4.x/drivers/nvram/nvram_linux.c"
SOURCE_SUFFIXES = {".c", ".h", ".cc", ".cpp", ".cxx", ".hpp"}
RAW_PATTERNS = re.compile(
    r"\bNVRAM_IOCTL_(?:GET|SET|COMMIT|CLEAR)\b|"
    r"/dev\s*/\s*nvram\b|\bPATH_DEV_NVRAM\b|\bNVRAM_DEVICE_NODE_PATH\b"
)
KERNEL_API = re.compile(r"\b(nvram_(?:get|getall|set|unset|commit|clear))\s*\(")
KERNEL_WRITERS = re.compile(r"\b(nvram_(?:set|unset|commit))\s*\(")


def strip_c_comments(source: str) -> str:
    """Remove C comments while preserving strings, characters and line numbers."""
    result: list[str] = []
    state = "code"
    index = 0
    while index < len(source):
        current = source[index]
        following = source[index + 1] if index + 1 < len(source) else ""

        if state == "code":
            if current == "/" and following == "/":
                result.extend((" ", " "))
                index += 2
                state = "line"
                continue
            if current == "/" and following == "*":
                result.extend((" ", " "))
                index += 2
                state = "block"
                continue
            if current == '"':
                result.append(current)
                state = "string"
            elif current == "'":
                result.append(current)
                state = "char"
            else:
                result.append(current)
        elif state == "line":
            if current == "\n":
                result.append("\n")
                state = "code"
            else:
                result.append(" ")
        elif state == "block":
            if current == "*" and following == "/":
                result.extend((" ", " "))
                index += 2
                state = "code"
                continue
            result.append("\n" if current == "\n" else " ")
        else:
            result.append(current)
            if current == "\\" and following:
                result.append(following)
                index += 2
                continue
            if (state == "string" and current == '"') or (
                state == "char" and current == "'"
            ):
                state = "code"
        index += 1
    return "".join(result)


def extract_function(source: str, name: str) -> str:
    """Extract one C function body, ignoring braces in literals."""
    clean = strip_c_comments(source)
    signature = re.compile(
        r"\b" + re.escape(name) + r"\s*\([^;{}]*?\)\s*\{", re.DOTALL
    )
    match = signature.search(clean)
    if not match:
        raise ValueError(f"missing function definition: {name}")
    opening = clean.find("{", match.start())
    depth = 0
    state = "code"
    index = opening
    while index < len(clean):
        current = clean[index]
        following = clean[index + 1] if index + 1 < len(clean) else ""
        if state == "code":
            if current == '"':
                state = "string"
            elif current == "'":
                state = "char"
            elif current == "{":
                depth += 1
            elif current == "}":
                depth -= 1
                if depth == 0:
                    return clean[opening + 1:index]
        else:
            if current == "\\" and following:
                index += 2
                continue
            if (state == "string" and current == '"') or (
                state == "char" and current == "'"
            ):
                state = "code"
        index += 1
    raise ValueError(f"unterminated function body: {name}")


def source_files(root: Path):
    if not root.is_dir():
        return
    for path in sorted(root.rglob("*")):
        if path.is_file() and path.suffix.lower() in SOURCE_SUFFIXES:
            yield path


def self_test() -> None:
    sample = (
        '/* NVRAM_IOCTL_SET /dev/nvram */\n'
        'const char *path = "/dev/nvram"; // NVRAM_IOCTL_GET\n'
        'int sample(void) { const char *brace = "}"; return 0; }\n'
    )
    stripped = strip_c_comments(sample)
    assert "NVRAM_IOCTL_SET" not in stripped
    assert "NVRAM_IOCTL_GET" not in stripped
    assert '"/dev/nvram"' in stripped
    assert extract_function(sample, "sample").count("return 0") == 1


def run() -> int:
    failures: list[str] = []
    self_test()

    adapter_path = ROOT / ADAPTER
    lock_path = ROOT / "trunk/user/shared/src/nvram_transaction_lock.h"
    open_policy_path = ROOT / "trunk/user/shared/src/nvram_open_policy.h"
    open_policy_test_path = ROOT / "trunk/user/shared/src/tests/nvram_open_policy.c"
    lock_test_path = ROOT / "trunk/user/shared/src/tests/nvram_transaction_lock.c"
    driver_path = ROOT / DRIVER
    driver_makefile = ROOT / "trunk/linux-4.4.x/drivers/nvram/Makefile"
    drivers_makefile = ROOT / "trunk/linux-4.4.x/drivers/Makefile"
    bcm_header = ROOT / "trunk/linux-4.4.x/include/nvram/bcmnvram.h"
    snapshot_policy_path = ROOT / "trunk/linux-4.4.x/drivers/nvram/nvram_snapshot_policy.h"
    snapshot_test_path = ROOT / "trunk/linux-4.4.x/drivers/nvram/tests/nvram_snapshot_policy.c"
    sensitive_policy_path = ROOT / "trunk/linux-4.4.x/drivers/nvram/nvram_sensitive_buffer_policy.h"
    kernel_common_path = ROOT / "trunk/linux-4.4.x/drivers/nvram/nvram.c"
    compaction_policy_path = ROOT / "trunk/linux-4.4.x/drivers/nvram/nvram_compaction_policy.h"
    compaction_test_path = ROOT / "trunk/linux-4.4.x/drivers/nvram/tests/nvram_compaction_policy.c"
    rehash_policy_path = ROOT / "trunk/linux-4.4.x/drivers/nvram/nvram_rehash_policy.h"
    rehash_test_path = ROOT / "trunk/linux-4.4.x/drivers/nvram/tests/nvram_rehash_policy.c"
    commit_serialization_test_path = ROOT / "trunk/linux-4.4.x/drivers/nvram/tests/nvram_commit_serialization.c"
    cli_source_path = ROOT / "trunk/user/nvram/src/nvram.c"
    cli_policy_path = ROOT / "trunk/user/nvram/src/nvram_cli_policy.h"
    cli_test_path = ROOT / "trunk/user/nvram/src/tests/nvram_cli_policy.c"
    cli_workflow_path = ROOT / ".github/workflows/nvram-transaction-boundary.yml"

    for required_path in (
        adapter_path, lock_path, open_policy_path, open_policy_test_path, lock_test_path,
        driver_path, driver_makefile,
        drivers_makefile, bcm_header, snapshot_policy_path, snapshot_test_path,
        kernel_common_path, compaction_policy_path, compaction_test_path,
        sensitive_policy_path,
        rehash_policy_path, rehash_test_path, commit_serialization_test_path,
        cli_source_path, cli_policy_path, cli_test_path, cli_workflow_path
    ):
        if not required_path.is_file():
            failures.append(f"required NVRAM source missing: {required_path}")
    if failures:
        for failure in failures:
            print(f"FAIL: {failure}", file=sys.stderr)
        return 1

    adapter = adapter_path.read_text(encoding="utf-8", errors="replace")
    lock_header = lock_path.read_text(encoding="utf-8", errors="replace")
    open_policy = open_policy_path.read_text(encoding="utf-8", errors="replace")
    open_policy_test = open_policy_test_path.read_text(encoding="utf-8", errors="replace")
    lock_test = lock_test_path.read_text(encoding="utf-8", errors="replace")
    kernel_driver = driver_path.read_text(encoding="utf-8", errors="replace")
    kernel_common = kernel_common_path.read_text(encoding="utf-8", errors="replace")
    compaction_policy = compaction_policy_path.read_text(encoding="utf-8", errors="replace")
    compaction_test = compaction_test_path.read_text(encoding="utf-8", errors="replace")
    sensitive_policy = sensitive_policy_path.read_text(encoding="utf-8", errors="replace")
    rehash_policy = rehash_policy_path.read_text(encoding="utf-8", errors="replace")
    rehash_test = rehash_test_path.read_text(encoding="utf-8", errors="replace")
    commit_serialization_test = commit_serialization_test_path.read_text(encoding="utf-8", errors="replace")
    cli_source = cli_source_path.read_text(encoding="utf-8", errors="replace")
    cli_policy = cli_policy_path.read_text(encoding="utf-8", errors="replace")
    cli_test = cli_test_path.read_text(encoding="utf-8", errors="replace")
    cli_workflow = cli_workflow_path.read_text(encoding="utf-8", errors="replace")

    cli_main = extract_function(cli_source, "main")
    try:
        legacy_get = cli_main.index('if (!strcmp(*argv, "get"))')
        checked_start = cli_main.index('else if (!strcmp(*argv, "get_checked"))', legacy_get)
        checked_end = cli_main.index('else if (!strcmp(*argv, "settmp"))', checked_start)
        checked_get = cli_main[checked_start:checked_end]
        read_call = checked_get.index("nvram_get_checked(*argv, &value)")
        errno_capture = checked_get.index("read_errno = errno;", read_call)
        status_map = checked_get.index("nvram_cli_get_error_status(read_status,", errno_capture)
        null_guard = checked_get.index("read_status == 1 && !value", status_map)
        error_gate = checked_get.index("if (read_error != 0)", null_guard)
        error_report = checked_get.index('perror("nvram get_checked")', error_gate)
        output_value = checked_get.index("puts(value)", error_report)
        if not read_call < errno_capture < status_map < null_guard < error_gate < error_report < output_value:
            failures.append("checked-get must preserve API errors before printing present values")
        if "nvram_get(*argv)" in checked_get:
            failures.append("checked-get must not use the error-collapsing nvram_get wrapper")
        if 'if (!strcmp(*argv, "get"))' not in cli_main:
            failures.append("legacy nvram get command must remain available")
    except ValueError as error:
        failures.append("NVRAM CLI checked-get source contract: " + str(error))

    for marker in ("nvram_cli_get_error_status", "api_status == 0 || api_status == 1",
                   "saved_errno > 0", "return EIO"):
        if marker not in cli_policy:
            failures.append("checked-get status helper is missing " + marker)
    for marker in ("test_checked_get_error_status",
                   "nvram_cli_get_error_status(-1, EACCES) == EACCES",
                   "nvram_cli_get_error_status(-1, 0) == EIO",
                   "nvram_cli_get_error_status(0, 0) == 0",
                   "NVRAM CLI assignment and checked-get policy: PASS"):
        if marker not in cli_test:
            failures.append("checked-get regression is missing " + marker)
    if ("Compile and run checked-get NVRAM CLI status regression" not in cli_workflow or
        "trunk/user/nvram/src/tests/nvram_cli_policy.c" not in cli_workflow):
        failures.append("NVRAM workflow must compile and execute checked-get regression")

    # Profile buffers and snapshots contain credentials; every release path
    # must perform the volatile wipe immediately before free().
    for marker in (
        "static void\\nnvram_cli_secure_clear(void *buffer, size_t length)",
        "volatile unsigned char *cursor",
        "*cursor++ = 0;",
    ):
        if marker not in cli_policy:
            failures.append("NVRAM CLI secure-clear policy is missing " + marker)
    try:
        def secure_free_pairs(body, variable, length_expression):
            wipe = re.escape("nvram_cli_secure_clear(" + variable + ", " +
                             length_expression + ");")
            release = re.escape("free(" + variable + ");")
            pairs = re.findall(wipe + r"\\s*" + release, body)
            frees = re.findall(release, body)
            return len(pairs) == len(frees)

        for function, resources in (
            ("nvram_save", (("buf", "NVRAM_SPACE"),)),
            ("nvram_restore", (("buf", "(size_t)profile_capacity"),
                               ("permanent_snapshot", "NVRAM_SPACE"),
                               ("temporary_snapshot", "NVRAM_SPACE"))),
            ("nvram_show", (("buf", "(size_t)buf_len"),)),
        ):
            body = extract_function(cli_source, function)
            for variable, length_expression in resources:
                if not secure_free_pairs(body, variable, length_expression):
                    failures.append(
                        f"NVRAM CLI {function} must erase every {variable} buffer before free"
                    )
    except ValueError as error:
        failures.append("NVRAM CLI sensitive-buffer source contract: " + str(error))
    for marker in (
        "test_sensitive_buffer_clear_erases_complete_region",
        "nvram_cli_secure_clear(NULL, sizeof(secret))",
        "NVRAM CLI sensitive-buffer erasure: PASS",
    ):
        if marker not in cli_test:
            failures.append("NVRAM CLI secure-clear regression is missing " + marker)

    # A raw userland ioctl/device path is allowed only in the locked shared adapter.
    raw_hits: list[tuple[str, int, str]] = []
    adapter_raw_count = 0
    device_node_path_references = 0
    for path in source_files(USER_ROOT):
        relative = path.relative_to(ROOT).as_posix()
        raw = path.read_text(encoding="utf-8", errors="replace")
        clean = strip_c_comments(raw)
        lines = clean.splitlines()
        for match in RAW_PATTERNS.finditer(clean):
            line_number = clean.count("\n", 0, match.start()) + 1
            line = lines[line_number - 1].strip()
            # ABI/path declarations are definitions, not access sites.
            if re.match(r"#\s*define\s+NVRAM_IOCTL_", line):
                continue
            if (
                relative == "trunk/user/rc/src/nvram_device_node_policy.h"
                and re.match(r'#\s*define\s+NVRAM_DEVICE_NODE_PATH\s+"/dev/nvram"', line)
            ):
                continue
            if (
                relative == "trunk/user/rc/src/init.c"
                and match.group(0) == "NVRAM_DEVICE_NODE_PATH"
            ):
                device_node_path_references += 1
                continue
            if relative == ADAPTER:
                adapter_raw_count += 1
            else:
                raw_hits.append((relative, line_number, line))

    if adapter_raw_count < 6:
        failures.append(
            "shared NVRAM adapter no longer exposes the expected GET/SET/COMMIT/CLEAR ioctl paths"
        )
    for path, line_number, line in raw_hits:
        print(f"UNLOCKED_USERLAND_NVRAM_ACCESS: {path}:{line_number}: {line}")
    if raw_hits:
        failures.append(
            "raw /dev/nvram or NVRAM_IOCTL access exists outside the locked shared adapter"
        )

    # A successful SET/COMMIT/CLEAR must not hide an fd close failure;
    # when ioctl and close both fail, retain the primary ioctl errno.
    close_policy_path = ROOT / "trunk/user/shared/src/nvram_ioctl_close_policy.h"
    close_policy_test_path = ROOT / "trunk/user/shared/src/tests/nvram_ioctl_close_policy.c"
    close_policy = close_policy_path.read_text(encoding="utf-8", errors="replace")
    close_policy_test = close_policy_test_path.read_text(encoding="utf-8", errors="replace")
    for marker in (
        "ioctl_status < 0",
        "close_status < 0",
        "ioctl_errno ? ioctl_errno : EIO",
        "close_errno ? close_errno : EIO",
    ):
        if marker not in close_policy:
            failures.append("NVRAM ioctl/close policy is missing " + marker)
    for marker in (
        "test_success_requires_successful_close",
        "test_ioctl_error_takes_precedence_over_close_error",
        "test_missing_error_values_fail_closed",
        "NVRAM ioctl/close error precedence regression: PASS",
    ):
        if marker not in close_policy_test:
            failures.append("NVRAM ioctl/close host regression is missing " + marker)
    try:
        finish_body = extract_function(adapter, "nvram_finish_ioctl")
        if "nvram_ioctl_close_status" not in finish_body:
            failures.append("NVRAM fd close result is not evaluated")
        for function in ("nvram_set_unlocked", "nvram_commit_unlocked", "nvram_clear_unlocked"):
            body = extract_function(adapter, function)
            if "nvram_finish_ioctl(nvram_fd, ret, saved_errno)" not in body:
                failures.append(f"{function} does not propagate close failure and ioctl errno")
    except ValueError as error:
        failures.append("NVRAM ioctl/close source contract: " + str(error))

    # Verify public shared-library operations hold the flock around each ioctl.
    wrapper_contracts = (
        ("nvram_get_checked", "1", "nvram_get_checked_unlocked"),
        ("nvram_getall", "0", "nvram_getall_unlocked"),
        ("nvram_set_guarded", "1", "nvram_set_unlocked"),
        ("nvram_commit", "1", "nvram_commit_unlocked"),
        ("nvram_clear", "1", "nvram_clear_unlocked"),
    )
    for function, mode, unlocked in wrapper_contracts:
        try:
            body = extract_function(adapter, function)
        except ValueError as error:
            failures.append(str(error))
            continue
        if f"nvram_api_lock_acquire({mode})" not in body:
            failures.append(
                f"{function} does not acquire the expected "
                f"{'exclusive' if mode == '1' else 'shared'} flock"
            )
        if unlocked not in body or "nvram_api_lock_finish" not in body:
            failures.append(
                f"{function} does not finish the locked operation through {unlocked}"
            )

    # The raw NVRAM device descriptor must not survive exec() and bypass
    # the process-scoped shared-userland flock transaction boundary.
    if '#include "nvram_open_policy.h"' not in adapter:
        failures.append("NVRAM adapter does not include the close-on-exec open policy")
    try:
        open_flags_body = extract_function(open_policy, "nvram_device_open_flags")
        if "O_RDWR" not in open_flags_body or "O_CLOEXEC" not in open_flags_body:
            failures.append("NVRAM device open policy must combine O_RDWR and O_CLOEXEC")
        for function in (
            "nvram_get_checked_unlocked",
            "nvram_getall_unlocked",
            "nvram_set_unlocked",
            "nvram_commit_unlocked",
            "nvram_clear_unlocked",
        ):
            body = extract_function(adapter, function)
            if "nvram_device_open_flags()" not in body:
                failures.append(f"{function} does not use the close-on-exec NVRAM open policy")
    except ValueError as error:
        failures.append("NVRAM close-on-exec source contract: " + str(error))
    for marker in (
        "test_device_open_flags_preserve_read_write",
        "test_open_descriptor_is_close_on_exec",
        "FD_CLOEXEC",
        "NVRAM device descriptor close-on-exec policy: PASS",
    ):
        if marker not in open_policy_test:
            failures.append("NVRAM close-on-exec regression is missing " + marker)
    if ("Compile and run NVRAM device descriptor close-on-exec regression" not in cli_workflow or
        "trunk/user/shared/src/tests/nvram_open_policy.c" not in cli_workflow):
        failures.append("NVRAM workflow must compile and execute close-on-exec regression")

    value_cache_path = ROOT / "trunk/user/shared/src/nvram_value_cache_policy.h"
    value_cache_test_path = ROOT / "trunk/user/shared/src/tests/nvram_value_cache_policy.c"
    value_cache_policy = value_cache_path.read_text(encoding="utf-8", errors="replace")
    value_cache_test = value_cache_test_path.read_text(encoding="utf-8", errors="replace")
    for marker in ("volatile unsigned char *cursor", "*cursor++ = 0;",
                   "*next_index = 0"):
        if marker not in value_cache_policy:
            failures.append("NVRAM transaction cache erasure policy is missing " + marker)
    for marker in (
        "test_full_cache_is_erased_and_cursor_reset",
        "test_null_cache_still_resets_cursor",
        "test_reserve_keeps_the_current_lap_when_space_is_sufficient",
        "test_reserve_erases_the_previous_lap_at_the_minimum_wrap_boundary",
        "test_reserve_erases_the_previous_lap_when_a_value_will_not_fit",
        "test_reserve_rejects_invalid_cursor_without_erasing",
    ):
        if marker not in value_cache_test:
            failures.append("NVRAM transaction cache regression is missing " + marker)

    try:
        cache_reader = extract_function(adapter, "nvram_get_checked_unlocked")
        if ("char *nvr_cache, int *nvr_index" not in adapter or
            "*nvr_index = i_cache + nvr.len_value" not in cache_reader):
            failures.append("NVRAM checked-get must use an explicit cache and cursor")
        tx_begin = extract_function(adapter, "nvram_transaction_begin")
        tx_end = extract_function(adapter, "nvram_transaction_end")
        tx_get = extract_function(adapter, "nvram_transaction_get")
        public_get = extract_function(adapter, "nvram_get_checked")
        if ("nvram_transaction_cache" not in tx_get or
            "nvram_api_cache" not in public_get):
            failures.append("transaction and legacy NVRAM getters must use separate value caches")
        if ("nvram_value_cache_secure_clear" not in tx_begin or
            "nvram_value_cache_secure_clear" not in tx_end):
            failures.append("NVRAM transaction cache must be erased at begin and end")
        if (tx_end.find("nvram_value_cache_secure_clear") < 0 or
            tx_end.find("nvram_value_cache_secure_clear") >
            tx_end.find("nvram_transaction_lock_release")):
            failures.append("NVRAM transaction cache must be erased before releasing its lease")
        if (cache_reader.count("nvram_value_cache_reserve(nvr_cache, MAX_CACHE_LEN,") != 2 or
            "MIN_CACHE_RES, 0, nvr_index)" not in cache_reader or
            "MIN_CACHE_RES, nvr.len_value," not in cache_reader):
            failures.append("legacy and transactional NVRAM cache wrap must erase before reuse")
    except ValueError as error:
        failures.append("NVRAM transaction value-cache source contract: " + str(error))

    transaction_contracts = (
        ("nvram_transaction_getall", "nvram_getall_unlocked"),
        ("nvram_transaction_get", "nvram_get_checked_unlocked"),
        ("nvram_transaction_set", "nvram_set_unlocked"),
        ("nvram_transaction_commit", "nvram_commit_unlocked"),
        ("nvram_transaction_clear", "nvram_clear_unlocked"),
    )
    for function, unlocked in transaction_contracts:
        try:
            body = extract_function(adapter, function)
        except ValueError as error:
            failures.append(str(error))
            continue
        if "nvram_transaction_valid" not in body or unlocked not in body:
            failures.append(
                f"{function} bypasses transaction-owner validation or the unlocked adapter"
            )

    try:
        begin_body = extract_function(adapter, "nvram_transaction_begin")
        if (
            "nvram_transaction_lock_acquire" not in begin_body
            or "transaction->owner_pid = getpid()" not in begin_body
        ):
            failures.append("transaction begin does not establish an exclusive owner-PID lease")
        valid_body = extract_function(adapter, "nvram_transaction_valid")
        if (
            "transaction->owner_pid != getpid()" not in valid_body
            or "fcntl(transaction->lock_fd, F_GETFD)" not in valid_body
        ):
            failures.append(
                "transaction validation does not reject cross-process/closed leases"
            )
    except ValueError as error:
        failures.append(str(error))

    if not all(token in lock_header for token in (
        "NVRAM_TRANSACTION_LOCK_FILE",
        '"/tmp/.nvram.api.lock"',
        "O_NOFOLLOW",
        "O_CLOEXEC",
        # Nonblocking mode flows through the bounded adapter; the real syscall
        # is isolated in nvram_transaction_lock_flock_real().
        "nvram_transaction_lock_flock_with(",
        "LOCK_NB",
        "flock(fd, operation)",
    )):
        failures.append(
            "shared NVRAM lock lost its stable path, no-follow/close-on-exec or bounded flock contract"
        )

    # Preserve the configured wait budget when signals interrupt contention sleeps.
    try:
        wait_interval = extract_function(lock_header, "nvram_transaction_lock_wait_interval")
        retry_helper = extract_function(lock_header, "nvram_transaction_lock_retry_interrupted")
        flock_retry = extract_function(lock_header, "nvram_transaction_lock_flock_with")
        acquire_path = extract_function(lock_header, "nvram_transaction_lock_acquire_path")
        release_path = extract_function(lock_header, "nvram_transaction_lock_release")
        if ("NVRAM_TRANSACTION_LOCK_MAX_INTERRUPTS 8U" not in lock_header or
            "nvram_transaction_lock_retry_interrupted(&interruptions)" not in wait_interval or
            "nanosleep(&requested, &remaining)" not in wait_interval or
            "requested = remaining" not in wait_interval or
            "errno != EINTR" not in wait_interval):
            failures.append("NVRAM lock wait must resume the exact nanosleep remainder after EINTR")
        if ("nvram_transaction_lock_wait_interval(1)" not in acquire_path or
            "(void)sleep(1)" in acquire_path):
            failures.append("NVRAM lock contention must not discard interrupted sleep time")
        if ("NVRAM_TRANSACTION_LOCK_MAX_INTERRUPTS" not in retry_helper or
            "*interruptions >= NVRAM_TRANSACTION_LOCK_MAX_INTERRUPTS" not in retry_helper or
            "++*interruptions" not in retry_helper):
            failures.append("NVRAM lock retry counter must enforce its interruption cap")
        if ("flock_fn(fd, operation, context)" not in flock_retry or
            "errno != EINTR" not in flock_retry or
            "nvram_transaction_lock_retry_interrupted(&interruptions)" not in flock_retry):
            failures.append("NVRAM flock/unlock EINTR retry must be bounded")
        if ("nvram_transaction_lock_flock_with(" not in acquire_path or
            "nvram_transaction_lock_flock_real" not in acquire_path or
            "if (errno == EINTR)\n\t\t\tgoto failed;" not in acquire_path):
            failures.append("NVRAM acquisition must use the bounded flock retry helper")
        if ("nvram_transaction_lock_flock_with(" not in release_path or
            "LOCK_UN" not in release_path):
            failures.append("NVRAM unlock must use the bounded flock retry helper")
    except ValueError as error:
        failures.append("NVRAM lock wait source contract: " + str(error))
    for marker in (
        "lock_flock_interrupt_retry_test",
        "NVRAM flock/unlock EINTR retry success and exhaustion budget: PASS",
        "lock_wait_interrupt_budget_test",
        "NVRAM shared-userland lock wait interruption budget: PASS",
        "interrupted_lock_wait_test",
        "ITIMER_REAL",
        "lock_wait_signal_count >= 3",
        "NVRAM shared-userland lock wait preserves its interval across EINTR: PASS",
    ):
        if marker not in lock_test:
            failures.append("NVRAM interrupted-lock-wait regression is missing " + marker)

    # The privileged device node itself must not grant untrusted processes ioctl access.
    device_policy_path = ROOT / "trunk/user/rc/src/nvram_device_node_policy.h"
    init_source_path = ROOT / "trunk/user/rc/src/init.c"
    device_policy = device_policy_path.read_text(encoding="utf-8", errors="replace")
    init_source = init_source_path.read_text(encoding="utf-8", errors="replace")
    if not re.search(r"(?m)^\s*#define\s+NVRAM_DEVICE_NODE_MODE\s+0600\s*$", device_policy):
        failures.append("NVRAM character device mode is not exactly 0600")
    try:
        node_body = extract_function(init_source, "init_nvram_node")
        compact_node = re.sub(r"\s+", "", node_body)
        for token in (
            "mknod(NVRAM_DEVICE_NODE_PATH,S_IFCHR|NVRAM_DEVICE_NODE_MODE,nvram_device)",
            "lstat(NVRAM_DEVICE_NODE_PATH,&st)",
            "chown(NVRAM_DEVICE_NODE_PATH,0,0)",
            "chmod(NVRAM_DEVICE_NODE_PATH,NVRAM_DEVICE_NODE_MODE)",
            "nvram_device_node_is_secure(&st,nvram_device)",
        ):
            if token not in compact_node:
                failures.append(f"init_nvram_node does not enforce required device-node policy: {token}")
    except ValueError as error:
        failures.append(str(error))
    if device_node_path_references < 5:
        failures.append("NVRAM device path is not confined to the audited PID 1 setup routine")
    try:
        init_nodes_body = re.sub(r"\s+", "", extract_function(init_source, "init_nodes"))
        if "nvram_result=init_nvram_node()" not in init_nodes_body or "returnnvram_result;" not in init_nodes_body:
            failures.append("init_nodes does not propagate an NVRAM device setup failure")
        main_body = re.sub(r"\s+", "", extract_function(init_source, "init_main_loop"))
        signal_init_body = re.sub(r"\s+", "", extract_function(init_source, "init_signals"))
        signal_installer_body = re.sub(r"\s+", "", extract_function(init_source, "init_signal_install_status"))
        for token in (
                "pid1_signal_install_action(sigaction,signal_number,action)",
                "pid1_signal_status_merge(first_status,status)",
        ):
            if token not in signal_installer_body:
                failures.append(f"PID 1 signal installer omits checked status handling: {token}")
        for token in ("signals_fatal[i]", "signals_shutdown[i]", "SIGCHLD", "signals_event[i]", "returnfirst_status;"):
            if token not in signal_init_body:
                failures.append(f"PID 1 signal setup does not check every signal group: {token}")
        if ("signal_status=init_signals();" not in main_body or
                "signal_status==0&&init_mdev_retry_state_is_pending" not in main_body):
            failures.append("PID 1 must suppress deferred mdev retry when signal handler setup fails")
        if ("router_init_status=INIT_ROUTER_RESULT_FATAL;" not in main_body or
                "if(dev_init_status==0&&nvram_node_result==0&&signal_status==0)router_init_status=init_router();" not in main_body or
                "router_ready=(dev_init_status==0&&nvram_node_result==0&&signal_status==0&&init_router_status_is_ready(router_init_status));" not in main_body):
            failures.append("PID 1 must fail closed for an unsafe NVRAM device while allowing only READY/DEGRADED router results")
    except ValueError as error:
        failures.append(str(error))

    # Verify the one Linux 4.4 NVRAM driver is wired into the common driver tree.
    driver_mk = driver_makefile.read_text(encoding="utf-8", errors="replace")
    drivers_mk = drivers_makefile.read_text(encoding="utf-8", errors="replace")
    header = bcm_header.read_text(encoding="utf-8", errors="replace")
    if not re.search(r"(?m)^\s*obj-y\s*\+=\s*nvram_linux\.o\s*$", driver_mk):
        failures.append("drivers/nvram Makefile no longer builds the common NVRAM driver")
    if not re.search(r"(?m)^\s*obj-y\s*\+=\s*nvram/\s*$", drivers_mk):
        failures.append("kernel drivers Makefile no longer includes the common NVRAM directory")
    for macro in (
        "NVRAM_IOCTL_COMMIT", "NVRAM_IOCTL_CLEAR",
        "NVRAM_IOCTL_SET", "NVRAM_IOCTL_GET"
    ):
        if macro not in header or macro not in kernel_driver:
            failures.append(f"kernel/user NVRAM ioctl ABI is missing {macro}")

    # Inventory kernel API references outside the implementation. These are
    # review candidates, not automatically assumed to be runtime call sites.
    kernel_refs: list[tuple[str, int, str, str]] = []
    kernel_writer_refs: list[tuple[str, int, str, str]] = []
    for path in source_files(KERNEL_ROOT):
        relative = path.relative_to(ROOT).as_posix()
        if relative == DRIVER:
            continue
        clean = strip_c_comments(path.read_text(encoding="utf-8", errors="replace"))
        lines = clean.splitlines()
        for match in KERNEL_API.finditer(clean):
            line_number = clean.count("\n", 0, match.start()) + 1
            line = lines[line_number - 1].strip()
            kind = (
                "writer"
                if match.group(1) in {"nvram_set", "nvram_unset", "nvram_commit"}
                else "reader"
            )
            kernel_refs.append((relative, line_number, kind, line))
        for match in KERNEL_WRITERS.finditer(clean):
            line_number = clean.count("\n", 0, match.start()) + 1
            line = lines[line_number - 1].strip()
            kernel_writer_refs.append((relative, line_number, match.group(1), line))

    user_files = sum(1 for _ in source_files(USER_ROOT))
    kernel_files = sum(1 for _ in source_files(KERNEL_ROOT))
    print(f"USERLAND_SOURCE_FILES_SCANNED: {user_files}")
    print(f"RAW_IOCTL_REFERENCES_IN_LOCKED_ADAPTER: {adapter_raw_count}")
    print(f"RAW_USERLAND_BYPASS_COUNT: {len(raw_hits)}")
    print(f"KERNEL_SOURCE_FILES_SCANNED: {kernel_files}")
    print(f"KERNEL_NVRAM_API_REFERENCE_COUNT: {len(kernel_refs)}")
    for path, line_number, kind, line in kernel_refs:
        print(f"KERNEL_NVRAM_{kind.upper()}_REFERENCE: {path}:{line_number}: {line}")
    print(f"KERNEL_NVRAM_WRITER_REFERENCE_COUNT: {len(kernel_writer_refs)}")

    # Verify the ioctl snapshots the value under nvram_lock and never
    # carries the pool pointer into strlen/copy_to_user after unlocking.
    snapshot_policy = snapshot_policy_path.read_text(encoding="utf-8", errors="replace")
    snapshot_test = snapshot_test_path.read_text(encoding="utf-8", errors="replace")
    for marker in (
        "nvram_ioctl_param_request_valid",
        "nvram_ioctl_set_value_request_valid",
        "length == 0",
        "return !has_pointer",
        "has_embedded_nul",
        "NVRAM_GET_SNAPSHOT_RETRIES 3",
        "NVRAM_SNAPSHOT_COPY_INVALID",
        "NVRAM_SNAPSHOT_COPY_READY",
        "NVRAM_SNAPSHOT_COPY_RETRY",
        "nvram_snapshot_copy_decision",
    ):
        if marker not in snapshot_policy:
            failures.append("NVRAM snapshot policy is missing " + marker)
    for marker in (
        "test_ioctl_getall_requires_an_empty_pointer_pair",
        "test_ioctl_parameter_embedded_nul_is_rejected",        "test_ioctl_set_value_pointer_and_mode_are_consistent",
        "nvram_ioctl_param_request_valid(0, 64, 0, 0)",
        "nvram_ioctl_param_request_valid(1, 64, 0, 0)",
        "nvram_ioctl_param_request_valid(5, 64, 1, 1)",
        "test_value_that_fits_is_copied",
        "test_growth_between_sizing_and_copy_retries",
        "test_shrinkage_between_sizing_and_copy_is_safe",
        "test_missing_terminator_at_pool_boundary_is_rejected",
        "NVRAM single-value snapshot sizing policy: PASS",
    ):
        if marker not in snapshot_test:
            failures.append("NVRAM snapshot host regression is missing " + marker)

    try:
        span_body = extract_function(kernel_driver, "nvram_value_span")
        for marker in ("nvram_values", "NVRAM_VALUES_SPACE", "return -EIO"):
            if marker not in span_body:
                failures.append("NVRAM snapshot pointer-span validation missing " + marker)

        snapshot_body = extract_function(kernel_driver, "nvram_get_snapshot")
        measure = snapshot_body.index("required = strnlen(value, remaining);")
        allocate = snapshot_body.index("copy = kmalloc(required, GFP_KERNEL);")
        unlock_after_measure = snapshot_body.rfind(
            "spin_unlock_irqrestore(&nvram_lock, flags);", measure, allocate
        )
        if unlock_after_measure < 0:
            raise ValueError("snapshot sizing path does not unlock before allocation")
        relock = snapshot_body.index(
            "spin_lock_irqsave(&nvram_lock, flags);", allocate
        )
        bounded_copy = snapshot_body.index("memcpy(copy, value, copy_len);")
        unlock_after_copy = snapshot_body.index(
            "spin_unlock_irqrestore(&nvram_lock, flags);", bounded_copy
        )
        if not (
            measure < unlock_after_measure < allocate < relock
            < bounded_copy < unlock_after_copy
        ):
            failures.append(
                "NVRAM snapshot must size under lock, allocate unlocked, "
                "then revalidate and copy under lock"
            )
        if "NVRAM_GET_SNAPSHOT_RETRIES" not in snapshot_body:
            failures.append("NVRAM snapshot retry loop is not bounded")
        if "copy_to_user" in snapshot_body:
            failures.append("NVRAM snapshot helper must not call copy_to_user under spinlock")

        set_body = extract_function(kernel_driver, "user_nvram_set")
        get_body = extract_function(kernel_driver, "user_nvram_get")
        store_length_body = extract_function(kernel_driver, "nvram_user_store_length")
        for function_name, function_body in (("user_nvram_set", set_body), ("user_nvram_get", get_body)):
            if "copy_from_user(&request, nvr, sizeof(request))" not in function_body:
                failures.append(f"{function_name} does not snapshot its ioctl request with copy_from_user")
            if "nvr->" in function_body or re.search(r"\bnvr\s+->", function_body):
                failures.append(f"{function_name} directly dereferences its __user request pointer")
        if "put_user(length, &nvr->len_value)" not in store_length_body:
            failures.append("NVRAM ioctl result length is not written through put_user")
        if "nvram_user_store_length(nvr," not in get_body:
            failures.append("NVRAM GET does not check user result-length writes")
        if "nvram_ioctl_set_value_request_valid(" not in set_body:
            failures.append("NVRAM SET does not validate its value pointer/length and is_temp mode")
        if "request.value != NULL" not in set_body:
            failures.append("NVRAM SET does not distinguish an empty value from an unset request")
        for function_name, function_body in (("user_nvram_set", set_body), ("user_nvram_get", get_body)):
            if "nvram_ioctl_param_request_valid(" not in function_body:
                failures.append(f"{function_name} does not validate the parameter pointer/length pair")
            if "memchr(param, '\\0', request.len_param)" not in function_body:
                failures.append(f"{function_name} does not reject embedded NUL bytes in parameter names")
        getall_branch = get_body.index("if (param[0] == '\\0')")
        if "request.len_param" not in get_body[:getall_branch]:
            failures.append("NVRAM GETALL dispatch is not preceded by request shape validation")

        snapshot_call = "nvram_get_snapshot(param, capacity, &value, &len)"
        if snapshot_call not in get_body:
            failures.append("single-value ioctl does not use the stable snapshot helper")
        if "value = nvram_get(param)" in get_body:
            failures.append("single-value ioctl still retains the unlocked nvram_get() pointer")
        try:
            copy_to_user = get_body.index("copy_to_user(request.value, value, len)")
            call_at = get_body.index(snapshot_call)
            if copy_to_user < call_at:
                failures.append("single-value ioctl copies before obtaining a stable snapshot")
        except ValueError:
            failures.append("single-value ioctl does not copy its private snapshot")
        if "kfree(value)" not in get_body:
            failures.append("single-value ioctl does not release its private snapshot")
        if "#include <linux/uaccess.h>" not in kernel_driver:
            failures.append("NVRAM ioctl must include the explicit uaccess API")


        # Serialize snapshot generation together with flash I/O: taking the
        # mutex after the snapshot permits an older concurrent commit to win.
        commit_body = extract_function(kernel_driver, "nvram_commit")
        commit_lock = commit_body.index("mutex_lock(&nvram_sem);")
        snapshot_lock = commit_body.index("spin_lock_irqsave(&nvram_lock, flags);")
        snapshot_generate = commit_body.index("_nvram_generate((struct nvram_header *)bufw)")
        snapshot_unlock = commit_body.index("spin_unlock_irqrestore(&nvram_lock, flags);")
        unlock_token = "mutex_unlock(&nvram_sem);"
        release_points = []
        unlock_cursor = 0
        while True:
            unlock_at = commit_body.find(unlock_token, unlock_cursor)
            if unlock_at < 0:
                break
            release_points.append(unlock_at)
            unlock_cursor = unlock_at + len(unlock_token)
        read_flash = commit_body.index("ra_mtd_read_nm(")
        write_flash = commit_body.index("ra_mtd_write_nm(")
        if (len(release_points) != 2 or
            not commit_lock < snapshot_lock < snapshot_generate < snapshot_unlock <
                release_points[0] < read_flash < write_flash < release_points[1]):
            failures.append(
                "NVRAM commit must take nvram_sem before snapshot generation and hold it through flash I/O"
            )
        for marker in (
            "test_snapshot_before_mutex_can_regress_flash",
            "test_mutex_before_snapshot_preserves_latest_flash",
            "NVRAM concurrent commit snapshot ordering: PASS",
        ):
            if marker not in commit_serialization_test:
                failures.append("NVRAM concurrent commit-order regression is missing " + marker)

        # Compaction must retain tuple volatility state and flash generation
        # must never silently omit persistent tuples when the image is full.
        compact_body = extract_function(kernel_driver, "nvram_compact_values")
        for marker in (
            "nvram_hash",
            "nvram_values",
            "nvram_offset",
            "nvram_compaction_value_length",
            "nvram_compaction_fits",
            "memcpy(t->value, snapshot + source_offset, value_length + 1)",
            "t->value =",
            "t->val_len =",
        ):
            if marker not in compact_body:
                failures.append("NVRAM arena compaction is missing " + marker)
        if "t->val_tmp" in compact_body or "nvram_rehash(" in compact_body or "nvram_free(" in compact_body:
            failures.append("NVRAM arena compaction must preserve tuple volatility metadata in place")

        set_temp_body = extract_function(kernel_driver, "nvram_set_temp")
        if "nvram_compact_values()" not in set_temp_body or not (
            set_temp_body.index("nvram_compact_values()")
            < set_temp_body.rindex("_nvram_set(")
        ):
            failures.append("NVRAM SET must compact its arena before retrying allocation")
        realloc_body = extract_function(kernel_driver, "_nvram_realloc")
        try:
            value_publish = realloc_body.index("strcpy(rt->value, value)")
            temp_publish = realloc_body.index("rt->val_tmp =")
            if value_publish >= temp_publish:
                failures.append("failed NVRAM value growth must not change tuple persistence state")
        except ValueError:
            failures.append("NVRAM tuple allocation does not publish its value/temp state")

        generate_body = extract_function(kernel_common, "_nvram_generate")
        if "nvram_generate_entry_size" not in generate_body or "return -ENOSPC;" not in generate_body:
            failures.append("NVRAM flash generation must reject an image that cannot contain every persistent tuple")
        if "if (t->val_tmp)" not in generate_body or "break;" in generate_body:
            failures.append("NVRAM flash generation must skip only volatile tuples and never silently truncate")
        if "nvram_free(" in generate_body or "nvram_rehash(" in generate_body:
            failures.append("NVRAM serialization must not rebuild volatile tuples as persistent")

        for marker in (
            "nvram_compaction_value_length",
            "nvram_compaction_fits",
            "nvram_generate_entry_size",
            "test_compaction_value_spans_are_bounded",
            "test_missing_terminator_is_rejected",
            "test_compaction_destination_boundaries",
            "test_flash_entry_size_fails_closed",
            "NVRAM compaction and flash-capacity policy: PASS",
        ):
            if marker not in compaction_policy + compaction_test:
                failures.append("NVRAM compaction regression is missing " + marker)

        rehash_body = extract_function(kernel_common, "nvram_rehash")
        init_body = extract_function(kernel_common, "_nvram_init")
        if "nvram_rehash_records(" not in rehash_body or "data_end_offset + 2" not in rehash_body:
            failures.append("NVRAM rehash must parse only the validated declared image span")
        if "nvram_rehash_rollback" not in rehash_body or "nvram_free();" not in kernel_common:
            failures.append("NVRAM rehash failure must clear every partially inserted tuple")
        validate_end = init_body.index("nvram_flash_data_end_offset(")
        crc_check = init_body.index("nvram_calc_crc(header)")
        rehash_call = init_body.index("nvram_rehash(header, data_end_offset)")
        if not validate_end < crc_check < rehash_call:
            failures.append("NVRAM image bounds must be validated before CRC and bounded rehash")
        if "rehash_status == NVRAM_REHASH_INVALID" not in init_body or "rehash_status != 0" not in init_body:
            failures.append("NVRAM initialization must reject malformed records and allocation failures")
        for marker in (
            "test_malformed_record_rolls_back_prior_entries",
            "test_allocation_failure_rolls_back",
            "test_missing_double_nul_is_rejected",
            "NVRAM flash rehash validation and rollback policy: PASS",
        ):
            if marker not in rehash_policy + rehash_test:
                failures.append("NVRAM flash rehash regression is missing " + marker)
        driver_init_body = extract_function(kernel_driver, "dev_nvram_init")
        header_alloc = driver_init_body.index("header = kzalloc(NVRAM_SPACE")
        header_failure = driver_init_body.index("if (!header)", header_alloc)
        flash_read = driver_init_body.index("ra_mtd_read_nm(", header_failure)
        image_failure = driver_init_body.index("if (ret != 0)", flash_read)
        rehash_failure = driver_init_body.index("if (check_res == NVRAM_REHASH_INIT_NOMEM)", image_failure)
        if not (header_alloc < header_failure < flash_read < image_failure < rehash_failure):
            failures.append("NVRAM device must fail initialization instead of exposing an empty device after RAM/flash read/rehash failure")
        for marker in (
            "nvram_flash_data_end_offset",
            "test_flash_rehash_is_bounded_by_declared_image_length",
        ):
            if marker not in compaction_policy + compaction_test:
                failures.append("NVRAM flash-image boundary regression is missing " + marker)

        exit_body = extract_function(kernel_driver, "dev_nvram_exit")
        unregister = exit_body.index("unregister_chrdev(nvram_major, MTD_NVRAM_NAME);")
        uninit = exit_body.index("_nvram_uninit();")
        pool_free = exit_body.index("kfree(nvram_values);")
        if not unregister < uninit < pool_free:
            failures.append("NVRAM exit must unregister readers before tearing down the value pool")
    except (ValueError, IndexError) as error:
        failures.append("NVRAM snapshot source contract: " + str(error))

    # .owner and explicit open/release references pin the module while ioctl
    # readers may use nvram_values; the exit path is not a live reader.
    if "owner:\t\tTHIS_MODULE" not in kernel_driver:
        failures.append("NVRAM file operations no longer pin the driver module while open")
    if "try_module_get(THIS_MODULE);" not in kernel_driver or "module_put(THIS_MODULE);" not in kernel_driver:
        failures.append("NVRAM open/release module reference pairing is missing")

    # Kernel NVRAM scratch buffers can contain credentials; scrub before release.
    for marker in (
        "NVRAM_SENSITIVE_BUFFER_ZERO",
        "memzero_explicit((buffer), (length))",
        "if (buffer && length)",
        "NVRAM_SENSITIVE_BUFFER_ZERO(buffer, length)",
    ):
        if marker not in sensitive_policy:
            failures.append("NVRAM sensitive-buffer policy is missing " + marker)
    for marker in (
        "test_sensitive_buffer_clear_wipes_the_complete_region",
        "test_sensitive_buffer_clear_ignores_null_and_empty_regions",
        "sensitive_zero_bytes == sizeof(secret)",
        "NVRAM_SENSITIVE_BUFFER_ZERO(buffer, length)",
    ):
        if marker not in compaction_test:
            failures.append("NVRAM sensitive-buffer host regression is missing " + marker)
    try:
        compact_body = extract_function(kernel_driver, "nvram_compact_values")
        commit_body = extract_function(kernel_driver, "nvram_commit")
        set_ioctl_body = extract_function(kernel_driver, "user_nvram_set")
        get_ioctl_body = extract_function(kernel_driver, "user_nvram_get")
        init_body = extract_function(kernel_driver, "dev_nvram_init")
        reset_body = extract_function(kernel_driver, "_nvram_reset")
        exit_body = extract_function(kernel_driver, "dev_nvram_exit")
        for body, wipe, free, label in (
            (compact_body, "nvram_sensitive_buffer_clear(snapshot, old_used)", "kfree(snapshot)", "compaction snapshot"),
            (commit_body, "nvram_sensitive_buffer_clear(bufw, NVRAM_SPACE)", "kfree(bufw)", "commit image"),
        ):
            if wipe not in body or free not in body or body.index(wipe) > body.index(free):
                failures.append(f"NVRAM {label} must be erased before kfree")
        if ("nvram_sensitive_buffer_clear(bufr, NVRAM_SPACE)" not in commit_body or
            commit_body.index("nvram_sensitive_buffer_clear(bufr, NVRAM_SPACE)") > commit_body.index("kfree(bufr)")):
            failures.append("NVRAM flash read-back image must be erased before kfree")
        for wipe in ("nvram_sensitive_buffer_clear(value, (size_t)request.len_value + 1U)",
                     "nvram_sensitive_buffer_clear(tmp, sizeof(tmp))"):
            if wipe not in set_ioctl_body:
                failures.append("NVRAM SET staging data is not erased: " + wipe)
        for wipe in ("nvram_sensitive_buffer_clear(value, NVRAM_SPACE)",
                     "nvram_sensitive_buffer_clear(value, (size_t)len)"):
            if wipe not in get_ioctl_body:
                failures.append("NVRAM GET staging data is not erased: " + wipe)
        header_wipes = [match.start() for match in re.finditer(r"nvram_sensitive_buffer_clear\(header, NVRAM_SPACE\);", init_body)]
        header_frees = [match.start() for match in re.finditer(r"kfree\(header\);", init_body)]
        if (len(header_wipes) != 2 or len(header_frees) != 2 or
            any(wipe > free for wipe, free in zip(header_wipes, header_frees))):
            failures.append("NVRAM flash initialization image must be erased on both exits")
        exit_wipe = exit_body.find("nvram_sensitive_buffer_clear(nvram_values, NVRAM_VALUES_SPACE)")
        exit_free = exit_body.find("kfree(nvram_values)")
        if exit_wipe < 0 or exit_free < 0 or exit_wipe > exit_free:
            failures.append("NVRAM value arena must be erased before module-exit kfree")
        if "nvram_sensitive_buffer_clear(nvram_values, NVRAM_VALUES_SPACE)" not in reset_body:
            failures.append("NVRAM arena reset must use explicit erasure")
        if "nvram_sensitive_buffer_clear(nvram_values, NVRAM_VALUES_SPACE)" not in compact_body:
            failures.append("NVRAM arena compaction must use explicit erasure")
    except ValueError as error:
        failures.append("NVRAM sensitive-buffer source contract: " + str(error))
    print("NVRAM kernel single-value ioctl snapshot contract: PASS")

    # Report board-specific selectors that change NVRAM MTD geometry.
    board_files = [
        path for path in sorted(BOARD_ROOT.rglob("*"))
        if path.is_file() and path.suffix.lower() in {".config", ".cfg"}
    ] if BOARD_ROOT.is_dir() else []
    print(f"BOARD_CONFIG_FILES_SCANNED: {len(board_files)}")
    for path in board_files:
        config = path.read_text(encoding="utf-8", errors="replace")
        selectors = [
            name for name in (
                "CONFIG_MTD_NAND_RALINK",
                "CONFIG_MTD_NAND_MTK",
                "CONFIG_MTD_NAND_USE_UBI_PART",
            )
            if re.search(r"(?m)^\s*" + name + r"=y\s*$", config)
        ]
        if selectors:
            print(
                f"BOARD_NVRAM_GEOMETRY: {path.relative_to(ROOT).as_posix()}: "
                + ",".join(selectors)
            )
    if not BOARD_ROOT.is_dir():
        failures.append("board configuration directory is missing")

    if failures:
        for failure in failures:
            print(f"FAIL: {failure}", file=sys.stderr)
        return 1

    print("NVRAM raw-userland boundary, wrapper-lock contracts and kernel caller inventory: PASS")
    print(
        "Scope note: shared-userland flock coverage does not imply kernel API "
        "or external-process global atomicity."
    )
    return 0


if __name__ == "__main__":
    self_test()
    sys.exit(run())
