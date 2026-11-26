#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "ftp_share_mode_policy.h"

static void
expect_mode(const char *text, int expected)
{
    int mode = -1;

    assert(ftp_share_mode_read_status(1, 0, text, &mode) == 0);
    assert(mode == expected);
}

int
main(void)
{
    int mode = -1;

    expect_mode("1", 1);
    expect_mode("2", 2);
    expect_mode("3", 3);
    expect_mode("4", 4);
    expect_mode(" +3 \t", 3);

    /* Preserve the configuration writer's mode-2 normalization. */
    expect_mode("", 2);
    expect_mode(" \t\r\n", 2);
    expect_mode("0", 2);
    expect_mode("5", 2);
    expect_mode("-1", 2);
    expect_mode("999", 2);

    /* A numeric prefix must never silently select anonymous mode 1. */
    assert(ftp_share_mode_read_status(1, 0, "1junk", &mode) == EINVAL);
    assert(mode == 2);
    assert(ftp_share_mode_read_status(1, 0, "junk1", &mode) == EINVAL);
    assert(mode == 2);
    assert(ftp_share_mode_read_status(1, 0, "1 2", &mode) == EINVAL);
    assert(mode == 2);

    assert(ftp_share_mode_read_status(0, 0, NULL, &mode) == 0);
    assert(mode == 2);
    assert(ftp_share_mode_read_status(-1, EACCES, NULL, &mode) == EACCES);
    assert(mode == 2);
    assert(ftp_share_mode_read_status(-1, 0, NULL, &mode) == EIO);
    assert(mode == 2);
    assert(ftp_share_mode_read_status(2, 0, "1", &mode) == EIO);
    assert(mode == 2);
    assert(ftp_share_mode_read_status(1, 0, NULL, &mode) == EIO);
    assert(mode == 2);
    assert(ftp_share_mode_read_status(1, 0,
           "999999999999999999999999999999999999", &mode) == ERANGE);
    assert(mode == 2);
    assert(ftp_share_mode_read_status(1, 0, "1", NULL) == EINVAL);

    puts("vsftpd checked share-mode snapshot policy: PASS");
    return 0;
}
