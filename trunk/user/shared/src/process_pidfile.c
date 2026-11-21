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
#include "process_io_retry_policy.h"

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


int
process_pidfile_parse_link_record(const char *text, size_t length,
				  pid_t *pid, unsigned long long *start_time)
{
	const char *line1_end, *line2_end, *line3_end;
	char record[PROCESS_PIDFILE_RECORD_SIZE];
	size_t line1_length, line2_length, line3_length, record_length;
	const char *p;

	if (!text || !pid || !start_time || length == 0 ||
	    text[length - 1] != '\n') {
		errno = EINVAL;
		return -1;
	}
	line1_end = (const char *)memchr(text, '\n', length);
	if (!line1_end || line1_end == text) {
		errno = EINVAL;
		return -1;
	}
	line2_end = (const char *)memchr(line1_end + 1, '\n',
					length - (size_t)(line1_end + 1 - text));
	if (!line2_end) {
		errno = EINVAL;
		return -1;
	}
	line3_end = (const char *)memchr(line2_end + 1, '\n',
					length - (size_t)(line2_end + 1 - text));
	if (!line3_end || line3_end != text + length - 1) {
		errno = EINVAL;
		return -1;
	}
	line1_length = (size_t)(line1_end - text);
	line2_length = (size_t)(line2_end - line1_end - 1);
	line3_length = (size_t)(line3_end - line2_end - 1);
	if (line1_length == 0 || line3_length == 0 ||
	    line1_length + line3_length + 2 >= sizeof(record)) {
		errno = EINVAL;
		return -1;
	}
	/* linkname PID files preserve their historic PID / ifname prefix.
	 * Accept an absent ifname, or a canonical Linux PPP interface name. */
	if (line2_length != 0) {
		if (line2_length < 4 || line2_length > 15 ||
		    memcmp(line1_end + 1, "ppp", 3) != 0) {
			errno = EINVAL;
			return -1;
		}
		for (p = line1_end + 4; p < line2_end; ++p) {
			if (*p < '0' || *p > '9') {
				errno = EINVAL;
				return -1;
			}
		}
	}
	memcpy(record, text, line1_length);
	record[line1_length] = ' ';
	memcpy(record + line1_length + 1, line2_end + 1, line3_length);
	record_length = line1_length + 1 + line3_length;
	record[record_length] = '\n';
	return process_pidfile_parse_record(record, record_length + 1,
					     pid, start_time);
}

static int
process_pidfile_read(const char *path, pid_t *pid,
		     unsigned long long *start_time)
{
	char buffer[PROCESS_PIDFILE_RECORD_SIZE];
	struct stat st;
	int fd, saved_errno;
	size_t used = 0;
	ssize_t count;

	fd = open(path, O_RDONLY | O_NOFOLLOW | O_CLOEXEC | O_NONBLOCK);
	if (fd < 0)
		return -1;
	if (fstat(fd, &st) != 0)
		goto failed;
	if (!S_ISREG(st.st_mode) || st.st_uid != 0 || st.st_nlink != 1 ||
	    (st.st_mode & 022) != 0) {
		errno = EPERM;
		goto failed;
	}
	while (used < sizeof(buffer)) {
		count = process_io_read(fd, buffer + used, sizeof(buffer) - used);
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
	if (close(fd) < 0)
		return -1;
	if (process_pidfile_parse_record(buffer, used, pid, start_time) == 0)
		return 0;
	return process_pidfile_parse_link_record(buffer, used, pid, start_time);

failed:
	saved_errno = errno ? errno : EIO;
	(void)close(fd);
	errno = saved_errno;
	return -1;
}

enum process_presence
process_pidfile_read_identity(const char *path, const char *process_name,
			      struct process_identity *identity)
{
	struct process_snapshot snapshot;
	enum process_presence state;
	pid_t pid;
	unsigned long long start_time;
	size_t i;

	if (!path || !*path || !process_name || !*process_name || !identity) {
		errno = EINVAL;
		return PROCESS_PRESENCE_UNKNOWN;
	}
	if (process_pidfile_read(path, &pid, &start_time) != 0) {
		if (errno == ENOENT)
			return PROCESS_PRESENCE_ABSENT;
		return PROCESS_PRESENCE_UNKNOWN;
	}
	state = process_inventory_scan(process_name, &snapshot);
	if (state == PROCESS_PRESENCE_UNKNOWN) {
		errno = snapshot.error ? snapshot.error : EIO;
		process_snapshot_release(&snapshot);
		return PROCESS_PRESENCE_UNKNOWN;
	}
	if (state != PROCESS_PRESENCE_RUNNING && state != PROCESS_PRESENCE_ABSENT) {
		process_snapshot_release(&snapshot);
		errno = EIO;
		return PROCESS_PRESENCE_UNKNOWN;
	}
	for (i = 0; i < snapshot.count; i++) {
		if (snapshot.items[i].pid != pid)
			continue;
		if (snapshot.items[i].start_time != start_time) {
			process_snapshot_release(&snapshot);
			errno = ESRCH;
			return PROCESS_PRESENCE_ABSENT;
		}
		state = process_identity_validate(process_name, &snapshot.items[i]);
		if (state != PROCESS_PRESENCE_RUNNING) {
			int saved_errno = state == PROCESS_PRESENCE_ABSENT ? ESRCH : EIO;
			process_snapshot_release(&snapshot);
			errno = saved_errno;
			return state == PROCESS_PRESENCE_ABSENT ? PROCESS_PRESENCE_ABSENT :
				PROCESS_PRESENCE_UNKNOWN;
		}
		*identity = snapshot.items[i];
		process_snapshot_release(&snapshot);
		return PROCESS_PRESENCE_RUNNING;
	}
	process_snapshot_release(&snapshot);
	errno = ESRCH;
	return PROCESS_PRESENCE_ABSENT;
}

static int
process_pidfile_write_all(int fd, const char *buffer, size_t length)
{
	size_t offset = 0;

	while (offset < length) {
		ssize_t count = process_io_write(fd, buffer + offset, length - offset);
		if (count < 0)
			return -1;
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
