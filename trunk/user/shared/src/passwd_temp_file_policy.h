#ifndef PADAVAN_PASSWD_TEMP_FILE_POLICY_H
#define PADAVAN_PASSWD_TEMP_FILE_POLICY_H

#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static int
passwd_temp_file_open(char *path_template, mode_t mode)
{
	int fd, saved_errno;
	if (!path_template || !path_template[0]) {
		errno = EINVAL;
		return -1;
	}
	fd = mkstemp(path_template);
	if (fd < 0)
		return -1;
	if (fchmod(fd, mode) != 0) {
		saved_errno = errno ? errno : EIO;
		(void)close(fd);
		(void)unlink(path_template);
		errno = saved_errno;
		return -1;
	}
	return fd;
}

#endif
