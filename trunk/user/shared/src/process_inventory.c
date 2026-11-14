/*
 * Fail-closed process inventory for Padavan shared user space.
 * BusyBox libbb/procps.c remains a separate implementation.
 */
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <limits.h>

#include "process_inventory.h"

#define PROCESS_STAT_BUFFER 4096
#define PROCESS_CMDLINE_BUFFER 4096
#define PROCESS_NAME_BUFFER 128

struct process_scan_context {
	size_t reserved;
};

static void *
proc_default_dir_open(void *context, const char *path)
{
	(void)context;
	return (void *)opendir(path);
}

static int
proc_default_dir_read(void *context, void *directory, char *name,
		      size_t capacity)
{
	DIR *dir = (DIR *)directory;
	struct dirent *entry;

	(void)context;
	errno = 0;
	entry = readdir(dir);
	if (!entry)
		return errno ? -1 : 0;
	if (strlen(entry->d_name) >= capacity) {
		errno = ENAMETOOLONG;
		return -1;
	}
	strcpy(name, entry->d_name);
	return 1;
}

static void
proc_default_dir_close(void *context, void *directory)
{
	(void)context;
	closedir((DIR *)directory);
}

static int
proc_default_read_file(void *context, const char *path, char *buffer,
		       size_t capacity, size_t *length)
{
	int fd, result = 0;
	size_t used = 0;
	char extra;

	(void)context;
	if (!buffer || capacity < 2 || !length) {
		errno = EINVAL;
		return -1;
	}
	fd = open(path, O_RDONLY);
	if (fd < 0)
		return -1;
	while (used < capacity - 1) {
		ssize_t count = read(fd, buffer + used, capacity - 1 - used);
		if (count < 0) {
			if (errno == EINTR)
				continue;
			result = -1;
			break;
		}
		if (count == 0)
			break;
		used += (size_t)count;
	}
	if (result == 0 && used == capacity - 1) {
		ssize_t count;
		do {
			count = read(fd, &extra, 1);
		} while (count < 0 && errno == EINTR);
		if (count != 0) {
			if (count > 0)
				errno = EOVERFLOW;
			result = -1;
		}
	}
	if (close(fd) != 0 && result == 0)
		result = -1;
	if (result != 0)
		return -1;
	buffer[used] = '\0';
	*length = used;
	return 0;
}

static void *
proc_default_allocate(void *context, size_t size)
{
	(void)context;
	return malloc(size);
}

static void *
proc_default_reallocate(void *context, void *pointer, size_t size)
{
	(void)context;
	return realloc(pointer, size);
}

static void
proc_default_release(void *context, void *pointer)
{
	(void)context;
	free(pointer);
}

static const struct process_inventory_ops proc_default_ops = {
	proc_default_dir_open, proc_default_dir_read, proc_default_dir_close,
	proc_default_read_file, proc_default_allocate,
	proc_default_reallocate, proc_default_release
};

static int
proc_ops_valid(const struct process_inventory_ops *ops)
{
	return ops && ops->dir_open && ops->dir_read && ops->dir_close &&
		ops->read_file && ops->allocate && ops->reallocate &&
		ops->release;
}

static unsigned long long
proc_hash(const char *value, size_t length)
{
	unsigned long long hash = 1469598103934665603ULL;
	size_t i;

	for (i = 0; i < length; i++) {
		hash ^= (unsigned char)value[i];
		hash *= 1099511628211ULL;
	}
	return hash;
}

static const char *
proc_basename(const char *value)
{
	const char *slash = strrchr(value, '/');
	return slash ? slash + 1 : value;
}

static int
proc_numeric_pid(const char *name, pid_t *pid)
{
	const char *p;
	char *end;
	unsigned long value;

	if (!name || !*name)
		return 0;
	for (p = name; *p; p++) {
		if (!isdigit((unsigned char)*p))
			return 0;
	}
	errno = 0;
	value = strtoul(name, &end, 10);
	if (errno == ERANGE || end == name || *end || value == 0 ||
	    value > (unsigned long)INT_MAX) {
		errno = EOVERFLOW;
		return -1;
	}
	*pid = (pid_t)value;
	return 1;
}

/*
 * In /proc/PID/stat, after comm, state is field 3 and start_time is field 22.
 * The latter is the nineteenth token beginning with ppid (field 4).
 */
static int
proc_parse_stat(const char *buffer, size_t length, pid_t expected_pid,
		struct process_identity *identity, char *state)
{
	char local[PROCESS_STAT_BUFFER];
	char *left, *right, *pid_end, *cursor;
	long parsed_pid;
	size_t comm_length, copy_length;
	int field;

	if (!buffer || length == 0 || length >= sizeof(local)) {
		errno = EPROTO;
		return -1;
	}
	memcpy(local, buffer, length);
	local[length] = '\0';
	left = strchr(local, '(');
	right = strrchr(local, ')');
	if (!left || !right || right <= left || right[1] != ' ') {
		errno = EPROTO;
		return -1;
	}
	errno = 0;
	parsed_pid = strtol(local, &pid_end, 10);
	if (errno == ERANGE || pid_end == local ||
	    parsed_pid != (long)expected_pid) {
		errno = EPROTO;
		return -1;
	}
	while (*pid_end == ' ')
		pid_end++;
	if (pid_end != left) {
		errno = EPROTO;
		return -1;
	}

	memset(identity, 0, sizeof(*identity));
	comm_length = (size_t)(right - left - 1);
	copy_length = comm_length;
	if (copy_length > sizeof(identity->comm) - 1)
		copy_length = sizeof(identity->comm) - 1;
	memcpy(identity->comm, left + 1, copy_length);
	identity->comm[copy_length] = '\0';
	identity->comm_truncated =
		(comm_length >= sizeof(identity->comm) - 1) ? 1 : 0;
	identity->pid = expected_pid;

	cursor = right + 2;
	for (field = 1; field <= 19; field++) {
		char *token_end;
		size_t token_length;

		while (*cursor == ' ' || *cursor == '\t')
			cursor++;
		if (!*cursor) {
			errno = EPROTO;
			return -1;
		}
		token_end = cursor;
		while (*token_end && *token_end != ' ' && *token_end != '\t')
			token_end++;
		token_length = (size_t)(token_end - cursor);
		if (field == 1) {
			if (token_length != 1) {
				errno = EPROTO;
				return -1;
			}
			*state = cursor[0];
		}
		if (field == 19) {
			char number[32];
			char *number_end;
			if (token_length == 0 || token_length >= sizeof(number)) {
				errno = EPROTO;
				return -1;
			}
			memcpy(number, cursor, token_length);
			number[token_length] = '\0';
			errno = 0;
			identity->start_time = strtoull(number, &number_end, 10);
			if (errno == ERANGE || number_end == number || *number_end) {
				errno = EPROTO;
				return -1;
			}
		}
		cursor = token_end;
	}
	return 0;
}

static enum process_presence
proc_read_identity(const char *process_name, pid_t pid,
		   struct process_identity *identity,
		   const struct process_inventory_ops *ops, void *context,
		   int *error)
{
	char path[64];
	char stat_buffer[PROCESS_STAT_BUFFER];
	char cmdline[PROCESS_CMDLINE_BUFFER];
	char state = '?';
	size_t stat_length, cmdline_length;
	char *argv0, *first_end, *second, *second_end;
	size_t argv0_length, argv1_length;
	int path_length;

	path_length = snprintf(path, sizeof(path), "/proc/%ld/stat", (long)pid);
	if (path_length < 0 || path_length >= (int)sizeof(path)) {
		*error = ENAMETOOLONG;
		return PROCESS_PRESENCE_UNKNOWN;
	}
	if (ops->read_file(context, path, stat_buffer, sizeof(stat_buffer),
			   &stat_length) != 0) {
		*error = errno ? errno : EIO;
		return (*error == ENOENT || *error == ESRCH) ?
			PROCESS_PRESENCE_ABSENT : PROCESS_PRESENCE_UNKNOWN;
	}
	if (proc_parse_stat(stat_buffer, stat_length, pid, identity, &state) != 0) {
		*error = errno ? errno : EPROTO;
		return PROCESS_PRESENCE_UNKNOWN;
	}
	if (state == 'Z' || state == 'X')
		return PROCESS_PRESENCE_ABSENT;

	path_length = snprintf(path, sizeof(path), "/proc/%ld/cmdline", (long)pid);
	if (path_length < 0 || path_length >= (int)sizeof(path)) {
		*error = ENAMETOOLONG;
		return PROCESS_PRESENCE_UNKNOWN;
	}
	if (ops->read_file(context, path, cmdline, sizeof(cmdline),
			   &cmdline_length) != 0) {
		int cmdline_error = errno ? errno : EIO;
		if (cmdline_error == ENOENT || cmdline_error == ESRCH) {
			path_length = snprintf(path, sizeof(path), "/proc/%ld/stat",
					       (long)pid);
			if (path_length >= 0 && path_length < (int)sizeof(path) &&
			    ops->read_file(context, path, stat_buffer,
					   sizeof(stat_buffer), &stat_length) != 0 &&
			    (errno == ENOENT || errno == ESRCH))
				return PROCESS_PRESENCE_ABSENT;
		}
		*error = cmdline_error;
		return PROCESS_PRESENCE_UNKNOWN;
	}

	identity->argv0_hash = 0;
	identity->argv1_hash = 0;
	if (cmdline_length > 0) {
		first_end = (char *)memchr(cmdline, '\0', cmdline_length);
		if (!first_end) {
			*error = EPROTO;
			return PROCESS_PRESENCE_UNKNOWN;
		}
		argv0_length = (size_t)(first_end - cmdline);
		argv0 = (char *)ops->allocate(context, argv0_length + 1);
		if (!argv0) {
			*error = ENOMEM;
			return PROCESS_PRESENCE_UNKNOWN;
		}
		memcpy(argv0, cmdline, argv0_length);
		argv0[argv0_length] = '\0';
		identity->argv0_hash = proc_hash(argv0, argv0_length);

		second = first_end + 1;
		argv1_length = 0;
		if (second < cmdline + cmdline_length) {
			second_end = (char *)memchr(second, '\0',
				(size_t)(cmdline + cmdline_length - second));
			if (!second_end)
				second_end = cmdline + cmdline_length;
			argv1_length = (size_t)(second_end - second);
			identity->argv1_hash = proc_hash(second, argv1_length);
		}
		if (!strcmp(proc_basename(argv0), process_name) ||
		    (identity->comm_truncated && argv1_length > 0 &&
		     !strcmp(proc_basename(second), process_name)) ||
		    (!identity->comm_truncated &&
		     !strcmp(identity->comm, process_name))) {
			ops->release(context, argv0);
			return PROCESS_PRESENCE_RUNNING;
		}
		ops->release(context, argv0);
	} else if (!identity->comm_truncated &&
		   !strcmp(identity->comm, process_name)) {
		return PROCESS_PRESENCE_RUNNING;
	}
	return PROCESS_PRESENCE_ABSENT;
}

enum process_presence
process_inventory_scan_with_ops(const char *process_name,
				struct process_snapshot *snapshot,
				const struct process_inventory_ops *ops,
				void *context)
{
	struct process_scan_context *scan_context;
	struct process_identity *items = NULL;
	void *directory = NULL;
	char entry[PROCESS_NAME_BUFFER];
	pid_t pid;
	int entry_result, numeric_result, error = 0;
	size_t count = 0, capacity = 0;
	enum process_presence state;

	if (!snapshot)
		return PROCESS_PRESENCE_UNKNOWN;
	memset(snapshot, 0, sizeof(*snapshot));
	snapshot->state = PROCESS_PRESENCE_UNKNOWN;
	if (!process_name || !*process_name || !proc_ops_valid(ops)) {
		snapshot->error = EINVAL;
		return snapshot->state;
	}
	scan_context = (struct process_scan_context *)
		ops->allocate(context, sizeof(*scan_context));
	if (!scan_context) {
		snapshot->error = ENOMEM;
		return snapshot->state;
	}
	scan_context->reserved = 0;
	directory = ops->dir_open(context, "/proc");
	if (!directory) {
		error = errno ? errno : EIO;
		goto failed;
	}

	for (;;) {
		struct process_identity identity;

		errno = 0;
		entry_result = ops->dir_read(context, directory, entry, sizeof(entry));
		if (entry_result == 0)
			break;
		if (entry_result < 0) {
			error = errno ? errno : EIO;
			goto failed;
		}
		numeric_result = proc_numeric_pid(entry, &pid);
		if (numeric_result == 0)
			continue;
		if (numeric_result < 0) {
			error = errno ? errno : EOVERFLOW;
			goto failed;
		}
		state = proc_read_identity(process_name, pid, &identity, ops, context,
					   &error);
		if (state == PROCESS_PRESENCE_UNKNOWN)
			goto failed;
		if (state == PROCESS_PRESENCE_ABSENT)
			continue;

		if (count == capacity) {
			size_t next = capacity ? capacity * 2 : 1;
			struct process_identity *grown;
			if (next < capacity ||
			    next > (size_t)-1 / sizeof(*items)) {
				error = EOVERFLOW;
				goto failed;
			}
			if (!items)
				grown = (struct process_identity *)ops->allocate(
					context, next * sizeof(*items));
			else
				grown = (struct process_identity *)ops->reallocate(
					context, items, next * sizeof(*items));
			if (!grown) {
				error = ENOMEM;
				goto failed;
			}
			items = grown;
			capacity = next;
		}
		items[count++] = identity;
	}
	ops->dir_close(context, directory);
	ops->release(context, scan_context);
	snapshot->items = items;
	snapshot->count = count;
	snapshot->state = count ? PROCESS_PRESENCE_RUNNING :
		PROCESS_PRESENCE_ABSENT;
	snapshot->error = 0;
	snapshot->release_items = ops->release;
	snapshot->release_context = context;
	return snapshot->state;

failed:
	if (directory)
		ops->dir_close(context, directory);
	if (items)
		ops->release(context, items);
	ops->release(context, scan_context);
	snapshot->items = NULL;
	snapshot->count = 0;
	snapshot->state = PROCESS_PRESENCE_UNKNOWN;
	snapshot->error = error ? error : EIO;
	return snapshot->state;
}

enum process_presence
process_identity_validate_with_ops(const char *process_name,
				   const struct process_identity *identity,
				   const struct process_inventory_ops *ops,
				   void *context)
{
	struct process_identity current;
	int error = 0;
	enum process_presence state;

	if (!identity || !process_name || !*process_name || !proc_ops_valid(ops))
		return PROCESS_PRESENCE_UNKNOWN;
	state = proc_read_identity(process_name, identity->pid, &current, ops,
				   context, &error);
	if (state != PROCESS_PRESENCE_RUNNING)
		return state;
	if (current.start_time != identity->start_time ||
	    current.argv0_hash != identity->argv0_hash ||
	    current.argv1_hash != identity->argv1_hash ||
	    current.comm_truncated != identity->comm_truncated ||
	    strcmp(current.comm, identity->comm) != 0)
		return PROCESS_PRESENCE_ABSENT;
	return PROCESS_PRESENCE_RUNNING;
}

enum process_presence
process_inventory_scan(const char *process_name, struct process_snapshot *snapshot)
{
	return process_inventory_scan_with_ops(process_name, snapshot,
					       &proc_default_ops, NULL);
}

enum process_presence
process_identity_validate(const char *process_name,
			  const struct process_identity *identity)
{
	return process_identity_validate_with_ops(process_name, identity,
						  &proc_default_ops, NULL);
}


