#ifndef INFOSVR_INTERFACE_POLICY_H
#define INFOSVR_INTERFACE_POLICY_H

#include <errno.h>
#include <stddef.h>
#include <string.h>

/* Linux interface names must fit, including their terminating NUL. */
static inline int
infosvr_interface_name_copy(char *destination, size_t capacity,
                            const char *source)
{
	size_t length;

	if (!destination || capacity == 0 || !source || !source[0]) {
		errno = EINVAL;
		return -1;
	}
	length = strlen(source);
	if (length >= capacity) {
		errno = ENAMETOOLONG;
		return -1;
	}
	memcpy(destination, source, length + 1);
	return 0;
}

#endif /* INFOSVR_INTERFACE_POLICY_H */
