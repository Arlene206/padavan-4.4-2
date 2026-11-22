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
	char text[16];
	unsigned int parsed = 0;
	size_t i;
	int status;

	if (!count || maximum < 0)
		return EINVAL;
	status = router_account_nvram_read_copy(get_value, context, name,
						"0", text, sizeof(text));
	if (status != 0)
		return status;
	if (text[0] == '\0') {
		*count = 0;
		return 0;
	}
	for (i = 0; text[i] != '\0'; ++i) {
		unsigned int digit;
		if (text[i] < '0' || text[i] > '9')
			return EINVAL;
		digit = (unsigned int)(text[i] - '0');
		if (digit > (unsigned int)maximum ||
		    parsed > ((unsigned int)maximum - digit) / 10U)
			return ERANGE;
		parsed = parsed * 10U + digit;
	}
	*count = (int)parsed;
	return 0;
}
#endif /* PADAVAN_ROUTER_ACCOUNT_NVRAM_POLICY_H */
