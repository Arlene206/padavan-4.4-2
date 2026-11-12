#!/usr/bin/env python3
"""Run the packaged MIPSel daemon's harmless CLI probes under QEMU."""

from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
BINARY = ROOT / "trunk/user/aliyundrive-webdav/aliyundrive-webdav"


def run_probe(emulator: str, option: str) -> str:
    result = subprocess.run(
        [emulator, str(BINARY), option],
        check=False,
        capture_output=True,
        text=True,
        timeout=20,
    )
    output = (result.stdout or "") + (result.stderr or "")
    if result.returncode != 0:
        raise SystemExit(
            f"bundled daemon {option} probe failed (exit={result.returncode}):\n"
            f"{output[-6000:]}"
        )
    return output


def main() -> int:
    emulator = shutil.which("qemu-mipsel-static") or shutil.which("qemu-mipsel")
    if not emulator:
        raise SystemExit("qemu-user installed but no qemu-mipsel emulator was found")
    version = run_probe(emulator, "--version")
    help_text = run_probe(emulator, "--help")
    print(f"QEMU emulator: {emulator}")
    print(f"bundled daemon --version: {version.strip()}")
    print("bundled daemon --help:")
    print(help_text.rstrip())

    required_options = (
        "-I", "--host", "--root", "-p", "-r", "-U", "-W", "-S",
        "--cache-size", "--cache-ttl", "--workdir", "--domain-id",
        "--no-trash", "--read-only",
    )
    missing = [option for option in required_options if option not in help_text]
    if missing:
        raise SystemExit(f"bundled daemon help is missing launcher-required options: {missing}")

    env_names = ("REFRESH_TOKEN", "WEBDAV_AUTH_USER", "WEBDAV_AUTH_PASSWORD")
    advertised_env = [name for name in env_names if name in help_text]
    print(f"credential environment names advertised by --help: {advertised_env}")
    print("AliyunDrive bundled MIPSel --version/--help compatibility: PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
