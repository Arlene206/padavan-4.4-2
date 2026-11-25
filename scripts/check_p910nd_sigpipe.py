#!/usr/bin/env python3
"""Pin the SIGPIPE policy to p910nd's bidirectional socket-write path."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "trunk/user/p910nd/p910nd.c").read_text()
POLICY = (ROOT / "trunk/user/p910nd/p910nd_sigpipe_policy.h").read_text()
IO_POLICY = (ROOT / "trunk/user/p910nd/p910nd_io_policy.h").read_text()
MAKEFILE = (ROOT / "trunk/user/p910nd/Makefile").read_text()


def compact(text):
    return re.sub(r"\s+", " ", text).strip()


assert '#include "p910nd_sigpipe_policy.h"' in SOURCE

main_start = SOURCE.index("int main(")
main = SOURCE[main_start:]
main_compact = compact(main)
assert re.search(
    r"if \(bidir && p910nd_install_sigpipe_policy\(\) < 0\)",
    main_compact,
), "bidirectional startup must fail if SIGPIPE policy installation fails"
install_at = main_compact.index("p910nd_install_sigpipe_policy()")
assert install_at < main_compact.index("server(lpnumber)")
assert install_at < main_compact.index("one_job(lpnumber)")

policy = compact(POLICY)
assert "action.sa_handler = SIG_IGN;" in policy
assert "sigemptyset(&action.sa_mask)" in policy
assert "sigaction(SIGPIPE, &action, NULL)" in policy

copy_start = SOURCE.index("int copy_stream(")
copy_end = SOURCE.index("void one_job(", copy_start)
copy = compact(SOURCE[copy_start:copy_end])
assert "initBuffer(&printerToNetworkBuffer, lp, fd, 0)" in copy
assert "writeBuffer(&printerToNetworkBuffer)" in copy
assert "printerToNetworkBuffer.outfd = -1;" in copy
assert "printerToNetworkBuffer.err = 0;" in copy
assert "p910nd_fd_is_selectable(fd, FD_SETSIZE)" in copy
assert "p910nd_fd_is_selectable(lp, FD_SETSIZE)" in copy
assert copy.index("int result;") < copy.index("Buffer_t networkToPrinterBuffer;") < copy.index("p910nd_fd_is_selectable(fd, FD_SETSIZE)")
assert copy.index("p910nd_fd_is_selectable") < copy.index("prepBuffer(&networkToPrinterBuffer")

assert '#include "p910nd_io_policy.h"' in SOURCE
io_policy = compact(IO_POLICY)
assert "while (result < 0 && errno == EINTR)" in io_policy
assert "return error == EAGAIN || error == EWOULDBLOCK;" in io_policy
assert "return requested > 0 && result == 0;" in io_policy
assert "return fd >= 0 && fd_setsize > 0 && fd < fd_setsize;" in io_policy
assert "$(PROG): p910nd.c p910nd_io_policy.h p910nd_sigpipe_policy.h" in MAKEFILE
assert "$(CC) -o $@ p910nd.c $(CFLAGS) $(DEFINES) $(LIBS)" in MAKEFILE

read_start = SOURCE.index("ssize_t readBuffer(")
write_start = SOURCE.index("ssize_t writeBuffer(", read_start)
read_body = compact(SOURCE[read_start:write_start])
assert "p910nd_read_retry_eintr(b->infd, b->buffer + b->endidx, avail, read)" in read_body
assert "if (p910nd_io_would_block(errno))" in read_body

copy_start = SOURCE.index("int copy_stream(")
copy_end = SOURCE.index("void one_job(", copy_start)
copy = compact(SOURCE[copy_start:copy_end])
assert "if (errno == EINTR) continue;" in copy

write_body = compact(SOURCE[write_start:copy_start])
assert "p910nd_write_retry_eintr(b->outfd, b->buffer + b->startidx, avail, write)" in write_body
assert "if (result < 0 && p910nd_io_would_block(errno))" in write_body
assert "p910nd_write_no_progress(result, avail)" in write_body
assert "errno = EIO;" in write_body

print("p910nd bidirectional SIGPIPE and stream-I/O source contract: PASS")
