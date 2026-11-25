#!/usr/bin/env python3
"""Regression tests for atomic/retryable musl sys/queue.h installation."""
from __future__ import annotations

import os
import shutil
import socket
import subprocess
import tempfile
import threading
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
INSTALLER = ROOT / "scripts/install_musl_queue_header.sh"
EXPECTED = b"/* complete sys/queue.h fixture */\n#define QUEUE_FIXTURE 1\n"


def run_installer(url: str, target: Path, *, env: dict[str, str] | None = None) -> subprocess.CompletedProcess[str]:
    process_env = os.environ.copy()
    if env:
        process_env.update(env)
    return subprocess.run(
        [str(INSTALLER), url, str(target)],
        cwd=ROOT,
        env=process_env,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        timeout=30,
        check=False,
    )


def test_partial_transfer_is_retried_and_atomically_published() -> None:
    class Handler(BaseHTTPRequestHandler):
        attempts = 0

        def do_GET(self) -> None:  # noqa: N802 - BaseHTTPRequestHandler API
            Handler.attempts += 1
            self.send_response(200)
            if Handler.attempts == 1:
                self.send_header("Content-Length", str(len(EXPECTED) + 64))
                self.end_headers()
                try:
                    self.wfile.write(b"partial-header")
                    self.wfile.flush()
                    self.connection.shutdown(socket.SHUT_RDWR)
                    self.connection.close()
                except OSError:
                    pass
                return
            self.send_header("Content-Length", str(len(EXPECTED)))
            self.end_headers()
            self.wfile.write(EXPECTED)

        def log_message(self, _format: str, *_args: object) -> None:
            pass

    Handler.attempts = 0
    server = ThreadingHTTPServer(("127.0.0.1", 0), Handler)
    server.daemon_threads = True
    thread = threading.Thread(target=server.serve_forever, daemon=True)
    thread.start()
    try:
        with tempfile.TemporaryDirectory(prefix="queue-header-retry-") as temp:
            target = Path(temp) / "sys" / "queue.h"
            target.parent.mkdir()
            target.write_bytes(b"previous-known-good-header\n")
            url = f"http://127.0.0.1:{server.server_port}/queue.h"
            result = run_installer(url, target)
            assert result.returncode == 0, result.stderr
            assert Handler.attempts >= 2, Handler.attempts
            assert target.read_bytes() == EXPECTED
            assert target.stat().st_mode & 0o777 == 0o644
            assert not list(target.parent.glob("queue.h.tmp.*"))
    finally:
        server.shutdown()
        server.server_close()
        thread.join(timeout=5)


def make_fake_curl(directory: Path) -> Path:
    fake = directory / "curl"
    fake.write_text(
        "#!/bin/sh\n"
        "printf '%s\\n' \"$@\" > \"$CURL_ARGUMENTS\"\n"
        "out=\n"
        "while [ $# -gt 0 ]; do\n"
        "  if [ \"$1\" = -o ]; then shift; out=$1; break; fi\n"
        "  shift\n"
        "done\n"
        "printf '%s\\n' \"$out\" > \"$CURL_OUTPUT_ARG\"\n"
        "case $CURL_MODE in\n"
        "  partial-failure) printf '%s' partial > \"$out\"; exit 56 ;;\n"
        "  empty-success) : > \"$out\"; exit 0 ;;\n"
        "  *) printf '%s\\n' 'unexpected curl mode' >&2; exit 2 ;;\n"
        "esac\n",
        encoding="utf-8",
    )
    fake.chmod(0o755)
    return fake


def test_failed_transfer_preserves_existing_header_and_cleans_partial_file() -> None:
    with tempfile.TemporaryDirectory(prefix="queue-header-failure-") as temp:
        root = Path(temp)
        fake_bin = root / "bin"
        fake_bin.mkdir()
        make_fake_curl(fake_bin)
        target_dir = root / "sys"
        target_dir.mkdir()
        target = target_dir / "queue.h"
        target.write_bytes(b"known-good-header\n")
        output_arg = root / "curl-output-arg"
        curl_arguments = root / "curl-arguments"
        env = {
            "PATH": f"{fake_bin}{os.pathsep}{os.environ.get('PATH', '')}",
            "CURL_MODE": "partial-failure",
            "CURL_OUTPUT_ARG": str(output_arg),
            "CURL_ARGUMENTS": str(curl_arguments),
        }
        result = run_installer("https://example.invalid/queue.h", target, env=env)
        assert result.returncode == 56, (result.returncode, result.stderr)
        assert target.read_bytes() == b"known-good-header\n"
        assert not list(target_dir.glob("queue.h.tmp.*"))
        assert output_arg.read_text(encoding="utf-8").strip()
        args = curl_arguments.read_text(encoding="utf-8").splitlines()
        assert "--retry" in args and args[args.index("--retry") + 1] == "5"
        assert "--retry-all-errors" in args


def test_empty_success_response_is_rejected_without_replacing_header() -> None:
    with tempfile.TemporaryDirectory(prefix="queue-header-empty-") as temp:
        root = Path(temp)
        fake_bin = root / "bin"
        fake_bin.mkdir()
        make_fake_curl(fake_bin)
        target_dir = root / "sys"
        target_dir.mkdir()
        target = target_dir / "queue.h"
        target.write_bytes(b"known-good-header\n")
        output_arg = root / "curl-output-arg"
        curl_arguments = root / "curl-arguments"
        env = {
            "PATH": f"{fake_bin}{os.pathsep}{os.environ.get('PATH', '')}",
            "CURL_MODE": "empty-success",
            "CURL_OUTPUT_ARG": str(output_arg),
            "CURL_ARGUMENTS": str(curl_arguments),
        }
        result = run_installer("https://example.invalid/queue.h", target, env=env)
        assert result.returncode != 0
        assert target.read_bytes() == b"known-good-header\n"
        assert not list(target_dir.glob("queue.h.tmp.*"))


def test_root_makefile_uses_atomic_installer_and_workflow_watches_it() -> None:
    makefile = (ROOT / "Makefile").read_text(encoding="utf-8")
    workflow = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text(encoding="utf-8")
    header_target = makefile.split("prepare-headers:", 1)[1].split("else\nprepare-headers:", 1)[0]
    assert 'if [ ! -s "$(QUEUE_H)" ]' in header_target
    assert '$(TOPDIR)/scripts/install_musl_queue_header.sh' in header_target
    assert "curl -fL" not in header_target
    for pattern in (
        '- "Makefile"',
        '- "scripts/install_musl_queue_header.sh"',
        '- "scripts/tests/test_install_musl_queue_header.py"',
    ):
        assert workflow.count(pattern) == 1, pattern
    assert "Verify atomic musl sys/queue.h installation" in workflow


def main() -> None:
    assert shutil.which("curl"), "curl is required by the firmware build and this regression"
    test_partial_transfer_is_retried_and_atomically_published()
    test_failed_transfer_preserves_existing_header_and_cleans_partial_file()
    test_empty_success_response_is_rejected_without_replacing_header()
    test_root_makefile_uses_atomic_installer_and_workflow_watches_it()
    print("musl sys/queue.h retry/atomic publication regression: PASS")


if __name__ == "__main__":
    main()
