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
    driver_path = ROOT / DRIVER
    driver_makefile = ROOT / "trunk/linux-4.4.x/drivers/nvram/Makefile"
    drivers_makefile = ROOT / "trunk/linux-4.4.x/drivers/Makefile"
    bcm_header = ROOT / "trunk/linux-4.4.x/include/nvram/bcmnvram.h"

    for required_path in (
        adapter_path, lock_path, driver_path, driver_makefile,
        drivers_makefile, bcm_header
    ):
        if not required_path.is_file():
            failures.append(f"required NVRAM source missing: {required_path}")
    if failures:
        for failure in failures:
            print(f"FAIL: {failure}", file=sys.stderr)
        return 1

    adapter = adapter_path.read_text(encoding="utf-8", errors="replace")
    lock_header = lock_path.read_text(encoding="utf-8", errors="replace")
    kernel_driver = driver_path.read_text(encoding="utf-8", errors="replace")

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

    transaction_contracts = (
        ("nvram_transaction_getall", "nvram_getall_unlocked"),
        ("nvram_transaction_set", "nvram_set_unlocked"),
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
        "flock(fd, operation | LOCK_NB)",
    )):
        failures.append(
            "shared NVRAM lock lost its stable path, no-follow/close-on-exec or bounded flock contract"
        )

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
        if "router_ready=(nvram_node_result==0&&init_router()==0);" not in main_body:
            failures.append("PID 1 does not block router startup when the NVRAM device is unsafe")
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

    # Preserve a known candidate as an explicit follow-up, not a false PASS.
    try:
        get_body = extract_function(kernel_driver, "user_nvram_get")
        if "value = nvram_get(param)" in get_body and "copy_to_user" in get_body:
            print(
                "KERNEL_FOLLOW_UP: user_nvram_get copies the nvram_get() tuple pointer "
                "after nvram_get releases nvram_lock; review snapshot lifetime "
                "against concurrent kernel writers."
            )
    except ValueError as error:
        failures.append(str(error))

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
