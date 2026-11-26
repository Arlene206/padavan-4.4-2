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


def compact_c(text):
    """Remove C comments and whitespace for formatting-independent checks."""
    without_comments = re.sub(
        r"/[*].*?[*]/|//[^\r\n]*", "", text, flags=re.S
    )
    return re.sub(r"\s+", "", without_comments)
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
assert "p910nd_timespec_deadline_reached" in io_policy
assert "p910nd_timespec_elapsed" in io_policy
assert "p910nd_timespec_add_milliseconds" in io_policy
assert "test_monotonic_timespec_policy" in (ROOT / "scripts/test_p910nd_sigpipe.c").read_text()
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
assert "clock_gettime(CLOCK_MONOTONIC" in copy
assert "gettimeofday(" not in copy
assert "p910nd_timespec_deadline_reached" in copy
assert "p910nd_timespec_elapsed" in copy
assert "p910nd_timespec_add_milliseconds" in copy

write_body = compact(SOURCE[write_start:copy_start])
assert "p910nd_write_retry_eintr(b->outfd, b->buffer + b->startidx, avail, write)" in write_body
assert "if (result < 0 && p910nd_io_would_block(errno))" in write_body
assert "p910nd_write_no_progress(result, avail)" in write_body
assert "errno = EIO;" in write_body

# Opening the default printer path must not retain open_printer()'s
# automatic local array in the global device pointer across retries.
open_printer_start = SOURCE.index("int open_printer(")
open_printer_end = SOURCE.index("int get_lock(", open_printer_start)
open_printer = re.sub(
    r"/[*].*?[*]/", "", SOURCE[open_printer_start:open_printer_end], flags=re.S
)
assert re.search(r"const\s+char\s*\*\s*printer_device\s*;", open_printer)
assert re.search(
    r"printer_device\s*=\s*device\s*\?\s*device\s*:\s*lpname\s*;",
    open_printer,
)
assert re.search(r"open\s*\(\s*printer_device\s*,", open_printer)
assert re.search(
    r'dolog\s*\(\s*LOGOPTS\s*,\s*"%s: %m, will try opening later\\n"'
    r'\s*,\s*printer_device\s*\)',
    open_printer,
)
assert not re.search(r"device\s*=\s*lpname\s*;", open_printer)

# accept(2) overwrites addrlen with the actual peer-address length.
# The server is dual-stack, so each iteration must reset the sockaddr storage
# and its capacity before accepting a new IPv4/IPv6 peer.
server_start = SOURCE.index("void server(")
server_end = SOURCE.index("int is_standalone(", server_start)
server = compact_c(SOURCE[server_start:server_end])
accept_loop = (
    "for(;;){"
    "clientlen=sizeof(client);"
    "memset(&client,0,sizeof(client));"
    "do{fd=accept(netfd,(structsockaddr*)&client,&clientlen);}"
    "while(fd<0&&errno==EINTR);"
    "if(fd<0)break;"
)
assert accept_loop in server, (
    "server must reset accept address storage/length on every iteration "
    "and retry interrupted accept calls"
)
accept_loop_at = server.index(accept_loop)
assert accept_loop_at < server.index("get_ip_str((structsockaddr*)&client", accept_loop_at)
assert accept_loop_at < server.index("hosts_ctl(", accept_loop_at) if "hosts_ctl(" in server else True

# A duplicate standalone start blocks in F_SETLKW while the current server
# owns the printer lock. It must not replace the live PID record until the
# lock is actually acquired, otherwise stopproc can target the waiting child.
server_start = SOURCE.index("void server(")
server_end = SOURCE.index("int is_standalone(", server_start)
server_lifecycle = compact(SOURCE[server_start:server_end])
lock_acquire = "if (get_lock(lpnumber) == 0) exit(1);"
pidfile_open = 'fopen(pidfilename, "w")'
assert lock_acquire in server_lifecycle
assert pidfile_open in server_lifecycle
assert server_lifecycle.index("setsid()") < server_lifecycle.index(lock_acquire)
assert server_lifecycle.index(lock_acquire) < server_lifecycle.index(pidfile_open), (
    "p910nd must acquire its blocking singleton lock before publishing PID"
)

print("p910nd bidirectional SIGPIPE, stream-I/O and PID publication source contract: PASS")
