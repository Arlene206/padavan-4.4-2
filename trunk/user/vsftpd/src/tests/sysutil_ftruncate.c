#define _GNU_SOURCE 1

#include "sysutil.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int
main(void)
{
  char path[] = "/tmp/vsftpd-ftruncate-XXXXXX";
  int fd = -1;
  int read_only_fd = -1;
  int status = 1;
  struct stat st;

  fd = mkstemp(path);
  if (fd < 0)
  {
    perror("mkstemp");
    goto out;
  }
  if (write(fd, "payload", 7) != 7)
  {
    perror("write");
    goto out;
  }
  if (vsf_sysutil_ftruncate(fd) != 0)
  {
    perror("vsf_sysutil_ftruncate(valid fd)");
    goto out;
  }
  if (fstat(fd, &st) != 0 || st.st_size != 0)
  {
    fprintf(stderr, "truncate did not reduce the file to zero bytes\n");
    goto out;
  }

  read_only_fd = open(path, O_RDONLY);
  if (read_only_fd < 0)
  {
    perror("open(O_RDONLY)");
    goto out;
  }
  {
    int expected_status;
    int expected_errno;

    errno = 0;
    expected_status = ftruncate(read_only_fd, 0);
    expected_errno = errno;
    if (expected_status == 0 || expected_errno == 0)
    {
      fprintf(stderr, "read-only ftruncate fixture unexpectedly succeeded\n");
      goto out;
    }

    errno = 0;
    if (vsf_sysutil_ftruncate(read_only_fd) == 0)
    {
      fprintf(stderr, "truncate unexpectedly succeeded on a read-only fd\n");
      goto out;
    }
    if (errno != expected_errno)
    {
      fprintf(stderr, "truncate error was not preserved (expected=%d actual=%d)\n",
              expected_errno, errno);
      goto out;
    }
  }

  errno = 0;
  if (vsf_sysutil_ftruncate(-1) == 0 || errno != EBADF)
  {
    fprintf(stderr, "invalid descriptor did not return EBADF (errno=%d)\n",
            errno);
    goto out;
  }

  /* A failed filesystem operation must return to the FTP caller, not die(). */
  if (write(fd, "x", 1) != 1)
  {
    perror("process did not continue after truncate failure");
    goto out;
  }
  puts("VSFTPD ftruncate returns filesystem errors without terminating the process: PASS");
  status = 0;

out:
  if (read_only_fd >= 0)
    close(read_only_fd);
  if (fd >= 0)
    close(fd);
  unlink(path);
  return status;
}
