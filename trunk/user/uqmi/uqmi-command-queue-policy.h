#ifndef UQMI_COMMAND_QUEUE_POLICY_H
#define UQMI_COMMAND_QUEUE_POLICY_H

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <string.h>

struct uqmi_cmd_handler;

struct uqmi_cmd {
	const struct uqmi_cmd_handler *handler;
	char *arg;
};

typedef void *(*uqmi_cmd_realloc_fn)(void *memory, size_t size);

/*
 * Insert only after a successful allocation. On every failure the original
 * pointer, count and entries remain untouched.
 */
static inline int
uqmi_cmd_queue_insert(struct uqmi_cmd **commands, int *count, int index,
		      const struct uqmi_cmd_handler *handler, char *arg,
		      uqmi_cmd_realloc_fn reallocate)
{
	struct uqmi_cmd *resized;

	if (!commands || !count || !handler || !reallocate ||
	    *count < 0 || index < 0 || index > *count)
		return EINVAL;
	if (*count == INT_MAX ||
	    (size_t)*count >= ((size_t)-1) / sizeof(**commands))
		return EOVERFLOW;

	errno = 0;
	resized = reallocate(*commands,
			     ((size_t)*count + 1) * sizeof(**commands));
	if (!resized)
		return errno ? errno : ENOMEM;

	memmove(&resized[index + 1], &resized[index],
		(size_t)(*count - index) * sizeof(**commands));
	resized[index].handler = handler;
	resized[index].arg = arg;
	*commands = resized;
	(*count)++;
	return 0;
}

#endif /* UQMI_COMMAND_QUEUE_POLICY_H */
