#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../uqmi-command-queue-policy.h"

struct uqmi_cmd_handler {
	int type;
};

static int fail_reallocation;

static void *
test_reallocate(void *memory, size_t size)
{
	if (fail_reallocation) {
		errno = ENOMEM;
		return NULL;
	}
	return realloc(memory, size);
}

static void
test_append_and_insert_preserve_order(void)
{
	struct uqmi_cmd *commands = NULL;
	struct uqmi_cmd_handler options = {-1};
	struct uqmi_cmd_handler first = {1};
	struct uqmi_cmd_handler second = {2};
	int count = 0;

	assert(uqmi_cmd_queue_insert(&commands, &count, count, &options,
				     "device", test_reallocate) == 0);
	assert(uqmi_cmd_queue_insert(&commands, &count, count, &first,
				     "first", test_reallocate) == 0);
	assert(uqmi_cmd_queue_insert(&commands, &count, 1, &second,
				     "second", test_reallocate) == 0);
	assert(count == 3);
	assert(commands[0].handler == &options);
	assert(commands[1].handler == &second);
	assert(commands[2].handler == &first);
	assert(strcmp(commands[0].arg, "device") == 0);
	assert(strcmp(commands[1].arg, "second") == 0);
	assert(strcmp(commands[2].arg, "first") == 0);
	free(commands);
}

static void
test_allocation_failure_preserves_existing_queue(void)
{
	struct uqmi_cmd initial = {0};
	struct uqmi_cmd *commands = malloc(sizeof(*commands));
	struct uqmi_cmd *original;
	struct uqmi_cmd_handler first = {1};
	struct uqmi_cmd_handler second = {2};
	int count = 1;

	assert(commands != NULL);
	commands[0] = initial;
	commands[0].handler = &first;
	commands[0].arg = "stable";
	original = commands;

	fail_reallocation = 1;
	assert(uqmi_cmd_queue_insert(&commands, &count, 0, &second, "lost",
				     test_reallocate) == ENOMEM);
	fail_reallocation = 0;

	assert(commands == original);
	assert(count == 1);
	assert(commands[0].handler == &first);
	assert(strcmp(commands[0].arg, "stable") == 0);
	free(commands);
}

static void
test_invalid_index_and_count_overflow_are_rejected(void)
{
	struct uqmi_cmd initial = {0};
	struct uqmi_cmd *commands = &initial;
	struct uqmi_cmd_handler handler = {1};
	int count = 1;

	assert(uqmi_cmd_queue_insert(&commands, &count, 2, &handler, "bad",
				     test_reallocate) == EINVAL);
	assert(commands == &initial && count == 1);

	count = INT_MAX;
	assert(uqmi_cmd_queue_insert(&commands, &count, 0, &handler, "overflow",
				     test_reallocate) == EOVERFLOW);
	assert(commands == &initial && count == INT_MAX);
}

int
main(void)
{
	test_append_and_insert_preserve_order();
	test_allocation_failure_preserves_existing_queue();
	test_invalid_index_and_count_overflow_are_rejected();
	puts("uqmi command queue allocation and ordering policy: PASS");
	return 0;
}
