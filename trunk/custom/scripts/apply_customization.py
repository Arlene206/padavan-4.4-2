#!/usr/bin/env python3
"""Apply validated router defaults without constructing sed commands from input."""

import errno
import ipaddress
import json
import os
import re
import stat
import sys
import tempfile
from pathlib import Path


ALLOWED_KEYS = {
    "lanip", "signaccount", "signpassword",
    "wifi2gssid", "wifi2gpsk", "wifi5gssid", "wifi5gpsk",
}


def fail(message):
    raise SystemExit("ERROR: " + message)


def c_string(value):
    # Escape '?' so user input cannot form a C trigraph before tokenization.
    return '"' + value.replace("\\", "\\\\").replace('"', '\\"').replace("?", "\\?") + '"'


def validate_text(key, value):
    if not isinstance(value, str):
        fail(f"{key} must be a string")
    if any(ord(ch) < 32 or ord(ch) == 127 for ch in value):
        fail(f"{key} contains control characters")


def replace_define(text, name, value):
    pattern = re.compile(r"(?m)^(#define\s+" + re.escape(name) + r"\s+).*$")
    updated, count = pattern.subn(lambda m: m.group(1) + c_string(value), text, count=1)
    if count != 1:
        fail(f"could not locate exactly one #define {name}")
    return updated



def _read_regular_text(path):
    """Read a regular file without following a final-component symlink."""
    flags = os.O_RDONLY | getattr(os, "O_NOFOLLOW", 0)
    fd = os.open(str(path), flags)
    try:
        info = os.fstat(fd)
        if not stat.S_ISREG(info.st_mode):
            raise OSError(errno.EINVAL, "target is not a regular file", str(path))
        stream = os.fdopen(fd, "r", encoding="utf-8")
        fd = -1
        with stream:
            return stream.read(), stat.S_IMODE(info.st_mode)
    finally:
        if fd >= 0:
            os.close(fd)


def _stage_text_file(target, contents, mode):
    """Write and fsync a replacement beside its target before publication."""
    fd, temp_name = tempfile.mkstemp(
        prefix="." + target.name + ".",
        suffix=".tmp",
        dir=str(target.parent),
    )
    try:
        os.fchmod(fd, mode)
        stream = os.fdopen(fd, "w", encoding="utf-8")
        fd = -1
        with stream:
            stream.write(contents)
            stream.flush()
            os.fsync(stream.fileno())
    except Exception:
        if fd >= 0:
            os.close(fd)
        try:
            os.unlink(temp_name)
        except OSError:
            pass
        raise
    return Path(temp_name)


def _sync_directory(path):
    flags = os.O_RDONLY | getattr(os, "O_DIRECTORY", 0)
    fd = os.open(str(path), flags)
    try:
        os.fsync(fd)
    finally:
        os.close(fd)


def _remove_temp(path):
    if path is None:
        return
    try:
        os.unlink(str(path))
    except FileNotFoundError:
        pass
    except OSError:
        # A leftover private temp is safer than masking the original failure.
        pass


def atomic_publish_text_files(changes):
    """
    Stage new contents and rollback copies before atomically replacing targets.

    A reported replace or directory-sync failure restores published targets in
    reverse order. If restoration fails, its private original snapshot remains
    available for manual recovery. Multiple renames are not crash-atomic as a
    single operation.
    """
    entries = []
    seen = set()
    directories = []
    try:
        for target_value, new_contents in changes:
            target = Path(target_value)
            key = os.path.normcase(os.path.abspath(str(target)))
            if key in seen:
                raise ValueError("duplicate customization target: " + str(target))
            seen.add(key)
            if not isinstance(new_contents, str):
                raise TypeError("customization contents must be text")

            old_contents, mode = _read_regular_text(target)
            new_temp = _stage_text_file(target, new_contents, mode)
            entry = {
                "target": target,
                "new_temp": new_temp,
                "backup_temp": None,
                "preserve_backup": False,
            }
            entries.append(entry)
            entry["backup_temp"] = _stage_text_file(target, old_contents, mode)
            if target.parent not in directories:
                directories.append(target.parent)

        # Make the rollback snapshots durable before changing any target.
        for directory in directories:
            _sync_directory(directory)
    except Exception:
        for entry in entries:
            _remove_temp(entry["new_temp"])
            _remove_temp(entry["backup_temp"])
        raise

    published = []
    try:
        for entry in entries:
            os.replace(str(entry["new_temp"]), str(entry["target"]))
            entry["new_temp"] = None
            published.append(entry)
        for directory in directories:
            _sync_directory(directory)
    except Exception as publish_error:
        rollback_failures = []
        for entry in reversed(published):
            try:
                os.replace(str(entry["backup_temp"]), str(entry["target"]))
                entry["backup_temp"] = None
            except Exception as rollback_error:
                entry["preserve_backup"] = True
                rollback_failures.append(
                    "{}: {}; snapshot retained at {}".format(
                        entry["target"], rollback_error, entry["backup_temp"]
                    )
                )
        for directory in directories:
            try:
                _sync_directory(directory)
            except Exception as sync_error:
                rollback_failures.append(
                    "{}: rollback directory sync failed: {}".format(
                        directory, sync_error
                    )
                )
        if rollback_failures:
            raise RuntimeError(
                "customization publication failed ({}); rollback incomplete: {}".format(
                    publish_error, "; ".join(rollback_failures)
                )
            ) from publish_error
        raise
    finally:
        for entry in entries:
            _remove_temp(entry["new_temp"])
            if not entry["preserve_backup"]:
                _remove_temp(entry["backup_temp"])

def main():
    if len(sys.argv) != 3:
        fail("usage: apply_customization.py DEFAULTS_H CN_DICT")

    data = json.loads(os.environ.get("CUSTOMIZATION_JSON", "{}"))
    if not isinstance(data, dict):
        fail("customization JSON must be an object")
    unknown = set(data) - ALLOWED_KEYS
    if unknown:
        fail("unknown customization keys: " + ", ".join(sorted(unknown)))

    for key, value in data.items():
        validate_text(key, value)

    macros = {}
    lanip = data.get("lanip", "")
    if lanip:
        try:
            addr = ipaddress.IPv4Address(lanip)
        except ipaddress.AddressValueError:
            fail("lanip must be a valid IPv4 address")
        octets = lanip.split(".")
        host = int(octets[3])
        if host < 1 or host > 254 or 100 <= host <= 244:
            fail("lanip host must be .1-.99 or .245-.254 (DHCP pool is .100-.244)")
        prefix = ".".join(octets[:3])
        macros["DEF_LAN_ADDR"] = lanip
        macros["DEF_LAN_DHCP_BEG"] = prefix + ".100"
        macros["DEF_LAN_DHCP_END"] = prefix + ".244"

    account = data.get("signaccount", "")
    if account:
        if len(account) > 32 or not re.fullmatch(r"[A-Za-z0-9_-]+", account):
            fail("signaccount must be 1-32 letters, digits, '_' or '-'")
        macros["SYS_USER_ROOT"] = account

    password = data.get("signpassword", "")
    if password:
        if len(password) > 128 or not password.isascii() or not password.isprintable():
            fail("signpassword must be at most 128 printable ASCII characters")
        macros["DEF_ROOT_PASSWORD"] = password

    for key, macro in (
        ("wifi2gssid", "DEF_WLAN_2G_SSID"),
        ("wifi5gssid", "DEF_WLAN_5G_SSID"),
    ):
        value = data.get(key, "")
        if value:
            if len(value.encode("utf-8")) > 32:
                fail(f"{key} must be at most 32 UTF-8 bytes")
            macros[macro] = value

    for key, macro in (
        ("wifi2gpsk", "DEF_WLAN_2G_PSK"),
        ("wifi5gpsk", "DEF_WLAN_5G_PSK"),
    ):
        value = data.get(key, "")
        if value:
            if not value.isascii() or not value.isprintable():
                fail(f"{key} must contain printable ASCII")
            if not (8 <= len(value) <= 63 or
                    (len(value) == 64 and re.fullmatch(r"[0-9A-Fa-f]{64}", value))):
                fail(f"{key} must be 8-63 printable characters or a 64-digit hex PSK")
            macros[macro] = value

    defaults_path = Path(sys.argv[1])
    dict_path = Path(sys.argv[2])
    defaults, _defaults_mode = _read_regular_text(defaults_path)
    dictionary = _read_regular_text(dict_path)[0] if lanip else None

    for name, value in macros.items():
        defaults = replace_define(defaults, name, value)

    if lanip:
        if "192.168.2.1" not in dictionary:
            fail("default LAN address not found in dictionary")
        dictionary = dictionary.replace("192.168.2.1", lanip)

    changes = [(defaults_path, defaults)]
    if lanip:
        if os.path.abspath(str(defaults_path)) == os.path.abspath(str(dict_path)):
            fail("defaults header and language dictionary must be different files")
        changes.append((dict_path, dictionary))

    try:
        atomic_publish_text_files(changes)
    except (OSError, RuntimeError, ValueError, TypeError) as error:
        fail("could not publish customization files: " + str(error))

    print(f"Applied {len(macros)} validated firmware default(s).")


if __name__ == "__main__":
    main()
