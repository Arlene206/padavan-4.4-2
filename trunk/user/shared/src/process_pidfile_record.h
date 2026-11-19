#ifndef PROCESS_PIDFILE_RECORD_H
#define PROCESS_PIDFILE_RECORD_H

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include "process_io_retry_policy.h"

static inline int
process_pidfile_record_parse_start_time(const char *text, size_t length,
					unsigned long long *start_time)
{
	const char *close = NULL, *p, *token;
	unsigned long long value = 0;
	size_t i;

	if (!text || !start_time || length == 0) {
		errno = EINVAL;
		return -1;
	}
	for (p = text; p < text + length; ++p)
		if (*p == ')')
			close = p;
	if (!close || (size_t)(text + length - close) < 5 ||
	    close[1] != ' ' || close[2] == ' ' || close[2] == '\n' ||
	    close[3] != ' ') {
		errno = EINVAL;
		return -1;
	}
	p = close + 4;
	for (i = 4; i <= 22; ++i) {
		while (p < text + length && *p == ' ')
			++p;
		token = p;
		while (p < text + length && *p != ' ' && *p != '\n')
			++p;
		if (p == token) {
			errno = EINVAL;
			return -1;
		}
		if (i == 22) {
			const char *digit;
			for (digit = token; digit < p; ++digit) {
				unsigned int n;
				if (*digit < '0' || *digit > '9') {
					errno = EINVAL;
					return -1;
				}
				n = (unsigned int)(*digit - '0');
				if (value > 1844674407370955161ULL ||
				    (value == 1844674407370955161ULL && n > 5)) {
					errno = ERANGE;
					return -1;
				}
				value = value * 10 + n;
			}
		}
	}
	if (value == 0) {
		errno = EINVAL;
		return -1;
	}
	*start_time = value;
	return 0;
}

static inline int
process_pidfile_record_read_start_time(pid_t pid,
				       unsigned long long *start_time)
{
	char path[64], buffer[4096], extra;
	int fd, path_length, saved_errno;
	size_t used = 0;
	ssize_t count;

	if (pid <= (pid_t)1 || !start_time) {
		errno = EINVAL;
		return -1;
	}
	path_length = snprintf(path, sizeof(path), "/proc/%ld/stat", (long)pid);
	if (path_length < 0 || path_length >= (int)sizeof(path)) {
		errno = ENAMETOOLONG;
		return -1;
	}
	fd = open(path, O_RDONLY);
	if (fd < 0)
		return -1;
	while (used < sizeof(buffer) - 1) {
		count = process_io_read(fd, buffer + used, sizeof(buffer) - 1 - used);
		if (count < 0) {
			saved_errno = errno;
			(void)close(fd);
			errno = saved_errno;
			return -1;
		}
		if (count == 0)
			break;
		used += (size_t)count;
	}
	if (used == sizeof(buffer) - 1) {
		count = process_io_read(fd, &extra, 1);
		if (count != 0) {
			saved_errno = count < 0 ? errno : EOVERFLOW;
			(void)close(fd);
			errno = saved_errno;
			return -1;
		}
	}
	if (close(fd) != 0)
		return -1;
	return process_pidfile_record_parse_start_time(buffer, used, start_time);
}

static inline int
process_pidfile_record_format(pid_t pid, unsigned long long start_time,
			      char *buffer, size_t buffer_size)
{
	int length;

	if (pid <= (pid_t)1 || start_time == 0 || !buffer || buffer_size == 0) {
		errno = EINVAL;
		return -1;
	}
	length = snprintf(buffer, buffer_size, "%ld %llu\n",
			  (long)pid, start_time);
	if (length <= 0 || (size_t)length >= buffer_size) {
		errno = EOVERFLOW;
		return -1;
	}
	return length;
}

#endif
