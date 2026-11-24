#ifndef PADAVAN_ROUTER_ACCOUNT_NVRAM_POLICY_H
#define PADAVAN_ROUTER_ACCOUNT_NVRAM_POLICY_H
#include <errno.h>
#include <stddef.h>
#include <string.h>
#define ROUTER_ACCOUNT_NV_MAX_ACCOUNTS 100
#define ROUTER_ACCOUNT_NV_NAME_SIZE 32

typedef int (*router_account_nvram_get_fn)(void *context, const char *name, char **value);

/* nvram_get_checked() returns a shared-cache pointer; copy it before another read. */
static inline int
router_account_nvram_read_copy(router_account_nvram_get_fn get_value,
                               void *context, const char *name,
                               const char *missing_default,
                               char *destination, size_t destination_size)
{
	char *value = NULL;
	const char *source;
	size_t length;
	int result, saved_errno;

	if (!get_value || !name || !destination || destination_size == 0)
		return EINVAL;
	errno = 0;
	result = get_value(context, name, &value);
	saved_errno = errno;
	if (result < 0)
		return saved_errno ? saved_errno : EIO;
	if (result == 0)
		source = missing_default ? missing_default : "";
	else if (result == 1 && value)
		source = value;
	else
		return EIO;
	for (length = 0; length < destination_size; ++length)
		if (source[length] == '\0')
			break;
	if (length >= destination_size)
		return ENAMETOOLONG;
	memcpy(destination, source, length + 1);
	return 0;
}

/* Missing count means zero; reject malformed and out-of-range configured values. */
static inline int
router_account_nvram_read_count(router_account_nvram_get_fn get_value,
                                void *context, const char *name,
                                int maximum, int *count)
{
	char *value = NULL;
	const unsigned char *cursor;
	unsigned int parsed = 0;
	unsigned int limit;
	int result, saved_errno;

	if (!get_value || !name || !count || maximum < 0)
		return EINVAL;
	errno = 0;
	result = get_value(context, name, &value);
	saved_errno = errno;
	if (result < 0)
		return saved_errno ? saved_errno : EIO;
	if (result == 0) {
		*count = 0;
		return 0;
	}
	if (result != 1 || !value)
		return EIO;

	/*
	 * Consume the shared-cache pointer in this call, before another NVRAM
	 * read can invalidate it. Parse in place so arbitrarily long decimal
	 * values are classified as ERANGE rather than copy-buffer ENAMETOOLONG.
	 */
	limit = (unsigned int)maximum;
	cursor = (const unsigned char *)value;
	while (*cursor != '\0') {
		unsigned int digit;
		if (*cursor < '0' || *cursor > '9')
			return EINVAL;
		digit = (unsigned int)(*cursor - '0');
		if (parsed > limit / 10U ||
		    (parsed == limit / 10U && digit > limit % 10U))
			return ERANGE;
		parsed = parsed * 10U + digit;
		++cursor;
	}
	*count = (int)parsed;
	return 0;
}
#endif /* PADAVAN_ROUTER_ACCOUNT_NVRAM_POLICY_H */
