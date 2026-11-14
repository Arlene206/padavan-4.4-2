#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "process_inventory.h"
#include "process_pidfile.h"

#define PROCESS_PIDFILE_RECORD_SIZE 64
#define PROCESS_PIDFILE_PATH_SIZE 4096

int
process_pidfile_parse_record(const char *text, size_t length,
			     pid_t *pid, unsigned long long *start_time)
{
	unsigned long pid_value = 0;
	unsigned long long start_value = 0;
	size_t i, separator = (size_t)-1;

	if (!text || !pid || !start_time || length == 0 ||
	    length >= PROCESS_PIDFILE_RECORD_SIZE) {
		errno = EINVAL;
		return -1;
	}
	if (text[length - 1] == '\n')
		length--;
	if (length == 0) {
		errno = EINVAL;
		return -1;
	}
	for (i = 0; i < length; i++) {
		if (text[i] == ' ') {
			if (separator != (size_t)-1 || i == 0) {
				errno = EINVAL;
				return -1;
			}
			separator = i;
		} else if (text[i] < '0' || text[i] > '9') {
			errno = EINVAL;
			return -1;
		}
	}
	if (separator == (size_t)-1 || separator + 1 >= length) {
		errno = EINVAL;
		return -1;
	}
	for (i = 0; i < separator; i++) {
		unsigned int digit = (unsigned int)(text[i] - '0');
		if (pid_value > ((unsigned long)INT_MAX - digit) / 10) {
			errno = ERANGE;
			return -1;
		}
		pid_value = pid_value * 10 + digit;
	}
	for (i = separator + 1; i < length; i++) {
		unsigned int digit = (unsigned int)(text[i] - '0');
		/* Avoid 64-bit division helpers in the MIPS shared object. */
		if (start_value > 1844674407370955161ULL ||
		    (start_value == 1844674407370955161ULL && digit > 5)) {
			errno = ERANGE;
			return -1;
		}
		start_value = start_value * 10 + digit;
	}
	if (pid_value <= 1 || start_value == 0) {
		errno = EINVAL;
		return -1;
	}
	*pid = (pid_t)pid_value;
	*start_time = start_value;
	return 0;
}

static int
process_pidfile_read(const char *path, pid_t *pid,
		     unsigned long long *start_time)
{
	char buffer[PROCESS_PIDFILE_RECORD_SIZE];
	int fd, saved_errno;
	size_t used = 0;
	ssize_t count;

	fd = open(path, O_RDONLY);
	if (fd < 0)
		return -1;
	while (used < sizeof(buffer)) {
		count = read(fd, buffer + used, sizeof(buffer) - used);
		if (count < 0) {
			if (errno == EINTR)
				continue;
			saved_errno = errno;
			close(fd);
			errno = saved_errno;
			return -1;
		}
		if (count == 0)
			break;
		used += (size_t)count;
	}
	if (close(fd) < 0)
		return -1;
	return process_pidfile_parse_record(buffer, used, pid, start_time);
}

static int
process_pidfile_write_all(int fd, const char *buffer, size_t length)
{
	size_t offset = 0;

	while (offset < length) {
		ssize_t count = write(fd, buffer + offset, length - offset);
		if (count < 0) {
			if (errno == EINTR)
				continue;
			return -1;
		}
		if (count == 0) {
			errno = EIO;
			return -1;
		}
		offset += (size_t)count;
	}
	return 0;
}

int
process_pidfile_write(const char *path, const char *process_name)
{
	struct process_snapshot snapshot;
	enum process_presence state;
	unsigned long long start_time = 0;
	char record[PROCESS_PIDFILE_RECORD_SIZE];
	char temp_path[PROCESS_PIDFILE_PATH_SIZE];
	int fd, length, saved_errno;
	size_t i;

	if (!path || !*path || !process_name || !*process_name) {
		errno = EINVAL;
		return -1;
	}
	state = process_inventory_scan(process_name, &snapshot);
	if (state == PROCESS_PRESENCE_UNKNOWN) {
		errno = snapshot.error ? snapshot.error : EIO;
		process_snapshot_release(&snapshot);
		return -1;
	}
	for (i = 0; i < snapshot.count; i++) {
		if (snapshot.items[i].pid == getpid()) {
			start_time = snapshot.items[i].start_time;
			break;
		}
	}
	process_snapshot_release(&snapshot);
	if (start_time == 0) {
		errno = ESRCH;
		return -1;
	}
	length = snprintf(record, sizeof(record), "%ld %llu\n",
			  (long)getpid(), start_time);
	if (length <= 0 || length >= (int)sizeof(record)) {
		errno = EOVERFLOW;
		return -1;
	}
	length = snprintf(temp_path, sizeof(temp_path), "%s.tmp.XXXXXX", path);
	if (length <= 0 || length >= (int)sizeof(temp_path)) {
		errno = ENAMETOOLONG;
		return -1;
	}
	fd = mkstemp(temp_path);
	if (fd < 0)
		return -1;
	if (fchmod(fd, 0644) < 0 ||
	    process_pidfile_write_all(fd, record, (size_t)length) < 0) {
		saved_errno = errno;
		close(fd);
		unlink(temp_path);
		errno = saved_errno;
		return -1;
	}
	if (close(fd) < 0) {
		saved_errno = errno;
		unlink(temp_path);
		errno = saved_errno;
		return -1;
	}
	if (rename(temp_path, path) < 0) {
		saved_errno = errno;
		unlink(temp_path);
		errno = saved_errno;
		return -1;
	}
	return 0;
}

int
process_pidfile_signal(const char *path, const char *process_name, int sig)
{
	struct process_snapshot snapshot;
	enum process_presence state;
	pid_t pid;
	unsigned long long start_time;
	size_t i;
	int result, saved_errno;

	if (!path || !*path || !process_name || !*process_name) {
		errno = EINVAL;
		return -1;
	}
	if (process_pidfile_read(path, &pid, &start_time) < 0)
		return -1;
	state = process_inventory_scan(process_name, &snapshot);
	if (state == PROCESS_PRESENCE_UNKNOWN) {
		errno = snapshot.error ? snapshot.error : EIO;
		process_snapshot_release(&snapshot);
		return -1;
	}
	for (i = 0; i < snapshot.count; i++) {
		if (snapshot.items[i].pid != pid)
			continue;
		if (snapshot.items[i].start_time != start_time) {
			process_snapshot_release(&snapshot);
			errno = ESRCH;
			return -1;
		}
		state = process_identity_validate(process_name, &snapshot.items[i]);
		if (state != PROCESS_PRESENCE_RUNNING) {
			saved_errno = state == PROCESS_PRESENCE_UNKNOWN ? EIO : ESRCH;
			process_snapshot_release(&snapshot);
			errno = saved_errno;
			return -1;
		}
		result = kill(pid, sig);
		saved_errno = errno;
		process_snapshot_release(&snapshot);
		errno = saved_errno;
		return result;
	}
	process_snapshot_release(&snapshot);
	errno = ESRCH;
	return -1;
}
