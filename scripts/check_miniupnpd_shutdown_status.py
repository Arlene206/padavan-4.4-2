#!/usr/bin/env python3
"""Require late MiniUPnPd event-loop failures to pass through the shared teardown path."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "trunk/user/miniupnpd/miniupnpd-2.x/miniupnpd.c").read_text()


def function_body(source, signature):
    position = source.index(signature)
    brace = source.index("{", position)
    depth = 0
    for index in range(brace, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[position:index + 1]
    raise AssertionError("unterminated function: " + signature)


main = function_body(SOURCE, "main(int argc, char * * argv)")
select_at = main.index("if(select(max_fd+1")
brace = main.index("{", select_at)
depth = 0
select_end = None
for index in range(brace, len(main)):
    if main[index] == "{":
        depth += 1
    elif main[index] == "}":
        depth -= 1
        if depth == 0:
            select_end = index + 1
            break
assert select_end is not None, "select error block is unterminated"
select_block = main[select_at:select_end]
assert "if(errno == EINTR) continue;" in select_block
assert "exit_status = 1;" in select_block
assert "goto shutdown;" in select_block
assert "return 1;" not in select_block, "fatal select failure still bypasses common teardown"
fatal_branch = select_block[
    select_block.index('syslog(LOG_ERR, "Failed to select open sockets. EXITING")') :
]
assert fatal_branch.index("exit_status = 1;") < fatal_branch.index("goto shutdown;")

assert main.count("int exit_status = 0;") == 1
shutdown_at = main.index("shutdown:")
shutdown = main[shutdown_at:]
assert 'unlink("/var/run/miniupnpd.ctl")' in shutdown
assert "unlink(pidfilename)" in shutdown
assert shutdown.rstrip().endswith("return exit_status;\n}")
assert "return 0;\n}" not in shutdown

print("MiniUPnPd event-loop error teardown and exit-status contract: PASS")
