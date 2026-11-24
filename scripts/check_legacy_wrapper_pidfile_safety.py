#!/usr/bin/env python3
"""Prevent stale-pidfile signals and plaintext credential exposure in legacy wrappers."""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
SOURCES = (
    ROOT / "trunk/user/scutclient/scripts/scutclient.sh",
    ROOT / "trunk/user/mentohust/scripts/mentohust.sh",
)
TEST = ROOT / "scripts/tests/legacy_wrapper_pidfile_safety.sh"

for source in SOURCES:
    content = source.read_text()
    if "pidfile" in content or re.search(r"\bkill\s+-9\b", content):
        raise SystemExit(
            "legacy wrapper pidfile safety: FAIL: unsafe stale PID handling remains in "
            + str(source.relative_to(ROOT))
        )

pidfile_source = (ROOT / "trunk/user/shared/src/shutils.c").read_text()
pidfile_policy = (ROOT / "trunk/user/shared/src/shutils_pidfile_policy.h").read_text()
pidfile_test = (ROOT / "trunk/user/shared/tests/shutils_pidfile_open.c").read_text()
if "#include <sys/stat.h>" not in pidfile_policy:
    raise SystemExit("legacy pidfile open: FAIL: metadata policy header is not self-contained")
if "shutils_pidfile_metadata_is_safe(&st, geteuid())" not in pidfile_source:
    raise SystemExit("legacy pidfile open: FAIL: privileged signalling skips owner/metadata validation")
for token in ("(status->st_mode & S_IFMT) == S_IFREG", "status->st_uid == owner", "status->st_nlink == 1", "(status->st_mode & 022) == 0"):
    if token not in pidfile_policy:
        raise SystemExit("legacy pidfile open: FAIL: owner/mode/link policy missing: " + token)
if "metadata_policy_rejects_unsafe_files" not in pidfile_test:
    raise SystemExit("legacy pidfile open regression: FAIL: unsafe metadata cases are not tested")
if "fd = shutils_pidfile_open(pidfile);" not in pidfile_source:
    raise SystemExit("legacy pidfile open: FAIL: production helper does not use the no-follow path")
if "O_NOFOLLOW" not in pidfile_policy or "O_NONBLOCK" not in pidfile_policy:
    raise SystemExit("legacy pidfile open: FAIL: symlink/FIFO-safe flags are missing")
for marker in (
    "regular_pidfile_opens",
    "symlink_pidfile_is_rejected",
    "errno == ELOOP",
    "shutils_pidfile_open",
):
    if marker not in pidfile_test:
        raise SystemExit("legacy pidfile open regression: FAIL: missing " + marker)

mentohust = SOURCES[1].read_text()
for token in (
    "umask 077",
    'mktemp "$conf_file.XXXXXX"',
    'chmod 600 "$conf_tmp"',
    'mv -f "$conf_tmp" "$conf_file"',
    'func_restrict_private_config "$bin_conf_file"',
    'if [ -L "$conf_file" ] || [ -d "$conf_file" ]; then',
    'case "$value" in',
    'newline=${newline%x}',
):
    if token not in mentohust:
        raise SystemExit(f"MentoHUST credential security: FAIL: missing private publication contract {token}")
if "sed -i" in mentohust:
    raise SystemExit("MentoHUST credential security: FAIL: unsafe sed-based config interpolation remains")

test = TEST.read_text()
for marker in (
    "2147483647",
    "scutclient wrapper consumed or rewrote",
    "mentohust wrapper consumed or rewrote",
    "generated MentoHUST credentials are not stored mode 0600",
    "existing MentoHUST config permissions were not restricted",
    "MentoHUST accepted a symlink config destination",
    "MentoHUST accepted a line-break-bearing setting",
    "MentoHUST accepted a carriage-return-bearing setting",
    r"printf 'invalid-secret\rInjected=1'",
    "SCUTClient credentials appeared in process argv",
    "SCUTClient accepted an unconsumed private credential file",
    "SCUTClient accepted a newline-bearing password",
    "SCUTClient accepted a carriage-return-bearing password",
    "Legacy wrappers preserve stale-pidfile safety and SCUTClient/MentoHUST credential confidentiality",
):
    if marker not in test:
        raise SystemExit(f"legacy wrapper security: FAIL: missing regression marker {marker}")

scutclient = SOURCES[0].read_text()
credentials_source = (ROOT / "trunk/user/scutclient/patches/scutclient_credentials.c").read_text()
credentials_patch = (ROOT / "trunk/user/scutclient/patches/0001-private-credentials-file.patch").read_text()
scutclient_makefile = (ROOT / "trunk/user/scutclient/Makefile").read_text()
for token in (
    'mktemp -d "${TMPDIR:-/tmp}/scutclient-auth.XXXXXX"',
    'chmod 700 "$scutclient_credentials_dir"',
    'chmod 600 "$scutclient_credentials_file"',
    'printf \'username=%s\\npassword=%s\\n\'',
    '--credentials-file "$scutclient_credentials_file"',
    'func_wait_credentials_consumed',
    "trap 'func_credentials_cleanup' 0",
    'scutclient_cr=$(printf \'\\r\')',
):
    if token not in scutclient:
        raise SystemExit(f"SCUTClient credential security: FAIL: missing wrapper contract {token}")
if '-u "$scutclient_username_value" -p "$scutclient_password_value"' in scutclient:
    raise SystemExit("SCUTClient credential security: FAIL: plaintext credentials remain in process argv")
for token in (
    "O_NOFOLLOW",
    "O_DIRECTORY",
    "AT_SYMLINK_NOFOLLOW",
    "openat(dirfd, basename",
    "fstatat(dirfd, basename",
    "unlinkat(dirfd, basename",
    "(directory_stat.st_mode & 07777) != 0700",
    "fstat(fd, &opened_stat)",
    "S_ISREG(opened_stat.st_mode)",
    "opened_stat.st_uid != geteuid()",
    "(opened_stat.st_mode & 07777) != 0600",
    "opened_stat.st_nlink != 1",
    "fstatat(dirfd, basename, &path_stat, AT_SYMLINK_NOFOLLOW)",
    "unlinkat(dirfd, basename, 0)",
    "SCUTCLIENT_CREDENTIALS_MAX 4096",
    "private parent directory",
    'memcmp(buffer, "username=",',
    'memcmp(pass_start, "password=",',
):
    if token not in credentials_source:
        raise SystemExit(f"SCUTClient credential parser: FAIL: missing private-file validation {token}")
for token in (
    '{"credentials-file", required_argument, NULL, \'F\'}',
    "scutclient_load_credentials_file(credentials_file, &UserName, &Password)",
    "src/scutclient_credentials.c",
):
    if token not in credentials_patch:
        raise SystemExit(f"SCUTClient upstream overlay: FAIL: missing patch contract {token}")
if not all(token in scutclient_makefile for token in (
    "apply_credentials_overlay: extract_test",
    "config_test: apply_credentials_overlay",
    "patch -d $(SRC_NAME) -p1",
    "scutclient_credentials.c",
)):
    raise SystemExit("SCUTClient package integration: FAIL: overlay is not applied before configuration")

print("Legacy wrapper stale-pidfile and SCUTClient/MentoHUST credential security contracts: PASS")
