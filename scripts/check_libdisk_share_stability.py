#!/usr/bin/env python3
"""Guard libdisk share-list output initialization and Samba error cleanup."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
LIBDISK = ROOT / "trunk/user/libdisk/src"
DISK_SOURCE = (LIBDISK / "disk_share.c").read_text()
POLICY = (LIBDISK / "disk_share_list_policy.h").read_text()
REGRESSION = (LIBDISK / "tests/disk_share_list_policy.c").read_text()
SAMBA_SOURCE = (ROOT / "trunk/user/rc/src/services_stor.c").read_text()


def function_body(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    state = "code"
    quote = ""
    index = opening
    while index < len(source):
        char = source[index]
        next_char = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == "/" and next_char == "*":
                state = "block_comment"
                index += 2
                continue
            if char == "/" and next_char == "/":
                state = "line_comment"
                index += 2
                continue
            if char in ("'", '"'):
                state = "quoted"
                quote = char
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[opening + 1:index]
        elif state == "quoted":
            if char == "\\":
                index += 2
                continue
            if char == quote:
                state = "code"
        elif state == "block_comment":
            if char == "*" and next_char == "/":
                state = "code"
                index += 2
                continue
        elif state == "line_comment" and char == "\n":
            state = "code"
        index += 1
    raise AssertionError("unterminated function: " + signature)


def require(condition, message):
    if not condition:
        raise SystemExit("libdisk share-list contract: FAIL: " + message)


require('#include "disk_share_list_policy.h"' in DISK_SOURCE,
        "disk_share.c must include the output-preparation policy")
require("disk_share_list_prepare_outputs" in POLICY,
        "shared output-preparation policy is missing")

for signature in (
    "int get_folder_list_in_mount_path(const char *const mount_path, int *sh_num, char ***folder_list)",
    "int get_all_folder_in_mount_path(const char *const mount_path, int *sh_num, char ***folder_list)",
):
    body = function_body(DISK_SOURCE, signature)
    prepare = body.index("disk_share_list_prepare_outputs(mount_path, sh_num, folder_list)")
    use = body.index("strlen(mount_path)") if "strlen(mount_path)" in body else body.index("opendir(mount_path)")
    require(prepare < use,
            signature + " must reset outputs before using the mount path")


require("disk_share_list_next_entry" in POLICY,
        "shared readdir status policy is missing")
all_folders = function_body(DISK_SOURCE,
                            "int get_all_folder_in_mount_path(const char *const mount_path, int *sh_num, char ***folder_list)")
compact_all_folders = " ".join(all_folders.split())
read_entry_match = re.search(
    r"disk_share_list_next_entry\(\s*pool_to_open,\s*readdir,\s*&dp,\s*&read_error\s*\)",
    compact_all_folders,
)
require(read_entry_match is not None,
        "directory loop must use the checked readdir adapter")
read_entry = read_entry_match.start()
eof_gate = compact_all_folders.index("if (read_status == 0)", read_entry)
error_gate = compact_all_folders.index("if (read_status < 0)", eof_gate)
errno_restore = compact_all_folders.index(
    "errno = read_error != 0 ? read_error : EIO;", error_gate
)
require(read_entry < eof_gate < error_gate < errno_restore,
        "directory iteration must distinguish clean EOF from readdir errors")
failure_cleanup = compact_all_folders[compact_all_folders.index("failed:"):]
require("saved_errno = errno ? errno : EIO;" in failure_cleanup and
        "errno = saved_errno;" in failure_cleanup,
        "directory read/close failures must preserve errno during cleanup")


require("disk_share_list_write_snapshot" in POLICY and
        "if (fflush(stream) != 0)" in POLICY,
        "snapshot writer must report buffered stream failures")
initial_writer = function_body(
    DISK_SOURCE, "int initial_folder_list_in_mount_path(const char *const mount_path)"
)
mkstemp = initial_writer.index("fd = mkstemp(temp_file)")
snapshot_write = initial_writer.index(
    "disk_share_list_write_snapshot(fp, sh_num, folder_list)", mkstemp
)
sync_file = initial_writer.index("fsync(fileno(fp))", snapshot_write)
close_file = initial_writer.index("if (fclose(fp) != 0)", sync_file)
publish_file = initial_writer.index("rename(temp_file, list_file)", close_file)
require(mkstemp < snapshot_write < sync_file < close_file < publish_file,
        "folder-list metadata must be flushed and closed before atomic publish")
require('fopen(list_file, "w")' not in initial_writer,
        "folder-list refresh must not truncate the last complete snapshot")
write_failure = initial_writer[initial_writer.index("failed:"):]
require("unlink(temp_file)" in write_failure and
        "errno = saved_errno;" in write_failure,
        "failed snapshot writes must remove the temporary file and preserve errno")


require("disk_share_write_account_defaults" in POLICY and
        "if (fflush(stream) != 0)" in POLICY,
        "account defaults writer must report buffered stream failures")
account_writer = function_body(
    DISK_SOURCE,
    "int initial_one_var_file_in_mount_path(const char *const account,"
)
account_mkstemp = account_writer.index("fd = mkstemp(temp_file)")
account_content = account_writer.index(
    "disk_share_write_account_defaults(fp, sh_num, folder_list",
    account_mkstemp
)
account_sync = account_writer.index("fsync(fileno(fp))", account_content)
account_close = account_writer.index("if (fclose(fp) != 0)", account_sync)
account_publish = account_writer.index("rename(temp_file, var_file)", account_close)
require(account_mkstemp < account_content < account_sync < account_close < account_publish,
        "account defaults must be fully written and closed before atomic publish")
account_failure = account_writer[account_writer.index("failed:"):]
require("unlink(temp_file)" in account_failure and
        "errno = saved_errno;" in account_failure,
        "failed account defaults writes must discard the temporary file")
require('fopen(var_file, "w")' not in account_writer,
        "account defaults must not truncate the last complete permission file")

samba = function_body(SAMBA_SOURCE, "write_smb_conf_unlocked(")
require("char **folder_list = NULL;" in samba,
        "Samba caller must initialize folder_list before the fallible API call")
call = samba.index("get_folder_list_in_mount_path(")
failure = samba.index("free_2_dimension_list(&sh_num, &folder_list);", call)
require(call < failure and "result = -1;" in samba[failure:],
        "Samba failure path must release the initialized list output")

for marker in (
    "test_invalid_mount_paths_clear_outputs",
    "test_missing_output_parameters_are_safe",
    "test_valid_mount_path_starts_with_empty_outputs",
    "test_readdir_entry_and_clean_eof",
    "test_readdir_error_is_not_clean_eof",
    "test_readdir_rejects_missing_adapters",
    "test_snapshot_write_flushes_complete_content",
    "test_snapshot_write_reports_io_failure",
    "test_snapshot_write_rejects_invalid_data",
    "test_account_defaults_write_complete_content",
    "test_account_defaults_write_reports_io_failure",
    "test_account_defaults_write_rejects_invalid_data",
):
    require(marker in REGRESSION and marker + "();" in REGRESSION,
            "missing output initialization regression: " + marker)



# Changes to share-list code and its installed public headers must schedule the XY-C1 build.
XY_WORKFLOW = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()
from check_xy_c1_firmware_inputs import is_non_firmware_input
for path in (
    "trunk/user/libdisk/src/Makefile",
    "trunk/user/libdisk/src/disk_share.c",
    "trunk/user/libdisk/src/disk_share.h",
    "trunk/user/libdisk/src/disk_share_list_policy.h",
):
    require(XY_WORKFLOW.count('      - "' + path + '"') == 1 and
            not is_non_firmware_input(path),
            "XY-C1 firmware path filter must watch production libdisk input: " + path)

print("libdisk share-list enumeration and Samba cleanup regression: PASS")
