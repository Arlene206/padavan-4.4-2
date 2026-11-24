#ifndef PADAVAN_LLDT_CONFIG_PATH_POLICY_H
#define PADAVAN_LLDT_CONFIG_PATH_POLICY_H

#include <errno.h>
#include <stddef.h>
#include <string.h>

#define LLDT_CONFIG_PATH_PREFIX "/etc/"
#define LLDT_CONFIG_PATH_PREFIX_LEN 5U

/* Preserve relative-path compatibility and always append a terminating NUL. */
static inline int
lldt_config_path_copy(const char *value, char *path, size_t capacity)
{
	size_t value_len;
	size_t prefix_len;

	if (!value || !path || value[0] == '\0') {
		errno = EINVAL;
		return -1;
	}

	value_len = strlen(value);
	prefix_len = value[0] == '/' ? 0 : LLDT_CONFIG_PATH_PREFIX_LEN;
	if (capacity <= prefix_len || value_len >= capacity - prefix_len) {
		errno = ENAMETOOLONG;
		return -1;
	}

	if (prefix_len != 0)
		memcpy(path, LLDT_CONFIG_PATH_PREFIX, prefix_len);
	memcpy(path + prefix_len, value, value_len + 1);
	return 0;
}

#endif /* PADAVAN_LLDT_CONFIG_PATH_POLICY_H */
