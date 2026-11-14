#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../process_inventory.h"

struct fake_process {
	pid_t pid;
	char stat_text[256];
	char cmdline[128];
	size_t cmdline_length;
	int stat_error;
	int cmdline_error;
	int disappear_on_cmdline;
};

struct fake_context {
	const char **entries;
	size_t entry_count;
	size_t entry_index;
	int open_error;
	int readdir_error_at;
	int allocate_calls;
	int fail_allocate_at;
	int reallocate_calls;
	int fail_reallocate_at;
	struct fake_process processes[8];
	size_t process_count;
};

static struct fake_process *
fake_find(struct fake_context *context, pid_t pid)
{
	size_t i;
	for (i = 0; i < context->process_count; i++) {
		if (context->processes[i].pid == pid)
			return &context->processes[i];
	}
	return NULL;
}

static void *
fake_dir_open(void *opaque, const char *path)
{
	struct fake_context *context = (struct fake_context *)opaque;
	assert(strcmp(path, "/proc") == 0);
	if (context->open_error) {
		errno = context->open_error;
		return NULL;
	}
	context->entry_index = 0;
	return context;
}

static int
fake_dir_read(void *opaque, void *directory, char *name, size_t capacity)
{
	struct fake_context *context = (struct fake_context *)opaque;
	size_t length;

	assert(directory == context);
	if (context->readdir_error_at >= 0 &&
	    (int)context->entry_index == context->readdir_error_at) {
		errno = EIO;
		return -1;
	}
	if (context->entry_index >= context->entry_count)
		return 0;
	length = strlen(context->entries[context->entry_index]);
	assert(length < capacity);
	memcpy(name, context->entries[context->entry_index], length + 1);
	context->entry_index++;
	return 1;
}

static void
fake_dir_close(void *opaque, void *directory)
{
	(void)opaque;
	(void)directory;
}

static int
fake_read_file(void *opaque, const char *path, char *buffer,
		   size_t capacity, size_t *length)
{
	struct fake_context *context = (struct fake_context *)opaque;
	long pid;
	char kind[16];
	struct fake_process *process;
	const char *value;
	size_t value_length;
	int error;

	if (sscanf(path, "/proc/%ld/%15s", &pid, kind) != 2) {
		errno = EINVAL;
		return -1;
	}
	if (strcmp(kind, "stat") != 0 && strcmp(kind, "cmdline") != 0) {
		errno = EINVAL;
		return -1;
	}
	process = fake_find(context, (pid_t)pid);
	if (!process) {
		errno = ENOENT;
		return -1;
	}
	error = strcmp(kind, "stat") == 0 ?
		process->stat_error : process->cmdline_error;
	if (strcmp(kind, "cmdline") == 0 && process->disappear_on_cmdline) {
		process->stat_error = ENOENT;
		errno = ENOENT;
		return -1;
	}
	if (error) {
		errno = error;
		return -1;
	}
	value = strcmp(kind, "stat") == 0 ? process->stat_text :
		process->cmdline;
	value_length = strcmp(kind, "stat") == 0 ? strlen(value) :
		process->cmdline_length;
	if (value_length >= capacity) {
		errno = EOVERFLOW;
		return -1;
	}
	memcpy(buffer, value, value_length);
	buffer[value_length] = '\0';
	*length = value_length;
	return 0;
}

static void *
fake_allocate(void *opaque, size_t size)
{
	struct fake_context *context = (struct fake_context *)opaque;
	context->allocate_calls++;
	if (context->fail_allocate_at == context->allocate_calls)
		return NULL;
	return malloc(size);
}

static void *
fake_reallocate(void *opaque, void *pointer, size_t size)
{
	struct fake_context *context = (struct fake_context *)opaque;
	context->reallocate_calls++;
	if (context->fail_reallocate_at == context->reallocate_calls)
		return NULL;
	return realloc(pointer, size);
}

static void
fake_release(void *opaque, void *pointer)
{
	(void)opaque;
	free(pointer);
}

static const struct process_inventory_ops fake_ops = {
	fake_dir_open, fake_dir_read, fake_dir_close, fake_read_file,
	fake_allocate, fake_reallocate, fake_release
};

static void
fake_init(struct fake_context *context, const char **entries, size_t count)
{
	memset(context, 0, sizeof(*context));
	context->entries = entries;
	context->entry_count = count;
	context->readdir_error_at = -1;
}

static void
fake_add(struct fake_context *context, pid_t pid, const char *comm,
	 const char *argv0)
{
	struct fake_process *process =
		&context->processes[context->process_count++];
	size_t length;

	process->pid = pid;
	snprintf(process->stat_text, sizeof(process->stat_text),
		 "%ld (%s) S 1 123 123 0 0 0 0 0 0 0 10 20 0 0 20 0 1 0 98765 1024 1",
		 (long)pid, comm);
	length = strlen(argv0);
	memcpy(process->cmdline, argv0, length);
	process->cmdline[length] = '\0';
	process->cmdline_length = length + 1;
}

static enum process_presence
run_scan(struct fake_context *context, struct process_snapshot *snapshot)
{
	return process_inventory_scan_with_ops("dnsmasq", snapshot, &fake_ops,
					       context);
}

static void
test_context_allocation_failure(void)
{
	const char *entries[] = { "123" };
	struct fake_context context;
	struct process_snapshot snapshot;
	fake_init(&context, entries, 1);
	fake_add(&context, 123, "dnsmasq", "/usr/sbin/dnsmasq");
	context.fail_allocate_at = 1;
	assert(run_scan(&context, &snapshot) == PROCESS_PRESENCE_UNKNOWN);
	assert(snapshot.count == 0 && snapshot.items == NULL);
	assert(snapshot.error == ENOMEM);
}

static void
test_proc_open_failure(void)
{
	struct fake_context context;
	struct process_snapshot snapshot;
	fake_init(&context, NULL, 0);
	context.open_error = EACCES;
	assert(run_scan(&context, &snapshot) == PROCESS_PRESENCE_UNKNOWN);
	assert(snapshot.error == EACCES);
}

static void
test_normal_eof_is_absent(void)
{
	struct fake_context context;
	struct process_snapshot snapshot;
	fake_init(&context, NULL, 0);
	assert(run_scan(&context, &snapshot) == PROCESS_PRESENCE_ABSENT);
	assert(snapshot.count == 0 && snapshot.items == NULL);
}

static void
test_readdir_error_is_unknown(void)
{
	const char *entries[] = { "123" };
	struct fake_context context;
	struct process_snapshot snapshot;
	fake_init(&context, entries, 1);
	context.readdir_error_at = 0;
	assert(run_scan(&context, &snapshot) == PROCESS_PRESENCE_UNKNOWN);
	assert(snapshot.error == EIO && snapshot.count == 0);
}

static void
test_disappeared_pid_is_skipped(void)
{
	const char *entries[] = { "123" };
	struct fake_context context;
	struct process_snapshot snapshot;
	fake_init(&context, entries, 1);
	fake_add(&context, 123, "dnsmasq", "/usr/sbin/dnsmasq");
	context.processes[0].stat_error = ENOENT;
	assert(run_scan(&context, &snapshot) == PROCESS_PRESENCE_ABSENT);
	assert(snapshot.count == 0);
}

static void
test_pid_disappearing_between_stat_and_cmdline_is_skipped(void)
{
	const char *entries[] = { "123" };
	struct fake_context context;
	struct process_snapshot snapshot;

	fake_init(&context, entries, 1);
	fake_add(&context, 123, "dnsmasq", "/usr/sbin/dnsmasq");
	context.processes[0].disappear_on_cmdline = 1;
	assert(run_scan(&context, &snapshot) == PROCESS_PRESENCE_ABSENT);
	assert(snapshot.count == 0 && snapshot.items == NULL);
}

static void
test_permission_and_io_failures_are_unknown(void)
{
	const char *entries[] = { "123" };
	struct fake_context context;
	struct process_snapshot snapshot;
	fake_init(&context, entries, 1);
	fake_add(&context, 123, "dnsmasq", "/usr/sbin/dnsmasq");
	context.processes[0].stat_error = EACCES;
	assert(run_scan(&context, &snapshot) == PROCESS_PRESENCE_UNKNOWN);

	fake_init(&context, entries, 1);
	fake_add(&context, 123, "dnsmasq", "/usr/sbin/dnsmasq");
	context.processes[0].stat_error = EIO;
	assert(run_scan(&context, &snapshot) == PROCESS_PRESENCE_UNKNOWN);
}

static void
test_malformed_stat_and_missing_cmdline_are_unknown(void)
{
	const char *entries[] = { "123" };
	struct fake_context context;
	struct process_snapshot snapshot;
	fake_init(&context, entries, 1);
	fake_add(&context, 123, "dnsmasq", "/usr/sbin/dnsmasq");
	strcpy(context.processes[0].stat_text, "not a stat record");
	assert(run_scan(&context, &snapshot) == PROCESS_PRESENCE_UNKNOWN);

	fake_init(&context, entries, 1);
	fake_add(&context, 123, "dnsmasq", "/usr/sbin/dnsmasq");
	context.processes[0].cmdline_error = EACCES;
	assert(run_scan(&context, &snapshot) == PROCESS_PRESENCE_UNKNOWN);
}

static void
test_argv0_allocation_failure_is_unknown(void)
{
	const char *entries[] = { "123" };
	struct fake_context context;
	struct process_snapshot snapshot;
	fake_init(&context, entries, 1);
	fake_add(&context, 123, "dnsmasq", "/usr/sbin/dnsmasq");
	context.fail_allocate_at = 2; /* context succeeds; argv0 copy fails */
	assert(run_scan(&context, &snapshot) == PROCESS_PRESENCE_UNKNOWN);
	assert(snapshot.error == ENOMEM && snapshot.count == 0);
}

static void
test_first_pid_vector_allocation_failure_is_unknown(void)
{
	const char *entries[] = { "123" };
	struct fake_context context;
	struct process_snapshot snapshot;
	fake_init(&context, entries, 1);
	fake_add(&context, 123, "dnsmasq", "/usr/sbin/dnsmasq");
	context.fail_allocate_at = 3; /* context, argv0, first PID slot */
	assert(run_scan(&context, &snapshot) == PROCESS_PRESENCE_UNKNOWN);
	assert(snapshot.error == ENOMEM && snapshot.count == 0);
}

static void
test_pid_vector_growth_failure_discards_partial_inventory(void)
{
	const char *entries[] = { "123", "456" };
	struct fake_context context;
	struct process_snapshot snapshot;
	fake_init(&context, entries, 2);
	fake_add(&context, 123, "dnsmasq", "/usr/sbin/dnsmasq");
	fake_add(&context, 456, "dnsmasq", "/usr/sbin/dnsmasq");
	context.fail_reallocate_at = 1;
	assert(run_scan(&context, &snapshot) == PROCESS_PRESENCE_UNKNOWN);
	assert(snapshot.error == ENOMEM && snapshot.count == 0);
	assert(snapshot.items == NULL);
}

static void
test_one_and_multiple_matches(void)
{
	const char *one_entries[] = { "123" };
	const char *many_entries[] = { "123", "456" };
	struct fake_context context;
	struct process_snapshot snapshot;

	fake_init(&context, one_entries, 1);
	fake_add(&context, 123, "dnsmasq", "/usr/sbin/dnsmasq");
	assert(run_scan(&context, &snapshot) == PROCESS_PRESENCE_RUNNING);
	assert(snapshot.count == 1 && snapshot.items[0].pid == 123);
	process_snapshot_release(&snapshot);

	fake_init(&context, many_entries, 2);
	fake_add(&context, 123, "dnsmasq", "/usr/sbin/dnsmasq");
	fake_add(&context, 456, "dnsmasq", "/usr/sbin/dnsmasq");
	assert(run_scan(&context, &snapshot) == PROCESS_PRESENCE_RUNNING);
	assert(snapshot.count == 2);
	process_snapshot_release(&snapshot);
}

static void
test_partial_match_then_readdir_error_is_unknown(void)
{
	const char *entries[] = { "123", "456" };
	struct fake_context context;
	struct process_snapshot snapshot;
	fake_init(&context, entries, 2);
	fake_add(&context, 123, "dnsmasq", "/usr/sbin/dnsmasq");
	fake_add(&context, 456, "dnsmasq", "/usr/sbin/dnsmasq");
	context.readdir_error_at = 1;
	assert(run_scan(&context, &snapshot) == PROCESS_PRESENCE_UNKNOWN);
	assert(snapshot.items == NULL && snapshot.count == 0);
	assert(snapshot.error == EIO);
}

int
main(void)
{
	test_context_allocation_failure();
	test_proc_open_failure();
	test_normal_eof_is_absent();
	test_readdir_error_is_unknown();
	test_disappeared_pid_is_skipped();
	test_pid_disappearing_between_stat_and_cmdline_is_skipped();
	test_permission_and_io_failures_are_unknown();
	test_malformed_stat_and_missing_cmdline_are_unknown();
	test_argv0_allocation_failure_is_unknown();
	test_first_pid_vector_allocation_failure_is_unknown();
	test_pid_vector_growth_failure_discards_partial_inventory();
	test_one_and_multiple_matches();
	test_partial_match_then_readdir_error_is_unknown();
	return 0;
}
