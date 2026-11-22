#!/usr/bin/env python3
"""Guard libdisk share-list output initialization and Samba error cleanup."""

from pathlib import Path

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
):
    require(marker in REGRESSION and marker + "();" in REGRESSION,
            "missing output initialization regression: " + marker)

print("libdisk share-list output initialization and Samba cleanup regression: PASS")
