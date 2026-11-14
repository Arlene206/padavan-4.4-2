#ifndef PROCESS_INVENTORY_H
#define PROCESS_INVENTORY_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

/* This is not a Boolean: callers must compare the state explicitly. */
enum process_presence {
	PROCESS_PRESENCE_UNKNOWN = -1,
	PROCESS_PRESENCE_ABSENT = 0,
	PROCESS_PRESENCE_RUNNING = 1
};

struct process_identity {
	pid_t pid;
	unsigned long long start_time;
	unsigned long long argv0_hash;
	unsigned long long argv1_hash;
	char comm[16];
	unsigned char comm_truncated;
};

struct process_snapshot {
	enum process_presence state;
	struct process_identity *items;
	size_t count;
	int error;
	void (*release_items)(void *context, void *pointer);
	void *release_context;
};

/*
 * dir_read: 1 = entry, 0 = clean EOF, -1 = error (errno is set).
 * read_file: 0 = complete file, -1 = error or truncation.
 */
struct process_inventory_ops {
	void *(*dir_open)(void *context, const char *path);
	int (*dir_read)(void *context, void *directory, char *name,
			size_t capacity);
	void (*dir_close)(void *context, void *directory);
	int (*read_file)(void *context, const char *path, char *buffer,
		 size_t capacity, size_t *length);
	void *(*allocate)(void *context, size_t size);
	void *(*reallocate)(void *context, void *pointer, size_t size);
	void (*release)(void *context, void *pointer);
};

enum process_presence
process_inventory_scan_with_ops(const char *process_name,
				struct process_snapshot *snapshot,
				const struct process_inventory_ops *ops,
				void *context);

enum process_presence
process_identity_validate_with_ops(const char *process_name,
				   const struct process_identity *identity,
				   const struct process_inventory_ops *ops,
				   void *context);

enum process_presence
process_inventory_scan(const char *process_name, struct process_snapshot *snapshot);

enum process_presence
process_identity_validate(const char *process_name,
			  const struct process_identity *identity);

static inline void
process_snapshot_release(struct process_snapshot *snapshot)
{
	if (!snapshot)
		return;
	if (snapshot->items) {
		if (snapshot->release_items)
			snapshot->release_items(snapshot->release_context,
					       snapshot->items);
		else
			free(snapshot->items);
	}
	memset(snapshot, 0, sizeof(*snapshot));
	snapshot->state = PROCESS_PRESENCE_UNKNOWN;
}

#endif
