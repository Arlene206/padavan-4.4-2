#!/usr/bin/env python3
"""Compile production IPTC helpers against both handle APIs with errno-clobbering mocks."""

from pathlib import Path
import os
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[1]
MINIUPNPD = ROOT / "trunk/user/miniupnpd/miniupnpd-2.x"
SOURCE = (MINIUPNPD / "netfilter/iptcrdr.c").read_text()


def extract_function(signature: str) -> str:
    signature_at = SOURCE.index(signature)
    declaration_at = SOURCE.rfind("static int", 0, signature_at)
    if declaration_at < 0 or SOURCE[declaration_at:signature_at].strip() != "static int":
        raise AssertionError("could not locate static int declaration: " + signature)

    brace_at = SOURCE.index("{", signature_at)
    depth = 0
    state = "code"
    index = brace_at
    while index < len(SOURCE):
        char = SOURCE[index]
        next_char = SOURCE[index + 1] if index + 1 < len(SOURCE) else ""
        if state == "code":
            if char == '"':
                state = "string"
            elif char == "'":
                state = "char"
            elif char == "/" and next_char == "*":
                state = "block_comment"
                index += 1
            elif char == "/" and next_char == "/":
                state = "line_comment"
                index += 1
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return SOURCE[declaration_at:index + 1]
        elif state in ("string", "char"):
            if char == "\\":
                index += 1
            elif state == "string" and char == '"':
                state = "code"
            elif state == "char" and char == "'":
                state = "code"
        elif state == "block_comment":
            if char == "*" and next_char == "/":
                state = "code"
                index += 1
        elif state == "line_comment" and char == "\n":
            state = "code"
        index += 1
    raise AssertionError("unterminated function: " + signature)


DELETE_HELPER = extract_function(
    "delete_rule_and_commit(unsigned int index, IPTC_HANDLE h,"
)
INSERT_HELPER = extract_function(
    "insert_rule_and_commit(unsigned int index, const struct ipt_entry *entry,"
)

PREAMBLE = r"""
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <syslog.h>
#include <string.h>

#include "miniupnpd_redirect_cleanup_policy.h"

struct mock_iptc_handle {
    int marker;
};
typedef struct mock_iptc_handle *IPTC_HANDLE;

struct ipt_entry {
    unsigned int marker;
};

static struct mock_iptc_handle mock_handle;
static int mock_delete_ok;
static int mock_delete_errno;
static int mock_insert_ok;
static int mock_insert_errno;
static int mock_commit_ok;
static int mock_commit_errno;
static int mock_init_ok;
static int mock_init_errno;
static int delete_calls;
static int insert_calls;
static int commit_calls;
static int init_calls;
static int free_calls;
static int log_calls;

#ifdef IPTABLES_143
int iptc_delete_num_entry(const char *, unsigned int, IPTC_HANDLE);
int iptc_insert_entry(const char *, const struct ipt_entry *, unsigned int,
                      IPTC_HANDLE);
int iptc_commit(IPTC_HANDLE);
void iptc_free(IPTC_HANDLE);
#else
int iptc_delete_num_entry(const char *, unsigned int, IPTC_HANDLE *);
int iptc_insert_entry(const char *, const struct ipt_entry *, unsigned int,
                      IPTC_HANDLE *);
int iptc_commit(IPTC_HANDLE *);
void iptc_free(IPTC_HANDLE *);
#endif

IPTC_HANDLE iptc_init(const char *);
const char *iptc_strerror(int);
void syslog(int, const char *, ...);

static void reset_mocks(void)
{
    mock_handle.marker = 0x49505443;
    mock_delete_ok = 1;
    mock_delete_errno = 0;
    mock_insert_ok = 1;
    mock_insert_errno = 0;
    mock_commit_ok = 1;
    mock_commit_errno = 0;
    mock_init_ok = 1;
    mock_init_errno = 0;
    delete_calls = 0;
    insert_calls = 0;
    commit_calls = 0;
    init_calls = 0;
    free_calls = 0;
    log_calls = 0;
}

#ifdef IPTABLES_143
int iptc_delete_num_entry(const char *chain, unsigned int index,
                          IPTC_HANDLE handle)
{
    assert(chain != NULL);
    assert(index == 7);
    assert(handle == &mock_handle);
    ++delete_calls;
    if (!mock_delete_ok) {
        errno = mock_delete_errno;
        return 0;
    }
    return 1;
}

int iptc_insert_entry(const char *chain, const struct ipt_entry *entry,
                      unsigned int index, IPTC_HANDLE handle)
{
    assert(chain != NULL);
    assert(entry != NULL);
    assert(index == 9);
    assert(handle == &mock_handle);
    ++insert_calls;
    if (!mock_insert_ok) {
        errno = mock_insert_errno;
        return 0;
    }
    return 1;
}

int iptc_commit(IPTC_HANDLE handle)
{
    assert(handle == &mock_handle);
    ++commit_calls;
    if (!mock_commit_ok) {
        errno = mock_commit_errno;
        return 0;
    }
    return 1;
}

void iptc_free(IPTC_HANDLE handle)
{
    assert(handle == &mock_handle);
    ++free_calls;
    errno = EBUSY;
}
#else
int iptc_delete_num_entry(const char *chain, unsigned int index,
                          IPTC_HANDLE *handle)
{
    assert(chain != NULL);
    assert(index == 7);
    assert(handle != NULL && *handle == &mock_handle);
    ++delete_calls;
    if (!mock_delete_ok) {
        errno = mock_delete_errno;
        return 0;
    }
    return 1;
}

int iptc_insert_entry(const char *chain, const struct ipt_entry *entry,
                      unsigned int index, IPTC_HANDLE *handle)
{
    assert(chain != NULL);
    assert(entry != NULL);
    assert(index == 9);
    assert(handle != NULL && *handle == &mock_handle);
    ++insert_calls;
    if (!mock_insert_ok) {
        errno = mock_insert_errno;
        return 0;
    }
    return 1;
}

int iptc_commit(IPTC_HANDLE *handle)
{
    assert(handle != NULL && *handle == &mock_handle);
    ++commit_calls;
    if (!mock_commit_ok) {
        errno = mock_commit_errno;
        return 0;
    }
    return 1;
}

void iptc_free(IPTC_HANDLE *handle)
{
    assert(handle != NULL && *handle == &mock_handle);
    ++free_calls;
    *handle = NULL;
    errno = EBUSY;
}
#endif

IPTC_HANDLE iptc_init(const char *table)
{
    assert(table != NULL);
    assert(strcmp(table, "filter") == 0);
    ++init_calls;
    if (!mock_init_ok) {
        errno = mock_init_errno;
        return NULL;
    }
    return &mock_handle;
}

const char *iptc_strerror(int error)
{
    (void)error;
    errno = ECHILD;
    return "mock IPTC error";
}

void syslog(int priority, const char *format, ...)
{
    (void)priority;
    (void)format;
    ++log_calls;
    errno = ECHILD;
}
"""

TESTS = r"""
int main(void)
{
    int result;
    struct ipt_entry entry;

    /* Delete operation failure: strerror, syslog and free clobber errno. */
    reset_mocks();
    mock_delete_ok = 0;
    mock_delete_errno = ENOSPC;
    errno = 0;
    result = delete_rule_and_commit(7, &mock_handle, "test", "delete");
    assert(result == -1);
    assert(errno == ENOSPC);
    assert(delete_calls == 1 && commit_calls == 0 && free_calls == 1);
    assert(log_calls == 1);

    /* Commit failure survives the same logging and handle-release path. */
    reset_mocks();
    mock_commit_ok = 0;
    mock_commit_errno = EROFS;
    errno = 0;
    result = delete_rule_and_commit(7, &mock_handle, "test", "delete");
    assert(result == -1);
    assert(errno == EROFS);
    assert(delete_calls == 1 && commit_calls == 1 && free_calls == 1);

    /* A failed backend operation with no errno deterministically becomes EIO. */
    reset_mocks();
    mock_delete_ok = 0;
    mock_delete_errno = 0;
    errno = 0;
    result = delete_rule_and_commit(7, &mock_handle, "test", "delete");
    assert(result == -1);
    assert(errno == EIO);

    /* Initialization failure is captured before strerror and syslog clobber. */
    reset_mocks();
    mock_init_ok = 0;
    mock_init_errno = EACCES;
    errno = 0;
    result = insert_rule_and_commit(9, &entry, "filter", "test", "insert");
    assert(result == -1);
    assert(errno == EACCES);
    assert(init_calls == 1 && insert_calls == 0 && free_calls == 0);
    assert(log_calls == 1);

    /* Insert failure survives logging and freeing the successful handle. */
    reset_mocks();
    mock_insert_ok = 0;
    mock_insert_errno = EINVAL;
    errno = 0;
    result = insert_rule_and_commit(9, &entry, "filter", "test", "insert");
    assert(result == -1);
    assert(errno == EINVAL);
    assert(init_calls == 1 && insert_calls == 1 && commit_calls == 0);
    assert(free_calls == 1 && log_calls == 1);

    /* Restore/rollback commit failure must preserve the commit errno too. */
    reset_mocks();
    mock_commit_ok = 0;
    mock_commit_errno = EROFS;
    errno = 0;
    result = insert_rule_and_commit(9, &entry, "filter", "test", "insert");
    assert(result == -1);
    assert(errno == EROFS);
    assert(init_calls == 1 && insert_calls == 1 && commit_calls == 1);
    assert(free_calls == 1);

    puts("MiniUPnP production IPTC errno helpers (legacy + IPTABLES_143): PASS");
    return 0;
}
"""


def main() -> None:
    translation_unit = PREAMBLE + "\n" + DELETE_HELPER + "\n" + INSERT_HELPER + "\n" + TESTS
    with tempfile.TemporaryDirectory(prefix="miniupnp-iptc-errno-") as temp:
        temp_path = Path(temp)
        c_path = temp_path / "iptc_errno_test.c"
        c_path.write_text(translation_unit)
        for variant, flags in (
            ("legacy", []),
            ("iptables-1.4.3", ["-DIPTABLES_143"]),
        ):
            binary = temp_path / ("iptc_errno_" + variant.replace("-", "_"))
            command = [
                os.environ.get("CC", "cc"),
                "-std=gnu89",
                "-Wall",
                "-Wextra",
                "-Werror",
                "-fsanitize=address,undefined",
                "-fno-omit-frame-pointer",
                "-I",
                str(MINIUPNPD),
                *flags,
                str(c_path),
                "-o",
                str(binary),
            ]
            subprocess.run(command, check=True, cwd=ROOT)
            environment = os.environ.copy()
            environment["ASAN_OPTIONS"] = "detect_leaks=1"
            subprocess.run([str(binary)], check=True, cwd=ROOT, env=environment)


if __name__ == "__main__":
    main()
